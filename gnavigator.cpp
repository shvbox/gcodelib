#include "gnavigator.h"

#include <QDebug>

GNavigator::GNavigator(GCode *data, QObject *parent) 
    : QObject(parent),
      mGCode(data),
      mRootItem(NULL)
{
    connect(mGCode, SIGNAL(dataChanged(int, int)), this, SIGNAL(dataChanged(int,int)));
    connect(mGCode, SIGNAL(selectionChanged(int,int)), this, SIGNAL(selectionChanged(int,int)));
    connect(mGCode, SIGNAL(visibilityChanged(int,int)), this, SIGNAL(visibilityChanged(int,int)));
    connect(mGCode, SIGNAL(beginReset()), this, SLOT(beginResetData()));
    connect(mGCode, SIGNAL(endReset()), this, SLOT(endResetData()));
    
    setupModelData();
}

GNavigator::~GNavigator()
{
    delete mRootItem;
}

GNavigatorItem* GNavigator::itemAtZ(double z)
{
    GNavigatorItem* item = mZMap.value(z);
    if (item) {
        return item;
    }
    
    for (int i = 0, lim = mRootItem->childCount(); i < lim; ++i) {
        item = mRootItem->child(i);
        if (qFuzzyCompare(item->info().z, z)) {
            mZMap.insert(z, item);
            return item;
        }
    }
    
    mZMap.insert(z, NULL);
    
    return NULL;
}

GNavigatorItem *GNavigator::itemAtZ(double z) const
{
    GNavigatorItem* item = mZMap.value(z);
    if (item) {
        return item;
    }
    
    for (int i = 0, lim = mRootItem->childCount(); i < lim; ++i) {
        item = mRootItem->child(i);
        if (qFuzzyCompare(item->info().z, z)) {
            return item;
        }
    }
    
    return NULL;
}

void GNavigator::select(GNavigatorItem *begin, GNavigatorItem *end)
{
    if (begin->parentItem() != end->parentItem()) {
        select(begin);
    }
    
    int firstLine = qMin(begin->firstLine(), end->firstLine());
    int lastLine = qMax(begin->lastLine(), end->lastLine());
    
    mGCode->select(firstLine, lastLine);
}

void GNavigator::deselect(GNavigatorItem *begin, GNavigatorItem *end)
{
    if (begin->parentItem() != end->parentItem()) {
        deselect(begin);
    }
    
    int firstLine = qMin(begin->firstLine(), end->firstLine());
    int lastLine = qMax(begin->lastLine(), end->lastLine());
    
    mGCode->deselect(firstLine, lastLine);
}

void GNavigator::toggleSelection(GNavigatorItem *begin, GNavigatorItem *end)
{
    if (begin->parentItem() != end->parentItem()) {
        toggleSelection(begin);
    }
    
    int firstLine = qMin(begin->firstLine(), end->firstLine());
    int lastLine = qMax(begin->lastLine(), end->lastLine());
    
    mGCode->toggleSelection(firstLine, lastLine);
}

void GNavigator::show(GNavigatorItem *begin, GNavigatorItem *end)
{
    if (begin->parentItem() != end->parentItem()) {
        show(begin);
    }
    
    int firstLine = qMin(begin->firstLine(), end->firstLine());
    int lastLine = qMax(begin->lastLine(), end->lastLine());
    
    mGCode->show(firstLine, lastLine);
}

void GNavigator::hide(GNavigatorItem *begin, GNavigatorItem *end)
{
    if (begin->parentItem() != end->parentItem()) {
        hide(begin);
    }
    
    int firstLine = qMin(begin->firstLine(), end->firstLine());
    int lastLine = qMax(begin->lastLine(), end->lastLine());
    
    mGCode->hide(firstLine, lastLine);
}

void GNavigator::toggleVisible(GNavigatorItem *begin, GNavigatorItem *end)
{
    if (begin->parentItem() != end->parentItem()) {
        toggleVisible(begin);
    }
    
    int firstLine = qMin(begin->firstLine(), end->firstLine());
    int lastLine = qMax(begin->lastLine(), end->lastLine());
    
    mGCode->toggleVisible(firstLine, lastLine);
}

//GNavigatorItem *GNavigator::parent(GNavigatorItem *child) const
//{
    
//}

//GNavigatorItem *GNavigator::child(GNavigatorItem *parent) const
//{
    
//}

void GNavigator::beginResetData()
{
    emit beginReset();
}

void GNavigator::endResetData()
{
    delete mRootItem;
    setupModelData();
//    qDebug() << __PRETTY_FUNCTION__;
    emit endReset();
}

void GNavigator::calculateRouteData(int move, GNavigatorItemInfo *pRouteData)
{
    double dE = mGCode->dEe(move);
    double dist = mGCode->distance(move);
    
    pRouteData->l += dist;
    if (dE != 0) pRouteData->lE += dist;
    pRouteData->dE += dE;
    if (dist > 0) pRouteData->dEl += dE;
}

void GNavigator::setupModelData()
{
    QList<QVariant> rootData;
    rootData << QString("root");
    mRootItem = new GNavigatorItem(0, mGCode->linesCount() - 1, rootData);
    
    if (mGCode->linesCount() == 0) return;
    
    double z = 0.0;
    GNavigatorItemInfo layerData(z);
    GNavigatorItemInfo routeData(z);
    
    GNavigatorItem *layer = new GNavigatorItem(0, mRootItem);
    GNavigatorItem *comment = NULL;
    GNavigatorItem *route = NULL;
    
    for (int line = 0; line < mGCode->linesCount(); ++line) {
        GLine::LineType lineType = mGCode->lineType(line);
        GNavigatorItem::ItemType itemType = GNavigatorItem::Invalid;
        int move = -1;
        
        if (lineType == GLine::Command) {
            move = mGCode->lineToMove(line);
            if (move >= 0) {
                double zm = mGCode->Z(move);
                if (z == zm) {
                    itemType = GNavigatorItem::Route;
                    
                } else {
                    itemType = GNavigatorItem::Layer;
                }
                
            } else {
                itemType = GNavigatorItem::Command;
            }
            
        } else if (lineType == GLine::Comment) {
            itemType = GNavigatorItem::Comment;
            
        } else {
            continue;
        }
        
        
        switch (itemType) {
        case GNavigatorItem::Comment:
            if (!comment) {
                comment = new GNavigatorItem(line, route ? route : layer);
                comment->setType(itemType);
                comment->appendData(";");
            }
            break;
            
        case GNavigatorItem::Command: {
            finishCommentItem(comment, line - 1);
            comment = NULL;
            
            GNavigatorItem *command = new GNavigatorItem(line, route ? route : layer);
            command->setType(itemType);
            command->appendData(mGCode->command(line));
            command->appendData(mGCode->comment(line));
        }
            break;
            
        case GNavigatorItem::Route: {
            finishCommentItem(comment, line - 1);
            comment = NULL;
            
            if (!route) {
                route = startRouteItem(line, layer);
                routeData = GNavigatorItemInfo(z);
                
            } else {
                if (mGCode->dEe(move) == 0.0 && mGCode->distance(move) > 0.0 && routeData.dE != 0.0) {
                    finishRouteItem(route, line - 1, routeData, &layerData);
                    
                    route = startRouteItem(line, layer);
                    routeData = GNavigatorItemInfo(z);
                }
            }

            calculateRouteData(move, &routeData);
        }
            break;
            
        case GNavigatorItem::Layer: {
            finishCommentItem(comment, line - 1);
            comment = NULL;
            finishRouteItem(route, line - 1, routeData, &layerData);
            route = NULL;
            
            finishLayerItem(layer, line - 1, layerData);
            
            layer = new GNavigatorItem(line, mRootItem);
            z = mGCode->Z(move);
            layerData = GNavigatorItemInfo(z);
            
            route = startRouteItem(line, layer);
            routeData = GNavigatorItemInfo(z);
            calculateRouteData(move, &routeData);
        }
            break;
            
//        case GNavigatorItem::Command: {
//        }
//            break;

        default:
            break;
        }
        
    }
    
    finishCommentItem(comment, mGCode->linesCount() - 1);
    finishRouteItem(route, mGCode->linesCount() - 1, routeData, &layerData);
    finishLayerItem(layer, mGCode->linesCount() - 1, layerData);
    
//    GNavigatorItem *firstItem = mRootItem->child(0);
//    mGCode->show(firstItem->firstLine(), firstItem->lastLine());
}

void GNavigator::finishLayerItem(GNavigatorItem *item, int lastLine, GNavigatorItemInfo data)
{
    if (item) {
        item->setType(GNavigatorItem::Layer);
        item->setLastLine(lastLine);
        item->setInfo(data);
    }
}

GNavigatorItem* GNavigator::startRouteItem(int firstLine, GNavigatorItem *layer)
{
    GNavigatorItem *route = new GNavigatorItem(firstLine, layer);
    route->setType(GNavigatorItem::Route);
    
    return route;
}

void GNavigator::finishRouteItem(GNavigatorItem *item, int lastLine, GNavigatorItemInfo data, GNavigatorItemInfo *pLayerData)
{
    if (item) {
        item->setLastLine(lastLine);
        item->setInfo(data);
        
        pLayerData->l += data.l;
        pLayerData->lE += data.lE;
        pLayerData->dE += data.dE;
        pLayerData->dEl += data.dEl;
        
    }
}

void GNavigator::finishCommentItem(GNavigatorItem *item, int lastLine)
{
    if (item) {
        item->setLastLine(lastLine);
        
        int line = item->firstLine();
        QString text = mGCode->comment(line++);
        while (text.trimmed().isEmpty() && line <= lastLine) {
            text = mGCode->comment(line++);
        }
        item->appendData(text);
    }
}

Qt::CheckState GNavigator::testState(GNavigatorItem *item, const QBitArray &state) const
{
    QBitArray ba(state);
    ba.fill(true, item->firstLine(), item->lastLine() + 1);
    ba ^= state;
    
    int cnt = ba.count(true);
    if (cnt == 0) {
        return Qt::Checked;
    }
    
    int len = item->lastLine() - item->firstLine() + 1;
    if (cnt == len) {
        return Qt::Unchecked;
    }
    
    return Qt::PartiallyChecked;
}


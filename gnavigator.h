#ifndef GNAVIGATOR_H
#define GNAVIGATOR_H

#include <QObject>
#include <QMap>

#include "gcode.h"
#include "gnavigatoritem.h"

class GNavigator : public QObject
{
    Q_OBJECT
public:
    GNavigator(GCode *data, QObject *parent = 0);
    virtual ~GNavigator();
 
    
    GNavigatorItem* root() const { return mRootItem; }
//    GNavigatorItem* parent(GNavigatorItem* child) const;
//    GNavigatorItem* child(GNavigatorItem* parent) const;
    
    GNavigatorItem* itemAtZ(double z);
    GNavigatorItem* itemAtZ(double z) const;
    
    Qt::CheckState selected(GNavigatorItem* item) const { return testState(item, mGCode->selection()); }
    void selectAll() { mGCode->selectAll(); }
    void select(GNavigatorItem* item) { mGCode->select(item->firstLine(), item->lastLine()); }
    void select(GNavigatorItem* begin, GNavigatorItem* end);
    void deselectAll() { mGCode->deselectAll(); }
    void deselect(GNavigatorItem* item) { mGCode->deselect(item->firstLine(), item->lastLine()); }
    void deselect(GNavigatorItem* begin, GNavigatorItem* end);
    void toggleSelection(GNavigatorItem* item) { mGCode->toggleSelection(item->firstLine(), item->lastLine()); }
    void toggleSelection(GNavigatorItem* begin, GNavigatorItem* end);
    
    Qt::CheckState visible(GNavigatorItem* item) const { return testState(item, mGCode->visibility()); }
    void showAll() { mGCode->showAll(); }
    void show(GNavigatorItem* item) { mGCode->show(item->firstLine(), item->lastLine()); }
    void show(GNavigatorItem* begin, GNavigatorItem* end);
    void hideAll() { mGCode->hideAll(); }
    void hide(GNavigatorItem* item) { mGCode->hide(item->firstLine(), item->lastLine()); }
    void hide(GNavigatorItem* begin, GNavigatorItem* end);
    void toggleVisible(GNavigatorItem* item) { mGCode->toggleVisible(item->firstLine(), item->lastLine()); }
    void toggleVisible(GNavigatorItem* begin, GNavigatorItem* end);
    
signals:
    void dataChanged(int top, int bottom);
    void selectionChanged(int top, int bottom);
    void visibilityChanged(int top, int bottom);
    void beginReset();
    void endReset();
    
public slots:
    
protected slots:
    void beginResetData();
    void endResetData();
    
private:
    void setupModelData();
    void finishLayerItem(GNavigatorItem *item, int lastLine, GNavigatorItemInfo data);
    GNavigatorItem *startRouteItem(int firstLine, GNavigatorItem *layer);
    void finishRouteItem(GNavigatorItem *item, int lastLine, GNavigatorItemInfo data, GNavigatorItemInfo *layerData);
    void finishCommentItem(GNavigatorItem *item, int lastLine);
    void calculateRouteData(int move, GNavigatorItemInfo *pRouteData);
    
    Qt::CheckState testState(GNavigatorItem* item, const QBitArray &state) const;
    
    GCode *mGCode;
    GNavigatorItem *mRootItem;
    
    QMap<double, GNavigatorItem*> mZMap;
};

#endif // GNAVIGATOR_H

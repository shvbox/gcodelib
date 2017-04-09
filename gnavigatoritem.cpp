#include "gnavigatoritem.h"

#include <QStringList>
#include <QDebug>

GNavigatorItem::GNavigatorItem(int firstLine, int lastLine, const QList<QVariant> &data, GNavigatorItem *parent)
    : mParentItem(parent),
      mType(Invalid),
      mData(data),
      mFirstLine(firstLine),
      mLastLine(firstLine)
{
    if (mParentItem == 0) {
        mType = Root;
        
    } else {
        mParentItem->appendChild(this);
    }
    
    setLastLine(lastLine);
}

GNavigatorItem::GNavigatorItem(int firstLine, GNavigatorItem *parent)
    : mParentItem(parent),
      mType(Invalid),
      mFirstLine(firstLine),
      mLastLine(firstLine)
{
    if (mParentItem == 0) {
        mType = Root;
        
    } else {
        mParentItem->appendChild(this);
    }
}

GNavigatorItem::~GNavigatorItem()
{
//    qDebug() << __PRETTY_FUNCTION__;
    qDeleteAll(mChildItems);
}

bool GNavigatorItem::setLastLine(int lastLine)
{
    if (lastLine >= mFirstLine) {
        mLastLine = lastLine;
        return true;
    }
    
    mLastLine = mFirstLine;
    return false;
}

void GNavigatorItem::setInfo(const GNavigatorItemInfo &info)
{
    mInfo = info;
}

void GNavigatorItem::setData(const QList<QVariant> &data)
{
    mData = data;
}

void GNavigatorItem::appendData(const QVariant &dataItem)
{
    mData.append(dataItem);
}

void GNavigatorItem::appendChild(GNavigatorItem *item)
{
    mChildItems.append(item);
}

GNavigatorItem *GNavigatorItem::child(int row)
{
    return mChildItems.value(row);
}

void GNavigatorItem::setType(GNavigatorItem::ItemType type)
{
    if (mType == Invalid) {
        mType = type;
    }
}

int GNavigatorItem::childCount() const
{
    return mChildItems.count();
}

int GNavigatorItem::dataSize() const
{
    return mData.count();
}

QVariant GNavigatorItem::data(int i) const
{
    return mData.value(i);
}

GNavigatorItem *GNavigatorItem::parentItem()
{
    return mParentItem;
}

int GNavigatorItem::row() const
{
    if (mParentItem) {
        return mParentItem->mChildItems.indexOf(const_cast<GNavigatorItem*>(this));
    }

    return 0;
}

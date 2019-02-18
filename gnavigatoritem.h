#ifndef GNAVIGATORITEM_H
#define GNAVIGATORITEM_H

#include <QList>
#include <QVariant>

struct GNavigatorItemInfo {
    GNavigatorItemInfo(double z = 0.0, double l = 0.0, double lE = 0.0, double dE = 0.0, double dEl = 0.0) 
        : z(z), l(l), lE(lE), dE(dE), dEl(dEl) {}
    double z;  // z value
    double l;  // distance
    double lE; // distance with extrusion
    double dE; // extrusion length
    double dEl; // extrusion length excepting retracts
};

class GNavigatorItem
{
public:
    enum ItemType {
        Invalid = 0x0,
        Root = 0x1,
        Layer = 0x2,
        Comment = 0x4,
        Command = 0x8,
        Route = 0x10
    };
    
    GNavigatorItem(int firstLine, int lastLine, const QList<QVariant> &data, GNavigatorItem *parent = 0);
    GNavigatorItem(int firstLine, GNavigatorItem *parent = 0);
    ~GNavigatorItem();

    bool setLastLine(int lastLine);
    void setInfo(const GNavigatorItemInfo &info);
    void setData(const QList<QVariant> &data);
    void appendData(const QVariant &dataItem);
    
    void appendChild(GNavigatorItem *child);

    GNavigatorItem* parentItem();
    GNavigatorItem* child(int row);
    GNavigatorItem& child(int row) const;
    GNavigatorItem* firstChild() { return mChildItems.isEmpty() ? NULL : mChildItems.first(); }
    GNavigatorItem* lastChild() { return mChildItems.isEmpty() ? NULL : mChildItems.last(); }
    
    ItemType type() const { return mType; }
    void setType(ItemType type);
    
    int firstLine() const { return mFirstLine; }
    int lastLine() const { return mLastLine; }

    int row() const;
    int childCount() const;
    
    GNavigatorItemInfo info() const { return mInfo; }
    int dataSize() const;
    QVariant data(int i) const;
    
    bool operator==(const GNavigatorItem &v) const { return mFirstLine == v.mFirstLine && mLastLine == v.mLastLine; }
    bool operator!=(const GNavigatorItem &v) const { return !(*this == v); }
    
private:
    GNavigatorItem *mParentItem;
    QList<GNavigatorItem*> mChildItems;
    
    ItemType mType;
    
    GNavigatorItemInfo mInfo;
    QList<QVariant> mData;
    
    int mFirstLine;
    int mLastLine;
};

#endif // GNAVIGATORITEM_H

#include <QStringList>

 #include "bnp_treeitem.h"

 BNPTreeItem::BNPTreeItem(const QList<QVariant> &data, BNPTreeItem *parent)
 {
     parentItem = parent;
     itemData = data;
 }

 BNPTreeItem::~BNPTreeItem()
 {
     qDeleteAll(childItems);
 }

 void BNPTreeItem::appendChild(BNPTreeItem *item)
 {
     childItems.append(item);
 }

 BNPTreeItem *BNPTreeItem::child(int row)
 {
     return childItems.value(row);
 }

 int BNPTreeItem::childCount() const
 {
     return childItems.count();
 }

 int BNPTreeItem::columnCount() const
 {
     return itemData.count();
 }

 QVariant BNPTreeItem::data(int column) const
 {
     return itemData.value(column);
 }

 BNPTreeItem *BNPTreeItem::parent()
 {
     return parentItem;
 }

 int BNPTreeItem::row() const
 {
     if (parentItem)
         return parentItem->childItems.indexOf(const_cast<BNPTreeItem*>(this));

     return 0;
 }

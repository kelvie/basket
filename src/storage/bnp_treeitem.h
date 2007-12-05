#ifndef BNP_TREEITEM_H
 #define BNP_TREEITEM_H

 #include <QList>
 #include <QVariant>

 class BNPTreeItem
 {
 public:
     BNPTreeItem(const QList<QVariant> &data, BNPTreeItem *parent = 0);
     ~BNPTreeItem();

     void appendChild(BNPTreeItem *child);

     BNPTreeItem *child(int row);
     int childCount() const;
     int columnCount() const;
     QVariant data(int column) const;
     int row() const;
     BNPTreeItem *parent();

 private:
     QList<BNPTreeItem*> childItems;
     QList<QVariant> itemData;
     BNPTreeItem *parentItem;
 };

 #endif


#include <QtGui>

 #include "bnp_treeitem.h"
 #include "bnp_treemodel.h"

BNPTreeModel::BNPTreeModel(const QString &data, QObject *parent)
     : QAbstractItemModel(parent)
 {
     QList<QVariant> rootData;
     rootData << "Title" << "Summary";
     rootItem = new BNPTreeItem(rootData);
     setupModelData(data.split(QString("\n")), rootItem);
 }

 BNPTreeModel::~BNPTreeModel()
 {
     delete rootItem;
 }

 int BNPTreeModel::columnCount(const QModelIndex &parent) const
 {
     if (parent.isValid())
         return static_cast<BNPTreeItem*>(parent.internalPointer())->columnCount();
     else
         return rootItem->columnCount();
 }

 QVariant BNPTreeModel::data(const QModelIndex &index, int role) const
 {
     if (!index.isValid())
         return QVariant();

     if (role != Qt::DisplayRole)
         return QVariant();

     BNPTreeItem *item = static_cast<BNPTreeItem*>(index.internalPointer());

     return item->data(index.column());
 }

 Qt::ItemFlags BNPTreeModel::flags(const QModelIndex &index) const
 {
     if (!index.isValid())
         return 0;

     return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
 }

 QVariant BNPTreeModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
 {
     if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
         return rootItem->data(section);

     return QVariant();
 }

 QModelIndex BNPTreeModel::index(int row, int column, const QModelIndex &parent)
             const
 {
     if (!hasIndex(row, column, parent))
         return QModelIndex();

     BNPTreeItem *parentItem;

     if (!parent.isValid())
         parentItem = rootItem;
     else
         parentItem = static_cast<BNPTreeItem*>(parent.internalPointer());

     BNPTreeItem *childItem = parentItem->child(row);
     if (childItem)
         return createIndex(row, column, childItem);
     else
         return QModelIndex();
 }

 QModelIndex BNPTreeModel::parent(const QModelIndex &index) const
 {
     if (!index.isValid())
         return QModelIndex();

     BNPTreeItem *childItem = static_cast<BNPTreeItem*>(index.internalPointer());
     BNPTreeItem *parentItem = childItem->parent();

     if (parentItem == rootItem)
         return QModelIndex();

     return createIndex(parentItem->row(), 0, parentItem);
 }

 int BNPTreeModel::rowCount(const QModelIndex &parent) const
 {
     BNPTreeItem *parentItem;
     if (parent.column() > 0)
         return 0;

     if (!parent.isValid())
         parentItem = rootItem;
     else
         parentItem = static_cast<BNPTreeItem*>(parent.internalPointer());

     return parentItem->childCount();
 }

 void BNPTreeModel::setupModelData(const QStringList &lines, BNPTreeItem *parent)
 {
     QList<BNPTreeItem*> parents;
     QList<int> indentations;
     parents << parent;
     indentations << 0;

     int number = 0;

     while (number < lines.count()) {
         int position = 0;
         while (position < lines[number].length()) {
             if (lines[number].mid(position, 1) != " ")
                 break;
             position++;
         }

         QString lineData = lines[number].mid(position).trimmed();

         if (!lineData.isEmpty()) {
             // Read the column data from the rest of the line.
             QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
             QList<QVariant> columnData;
             for (int column = 0; column < columnStrings.count(); ++column)
                 columnData << columnStrings[column];

             if (position > indentations.last()) {
                 // The last child of the current parent is now the new parent
                 // unless the current parent has no children.

                 if (parents.last()->childCount() > 0) {
                     parents << parents.last()->child(parents.last()->childCount()-1);
                     indentations << position;
                 }
             } else {
                 while (position < indentations.last() && parents.count() > 0) {
                     parents.pop_back();
                     indentations.pop_back();
                 }
             }

             // Append a new item to the current parent's list of children.
             parents.last()->appendChild(new BNPTreeItem(columnData, parents.last()));
         }

         number++;
     }
 }

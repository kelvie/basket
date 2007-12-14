#ifndef BASKETDELEGATE_H
#define BASKETDELEGATE_H

#include <QItemDelegate>

namespace Akonadi {
}

class BasketDelegate : public QItemDelegate {
	Q_OBJECT

	public:
		BasketDelegate( QObject* parent = 0 );
		QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const;

};

#endif //BASKETDELEGATE_H


#ifndef BASKETCONTENT_H
#define BASKETCONTENT_H

#include <QGraphicsScene>
#include <QHash>

class KJob;
class NoteWidget;

namespace Akonadi {
	class Item;
}

class BasketContent : public QGraphicsScene {
	Q_OBJECT

	public:
		BasketContent( int basketId, QObject* parent = 0 );
		~BasketContent();

	public slots:
		void itemAdded( const Akonadi::Item& item );

		void toggleFormatTextBold();

	private slots:
		void mousePressEvent( QGraphicsSceneMouseEvent* mouseEvent );
		void addItemDone( KJob* job );
		void itemListFetched( KJob* job	);

	private:
		QHash<int, NoteWidget*> mNoteIdToNoteWidget;

		int mBasketId;
};

#endif

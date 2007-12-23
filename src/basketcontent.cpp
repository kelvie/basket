#include <QLinearGradient>
#include <QGraphicsSceneMouseEvent>

#include <KDebug>

#include "basketcontent.h"
#include "notewidget.h"
#include "note.h"

#include <libakonadi/collection.h>
#include <libakonadi/itemappendjob.h>
#include <libakonadi/item.h>
#include <libakonadi/itemfetchjob.h>

#include <boost/shared_ptr.hpp>

using namespace Akonadi;

BasketContent::BasketContent( int basketId, QObject* parent ) : QGraphicsScene( parent ), mBasketId( basketId ) {
	ItemFetchJob* job = new ItemFetchJob( Collection(basketId), this );
	job->fetchAllParts();
	connect( job, SIGNAL( result( KJob* ) ), this, SLOT( itemListFetched( KJob* ) ) );
	//QFont font( "Helvetica", 16, QFont::Bold );
	//addText( "It could be the place for your advertisment!", font );
	
	/*QLinearGradient gradient( -100, -100, 100, 100 );
	gradient.setColorAt( 0, QColor( 200, 200, 200 ) );
	gradient.setColorAt( 1, QColor( 100, 100, 100 ) );
	setBackgroundBrush( QBrush( gradient ) );*/

	//setBackgroundBrush( QColor( 200, 200, 200 ) );
	//NoteWidget* note = new NoteWidget();
	//addItem( note );
	//note->setPos( 0, 100 );

	//addItem( new NoteWidget() );
	/*for (int i = 0; i < 100; i++) {
		for (int j = 0; j < 100; j++) {
			NoteWidget* note = new NoteWidget();
			addItem( note );
			note->setPos( i * 250, j * 100 );
		}
	}*/
}

BasketContent::~BasketContent() {
}

void BasketContent::addItemDone( KJob* job ) {
	if ( job->error() ) return;
}

void BasketContent::mousePressEvent( QGraphicsSceneMouseEvent* mouseEvent ) {
	QGraphicsScene::mousePressEvent( mouseEvent );
	if ( mouseEvent->button() == Qt::RightButton && itemAt( mouseEvent->scenePos() ) == 0 ) {
		kDebug() << mouseEvent->scenePos() << endl;
		/*Item* item = new Item( "basket/note" );
		item->setPayload<NotePtr>( NotePtr( new Note( mouseEvent->scenePos(), "<default text>" ) ) );
		ItemAppendJob* job = new ItemAppendJob( *item, Collection( mBasketId ) );
		connect( job, SIGNAL( result( KJob* ) ), this, SLOT( addItemDone( KJob* ) ) );*/
	}
}

void BasketContent::itemAdded( const Akonadi::Item& item ) {
	//TODO remove it, because it's not necessary if everything else works ok
	kDebug() << "item added: " << item.reference().id() << endl;
	QHash<int, NoteWidget*>::const_iterator i = mNoteIdToNoteWidget.find( item.reference().id() );
	if ( i != mNoteIdToNoteWidget.constEnd() ) {
		kDebug() << "2 identical notes in the same basket! ID: " << item.reference().id() << endl;
		return;
	}
	/*if ( !item.hasPayload() ) {
		ItemFetchJob* job = new ItemFetchJob( item.reference(), this );
		job->fetchAllParts();
		connect( job, SIGNAL( result( KJob* ) ), this, SLOT( itemListFetched( KJob* ) ) );
		kDebug() << "BUG: " << item.reference().id() << endl;
		return;
	}*/

	int id = item.reference().id();
	//NotePtr note = item.payload<NotePtr>();
	NoteWidget* noteWidget = new NoteWidget( item );
	//noteWidget->setPos( note->pos() );
	//noteWidget->setPlainText( note->text() );
	addItem( noteWidget );
	mNoteIdToNoteWidget[ id ] = noteWidget;
}

void BasketContent::itemListFetched( KJob* job ) {
	if ( job->error() ) return;
	Item::List items = static_cast<ItemFetchJob*>( job )->items();
	foreach( const Item item, items )
		itemAdded( item );
}


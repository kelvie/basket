#include <QDockWidget>
#include <QGraphicsView>
#include <QTreeView>
#include <QModelIndex>
#include <QVBoxLayout>

#include <KAction>
#include <KStandardAction>
#include <KLocale>
#include <KApplication>
#include <KActionCollection>
#include <KDebug>
#include <KLineEdit>
#include <KStandardShortcut>
#include <KMenu>
#include <KMenuBar>

#include "basketmainwindow.h"
#include "baskettreeview.h"
#include "basketviewcontainer.h"
#include "newbasketdialog.h"

BasketMainWindow::BasketMainWindow( QWidget *parent ) : KXmlGuiWindow( parent ) {
	init();
}

BasketMainWindow::~BasketMainWindow() {
}

void BasketMainWindow::init() {
	BasketViewContainer *view = new BasketViewContainer();
	setCentralWidget( view );

	setupActions();
	setupDockWidgets();

	setupGUI();
}

void BasketMainWindow::createNewBasket() {
	NewBasketDialog dlg;
	dlg.exec();
	if ( dlg.isOk() ) {
		kDebug() << "create new basket : " << dlg.name() << endl;
	} else {
		kDebug() << "cancelled" << endl;
	}
}

void BasketMainWindow::setupActions() {
	KAction *newBasket = actionCollection()->addAction( "basket_new" );
	newBasket->setIcon( KIcon( "document-new" ) );
	newBasket->setText( i18n( "New Basket" ) );
	newBasket->setShortcut( Qt::CTRL | Qt::Key_N );
	connect( newBasket, SIGNAL( triggered( bool ) ), this, SLOT( createNewBasket() ) );

	KAction *quitBasket = actionCollection()->addAction( "basket_quit" );
	quitBasket->setIcon( KIcon( "application-exit" ) );
	quitBasket->setText( i18n( "Quit" ) );
	quitBasket->setShortcut( Qt::CTRL | Qt::Key_Q );
	connect( quitBasket, SIGNAL( triggered( bool ) ), kapp, SLOT( quit() ) );
}

void BasketMainWindow::setupDockWidgets() {
	QDockWidget *treeViewDock = new QDockWidget( i18n( "Tree view" ) );
	treeViewDock->setObjectName( "treeViewDock" );
	treeViewDock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

	BasketTreeView *treeView = new BasketTreeView();

	treeViewDock->setWidget( treeView );

	addDockWidget( Qt::LeftDockWidgetArea, treeViewDock );

	connect( treeView, SIGNAL(clicked( const QModelIndex& )), this, SLOT(update0(const QModelIndex&)) );

	treeViewDock->toggleViewAction()->setText( i18n( "Tree view" ) );
	treeViewDock->toggleViewAction()->setShortcut( Qt::Key_F9 );
	actionCollection()->addAction( "tree_view", treeViewDock->toggleViewAction() );
}

void BasketMainWindow::update0( const QModelIndex& index ) {
	kDebug() << "clicked: " << index.row() << " : " << index.column() << endl;
}


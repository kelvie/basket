#include <QDockWidget>
#include <QGraphicsView>
#include <QTreeView>
#include <QModelIndex>
#include <QVBoxLayout>
#include <QAction>

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
#include <KFontAction>
#include <KFontSizeAction>
#include <KToggleAction>

#include "basketmainwindow.h"
#include "baskettreewidget.h"
#include "basketviewcontainer.h"
#include "newbasketdialog.h"
#include "basketsettingsdialog.h"

BasketMainWindow::BasketMainWindow( QWidget *parent ) : KXmlGuiWindow( parent ) {
	init();
}

BasketMainWindow::~BasketMainWindow() {
}

void BasketMainWindow::init() {
	mViewContainer = new BasketViewContainer( this );
	setCentralWidget( mViewContainer );

	setupActions();
	setupDockWidgets();

	setupGUI();
}

void BasketMainWindow::createNewBasket() {
	NewBasketDialog dlg;
	dlg.exec();
}

void BasketMainWindow::setupActions() {
	KStandardAction::openNew( this, SLOT( createNewBasket() ), actionCollection() );
	KStandardAction::quit( kapp, SLOT( quit() ), actionCollection() );
	KStandardAction::undo( this, SLOT( undo() ), actionCollection() );
	KStandardAction::redo( this, SLOT( redo() ), actionCollection() );
	KStandardAction::paste( this, SLOT( paste() ), actionCollection() );
	//KStandardAction::cut( this, SLOT(), actionCollection() );
	//KStandardAction::copy( this, SLOT(), actionCollection() );
	//KStandardAction::fullScreen( this, SLOT(), actionCollection() );
	
	actionCollection()->addAction( "font_select", new KFontAction( this ) );
	actionCollection()->addAction( "fontsize_select", new KFontSizeAction( this ) );

	KToggleAction* boldAction = new KToggleAction( this );
	boldAction->setIcon( KIcon( "format-text-bold" ) );
	actionCollection()->addAction( "format_text_bold", boldAction );

	KToggleAction* italicAction = new KToggleAction( this );
	italicAction->setIcon( KIcon( "format-text-italic" ) );
	actionCollection()->addAction( "format_text_italic", italicAction );

	KToggleAction* underlineAction = new KToggleAction( this );
	underlineAction->setIcon( KIcon( "format-text-underline" ) );
	actionCollection()->addAction( "format_text_underline", underlineAction );

	KToggleAction* leftAction = new KToggleAction( this );
	leftAction->setIcon( KIcon( "format-justify-left" ) );
	actionCollection()->addAction( "format_justify_left", leftAction );

	KToggleAction* rightAction = new KToggleAction( this );
	rightAction->setIcon( KIcon( "format-justify-right" ) );
	actionCollection()->addAction( "format_justify_right", rightAction );

	KToggleAction* centerAction = new KToggleAction( this );
	centerAction->setIcon( KIcon( "format-justify-center" ) );
	actionCollection()->addAction( "format_justify_center", centerAction );

	KToggleAction* fillAction = new KToggleAction( this );
	fillAction->setIcon( KIcon( "format-justify-fill" ) );
	actionCollection()->addAction( "format_justify_fill", fillAction );

	QActionGroup* justifyGroup = new QActionGroup( this );
	justifyGroup->addAction( leftAction );
	justifyGroup->addAction( rightAction );
	justifyGroup->addAction( centerAction );
	justifyGroup->addAction( fillAction );

	KAction* basketSettings = new KAction( this );
	basketSettings->setIcon( KIcon( "configure" ) );
	basketSettings->setText( i18n( "Configure Basket" ) );
	actionCollection()->addAction( "settings_prefs", basketSettings );
	connect( basketSettings, SIGNAL( triggered() ) , this, SLOT( showSettingsDialog() ) );
}

void BasketMainWindow::setupDockWidgets() {
	QDockWidget *treeViewDock = new QDockWidget( i18n( "Tree view" ) );
	treeViewDock->setObjectName( "treeViewDock" );
	treeViewDock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

	mTreeWidget = new BasketTreeWidget( this, treeViewDock );

	treeViewDock->setWidget( mTreeWidget );

	addDockWidget( Qt::LeftDockWidgetArea, treeViewDock );

	//FIXME connect( m_treeView, SIGNAL(clicked( const QModelIndex& )), this, SLOT(update0(const QModelIndex&)) );

	treeViewDock->toggleViewAction()->setText( i18n( "Tree view" ) );
	treeViewDock->toggleViewAction()->setShortcut( Qt::Key_F9 );
	actionCollection()->addAction( "tree_view", treeViewDock->toggleViewAction() );
}

void BasketMainWindow::setCurrentBasket( int newBasketId ) {
	mViewContainer->setCurrentBasket( newBasketId );
}

void BasketMainWindow::undo() {
	kDebug() << "undo clicked" << endl;
}

void BasketMainWindow::redo() {
	kDebug() << "redo clicked" << endl;
}

void BasketMainWindow::paste() {
	kDebug() << "paste clicked" << endl;
}

void BasketMainWindow::showSettingsDialog() {
	BasketSettingsDialog dlg( this );
	dlg.exec();
}


/***************************************************************************
 *   Copyright (C) 2003 by Sébastien Laoût                                 *
 *   slaout@linux62.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/// NEW:

#include <QMenu>
#include <QStackedWidget>
#include <qregexp.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qwhatsthis.h>
#include <kmenu.h>
#include <qsignalmapper.h>
#include <qdir.h>
#include <kicontheme.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstringhandler.h>
#include <kactioncollection.h>
#include <kstandardshortcut.h>
#include <kactionmenu.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <QProgressBar>
#include <kstandarddirs.h>
#include <kaboutdata.h>
#include <kwindowsystem.h>
#include <kaction.h>
#include <kpassivepopup.h>
#include <kxmlguifactory.h>
#include <kcmdlineargs.h>
#include <kglobalaccel.h>
#include <kapplication.h>
#include <kshortcutsdialog.h>
#include <QDBusConnection>
#include <kdebug.h>
#include <iostream>
#include <kshortcutsdialog.h>
#include "bnpview.h"
#include "basket.h"
#include "tools.h"
#include "settings.h"
#include "debugwindow.h"
#include "xmlwork.h"
#include "basketfactory.h"
#include "softwareimporters.h"
#include "colorpicker.h"
#include "regiongrabber.h"
#include "baskettree.h"
#include "basketproperties.h"
#include "password.h"
#include "newbasketdialog.h"
#include "notedrag.h"
#include "formatimporter.h"
#include "basketstatusbar.h"
#include "backgroundmanager.h"
#include "noteedit.h" // To launch InlineEditors::initToolBars()
#include "archive.h"
#include "htmlexporter.h"
#include "crashhandler.h"
#include "likeback.h"
#include "backup.h"
#include "basketfactory.h"
#include <QKeyEvent>
#include <QTreeWidgetItemIterator>
#include <kprogressdialog.h>
#include <ktoggleaction.h>

/** class BNPView: */

const int BNPView::c_delayTooltipTime = 275;

BNPView::BNPView ( QWidget *parent, const char *name, KXMLGUIClient *aGUIClient,
                   KActionCollection *actionCollection, BasketStatusBar *bar )
		: QSplitter ( Qt::Horizontal, parent ), m_actLockBasket ( 0 ), m_actPassBasket ( 0 ),
		m_loading ( true ), m_newBasketPopup ( false ), m_firstShow ( true ),
		m_regionGrabber ( 0 ), m_passiveDroppedSelection ( 0 ), m_passivePopup ( 0 ), m_actionCollection ( actionCollection ),
		m_guiClient ( aGUIClient ), m_statusbar ( bar ), m_tryHideTimer ( 0 ), m_hideTimer ( 0 )
{
//TODO	QDBusConnection::sessionBus().registerObject("org.kde.basket", this,
//                            QDBusConnection::ExportScriptableSlots);
	/* Settings */
	Settings::loadConfig();

	Global::bnpView = this;

	// Needed when loading the baskets:
// TODO add to KAction with  setGlobalShortcut()	Global::globalAccel       = new KGlobalAccel(this); // FIXME: might be null (KPart case)!
// TODO AS UPPER	Global::backgroundManager = new BackgroundManager();

	setupGlobalShortcuts();
	initialize();
	QTimer::singleShot ( 0, this, SLOT ( lateInit() ) );
}

BNPView::~BNPView()
{
	int treeWidth = Global::bnpView->sizes() [Settings::treeOnLeft() ? 0 : 1];

	Settings::setBasketTreeWidth ( treeWidth );

	if ( currentBasket() && currentBasket()->isDuringEdit() )
		currentBasket()->closeEditor();

	Settings::saveConfig();

	Global::bnpView = 0;

	delete Global::systemTray;
	Global::systemTray = 0;
	delete m_colorPicker;
	delete m_statusbar;

	NoteDrag::createAndEmptyCuttingTmpFolder(); // Clean the temporary folder we used
}

void BNPView::lateInit()
{
	/*
		InlineEditors* instance = InlineEditors::instance();

		if(instance)
		{
			KToolBar* toolbar = instance->richTextToolBar();

			if(toolbar)
				toolbar->hide();
		}
	*/
	if ( !isPart() )
	{
		if ( Settings::useSystray() && KCmdLineArgs::parsedArgs() && KCmdLineArgs::parsedArgs()->isSet ( "start-hidden" ) )
			if ( Global::mainWindow() ) Global::mainWindow()->hide();
			else if ( Settings::useSystray() && kapp->isSessionRestored() )
				if ( Global::mainWindow() ) Global::mainWindow()->setShown ( !Settings::startDocked() );
				else
					showMainWindow();
	}

	// If the main window is hidden when session is saved, Container::queryClose()
	//  isn't called and the last value would be kept
	Settings::setStartDocked ( true );
	Settings::saveConfig();

	/* System tray icon */
	Global::systemTray = new SystemTray ( Global::mainWindow() );
	connect ( Global::systemTray, SIGNAL ( showPart() ), this, SIGNAL ( showPart() ) );
	if ( Settings::useSystray() )
		Global::systemTray->show();

	// Load baskets
	kDebug() << "Baskets are loaded from " + Global::basketsFolder();

	NoteDrag::createAndEmptyCuttingTmpFolder(); // If last exec hasn't done it: clean the temporary folder we will use
	Tag::loadTags(); // Tags should be ready before loading baskets, but tags need the mainContainer to be ready to create KActions!
	load();

	// If no basket has been found, try to import from an older version,
	if ( !firstListViewItem() )
	{
		kDebug() << "Importing" << endl;
		QDir dir;
		dir.mkdir ( Global::basketsFolder() );
		if ( FormatImporter::shouldImportBaskets() )
		{
			FormatImporter::importBaskets();
			load();
		}
		if ( !firstListViewItem() )
		{
			kDebug() << "Creating the first Basket" << endl;
			//Create first basket:
			BasketFactory::newBasket ( /*icon=*/"", /*name=*/i18n ( "General" ), /*backgroundImage=*/"", /*backgroundColor=*/QColor(), /*textColor=*/QColor(), /*templateName=*/"1column", /*createIn=*/0 );
		}
	}

	// Load the Welcome Baskets if it is the First Time:
	if ( !Settings::welcomeBasketsAdded() )
	{
		addWelcomeBaskets();
		Settings::setWelcomeBasketsAdded ( true );
		Settings::saveConfig();
	}

	m_tryHideTimer = new QTimer ( this );
	m_hideTimer    = new QTimer ( this );
	connect ( m_tryHideTimer, SIGNAL ( timeout() ), this, SLOT ( timeoutTryHide() ) );
	connect ( m_hideTimer,    SIGNAL ( timeout() ), this, SLOT ( timeoutHide() ) );

	// Preload every baskets for instant filtering:
	/*StopWatch::start(100);
		QTreeWidgetIterator it(m_tree);
		while (it.current()) {
			BasketListViewItem *item = ((BasketListViewItem*)it.current());
			item->basket()->load();
			kapp->processEvents();
			++it;
		}
	StopWatch::check(100);*/
}

void BNPView::addWelcomeBaskets()
{
	// Possible paths where to find the welcome basket archive, trying the translated one, and falling back to the English one:
	QStringList possiblePaths;
	if ( QString ( KGlobal::locale()->encoding() ) == QString ( "UTF-8" ) )   // Welcome baskets are encoded in UTF-8. If the system is not, then use the English version:
	{
		possiblePaths.append ( KGlobal::dirs()->findResource ( "data", "basket/welcome/Welcome_" + KGlobal::locale()->language() + ".baskets" ) );
		possiblePaths.append ( KGlobal::dirs()->findResource ( "data", "basket/welcome/Welcome_" + ( KGlobal::locale()->language().split ( "_" ) ) [0] + ".baskets" ) );
	}
	possiblePaths.append ( KGlobal::dirs()->findResource ( "data", "basket/welcome/Welcome_en_US.baskets" ) );

	// Take the first EXISTING basket archive found:
	QDir dir;
	QString path;
	for ( QStringList::Iterator it = possiblePaths.begin(); it != possiblePaths.end(); ++it )
	{
		if ( dir.exists ( *it ) )
		{
			path = *it;
			break;
		}
	}

	// Extract:
	if ( !path.isEmpty() )
		Archive::open ( path );
}

void BNPView::onFirstShow()
{
	// Don't enable LikeBack until bnpview is shown. This way it works better with kontact.
	/* LikeBack */
	/*	Global::likeBack = new LikeBack(LikeBack::AllButtons, / *showBarByDefault=* /true, Global::config(), Global::about());
		Global::likeBack->setServer("basket.linux62.org", "/likeback/send.php");
		Global:likeBack->setAcceptedLanguages(QStringList::split(";", "en;fr"), i18n("Only english and french languages are accepted."));
		if (isPart())
			Global::likeBack->disableBar(); // See BNPView::shown() and BNPView::hide().
	*/

	if ( isPart() )
		Global::likeBack->disableBar(); // See BNPView::shown() and BNPView::hide().

	/*
		LikeBack::init(Global::config(), Global::about(), LikeBack::AllButtons);
		LikeBack::setServer("basket.linux62.org", "/likeback/send.php");
	//	LikeBack::setServer("localhost", "/~seb/basket/likeback/send.php");
		LikeBack::setCustomLanguageMessage(i18n("Only english and french languages are accepted."));
	//	LikeBack::setWindowNamesListing(LikeBack:: / *NoListing* / / *WarnUnnamedWindows* / AllWindows);
	*/

	// In late init, because we need kapp->mainWidget() to be set!
	if ( !isPart() )
		connectTagsMenu();

	m_statusbar->setupStatusBar();

	int treeWidth = Settings::basketTreeWidth();
	if ( treeWidth < 0 )
		treeWidth = m_tree->fontMetrics().maxWidth() * 11;
	QList<int> splitterSizes;
	splitterSizes.append ( treeWidth );
	setSizes ( splitterSizes );
}

void BNPView::setupGlobalShortcuts()
{
	/* Global shortcuts */
	KGlobalAccel *globalAccel = Global::globalAccel; // Better for the following lines

	// Ctrl+Shift+W only works when started standalone:
//FIXME	QWidget *basketMainWindow = (QWidget*) (Global::bnpView->parent()->inherits("MainWindow") ? Global::bnpView->parent() : 0);

	/* TODO	if (basketMainWindow) {
			globalAccel->insert( "global_show_hide_main_window", i18n("Show/hide main window"),
								i18n("Allows you to show main Window if it is hidden, and to hide it if it is shown."),
								Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_W, Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_W,
								basketMainWindow, SLOT(changeActive()),           true, true );
		}
		globalAccel->insert( "global_paste", i18n("Paste clipboard contents in current basket"),
							 i18n("Allows you to paste clipboard contents in the current basket without having to open the main window."),
							 Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_V, Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_V,
							 Global::bnpView, SLOT(globalPasteInCurrentBasket()), true, true );
		globalAccel->insert( "global_show_current_basket", i18n("Show current basket name"),
							 i18n("Allows you to know basket is current without opening the main window."),
							 "", "",
							 Global::bnpView, SLOT(showPassiveContentForced()), true, true );
		globalAccel->insert( "global_paste_selection", i18n("Paste selection in current basket"),
							 i18n("Allows you to paste clipboard selection in the current basket without having to open the main window."),
							 Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_S, Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_S,
							 Global::bnpView, SLOT(pasteSelInCurrentBasket()),  true, true );
		globalAccel->insert( "global_new_basket", i18n("Create a new basket"),
							 i18n("Allows you to create a new basket without having to open the main window (you then can use the other global shortcuts to add a note, paste clipboard or paste selection in this new basket)."),
							 "", "",
							 Global::bnpView, SLOT(askNewBasket()),       true, true );
		globalAccel->insert( "global_previous_basket", i18n("Go to previous basket"),
							 i18n("Allows you to change current basket to the previous one without having to open the main window."),
							 "", "",
							 Global::bnpView,    SLOT(goToPreviousBasket()), true, true );
		globalAccel->insert( "global_next_basket", i18n("Go to next basket"),
							 i18n("Allows you to change current basket to the next one without having to open the main window."),
							 "", "",
							 Global::bnpView,    SLOT(goToNextBasket()),     true, true );
	//	globalAccel->insert( "global_note_add_text", i18n("Insert plain text note"),
	//						 i18n("Add a plain text note to the current basket without having to open the main window."),
	//						 "", "", //Qt::CTRL+Qt::ALT+Qt::Key_T, Qt::CTRL+Qt::ALT+Qt::Key_T,
	//						 Global::bnpView, SLOT(addNoteText()),        true, true );
		globalAccel->insert( "global_note_add_html", i18n("Insert text note"),
							 i18n("Add a text note to the current basket without having to open the main window."),
							 Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_T, Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_T, //"", "",
							 Global::bnpView, SLOT(addNoteHtml()),        true, true );
		globalAccel->insert( "global_note_add_image", i18n("Insert image note"),
							 i18n("Add an image note to the current basket without having to open the main window."),
							 "", "",
							 Global::bnpView, SLOT(addNoteImage()),       true, true );
		globalAccel->insert( "global_note_add_link", i18n("Insert link note"),
							 i18n("Add a link note to the current basket without having to open the main window."),
							 "", "",
							 Global::bnpView, SLOT(addNoteLink()),        true, true );
		globalAccel->insert( "global_note_add_color", i18n("Insert color note"),
							 i18n("Add a color note to the current basket without having to open the main window."),
							 "", "",
							 Global::bnpView, SLOT(addNoteColor()),       true, true );
		globalAccel->insert( "global_note_pick_color", i18n("Pick color from screen"),
							 i18n("Add a color note picked from one pixel on screen to the current basket without "
									 "having to open the main window."),
							 "", "",
							 Global::bnpView, SLOT(slotColorFromScreenGlobal()), true, true );
		globalAccel->insert( "global_note_grab_screenshot", i18n("Grab screen zone"),
							 i18n("Grab a screen zone as an image in the current basket without "
									 "having to open the main window."),
							 "", "",
							 Global::bnpView, SLOT(grabScreenshotGlobal()), true, true );
		globalAccel->readSettings();
		globalAccel->updateConnections();*/
}

void BNPView::initialize()
{
	// Configure the List View Columns:
	m_tree  = new BasketTree ( this );
	QStringList headers;
	headers << i18n ( "Baskets" );
	m_tree->setHeaderLabels ( headers );

	//m_tree->setColumnWidth(0, 10);

	/*QList<QTreeWidgetItem *> items;
	for (int i = 0; i < 10; ++i)
		      items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("item: %1").arg(i))));
	m_tree->insertTopLevelItems(0, items);*/


//FIXME 1.5	m_tree->setColumnWidthMode(0, QTreeWidget::maximum);
//FIXME	m_tree->setFullWidth(true);
//FIXME	m_tree->setSorting(false);
	m_tree->setRootIsDecorated ( true );
//FIXME	1.5	m_tree->setTreeStepSize(16);
//FIXME	1.5	m_tree->setLineWidth(1);
//FIXME	1.5	m_tree->setMidLineWidth(0);
//FIXME	1.5 m_tree->setFocusPolicy(QWidget::NoFocus);

	/// Configure the List View Drag and Drop:
//FIXME	1.5	m_tree->setDragEnabled(true);
	m_tree->setAcceptDrops(true);
//FIXME	1.5	m_tree->setItemsMovable(true);
//FIXME	1.5	m_tree->setDragAutoScroll(true);
//FIXME	1.5	m_tree->setDropVisualizer(true);
//FIXME	1.5	m_tree->setDropHighlighter(true);

	/// Configure the Splitter:
	m_stack = new QStackedWidget ( this );

	//new QTextEdit( m_stack );
	//new QTreeView( m_stack );

	//setOpaqueResize(true);

//FIXME 1.5	setResizeMode(m_tree,  QSplitter::KeepSize);
//FIXME 1.5	setResizeMode(m_stack, QSplitter::Stretch);

//FIXME 1.5	setCollapsible(m_tree,  true);
//FIXME 1.5	setCollapsible(m_stack, false);
	setCollapsible ( 0, true );
	setCollapsible ( 1, false );

	/// Configure the BasketTree Signals:
	connect( m_tree, SIGNAL(returnPressed(QTreeWidget*)),    this, SLOT(slotPressed(QTreeWidget*)) );
	connect( m_tree, SIGNAL(selectionChanged(QTreeWidget*)), this, SLOT(slotPressed(QTreeWidget*)) );
	connect( m_tree, SIGNAL(pressed(QTreeWidget*)),          this, SLOT(slotPressed(QTreeWidget*)) );
	connect( m_tree, SIGNAL(expanded(QTreeWidget*)),         this, SLOT(needSave(QTreeWidget*))    );
	connect( m_tree, SIGNAL(collapsed(QTreeWidget*)),        this, SLOT(needSave(QTreeWidget*))    );
	connect( m_tree, SIGNAL(contextMenu(QTreeWidget*, QTreeWidget*, const QPoint&)),      this, SLOT(slotContextMenu(QTreeWidget*, QTreeWidgetItem*, const QPoint&))      );
	connect( m_tree, SIGNAL(mouseButtonPressed(int, QTreeWidget*, const QPoint&, int)), this, SLOT(slotMouseButtonPressed(int, QTreeWidgetItem*, const QPoint&, int)) );
	connect( m_tree, SIGNAL(doubleClicked(QTreeWidgetItem*, const QPoint&, int)), this, SLOT(slotShowProperties(QTreeWidgetItem*, const QPoint&, int)) );

	connect( m_tree, SIGNAL(expanded(QTreeWidgetItem*)),  this, SIGNAL(basketChanged()) );
	connect( m_tree, SIGNAL(collapsed(QTreeWidgetItem*)), this, SIGNAL(basketChanged()) );
	connect( this,   SIGNAL(basketNumberChanged(int)),  this, SIGNAL(basketChanged()) );

	connect( this, SIGNAL(basketNumberChanged(int)), this, SLOT(slotBasketNumberChanged(int)) );
	connect( this, SIGNAL(basketChanged()),          this, SLOT(slotBasketChanged())          );

	/* LikeBack */
	Global::likeBack = new LikeBack ( LikeBack::AllButtons, /*showBarByDefault=*/false, Global::config(), Global::about() );
	Global::likeBack->setServer ( "basket.linux62.org", "/likeback/send.php" );

// There are too much comments, and people reading comments are more and more international, so we accept only English:
//	Global::likeBack->setAcceptedLanguages(QStringList::split(";", "en;fr"), i18n("Please write in English or French."));

//	if (isPart())
//		Global::likeBack->disableBar(); // See BNPView::shown() and BNPView::hide().

	Global::likeBack->sendACommentAction ( actionCollection() ); // Just create it!
	setupActions();

	/// What's This Help for the tree:
	/*FIXME 1.5	QWhatsThis::add(m_tree, i18n(
				"<h2>Basket Tree</h2>"
						"Here is the list of your baskets. "
						"You can organize your data by putting them in different baskets. "
						"You can group baskets by subject by creating new baskets inside others. "
						"You can browse between them by clicking a basket to open it, or reorganize them using drag and drop."));
	*/
	setTreePlacement ( Settings::treeOnLeft() );
}

void BNPView::setupActions()
{
	m_actSaveAsArchive = new KAction ( this );
	m_actSaveAsArchive->setText ( i18n ( "&Basket Archive..." ) );
	actionCollection()->addAction ( "basket_export_basket_archive", m_actSaveAsArchive );
	connect ( m_actSaveAsArchive, SIGNAL ( triggered ( bool ) ), this, SLOT ( saveAsArchive() ) );

	m_actOpenArchive = new KAction ( this );
	m_actOpenArchive->setText ( i18n ( "&Basket Archive..." ) );
	actionCollection()->addAction ( "basket_import_basket_archive", m_actOpenArchive );
	connect ( m_actOpenArchive, SIGNAL ( triggered ( bool ) ), this, SLOT ( openArchive() ) );

	m_actHideWindow = new KAction ( this );
	m_actHideWindow->setText ( i18n ( "&Hide Window" ) );
	actionCollection()->addAction ( "window_hide", m_actHideWindow );
	connect ( m_actHideWindow, SIGNAL ( triggered ( bool ) ), this, SLOT ( hideOnEscape() ) );
	m_actHideWindow->setEnabled ( Settings::useSystray() ); // Init here !

	m_actExportToHtml = new KAction ( this );
	m_actExportToHtml->setText ( i18n ( "&HTML Web Page..." ) );
	actionCollection()->addAction ( "basket_export_html", m_actExportToHtml );
	connect ( m_actExportToHtml, SIGNAL ( triggered ( bool ) ), this, SLOT ( exportToHTML ) );

	KAction *temp;

	temp = new KAction ( this );
	temp->setText ( i18n ( "K&Notes" ) );
	actionCollection()->addAction ( "basket_import_knotes", temp );
	connect ( temp, SIGNAL ( triggered ( bool ) ), this, SLOT ( importKNotes() ) );

	temp = new KAction ( this );
	temp->setText ( i18n ( "K&Jots" ) );
	actionCollection()->addAction ( "basket_import_kjots", temp );
	connect ( temp, SIGNAL ( triggered ( bool ) ), this, SLOT ( importKJots() ) );

	temp = new KAction ( this );
	temp->setText ( i18n ( "&KnowIt..." ) );
	actionCollection()->addAction ( "basket_import_knowit", temp );
	connect ( temp, SIGNAL ( triggered ( bool ) ), this, SLOT ( importKnowIt() ) );

	/*FIXME	new KAction( i18n("&Backup && Restore..."), "", 0,
		             this, SLOT(backupRestore()), actionCollection(), "basket_backup_restore" );

		new KAction( i18n("Tux&Cards..."), "tuxcards", 0,
		             this, SLOT(importTuxCards()),    actionCollection(), "basket_import_tuxcards" );
		new KAction( i18n("&Sticky Notes"), "gnome", 0,
		             this, SLOT(importStickyNotes()), actionCollection(), "basket_import_sticky_notes" );
		new KAction( i18n("&Tomboy"), "tintin", 0,
		             this, SLOT(importTomboy()),      actionCollection(), "basket_import_tomboy" );
		new KAction( i18n("Text &File..."), "txt", 0,
		             this, SLOT(importTextFile()),    actionCollection(), "basket_import_text_file" );
	*/
	/** Note : ****************************************************************/

	/*FIXME 1.5	m_actDelNote  = new KAction( i18n("D&elete"), "edit-delete", "Delete",
									 this, SLOT(delNote()), actionCollection(), "edit_delete" );
		m_actCutNote  = KStandardAction::cut(   this, SLOT(cutNote()),               actionCollection() );
		m_actCopyNote = KStandardAction::copy(  this, SLOT(copyNote()),              actionCollection() );

		m_actSelectAll = KStandardAction::selectAll( this, SLOT( slotSelectAll() ), actionCollection() );
		m_actSelectAll->setStatusText( i18n( "Selects all notes" ) );
		m_actUnselectAll = new KAction( i18n( "U&nselect All" ), "", this, SLOT( slotUnselectAll() ),
										actionCollection(), "edit_unselect_all" );
		m_actUnselectAll->setStatusText( i18n( "Unselects all selected notes" ) );
		m_actInvertSelection = new KAction( i18n( "&Invert Selection" ), CTRL+Key_Asterisk,
											this, SLOT( slotInvertSelection() ),
											actionCollection(), "edit_invert_selection" );
		m_actInvertSelection->setStatusText( i18n( "Inverts the current selection of notes" ) );

		m_actEditNote         = new KAction( i18n("Verb; not Menu", "&Edit..."), "edit",   "Return",
											 this, SLOT(editNote()), actionCollection(), "note_edit" );

		m_actOpenNote         = KStandardAction::open( this, SLOT(openNote()), actionCollection(), "note_open" );
		m_actOpenNote->setIcon("window-new");
		m_actOpenNote->setText(i18n("&Open"));
		m_actOpenNote->setShortcut("F9");

		m_actOpenNoteWith     = new KAction( i18n("Open &With..."), "", "Shift+F9",
											 this, SLOT(openNoteWith()), actionCollection(), "note_open_with" );
		m_actSaveNoteAs       = KStandardAction::saveAs( this, SLOT(saveNoteAs()), actionCollection(), "note_save_to_file" );
		m_actSaveNoteAs->setIcon("");
		m_actSaveNoteAs->setText(i18n("&Save to File..."));
		m_actSaveNoteAs->setShortcut("F10");

		m_actGroup        = new KAction( i18n("&Group"),          "attach",     "Ctrl+G",
										 this, SLOT(noteGroup()),    actionCollection(), "note_group" );
		m_actUngroup      = new KAction( i18n("U&ngroup"),        "",           "Ctrl+Shift+G",
										 this, SLOT(noteUngroup()),  actionCollection(), "note_ungroup" );

		m_actMoveOnTop    = new KAction( i18n("Move on &Top"),    "arrow-up-double",   "Ctrl+Shift+Home",
										 this, SLOT(moveOnTop()),    actionCollection(), "note_move_top" );
		m_actMoveNoteUp   = new KAction( i18n("Move &Up"),        "arrow-up",   "Ctrl+Shift+Up",
										 this, SLOT(moveNoteUp()),   actionCollection(), "note_move_up" );
		m_actMoveNoteDown = new KAction( i18n("Move &Down"),      "arrow-down", "Ctrl+Shift+Down",
										 this, SLOT(moveNoteDown()), actionCollection(), "note_move_down" );
		m_actMoveOnBottom = new KAction( i18n("Move on &Bottom"), "arrow-down-double", "Ctrl+Shift+End",
										 this, SLOT(moveOnBottom()), actionCollection(), "note_move_bottom" );
	#if KDE_IS_VERSION( 3, 1, 90 ) // KDE 3.2.x
		m_actPaste = KStandardAction::pasteText( this, SLOT(pasteInCurrentBasket()), actionCollection() );
	#else
		m_actPaste = KStandardAction::paste(     this, SLOT(pasteInCurrentBasket()), actionCollection() );
	#endif
	*/
	/** Insert : **************************************************************/

	/*FIXME 1.5	QSignalMapper *insertEmptyMapper  = new QSignalMapper(this);
		QSignalMapper *insertWizardMapper = new QSignalMapper(this);
		connect( insertEmptyMapper,  SIGNAL(mapped(int)), this, SLOT(insertEmpty(int))  );
		connect( insertWizardMapper, SIGNAL(mapped(int)), this, SLOT(insertWizard(int)) );

	//	m_actInsertText   = new KAction( i18n("Plai&n Text"), "text",     "Ctrl+T", actionCollection(), "insert_text"     );
		m_actInsertHtml   = new KAction( i18n("&Text"),       "html",     "Insert", actionCollection(), "insert_html"     );
		m_actInsertLink   = new KAction( i18n("&Link"),       "link",     "Ctrl+Y", actionCollection(), "insert_link"     );
		m_actInsertImage  = new KAction( i18n("&Image"),      "image",    "",       actionCollection(), "insert_image"    );
		m_actInsertColor  = new KAction( i18n("&Color"),      "colorset", "",       actionCollection(), "insert_color"    );
		m_actInsertLauncher=new KAction( i18n("L&auncher"),   "launch",   "",       actionCollection(), "insert_launcher" );

		m_actImportKMenu  = new KAction( i18n("Import Launcher from &KDE Menu..."), "kmenu",      "", actionCollection(), "insert_kmenu"     );
		m_actImportIcon   = new KAction( i18n("Im&port Icon..."),                   "icons",      "", actionCollection(), "insert_icon"      );
		m_actLoadFile     = new KAction( i18n("Load From &File..."),                "file-import", "", actionCollection(), "insert_from_file" );

	//	connect( m_actInsertText,     SIGNAL(activated()), insertEmptyMapper, SLOT(map()) );
		connect( m_actInsertHtml,     SIGNAL(activated()), insertEmptyMapper, SLOT(map()) );
		connect( m_actInsertImage,    SIGNAL(activated()), insertEmptyMapper, SLOT(map()) );
		connect( m_actInsertLink,     SIGNAL(activated()), insertEmptyMapper, SLOT(map()) );
		connect( m_actInsertColor,    SIGNAL(activated()), insertEmptyMapper, SLOT(map()) );
		connect( m_actInsertLauncher, SIGNAL(activated()), insertEmptyMapper, SLOT(map()) );
	//	insertEmptyMapper->setMapping(m_actInsertText,     NoteType::Text    );
		insertEmptyMapper->setMapping(m_actInsertHtml,     NoteType::Html    );
		insertEmptyMapper->setMapping(m_actInsertImage,    NoteType::Image   );
		insertEmptyMapper->setMapping(m_actInsertLink,     NoteType::Link    );
		insertEmptyMapper->setMapping(m_actInsertColor,    NoteType::Color   );
		insertEmptyMapper->setMapping(m_actInsertLauncher, NoteType::Launcher);

		connect( m_actImportKMenu, SIGNAL(activated()), insertWizardMapper, SLOT(map()) );
		connect( m_actImportIcon,  SIGNAL(activated()), insertWizardMapper, SLOT(map()) );
		connect( m_actLoadFile,    SIGNAL(activated()), insertWizardMapper, SLOT(map()) );
		insertWizardMapper->setMapping(m_actImportKMenu,  1 );
		insertWizardMapper->setMapping(m_actImportIcon,   2 );
		insertWizardMapper->setMapping(m_actLoadFile,     3 );

		m_colorPicker = new DesktopColorPicker();
		m_actColorPicker = new KAction( i18n("C&olor from Screen"), "kcolorchooser", "",
										this, SLOT(slotColorFromScreen()), actionCollection(), "insert_screen_color" );
		connect( m_colorPicker, SIGNAL(pickedColor(const QColor&)), this, SLOT(colorPicked(const QColor&)) );
		connect( m_colorPicker, SIGNAL(canceledPick()),             this, SLOT(colorPickingCanceled())     );

		m_actGrabScreenshot = new KAction( i18n("Grab Screen &Zone"), "ksnapshot", "",
										   this, SLOT(grabScreenshot()), actionCollection(), "insert_screen_capture" );
		//connect( m_actGrabScreenshot, SIGNAL(regionGrabbed(const QPixmap&)), this, SLOT(screenshotGrabbed(const QPixmap&)) );
		//connect( m_colorPicker, SIGNAL(canceledPick()),             this, SLOT(colorPickingCanceled())     );

	//	m_insertActions.append( m_actInsertText     );
		m_insertActions.append( m_actInsertHtml     );
		m_insertActions.append( m_actInsertLink     );
		m_insertActions.append( m_actInsertImage    );
		m_insertActions.append( m_actInsertColor    );
		m_insertActions.append( m_actImportKMenu    );
		m_insertActions.append( m_actInsertLauncher );
		m_insertActions.append( m_actImportIcon     );
		m_insertActions.append( m_actLoadFile       );
		m_insertActions.append( m_actColorPicker    );
		m_insertActions.append( m_actGrabScreenshot );
	*/
	/** Basket : **************************************************************/

	/*FIXME 1.5
		// At this stage, main.cpp has not set kapp->mainWidget(), so Global::runInsideKontact()
		// returns true. We do it ourself:
		bool runInsideKontact = true;
		QWidget *parentWidget = (QWidget*) parent();
		while (parentWidget) {
			if (parentWidget->inherits("MainWindow"))
				runInsideKontact = false;
			parentWidget = (QWidget*) parentWidget->parent();
		} */

	actNewBasket = new KAction ( this );
	actNewBasket->setText ( i18n ( "&New Basket..." ) );
	actNewBasket->setShortcut ( KStandardShortcut::shortcut ( KStandardShortcut::New ) );
	actionCollection()->addAction ( "basket_new", actNewBasket );
	connect ( actNewBasket, SIGNAL ( triggered ( bool ) ), this, SLOT ( askNewBasket() ) );

	actNewSubBasket = new KAction ( this );
	actNewSubBasket->setText ( i18n ( "New &Sub-Basket..." ) );
	actNewSubBasket->setShortcut ( Qt::CTRL + Qt::SHIFT + Qt::Key_N );
	actionCollection()->addAction ( "basket_new", actNewSubBasket );
	connect ( actNewSubBasket, SIGNAL ( triggered ( bool ) ), this, SLOT ( askNewSubBasket() ) );

	actNewSiblingBasket = new KAction ( this );
	actNewSiblingBasket->setText ( i18n ( "New Si&bling Basket..." ) );
	actionCollection()->addAction ( "basket_new", actNewSiblingBasket );
	connect ( actNewSiblingBasket, SIGNAL ( triggered ( bool ) ), this, SLOT ( askNewSiblingBasket() ) );

	//KActionMenu *newBasketMenu = new KActionMenu(i18n("&New"), "document-new", actionCollection(), "basket_new_menu");
	KActionMenu *newBasketMenu = new KActionMenu ( this );
	newBasketMenu->setText ( i18n ( "&New" ) );
	newBasketMenu->addAction ( actNewBasket );
	newBasketMenu->addAction ( actNewSubBasket );
	newBasketMenu->addAction ( actNewSiblingBasket );
	actionCollection()->addAction ( "basket_new_menu", newBasketMenu );
	connect ( newBasketMenu, SIGNAL ( activated() ), this, SLOT ( askNewBasket() ) );

	/*	// Use the "basket" incon in Kontact so it is consistent with the Kontact "New..." icon

		m_actPropBasket = new KAction( i18n("&Properties..."), "misc", "F2",
									   this, SLOT(propBasket()), actionCollection(), "basket_properties" );
		m_actDelBasket  = new KAction( i18n("Remove Basket", "&Remove"), "", 0,
									   this, SLOT(delBasket()), actionCollection(), "basket_remove" );
	#ifdef HAVE_LIBGPGME
		m_actPassBasket = new KAction( i18n("Password protection", "Pass&word..."), "", 0,
									   this, SLOT(password()), actionCollection(), "basket_password" );
		m_actLockBasket = new KAction( i18n("Lock Basket", "&Lock"), "", "Ctrl+L",
									   this, SLOT(lockBasket()), actionCollection(), "basket_lock" );
	#endif
	*/
	/** Edit : ****************************************************************/

	/*FIXME 1.5
		//m_actUndo     = KStandardAction::undo(  this, SLOT(undo()),                 actionCollection() );
		//m_actUndo->setEnabled(false); // Not yet implemented !
		//m_actRedo     = KStandardAction::redo(  this, SLOT(redo()),                 actionCollection() );
		//m_actRedo->setEnabled(false); // Not yet implemented !

		m_actShowFilter  = new KToggleAction( i18n("&Filter"), "search-filter", KStandardShortcut::shortcut(KStandardShortcut::Find),
											  actionCollection(), "edit_filter" );
		connect( m_actShowFilter, SIGNAL(toggled(bool)), this, SLOT(showHideFilterBar(bool)) );

		m_actFilterAllBaskets = new KToggleAction( i18n("Filter all &Baskets"), "edit-find", "Ctrl+Shift+F",
												   actionCollection(), "edit_filter_all_baskets" );
		connect( m_actFilterAllBaskets, SIGNAL(toggled(bool)), this, SLOT(toggleFilterAllBaskets(bool)) );

		m_actResetFilter = new KAction( i18n( "&Reset Filter" ), "locationbar_erase", "Ctrl+R",
										this, SLOT( slotResetFilter() ), actionCollection(), "edit_filter_reset" );
	*/
	/** Go : ******************************************************************/

	/*FIXME 1.5v
		m_actPreviousBasket = new KAction( i18n( "&Previous Basket" ), "go-up",      "Alt+Up",
										   this, SLOT(goToPreviousBasket()), actionCollection(), "go_basket_previous" );
		m_actNextBasket     = new KAction( i18n( "&Next Basket" ),     "go-down",    "Alt+Down",
										   this, SLOT(goToNextBasket()),     actionCollection(), "go_basket_next"     );
		m_actFoldBasket     = new KAction( i18n( "&Fold Basket" ),     "go-previous",    "Alt+Left",
										   this, SLOT(foldBasket()),         actionCollection(), "go_basket_fold"     );
		m_actExpandBasket   = new KAction( i18n( "&Expand Basket" ),   "go-next", "Alt+Right",
										   this, SLOT(expandBasket()),       actionCollection(), "go_basket_expand"   );
		// FOR_BETA_PURPOSE:
	//	m_convertTexts = new KAction( i18n("Convert text notes to rich text notes"), "compfile", "",
	//								  this, SLOT(convertTexts()), actionCollection(), "beta_convert_texts" );

		InlineEditors::instance()->initToolBars(actionCollection());

		actConfigGlobalShortcuts = KStandardAction::keyBindings(this, SLOT(showGlobalShortcutsSettingsDialog()),
				actionCollection(), "options_configure_global_keybinding");
		actConfigGlobalShortcuts->setText(i18n("Configure &Global Shortcuts..."));
	*/
	/** Help : ****************************************************************/
	temp = new KAction ( this );
	temp->setText ( i18n ( "&Welcome Baskets" ) );
	actionCollection()->addAction ( "help_welcome_baskets", temp );
	connect ( temp, SIGNAL ( triggered ( bool ) ), this, SLOT ( addWelcomeBaskets() ) );
}

QTreeWidgetItem* BNPView::firstListViewItem()
{
	return m_tree->topLevelItem ( 0 );
	//return m_tree->firstChild();
}

void BNPView::slotShowProperties ( QTreeWidgetItem *item, const QPoint&, int )
{
	if ( item )
		propBasket();
}

void BNPView::slotMouseButtonPressed ( int button, QTreeWidgetItem *item, const QPoint &/*pos*/, int /*column*/ )
{
	if ( item && ( button & Qt::MidButton ) )
	{
		// TODO: Paste into ((BasketListViewItem*)listViewItem)->basket()
	}
}

void BNPView::slotContextMenu ( QTreeWidget */*listView*/, QTreeWidgetItem *item, const QPoint &pos )
{
	QString menuName;
	if ( item )
	{
		Basket* basket = ( ( BasketTreeItem* ) item )->basket();

		setCurrentBasket ( basket );
		menuName = "basket_popup";
	}
	else
	{
		menuName = "tab_bar_popup";
		/*
		* "File -> New" create a new basket with the same parent basket as the the current one.
		* But when invoked when right-clicking the empty area at the bottom of the basket tree,
		* it is obvious the user want to create a new basket at the bottom of the tree (with no parent).
		* So we set a temporary variable during the time the popup menu is shown,
		 * so the slot askNewBasket() will do the right thing:
		*/
		setNewBasketPopup();
	}

	QMenu *menu = popupMenu ( menuName );
	connect ( qobject_cast<QObject*> ( menu ), SIGNAL ( aboutToHide() ),  qobject_cast<QObject*> ( this ), SLOT ( aboutToHideNewBasketPopup() ) );
	menu->exec ( pos );
}

void BNPView::save()
{
	kDebug() << "saving..." << endl;
	DEBUG_WIN << "Basket Tree: Saving...";

	// Create Document:
	QDomDocument document ( "basketTree" );
	QDomElement root = document.createElement ( "basketTree" );
	document.appendChild ( root );

	// Save Basket Tree:
	save ( m_tree->topLevelItem ( 0 ), document, root );

	// Write to Disk:
	Basket::safelySaveToFile ( Global::basketsFolder() + "baskets.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n" + document.toString() );
	QFile file ( Global::basketsFolder() + "baskets.xml" );
	if ( file.open ( QIODevice::WriteOnly ) )
	{
		QTextStream stream ( &file );
		//FIXME: should remove that stream.setEncoding(QTextStream::UnicodeUTF8);
		QString xml = document.toString();
		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
		stream << xml;
		file.close();
	}
}

void BNPView::save ( QTreeWidgetItem *firstItem, QDomDocument &document, QDomElement &parentElement )
{
	kDebug() << "Enter" << endl;
	QTreeWidgetItem *item = firstItem;
	while ( item )
	{
		Basket *basket = ( ( BasketTreeItem* ) item )->basket();
		QDomElement basketElement = this->basketElement ( item, document, parentElement );
		/*
				QDomElement basketElement = document.createElement("basket");
				parentElement.appendChild(basketElement);
				// Save Attributes:
				basketElement.setAttribute("folderName", basket->folderName());
				if (item->firstChild()) // If it can be expanded/folded:
					basketElement.setAttribute("folded", XMLWork::trueOrFalse(!item->isOpen()));
				if (((BasketListViewItem*)item)->isCurrentBasket())
					basketElement.setAttribute("lastOpened", "true");
				// Save Properties:
				QDomElement properties = document.createElement("properties");
				basketElement.appendChild(properties);
				basket->saveProperties(document, properties);
		*/
		// Save Child Basket:
		/* FIXME 1.5		if (item->firstChild())
					save(item->firstChild(), document, basketElement);
				// Next Basket:
				item = item->nextSibling();*/
	}
}

QDomElement BNPView::basketElement ( QTreeWidgetItem *item, QDomDocument &document, QDomElement &parentElement )
{
	Basket *basket = ( ( BasketTreeItem* ) item )->basket();
	QDomElement basketElement = document.createElement ( "basket" );
	parentElement.appendChild ( basketElement );
	// Save Attributes:
	basketElement.setAttribute ( "folderName", basket->folderName() );
	if ( item->childCount() ) // If it can be expanded/folded:
		basketElement.setAttribute ( "folded", XMLWork::trueOrFalse ( !item->isExpanded () ) );
	if ( ( ( BasketTreeItem* ) item )->isCurrentBasket() )
		basketElement.setAttribute ( "lastOpened", "true" );
	// Save Properties:
	QDomElement properties = document.createElement ( "properties" );
	basketElement.appendChild ( properties );
	basket->saveProperties ( document, properties );
	return basketElement;
}

void BNPView::saveSubHierarchy ( QTreeWidgetItem *item, QDomDocument &document, QDomElement &parentElement, bool recursive )
{
	QDomElement element = basketElement ( item, document, parentElement );
	if ( recursive && item->childCount() !=0 )
		save ( item->child ( 0 ), document, element );
}

void BNPView::load()
{
	QDomDocument *doc = XMLWork::openFile ( "basketTree", Global::basketsFolder() + "baskets.xml" );
	//BEGIN Compatibility with 0.6.0 Pre-Alpha versions:
	//if ( !doc )
	//	doc = XMLWork::openFile ( "basketsTree", Global::basketsFolder() + "baskets.xml" );
	//END
	if ( doc != 0 )
	{
		QDomElement docElem = doc->documentElement();
		load ( m_tree, 0L, docElem );
	}
	m_loading = false;
}

void BNPView::load ( QTreeWidget */*listView*/, QTreeWidgetItem *item, const QDomElement &baskets )
{
	QDomNode n = baskets.firstChild();
	while ( ! n.isNull() )
	{
		DEBUG_WIN<<"load";
		QDomElement element = n.toElement();
		if ( ( !element.isNull() ) && element.tagName() == "basket" )
		{
			QString folderName = element.attribute ( "folderName" );
			if ( !folderName.isEmpty() )
			{
				Basket *basket = loadBasket ( folderName );
				BasketTreeItem *basketItem = appendBasket ( basket, ( BasketTreeItem* ) item );
				basketItem->setExpanded ( !XMLWork::trueOrFalse ( element.attribute ( "folded", "false" ), false ) );
				basket->loadProperties ( XMLWork::getElement ( element, "properties" ) );
				if ( XMLWork::trueOrFalse ( element.attribute ( "lastOpened", element.attribute ( "lastOpenned", "false" ) ), false ) ) // Compat with 0.6.0-Alphas
					setCurrentBasket ( basket );
				// Load Sub-baskets:*/
				load ( /*(QListView*)*/0L, basketItem, element );
			}
		}
		n = n.nextSibling();
	}
}

Basket* BNPView::loadBasket ( const QString &folderName )
{
	kDebug() << folderName << endl;
	if ( folderName.isEmpty() )
		return 0;

	DecoratedBasket *decoBasket = new DecoratedBasket ( m_stack, folderName );
	Basket          *basket     = decoBasket->basket();
	m_stack->addWidget ( decoBasket );
	connect ( basket, SIGNAL ( countsChanged ( Basket* ) ), this, SLOT ( countsChanged ( Basket* ) ) );
//	Important: Create listViewItem and connect signal BEFORE loadProperties(), so we get the listViewItem updated without extra work:
	connect ( basket, SIGNAL ( propertiesChanged ( Basket* ) ), this, SLOT ( updateBasketListViewItem ( Basket* ) ) );

	connect ( basket->decoration()->filterBar(), SIGNAL ( newFilter ( const FilterData& ) ), this, SLOT ( newFilterFromFilterBar() ) );

	kDebug() << "returning..." << endl;
	return basket;
}

int BNPView::basketCount ( QTreeWidgetItem *parent )
{
	int count = 0;
	if ( parent!=0 )
		count=parent->childCount();
	return count;
}

bool BNPView::canFold()
{
	BasketTreeItem *item = listViewItemForBasket ( currentBasket() );
	if ( !item )
		return false;
	return item->parent() || ( ( item->childCount() !=0 ) && item->isExpanded() );
}

bool BNPView::canExpand()
{
	BasketTreeItem *item = listViewItemForBasket ( currentBasket() );
	if ( !item )
		return false;
	return ( item->childCount() >0 );
}

BasketTreeItem* BNPView::appendBasket ( Basket *basket, BasketTreeItem *parentItem )
{
	kDebug() << "append basket to the tree" << endl;
	kDebug() << (int)basket << " " << (int)parentItem << endl;

	BasketTreeItem *newBasketItem;
	if ( parentItem ) {
		QTreeWidgetItem *lastChild = 0;
		if ( parentItem->childCount() )
			lastChild= parentItem->child ( parentItem->childCount()-1 );
		newBasketItem = new BasketTreeItem ( parentItem, lastChild, basket );
	} else {
		/*FIXME, REMOVE, NEW IMPLEMENTATION IN 1.5: QTreeWidgetItem *lastChild = 0;
		QTreeWidgetItem *topLevel=0;
		if ( m_tree->topLevelItemCount() )
			topLevel= m_tree->topLevelItem ( m_tree->topLevelItemCount()-1 );
		if ( topLevel != 0 && topLevel->childCount() )
			lastChild=topLevel->child ( topLevel->childCount()-1 );
		newBasketItem = new BasketTreeItem ( topLevel, lastChild, basket );*/

		newBasketItem = new BasketTreeItem( (QTreeWidgetItem*)0, basket );
		m_tree->addTopLevelItem( newBasketItem );
	}

	emit basketNumberChanged ( basketCount() );

	kDebug() << "exiting..." << endl;
	return newBasketItem;
}

void BNPView::loadNewBasket ( const QString &folderName, const QDomElement &properties, Basket *parent )
{
	kDebug() << folderName << " " << (int)parent << endl;
	Basket *basket = loadBasket ( folderName );
	appendBasket ( basket, ( basket ? listViewItemForBasket ( parent ) : 0 ) );
	//basket->loadProperties ( properties );
	setCurrentBasket ( basket );
//FIXME: In order to remove, masked in previous version:	save();
	kDebug() << "End" << endl;
}

BasketTreeItem* BNPView::lastListViewItem()
{
	QTreeWidgetItem *lastChild = 0;
//	Set lastChild to the last primary child of the list view:
	if ( m_tree->topLevelItemCount() )
	{
		lastChild = m_tree->topLevelItem ( m_tree->topLevelItemCount()-1 );
//	If this child have child(s), recursivly browse through them to find the real last one:
		while ( lastChild && lastChild->childCount() )
		{
			lastChild = lastChild->child ( lastChild->childCount()-1 );
		}

	}
	return ( BasketTreeItem* ) lastChild;
}

void BNPView::goToPreviousBasket()
{
	if ( !m_tree->topLevelItem ( 0 ) )
		return;

	BasketTreeItem *item     = listViewItemForBasket ( currentBasket() );
	BasketTreeItem *toSwitch = item->shownItemAbove();
	if ( !toSwitch )
	{
		toSwitch = lastListViewItem();
		if ( toSwitch && !toSwitch->isVisible() )
			toSwitch = toSwitch->shownItemAbove();
	}

	if ( toSwitch )
		setCurrentBasket ( toSwitch->basket() );

	if ( Settings::usePassivePopup() )
		showPassiveContent();
}

void BNPView::goToNextBasket()
{
	if ( !m_tree->topLevelItem ( 0 ) )
		return;

	BasketTreeItem *item     = listViewItemForBasket ( currentBasket() );
	BasketTreeItem *toSwitch = item->shownItemBelow();
	if ( !toSwitch )
		toSwitch = ( ( BasketTreeItem* ) m_tree->topLevelItem ( 0 ) );

	if ( toSwitch )
		setCurrentBasket ( toSwitch->basket() );

	if ( Settings::usePassivePopup() )
		showPassiveContent();
}

void BNPView::foldBasket()
{
	BasketTreeItem *item = listViewItemForBasket ( currentBasket() );
	if ( item && ( item->childCount() ==0 ) )
		item->setExpanded ( false ); // If Alt+Left is hitted and there is nothing to close, make sure the focus will go to the parent basket

	QKeyEvent* keyEvent = new QKeyEvent ( QEvent::KeyPress, Qt::Key_Left, 0, 0 );
	QApplication::postEvent ( m_tree, keyEvent );

}

void BNPView::expandBasket()
{
	QKeyEvent* keyEvent = new QKeyEvent ( QEvent::KeyPress, Qt::Key_Right, 0, 0 );
	QApplication::postEvent ( m_tree, keyEvent );
}

void BNPView::closeAllEditors()
{
	QTreeWidgetItemIterator it ( m_tree );
	while ( *it )
	{
		BasketTreeItem *item = ( BasketTreeItem* ) ( *it );
		item->basket()->closeEditor();
		++it;
	}
}

bool BNPView::convertTexts()
{
	bool convertedNotes = false;
	KProgressDialog dialog (
	    /*parent=*/0,
	    /*caption=*/i18n ( "Plain Text Notes Conversion" ),
	    /*text=*/i18n ( "Converting plain text notes to rich text ones..." ) );
	dialog.setObjectName ( "" );
	dialog.setModal ( true );
	dialog.progressBar()->setRange ( 0, basketCount() );
	dialog.show(); //setMinimumDuration(50/*ms*/);

	QTreeWidgetItemIterator it ( m_tree );
	while ( *it )
	{
		BasketTreeItem *item = ( BasketTreeItem* ) ( *it );
		if ( item->basket()->convertTexts() )
			convertedNotes = true;
		dialog.progressBar()->setValue ( dialog.progressBar()->value() +1 );
		if ( dialog.wasCancelled() )
			break;
		++it;
	}

	return convertedNotes;
}

/** isRunning is to avoid recursive calls because this method can be called
 * when clicking the menu action or when using the filter-bar icon... either of those calls
 * call the other to be checked... and it can cause recursive calls.
 * PS: Uggly hack? Yes, I think so :-)
 */
void BNPView::toggleFilterAllBaskets ( bool doFilter )
{
	static bool isRunning = false;
	if ( isRunning )
		return;
	isRunning = true;

	// Set the state:
	m_actFilterAllBaskets->setChecked ( doFilter );
	currentBasket()->decoration()->filterBar()->setFilterAll ( doFilter );

//	Basket *current = currentBasket();
	QTreeWidgetItemIterator it ( m_tree );
	while ( *it )
	{
		BasketTreeItem *item = ( ( BasketTreeItem* ) ( *it ) );
		item->basket()->decoration()->filterBar()->setFilterAll ( doFilter );
		++it;
	}

	// Protection is not necessary anymore:
	isRunning = false;

	if ( doFilter )
		currentBasket()->decoration()->filterBar()->setEditFocus();

	// Filter every baskets:
	newFilter();
}

/** This function can be called recursively because we call kapp->processEvents().
 * If this function is called whereas another "instance" is running,
 * this new "instance" leave and set up a flag that is read by the first "instance"
 * to know it should re-begin the work.
 * PS: Yes, that's a very lame pseudo-threading but that works, and it's programmer-efforts cheap :-)
 */
void BNPView::newFilter()
{
	static bool alreadyEntered = false;
	static bool shouldRestart  = false;

	if ( alreadyEntered )
	{
		shouldRestart = true;
		return;
	}
	alreadyEntered = true;
	shouldRestart  = false;

	Basket *current = currentBasket();
	const FilterData &filterData = current->decoration()->filterBar()->filterData();

	// Set the filter data for every other baskets, or reset the filter for every other baskets if we just disabled the filterInAllBaskets:
	/* FIXME 1.5	QTreeWidgetIterator it(m_tree);
		while (it.current()) {
			BasketListViewItem *item = ((BasketListViewItem*)it.current());
			if (item->basket() != current)
				if (isFilteringAllBaskets())
					item->basket()->decoration()->filterBar()->setFilterData(filterData); // Set the new FilterData for every other baskets
				else
					item->basket()->decoration()->filterBar()->setFilterData(FilterData()); // We just disabled the global filtering: remove the FilterData
			++it;
		}
	*/
	// Show/hide the "little filter icons" (during basket load)
	// or the "little numbers" (to show number of found notes in the baskets) is the tree:
//FIXME 1.5	m_tree->triggerUpdate();
	kapp->processEvents();

	// Load every baskets for filtering, if they are not already loaded, and if necessary:
	if ( filterData.isFiltering )
	{
		Basket *current = currentBasket();
		/* FIXME 1.5		QTreeWidgetIterator it(m_tree);
				while (it.current()) {
					BasketListViewItem *item = ((BasketListViewItem*)it.current());
					if (item->basket() != current) {
						Basket *basket = item->basket();
						if (!basket->loadingLaunched() && !basket->isLocked())
							basket->load();
						basket->filterAgain();
						m_tree->triggerUpdate();
						kapp->processEvents();
						if (shouldRestart) {
							alreadyEntered = false;
							shouldRestart  = false;
							newFilter();
							return;
						}
					}
					++it;
				}*/
	}

//FIXME 1.5	m_tree->triggerUpdate();
//	kapp->processEvents();

	alreadyEntered = false;
	shouldRestart  = false;
}

void BNPView::newFilterFromFilterBar()
{
	if ( isFilteringAllBaskets() )
		QTimer::singleShot ( 0, this, SLOT ( newFilter() ) ); // Keep time for the QLineEdit to display the filtered character and refresh correctly!
}

bool BNPView::isFilteringAllBaskets()
{
//FIXME 1.5	return m_actFilterAllBaskets->isChecked();
	return true; //TODO to remove when fixed
}


BasketTreeItem* BNPView::listViewItemForBasket ( Basket *basket )
{
	QTreeWidgetItemIterator it(m_tree);
	while (*it) {
		BasketTreeItem *item = ((BasketTreeItem*)*it);
		if (item->basket() == basket)
			return item;
		++it;
	}
	return 0L;
}

Basket* BNPView::currentBasket()
{
	DecoratedBasket *decoBasket = ( DecoratedBasket* ) m_stack->currentWidget();
	if ( decoBasket )
		return decoBasket->basket();
	else
		return 0;
}

Basket* BNPView::parentBasketOf ( Basket *basket )
{
	BasketTreeItem *item = ( BasketTreeItem* ) ( listViewItemForBasket ( basket )->parent() );
	if ( item )
		return item->basket();
	else
		return 0;
}

void BNPView::setCurrentBasket ( Basket *basket )
{
	if ( currentBasket() == basket )
		return;

	if ( currentBasket() )
		currentBasket()->closeBasket();

	if ( basket )
		basket->aboutToBeActivated();

	BasketTreeItem *item = listViewItemForBasket ( basket );
	/* FIXME 1.5	if (item) {
			m_tree->setSelected(item, true);
			item->ensureVisible();
			m_stack->raiseWidget(basket->decoration());
			// If the window has changed size, only the current basket receive the event,
			// the others will receive ony one just before they are shown.
			// But this triggers unwanted animations, so we eliminate it:*/
// FIXME 1.5		basket->relayoutNotes(/*animate=*/false);
	/* FIXME 1.5		basket->openBasket();
			setCaption(item->basket()->basketName());
			countsChanged(basket);
			updateStatusBarHint();
			if (Global::systemTray)
				Global::systemTray->updateToolTip();
			m_tree->ensureItemVisible(m_tree->currentItem());
			item->basket()->setFocus();
		}*/
	m_tree->viewport()->update();
	emit basketChanged();
}

void BNPView::removeBasket ( Basket *basket )
{
	if ( basket->isDuringEdit() )
		basket->closeEditor();

	// Find a new basket to switch to and select it.
	// Strategy: get the next sibling, or the previous one if not found.
	// If there is no such one, get the parent basket:
	BasketTreeItem *basketItem = listViewItemForBasket ( basket );
	/* FIXME 1.5	BasketListViewItem *nextBasketItem = (BasketListViewItem*)(basketItem->nextSibling());
		if (!nextBasketItem)
			nextBasketItem = basketItem->prevSibling();*/
// FIXME 1.5	if (!nextBasketItem)
// 		nextBasketItem = (BasketListViewItem*)(basketItem->parent());
//
// 	if (nextBasketItem)
// 		setCurrentBasket(nextBasketItem->basket());

	// Remove from the view:
	basket->unsubscribeBackgroundImages();
	m_stack->removeWidget ( basket->decoration() );
//	delete basket->decoration();
	delete basketItem;
//	delete basket;

	// If there is no basket anymore, add a new one:
// FIXME 1.5	if (!nextBasketItem)
// 		BasketFactory::newBasket(/*icon=*/"", /*name=*/i18n("General"), /*backgroundImage=*/"", /*backgroundColor=*/QColor(), /*textColor=*/QColor(), /*templateName=*/"1column", /*createIn=*/0);
// 	else // No need to save two times if we add a basket
// 		save();

	emit basketNumberChanged ( basketCount() );
}

void BNPView::setTreePlacement ( bool onLeft )
{
	/*FIXME	if (onLeft)
			moveToFirst(m_tree);
		else
			moveToLast(m_tree);
		//updateGeometry();
		kapp->postEvent( this, new QResizeEvent(size(), size()) );*/
}

void BNPView::relayoutAllBaskets()
{
	QTreeWidgetItemIterator it ( m_tree );
	while ( *it )
	{
		BasketTreeItem *item = ( ( BasketTreeItem* ) ( *it ) );
		item->basket()->unbufferizeAll();
		item->basket()->unsetNotesWidth();
		item->basket()->relayoutNotes ( true );
		++it;
	}
}

void BNPView::recomputeAllStyles()
{
	QTreeWidgetItemIterator it ( m_tree );
	while ( *it )
	{
		BasketTreeItem *item = ( ( BasketTreeItem* ) ( *it ) );
		item->basket()->recomputeAllStyles();
		item->basket()->unsetNotesWidth();
		item->basket()->relayoutNotes ( true );
		++it;
	}
}

void BNPView::removedStates ( const QList<State*> &deletedStates )
{
	QTreeWidgetItemIterator it ( m_tree );
	while ( *it )
	{
		BasketTreeItem *item = ( ( BasketTreeItem* ) ( *it ) );
		item->basket()->removedStates ( deletedStates );
		++it;
	}
}

void BNPView::linkLookChanged()
{
	QTreeWidgetItemIterator it ( m_tree );
	while ( *it )
	{
		BasketTreeItem *item = ( ( BasketTreeItem* ) ( *it ) );
		item->basket()->linkLookChanged();
		++it;
	}
}

void BNPView::filterPlacementChanged ( bool onTop )
{
	QTreeWidgetItemIterator it ( m_tree );
	while ( *it )
	{
		BasketTreeItem *item        = dynamic_cast<BasketTreeItem*> ( *it );
		DecoratedBasket    *decoration  = dynamic_cast<DecoratedBasket*> ( item->basket()->parent() );
		decoration->setFilterBarPosition ( onTop );
		++it;
	}
}

void BNPView::updateBasketListViewItem ( Basket *basket )
{
	BasketTreeItem *item = listViewItemForBasket ( basket );
	if ( item )
		item->setup();

	if ( basket == currentBasket() )
	{
		setCaption ( basket->basketName() );
		if ( Global::systemTray )
			Global::systemTray->updateToolTip();
	}

	// Don't save if we are loading!
	if ( !m_loading )
		save();
}

void BNPView::needSave ( QTreeWidgetItem* )
{
	if ( !m_loading )
		// A basket has been collapsed/expanded or a new one is select: this is not urgent:
		QTimer::singleShot ( 500/*ms*/, this, SLOT ( save() ) );
}

void BNPView::slotPressed ( QTreeWidgetItem *item, const QPoint &/*pos*/, int /*column*/ )
{
	Basket *basket = currentBasket();
	if ( basket == 0 )
		return;

	// Impossible to Select no Basket:
	/* FIXME 1.5	if (!item)
			m_tree->setSelected(listViewItemForBasket(basket), true);
		else if (dynamic_cast<BasketListViewItem*>(item) != 0 && currentBasket() != ((BasketListViewItem*)item)->basket()) {
			setCurrentBasket( ((BasketListViewItem*)item)->basket() );
			needSave(0);
		}*/
	basket->setFocus();
}

DecoratedBasket* BNPView::currentDecoratedBasket()
{
	if ( currentBasket() )
		return currentBasket()->decoration();
	else
		return 0;
}

// Redirected actions :

void BNPView::exportToHTML()              { HTMLExporter exporter ( currentBasket() );  }
void BNPView::editNote()
{
	if ( currentBasket()  )
		currentBasket()->noteEdit();
}
void BNPView::cutNote()
{
	if ( currentBasket()  )
		currentBasket()->noteCut();
}
void BNPView::copyNote()
{
	if ( currentBasket()  )
		currentBasket()->noteCopy();
}
void BNPView::delNote()
{
	if ( currentBasket()  )
		currentBasket()->noteDelete();
}
void BNPView::openNote()
{
	if ( currentBasket()  )
		currentBasket()->noteOpen();
}
void BNPView::openNoteWith()
{
	if ( currentBasket()  )
		currentBasket()->noteOpenWith();
}
void BNPView::saveNoteAs()
{
	if ( currentBasket()  )
		currentBasket()->noteSaveAs();
}
void BNPView::noteGroup()
{
	if ( currentBasket()  )
		currentBasket()->noteGroup();
}
void BNPView::noteUngroup()
{
	if ( currentBasket()  )
		currentBasket()->noteUngroup();
}
void BNPView::moveOnTop()
{
	if ( currentBasket()  )
		currentBasket()->noteMoveOnTop();
}
void BNPView::moveOnBottom()
{
	if ( currentBasket()  )
		currentBasket()->noteMoveOnBottom();
}
void BNPView::moveNoteUp()
{
	if ( currentBasket()  )
		currentBasket()->noteMoveNoteUp();
}
void BNPView::moveNoteDown()
{
	if ( currentBasket()  )
		currentBasket()->noteMoveNoteDown();
}
void BNPView::slotSelectAll()
{
	if ( currentBasket()  )
		currentBasket()->selectAll();
}
void BNPView::slotUnselectAll()
{
	if ( currentBasket()  )
		currentBasket()->unselectAll();
}
void BNPView::slotInvertSelection()
{
	if ( currentBasket()  )
		currentBasket()->invertSelection();
}
void BNPView::slotResetFilter()
{
	if ( currentDecoratedBasket()  )
		currentDecoratedBasket()->resetFilter();
}

void BNPView::importKJots()       { SoftwareImporters::importKJots();       }
void BNPView::importKNotes()      { SoftwareImporters::importKNotes();      }
void BNPView::importKnowIt()      { SoftwareImporters::importKnowIt();      }
void BNPView::importTuxCards()    { SoftwareImporters::importTuxCards();    }
void BNPView::importStickyNotes() { SoftwareImporters::importStickyNotes(); }
void BNPView::importTomboy()      { SoftwareImporters::importTomboy();      }
void BNPView::importTextFile()    { SoftwareImporters::importTextFile();    }

void BNPView::backupRestore()
{
	BackupDialog dialog;
	dialog.exec();
}

void BNPView::countsChanged ( Basket *basket )
{
	if ( basket == currentBasket() )
		notesStateChanged();
}

void BNPView::notesStateChanged()
{
	Basket *basket = currentBasket();

	// Update statusbar message :
	if ( currentBasket() && currentBasket()->isLocked() )
		setSelectionStatus ( i18n ( "Locked" ) );
	else if ( !basket->isLoaded() )
		setSelectionStatus ( i18n ( "Loading..." ) );
	else if ( basket->count() == 0 )
		setSelectionStatus ( i18n ( "No notes" ) );
	else
	{
		QString count     = i18nc ( "%n note",     "%n notes",    basket->count() );
		QString selecteds = i18nc ( "%n selected", "%n selected", basket->countSelecteds() );
		QString showns    = ( currentDecoratedBasket()->filterData().isFiltering ? i18n ( "all matches" ) : i18n ( "no filter" ) );
		if ( basket->countFounds() != basket->count() )
			showns = i18nc ( "%n match", "%n matches", basket->countFounds() );
		setSelectionStatus (
		    i18nc ( "e.g. '18 notes, 10 matches, 5 selected'", "%1, %2, %3" ).arg ( count, showns, selecteds ) );
	}

	// If we added a note that match the global filter, update the count number in the tree:
// FIXME 1.5	if (isFilteringAllBaskets())
// 		listViewItemForBasket(basket)->listView()->triggerUpdate();

	if ( currentBasket()->redirectEditActions() )
	{
		m_actSelectAll->setEnabled ( !currentBasket()->selectedAllTextInEditor() );
		m_actUnselectAll->setEnabled ( currentBasket()->hasSelectedTextInEditor() );
	}
	else
	{
		m_actSelectAll->setEnabled ( basket->countSelecteds() < basket->countFounds() );
		m_actUnselectAll->setEnabled ( basket->countSelecteds() > 0 );
	}
	m_actInvertSelection->setEnabled ( basket->countFounds() > 0 );

	updateNotesActions();
}

void BNPView::updateNotesActions()
{
	bool isLocked             = currentBasket()->isLocked();
	bool oneSelected          = currentBasket()->countSelecteds() == 1;
	bool oneOrSeveralSelected = currentBasket()->countSelecteds() >= 1;
	bool severalSelected      = currentBasket()->countSelecteds() >= 2;

	// FIXME: m_actCheckNotes is also modified in void BNPView::areSelectedNotesCheckedChanged(bool checked)
	//        bool Basket::areSelectedNotesChecked() should return false if bool Basket::showCheckBoxes() is false
//	m_actCheckNotes->setChecked( oneOrSeveralSelected &&
//	                             currentBasket()->areSelectedNotesChecked() &&
//	                             currentBasket()->showCheckBoxes()             );

	Note *selectedGroup = ( severalSelected ? currentBasket()->selectedGroup() : 0 );

	m_actEditNote            ->setEnabled ( !isLocked && oneSelected && !currentBasket()->isDuringEdit() );
	if ( currentBasket()->redirectEditActions() )
	{
		m_actCutNote         ->setEnabled ( currentBasket()->hasSelectedTextInEditor() );
		m_actCopyNote        ->setEnabled ( currentBasket()->hasSelectedTextInEditor() );
		m_actPaste           ->setEnabled ( true );
		m_actDelNote         ->setEnabled ( currentBasket()->hasSelectedTextInEditor() );
	}
	else
	{
		m_actCutNote         ->setEnabled ( !isLocked && oneOrSeveralSelected );
		m_actCopyNote        ->setEnabled ( oneOrSeveralSelected );
		m_actPaste           ->setEnabled ( !isLocked );
		m_actDelNote         ->setEnabled ( !isLocked && oneOrSeveralSelected );
	}
	m_actOpenNote        ->setEnabled ( oneOrSeveralSelected );
	m_actOpenNoteWith    ->setEnabled ( oneSelected );                      // TODO: oneOrSeveralSelected IF SAME TYPE
	m_actSaveNoteAs      ->setEnabled ( oneSelected );                      // IDEM?
	m_actGroup           ->setEnabled ( !isLocked && severalSelected && ( !selectedGroup || selectedGroup->isColumn() ) );
	m_actUngroup         ->setEnabled ( !isLocked && selectedGroup && !selectedGroup->isColumn() );
	m_actMoveOnTop       ->setEnabled ( !isLocked && oneOrSeveralSelected && !currentBasket()->isFreeLayout() );
	m_actMoveNoteUp      ->setEnabled ( !isLocked && oneOrSeveralSelected ); // TODO: Disable when unavailable!
	m_actMoveNoteDown    ->setEnabled ( !isLocked && oneOrSeveralSelected );
	m_actMoveOnBottom    ->setEnabled ( !isLocked && oneOrSeveralSelected && !currentBasket()->isFreeLayout() );

// FIXME 1.5	for (KAction *action = m_insertActions.first(); action; action = m_insertActions.next())
// 		action->setEnabled( !isLocked );

	// From the old Note::contextMenuEvent(...) :
	/*	if (useFile() || m_type == Link) {
		m_type == Link ? i18n("&Open target")         : i18n("&Open")
		m_type == Link ? i18n("Open target &with...") : i18n("Open &with...")
		m_type == Link ? i18n("&Save target as...")   : i18n("&Save a copy as...")
			// If useFile() theire is always a file to open / open with / save, but :
		if (m_type == Link) {
				if (url().prettyUrl().isEmpty() && runCommand().isEmpty())     // no URL nor runCommand :
		popupMenu->setItemEnabled(7, false);                       //  no possible Open !
				if (url().prettyUrl().isEmpty())                               // no URL :
		popupMenu->setItemEnabled(8, false);                       //  no possible Open with !
				if (url().prettyUrl().isEmpty() || url().path().endsWith("/")) // no URL or target a folder :
		popupMenu->setItemEnabled(9, false);                       //  not possible to save target file
	}
	} else if (m_type != Color) {
		popupMenu->insertSeparator();
		popupMenu->insertItem( SmallIconSet("document-save-as"), i18n("&Save a copy as..."), this, SLOT(slotSaveAs()), 0, 10 );
	}*/
}

// BEGIN Color picker (code from KColorEdit):

/* Activate the mode
 */
void BNPView::slotColorFromScreen ( bool global )
{
	m_colorPickWasGlobal = global;
	if ( isMainWindowActive() )
	{
		if ( Global::mainWindow() ) Global::mainWindow()->hide();
		m_colorPickWasShown = true;
	}
	else
		m_colorPickWasShown = false;

	currentBasket()->saveInsertionData();
	m_colorPicker->pickColor();

	/*	m_gettingColorFromScreen = true;
			kapp->processEvents();
			QTimer::singleShot( 100, this, SLOT(grabColorFromScreen()) );*/
}

void BNPView::slotColorFromScreenGlobal()
{
	slotColorFromScreen ( true );
}

void BNPView::colorPicked ( const QColor &color )
{
	if ( !currentBasket()->isLoaded() )
	{
		showPassiveLoading ( currentBasket() );
		currentBasket()->load();
	}
	currentBasket()->insertColor ( color );

	if ( m_colorPickWasShown )
		showMainWindow();

	if ( Settings::usePassivePopup() )
		showPassiveDropped ( i18n ( "Picked color to basket <i>%1</i>" ) );
}

void BNPView::colorPickingCanceled()
{
	if ( m_colorPickWasShown )
		showMainWindow();
}

void BNPView::slotConvertTexts()
{
	/*
		int result = KMessageBox::questionYesNoCancel(
			this,
			i18n(
				"<p>This will convert every text notes into rich text notes.<br>"
				"The content of the notes will not change and you will be able to apply formating to those notes.</p>"
				"<p>This process cannot be reverted back: you will not be able to convert the rich text notes to plain text ones later.</p>"
				"<p>As a beta-tester, you are strongly encouraged to do the convert process because it is to test if plain text notes are still needed.<br>"
				"If nobody complain about not having plain text notes anymore, then the final version is likely to not support plain text notes anymore.</p>"
				"<p><b>Which basket notes do you want to convert?</b></p>"
			),
			i18n("Convert Text Notes"),
			KGuiItem(i18n("Only in the Current Basket")),
			KGuiItem(i18n("In Every Baskets"))
		);
		if (result == KMessageBox::Cancel)
			return;
	*/

	bool conversionsDone;
//	if (result == KMessageBox::Yes)
//		conversionsDone = currentBasket()->convertTexts();
//	else
	conversionsDone = convertTexts();

	if ( conversionsDone )
		KMessageBox::information ( this, i18n ( "The plain text notes have been converted to rich text." ), i18n ( "Conversion Finished" ) );
	else
		KMessageBox::information ( this, i18n ( "There are no plain text notes to convert." ), i18n ( "Conversion Finished" ) );
}

QMenu* BNPView::popupMenu ( const QString &menuName )
{
	QMenu *menu = 0;
	bool hack = false; // TODO fix this
	// When running in kontact and likeback Information message is shown
	// factory is 0. Don't show error then and don't crash either :-)

	if ( m_guiClient )
	{
		KXMLGUIFactory* factory = m_guiClient->factory();
		if ( factory )
		{
			menu = ( QMenu * ) factory->container ( menuName, m_guiClient );
		}
		else
			hack = isPart();
	}
	if ( menu == 0 )
	{
		/* FIXME 1.5		if(!hack)
				{
					KStandardDirs stdDirs;
					KMessageBox::error( this, i18n(
							"<p><b>The file basketui.rc seems to not exist or is too old.<br>"
									"%1 cannot run without it and will stop.</b></p>"
									"<p>Please check your installation of %2.</p>"
									"<p>If you do not have administrator access to install the application "
									"system wide, you can copy the file basketui.rc from the installation "
									"archive to the folder <a href='file://%3'>%4</a>.</p>"
									"<p>As last ressort, if you are sure the application is correctly installed "
									"but you had a preview version of it, try to remove the "
									"file %5basketui.rc</p>")
									.arg(KCmdLineArgs::aboutData( )->programName(), KCmdLineArgs::aboutData( )->programName(),
										stdDirs.saveLocation("data", "basket/")).arg(stdDirs.saveLocation("data", "basket/"), stdDirs.saveLocation("data", "basket/")),
							i18n("Ressource not Found"), KMessageBox::AllowLink );
				}*/
		if ( !isPart() )
			exit ( 1 ); // We SHOULD exit right now and abord everything because the caller except menu != 0 to not crash.
		else
			menu = new KMenu; // When running in kpart we cannot exit
	}
	return menu;
}

void BNPView::showHideFilterBar ( bool show, bool switchFocus )
{
//	if (show != m_actShowFilter->isChecked())
//		m_actShowFilter->setChecked(show);
	/* FIXME 1.5	m_actShowFilter->setChecked(currentDecoratedBasket()->filterData().isFiltering);

		currentDecoratedBasket()->setFilterBarShown(show, switchFocus);
		currentDecoratedBasket()->resetFilter();*/
}

void BNPView::insertEmpty ( int type )
{
	if ( currentBasket()->isLocked() )
	{
		showPassiveImpossible ( i18n ( "Cannot add note." ) );
		return;
	}
	currentBasket()->insertEmptyNote ( type );
}

void BNPView::insertWizard ( int type )
{
	if ( currentBasket()->isLocked() )
	{
		showPassiveImpossible ( i18n ( "Cannot add note." ) );
		return;
	}
	currentBasket()->insertWizard ( type );
}

// BEGIN Screen Grabbing: // FIXME

void BNPView::grabScreenshot ( bool global )
{
	if ( m_regionGrabber )
	{
		KWindowSystem::activateWindow ( m_regionGrabber->winId() );
		return;
	}

	// Delay before to take a screenshot because if we hide the main window OR the systray popup menu,
	// we should wait the windows below to be repainted!!!
	// A special case is where the action is triggered with the global keyboard shortcut.
	// In this case, global is true, and we don't wait.
	// In the future, if global is also defined for other cases, check for
	// enum KAction::ActivationReason { UnknownActivation, EmulatedActivation, AccelActivation, PopupMenuActivation, ToolBarActivation };
	int delay = ( isMainWindowActive() ? 500 : ( global/*kapp->activePopupWidget()*/ ? 0 : 200 ) );

	m_colorPickWasGlobal = global;
	if ( isMainWindowActive() )
	{
		if ( Global::mainWindow() ) Global::mainWindow()->hide();
		m_colorPickWasShown = true;
	}
	else
		m_colorPickWasShown = false;

	currentBasket()->saveInsertionData();
	m_regionGrabber = new RegionGrabber ( delay );
	connect ( m_regionGrabber, SIGNAL ( regionGrabbed ( const QPixmap& ) ), this, SLOT ( screenshotGrabbed ( const QPixmap& ) ) );
}

void BNPView::grabScreenshotGlobal()
{
	grabScreenshot ( true );
}

void BNPView::screenshotGrabbed ( const QPixmap &pixmap )
{
	delete m_regionGrabber;
	m_regionGrabber = 0;

	// Cancelled (pressed Escape):
	if ( pixmap.isNull() )
	{
		if ( m_colorPickWasShown )
			showMainWindow();
		return;
	}

	if ( !currentBasket()->isLoaded() )
	{
		showPassiveLoading ( currentBasket() );
		currentBasket()->load();
	}
	currentBasket()->insertImage ( pixmap );

	if ( m_colorPickWasShown )
		showMainWindow();

	if ( Settings::usePassivePopup() )
		showPassiveDropped ( i18n ( "Grabbed screen zone to basket <i>%1</i>" ) );
}

Basket* BNPView::basketForFolderName ( const QString &/*folderName*/ )
{
	/*	QPtrList<Basket> basketsList = listBaskets();
		Basket *basket;
		for (basket = basketsList.first(); basket; basket = basketsList.next())
		if (basket->folderName() == folderName)
		return basket;
	*/
	return 0;
}

void BNPView::setFiltering ( bool filtering )
{
	/* FIXME 1.5	m_actShowFilter->setChecked(filtering);
		m_actResetFilter->setEnabled(filtering);*/
}

void BNPView::undo()
{
	// TODO
}

void BNPView::redo()
{
	// TODO
}

void BNPView::pasteToBasket ( int /*index*/, QClipboard::Mode /*mode*/ )
{
	//TODO: REMOVE!
	//basketAt(index)->pasteNote(mode);
}

void BNPView::propBasket()
{
	BasketPropertiesDialog dialog ( currentBasket(), this );
	dialog.exec();
}

void BNPView::delBasket()
{
//	DecoratedBasket *decoBasket    = currentDecoratedBasket();
	Basket          *basket        = currentBasket();

#if 0
	KDialog *dialog = new KDialog ( this, /*name=*/0, /*modal=*/true, /*caption=*/i18n ( "Delete Basket" ),
	                                KDialog::User1 | KDialog::User2 | KDialog::No, KDialog::User1,
	                                /*separator=*/false,
	                                /*user1=*/KGuiItem ( i18n ( "Delete Only that Basket" ) /*, icon=""*/ ),
	                                /*user2=*/KGuiItem ( i18n ( "Delete Note & Children" ) /*, icon=""*/ ) );
	QStringList basketsList;
	basketsList.append ( "Basket 1" );
	basketsList.append ( "  Basket 2" );
	basketsList.append ( "    Basket 3" );
	basketsList.append ( "  Basket 4" );
	KMessageBox::createKMessageBox (
	    dialog, QMessageBox::Information,
	    i18n ( "<qt>Do you really want to remove the basket <b>%1</b> and its contents?</qt>" )
	    .arg ( Tools::textToHTMLWithoutP ( basket->basketName() ) ),
	    basketsList, /*ask=*/"", /*checkboxReturn=*/0, /*options=*/KMessageBox::Notify/*, const QString &details=QString::null*/ );
#endif

	int really = KMessageBox::questionYesNo ( this,
	             i18n ( "<qt>Do you really want to remove the basket <b>%1</b> and its contents?</qt>" )
	             .arg ( Tools::textToHTMLWithoutP ( basket->basketName() ) ),
	             i18n ( "Remove Basket" )
#if KDE_IS_VERSION( 3, 2, 90 ) // KDE 3.3.x
	             , KGuiItem ( i18n ( "&Remove Basket" ), "edit-delete" ), KStandardGuiItem::cancel() );
#else
	                                        );
#endif

	if ( really == KMessageBox::No )
		return;

	QStringList basketsList = listViewItemForBasket ( basket )->childNamesTree();
	if ( basketsList.count() > 0 )
	{
		int deleteChilds = KMessageBox::questionYesNoList ( this,
		                   i18n ( "<qt><b>%1</b> have the following children baskets.<br>Do you want to remove them too?</qt>" )
		                   .arg ( Tools::textToHTMLWithoutP ( basket->basketName() ) ),
		                   basketsList,
		                   i18n ( "Remove Children Baskets" )
#if KDE_IS_VERSION( 3, 2, 90 ) // KDE 3.3.x
		                   , KGuiItem ( i18n ( "&Remove Children Baskets" ), "edit-delete" ) );
#else
		                                                  );
#endif

		if ( deleteChilds == KMessageBox::No )
			listViewItemForBasket ( basket )->moveChildsBaskets();
	}

	doBasketDeletion ( basket );

//	basketNumberChanged();
//	rebuildBasketsMenu();
}

void BNPView::doBasketDeletion ( Basket *basket )
{
	basket->closeEditor();

	/* FIXME 1.5	QTreeWidget *basketItem = listViewItemForBasket(basket);
		QTreeWidget *nextOne;
		for (QTreeWidget *child = basketItem->firstChild(); child; child = nextOne) {
			nextOne = child->nextSibling();
			// First delete the child baskets:
			doBasketDeletion(((BasketListViewItem*)child)->basket());
		}
		// Then, basket have no child anymore, delete it:
		DecoratedBasket *decoBasket = basket->decoration();
		basket->deleteFiles();
		removeBasket(basket);
		// Remove the action to avoir keyboard-shortcut clashes:
		delete basket->m_action; // FIXME: It's quick&dirty. In the future, the Basket should be deleted, and then the KAction deleted in the Basket destructor.
		delete decoBasket;
	//	delete basket;*/
}

void BNPView::password()
{
#ifdef HAVE_LIBGPGME
	PasswordDlg dlg ( kapp->activeWindow(), "Password" );
	Basket *cur = currentBasket();

	dlg.setType ( cur->encryptionType() );
	dlg.setKey ( cur->encryptionKey() );
	if ( dlg.exec() )
	{
		cur->setProtection ( dlg.type(), dlg.key() );
		if ( cur->encryptionType() != Basket::NoEncryption )
			cur->lock();
	}
#endif
}

void BNPView::lockBasket()
{
#ifdef HAVE_LIBGPGME
	Basket *cur = currentBasket();

	cur->lock();
#endif
}

void BNPView::saveAsArchive()
{
	Basket *basket = currentBasket();

	QDir dir;

	/* FIXME 1.5	KConfig *config = KGlobal::config();
		config->setGroup("Basket Archive");
		QString folder = config->readEntry("lastFolder", QDir::homePath()) + "/";
		QString url = folder + QString(basket->basketName()).replace("/", "_") + ".baskets";

		QString filter = "*.baskets|" + i18n("Basket Archives") + "\n*|" + i18n("All Files");
		QString destination = url;
		for (bool askAgain = true; askAgain; ) {
			destination = KFileDialog::getSaveFileName(destination, filter, this, i18n("Save as Basket Archive"));
			if (destination.isEmpty()) // User canceled
				return;
			if (dir.exists(destination)) {
				int result = KMessageBox::questionYesNoCancel(
					this,
					"<qt>" + i18n("The file <b>%1</b> already exists. Do you really want to override it?")
						.arg(KUrl(destination).fileName()),
					i18n("Override File?"),
					KGuiItem(i18n("&Override"), "document-save")
				);
				if (result == KMessageBox::Cancel)
					return;
				else if (result == KMessageBox::Yes)
					askAgain = false;
			} else
				askAgain = false;
		}
		bool withSubBaskets = true;//KMessageBox::questionYesNo(this, i18n("Do you want to export sub-baskets too?"), i18n("Save as Basket Archive")) == KMessageBox::Yes;

		config->writeEntry("lastFolder", KUrl(destination).directory());
		config->sync();

		Archive::save(basket, withSubBaskets, destination);*/
}

QString BNPView::s_fileToOpen = "";

void BNPView::delayedOpenArchive()
{
	Archive::open ( s_fileToOpen );
}

void BNPView::openArchive()
{
	QString filter = "*.baskets|" + i18n ( "Basket Archives" ) + "\n*|" + i18n ( "All Files" );
	/* FIXME 1.5	QString path = KFileDialog::getOpenFileName(QString::null, filter, this, i18n("Open Basket Archive"));
		if (!path.isEmpty()) // User has not canceled
			Archive::open(path);*/
}


void BNPView::activatedTagShortcut()
{
	Tag *tag = Tag::tagForKAction ( ( KAction* ) sender() );
	currentBasket()->activatedTagShortcut ( tag );
}

void BNPView::slotBasketNumberChanged ( int number )
{
	m_actPreviousBasket->setEnabled ( number > 1 );
	m_actNextBasket    ->setEnabled ( number > 1 );
}

void BNPView::slotBasketChanged()
{
	m_actFoldBasket->setEnabled ( canFold() );
	m_actExpandBasket->setEnabled ( canExpand() );
	setFiltering ( currentBasket() && currentBasket()->decoration()->filterData().isFiltering );
}

void BNPView::currentBasketChanged()
{
}

void BNPView::isLockedChanged()
{
	bool isLocked = currentBasket()->isLocked();

	setLockStatus ( isLocked );

//	m_actLockBasket->setChecked(isLocked);
	m_actPropBasket->setEnabled ( !isLocked );
	m_actDelBasket ->setEnabled ( !isLocked );
	updateNotesActions();
}

void BNPView::askNewBasket()
{
	askNewBasket ( 0, 0 );
}

void BNPView::askNewBasket ( Basket *parent, Basket *pickProperties )
{
	NewBasketDefaultProperties properties;
	if ( pickProperties )
	{
		properties.icon            = pickProperties->icon();
		properties.backgroundImage = pickProperties->backgroundImageName();
		properties.backgroundColor = pickProperties->backgroundColorSetting();
		properties.textColor       = pickProperties->textColorSetting();
		properties.freeLayout      = pickProperties->isFreeLayout();
		properties.columnCount     = pickProperties->columnsCount();
	}

	NewBasketDialog ( parent, properties, this ).exec();
}

void BNPView::askNewSubBasket()
{
	askNewBasket ( /*parent=*/currentBasket(), /*pickPropertiesOf=*/currentBasket() );
}

void BNPView::askNewSiblingBasket()
{
	askNewBasket ( /*parent=*/parentBasketOf ( currentBasket() ), /*pickPropertiesOf=*/currentBasket() );
}

void BNPView::globalPasteInCurrentBasket()
{
	currentBasket()->setInsertPopupMenu();
	pasteInCurrentBasket();
	currentBasket()->cancelInsertPopupMenu();
}

void BNPView::pasteInCurrentBasket()
{
	currentBasket()->pasteNote();

	if ( Settings::usePassivePopup() )
		showPassiveDropped ( i18n ( "Clipboard content pasted to basket <i>%1</i>" ) );
}

void BNPView::pasteSelInCurrentBasket()
{
	currentBasket()->pasteNote ( QClipboard::Selection );

	if ( Settings::usePassivePopup() )
		showPassiveDropped ( i18n ( "Selection pasted to basket <i>%1</i>" ) );
}

void BNPView::showPassiveDropped ( const QString &title )
{
	if ( ! currentBasket()->isLocked() )
	{
		// TODO: Keep basket, so that we show the message only if something was added to a NOT visible basket
		m_passiveDroppedTitle     = title;
		m_passiveDroppedSelection = currentBasket()->selectedNotes();
		QTimer::singleShot ( c_delayTooltipTime, this, SLOT ( showPassiveDroppedDelayed() ) );
		// DELAY IT BELOW:
	}
	else
		showPassiveImpossible ( i18n ( "No note was added." ) );
}

void BNPView::showPassiveDroppedDelayed()
{
	if ( isMainWindowActive() || m_passiveDroppedSelection == 0 )
		return;

	QString title = m_passiveDroppedTitle;

	delete m_passivePopup; // Delete previous one (if exists): it will then hide it (only one at a time)
	m_passivePopup = new KPassivePopup ( Settings::useSystray() ? ( QWidget* ) Global::systemTray : this );
	QPixmap contentsPixmap = NoteDrag::feedbackPixmap ( m_passiveDroppedSelection );
	/*FIXME 1.5	QMimeSourceFactory::defaultFactory()->setPixmap("_passivepopup_image_", contentsPixmap);
		m_passivePopup->setView(
				title.arg(Tools::textToHTMLWithoutP(currentBasket()->basketName())),
		(contentsPixmap.isNull() ? "" : "<img src=\"_passivepopup_image_\">"),
		KIconLoader::global()->loadIcon(currentBasket()->icon(), K3Icon::NoGroup, 16, KIcon::DefaultState, 0L, true));
		m_passivePopup->show();*/
}

void BNPView::showPassiveImpossible ( const QString &message )
{
	/*FIXME 1.5	delete m_passivePopup; // Delete previous one (if exists): it will then hide it (only one at a time)
		m_passivePopup = new KPassivePopup(Settings::useSystray() ? (QWidget*)Global::systemTray : (QWidget*)this);
		m_passivePopup->setView(
				QString("<font color=red>%1</font>")
				.arg(i18n("Basket <i>%1</i> is locked"))
				.arg(Tools::textToHTMLWithoutP(currentBasket()->basketName())),
		message,
	 	KIconLoader::global()->loadIcon(currentBasket()->icon(), K3Icon::NoGroup, 16, KIcon::DefaultState, 0L, true));
		m_passivePopup->show();*/
}

void BNPView::showPassiveContentForced()
{
	showPassiveContent ( /*forceShow=*/true );
}

void BNPView::showPassiveContent ( bool forceShow/* = false*/ )
{
	/*FIXME 1.5	if (!forceShow && isMainWindowActive())
			return;

		// FIXME: Duplicate code (2 times)
		QString message;

		delete m_passivePopup; // Delete previous one (if exists): it will then hide it (only one at a time)
		m_passivePopup = new KPassivePopup(Settings::useSystray() ? (QWidget*)Global::systemTray : (QWidget*)this);
		m_passivePopup->setView(
				"<qt>" + KInstance::makeStandardCaption( currentBasket()->isLocked()
				? QString("%1 <font color=gray30>%2</font>")
				.arg(Tools::textToHTMLWithoutP(currentBasket()->basketName()), i18n("(Locked)"))
		: Tools::textToHTMLWithoutP(currentBasket()->basketName()) ),
		message,
		KIconLoader::global()->loadIcon(currentBasket()->icon(), K3Icon::NoGroup, 16, KIcon::DefaultState, 0L, true));
		m_passivePopup->show();*/
}

void BNPView::showPassiveLoading ( Basket *basket )
{
	/* FIXME 1.5	if (isMainWindowActive())
			return;

		delete m_passivePopup; // Delete previous one (if exists): it will then hide it (only one at a time)
		m_passivePopup = new KPassivePopup(Settings::useSystray() ? (QWidget*)Global::systemTray : (QWidget*)this);
		m_passivePopup->setView(
				Tools::textToHTMLWithoutP(basket->basketName()),
		i18n("Loading..."),
		KIconLoader::global()->loadIcon(basket->icon(), K3Icon::NoGroup, 16, KIcon::DefaultState, 0L, true));
		m_passivePopup->show();*/
}

void BNPView::addNoteText()  { showMainWindow(); currentBasket()->insertEmptyNote ( NoteType::Text );  }
void BNPView::addNoteHtml()  { showMainWindow(); currentBasket()->insertEmptyNote ( NoteType::Html );  }
void BNPView::addNoteImage() { showMainWindow(); currentBasket()->insertEmptyNote ( NoteType::Image ); }
void BNPView::addNoteLink()  { showMainWindow(); currentBasket()->insertEmptyNote ( NoteType::Link );  }
void BNPView::addNoteColor() { showMainWindow(); currentBasket()->insertEmptyNote ( NoteType::Color ); }

void BNPView::aboutToHideNewBasketPopup()
{
	QTimer::singleShot ( 0, this, SLOT ( cancelNewBasketPopup() ) );
}

void BNPView::cancelNewBasketPopup()
{
	m_newBasketPopup = false;
}

void BNPView::setNewBasketPopup()
{
	m_newBasketPopup = true;
}

void BNPView::setCaption ( QString s )
{
	emit setWindowCaption ( s );
}

void BNPView::updateStatusBarHint()
{
	m_statusbar->updateStatusBarHint();
}

void BNPView::setSelectionStatus ( QString s )
{
	m_statusbar->setSelectionStatus ( s );
}

void BNPView::setLockStatus ( bool isLocked )
{
	m_statusbar->setLockStatus ( isLocked );
}

void BNPView::postStatusbarMessage ( const QString& msg )
{
	m_statusbar->postStatusbarMessage ( msg );
}

void BNPView::setStatusBarHint ( const QString &hint )
{
	m_statusbar->setStatusBarHint ( hint );
}

void BNPView::setUnsavedStatus ( bool isUnsaved )
{
	m_statusbar->setUnsavedStatus ( isUnsaved );
}

void BNPView::setActive ( bool active )
{
//	std::cout << "Main Window Position: setActive(" << (active ? "true" : "false") << ")" << std::endl;
	KMainWindow* win = Global::mainWindow();
	if ( !win )
		return;

#if KDE_IS_VERSION( 3, 2, 90 )   // KDE 3.3.x
	/*FIXME 1.5	if (active) {
			kapp->updateUserTimestamp(); // If "activate on mouse hovering systray", or "on drag throught systray"
			Global::systemTray->setActive();   //  FIXME: add this in the places it need
		} else
			Global::systemTray->setInactive();*/
#elif KDE_IS_VERSION( 3, 1, 90 ) // KDE 3.2.x
	// Code from Kopete (that seem to work, in waiting KSystemTray make puplic the toggleSHown) :
	if ( active )
	{
		win->show();
		//raise() and show() should normaly deIconify the window. but it doesn't do here due
		// to a bug in Qt or in KDE  (qt3.1.x or KDE 3.1.x) then, i have to call KWindowSystem's method
		if ( win->isMinimized() )
			KWindowSystem::deIconifyWindow ( winId() );

		if ( ! KWindowSystem::windowInfo ( winId(), NET::WMDesktop ).onAllDesktops() )
			KWindowSystem::setOnDesktop ( winId(), KWindowSystem::currentDesktop() );
		win->raise();
		// Code from me: expected and correct behavviour:
		kapp->updateUserTimestamp(); // If "activate on mouse hovering systray", or "on drag throught systray"
		KWindowSystem::activateWindow ( win->winId() );
	}
	else
		win->hide();
#else                            // KDE 3.1.x and lower
	if ( win->active )
	{
		if ( win->isMinimized() )
			win->hide();        // If minimized, show() doesn't work !
		win->show();            // Show it
		//		showNormal();      // If it was minimized
		win->raise();           // Raise it on top
		win->setActiveWindow(); // And set it the active window
	}
	else
		win->hide();
#endif
}

void BNPView::hideOnEscape()
{
	if ( Settings::useSystray() )
		setActive ( false );
}

bool BNPView::isPart()
{
	/* FIXME 1.5	return (strcmp(name(), "BNPViewPart") == 0);*/
}

bool BNPView::isMainWindowActive()
{
	KMainWindow* main = Global::mainWindow();
	if ( main && main->isActiveWindow() )
		return true;
	return false;
}

void BNPView::newBasket()
{
	askNewBasket();
}

void BNPView::handleCommandLine()
{
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	/* Custom data folder */
	QString customDataFolder = args->getOption ( "data-folder" );
	if ( /*customDataFolder != 0 && */!customDataFolder.isEmpty() )
	{
		Global::setCustomSavesFolder ( customDataFolder );
	}

	/* Debug window */
	/*FIXME	if (args->isSet("debug")) {
			new DebugWindow();
			Global::debugWindow->show();
		}*/

	/* Crash Handler to Mail Developers when Crashing: */
	/*FIXME #ifndef BASKET_USE_DRKONQI
		if (!args->isSet("use-drkonquy"))
			KCrash::setCrashHandler(Crash::crashHandler);
	#endif*/
}

/** Scenario of "Hide main window to system tray icon when mouse move out of the window" :
 * - At enterEvent() we stop m_tryHideTimer
 * - After that and before next, we are SURE cursor is hovering window
 * - At leaveEvent() we restart m_tryHideTimer
 * - Every 'x' ms, timeoutTryHide() seek if cursor hover a widget of the application or not
 * - If yes, we musn't hide the window
 * - But if not, we start m_hideTimer to hide main window after a configured elapsed time
 * - timeoutTryHide() continue to be called and if cursor move again to one widget of the app, m_hideTimer is stopped
 * - If after the configured time cursor hasn't go back to a widget of the application, timeoutHide() is called
 * - It then hide the main window to systray icon
 * - When the user will show it, enterEvent() will be called the first time he enter mouse to it
 * - ...
 */

/** Why do as this ? Problems with the use of only enterEvent() and leaveEvent() :
 * - Resize window or hover titlebar isn't possible : leave/enterEvent
 *   are
 *   > Use the grip or Alt+rightDND to resize window
 *   > Use Alt+DND to move window
 * - Each menu trigger the leavEvent
 */

void BNPView::enterEvent ( QEvent* )
{
	if ( m_tryHideTimer )
		m_tryHideTimer->stop();
	if ( m_hideTimer )
		m_hideTimer->stop();
}

void BNPView::leaveEvent ( QEvent* )
{
	if ( Settings::useSystray() && Settings::hideOnMouseOut() && m_tryHideTimer )
		m_tryHideTimer->start ( 50 );
}

void BNPView::timeoutTryHide()
{
	// If a menu is displayed, do nothing for the moment
	if ( kapp->activePopupWidget() != 0L )
		return;

	if ( kapp->widgetAt ( QCursor::pos() ) != 0L )
		m_hideTimer->stop();
	else if ( ! m_hideTimer->isActive() )
	{
// Start only one time
		m_hideTimer->setSingleShot ( true );
		m_hideTimer->start ( Settings::timeToHideOnMouseOut() * 100 );
	}
	// If a sub-dialog is oppened, we musn't hide the main window:
	if ( kapp->activeWindow() != 0L && kapp->activeWindow() != Global::mainWindow() )
		m_hideTimer->stop();
}

void BNPView::timeoutHide()
{
	// We check that because the setting can have been set to off
	if ( Settings::useSystray() && Settings::hideOnMouseOut() )
		setActive ( false );
	m_tryHideTimer->stop();
}

void BNPView::changedSelectedNotes()
{
//	tabChanged(0); // FIXME: NOT OPTIMIZED
}

/*void BNPView::areSelectedNotesCheckedChanged(bool checked)
{
	m_actCheckNotes->setChecked(checked && currentBasket()->showCheckBoxes());
}*/

void BNPView::enableActions()
{
	Basket *basket = currentBasket();
	if ( !basket )
		return;
	if ( m_actLockBasket )
		m_actLockBasket->setEnabled ( !basket->isLocked() && basket->isEncrypted() );
	if ( m_actPassBasket )
		m_actPassBasket->setEnabled ( !basket->isLocked() );
	m_actPropBasket->setEnabled ( !basket->isLocked() );
	m_actDelBasket->setEnabled ( !basket->isLocked() );
	m_actExportToHtml->setEnabled ( !basket->isLocked() );
	/* FIXME 1.5	m_actShowFilter->setEnabled(!basket->isLocked());
		m_actFilterAllBaskets->setEnabled(!basket->isLocked());
		m_actResetFilter->setEnabled(!basket->isLocked());
		basket->decoration()->filterBar()->setEnabled(!basket->isLocked());*/
}

void BNPView::showMainWindow()
{
	KMainWindow *win = Global::mainWindow();

	if ( win )
	{
		win->show();
	}
	setActive ( true );
	emit showPart();
}

void BNPView::populateTagsMenu()
{
	KMenu *menu = ( KMenu* ) ( popupMenu ( "tags" ) );
	if ( menu == 0 || currentBasket() == 0 ) // TODO: Display a messagebox. [menu is 0, surely because on first launch, the XMLGUI does not work!]
		return;
	menu->clear();

	Note *referenceNote;
	if ( currentBasket()->focusedNote() && currentBasket()->focusedNote()->isSelected() )
		referenceNote = currentBasket()->focusedNote();
	else
		referenceNote = currentBasket()->firstSelected();

	populateTagsMenu ( *menu, referenceNote );

	m_lastOpenedTagsMenu = menu;
//	connect( menu, SIGNAL(aboutToHide()), this, SLOT(disconnectTagsMenu()) );
}

void BNPView::populateTagsMenu ( KMenu &menu, Note *referenceNote )
{
	if ( currentBasket() == 0 )
		return;

	currentBasket()->m_tagPopupNote = referenceNote;
	bool enable = currentBasket()->countSelecteds() > 0;

	QList<Tag*>::iterator it;
	Tag *currentTag;
	State *currentState;
	int i = 10;
	for ( it = Tag::all.begin(); it != Tag::all.end(); ++it )
	{
		// Current tag and first state of it:
		currentTag = *it;
		currentState = currentTag->states().first();
		QKeySequence sequence;
		/* FIXME 1.5		if (!currentTag->shortcut().isNull())
					sequence = currentTag->shortcut().operator QKeySequence();
				menu.insertItem(StateMenuItem::checkBoxIconSet(
					(referenceNote ? referenceNote->hasTag(currentTag) : false),
					menu.colorGroup()),
					new StateMenuItem(currentState, sequence, true),
					i
				);
				if (!currentTag->shortcut().isNull())
					menu.setAccel(sequence, i);
				menu.setItemEnabled(i, enable);*/
		++i;
	}

// FIXME 1.5	menu.insertSeparator();
// //	menu.insertItem( /*SmallIconSet("edit-delete"),*/ "&Assign new Tag...", 1 );
// 	//id = menu.insertItem( SmallIconSet("edit-delete"), "&Remove All", -2 );
// 	//if (referenceNote->states().isEmpty())
// 	//	menu.setItemEnabled(id, false);
// //	menu.insertItem( SmallIconSet("configure"),  "&Customize...", 3 );
// 	menu.insertItem( new IndentedMenuItem(i18n("&Assign new Tag...")),          1 );
// 	menu.insertItem( new IndentedMenuItem(i18n("&Remove All"),   "edit-delete"), 2 );
// 	menu.insertItem( new IndentedMenuItem(i18n("&Customize..."), "configure"),  3 );
//
// 	menu.setItemEnabled(1, enable);
// 	if (!currentBasket()->selectedNotesHaveTags())
// 		menu.setItemEnabled(2, false);

	connect ( &menu, SIGNAL ( activated ( int ) ), currentBasket(), SLOT ( toggledTagInMenu ( int ) ) );
	connect ( &menu, SIGNAL ( aboutToHide() ),  currentBasket(), SLOT ( unlockHovering() ) );
	connect ( &menu, SIGNAL ( aboutToHide() ),  currentBasket(), SLOT ( disableNextClick() ) );
}

void BNPView::connectTagsMenu()
{
	connect ( popupMenu ( "tags" ), SIGNAL ( aboutToShow() ), this, SLOT ( populateTagsMenu() ) );
	connect ( popupMenu ( "tags" ), SIGNAL ( aboutToHide() ), this, SLOT ( disconnectTagsMenu() ) );
}

/*
 * The Tags menu is ONLY created once the BasKet KPart is first shown.
 * So we can use this menu only from then?
 * When the KPart is changed in Kontact, and then the BasKet KPart is shown again,
 * Kontact created a NEW Tags menu. So we should connect again.
 * But when Kontact main window is hidden and then re-shown, the menu does not change.
 * So we disconnect at hide event to ensure only one connection: the next show event will not connects another time.
 */

void BNPView::showEvent ( QShowEvent* )
{
	if ( isPart() )
		QTimer::singleShot ( 0, this, SLOT ( connectTagsMenu() ) );

	if ( m_firstShow )
	{
		m_firstShow = false;
		onFirstShow();
	}
	if ( isPart() /*TODO: && !LikeBack::enabledBar()*/ )
	{
		Global::likeBack->enableBar();
	}
}

void BNPView::hideEvent ( QHideEvent* )
{
	if ( isPart() )
	{
		disconnect ( popupMenu ( "tags" ), SIGNAL ( aboutToShow() ), this, SLOT ( populateTagsMenu() ) );
		disconnect ( popupMenu ( "tags" ), SIGNAL ( aboutToHide() ), this, SLOT ( disconnectTagsMenu() ) );
	}

	if ( isPart() )
		Global::likeBack->disableBar();
}

void BNPView::disconnectTagsMenu()
{
	QTimer::singleShot ( 0, this, SLOT ( disconnectTagsMenuDelayed() ) );
}

void BNPView::disconnectTagsMenuDelayed()
{
	disconnect ( m_lastOpenedTagsMenu, SIGNAL ( activated ( int ) ), currentBasket(), SLOT ( toggledTagInMenu ( int ) ) );
	disconnect ( m_lastOpenedTagsMenu, SIGNAL ( aboutToHide() ),  currentBasket(), SLOT ( unlockHovering() ) );
	disconnect ( m_lastOpenedTagsMenu, SIGNAL ( aboutToHide() ),  currentBasket(), SLOT ( disableNextClick() ) );
}

void BNPView::showGlobalShortcutsSettingsDialog()
{
	/* FIXME 1.5	KShortcutsDialog::configure(Global::globalAccel);
		//.setCaption(..)
		Global::globalAccel->writeSettings();*/
}

#include "bnpview.moc"

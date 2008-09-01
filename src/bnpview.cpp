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

#include <QStackedWidget>
#include <qregexp.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qimage.h>
#include <qbitmap.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QShowEvent>
#include <Q3ValueList>
#include <QKeyEvent>
#include <QEvent>
#include <QHideEvent>
#include <kmenu.h>
#include <qsignalmapper.h>
#include <qdir.h>
#include <kicontheme.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstringhandler.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kprogressdialog.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>
#include <kwindowsystem.h>
#include <kpassivepopup.h>
#include <kxmlguifactory.h>
#include <kcmdlineargs.h>
#include <kglobalaccel.h>
#include <kapplication.h>
#include <KShortcutsDialog>
#include <kdebug.h>
#include <cstdlib>

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
#include "basketlistview.h"
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
#include "notefactory.h"
#include "notecontent.h"

#include <KAction>
#include <KActionMenu>
#include <KActionCollection>
#include <KStandardShortcut>
#include <KToggleAction>

#include "bnpviewadaptor.h"
/** class BNPView: */

const int BNPView::c_delayTooltipTime = 275;

BNPView::BNPView(QWidget *parent, const char *name, KXMLGUIClient *aGUIClient,
		 KActionCollection *actionCollection, BasketStatusBar *bar)
	: QSplitter(Qt::Horizontal, parent)
	, m_actLockBasket(0)
	, m_actPassBasket(0)
	, m_loading(true)
	, m_newBasketPopup(false)
	, m_firstShow(true)
	, m_regionGrabber(0)
	, m_passiveDroppedSelection(0)
	, m_passivePopup(0)
	, m_actionCollection(actionCollection)
	, m_guiClient(aGUIClient)
	, m_statusbar(bar)
	, m_tryHideTimer(0)
	, m_hideTimer(0)
{

	new BNPViewAdaptor(this);
	QDBusConnection dbus = QDBusConnection::sessionBus();
	dbus.registerObject("/BNPView",this);

	setObjectName(name);

	/* Settings */
	Settings::loadConfig();

	Global::bnpView = this;

	// Needed when loading the baskets:
	Global::backgroundManager = new BackgroundManager();

	setupGlobalShortcuts();
	initialize();
	QTimer::singleShot(0, this, SLOT(lateInit()));
}

BNPView::~BNPView()
{
    int treeWidth = Global::bnpView->sizes()[Settings::treeOnLeft() ? 0 : 1];

    Settings::setBasketTreeWidth(treeWidth);

    if (currentBasket() && currentBasket()->isDuringEdit())
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
	if(!isPart())
	{
		if (Settings::useSystray() && KCmdLineArgs::parsedArgs() && KCmdLineArgs::parsedArgs()->isSet("start-hidden"))
			if(Global::mainWindow()) Global::mainWindow()->hide();
		else if (Settings::useSystray() && kapp->isSessionRestored())
			if(Global::mainWindow()) Global::mainWindow()->setShown(!Settings::startDocked());
		else
			showMainWindow();
	}

	// If the main window is hidden when session is saved, Container::queryClose()
	//  isn't called and the last value would be kept
	Settings::setStartDocked(true);
	Settings::saveConfig();

	/* System tray icon */
	Global::systemTray = new SystemTray(Global::mainWindow());
	connect( Global::systemTray, SIGNAL(showPart()), this, SIGNAL(showPart()) );
	if (Settings::useSystray())
		Global::systemTray->show();

	// Load baskets
	DEBUG_WIN << "Baskets are loaded from " + Global::basketsFolder();

	NoteDrag::createAndEmptyCuttingTmpFolder(); // If last exec hasn't done it: clean the temporary folder we will use
	Tag::loadTags(); // Tags should be ready before loading baskets, but tags need the mainContainer to be ready to create KActions!
	load();

	// If no basket has been found, try to import from an older version,
	if (!firstListViewItem()) {
		QDir dir;
		dir.mkdir(Global::basketsFolder());
		if (FormatImporter::shouldImportBaskets()) {
			FormatImporter::importBaskets();
			load();
		}
		if (!firstListViewItem()) {
		// Create first basket:
			BasketFactory::newBasket(/*icon=*/"", /*name=*/i18n("General"), /*backgroundImage=*/"", /*backgroundColor=*/QColor(), /*textColor=*/QColor(), /*templateName=*/"1column", /*createIn=*/0);
		}
	}

	// Load the Welcome Baskets if it is the First Time:
	if (!Settings::welcomeBasketsAdded()) {
		addWelcomeBaskets();
		Settings::setWelcomeBasketsAdded(true);
		Settings::saveConfig();
	}

	m_tryHideTimer = new QTimer(this);
	m_hideTimer    = new QTimer(this);
	connect( m_tryHideTimer, SIGNAL(timeout()), this, SLOT(timeoutTryHide()) );
	connect( m_hideTimer,    SIGNAL(timeout()), this, SLOT(timeoutHide())    );

	// Preload every baskets for instant filtering:
/*StopWatch::start(100);
	QListViewItemIterator it(m_tree);
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
	if (QString(KGlobal::locale()->encoding()) == QString("UTF-8")) { // Welcome baskets are encoded in UTF-8. If the system is not, then use the English version:
		possiblePaths.append(KGlobal::dirs()->findResource("data", "basket/welcome/Welcome_" + KGlobal::locale()->language() + ".baskets"));
		possiblePaths.append(KGlobal::dirs()->findResource("data", "basket/welcome/Welcome_" + QStringList::split("_", KGlobal::locale()->language())[0] + ".baskets"));
	}
	possiblePaths.append(KGlobal::dirs()->findResource("data", "basket/welcome/Welcome_en_US.baskets"));

	// Take the first EXISTING basket archive found:
	QDir dir;
	QString path;
	for (QStringList::Iterator it = possiblePaths.begin(); it != possiblePaths.end(); ++it) {
		if (dir.exists(*it)) {
			path = *it;
			break;
		}
	}

	// Extract:
	if (!path.isEmpty())
		Archive::open(path);
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

	if (isPart())
		Global::likeBack->disableBar(); // See BNPView::shown() and BNPView::hide().

/*
	LikeBack::init(Global::config(), Global::about(), LikeBack::AllButtons);
	LikeBack::setServer("basket.linux62.org", "/likeback/send.php");
//	LikeBack::setServer("localhost", "/~seb/basket/likeback/send.php");
	LikeBack::setCustomLanguageMessage(i18n("Only english and french languages are accepted."));
//	LikeBack::setWindowNamesListing(LikeBack:: / *NoListing* / / *WarnUnnamedWindows* / AllWindows);
*/

	// In late init, because we need kapp->mainWidget() to be set!
	if (!isPart())
		connectTagsMenu();

	m_statusbar->setupStatusBar();

    int treeWidth = Settings::basketTreeWidth();
    if (treeWidth < 0)
      treeWidth = m_tree->fontMetrics().maxWidth() * 11;
    Q3ValueList<int> splitterSizes;
    splitterSizes.append(treeWidth);
    setSizes(splitterSizes);
}

void BNPView::setupGlobalShortcuts()
{
    KActionCollection *ac = new KActionCollection(this);
    KAction *a = NULL;

    // Ctrl+Shift+W only works when started standalone:
    QWidget *basketMainWindow =
	    qobject_cast<KMainWindow *>(Global::bnpView->parent());

    int modifier = Qt::CTRL + Qt::ALT + Qt::SHIFT;

    if (basketMainWindow) {
	a = ac->addAction("global_show_hide_main_window", Global::systemTray,
			  SLOT(toggleActive()));
	a->setText(i18n("Show/hide main window"));
	a->setStatusTip(
	    i18n("Allows you to show main Window if it is hidden, and to hide "
		 "it if it is shown.") );
	a->setGlobalShortcut(KShortcut(modifier + Qt::Key_W));
    }

    a = ac->addAction("global_paste", Global::bnpView,
		      SLOT(globalPasteInCurrentBasket()));
    a->setText(i18n("Paste clipboard contents in current basket"));
    a->setStatusTip(
	i18n("Allows you to paste clipboard contents in the current basket "
	     "without having to open the main window.") );
    a->setGlobalShortcut(KShortcut(modifier + Qt::Key_V));



    a = ac->addAction("global_show_current_basket", Global::bnpView,
		      SLOT(showPassiveContentForced()));
    a->setText(i18n("Show current basket name"));
    a->setStatusTip(i18n("Allows you to know basket is current without opening "
			 "the main window."));


    a = ac->addAction("global_paste_selection", Global::bnpView,
		      SLOT(pasteSelInCurrentBasket()));
    a->setText(i18n("Paste selection in current basket"));
    a->setStatusTip(
	i18n("Allows you to paste clipboard selection in the current basket "
	     "without having to open the main window.") );
    a->setGlobalShortcut(KShortcut(Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_S));

    a = ac->addAction("global_new_basket", Global::bnpView,
		      SLOT(askNewBasket()));
    a->setText(i18n("Create a new basket"));
    a->setStatusTip(
	i18n("Allows you to create a new basket without having to open the "
	     "main window (you then can use the other global shortcuts to add "
	     "a note, paste clipboard or paste selection in this new basket).")
	);

    a = ac->addAction("global_previous_basket", Global::bnpView,
		      SLOT(goToPreviousBasket()));
    a->setText(i18n("Go to previous basket"));
    a->setStatusTip(
	i18n("Allows you to change current basket to the previous one without "
	     "having to open the main window.") );

    a = ac->addAction("global_next_basket", Global::bnpView,
		      SLOT(goToNextBasket()));
    a->setText(i18n("Go to next basket"));
    a->setStatusTip(i18n("Allows you to change current basket to the next one "
			 "without having to open the main window."));

    a = ac->addAction("global_note_add_html", Global::bnpView,
		      SLOT(addNoteHtml()));
    a->setText(i18n("Insert text note"));
    a->setStatusTip(
	i18n("Add a text note to the current basket without having to open "
	     "the main window.") );
    a->setGlobalShortcut(KShortcut(modifier + Qt::Key_T));

    a = ac->addAction("global_note_add_image", Global::bnpView,
		      SLOT(addNoteImage()));
    a->setText(i18n("Insert image note"));
    a->setStatusTip(
	i18n("Add an image note to the current basket without having to open "
	     "the main window.") );

    a = ac->addAction("global_note_add_link", Global::bnpView,
		      SLOT(addNoteLink()));
    a->setText(i18n("Insert link note"));
    a->setStatusTip(
	i18n("Add a link note to the current basket without having "
	     "to open the main window."));

    a = ac->addAction("global_note_add_color", Global::bnpView,
		      SLOT(addNoteColor()));
    a->setText(i18n("Insert color note"));
    a->setStatusTip(
	i18n("Add a color note to the current basket without having to open "
	     "the main window."));

    a = ac->addAction("global_note_pick_color", Global::bnpView,
		      SLOT(slotColorFromScreenGlobal()));
    a->setText(i18n("Pick color from screen"));
    a->setStatusTip(
	i18n("Add a color note picked from one pixel on screen to the current "
	     "basket without " "having to open the main window.") );

    a = ac->addAction("global_note_grab_screenshot", Global::bnpView,
		      SLOT(grabScreenshotGlobal()));
    a->setText(i18n("Grab screen zone"));
    a->setStatusTip(
	i18n("Grab a screen zone as an image in the current basket without "
	     "having to open the main window."));

#if 0
    a = ac->addAction("global_note_add_text", Global::bnpView,
		      SLOT(addNoteText()));
    a->setText(i18n("Insert plain text note"));
    a->setStatusTip(
	    i18n("Add a plain text note to the current basket without having to "
		 "open the main window."));
#endif
}

void BNPView::initialize()
{
	/// Configure the List View Columns:
	m_tree  = new BasketTreeListView(this);
	m_tree->addColumn(i18n("Baskets"));
	m_tree->setColumnWidthMode(0, Q3ListView::Maximum);
	m_tree->setFullWidth(true);
	m_tree->setSorting(-1/*Disabled*/);
	m_tree->setRootIsDecorated(true);
	m_tree->setTreeStepSize(16);
	m_tree->setLineWidth(1);
	m_tree->setMidLineWidth(0);
	m_tree->setFocusPolicy(Qt::NoFocus);

	/// Configure the List View Drag and Drop:
	m_tree->setDragEnabled(true);
	m_tree->setAcceptDrops(true);
	m_tree->setItemsMovable(true);
	m_tree->setDragAutoScroll(true);
	m_tree->setDropVisualizer(true);
	m_tree->setDropHighlighter(true);

	/// Configure the Splitter:
	m_stack = new QStackedWidget(this);

	setOpaqueResize(true);

	setCollapsible(m_tree,  true);
	setCollapsible(m_stack, false);
	setResizeMode(m_tree,  QSplitter::KeepSize);
	setResizeMode(m_stack, QSplitter::Stretch);

	/// Configure the List View Signals:
	connect( m_tree, SIGNAL(returnPressed(Q3ListViewItem*)),    this, SLOT(slotPressed(Q3ListViewItem*)) );
	connect( m_tree, SIGNAL(selectionChanged(Q3ListViewItem*)), this, SLOT(slotPressed(Q3ListViewItem*)) );
	connect( m_tree, SIGNAL(pressed(Q3ListViewItem*)),          this, SLOT(slotPressed(Q3ListViewItem*)) );
	connect( m_tree, SIGNAL(expanded(Q3ListViewItem*)),         this, SLOT(needSave(Q3ListViewItem*))    );
	connect( m_tree, SIGNAL(collapsed(Q3ListViewItem*)),        this, SLOT(needSave(Q3ListViewItem*))    );
	connect( m_tree, SIGNAL(contextMenu(K3ListView*, Q3ListViewItem*, const QPoint&)),      this, SLOT(slotContextMenu(K3ListView*, Q3ListViewItem*, const QPoint&))      );
	connect( m_tree, SIGNAL(mouseButtonPressed(int, Q3ListViewItem*, const QPoint&, int)), this, SLOT(slotMouseButtonPressed(int, Q3ListViewItem*, const QPoint&, int)) );
	connect( m_tree, SIGNAL(doubleClicked(Q3ListViewItem*, const QPoint&, int)), this, SLOT(slotShowProperties(Q3ListViewItem*, const QPoint&, int)) );

	connect( m_tree, SIGNAL(expanded(Q3ListViewItem*)),  this, SIGNAL(basketChanged()) );
	connect( m_tree, SIGNAL(collapsed(Q3ListViewItem*)), this, SIGNAL(basketChanged()) );
	connect( this,   SIGNAL(basketNumberChanged(int)),  this, SIGNAL(basketChanged()) );

	connect( this, SIGNAL(basketNumberChanged(int)), this, SLOT(slotBasketNumberChanged(int)) );
	connect( this, SIGNAL(basketChanged()),          this, SLOT(slotBasketChanged())          );

	/* LikeBack */
	Global::likeBack = new LikeBack(LikeBack::AllButtons, /*showBarByDefault=*/false, Global::config(), Global::about());
	Global::likeBack->setServer("basket.linux62.org", "/likeback/send.php");

// There are too much comments, and people reading comments are more and more international, so we accept only English:
//	Global::likeBack->setAcceptedLanguages(QStringList::split(";", "en;fr"), i18n("Please write in English or French."));

//	if (isPart())
//		Global::likeBack->disableBar(); // See BNPView::shown() and BNPView::hide().

	Global::likeBack->sendACommentAction(actionCollection()); // Just create it!
	setupActions();

	/// What's This Help for the tree:
	Q3WhatsThis::add(m_tree, i18n(
			"<h2>Basket Tree</h2>"
					"Here is the list of your baskets. "
					"You can organize your data by putting them in different baskets. "
					"You can group baskets by subject by creating new baskets inside others. "
					"You can browse between them by clicking a basket to open it, or reorganize them using drag and drop."));

	setTreePlacement(Settings::treeOnLeft());
}

void BNPView::setupActions()
{
    KAction *a = NULL;
    KActionCollection *ac = actionCollection();

    a = ac->addAction("basket_export_basket_archive", this,
		      SLOT(saveAsArchive()));
    a->setText(i18n("&Basket Archive..."));
    a->setIcon(KIcon("baskets"));
    a->setShortcut(0);
    m_actSaveAsArchive = a;

    a = ac->addAction("basket_import_basket_archive", this,
		      SLOT(openArchive()));
    a->setText(i18n("&Basket Archive..."));
    a->setIcon(KIcon("baskets"));
    a->setShortcut(0);
    m_actOpenArchive = a;

    a = ac->addAction("window_hide", this, SLOT(hideOnEscape()));
    a->setText(i18n("&Hide Window"));
    a->setShortcut(KStandardShortcut::shortcut(KStandardShortcut::Close));
    m_actHideWindow = a;

    m_actHideWindow->setEnabled(Settings::useSystray()); // Init here !

    a = ac->addAction("basket_export_html", this, SLOT(exportToHTML()));
    a->setText(i18n("&HTML Web Page..."));
    a->setIcon(KIcon("text-html"));
    a->setShortcut(0);
    m_actExportToHtml = a;

    a = ac->addAction("basket_import_knotes", this, SLOT(importKNotes()));
    a->setText(i18n("K&Notes"));
    a->setIcon(KIcon("knotes"));
    a->setShortcut(0);

    a = ac->addAction("basket_import_kjots", this, SLOT(importKJots()));
    a->setText(i18n("K&Jots"));
    a->setIcon(KIcon("kjots"));
    a->setShortcut(0);

    a = ac->addAction("basket_import_knowit", this, SLOT(importKnowIt()));
    a->setText(i18n("&KnowIt..."));
    a->setIcon(KIcon("knowit"));
    a->setShortcut(0);

    a = ac->addAction("basket_import_tuxcards", this, SLOT(importTuxCards()));
    a->setText(i18n("Tux&Cards..."));
    a->setIcon(KIcon("tuxcards"));
    a->setShortcut(0);

    a = ac->addAction("basket_import_sticky_notes", this,
		      SLOT(importStickyNotes()));
    a->setText(i18n("&Sticky Notes"));
    a->setIcon(KIcon("gnome"));
    a->setShortcut(0);

    a = ac->addAction("basket_import_tomboy", this, SLOT(importTomboy()));
    a->setText(i18n("&Tomboy"));
    a->setIcon(KIcon("tintin"));
    a->setShortcut(0);

    a = ac->addAction("basket_import_text_file", this, SLOT(importTextFile()));
    a->setText(i18n("Text &File..."));
    a->setIcon(KIcon("text-plain"));
    a->setShortcut(0);

    a = ac->addAction("basket_backup_restore", this, SLOT(backupRestore()));
    a->setText(i18n("&Backup && Restore..."));
    a->setShortcut(0);

    /** Note : ****************************************************************/

    a = ac->addAction("edit_delete", this, SLOT(delNote()));
    a->setText(i18n("D&elete"));
    a->setIcon(KIcon("edit-delete"));
    a->setShortcut(KShortcut("Delete"));
    m_actDelNote = a;

    m_actCutNote  = ac->addAction(KStandardAction::Cut, this, SLOT(cutNote()));
    m_actCopyNote = ac->addAction(KStandardAction::Copy, this, SLOT(copyNote()));

    m_actSelectAll = ac->addAction(KStandardAction::SelectAll, this,
				   SLOT(slotSelectAll()));
    m_actSelectAll->setStatusTip( i18n( "Selects all notes" ) );

    a = ac->addAction("edit_unselect_all", this, SLOT( slotUnselectAll() ));
    a->setText(i18n( "U&nselect All" ));
    m_actUnselectAll = a;
    m_actUnselectAll->setStatusTip( i18n( "Unselects all selected notes" ) );

    a = ac->addAction("edit_invert_selection", this,
		      SLOT( slotInvertSelection() ));
    a->setText(i18n( "&Invert Selection" ));
    a->setShortcut(Qt::CTRL+Qt::Key_Asterisk);
    m_actInvertSelection = a;

    m_actInvertSelection->setStatusTip(
	    i18n( "Inverts the current selection of notes" )
	    );

    a = ac->addAction("note_edit", this, SLOT(editNote()));
    a->setText(i18nc("Verb; not Menu", "&Edit..."));
    a->setIcon(KIcon("edit"));
    a->setShortcut(KShortcut("Return"));
    m_actEditNote = a;

    m_actOpenNote = ac->addAction(KStandardAction::Open, "note_open",
				  this, SLOT(openNote()));
    m_actOpenNote->setIcon(KIcon("window-new"));
    m_actOpenNote->setText(i18n("&Open"));
    m_actOpenNote->setShortcut(KShortcut("F9"));

    a = ac->addAction("note_open_with", this, SLOT(openNoteWith()));
    a->setText(i18n("Open &With..."));
    a->setShortcut(KShortcut("Shift+F9"));
    m_actOpenNoteWith = a;

    m_actSaveNoteAs = ac->addAction(KStandardAction::SaveAs,
				    "note_save_to_file",
				    this, SLOT(saveNoteAs()));
    m_actSaveNoteAs->setText(i18n("&Save to File..."));
    m_actSaveNoteAs->setShortcut(KShortcut("F10"));

    a = ac->addAction("note_group", this, SLOT(noteGroup()));
    a->setText(i18n("&Group"));
    a->setIcon(KIcon("attach"));
    a->setShortcut(KShortcut("Ctrl+G"));
    m_actGroup = a;

    a = ac->addAction("note_ungroup", this, SLOT(noteUngroup()));
    a->setText(i18n("U&ngroup"));
    a->setShortcut(KShortcut("Ctrl+Shift+G"));
    m_actUngroup = a;

    a = ac->addAction("note_move_top", this, SLOT(moveOnTop()));
    a->setText(i18n("Move on &Top"));
    a->setIcon(KIcon("arrow-up-double"));
    a->setShortcut(KShortcut("Ctrl+Shift+Home"));
    m_actMoveOnTop = a;

    a = ac->addAction("note_move_up", this, SLOT(moveNoteUp()));
    a->setText(i18n("Move &Up"));
    a->setIcon(KIcon("arrow-up"));
    a->setShortcut(KShortcut("Ctrl+Shift+Up"));
    m_actMoveNoteUp = a;

    a = ac->addAction("note_move_down", this, SLOT(moveNoteDown()));
    a->setText(i18n("Move &Down"));
    a->setIcon(KIcon("arrow-down"));
    a->setShortcut(KShortcut("Ctrl+Shift+Down"));
    m_actMoveNoteDown = a;

    a = ac->addAction("note_move_bottom", this, SLOT(moveOnBottom()));
    a->setText(i18n("Move on &Bottom"));
    a->setIcon(KIcon("arrow-down-double"));
    a->setShortcut(KShortcut("Ctrl+Shift+End"));
    m_actMoveOnBottom = a;

    m_actPaste = ac->addAction(KStandardAction::Paste, this,
			       SLOT(pasteInCurrentBasket()));

    /** Insert : **************************************************************/

    QSignalMapper *insertEmptyMapper  = new QSignalMapper(this);
    QSignalMapper *insertWizardMapper = new QSignalMapper(this);
    connect( insertEmptyMapper,  SIGNAL(mapped(int)), this, SLOT(insertEmpty(int))  );
    connect( insertWizardMapper, SIGNAL(mapped(int)), this, SLOT(insertWizard(int)) );

#if 0
    a = ac->addAction("insert_text");
    a->setText(i18n("Plai&n Text"));
    a->setIcon(KIcon("text"));
    a->setShortcut(KShortcut("Ctrl+T"));
    m_actInsertText = a;
#endif

    a = ac->addAction("insert_html");
    a->setText(i18n("&Text"));
    a->setIcon(KIcon("html"));
    a->setShortcut(KShortcut("Insert"));
    m_actInsertHtml = a;

    a = ac->addAction("insert_link");
    a->setText(i18n("&Link"));
    a->setIcon(KIcon("link"));
    a->setShortcut(KShortcut("Ctrl+Y"));
    m_actInsertLink = a;

    a = ac->addAction("insert_image");
    a->setText(i18n("&Image"));
    a->setIcon(KIcon("image"));
    m_actInsertImage = a;

    a = ac->addAction("insert_color");
    a->setText(i18n("&Color"));
    a->setIcon(KIcon("colorset"));
    m_actInsertColor = a;

    a = ac->addAction("insert_launcher");
    a->setText(i18n("L&auncher"));
    a->setIcon(KIcon("launch"));
    m_actInsertLauncher = a;

    a = ac->addAction("insert_kmenu");
    a->setText(i18n("Import Launcher from &KDE Menu..."));
    a->setIcon(KIcon("kmenu"));
    m_actImportKMenu = a;

    a = ac->addAction("insert_icon");
    a->setText(i18n("Im&port Icon..."));
    a->setIcon(KIcon("icons"));
    m_actImportIcon = a;

    a = ac->addAction("insert_from_file");
    a->setText(i18n("Load From &File..."));
    a->setIcon(KIcon("document-import"));
    m_actLoadFile = a;

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

    a = ac->addAction("insert_screen_color", this, SLOT(slotColorFromScreen()));
    a->setText(i18n("C&olor from Screen"));
    a->setIcon(KIcon("kcolorchooser"));
    m_actColorPicker = a;

    connect(m_colorPicker, SIGNAL(pickedColor(const QColor&)),
	    this, SLOT(colorPicked(const QColor&)));
    connect(m_colorPicker, SIGNAL(canceledPick()),
	    this, SLOT(colorPickingCanceled()));

    a = ac->addAction("insert_screen_capture", this, SLOT(grabScreenshot()));
    a->setText(i18n("Grab Screen &Zone"));
    a->setIcon(KIcon("ksnapshot"));
    m_actGrabScreenshot = a;

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

    /** Basket : **************************************************************/

    // At this stage, main.cpp has not set kapp->mainWidget(), so Global::runInsideKontact()
    // returns true. We do it ourself:
    bool runInsideKontact = true;
    QWidget *parentWidget = (QWidget*) parent();
    while (parentWidget) {
	    if (parentWidget->inherits("MainWindow"))
		    runInsideKontact = false;
	    parentWidget = (QWidget*) parentWidget->parent();
    }

    // Use the "basket" incon in Kontact so it is consistent with the Kontact "New..." icon

    a = ac->addAction("basket_new", this, SLOT(askNewBasket()));
    a->setText(i18n("&New Basket..."));
    a->setIcon(KIcon((runInsideKontact ? "basket" : "document-new")));
    a->setShortcut(KStandardShortcut::shortcut(KStandardShortcut::New));
    actNewBasket = a;

    a = ac->addAction("basket_new_sub", this, SLOT(askNewSubBasket()));
    a->setText(i18n("New &Sub-Basket..."));
    a->setShortcut(KShortcut("Ctrl+Shift+N"));
    actNewSubBasket = a;

    a = ac->addAction("basket_new_sibling", this, SLOT(askNewSiblingBasket()));
    a->setText(i18n("New Si&bling Basket..."));
    actNewSiblingBasket = a;

    KActionMenu *newBasketMenu = new KActionMenu(i18n("&New"), ac);
    newBasketMenu->setIcon(KIcon("document-new"));
    ac->addAction("basket_new_menu", newBasketMenu);

    newBasketMenu->addAction(actNewBasket);
    newBasketMenu->addAction(actNewSubBasket);
    newBasketMenu->addAction(actNewSiblingBasket);
    connect( newBasketMenu, SIGNAL(activated()), this, SLOT(askNewBasket()) );

    a = ac->addAction("basket_properties", this, SLOT(propBasket()));
    a->setText(i18n("&Properties..."));
    a->setIcon(KIcon("misc"));
    a->setShortcut(KShortcut("F2"));
    m_actPropBasket = a;

    a = ac->addAction("basket_remove", this, SLOT(delBasket()));
    a->setText(i18nc("Remove Basket", "&Remove"));
    a->setShortcut(0);
    m_actDelBasket = a;

#ifdef HAVE_LIBGPGME
    a = ac->addAction("basket_password", this, SLOT(password()));
    a->setText(i18nc("Password protection", "Pass&word..."));
    a->setShortcut(0);
    m_actPassBasket = a;

    a = ac->addAction("basket_lock", this, SLOT(lockBasket()));
    a->setText(i18nc("Lock Basket", "&Lock"));
    a->setShortcut(KShortcut("Ctrl+L"));
    m_actLockBasket = a;
#endif

    /** Edit : ****************************************************************/

    //m_actUndo     = KStandardAction::undo(  this, SLOT(undo()),                 actionCollection() );
    //m_actUndo->setEnabled(false); // Not yet implemented !
    //m_actRedo     = KStandardAction::redo(  this, SLOT(redo()),                 actionCollection() );
    //m_actRedo->setEnabled(false); // Not yet implemented !

    KToggleAction *toggleAct = NULL;
    toggleAct = new KToggleAction(i18n("&Filter"), ac);
    ac->addAction("edit_filter", toggleAct);
    toggleAct->setIcon(KIcon("view-filter"));
    toggleAct->setShortcut(KStandardShortcut::shortcut(KStandardShortcut::Find));
    m_actShowFilter = toggleAct;

    connect(m_actShowFilter, SIGNAL(toggled(bool)),
	    this, SLOT(showHideFilterBar(bool)) );

    toggleAct = new KToggleAction(ac);
    ac->addAction("edit_filter_all_baskets", toggleAct);
    toggleAct->setText(i18n("Filter all &Baskets"));
    toggleAct->setIcon(KIcon("edit-find"));
    toggleAct->setShortcut(KShortcut("Ctrl+Shift+F"));
    m_actFilterAllBaskets = toggleAct;

    connect(m_actFilterAllBaskets, SIGNAL(toggled(bool)),
	    this, SLOT(toggleFilterAllBaskets(bool)));

    a = ac->addAction("edit_filter_reset", this, SLOT( slotResetFilter() ));
    a->setText(i18n( "&Reset Filter" ));
    a->setIcon(KIcon("edit-clear-locationbar-rtl"));
    a->setShortcut(KShortcut("Ctrl+R"));
    m_actResetFilter = a;

    /** Go : ******************************************************************/

    a = ac->addAction("go_basket_previous", this, SLOT(goToPreviousBasket()));
    a->setText(i18n( "&Previous Basket" ));
    a->setIcon(KIcon("go-up"));
    a->setShortcut(KShortcut("Alt+Up"));
    m_actPreviousBasket = a;

    a = ac->addAction("go_basket_next", this, SLOT(goToNextBasket()));
    a->setText(i18n( "&Next Basket" ));
    a->setIcon(KIcon("go-down"));
    a->setShortcut(KShortcut("Alt+Down"));
    m_actNextBasket = a;

    a = ac->addAction("go_basket_fold", this, SLOT(foldBasket()));
    a->setText(i18n( "&Fold Basket" ));
    a->setIcon(KIcon("go-previous"));
    a->setShortcut(KShortcut("Alt+Left"));
    m_actFoldBasket = a;

    a = ac->addAction("go_basket_expand", this, SLOT(expandBasket()));
    a->setText(i18n( "&Expand Basket" ));
    a->setIcon(KIcon("go-next"));
    a->setShortcut(KShortcut("Alt+Right"));
    m_actExpandBasket = a;

#if 0
   // FOR_BETA_PURPOSE:
    a = ac->addAction("beta_convert_texts", this, SLOT(convertTexts()));
    a->setText(i18n("Convert text notes to rich text notes"));
    a->setIcon(KIcon("run-build-file"));
    m_convertTexts = a;
#endif

    InlineEditors::instance()->initToolBars(actionCollection());
    /** Help : ****************************************************************/

    a = ac->addAction("help_welcome_baskets", this, SLOT(addWelcomeBaskets()));
    a->setText(i18n("&Welcome Baskets"));
}

Q3ListViewItem* BNPView::firstListViewItem()
{
	return m_tree->firstChild();
}

void BNPView::slotShowProperties(Q3ListViewItem *item, const QPoint&, int)
{
	if (item)
		propBasket();
}

void BNPView::slotMouseButtonPressed(int button, Q3ListViewItem *item, const QPoint &/*pos*/, int /*column*/)
{
	if (item && (button & Qt::MidButton)) {
		// TODO: Paste into ((BasketListViewItem*)listViewItem)->basket()
	}
}

void BNPView::slotContextMenu(K3ListView */*listView*/, Q3ListViewItem *item, const QPoint &pos)
{
	QString menuName;
	if (item) {
		Basket* basket = ((BasketListViewItem*)item)->basket();

		setCurrentBasket(basket);
		menuName = "basket_popup";
	} else {
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

	KMenu *menu = popupMenu(menuName);
	connect( menu, SIGNAL(aboutToHide()),  this, SLOT(aboutToHideNewBasketPopup()) );
	menu->exec(pos);
}

void BNPView::save()
{
	DEBUG_WIN << "Basket Tree: Saving...";

	// Create Document:
	QDomDocument document("basketTree");
	QDomElement root = document.createElement("basketTree");
	document.appendChild(root);

	// Save Basket Tree:
	save(m_tree->firstChild(), document, root);

	// Write to Disk:
	Basket::safelySaveToFile(Global::basketsFolder() + "baskets.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n" + document.toString());
// 	QFile file(Global::basketsFolder() + "baskets.xml");
// 	if (file.open(QIODevice::WriteOnly)) {
// 		QTextStream stream(&file);
// 		stream.setEncoding(QTextStream::UnicodeUTF8);
// 		QString xml = document.toString();
// 		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
// 		stream << xml;
// 		file.close();
// 	}
}

void BNPView::save(Q3ListViewItem *firstItem, QDomDocument &document, QDomElement &parentElement)
{
	Q3ListViewItem *item = firstItem;
	while (item) {
//		Basket *basket = ((BasketListViewItem*)item)->basket();
		QDomElement basketElement = this->basketElement(item, document, parentElement);
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
		if (item->firstChild())
			save(item->firstChild(), document, basketElement);
		// Next Basket:
		item = item->nextSibling();
	}
}

QDomElement BNPView::basketElement(Q3ListViewItem *item, QDomDocument &document, QDomElement &parentElement)
{
	Basket *basket = ((BasketListViewItem*)item)->basket();
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
	return basketElement;
}

void BNPView::saveSubHierarchy(Q3ListViewItem *item, QDomDocument &document, QDomElement &parentElement, bool recursive)
{
	QDomElement element = basketElement(item, document, parentElement);
	if (recursive && item->firstChild())
		save(item->firstChild(), document, element);
}

void BNPView::load()
{
	QDomDocument *doc = XMLWork::openFile("basketTree", Global::basketsFolder() + "baskets.xml");
	//BEGIN Compatibility with 0.6.0 Pre-Alpha versions:
	if (!doc)
		doc = XMLWork::openFile("basketsTree", Global::basketsFolder() + "baskets.xml");
	//END
	if (doc != 0) {
		QDomElement docElem = doc->documentElement();
		load(m_tree, 0L, docElem);
	}
	m_loading = false;
}

void BNPView::load(K3ListView */*listView*/, Q3ListViewItem *item, const QDomElement &baskets)
{
	QDomNode n = baskets.firstChild();
	while ( ! n.isNull() ) {
		QDomElement element = n.toElement();
		if ( (!element.isNull()) && element.tagName() == "basket" ) {
			QString folderName = element.attribute("folderName");
			if (!folderName.isEmpty()) {
				Basket *basket = loadBasket(folderName);
				BasketListViewItem *basketItem = appendBasket(basket, item);
				basketItem->setOpen(!XMLWork::trueOrFalse(element.attribute("folded", "false"), false));
				basket->loadProperties(XMLWork::getElement(element, "properties"));
				if (XMLWork::trueOrFalse(element.attribute("lastOpened", element.attribute("lastOpenned", "false")), false)) // Compat with 0.6.0-Alphas
					setCurrentBasket(basket);
				// Load Sub-baskets:
				load(/*(QListView*)*/0L, basketItem, element);
			}
		}
		n = n.nextSibling();
	}
}

Basket* BNPView::loadBasket(const QString &folderName)
{
	if (folderName.isEmpty())
		return 0;

	DecoratedBasket *decoBasket = new DecoratedBasket(m_stack, folderName);
	Basket          *basket     = decoBasket->basket();
	m_stack->addWidget(decoBasket);
	connect( basket, SIGNAL(countsChanged(Basket*)), this, SLOT(countsChanged(Basket*)) );
	// Important: Create listViewItem and connect signal BEFORE loadProperties(), so we get the listViewItem updated without extra work:
	connect( basket, SIGNAL(propertiesChanged(Basket*)), this, SLOT(updateBasketListViewItem(Basket*)) );

	connect( basket->decoration()->filterBar(), SIGNAL(newFilter(const FilterData&)), this, SLOT(newFilterFromFilterBar()) );

	return basket;
}

int BNPView::basketCount(Q3ListViewItem *parent)
{
	int count = 0;

	Q3ListViewItem *item = (parent ? parent->firstChild() : m_tree->firstChild());
	while (item) {
		count += 1 + basketCount(item);
		item = item->nextSibling();
	}

	return count;
}

bool BNPView::canFold()
{
	BasketListViewItem *item = listViewItemForBasket(currentBasket());
	if (!item)
		return false;
	return item->parent() || (item->firstChild() && item->isOpen());
}

bool BNPView::canExpand()
{
	BasketListViewItem *item = listViewItemForBasket(currentBasket());
	if (!item)
		return false;
	return item->firstChild();
}

BasketListViewItem* BNPView::appendBasket(Basket *basket, Q3ListViewItem *parentItem)
{
	BasketListViewItem *newBasketItem;
	if (parentItem)
		newBasketItem = new BasketListViewItem(parentItem, ((BasketListViewItem*)parentItem)->lastChild(), basket);
	else {
		Q3ListViewItem *child     = m_tree->firstChild();
		Q3ListViewItem *lastChild = 0;
		while (child) {
			lastChild = child;
			child = child->nextSibling();
		}
		newBasketItem = new BasketListViewItem(m_tree, lastChild, basket);
	}

	emit basketNumberChanged(basketCount());

	return newBasketItem;
}

void BNPView::loadNewBasket(const QString &folderName, const QDomElement &properties, Basket *parent)
{
	Basket *basket = loadBasket(folderName);
	appendBasket(basket, (basket ? listViewItemForBasket(parent) : 0));
	basket->loadProperties(properties);
	setCurrentBasket(basket);
//	save();
}

BasketListViewItem* BNPView::lastListViewItem()
{
	Q3ListViewItem *child     = m_tree->firstChild();
	Q3ListViewItem *lastChild = 0;
	// Set lastChild to the last primary child of the list view:
	while (child) {
		lastChild = child;
		child = child->nextSibling();
	}
	// If this child have child(s), recursivly browse through them to find the real last one:
	while (lastChild && lastChild->firstChild()) {
		child = lastChild->firstChild();
		while (child) {
			lastChild = child;
			child = child->nextSibling();
		}
	}
	return (BasketListViewItem*)lastChild;
}

void BNPView::goToPreviousBasket()
{
	if (!m_tree->firstChild())
		return;

	BasketListViewItem *item     = listViewItemForBasket(currentBasket());
	BasketListViewItem *toSwitch = item->shownItemAbove();
	if (!toSwitch) {
		toSwitch = lastListViewItem();
		if (toSwitch && !toSwitch->isShown())
			toSwitch = toSwitch->shownItemAbove();
	}

	if (toSwitch)
		setCurrentBasket(toSwitch->basket());

	if (Settings::usePassivePopup())
		showPassiveContent();
}

void BNPView::goToNextBasket()
{
	if (!m_tree->firstChild())
		return;

	BasketListViewItem *item     = listViewItemForBasket(currentBasket());
	BasketListViewItem *toSwitch = item->shownItemBelow();
	if (!toSwitch)
		toSwitch = ((BasketListViewItem*)m_tree->firstChild());

	if (toSwitch)
		setCurrentBasket(toSwitch->basket());

	if (Settings::usePassivePopup())
		showPassiveContent();
}

void BNPView::foldBasket()
{
	BasketListViewItem *item = listViewItemForBasket(currentBasket());
	if (item && !item->firstChild())
		item->setOpen(false); // If Alt+Left is hitted and there is nothing to close, make sure the focus will go to the parent basket

	QKeyEvent* keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, 0, 0);
	QApplication::postEvent(m_tree, keyEvent);
}

void BNPView::expandBasket()
{
	QKeyEvent* keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, 0, 0);
	QApplication::postEvent(m_tree, keyEvent);
}

void BNPView::closeAllEditors()
{
	Q3ListViewItemIterator it(m_tree);
	while (it.current()) {
		BasketListViewItem *item = (BasketListViewItem*)(it.current());
		item->basket()->closeEditor();
		++it;
	}
}

bool BNPView::convertTexts()
{
	bool convertedNotes = false;
	KProgressDialog dialog(
			/*parent=*/0,
			/*caption=*/i18n("Plain Text Notes Conversion"),
			/*text=*/i18n("Converting plain text notes to rich text ones...")
		);
	dialog.setModal(true);
	dialog.progressBar()->setRange(0, basketCount());
	dialog.show(); //setMinimumDuration(50/*ms*/);

	Q3ListViewItemIterator it(m_tree);
	while (it.current()) {
		BasketListViewItem *item = (BasketListViewItem*)(it.current());
		if (item->basket()->convertTexts())
			convertedNotes = true;

		QProgressBar *pb = dialog.progressBar();
		pb->setValue(pb->value() + 1);

		if (dialog.wasCancelled())
			break;
		++it;
	}

	return convertedNotes;
}

void BNPView::toggleFilterAllBaskets(bool doFilter)
{
	// Set the state:
	m_actFilterAllBaskets->setChecked(doFilter);

	// If the filter isn't already showing, we make sure it does.
	if (doFilter)
		m_actShowFilter->setChecked(true);

	//currentBasket()->decoration()->filterBar()->setFilterAll(doFilter);

//	Basket *current = currentBasket();
	Q3ListViewItemIterator it(m_tree);
	while (it.current()) {
		BasketListViewItem *item = ((BasketListViewItem*)it.current());
		item->basket()->decoration()->filterBar()->setFilterAll(doFilter);
		++it;
	}

	if (doFilter)
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

	if (alreadyEntered) {
		shouldRestart = true;
		return;
	}
	alreadyEntered = true;
	shouldRestart  = false;

	Basket *current = currentBasket();
	const FilterData &filterData = current->decoration()->filterBar()->filterData();

	// Set the filter data for every other baskets, or reset the filter for every other baskets if we just disabled the filterInAllBaskets:
	Q3ListViewItemIterator it(m_tree);
	while (it.current()) {
		BasketListViewItem *item = ((BasketListViewItem*)it.current());
		if (item->basket() != current)
			if (isFilteringAllBaskets())
				item->basket()->decoration()->filterBar()->setFilterData(filterData); // Set the new FilterData for every other baskets
			else
				item->basket()->decoration()->filterBar()->setFilterData(FilterData()); // We just disabled the global filtering: remove the FilterData
		++it;
	}

	// Show/hide the "little filter icons" (during basket load)
	// or the "little numbers" (to show number of found notes in the baskets) is the tree:
	m_tree->triggerUpdate();
	kapp->processEvents();

	// Load every baskets for filtering, if they are not already loaded, and if necessary:
	if (filterData.isFiltering) {
		Basket *current = currentBasket();
		Q3ListViewItemIterator it(m_tree);
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
		}
	}

	m_tree->triggerUpdate();
//	kapp->processEvents();

	alreadyEntered = false;
	shouldRestart  = false;
}

void BNPView::newFilterFromFilterBar()
{
	if (isFilteringAllBaskets())
		QTimer::singleShot(0, this, SLOT(newFilter())); // Keep time for the QLineEdit to display the filtered character and refresh correctly!
}

bool BNPView::isFilteringAllBaskets()
{
	return m_actFilterAllBaskets->isChecked();
}


BasketListViewItem* BNPView::listViewItemForBasket(Basket *basket)
{
	Q3ListViewItemIterator it(m_tree);
	while (it.current()) {
		BasketListViewItem *item = ((BasketListViewItem*)it.current());
		if (item->basket() == basket)
			return item;
		++it;
	}
	return 0L;
}

Basket* BNPView::currentBasket()
{
	DecoratedBasket *decoBasket = (DecoratedBasket*)m_stack->currentWidget();
	if (decoBasket)
		return decoBasket->basket();
	else
		return 0;
}

Basket* BNPView::parentBasketOf(Basket *basket)
{
	BasketListViewItem *item = (BasketListViewItem*)(listViewItemForBasket(basket)->parent());
	if (item)
		return item->basket();
	else
		return 0;
}

void BNPView::setCurrentBasket(Basket *basket)
{
	if (currentBasket() == basket)
		return;

	if (currentBasket())
		currentBasket()->closeBasket();

	if (basket)
		basket->aboutToBeActivated();

	BasketListViewItem *item = listViewItemForBasket(basket);
	if (item) {
		m_tree->setSelected(item, true);
		item->ensureVisible();
		m_stack->setCurrentWidget(basket->decoration());
		// If the window has changed size, only the current basket receive the event,
		// the others will receive ony one just before they are shown.
		// But this triggers unwanted animations, so we eliminate it:
		basket->relayoutNotes(/*animate=*/false);
		basket->openBasket();
		setCaption(item->basket()->basketName());
		countsChanged(basket);
		updateStatusBarHint();
		if (Global::systemTray)
			Global::systemTray->updateDisplay();
		m_tree->ensureItemVisible(m_tree->currentItem());
		item->basket()->setFocus();
	}
	m_tree->viewport()->update();
	emit basketChanged();
}

void BNPView::removeBasket(Basket *basket)
{
	if (basket->isDuringEdit())
		basket->closeEditor();

	// Find a new basket to switch to and select it.
	// Strategy: get the next sibling, or the previous one if not found.
	// If there is no such one, get the parent basket:
	BasketListViewItem *basketItem = listViewItemForBasket(basket);
	BasketListViewItem *nextBasketItem = (BasketListViewItem*)(basketItem->nextSibling());
	if (!nextBasketItem)
		nextBasketItem = basketItem->prevSibling();
	if (!nextBasketItem)
		nextBasketItem = (BasketListViewItem*)(basketItem->parent());

	if (nextBasketItem)
		setCurrentBasket(nextBasketItem->basket());

	// Remove from the view:
	basket->unsubscribeBackgroundImages();
	m_stack->removeWidget(basket->decoration());
//	delete basket->decoration();
	delete basketItem;
//	delete basket;

	// If there is no basket anymore, add a new one:
	if (!nextBasketItem)
		BasketFactory::newBasket(/*icon=*/"", /*name=*/i18n("General"), /*backgroundImage=*/"", /*backgroundColor=*/QColor(), /*textColor=*/QColor(), /*templateName=*/"1column", /*createIn=*/0);
	else // No need to save two times if we add a basket
		save();

	emit basketNumberChanged(basketCount());
}

void BNPView::setTreePlacement(bool onLeft)
{
	if (onLeft)
		insertWidget(0, m_tree);
	else
		addWidget(m_tree);
	//updateGeometry();
	kapp->postEvent( this, new QResizeEvent(size(), size()) );
}

void BNPView::relayoutAllBaskets()
{
	Q3ListViewItemIterator it(m_tree);
	while (it.current()) {
		BasketListViewItem *item = ((BasketListViewItem*)it.current());
		//item->basket()->unbufferizeAll();
		item->basket()->unsetNotesWidth();
		item->basket()->relayoutNotes(true);
		++it;
	}
}

void BNPView::recomputeAllStyles()
{
	Q3ListViewItemIterator it(m_tree);
	while (it.current()) {
		BasketListViewItem *item = ((BasketListViewItem*)it.current());
		item->basket()->recomputeAllStyles();
		item->basket()->unsetNotesWidth();
		item->basket()->relayoutNotes(true);
		++it;
	}
}

void BNPView::removedStates(const Q3ValueList<State*> &deletedStates)
{
	Q3ListViewItemIterator it(m_tree);
	while (it.current()) {
		BasketListViewItem *item = ((BasketListViewItem*)it.current());
		item->basket()->removedStates(deletedStates);
		++it;
	}
}

void BNPView::linkLookChanged()
{
	Q3ListViewItemIterator it(m_tree);
	while (it.current()) {
		BasketListViewItem *item = ((BasketListViewItem*)it.current());
		item->basket()->linkLookChanged();
		++it;
	}
}

void BNPView::filterPlacementChanged(bool onTop)
{
	Q3ListViewItemIterator it(m_tree);
	while (it.current()) {
		BasketListViewItem *item        = static_cast<BasketListViewItem*>(it.current());
		DecoratedBasket    *decoration  = static_cast<DecoratedBasket*>(item->basket()->parent());
		decoration->setFilterBarPosition(onTop);
		++it;
	}
}

void BNPView::updateBasketListViewItem(Basket *basket)
{
	BasketListViewItem *item = listViewItemForBasket(basket);
	if (item)
		item->setup();

	if (basket == currentBasket()) {
		setCaption(basket->basketName());
		if (Global::systemTray)
			Global::systemTray->updateDisplay();
	}

	// Don't save if we are loading!
	if (!m_loading)
		save();
}

void BNPView::needSave(Q3ListViewItem*)
{
	if (!m_loading)
		// A basket has been collapsed/expanded or a new one is select: this is not urgent:
		QTimer::singleShot(500/*ms*/, this, SLOT(save()));
}

void BNPView::slotPressed(Q3ListViewItem *item, const QPoint &/*pos*/, int /*column*/)
{
	Basket *basket = currentBasket();
	if (basket == 0)
		return;

	// Impossible to Select no Basket:
	if (!item)
		m_tree->setSelected(listViewItemForBasket(basket), true);
	else if (dynamic_cast<BasketListViewItem*>(item) != 0 && currentBasket() != ((BasketListViewItem*)item)->basket()) {
		setCurrentBasket( ((BasketListViewItem*)item)->basket() );
		needSave(0);
	}
	basket->setFocus();
}

DecoratedBasket* BNPView::currentDecoratedBasket()
{
	if (currentBasket())
		return currentBasket()->decoration();
	else
		return 0;
}

// Redirected actions :

void BNPView::exportToHTML()              { HTMLExporter exporter(currentBasket());  }
void BNPView::editNote()                  { currentBasket()->noteEdit();             }
void BNPView::cutNote()                   { currentBasket()->noteCut();              }
void BNPView::copyNote()                  { currentBasket()->noteCopy();             }
void BNPView::delNote()                   { currentBasket()->noteDelete();           }
void BNPView::openNote()                  { currentBasket()->noteOpen();             }
void BNPView::openNoteWith()              { currentBasket()->noteOpenWith();         }
void BNPView::saveNoteAs()                { currentBasket()->noteSaveAs();           }
void BNPView::noteGroup()                 { currentBasket()->noteGroup();            }
void BNPView::noteUngroup()               { currentBasket()->noteUngroup();          }
void BNPView::moveOnTop()                 { currentBasket()->noteMoveOnTop();        }
void BNPView::moveOnBottom()              { currentBasket()->noteMoveOnBottom();     }
void BNPView::moveNoteUp()                { currentBasket()->noteMoveNoteUp();       }
void BNPView::moveNoteDown()              { currentBasket()->noteMoveNoteDown();     }
void BNPView::slotSelectAll()             { currentBasket()->selectAll();            }
void BNPView::slotUnselectAll()           { currentBasket()->unselectAll();          }
void BNPView::slotInvertSelection()       { currentBasket()->invertSelection();      }
void BNPView::slotResetFilter()           { currentDecoratedBasket()->resetFilter(); }

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

void BNPView::countsChanged(Basket *basket)
{
	if (basket == currentBasket())
		notesStateChanged();
}

void BNPView::notesStateChanged()
{
	Basket *basket = currentBasket();

	// Update statusbar message :
	if (currentBasket()->isLocked())
		setSelectionStatus(i18n("Locked"));
	else if (!basket->isLoaded())
		setSelectionStatus(i18n("Loading..."));
	else if (basket->count() == 0)
		setSelectionStatus(i18n("No notes"));
	else {
		QString count     = i18np("%1 note",     "%1 notes",    basket->count()         );
		QString selecteds = i18np("%1 selected", "%1 selected", basket->countSelecteds());
		QString showns    = (currentDecoratedBasket()->filterData().isFiltering ? i18n("all matches") : i18n("no filter"));
		if (basket->countFounds() != basket->count())
			showns = i18np("%1 match", "%1 matches", basket->countFounds());
		setSelectionStatus(
				i18nc("e.g. '18 notes, 10 matches, 5 selected'", "%1, %2, %3",count, showns, selecteds));
	}

	// If we added a note that match the global filter, update the count number in the tree:
	if (isFilteringAllBaskets())
		listViewItemForBasket(basket)->listView()->triggerUpdate();

	if (currentBasket()->redirectEditActions()) {
		m_actSelectAll         ->setEnabled( !currentBasket()->selectedAllTextInEditor() );
		m_actUnselectAll       ->setEnabled( currentBasket()->hasSelectedTextInEditor()  );
	} else {
		m_actSelectAll         ->setEnabled( basket->countSelecteds() < basket->countFounds() );
		m_actUnselectAll       ->setEnabled( basket->countSelecteds() > 0                     );
	}
	m_actInvertSelection   ->setEnabled( basket->countFounds() > 0 );

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

	Note *selectedGroup = (severalSelected ? currentBasket()->selectedGroup() : 0);

	m_actEditNote            ->setEnabled( !isLocked && oneSelected && !currentBasket()->isDuringEdit() );
	if (currentBasket()->redirectEditActions()) {
		m_actCutNote         ->setEnabled( currentBasket()->hasSelectedTextInEditor() );
		m_actCopyNote        ->setEnabled( currentBasket()->hasSelectedTextInEditor() );
		m_actPaste           ->setEnabled( true                                       );
		m_actDelNote         ->setEnabled( currentBasket()->hasSelectedTextInEditor() );
	} else {
		m_actCutNote         ->setEnabled( !isLocked && oneOrSeveralSelected );
		m_actCopyNote        ->setEnabled(              oneOrSeveralSelected );
		m_actPaste           ->setEnabled( !isLocked                         );
		m_actDelNote         ->setEnabled( !isLocked && oneOrSeveralSelected );
	}
	m_actOpenNote        ->setEnabled(              oneOrSeveralSelected );
	m_actOpenNoteWith    ->setEnabled(              oneSelected          ); // TODO: oneOrSeveralSelected IF SAME TYPE
	m_actSaveNoteAs      ->setEnabled(              oneSelected          ); // IDEM?
	m_actGroup           ->setEnabled( !isLocked && severalSelected && (!selectedGroup || selectedGroup->isColumn()) );
	m_actUngroup         ->setEnabled( !isLocked && selectedGroup && !selectedGroup->isColumn() );
	m_actMoveOnTop       ->setEnabled( !isLocked && oneOrSeveralSelected && !currentBasket()->isFreeLayout() );
	m_actMoveNoteUp      ->setEnabled( !isLocked && oneOrSeveralSelected ); // TODO: Disable when unavailable!
	m_actMoveNoteDown    ->setEnabled( !isLocked && oneOrSeveralSelected );
	m_actMoveOnBottom    ->setEnabled( !isLocked && oneOrSeveralSelected && !currentBasket()->isFreeLayout() );

	for (KAction *action = m_insertActions.first(); action; action = m_insertActions.next())
		action->setEnabled( !isLocked );

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
	popupMenu->insertItem( KIcon("document-save-as"), i18n("&Save a copy as..."), this, SLOT(slotSaveAs()), 0, 10 );
}*/
}

// BEGIN Color picker (code from KColorEdit):

/* Activate the mode
 */
void BNPView::slotColorFromScreen(bool global)
{
	m_colorPickWasGlobal = global;
	if (isMainWindowActive()) {
		if(Global::mainWindow()) Global::mainWindow()->hide();
		m_colorPickWasShown = true;
	} else
		m_colorPickWasShown = false;

		currentBasket()->saveInsertionData();
		m_colorPicker->pickColor();

/*	m_gettingColorFromScreen = true;
		kapp->processEvents();
		QTimer::singleShot( 100, this, SLOT(grabColorFromScreen()) );*/
}

void BNPView::slotColorFromScreenGlobal()
{
	slotColorFromScreen(true);
}

void BNPView::colorPicked(const QColor &color)
{
	if (!currentBasket()->isLoaded()) {
		showPassiveLoading(currentBasket());
		currentBasket()->load();
	}
	currentBasket()->insertColor(color);

	if (m_colorPickWasShown)
		showMainWindow();

	if (Settings::usePassivePopup())
		showPassiveDropped(i18n("Picked color to basket <i>%1</i>"));
}

void BNPView::colorPickingCanceled()
{
	if (m_colorPickWasShown)
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

	if (conversionsDone)
		KMessageBox::information(this, i18n("The plain text notes have been converted to rich text."), i18n("Conversion Finished"));
	else
		KMessageBox::information(this, i18n("There are no plain text notes to convert."), i18n("Conversion Finished"));
}

KMenu* BNPView::popupMenu(const QString &menuName)
{
	KMenu *menu = 0;
	bool hack = false; // TODO fix this
	// When running in kontact and likeback Information message is shown
	// factory is 0. Don't show error then and don't crash either :-)

	if(m_guiClient)
	{
		KXMLGUIFactory* factory = m_guiClient->factory();
		if(factory)
		{
			menu = (KMenu *)factory->container(menuName, m_guiClient);
		}
		else
			hack = isPart();
	}
	if (menu == 0) {
		if(!hack)
		{
			KStandardDirs stdDirs;
			const KAboutData *aboutData = KGlobal::mainComponent().aboutData();
			KMessageBox::error( this, i18n(
					"<p><b>The file basketui.rc seems to not exist or is too old.<br>"
							"%1 cannot run without it and will stop.</b></p>"
							"<p>Please check your installation of %2.</p>"
							"<p>If you do not have administrator access to install the application "
							"system wide, you can copy the file basketui.rc from the installation "
							"archive to the folder <a href='file://%3'>%4</a>.</p>"
							"<p>As last ressort, if you are sure the application is correctly installed "
							"but you had a preview version of it, try to remove the "
							"file %5basketui.rc</p>",
							aboutData->programName(), aboutData->programName(),
								stdDirs.saveLocation("data", "basket/"),stdDirs.saveLocation("data", "basket/"), stdDirs.saveLocation("data", "basket/")),
					i18n("Ressource not Found"), KMessageBox::AllowLink );
		}
		if(!isPart())
			exit(1); // We SHOULD exit right now and abord everything because the caller except menu != 0 to not crash.
		else
			menu = new KMenu; // When running in kpart we cannot exit
	}
	return menu;
}

void BNPView::showHideFilterBar(bool show, bool switchFocus)
{
//	if (show != m_actShowFilter->isChecked())
//		m_actShowFilter->setChecked(show);
	m_actShowFilter->setChecked(show);

	currentDecoratedBasket()->setFilterBarShown(show, switchFocus);
	if (!show)
		currentDecoratedBasket()->resetFilter();
}

void BNPView::insertEmpty(int type)
{
	if (currentBasket()->isLocked()) {
		showPassiveImpossible(i18n("Cannot add note."));
		return;
	}
	currentBasket()->insertEmptyNote(type);
}

void BNPView::insertWizard(int type)
{
	if (currentBasket()->isLocked()) {
		showPassiveImpossible(i18n("Cannot add note."));
		return;
	}
	currentBasket()->insertWizard(type);
}

// BEGIN Screen Grabbing:
void BNPView::grabScreenshot(bool global)
{
	if (m_regionGrabber) {
		KWindowSystem::activateWindow(m_regionGrabber->winId());
		return;
	}

	// Delay before to take a screenshot because if we hide the main window OR the systray popup menu,
	// we should wait the windows below to be repainted!!!
	// A special case is where the action is triggered with the global keyboard shortcut.
	// In this case, global is true, and we don't wait.
	// In the future, if global is also defined for other cases, check for
	// enum KAction::ActivationReason { UnknownActivation, EmulatedActivation, AccelActivation, PopupMenuActivation, ToolBarActivation };
	int delay = (isMainWindowActive() ? 500 : (global/*kapp->activePopupWidget()*/ ? 0 : 200));

	m_colorPickWasGlobal = global;
	if (isMainWindowActive()) {
		if(Global::mainWindow()) Global::mainWindow()->hide();
		m_colorPickWasShown = true;
	} else
		m_colorPickWasShown = false;

		currentBasket()->saveInsertionData();
		usleep(delay * 1000);
		m_regionGrabber = new RegionGrabber;
		connect( m_regionGrabber, SIGNAL(regionGrabbed(const QPixmap&)), this, SLOT(screenshotGrabbed(const QPixmap&)) );
}

void BNPView::grabScreenshotGlobal()
{
	grabScreenshot(true);
}

void BNPView::screenshotGrabbed(const QPixmap &pixmap)
{
	delete m_regionGrabber;
	m_regionGrabber = 0;

	// Cancelled (pressed Escape):
	if (pixmap.isNull()) {
		if (m_colorPickWasShown)
			showMainWindow();
		return;
	}

	if (!currentBasket()->isLoaded()) {
		showPassiveLoading(currentBasket());
		currentBasket()->load();
	}
	currentBasket()->insertImage(pixmap);

	if (m_colorPickWasShown)
		showMainWindow();

	if (Settings::usePassivePopup())
		showPassiveDropped(i18n("Grabbed screen zone to basket <i>%1</i>"));
}

Basket* BNPView::basketForFolderName(const QString &folderName)
{
/*	QPtrList<Basket> basketsList = listBaskets();
	Basket *basket;
	for (basket = basketsList.first(); basket; basket = basketsList.next())
	if (basket->folderName() == folderName)
	return basket;
*/

	QString name = folderName;
	if (!name.endsWith("/"))
		name += "/";

	Q3ListViewItemIterator it(m_tree);
	while (it.current()) {
		BasketListViewItem *item = ((BasketListViewItem*)it.current());
		if (item->basket()->folderName() == name)
			return item->basket();
		++it;
	}


	return 0;
}

Note* BNPView::noteForFileName(const QString &fileName, Basket &basket, Note* note)
{
	if (!note)
		note = basket.firstNote();
	if (note->fullPath().endsWith(fileName))
		return note;
	Note* child = note->firstChild();
	Note* found;
	while(child)
	{
		found = noteForFileName(fileName, basket, child);
		if (found)
			return found;
		child = child->next();
	}
	return 0;
}

void BNPView::setFiltering(bool filtering)
{
	m_actShowFilter->setChecked(filtering);
	m_actResetFilter->setEnabled(filtering);
	if (!filtering)
		m_actFilterAllBaskets->setEnabled(false);
}

void BNPView::undo()
{
	// TODO
}

void BNPView::redo()
{
	// TODO
}

void BNPView::pasteToBasket(int /*index*/, QClipboard::Mode /*mode*/)
{
	//TODO: REMOVE!
	//basketAt(index)->pasteNote(mode);
}

void BNPView::propBasket()
{
	BasketPropertiesDialog dialog(currentBasket(), this);
	dialog.exec();
}

void BNPView::delBasket()
{
//	DecoratedBasket *decoBasket    = currentDecoratedBasket();
	Basket          *basket        = currentBasket();

	int really = KMessageBox::questionYesNo( this,
											 i18n("<qt>Do you really want to remove the basket <b>%1</b> and its contents?</qt>",
													 Tools::textToHTMLWithoutP(basket->basketName())),
											 i18n("Remove Basket")
#if KDE_IS_VERSION( 3, 2, 90 ) // KDE 3.3.x
													 , KGuiItem(i18n("&Remove Basket"), "edit-delete"), KStandardGuiItem::cancel());
#else
		                    );
#endif

	if (really == KMessageBox::No)
		return;

	QStringList basketsList = listViewItemForBasket(basket)->childNamesTree();
	if (basketsList.count() > 0) {
		int deleteChilds = KMessageBox::questionYesNoList( this,
				i18n("<qt><b>%1</b> have the following children baskets.<br>Do you want to remove them too?</qt>",
						Tools::textToHTMLWithoutP(basket->basketName())),
				basketsList,
				i18n("Remove Children Baskets")
#if KDE_IS_VERSION( 3, 2, 90 ) // KDE 3.3.x
						, KGuiItem(i18n("&Remove Children Baskets"), "edit-delete"));
#else
		);
#endif

		if (deleteChilds == KMessageBox::No)
			listViewItemForBasket(basket)->moveChildsBaskets();
	}

	doBasketDeletion(basket);

//	basketNumberChanged();
//	rebuildBasketsMenu();
}

void BNPView::doBasketDeletion(Basket *basket)
{
	basket->closeEditor();

	Q3ListViewItem *basketItem = listViewItemForBasket(basket);
	Q3ListViewItem *nextOne;
	for (Q3ListViewItem *child = basketItem->firstChild(); child; child = nextOne) {
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
//	delete basket;
}

void BNPView::password()
{
#ifdef HAVE_LIBGPGME
	PasswordDlg dlg(kapp->activeWindow());
	Basket *cur = currentBasket();

	dlg.setType(cur->encryptionType());
	dlg.setKey(cur->encryptionKey());
	if(dlg.exec()) {
		cur->setProtection(dlg.type(), dlg.key());
		if (cur->encryptionType() != Basket::NoEncryption)
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

	KConfigGroup config = KGlobal::config()->group("Basket Archive");
	QString folder = config.readEntry("lastFolder", QDir::homePath()) + "/";
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
				"<qt>" + i18n("The file <b>%1</b> already exists. Do you really want to override it?",
					KUrl(destination).fileName()),
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

	config.writeEntry("lastFolder", KUrl(destination).directory());
	config.sync();

	Archive::save(basket, withSubBaskets, destination);
}

QString BNPView::s_fileToOpen = "";

void BNPView::delayedOpenArchive()
{
	Archive::open(s_fileToOpen);
}

void BNPView::openArchive()
{
	QString filter = "*.baskets|" + i18n("Basket Archives") + "\n*|" + i18n("All Files");
	QString path = KFileDialog::getOpenFileName(KUrl(), filter, this, i18n("Open Basket Archive"));
	if (!path.isEmpty()) // User has not canceled
		Archive::open(path);
}


void BNPView::activatedTagShortcut()
{
	Tag *tag = Tag::tagForKAction((KAction*)sender());
	currentBasket()->activatedTagShortcut(tag);
}

void BNPView::slotBasketNumberChanged(int number)
{
	m_actPreviousBasket->setEnabled(number > 1);
	m_actNextBasket    ->setEnabled(number > 1);
}

void BNPView::slotBasketChanged()
{
	m_actFoldBasket->setEnabled(canFold());
	m_actExpandBasket->setEnabled(canExpand());
	setFiltering(currentBasket() && currentBasket()->decoration()->filterData().isFiltering);
}

void BNPView::currentBasketChanged()
{
}

void BNPView::isLockedChanged()
{
	bool isLocked = currentBasket()->isLocked();

	setLockStatus(isLocked);

//	m_actLockBasket->setChecked(isLocked);
	m_actPropBasket->setEnabled(!isLocked);
	m_actDelBasket ->setEnabled(!isLocked);
	updateNotesActions();
}

void BNPView::askNewBasket()
{
	askNewBasket(0, 0);
}

void BNPView::askNewBasket(Basket *parent, Basket *pickProperties)
{
	NewBasketDefaultProperties properties;
	if (pickProperties) {
		properties.icon            = pickProperties->icon();
		properties.backgroundImage = pickProperties->backgroundImageName();
		properties.backgroundColor = pickProperties->backgroundColorSetting();
		properties.textColor       = pickProperties->textColorSetting();
		properties.freeLayout      = pickProperties->isFreeLayout();
		properties.columnCount     = pickProperties->columnsCount();
	}

	NewBasketDialog(parent, properties, this).exec();
}

void BNPView::askNewSubBasket()
{
	askNewBasket( /*parent=*/currentBasket(), /*pickPropertiesOf=*/currentBasket() );
}

void BNPView::askNewSiblingBasket()
{
	askNewBasket( /*parent=*/parentBasketOf(currentBasket()), /*pickPropertiesOf=*/currentBasket() );
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

	if (Settings::usePassivePopup())
		showPassiveDropped(i18n("Clipboard content pasted to basket <i>%1</i>"));
}

void BNPView::pasteSelInCurrentBasket()
{
	currentBasket()->pasteNote(QClipboard::Selection);

	if (Settings::usePassivePopup())
		showPassiveDropped(i18n("Selection pasted to basket <i>%1</i>"));
}

void BNPView::showPassiveDropped(const QString &title)
{
	if ( ! currentBasket()->isLocked() ) {
		// TODO: Keep basket, so that we show the message only if something was added to a NOT visible basket
		m_passiveDroppedTitle     = title;
		m_passiveDroppedSelection = currentBasket()->selectedNotes();
		QTimer::singleShot( c_delayTooltipTime, this, SLOT(showPassiveDroppedDelayed()) );
		// DELAY IT BELOW:
	} else
		showPassiveImpossible(i18n("No note was added."));
}

void BNPView::showPassiveDroppedDelayed()
{
	if (isMainWindowActive() || m_passiveDroppedSelection == 0)
		return;

	QString title = m_passiveDroppedTitle;

	delete m_passivePopup; // Delete previous one (if exists): it will then hide it (only one at a time)
	m_passivePopup = new KPassivePopup(Settings::useSystray() ? (QWidget*)Global::systemTray : this);
	QPixmap contentsPixmap = NoteDrag::feedbackPixmap(m_passiveDroppedSelection);
	Q3MimeSourceFactory::defaultFactory()->setPixmap("_passivepopup_image_", contentsPixmap);
	m_passivePopup->setView(
		title.arg(Tools::textToHTMLWithoutP(currentBasket()->basketName())),
		(contentsPixmap.isNull() ? "" : "<img src=\"_passivepopup_image_\">"),
		KIconLoader::global()->loadIcon(
			currentBasket()->icon(), KIconLoader::NoGroup, 16,
			KIconLoader::DefaultState, QStringList(), 0L, true
			)
		);
	m_passivePopup->show();
}

void BNPView::showPassiveImpossible(const QString &message)
{
	delete m_passivePopup; // Delete previous one (if exists): it will then hide it (only one at a time)
	m_passivePopup = new KPassivePopup(Settings::useSystray() ? (QWidget*)Global::systemTray : (QWidget*)this);
	m_passivePopup->setView(
		QString("<font color=red>%1</font>")
		    .arg(i18n("Basket <i>%1</i> is locked"))
		    .arg(Tools::textToHTMLWithoutP(currentBasket()->basketName())),
		message,
		KIconLoader::global()->loadIcon(
			currentBasket()->icon(), KIconLoader::NoGroup, 16,
			KIconLoader::DefaultState, QStringList(), 0L, true
			)
		);
	m_passivePopup->show();
}

void BNPView::showPassiveContentForced()
{
	showPassiveContent(/*forceShow=*/true);
}

void BNPView::showPassiveContent(bool forceShow/* = false*/)
{
	if (!forceShow && isMainWindowActive())
		return;

	// FIXME: Duplicate code (2 times)
	QString message;

	delete m_passivePopup; // Delete previous one (if exists): it will then hide it (only one at a time)
	m_passivePopup = new KPassivePopup(Settings::useSystray() ? (QWidget*)Global::systemTray : (QWidget*)this);
	m_passivePopup->setView(
			"<qt>" + KDialog::makeStandardCaption(
				currentBasket()->isLocked() ? QString("%1 <font color=gray30>%2</font>")
				    .arg(Tools::textToHTMLWithoutP(currentBasket()->basketName()), i18n("(Locked)"))
				: Tools::textToHTMLWithoutP(currentBasket()->basketName())
				),
			message,
			KIconLoader::global()->loadIcon(
				currentBasket()->icon(), KIconLoader::NoGroup, 16,
				KIconLoader::DefaultState, QStringList(), 0L, true
				)
		);
	m_passivePopup->show();
}

void BNPView::showPassiveLoading(Basket *basket)
{
	if (isMainWindowActive())
		return;

	delete m_passivePopup; // Delete previous one (if exists): it will then hide it (only one at a time)
	m_passivePopup = new KPassivePopup(Settings::useSystray() ? (QWidget*)Global::systemTray : (QWidget*)this);
	m_passivePopup->setView(
		Tools::textToHTMLWithoutP(basket->basketName()),
		i18n("Loading..."),
		KIconLoader::global()->loadIcon(
			basket->icon(), KIconLoader::NoGroup, 16, KIconLoader::DefaultState,
			QStringList(), 0L, true
			)
		);
	m_passivePopup->show();
}

void BNPView::addNoteText()  { showMainWindow(); currentBasket()->insertEmptyNote(NoteType::Text);  }
void BNPView::addNoteHtml()  { showMainWindow(); currentBasket()->insertEmptyNote(NoteType::Html);  }
void BNPView::addNoteImage() { showMainWindow(); currentBasket()->insertEmptyNote(NoteType::Image); }
void BNPView::addNoteLink()  { showMainWindow(); currentBasket()->insertEmptyNote(NoteType::Link);  }
void BNPView::addNoteColor() { showMainWindow(); currentBasket()->insertEmptyNote(NoteType::Color); }

void BNPView::aboutToHideNewBasketPopup()
{
	QTimer::singleShot(0, this, SLOT(cancelNewBasketPopup()));
}

void BNPView::cancelNewBasketPopup()
{
	m_newBasketPopup = false;
}

void BNPView::setNewBasketPopup()
{
	m_newBasketPopup = true;
}

void BNPView::setCaption(QString s)
{
	emit setWindowCaption(s);
}

void BNPView::updateStatusBarHint()
{
	m_statusbar->updateStatusBarHint();
}

void BNPView::setSelectionStatus(QString s)
{
	m_statusbar->setSelectionStatus(s);
}

void BNPView::setLockStatus(bool isLocked)
{
	m_statusbar->setLockStatus(isLocked);
}

void BNPView::postStatusbarMessage(const QString& msg)
{
	m_statusbar->postStatusbarMessage(msg);
}

void BNPView::setStatusBarHint(const QString &hint)
{
	m_statusbar->setStatusBarHint(hint);
}

void BNPView::setUnsavedStatus(bool isUnsaved)
{
	m_statusbar->setUnsavedStatus(isUnsaved);
}

void BNPView::setActive(bool active)
{
    KMainWindow* win = Global::mainWindow();
    if(!win)
	return;

    if (active == isMainWindowActive())
	return;
    kapp->updateUserTimestamp(); // If "activate on mouse hovering systray", or "on drag throught systray"
    Global::systemTray->toggleActive();
}

void BNPView::hideOnEscape()
{
	if (Settings::useSystray())
		setActive(false);
}

bool BNPView::isPart()
{
	return (strcmp(name(), "BNPViewPart") == 0);
}

bool BNPView::isMainWindowActive()
{
	KMainWindow* main = Global::mainWindow();
	if (main && main->isActiveWindow())
		return true;
	return false;
}

void BNPView::newBasket()
{
	askNewBasket();
}

bool BNPView::createNoteHtml(const QString content, const QString basket)
{
	Basket* b = basketForFolderName(basket);
	if (!b)
		return false;
	Note* note = NoteFactory::createNoteHtml(content, b);
	if (!note)
		return false;
	b -> insertCreatedNote(note);
	return true;
}

bool BNPView::changeNoteHtml(const QString content, const QString basket, const QString noteName)
{
	Basket* b = basketForFolderName(basket);
	if (!b)
		return false;
	Note* note = noteForFileName(noteName , *b);
	if (!note || note->content()->type()!=NoteType::Html)
		return false;
	HtmlContent* noteContent = (HtmlContent*)note->content();
	noteContent->setHtml(content);
	note->saveAgain();
	return true;
}

bool BNPView::createNoteFromFile(const QString url, const QString basket)
{
	Basket* b = basketForFolderName(basket);
	if (!b)
		return false;
	KUrl kurl(url);
	if ( url.isEmpty() )
		return false;
	Note* n = NoteFactory::copyFileAndLoad(kurl, b);
	if (!n)
		return false;
	b->insertCreatedNote(n);
	return true;
}

QStringList BNPView::listBaskets()
{
	QStringList basketList;

	Q3ListViewItemIterator it(m_tree);
	while (it.current()) {
		BasketListViewItem *item = ((BasketListViewItem*)it.current());
		basketList.append(item->basket()->basketName());
		basketList.append(item->basket()->folderName());
		++it;
	}
	return basketList;
}

void BNPView::handleCommandLine()
{
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	/* Custom data folder */
	QString customDataFolder = args->getOption("data-folder");
	if (!customDataFolder.isNull() && !customDataFolder.isEmpty())
	{
		Global::setCustomSavesFolder(customDataFolder);
	}
	/* Debug window */
	if (args->isSet("debug")) {
		new DebugWindow();
		Global::debugWindow->show();
	}

	/* Crash Handler to Mail Developers when Crashing: */
#ifndef BASKET_USE_DRKONQI
	if (!args->isSet("use-drkonqi"))
		KCrash::setCrashHandler(Crash::crashHandler);
#endif
}

void BNPView::reloadBasket(const QString &folderName)
{
	basketForFolderName(folderName)->reload();
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

void BNPView::enterEvent(QEvent*)
{
	if(m_tryHideTimer)
		m_tryHideTimer->stop();
	if(m_hideTimer)
		m_hideTimer->stop();
}

void BNPView::leaveEvent(QEvent*)
{
	if (Settings::useSystray() && Settings::hideOnMouseOut() && m_tryHideTimer)
		m_tryHideTimer->start(50);
}

void BNPView::timeoutTryHide()
{
	// If a menu is displayed, do nothing for the moment
	if (kapp->activePopupWidget() != 0L)
		return;

	if (kapp->widgetAt(QCursor::pos()) != 0L)
		m_hideTimer->stop();
	else if ( ! m_hideTimer->isActive() ) // Start only one time
		m_hideTimer->start(Settings::timeToHideOnMouseOut() * 100, true);

	// If a sub-dialog is oppened, we musn't hide the main window:
	if (kapp->activeWindow() != 0L && kapp->activeWindow() != Global::mainWindow())
		m_hideTimer->stop();
}

void BNPView::timeoutHide()
{
	// We check that because the setting can have been set to off
	if (Settings::useSystray() && Settings::hideOnMouseOut())
		setActive(false);
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
	if(!basket)
		return;
	if(m_actLockBasket)
		m_actLockBasket->setEnabled(!basket->isLocked() && basket->isEncrypted());
	if(m_actPassBasket)
		m_actPassBasket->setEnabled(!basket->isLocked());
	m_actPropBasket->setEnabled(!basket->isLocked());
	m_actDelBasket->setEnabled(!basket->isLocked());
	m_actExportToHtml->setEnabled(!basket->isLocked());
	m_actShowFilter->setEnabled(!basket->isLocked());
	m_actFilterAllBaskets->setEnabled(!basket->isLocked());
	m_actResetFilter->setEnabled(!basket->isLocked());
	basket->decoration()->filterBar()->setEnabled(!basket->isLocked());
}

void BNPView::showMainWindow()
{
	KMainWindow *win = Global::mainWindow();

	if (win)
	{
		win->show();
	}
	setActive(true);
	emit showPart();
}

void BNPView::populateTagsMenu()
{
	KMenu *menu = (KMenu*)(popupMenu("tags"));
	if (menu == 0 || currentBasket() == 0) // TODO: Display a messagebox. [menu is 0, surely because on first launch, the XMLGUI does not work!]
		return;
	menu->clear();

	Note *referenceNote;
	if (currentBasket()->focusedNote() && currentBasket()->focusedNote()->isSelected())
		referenceNote = currentBasket()->focusedNote();
	else
		referenceNote = currentBasket()->firstSelected();

	populateTagsMenu(*menu, referenceNote);

	m_lastOpenedTagsMenu = menu;
//	connect( menu, SIGNAL(aboutToHide()), this, SLOT(disconnectTagsMenu()) );
}

void BNPView::populateTagsMenu(KMenu &menu, Note *referenceNote)
{
	if (currentBasket() == 0)
		return;

	currentBasket()->m_tagPopupNote = referenceNote;
	bool enable = currentBasket()->countSelecteds() > 0;

	Q3ValueList<Tag*>::iterator it;
	Tag *currentTag;
	State *currentState;
	int i = 10;
	for (it = Tag::all.begin(); it != Tag::all.end(); ++it) {
	    // Current tag and first state of it:
	    currentTag = *it;
	    currentState = currentTag->states().first();

	    QKeySequence sequence;
		if (!currentTag->shortcut().isEmpty())
			sequence = currentTag->shortcut().primary();

	    StateAction *mi = new StateAction(currentState, KShortcut(sequence), this, true);

		// The previously set ID will be set in the actions themselves as data.
		mi->setData(i);

		if (referenceNote && referenceNote->hasTag(currentTag))
			mi->setChecked(true);

	    menu.addAction(mi);

	    if (!currentTag->shortcut().isEmpty())
			mi->setShortcut(sequence);

		mi->setEnabled(enable);
	    ++i;
	}

	// I don't like how this is implemented; but I can't think of a better way
	// to do this, so I will have to leave it for now
	menu.insertSeparator();
	KAction *act;
	act = new KAction(i18n("&Assign new Tag..."), &menu);
	act->setData(1);
	menu.addAction(act);

	act = new KAction(KIcon("edit-delete"), i18n("&Remove All"), &menu);
	act->setData(2);
	menu.addAction(act);

	act = new KAction(KIcon("configure"), i18n("&Customize..."), &menu);
	act->setData(3);
	menu.addAction(act);

	menu.setItemEnabled(1, enable);
	if (!currentBasket()->selectedNotesHaveTags())
		menu.setItemEnabled(2, false);

	connect( &menu, SIGNAL(triggered(QAction *)), currentBasket(), SLOT(toggledTagInMenu(QAction *)) );
	connect( &menu, SIGNAL(aboutToHide()),  currentBasket(), SLOT(unlockHovering())      );
	connect( &menu, SIGNAL(aboutToHide()),  currentBasket(), SLOT(disableNextClick())    );
}

void BNPView::connectTagsMenu()
{
	connect( popupMenu("tags"), SIGNAL(aboutToShow()), this, SLOT(populateTagsMenu())   );
	connect( popupMenu("tags"), SIGNAL(aboutToHide()), this, SLOT(disconnectTagsMenu()) );
}

/*
 * The Tags menu is ONLY created once the BasKet KPart is first shown.
 * So we can use this menu only from then?
 * When the KPart is changed in Kontact, and then the BasKet KPart is shown again,
 * Kontact created a NEW Tags menu. So we should connect again.
 * But when Kontact main window is hidden and then re-shown, the menu does not change.
 * So we disconnect at hide event to ensure only one connection: the next show event will not connects another time.
 */

void BNPView::showEvent(QShowEvent*)
{
	if (isPart())
		QTimer::singleShot( 0, this, SLOT(connectTagsMenu()) );

	if (m_firstShow) {
		m_firstShow = false;
		onFirstShow();
	}
	if (isPart()/*TODO: && !LikeBack::enabledBar()*/) {
		Global::likeBack->enableBar();
	}
}

void BNPView::hideEvent(QHideEvent*)
{
	if (isPart()) {
		disconnect( popupMenu("tags"), SIGNAL(aboutToShow()), this, SLOT(populateTagsMenu())   );
		disconnect( popupMenu("tags"), SIGNAL(aboutToHide()), this, SLOT(disconnectTagsMenu()) );
	}

	if (isPart())
		Global::likeBack->disableBar();
}

void BNPView::disconnectTagsMenu()
{
	QTimer::singleShot( 0, this, SLOT(disconnectTagsMenuDelayed()) );
}

void BNPView::disconnectTagsMenuDelayed()
{
	disconnect( m_lastOpenedTagsMenu, SIGNAL(triggered(QAction *)), currentBasket(), SLOT(toggledTagInMenu(QAction *)) );
	disconnect( m_lastOpenedTagsMenu, SIGNAL(aboutToHide()),  currentBasket(), SLOT(unlockHovering())      );
	disconnect( m_lastOpenedTagsMenu, SIGNAL(aboutToHide()),  currentBasket(), SLOT(disableNextClick())    );
}

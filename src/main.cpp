/***************************************************************************
 *   Copyright (C) 2003 by S�bastien Lao�t                                 *
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

#include <stdlib.h>

#include <kuniqueapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <qpixmap.h>
#include <klocale.h>
#include <kglobalaccel.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

#include <kconfig.h> // TMP IN ALPHA 1

#include "backgroundmanager.h"
#include "container.h"
#include "settings.h"
#include "global.h"
#include "debugwindow.h"
#include "notedrag.h"
#include "basket.h"
#include "likeback.h"

#include "crashhandler.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* Thanks to JuK for this Application class */
#if KDE_IS_VERSION( 3, 1, 90 )
typedef KUniqueApplication Application; // KDE 3.2 and later already re-show the main window
#else
class Application : public KUniqueApplication
{
  public:
	Application() : KUniqueApplication(true, true, false) {}
	virtual ~Application() {}
	virtual int newInstance() {
		if (Global::mainContainer)
			Global::mainContainer->setActive(true);
		return KUniqueApplication::newInstance();
	}
};
#endif

int main(int argc, char *argv[])
{
	/* Application */
	static const char description[] = I18N_NOOP("A set of baskets to keep a full range of data on hand.");
	static KCmdLineOptions options[] =
	{
		{ "d", 0, 0 },
		{ "debug", I18N_NOOP("Show the debug window"), 0 },
		{ "f", 0, 0 },
		{ "data-folder <folder>", I18N_NOOP("Custom folder where to load and save basket data and application data (useful for debugging purpose)"), 0 },
		{ "h", 0, 0 },
		{ "start-hidden", I18N_NOOP("Hide the main window in the system tray icon on startup"), 0 },
		{ "k", 0, 0 },
		{ "use-dr-konquy", I18N_NOOP("When crashing, use the standard KDE report dialog instead of sending an email"), 0 },
		{ 0, 0, 0 }
	};
	KAboutData aboutData( "basket", I18N_NOOP("BasKet Note Pads"),
	                      VERSION, description, KAboutData::License_GPL_V2,
	                      "(c) 2003-2005, Sébastien Laoût", 0,
	                      "http://basket.kde.org/",
	                      "slaout@linux62.org"                              );
	aboutData.addAuthor( "Sébastien Laoût", I18N_NOOP("Author, maintainer"), "slaout@linux62.org" );
	aboutData.addAuthor( "Marco Martin",      I18N_NOOP("Icon"),               "m4rt@libero.it"     );

	KCmdLineArgs::init(argc, argv, &aboutData);
	KCmdLineArgs::addCmdLineOptions(options);

	KUniqueApplication::addCmdLineOptions();
	KUniqueApplication app;


	/* Crash Handler to Mail Developers when Crashing: */
#ifndef BASKET_USE_DRKONQI
	if (!KCmdLineArgs::parsedArgs()->isSet("use-dr-konquy"))
		KCrash::setCrashHandler( Crash::crashHandler );
#endif


	/******* ALPHA 1 ***********/
	/*KConfig *config = KGlobal::config();
	config->setGroup("Development Version");
	QString keyName = "bnp060b1__messagesAlreadyShown";
	if (!config->readBoolEntry(keyName, false)) {
		int reply = KMessageBox::warningYesNo(0, QString(
			"<p>It is recommanded for you to <b>backup your BasKet data</b> before using this pre-version.</p>"
			"<p>Then, if you miss some older features that are not re-added in the product yet, you will be able to revert back to 0.5.0.</p>"
			"<P>To backup, you just have to copy the folder <a href='file://%1'>%2</a> in a safe destination.</p>"
			"<p>I, the developer, am using this version daily and haven't experienced any data-loss. You should be happy with it. "
			"But some features are temporarily disabled or some bugs can to not have been found yet. "
			"There is no way to come back from 0.6.0 to 0.5.0 except by recovering your backed up data.</p>"
			"<p><b>The feature that is not fully re-added yet is Keyboard Navigation.</b></p>"
			"<p>Note that some features have been removed and will not be re-added in the final release. "
			"See <a href='http://basket.kde.org/usability-0.6.0.php'>this page</a> for more information.</p>"
			"<p><b>IMPORTANT</b>: If you were a user of the private Pre-Alpha versions (Alpha 0.1 to 0.9), please remove the file %3tags.xml to be able to export tags in texts.</p>"
			"<p>When you've done the required steps, you can continue.</p>")
				.arg(Global::savesFolder(), Global::savesFolder(), Global::savesFolder()),
			"Please Backup your Data", KStdGuiItem::cont(), KStdGuiItem::quit(), QString::null, KMessageBox::Notify | KMessageBox::AllowLink);
		if (reply != KMessageBox::Yes)
			exit(0);
		config->writeEntry(keyName, true);
	}*/
	/***************************/


	/* LikeBack */
	QString version = VERSION;
	// TODO: LikeBack::init() instead of all those if()s    [or implicitely called if not by hand?!]:
	if (LikeBack::userWantToParticipate())
		if ( version.find("alpha", /*index=*/0, /*caseSensitive=*/false) != -1 ||
		     version.find("beta",  /*index=*/0, /*caseSensitive=*/false) != -1 ||
		     version.find("rc",    /*index=*/0, /*caseSensitive=*/false) != -1 ||
		     version.find("svn",   /*index=*/0, /*caseSensitive=*/false) != -1    ) // TODO: LikeBack::isDevelVersion
			new LikeBack(LikeBack::AllButtons, LikeBack::NoListing, i18n("Only english and french languages are accepted."));
	LikeBack::setServer("basket.linux62.org", "/likeback/send.php");


	/******* RICH TEXTS INSTEAD OF PLAIN TEXTS ***********/
	KMessageBox::information(0,
		"<h1>This version use rich text instead of plain text notes!</h1>"
		"<p>Be rassured: every drawbacks of rich text notes have been solved. You can happily enjoy theire power.</p>"
		"<p>This version is a <b>test to see how well people will receive the abandon of text notes, <u>in order to remove them completely in the final version</u></b>. "
		"Please test the application during a few days. And then, if you are disapointed, please mail me, or click the colored hands bellow every window title bar to send your feedback.</p>",
		"Rich Text Notes now Standard", "richTextNotesAreNowStandard");
	/***************************/


	/* Custom data folder */
	QCString customDataFolder = KCmdLineArgs::parsedArgs()->getOption("data-folder");
	if (customDataFolder != 0 && !customDataFolder.isEmpty())
		Global::setCustomSavesFolder(customDataFolder);

	/* Settings */
	Settings::loadConfig();

	/* Debug window */
	if ( KCmdLineArgs::parsedArgs()->isSet("debug") ) {
		new DebugWindow();
		Global::debugWindow->show();
	}

	// Needed when loading the baskets:
	Global::globalAccel       = new KGlobalAccel(Global::mainContainer); // FIXME: Global::mainContainer is null at this point!
	Global::backgroundManager = new BackgroundManager();

	/* Main container */
	/*Global::mainContainer = */new Container();
	app.setMainWidget(Global::mainContainer);
//	Global::basketTree->setTreePlacement(Settings::treeOnLeft());

	/* Self-test of the presence of basketui.rc (the only requiered file after basket executable) */
	if (Global::mainContainer->popupMenu("basket") == 0L)
		// An error message will be show by Container::popupMenu()
		return 1;

	/* System tray icon */
	Global::tray = new ContainerSystemTray(Global::mainContainer);
	if (Settings::useSystray())
		Global::tray->show();

	if (Settings::useSystray() && KCmdLineArgs::parsedArgs()->isSet("start-hidden"))
		Global::mainContainer->hide();
	else if (Settings::useSystray() && app.isRestored())
		Global::mainContainer->setShown( !Settings::startDocked() );
	else
		Global::mainContainer->show();

	// If the main window is hidden when session is saved, Container::queryClose()
	//  isn't called and the last value would be kept
	Settings::setStartDocked(true);

	/* Global shortcuts */
	KGlobalAccel *globalAccel = Global::globalAccel; // Better for the following lines
	globalAccel->insert( "global_show_hide_main_window", i18n("Show/hide main window"),
	                     i18n("Allows you to show main Window if it is hidden, and to hide it if it is shown."),
	                     Qt::CTRL+Qt::SHIFT+Qt::Key_W, Qt::CTRL+Qt::SHIFT+Qt::Key_W,
	                     Global::mainContainer, SLOT(changeActive()),             true, true );
	globalAccel->insert( "global_paste", i18n("Paste clipboard contents in current basket"),
	                     i18n("Allows you to paste clipboard contents in the current basket without having to open main window."),
	                     Qt::CTRL+Qt::ALT+Qt::Key_V, Qt::CTRL+Qt::ALT+Qt::Key_V,
	                     Global::mainContainer, SLOT(globalPasteInCurrentBasket()), true, true );
	globalAccel->insert( "global_show_current_basket", i18n("Show current basket name"),
	                     i18n("Allows you to know basket is current without opening the main window."),
	                     "", "",
	                     Global::mainContainer, SLOT(showPassiveContentForced()), true, true );
	globalAccel->insert( "global_paste_selection", i18n("Paste selection in current basket"),
	                     i18n("Allows you to paste clipboard selection in the current basket without having to open main window."),
	                     Qt::CTRL+Qt::ALT+Qt::Key_S, Qt::CTRL+Qt::ALT+Qt::Key_S,
	                     Global::mainContainer, SLOT(pasteSelInCurrentBasket()),  true, true );
	globalAccel->insert( "global_new_basket", i18n("Create a new basket"),
	                     i18n("Allows you to create a new basket without having to open main window (you then can use the other global shortcuts to add a note, paste clipboard or paste selection in this new basket)."),
	                     "", "",
	                     Global::mainContainer, SLOT(askNewBasket()),       true, true );
	globalAccel->insert( "global_previous_basket", i18n("Go to previous basket"),
	                     i18n("Allows you to change current basket to the previous one without having to open main window."),
	                     "", "",
	                     Global::basketTree,    SLOT(goToPreviousBasket()), true, true );
	globalAccel->insert( "global_next_basket", i18n("Go to next basket"),
	                     i18n("Allows you to change current basket to the next one without having to open main window."),
	                     "", "",
	                     Global::basketTree,    SLOT(goToNextBasket()),     true, true );


	globalAccel->insert( "global_note_add_text", i18n("Insert text note"),
	                     i18n("Add a text note to the current basket without having to open main window."),
	                     "", "", //Qt::CTRL+Qt::ALT+Qt::Key_T, Qt::CTRL+Qt::ALT+Qt::Key_T,
	                     Global::mainContainer, SLOT(addNoteText()),        true, true );
	globalAccel->insert( "global_note_add_html", i18n("Insert rich text note"),
	                     i18n("Add a rich text note to the current basket without having to open main window."),
	                     Qt::CTRL+Qt::ALT+Qt::Key_H, Qt::CTRL+Qt::ALT+Qt::Key_H, //"", "",
	                     Global::mainContainer, SLOT(addNoteHtml()),        true, true );
	globalAccel->insert( "global_note_add_image", i18n("Insert image note"),
	                     i18n("Add an image note to the current basket without having to open main window."),
	                     "", "",
	                     Global::mainContainer, SLOT(addNoteImage()),       true, true );
	globalAccel->insert( "global_note_add_link", i18n("Insert link note"),
	                     i18n("Add a link note to the current basket without having to open main window."),
	                     "", "",
	                     Global::mainContainer, SLOT(addNoteLink()),        true, true );
	globalAccel->insert( "global_note_add_color", i18n("Insert color note"),
	                     i18n("Add a color note to the current basket without having to open main window."),
	                     "", "",
	                     Global::mainContainer, SLOT(addNoteColor()),       true, true );
	globalAccel->insert( "global_note_pick_color", i18n("Pick color from screen"),
	                     i18n("Add a color note picked from one pixel on screen to the current basket without "
	                          "having to open main window."),
	                     "", "",
	                     Global::mainContainer, SLOT(slotColorFromScreenGlobal()), true, true );
	globalAccel->insert( "global_note_grab_screenshot", i18n("Grab screen zone"),
	                     i18n("Grab a screen zone as an image in the current basket without "
	                          "having to open main window."),
	                     "", "",
	                     Global::mainContainer, SLOT(grabScreenshotGlobal()), true, true );
	globalAccel->readSettings();
	globalAccel->updateConnections();

	/* Go */
	NoteDrag::createAndEmptyCuttingTmpFolder(); // If last exec hasn't done it: clean the temporary folder we will use
	int result = app.exec();
	//if (Global::mainContainer->currentBasket()->isDuringEdit())
	//	Global::mainContainer->currentBasket()->closeEditor();
//	Settings::setMainWindowPosition(Global::mainContainer->pos());
//	Settings::setMainWindowSize(Global::mainContainer->size());
	Settings::saveConfig();
	delete Global::mainContainer; // We do it explicitly here, because the DesktopColorPicker need to be deleted to not crash!
	NoteDrag::createAndEmptyCuttingTmpFolder(); // Clean the temporary folder we used
	return result;
}

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

#include <stdlib.h>

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <qpixmap.h>
#include <klocale.h>
#include <kglobalaccel.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <qfile.h>
#include <qfileinfo.h>

#include <kconfig.h> // TMP IN ALPHA 1
#include <QApplication>
#include "application.h"
#include "backgroundmanager.h"
#include "mainwindow.h"
#include "settings.h"
#include "global.h"
#include "debugwindow.h"
#include "notedrag.h"
#include "basket.h"
#include "aboutdata.h"
//#include "basket_options.h"
#include "backup.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* Thanks to JuK for this Application class */
/*#if KDE_IS_VERSION( 3, 1, 90 )
typedef KUniqueApplication Application; // KDE 3.2 and later already re-show the main window
#else
class Application : public KUniqueApplication
{
  public:
	Application() : KUniqueApplication(true, true, false) {}
	virtual ~Application() {}
	virtual int newInstance() {
		if (win)
			win->setActive(true);
		return KUniqueApplication::newInstance();
	}
};
#endif
*/

int main(int argc, char *argv[])
{
	// KCmdLineArgs::init will modify argv[0] so we remember it:
	const char *argv0 = (argc >= 1 ? argv[0] : "");

	KCmdLineArgs::init(argc, argv, Global::about());

	KCmdLineOptions basket_options;
	basket_options.add ( "d", KLocalizedString(), 0 );
	basket_options.add ( "debug", ki18n ( "Show the debug window" ), 0 );
	basket_options.add ( "f", KLocalizedString(), 0 );
	basket_options.add ( "data-folder <folder>", ki18n ( "Custom folder where to load and save basket data" "and application data (useful for debugging purpose)" ), 0 );
	basket_options.add ( "h", KLocalizedString(), 0 );
	basket_options.add ( "start-hidden", ki18n ( "Hide the main window in the system tray icon on startup" ), 0 );
	basket_options.add ( "k", KLocalizedString(), 0 );
	//basket_options.add ( "use-drkonquy", ki18n ( "When crashing, use the standard KDE report dialog instead" "of sending an email" ), 0 );
	basket_options.add ( "+[file]", ki18n ( "Open basket archive or template" ), 0 );
	KCmdLineArgs::addCmdLineOptions(basket_options);

	//KUniqueApplication::addCmdLineOptions();
	//KUniqueApplication app;
	KApplication app;
	//Application app;

	//Backup::figureOutBinaryPath ( argv0, app );

	/* Main Window */
	MainWindow* win = new MainWindow();
	Global::bnpView->handleCommandLine();

//app.setMainWidget(win);
//	if (!(Settings::useSystray() && KCmdLineArgs::parsedArgs() && KCmdLineArgs::parsedArgs()->isSet("start-hidden")))
//		win->show();

	if (Settings::useSystray()) {
		// The user wanted to not show the window (but it is already hidden by default, so we do nothing):
		if (KCmdLineArgs::parsedArgs() && KCmdLineArgs::parsedArgs()->isSet("start-hidden"))
			;
		// When the application is restored by KDE session, restore its state:
		else if ( app.isSessionRestored() )
			win->setShown(!Settings::startDocked());
		// Else, the application has been launched explicitely by the user (KMenu, keyboard shortcut...), so he need it, we show it:
		else
			win->show();
	} else
		// No system tray icon: always show:
		win->show();

	// Self-test of the presence of basketui.rc (the only requiered file after basket executable)
	//if ( Global::bnpView->popupMenu ( "basket" ) == 0L )
		// An error message will be show by BNPView::popupMenu()
	//	return 1;

	/* Go */
	int result = app.exec();
	//return result;
	exit(result); // Do not clean up memory to not crash while deleting the KApplication, or do not hang up on KDE exit
}

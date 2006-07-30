/***************************************************************************
 *   Copyright (C) 2003 by S�astien Laot                                 *
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
#include <kdebug.h>

#include <kconfig.h> // TMP IN ALPHA 1

#include "backgroundmanager.h"
#include "mainwindow.h"
#include "settings.h"
#include "global.h"
#include "debugwindow.h"
#include "notedrag.h"
#include "basket.h"
#include "aboutdata.h"
#include "basket_options.h"

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
		if (win)
			win->setActive(true);
		return KUniqueApplication::newInstance();
	}
};
#endif

int main(int argc, char *argv[])
{
	KCmdLineArgs::init(argc, argv, Global::about());
	KCmdLineArgs::addCmdLineOptions(basket_options);

	KUniqueApplication::addCmdLineOptions();
	KUniqueApplication app;

	/* Main Window */
	MainWindow* win = new MainWindow();
	Global::bnpView->handleCommandLine();
	app.setMainWidget(win);
	win->show();

	// Self-test of the presence of basketui.rc (the only requiered file after basket executable)
	if (Global::bnpView->popupMenu("basket") == 0L)
		// An error message will be show by BNPView::popupMenu()
		return 1;

	/* Go */
	int result = app.exec();
	return result;
}

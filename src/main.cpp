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
#include "likeback.h"

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
	AboutData aboutData;
	KCmdLineArgs::init(argc, argv, &aboutData);
	KCmdLineArgs::addCmdLineOptions(basket_options);

	KUniqueApplication::addCmdLineOptions();
	KUniqueApplication app;

	/* LikeBack */
	LikeBack::init();
	LikeBack::setServer("basket.linux62.org", "/likeback/send.php");
//	LikeBack::setServer("localhost", "/~seb/basket/likeback/send.php");
	LikeBack::setCustomLanguageMessage(i18n("Only english and french languages are accepted."));
	LikeBack::setWindowNamesListing(LikeBack:: /*NoListing*/ /*WarnUnnamedWindows*/ AllWindows);

	/******* RICH TEXTS INSTEAD OF PLAIN TEXTS ***********/
	KMessageBox::information(0, i18n(
		"<h1>This version use rich text instead of plain text notes!</h1>"
		"<p>Be rassured: every drawbacks of rich text notes have been solved. You can happily enjoy theire power.</p>"
		"<p>This version is a <b>test to see how well people will receive the abandon of text notes, <u>in order to remove them completely in the final version</u></b>. "
		"Please test the application during a few days. And then, if you are disapointed, please mail me, or click the colored hands bellow every window title bar to send your feedback.</p>"
		"<p>Your plain text notes were <b>not</b> converted.<br>"
		"At any moment, you can click the last icon in the toolbar to <i>convert the notes of only one or all baskets</i>. You are even <u>encouraged</u> to do so.</p>"),
		"Rich Text Notes now Standard", "richTextNotesNowStandard");
	/***************************/

	/* Main container */
	MainWindow* win = new MainWindow();
	Global::bnpView->handleCommandLine();
	app.setMainWidget(win);
	win->show();

	// Self-test of the presence of basketui.rc (the only requiered file after basket executable)
	if (Global::bnpView->popupMenu("basket") == 0L)
		// An error message will be show by Container::popupMenu()
		return 1;

	/* Go */
	int result = app.exec();
	return result;
}

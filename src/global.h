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

#ifndef GLOBAL_H
#define GLOBAL_H

#include <qstring.h>
#include "aboutdata.h"

class LikeBack;
class DebugWindow;
class BackgroundManager;
class SystemTray;
class BNPView;
class KMainWindow;
class KAboutData;

/** Handle all global variables of the application.
  * This file only declare classes : developer should include
  * the .h files of variables he use.
  * @author S�astien Laot
  */
class Global
{
  private:
	static QString s_customSavesFolder;
  public:
	// Global Variables:
	static LikeBack          *likeBack;
	static DebugWindow       *debugWindow;
	static BackgroundManager *backgroundManager;
	static SystemTray        *systemTray;
	static BNPView           *bnpView;
	static KSharedConfig::Ptr basketConfig;
	static AboutData          basketAbout;

	// Application Folders:
	static void setCustomSavesFolder(const QString &folder);
	static QString savesFolder();       /// << @return e.g. "/home/username/.kde/share/apps/basket/".
	static QString basketsFolder();     /// << @return e.g. "/home/username/.kde/share/apps/basket/baskets/".
	static QString backgroundsFolder(); /// << @return e.g. "/home/username/.kde/share/apps/basket/backgrounds/".
	static QString templatesFolder();   /// << @return e.g. "/home/username/.kde/share/apps/basket/templates/".
	static QString tempCutFolder();     /// << @return e.g. "/home/username/.kde/share/apps/basket/temp-cut/".   (was ".tmp/")

	// Various Things:
	static QString openNoteIcon();      /// << @return the icon used for the "Open" action on notes.
	static KMainWindow* mainWindow();
	static KConfig* config();
	static KAboutData* about() { return &basketAbout; };
};

#endif // GLOBAL_H

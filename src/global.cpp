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

#include <kglobal.h>
#include <kstandarddirs.h>
#include <qstring.h>
#include <kaction.h>
#include <qdir.h>

#include "global.h"
#include "container.h"

/** Define initial values for global variables : */

QString              Global::s_customSavesFolder = "";
DebugWindow         *Global::debugWindow         = 0L;
BackgroundManager   *Global::backgroundManager   = 0L;
Container           *Global::mainContainer       = 0L;
ContainerSystemTray *Global::tray                = 0L;
BasketTree          *Global::basketTree          = 0L;
KGlobalAccel        *Global::globalAccel         = 0L;

void Global::setCustomSavesFolder(const QString &folder)
{
	s_customSavesFolder = folder;
}

QString Global::savesFolder()
{
	static QString *folder = 0L; // Memorize the folder to do not have to re-compute it each time it's needed

	if (folder == 0L) {            // Initialize it if not yet done
		if (s_customSavesFolder.isEmpty())
			folder = new QString( KGlobal::dirs()->saveLocation("data", "basket/") );
		else {
			QDir dir;
			dir.mkdir(s_customSavesFolder);
			folder = new QString(s_customSavesFolder + "/");
		}
	}

	return *folder;
}

QString Global::basketsFolder()     { return savesFolder() + "baskets/";     }
QString Global::backgroundsFolder() { return savesFolder() + "backgrounds/"; }
QString Global::templatesFolder()   { return savesFolder() + "templates/";   }
QString Global::tempCutFolder()     { return savesFolder() + "temp-cut/";    }

QString Global::openNoteIcon()
{
	return Global::mainContainer->m_actOpenNote->icon();
}

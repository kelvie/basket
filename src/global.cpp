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

#include <KDE/KGlobal>
#include <KDE/KStandardDirs>
#include <QString>
#include <KDE/KAction>
#include <KDE/KApplication>
#include <KDE/KMainWindow>
#include <QDir>
#include <KDE/KDebug>
#include <KDE/KConfig>

#include "global.h"
#include "bnpview.h"
#include "settings.h"

/** Define initial values for global variables : */

QString            Global::s_customSavesFolder = "";
LikeBack          *Global::likeBack            = 0L;
DebugWindow       *Global::debugWindow         = 0L;
BackgroundManager *Global::backgroundManager   = 0L;
SystemTray        *Global::systemTray          = 0L;
BNPView           *Global::bnpView             = 0L;
KSharedConfig::Ptr Global::basketConfig;
AboutData          Global::basketAbout;

void Global::setCustomSavesFolder(const QString &folder)
{
    s_customSavesFolder = folder;
}

QString Global::savesFolder()
{
    static QString *folder = 0L; // Memorize the folder to do not have to re-compute it each time it's needed

    if (folder == 0L) {          // Initialize it if not yet done
        if (!s_customSavesFolder.isEmpty()) { // Passed by command line (for development & debug purpose)
            QDir dir;
            dir.mkdir(s_customSavesFolder);
            folder = new QString(s_customSavesFolder.endsWith("/") ? s_customSavesFolder : s_customSavesFolder + "/");
        } else if (!Settings::dataFolder().isEmpty()) { // Set by config option (in Basket -> Backup & Restore)
            QDir dir;
            dir.mkdir(s_customSavesFolder);
            folder = new QString(Settings::dataFolder().endsWith("/") ? Settings::dataFolder() : Settings::dataFolder() + "/");
        } else { // The default path (should be that for most computers)
            folder = new QString(KGlobal::dirs()->saveLocation("data", "basket/"));
        }
    }

    return *folder;
}

QString Global::basketsFolder()
{
    return savesFolder() + "baskets/";
}
QString Global::backgroundsFolder()
{
    return savesFolder() + "backgrounds/";
}
QString Global::templatesFolder()
{
    return savesFolder() + "templates/";
}
QString Global::tempCutFolder()
{
    return savesFolder() + "temp-cut/";
}

QString Global::openNoteIcon() // FIXME: Now an edit icon
{
    return QVariant(Global::bnpView->m_actEditNote->icon()).toString();
}

KMainWindow* Global::mainWindow()
{
    QWidget* res = kapp->activeWindow();

    if (res && res->inherits("KMainWindow")) {
        return static_cast<KMainWindow*>(res);
    }
    return 0;
}

KConfig* Global::config()
{
    //The correct solution is to go and replace all KConfig* with KSharedConfig::Ptr, but that seems awfully annoying to do right now
    return Global::basketConfig.data();
}

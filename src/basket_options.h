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

#ifndef BASKET_OPTIONS_H
#define BASKET_OPTIONS_H

#include <kcmdlineargs.h>
#include <klocale.h>

KCmdLineOptions basket_options[] =
{
	{ "d", 0, 0 },
	{ "debug", I18N_NOOP("Show the debug window"), 0 },
	{ "f", 0, 0 },
	{ "data-folder <folder>", I18N_NOOP("Custom folder where to load and save basket data and application data (useful for debugging purpose)"), 0 },
	{ "h", 0, 0 },
	{ "start-hidden", I18N_NOOP("Hide the main window in the system tray icon on startup"), 0 },
	{ "k", 0, 0 },
	{ "use-drkonquy", I18N_NOOP("When crashing, use the standard KDE report dialog instead of sending an email"), 0 },
	{ 0, 0, 0 }
};

#endif // BASKET_OPTIONS_H

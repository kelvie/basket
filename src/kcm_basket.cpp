/***************************************************************************
 *   Copyright (C) 2006 by Petri Damsten                                   *
 *   damu@iki.fi                                                           *
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

// This must be first
#include <config.h>
#include "settings.h"
#include <kcmodule.h>

//----------------------------
// KCM stuff
//----------------------------
extern "C"
{
	KDE_EXPORT KCModule *create_basket_config_general(QWidget *parent, const char *)
	{
		GeneralPage *page = new GeneralPage(parent, "kcmbasket_config_general");
		return page;
	}
}

extern "C"
{
	KDE_EXPORT KCModule *create_basket_config_notes(QWidget *parent, const char *)
	{
		NotesPage *page = new NotesPage(parent, "kcmbasket_config_notes");
		return page;
	}
}

extern "C"
{
	KDE_EXPORT KCModule *create_basket_config_apps(QWidget *parent, const char *)
	{
		AppsPage *page = new AppsPage(parent, "kcmbasket_config_apps");
		return page;
	}
}

extern "C"
{
	KDE_EXPORT KCModule *create_basket_config_features(QWidget *parent, const char *)
	{
		FeaturesPage *page = new FeaturesPage(parent, "kcmbasket_config_features");
		return page;
	}
}

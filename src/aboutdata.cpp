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

#include "aboutdata.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

static const char description[] = I18N_NOOP(
	"<p><b>Taking care of your ideas.</b></p>"
	"<p>A note-taking application that makes it easy to record ideas as you think, and quickly find them later. "
	"Organizing your notes has never been so easy.</p>");

	// Or how to make order of disorganized toughts.

AboutData::AboutData()
 : KAboutData( "basket", I18N_NOOP("BasKet Note Pads"),
   VERSION, description, KAboutData::License_GPL_V2,
   "(c) 2003-2007, S\303\251bastien Lao\303\273t", 0,
   "http://basket.kde.org/",
   "http://basket.kde.org/bugs/" )
{
	addAuthor( "Kelvie Wong",
		   I18N_NOOP("Maintainer"),
		   "kelvie@ieee.org" );

	addAuthor( "S\303\251bastien Lao\303\273t",
	           I18N_NOOP("Original Author"),
	           "slaout@linux62.org" );

	addAuthor( "Petri Damst\303\251n",
	           I18N_NOOP("Basket encryption, Kontact integration, KnowIt importer"),
	           "damu@iki.fi" );

	addAuthor( "Alex Gontmakher",
	           I18N_NOOP("Baskets auto lock, save-status icon, HTML copy/paste, basket name tooltip, drop to basket name"),
	           "gsasha@cs.technion.ac.il" );

	addAuthor( "Marco Martin",
	           I18N_NOOP("Icon"),
	           "m4rt@libero.it" );
}

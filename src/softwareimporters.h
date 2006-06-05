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

#ifndef SOFTWAREIMPORTERS_H
#define SOFTWAREIMPORTERS_H

#include <qnamespace.h>

class QString;

class Basket;

/** Functions that import data from other softwares.
  * @author S�astien Laot
  */
namespace SoftwareImporters
{
	// Useful methods to design importers:
	QString fromICS(const QString &ics);
	QString fromTomboy(QString tomboy);
	void insertTitledNote(Basket *parent, const QString &title, const QString &content, Qt::TextFormat format = Qt::PlainText);
	void finishImport(Basket *basket);

	// The importers in themselves:
	void importKNotes();
	void importKJots();
	void importStickyNotes();
	void importTomboy();
  /** @author Petri Damsten <petri.damsten@iki.fi>
   */
	void importKnowIt();
}

#endif // SOFTWAREIMPORTERS_H

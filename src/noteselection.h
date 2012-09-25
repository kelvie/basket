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

#ifndef NOTESELECTION_H
#define NOTESELECTION_H

#include <QtCore/QList>
#include <QtCore/QString>

class Note;

/** This represent a hierarchy of the selected classes.
  * If this is null, then there is no selected note.
  */
class NoteSelection
{
public:
    NoteSelection()        : note(0), parent(0), firstChild(0), next(0), fullPath() {}
    NoteSelection(Note *n) : note(n), parent(0), firstChild(0), next(0), fullPath() {}

    Note          *note;
    NoteSelection *parent;
    NoteSelection *firstChild;
    NoteSelection *next;
    QString        fullPath; // Needeed for 'Cut' code to store temporary path of the cut note.

    NoteSelection* firstStacked();
    NoteSelection* nextStacked();
    void append(NoteSelection *node);
    int count();

    QList<Note*> parentGroups();
};

#endif // NOTESELECTION_H

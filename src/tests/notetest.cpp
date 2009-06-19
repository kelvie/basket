/*
 *   Copyright (C) 2009 by Matt Rogers <mattr@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <QObject>
#include <qtest_kde.h>

#include "note.h"

class NoteTest: public QObject
{
Q_OBJECT
public Q_SLOTS:
	void testCreation();
};

QTEST_KDEMAIN(NoteTest, NoGUI)

void NoteTest::testCreation()
{
	Note* n = new Note(0);
	QCOMPARE(n->basket(), 0);
	QCOMPARE(n->next(), 0);
	QCOMPARE(n->prev(), 0);
	QCOMPARE(n->x(), 0);
	QCOMPARE(n->y(), -1);
	QCOMPARE(n->width(), -1);
	QCOMPARE(n->height(), -1);
	delete n;
}

#include "notetest.moc"

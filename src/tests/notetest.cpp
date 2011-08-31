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
#include <QtTest/QtTest>
#include <qtest_kde.h>

#include "note.h"
#include "basketview.h"

class NoteTest: public QObject
{
Q_OBJECT
private Q_SLOTS:
    void testCreation();
};

QTEST_KDEMAIN(NoteTest, GUI)

void NoteTest::testCreation()
{
    Note* n = new Note(0);
    QVERIFY(n->basket() == 0);
    QVERIFY(n->next() == 0);
    QVERIFY(n->prev() == 0);
    QVERIFY(n->content() == 0);
    QCOMPARE(n->x(), 0.0);
    QCOMPARE(n->y(), 0.0);
    QCOMPARE(n->width(), Note::GROUP_WIDTH);
    QCOMPARE(n->height(), Note::MIN_HEIGHT);
    delete n;
}

#include "notetest.moc"
/* vim: set et sts=4 sw=4 ts=8 tw=0 : */

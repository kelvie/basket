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

class BasketViewTest: public QObject
{
Q_OBJECT
private Q_SLOTS:
    void testCreation();
};

QTEST_KDEMAIN(BasketViewTest, GUI)

void BasketViewTest::testCreation()
{

}
#include "basketviewtest.moc"
/* vim: set et sts=4 sw=4 ts=8 tw=0 : */

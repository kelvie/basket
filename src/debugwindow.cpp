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

#include <QLayout>
#include <QTextBrowser>
#include <QString>
#include <QEvent>
//Added by qt3to4:
#include <QCloseEvent>
#include <QVBoxLayout>
#include <klocale.h>

#include "global.h"
#include "debugwindow.h"

DebugWindow::DebugWindow(QWidget *parent)
 : QWidget(parent)
{
	Global::debugWindow = this;
	setWindowTitle(i18n("Debug Window"));

	layout      = new QVBoxLayout(this);
	textBrowser = new QTextBrowser(this);

	textBrowser->setWordWrapMode(QTextOption::NoWrap);

	layout->addWidget(textBrowser);
	textBrowser->show();
}

DebugWindow::~DebugWindow()
{
	delete textBrowser;
	delete layout;
}

void DebugWindow::postMessage(const QString msg)
{
	textBrowser->append(msg);
}

DebugWindow& DebugWindow::operator<<(const QString msg)
{
	textBrowser->append(msg);
	return *this;
}

void DebugWindow::insertHLine()
{
	textBrowser->append("<hr>");
}

void DebugWindow::closeEvent(QCloseEvent *event)
{
	Global::debugWindow = 0L;
	QWidget::closeEvent(event);
}


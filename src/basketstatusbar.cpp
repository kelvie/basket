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

#include <kparts/statusbarextension.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kdebug.h>
#include <qlabel.h>
#include <qobjectlist.h>
#include "basketstatusbar.h"
#include "clickablelabel.h"
#include "global.h"
#include "bnpview.h"
#include "basket.h"
#include "tools.h"
#include <kiconloader.h>
#include <qtooltip.h>

BasketStatusBar::BasketStatusBar(KStatusBar *bar)
	: m_bar(bar), m_extension(0), m_selectionStatus(0), m_lockStatus(0), m_basketStatus(0)
{
}

BasketStatusBar::BasketStatusBar(KParts::StatusBarExtension *extension)
	: m_bar(0), m_extension(extension), m_selectionStatus(0), m_lockStatus(0), m_basketStatus(0)
{
}

BasketStatusBar::~BasketStatusBar()
{
	//delete m_extension;
}

KStatusBar *BasketStatusBar::statusBar () const
{
	if(m_extension)
		return m_extension->statusBar();
	else
		return m_bar;
}

void BasketStatusBar::addWidget(QWidget * widget, int stretch, bool permanent)
{
	if(m_extension)
		m_extension->addStatusBarItem(widget, stretch, permanent);
	else
		m_bar->addWidget(widget, stretch, permanent);
}

void BasketStatusBar::setupStatusBar()
{
	QWidget* parent = statusBar();
	QObjectList* lst = parent->queryList("KRSqueezedTextLabel");

	//Tools::printChildren(parent);
	if(lst->count() == 0)
	{
		m_basketStatus = new QLabel(parent);
		m_basketStatus->setSizePolicy( QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored, 0, 0, false) );
		addWidget( m_basketStatus, 1, false ); // Fit all extra space and is hiddable
	}
	else
		m_basketStatus = static_cast<QLabel*>(lst->at(0));
	delete lst;

	m_selectionStatus = new QLabel(parent);
	addWidget( m_selectionStatus, 0, true );

	m_lockStatus = new ClickableLabel(0/*this*/);
	m_lockStatus->setMinimumSize(18, 18);
	m_lockStatus->setAlignment(Qt::AlignCenter);
//	addWidget( m_lockStatus, 0, true );
	connect( m_lockStatus, SIGNAL(clicked()), Global::bnpView, SLOT(lockBasket()) );

	m_savedStatusIconSet = SmallIconSet("filesave");
	m_savedStatus = new QLabel(parent);
	m_savedStatus->setPixmap(m_savedStatusIconSet.pixmap(QIconSet::Small, QIconSet::Disabled));
	//m_savedStatus->setEnabled(false);
	addWidget( m_savedStatus, 0, true );
	QToolTip::add(m_savedStatus, "<p>" + i18n("Shows if there are changes that have not been saved yet."));
}

void BasketStatusBar::postStatusbarMessage(const QString& text)
{
	if(statusBar())
		statusBar()->message(text, 2000);
}

void BasketStatusBar::setStatusText(const QString &txt)
{
	if(m_basketStatus && m_basketStatus->text() != txt)
		m_basketStatus->setText(txt);
}

void BasketStatusBar::setStatusBarHint(const QString &hint)
{
	if (hint.isEmpty())
		updateStatusBarHint();
	else
		setStatusText(hint);
}

void BasketStatusBar::updateStatusBarHint()
{
	QString message = "";

	if (Global::bnpView->currentBasket()->isDuringDrag())
		message = i18n("Ctrl+drop: copy, Shift+drop: move, Shift+Ctrl+drop: link.");
// Too much noise information:
//	else if (currentBasket()->inserterShown() && currentBasket()->inserterSplit() && !currentBasket()->inserterGroup())
//		message = i18n("Click to insert a note, right click for more options. Click on the right of the line to group instead of insert.");
//	else if (currentBasket()->inserterShown() && currentBasket()->inserterSplit() && currentBasket()->inserterGroup())
//		message = i18n("Click to group a note, right click for more options. Click on the left of the line to group instead of insert.");
	else if (Global::debugWindow)
		message = "DEBUG: " + Global::bnpView->currentBasket()->folderName();

	setStatusText(message);
}

void BasketStatusBar::setLockStatus(bool isLocked)
{
	if(!m_lockStatus)
		return;

	if (isLocked) {
		m_lockStatus->setPixmap(SmallIcon("encrypted.png"));
		QToolTip::add(m_lockStatus, i18n(
				"<p>This basket is <b>locked</b>.<br>Click to unlock it.</p>").replace(" ", "&nbsp;") );
//		QToolTip::add(m_lockStatus, i18n("This basket is locked.\nClick to unlock it."));
	} else {
		m_lockStatus->clear();
		QToolTip::add(m_lockStatus, i18n(
				"<p>This basket is <b>unlocked</b>.<br>Click to lock it.</p>").replace(" ", "&nbsp;") );
//		QToolTip::add(m_lockStatus, i18n("This basket is unlocked.\nClick to lock it."));
	}
}

void BasketStatusBar::setSelectionStatus(const QString &s)
{
	if(m_selectionStatus)
		m_selectionStatus->setText(s);
}

void BasketStatusBar::setUnsavedStatus(bool isUnsaved)
{
	//m_savedStatus->setEnabled(isUnsaved);
	m_savedStatus->setPixmap(m_savedStatusIconSet.pixmap(QIconSet::Small, isUnsaved));
}

#include "basketstatusbar.moc"

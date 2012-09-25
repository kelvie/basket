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
#ifndef BASKETSTATUSBAR_H
#define BASKETSTATUSBAR_H

#include <QtCore/QObject>
#include <QtGui/QPixmap>

#include "basket_export.h"

class KStatusBar;
namespace KParts
{
    class StatusBarExtension;
}

class QWidget;
class QLabel;

/**
    @author Sébastien Laoût <slaout@linux62.org>
*/
class BASKET_EXPORT BasketStatusBar : public QObject
{
    Q_OBJECT
public:
    BasketStatusBar(KStatusBar *bar);
    BasketStatusBar(KParts::StatusBarExtension *extension);
    ~BasketStatusBar();

public slots:
    /** GUI Main Window actions **/
    void setStatusBarHint(const QString &hint); /// << Set a specific message or update if hint is empty
    void updateStatusBarHint();                 /// << Display the current state message (dragging, editing) or reset the startsbar message
    void postStatusbarMessage(const QString &text);
    void setSelectionStatus(const QString &s);
    void setLockStatus(bool isLocked);
    void setupStatusBar();
    void setUnsavedStatus(bool isUnsaved);

protected:
    KStatusBar *statusBar() const;
    void addWidget(QWidget * widget, int stretch = 0, bool permanent = false);
    void setStatusText(const QString &txt);
    bool eventFilter(QObject * obj, QEvent * event);
private:
    KStatusBar                 *m_bar;
    KParts::StatusBarExtension *m_extension;
    QLabel                     *m_selectionStatus;
    QLabel                     *m_lockStatus;
    QLabel                     *m_basketStatus;
    QLabel                     *m_savedStatus;
    QPixmap                     m_savedStatusPixmap;
};

#endif

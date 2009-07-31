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

#ifndef DISKERRORDIALOG_H
#define DISKERRORDIALOG_H

#include <KDialog>

class QCloseEvent;
class QKeyEvent;

/** Provide a dialog to avert the user the disk is full.
  * This dialog is modal and is shown until the user has made space on the disk.
  * @author Sébastien Laoût
  */
class DiskErrorDialog : public KDialog
{
    Q_OBJECT
public:
    DiskErrorDialog(const QString &titleMessage, const QString &message, QWidget *parent = 0);
    ~DiskErrorDialog();
protected:
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent*);
};

#endif // DISKERRORDIALOG_H

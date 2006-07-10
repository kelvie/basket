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

#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <qlabel.h>

/** This class is a QLabel that can emit a clicked() signal when clicked !
  * @author S�astien Laot
  */
class ClickableLabel : public QLabel
{
  Q_OBJECT
  public:
	/** Construtor, initializer and destructor */
	ClickableLabel(QWidget *parent = 0, const char *name = 0)
	 : QLabel(parent, name) {}
	~ClickableLabel()       {}
  signals:
	void clicked();
  protected:
	virtual void mousePressEvent(QMouseEvent *event);
};

#endif // CLICKABLELABEL_H

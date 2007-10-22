/***************************************************************************
 *   Copyright (C) 2003 by S�astien Lao�t                                 *
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

#ifndef BASKETPROPERTIES_H
#define BASKETPROPERTIES_H

#include <kdialog.h>
#include <qmap.h>
#include <qstring.h>

class QLineEdit;
class QComboBox;
class QButtonGroup;
class KIntNumInput;
class KKeySequenceWidget;
class KShortcut;

class Basket;
class KColorCombo2;
class KIconButton;
/** The dialog that hold basket settings.
  * @author S�astien Lao�t
  */
class BasketPropertiesDialog : public KDialog
{
  Q_OBJECT
  public:
	BasketPropertiesDialog(Basket *basket, QWidget *parent = 0);
	~BasketPropertiesDialog();
	void polish();
	void applyChanges();
  protected slots:
	void slotApply();
	void slotOk();
	void capturedShortcut(const KShortcut &shortcut);
	void selectColumnsLayout();
  private:
	Basket        *m_basket;
	KIconButton   *m_icon;
	QLineEdit     *m_name;
	QComboBox     *m_backgroundImage;
	KColorCombo2  *m_backgroundColor;
	KColorCombo2  *m_textColor;
	QButtonGroup *m_disposition;
	KIntNumInput  *m_columnCount;
	KKeySequenceWidget    *m_shortcut;
	QButtonGroup *m_shortcutRole;
	QMap<int, QString> m_backgroundImagesMap;
};

#endif // BASKETPROPERTIES_H

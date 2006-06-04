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

#ifndef NEWBASKETDIALOG_H
#define NEWBASKETDIALOG_H

#include <kdialogbase.h>
#include <kiconview.h>
#include <qmap.h>

class KIconButton;
class QLineEdit;
class QDragObject;
class QListViewItem;

class Basket;

class KColorCombo2;

/** The class KIconView allow to drag items. We don't want to, so we disable it.
  * This class also unselect the selected item when the user right click an empty space. We don't want to, so we reselect it if that happens.
  * @author Sébastien Laoût
  */
class SingleSelectionKIconView : public KIconView
{
  Q_OBJECT
  public:
	SingleSelectionKIconView(QWidget *parent = 0, const char *name = 0, WFlags f = 0);
	QDragObject* dragObject();
	QIconViewItem* selectedItem() { return m_lastSelected; }
  private slots:
	void slotSelectionChanged(QIconViewItem *item);
	void slotSelectionChanged();
  private:
	QIconViewItem *m_lastSelected;
};



/** The dialog to create a new basket from a template.
  * @author Sébastien Laoût
  */
class NewBasketDialog : public KDialogBase
{
  Q_OBJECT
  public:
	NewBasketDialog(Basket *parentBasket, QWidget *parent = 0);
	~NewBasketDialog();
	void polish();
  protected slots:
	void slotOk();
	void returnPressed();
	void manageTemplates();
  private:
	int populateBasketsList(QListViewItem *item, int indent, int index);
	KIconButton        *m_icon;
	QLineEdit          *m_name;
	KColorCombo2       *m_backgroundColor;
	KIconView          *m_templates;
	QComboBox          *m_createIn;
	QMap<int, Basket*>  m_basketsMap;
};

#endif // NEWBASKETDIALOG_H

/***************************************************************************
 *   Copyright (C) 2003 by S�bastien Lao�t                                 *
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

#include <KDialog>
#include <k3iconview.h>
#include <qmap.h>

class KIconButton;
class QLineEdit;
class Q3DragObject;
class Q3ListViewItem;
class QComboBox;

class Basket;

class KColorCombo2;

/** The class K3IconView allow to drag items. We don't want to, so we disable it.
  * This class also unselect the selected item when the user right click an empty space. We don't want to, so we reselect it if that happens.
  * @author S�bastien Lao�t
  */
class SingleSelectionKIconView : public K3IconView
{
  Q_OBJECT
  public:
	SingleSelectionKIconView(QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0);
	Q3DragObject* dragObject();
	Q3IconViewItem* selectedItem() { return m_lastSelected; }
  private slots:
	void slotSelectionChanged(Q3IconViewItem *item);
	void slotSelectionChanged();
  private:
	Q3IconViewItem *m_lastSelected;
};

/** Struct to store default properties of a new basket.
  * When the dialog shows up, the @p icon is used, as well as the @p backgroundColor.
  * A template is choosen depending on @p freeLayout and @p columnLayout.
  * If @p columnLayout is too high, the template with the more columns will be chosen instead.
  * If the user change the background color in the dialog, then @p backgroundImage and @p textColor will not be used!
  * @author S�bastien Lao�t
  */
struct NewBasketDefaultProperties
{
	QString icon;
	QString backgroundImage;
	QColor  backgroundColor;
	QColor  textColor;
	bool    freeLayout;
	int     columnCount;

	NewBasketDefaultProperties();
};

/** The dialog to create a new basket from a template.
  * @author S�bastien Lao�t
  */
class NewBasketDialog : public KDialog
{
  Q_OBJECT
  public:
	NewBasketDialog(Basket *parentBasket, const NewBasketDefaultProperties &defaultProperties, QWidget *parent = 0);
	~NewBasketDialog();
	void polish();
  protected slots:
	void slotOk();
	void returnPressed();
	void manageTemplates();
	void nameChanged(const QString &newName);
  private:
	int populateBasketsList(Q3ListViewItem *item, int indent, int index);
	NewBasketDefaultProperties  m_defaultProperties;
	KIconButton                *m_icon;
	QLineEdit                  *m_name;
	KColorCombo2               *m_backgroundColor;
	K3IconView                  *m_templates;
	QComboBox                  *m_createIn;
	QMap<int, Basket*>          m_basketsMap;
};

#endif // NEWBASKETDIALOG_H

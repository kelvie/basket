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

#include <KDialog>
#include <QListWidget>
#include <QTreeWidgetItem>
#include <qmap.h>

class KIconButton;
class QLineEdit;
class QMimeData;
class Q3ListViewItem;
class QComboBox;

class Basket;

class KColorCombo2;

/** The class QListWidget allow to drag items. We don't want to, so we disable it.
  * This class also unselect the selected item when the user right click an empty space. We don't want to, so we reselect it if that happens.
  * @author Sébastien Laoût
  */
class SingleSelectionKIconView : public QListWidget
{
  Q_OBJECT
  public:
	SingleSelectionKIconView(QWidget *parent = 0);
	QMimeData* dragObject();
	QListWidgetItem* selectedItem() { return m_lastSelected; }
  private slots:
	void slotSelectionChanged(QListWidgetItem *cur);
  private:
	QListWidgetItem *m_lastSelected;
};

/** Struct to store default properties of a new basket.
  * When the dialog shows up, the @p icon is used, as well as the @p backgroundColor.
  * A template is choosen depending on @p freeLayout and @p columnLayout.
  * If @p columnLayout is too high, the template with the more columns will be chosen instead.
  * If the user change the background color in the dialog, then @p backgroundImage and @p textColor will not be used!
  * @author Sébastien Laoût
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
  * @author Sébastien Laoût
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
	int populateBasketsList(QTreeWidgetItem *item, int indent, int index);
	NewBasketDefaultProperties  m_defaultProperties;
	KIconButton                *m_icon;
	QLineEdit                  *m_name;
	KColorCombo2               *m_backgroundColor;
	QListWidget                 *m_templates;
	QComboBox                  *m_createIn;
	QMap<int, Basket*>          m_basketsMap;
};

#endif // NEWBASKETDIALOG_H

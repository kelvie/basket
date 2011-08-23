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

#ifndef BASKETPROPERTIES_H
#define BASKETPROPERTIES_H

#include <KDE/KDialog>
#include <QtCore/QMap>

#include "ui_basketproperties.h"

class KIconButton;
class KLineEdit;
class QGroupBox;
class QVBoxLayout;
class QRadioButton;
class QString;

class KComboBox;
class KIntNumInput;
class KShortcutWidget;
class KShortcut;
class KColorCombo2;

class BasketView;

/** The dialog that hold basket settings.
  * @author Sébastien Laoût
  */
class BasketPropertiesDialog : public KDialog, private Ui::BasketPropertiesUi
{
    Q_OBJECT
public:
    explicit BasketPropertiesDialog(BasketView *basket, QWidget *parent = 0);
    ~BasketPropertiesDialog();
    void ensurePolished();

public slots:
    void applyChanges();

protected slots:
    void capturedShortcut(const KShortcut &shortcut);
    void selectColumnsLayout();

private:
    BasketView    *m_basket;
    KIconButton   *m_icon;
    KLineEdit     *m_name;
    KComboBox     *m_backgroundImage;
    KColorCombo2  *m_backgroundColor;
    KColorCombo2  *m_textColor;
    QMap<int, QString> m_backgroundImagesMap;
};

#endif // BASKETPROPERTIES_H

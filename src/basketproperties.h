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
#include <QMap>
#include <QString>

class KIconButton;
class QLineEdit;
class QComboBox;
class QGroupBox;
class QVBoxLayout;
class QRadioButton;
class KIntNumInput;
class KShortcutWidget;
class KShortcut;

class BasketView;
class KColorCombo2;

/** The dialog that hold basket settings.
  * @author Sébastien Laoût
  */
class BasketPropertiesDialog : public KDialog
{
    Q_OBJECT
public:
    BasketPropertiesDialog(BasketView *basket, QWidget *parent = 0);
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
    QLineEdit     *m_name;
    QComboBox     *m_backgroundImage;
    KColorCombo2  *m_backgroundColor;
    KColorCombo2  *m_textColor;
    QGroupBox     *m_disposition;
    QRadioButton  *columnForm;
    QRadioButton  *mindMap;
    QRadioButton  *freeForm;
    KIntNumInput  *m_columnCount;
    KShortcutWidget *m_shortcut;
    QVBoxLayout *m_shortcutRoleLayout;
    QRadioButton * m_showButton;
    QRadioButton * m_globalButton;
    QRadioButton * m_switchButton;
    QMap<int, QString> m_backgroundImagesMap;
};

#endif // BASKETPROPERTIES_H

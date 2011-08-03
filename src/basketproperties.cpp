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

#include <QLineEdit>
#include <QComboBox>
//Added by qt3to4:
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPixmap>
#include <QVBoxLayout>
#include <KDE/KNumInput>
#include <kshortcutwidget.h>
#include <QLayout>
#include <QLabel>
#include <QRadioButton>
#include <QGroupBox>
#include <QButtonGroup>
#include <QStringList>
#include <KDE/KLocale>
#include <QStyle>
#include <KDE/KApplication>
#include <KDE/KIconLoader>
#include <KDE/KIconDialog>

#include "basketproperties.h"
#include "basketview.h"
#include "kcolorcombo2.h"
#include "variouswidgets.h"
#include "global.h"
#include "backgroundmanager.h"

#include "ui_basketproperties.h"

BasketPropertiesDialog::BasketPropertiesDialog(BasketView *basket, QWidget *parent)
        : KDialog(parent)
        , Ui::BasketPropertiesUi()
        , m_basket(basket)
{
    QWidget *mainWidget = new QWidget(this);
    setupUi(mainWidget);
    setMainWidget(mainWidget);
  
    // Set up dialog options
    setCaption(i18n("Basket Properties"));
    setButtons(Ok | Apply | Cancel);
    setDefaultButton(Ok);
    setObjectName("BasketProperties");
    setModal(true);
    showButtonSeparator(false);

    icon->setIconType(KIconLoader::NoGroup, KIconLoader::Action);
    icon->setIconSize(16);
    icon->setIcon(m_basket->icon());
    
    int size = qMax(icon->sizeHint().width(), icon->sizeHint().height());
    icon->setFixedSize(size, size); // Make it square!
    icon->setToolTip(i18n("Icon"));
    name->setText(m_basket->basketName());
    name->setMinimumWidth(name->fontMetrics().maxWidth()*20);
    name->setToolTip(i18n("Name"));

    // Appearance:    
    m_backgroundColor = new KColorCombo2(m_basket->backgroundColorSetting(), palette().color(QPalette::Base), appearanceGroup);
    m_textColor       = new KColorCombo2(m_basket->textColorSetting(),       palette().color(QPalette::Text), appearanceGroup);
    
    bgColorLbl->setBuddy(m_backgroundColor);
    txtColorLbl->setBuddy(m_textColor);
    
    appearanceLayout->addWidget(m_backgroundColor, 1, 2);
    appearanceLayout->addWidget(m_textColor, 2, 2);
    
    setTabOrder(backgroundImage, m_backgroundColor);
    setTabOrder(m_backgroundColor, m_textColor);
    setTabOrder(m_textColor, columnForm);
    
    backgroundImage->addItem(i18n("(None)"));
    m_backgroundImagesMap.insert(0, "");
    backgroundImage->setIconSize(QSize(100, 75));
    QStringList backgrounds = Global::backgroundManager->imageNames();
    int index = 1;
    for (QStringList::Iterator it = backgrounds.begin(); it != backgrounds.end(); ++it) {
        QPixmap *preview = Global::backgroundManager->preview(*it);
        if (preview) {
            m_backgroundImagesMap.insert(index, *it);
            backgroundImage->insertItem(index, *it);
            backgroundImage->setItemData(index, *preview, Qt::DecorationRole);
            if (m_basket->backgroundImageName() == *it)
                backgroundImage->setCurrentIndex(index);
            index++;
        }
    }
//  m_backgroundImage->insertItem(i18n("Other..."), -1);
    int BUTTON_MARGIN = kapp->style()->pixelMetric(QStyle::PM_ButtonMargin);
    backgroundImage->setMaxVisibleItems(50/*75 * 6 / m_backgroundImage->sizeHint().height()*/);
    backgroundImage->setMinimumHeight(75 + 2 * BUTTON_MARGIN);

    // Disposition:

    columnCount->setValue(m_basket->columnsCount());
    columnCount->setRange(1, 20, /*step=*/1);
    columnCount->setSliderEnabled(false);
    columnCount->setValue(m_basket->columnsCount());
    connect(columnCount, SIGNAL(valueChanged(int)), this, SLOT(selectColumnsLayout()));

    int height = qMax(mindMap->sizeHint().height(), columnCount->sizeHint().height()); // Make all radioButtons vertically equaly-spaced!
    mindMap->setMinimumSize(mindMap->sizeHint().width(), height); // Because the m_columnCount can be heigher, and make radio1 and radio2 more spaced than radio2 and radio3.
    
    if (!m_basket->isFreeLayout())
        columnForm->setChecked(true);
    else if (m_basket->isMindMap())
        mindMap->setChecked(true);
    else
        freeForm->setChecked(true);
    
    mindMap->hide();
  
    // Keyboard Shortcut:
    shortcut->setShortcut(m_basket->shortcut());

    HelpLabel *helpLabel = new HelpLabel(i18n("Learn some tips..."), i18n(
                                             "<p><strong>Easily Remember your Shortcuts</strong>:<br>"
                                             "With the first option, giving the basket a shortcut of the form <strong>Alt+Letter</strong> will underline that letter in the basket tree.<br>"
                                             "For instance, if you are assigning the shortcut <i>Alt+T</i> to a basket named <i>Tips</i>, the basket will be displayed as <i><u>T</u>ips</i> in the tree. "
                                             "It helps you visualize the shortcuts to remember them more quickly.</p>"
                                             "<p><strong>Local vs Global</strong>:<br>"
                                             "The first option allows to show the basket while the main window is active. "
                                             "Global shortcuts are valid from anywhere, even if the window is hidden.</p>"
                                             "<p><strong>Show vs Switch</strong>:<br>"
                                             "The last option makes this basket the current one without opening the main window. "
                                             "It is useful in addition to the configurable global shortcuts, eg. to paste the clipboard or the selection into the current basket from anywhere.</p>"), 0);

    shortcutLayout->addWidget(helpLabel);
    connect(shortcut, SIGNAL(shortcutChanged(const KShortcut&)), this, SLOT(capturedShortcut(const KShortcut&)));
    
    setTabOrder(columnCount, shortcut);
    setTabOrder(shortcut, helpLabel);
    setTabOrder(helpLabel, showBasket);
    
    switch (m_basket->shortcutAction()) {
        default:
        case 0: showBasket->setChecked(true); break;
        case 1: globalButton->setChecked(true); break;
        case 2: switchButton->setChecked(true); break;
    }

   
    // Connect the Ok and Apply buttons to actually apply the changes
    connect(this, SIGNAL(okClicked()), SLOT(applyChanges()));
    connect(this, SIGNAL(applyClicked()), SLOT(applyChanges()));
}

BasketPropertiesDialog::~BasketPropertiesDialog()
{
}

void BasketPropertiesDialog::ensurePolished()
{
    ensurePolished();
    name->setFocus();
}

void BasketPropertiesDialog::applyChanges()
{
    if (columnForm->isChecked()) {
        m_basket->setDisposition(0, columnCount->value());
    } else if (freeForm->isChecked()) {
        m_basket->setDisposition(1, columnCount->value());
    } else {
        m_basket->setDisposition(2, columnCount->value());
    }

    if (showBasket->isChecked()) {
        m_basket->setShortcut(shortcut->shortcut(), 0);
    } else if (globalButton->isChecked()) {
        m_basket->setShortcut(shortcut->shortcut(), 1);
    } else if (switchButton->isChecked()) {
        m_basket->setShortcut(shortcut->shortcut(), 2);
    }

    // Should be called LAST, because it will emit the propertiesChanged() signal and the tree will be able to show the newly set Alt+Letter shortcut:
    m_basket->setAppearance(icon->icon(), name->text(), m_backgroundImagesMap[backgroundImage->currentIndex()], m_backgroundColor->color(), m_textColor->color());
    m_basket->save();
}

void BasketPropertiesDialog::capturedShortcut(const KShortcut &sc)
{
    // TODO: Validate it!
    shortcut->setShortcut(sc);
}

void BasketPropertiesDialog::selectColumnsLayout()
{
    columnForm->setChecked(true);
}


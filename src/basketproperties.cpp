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

#include "basketproperties.h"

#include <QtCore/QStringList>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QPixmap>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QGroupBox>
#include <QtGui/QButtonGroup>
#include <QtGui/QStyle>

#include <KDE/KComboBox>
#include <KDE/KLineEdit>
#include <KDE/KLocale>
#include <KDE/KNumInput>
#include <KDE/KApplication>
#include <KDE/KIconLoader>
#include <KDE/KIconDialog>
#include <kshortcutWidget.h>

#include "basketscene.h"
#include "kcolorcombo2.h"
#include "variouswidgets.h"
#include "global.h"
#include "backgroundmanager.h"

#include "ui_basketproperties.h"

BasketPropertiesDialog::BasketPropertiesDialog(BasketScene *basket, QWidget *parent)
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

    QWidget *page = new QWidget(this);
    QVBoxLayout *topLayout = new QVBoxLayout(page);

    // Icon and Name:
    QHBoxLayout *nameLayout = new QHBoxLayout(0);
    int margin = marginHint() * 2 / 3;
    nameLayout->setContentsMargins(margin, margin, margin, margin);
    m_icon = new KIconButton(page);
    m_icon->setIconType(KIconLoader::NoGroup, KIconLoader::Action);
    m_icon->setIconSize(16);
    m_icon->setIcon(m_basket->icon());
    int size = qMax(m_icon->sizeHint().width(), m_icon->sizeHint().height());
    m_icon->setFixedSize(size, size); // Make it square!
    m_icon->setToolTip(i18n("Icon"));
    m_name = new KLineEdit(m_basket->basketName(), page);
    m_name->setMinimumWidth(m_name->fontMetrics().maxWidth()*20);
    m_name->setToolTip(i18n("Name"));
    nameLayout->addWidget(m_icon);
    nameLayout->addWidget(m_name);
    topLayout->addLayout(nameLayout);

    // Appearance:
    QGroupBox *appearance = new QGroupBox(i18n("Appearance"), page);
    QVBoxLayout* appearanceLayout = new QVBoxLayout;
    appearance->setLayout(appearanceLayout);
    QWidget *appearanceWidget = new QWidget;
    appearanceLayout->addWidget(appearanceWidget);

    //QGridLayout *grid = new QGridLayout(appearanceWidget, /*nRows=*/3, /*nCols=*/2, /*margin=*/0, spacingHint());
    QGridLayout *grid = new QGridLayout(appearanceWidget);

    m_backgroundImage = new KComboBox(appearanceWidget);
    m_backgroundColor = new KColorCombo2(m_basket->backgroundColorSetting(), palette().color(QPalette::Base), appearanceWidget);
    m_textColor       = new KColorCombo2(m_basket->textColorSetting(),       palette().color(QPalette::Text), appearanceWidget);

    QLabel *label1 = new QLabel(appearanceWidget);
    label1->setBuddy(m_backgroundImage);
    label1->setText(i18n("Background &image:"));

    QLabel *label2 = new QLabel(appearanceWidget);
    label2->setBuddy(m_backgroundColor);
    label2->setText(i18n("&Background color:"));

    QLabel *label3 = new QLabel(appearanceWidget);
    label3->setBuddy(m_textColor);
    label3->setText(i18n("&Text color:"));

    grid->addWidget(label1,            0, 0, Qt::AlignVCenter);
    grid->addWidget(label2,            1, 0, Qt::AlignVCenter);
    grid->addWidget(label3,            2, 0, Qt::AlignVCenter);
    grid->addWidget(m_backgroundImage, 0, 1, Qt::AlignVCenter);
    grid->addWidget(m_backgroundColor, 1, 1, Qt::AlignVCenter);
    grid->addWidget(m_textColor,       2, 1, Qt::AlignVCenter);
    topLayout->addWidget(appearance);

    m_backgroundImage->addItem(i18n("(None)"));
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


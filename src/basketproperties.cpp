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
#include "KDE/KIconDialog"

#include "basketproperties.h"
#include "basketview.h"
#include "kcolorcombo2.h"
#include "variouswidgets.h"
#include "global.h"
#include "backgroundmanager.h"

BasketPropertiesDialog::BasketPropertiesDialog(BasketView *basket, QWidget *parent)
        : KDialog(parent)
        , m_basket(basket)
{
    // Set up dialog options
    setCaption(i18n("Basket Properties"));
    setButtons(Ok | Apply | Cancel);
    setDefaultButton(Ok);
    setObjectName("BasketProperties");
    setModal(true);
    showButtonSeparator(false);

    QWidget *page = new QWidget(this);
    QTabWidget *tabWidget = new QTabWidget(this);
    tabWidget->addTab(page, i18n("Basic"));

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
    m_name = new QLineEdit(m_basket->basketName(), page);
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

    m_backgroundImage = new QComboBox(appearanceWidget);
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
    m_backgroundImage->setIconSize(QSize(100, 75));
    QStringList backgrounds = Global::backgroundManager->imageNames();
    int index = 1;
    for (QStringList::Iterator it = backgrounds.begin(); it != backgrounds.end(); ++it) {
        QPixmap *preview = Global::backgroundManager->preview(*it);
        if (preview) {
            m_backgroundImagesMap.insert(index, *it);
            m_backgroundImage->insertItem(index, *it);
            m_backgroundImage->setItemData(index, *preview, Qt::DecorationRole);
            if (m_basket->backgroundImageName() == *it)
                m_backgroundImage->setCurrentIndex(index);
            index++;
        }
    }
//  m_backgroundImage->insertItem(i18n("Other..."), -1);
    int BUTTON_MARGIN = kapp->style()->pixelMetric(QStyle::PM_ButtonMargin);
    m_backgroundImage->setMaxVisibleItems(50/*75 * 6 / m_backgroundImage->sizeHint().height()*/);
    m_backgroundImage->setMinimumHeight(75 + 2 * BUTTON_MARGIN);

    // Disposition:
    m_disposition = new QGroupBox(i18n("Disposition"), page);
    QButtonGroup* bg = new QButtonGroup(m_disposition);

    QVBoxLayout* dispLayout = new QVBoxLayout;
    m_disposition->setLayout(dispLayout);

    QHBoxLayout *colCountLayout = new QHBoxLayout(m_disposition);
    columnForm = new QRadioButton(i18n("Col&umns:"), m_disposition);
    colCountLayout->addWidget(columnForm);
    bg->addButton(columnForm);


    m_columnCount = new KIntNumInput(m_basket->columnsCount(), m_disposition);
    m_columnCount->setRange(1, 20, /*step=*/1);
    m_columnCount->setSliderEnabled(false);
    m_columnCount->setValue(m_basket->columnsCount());
    connect(m_columnCount, SIGNAL(valueChanged(int)), this, SLOT(selectColumnsLayout()));
    dispLayout->addLayout(colCountLayout);
    colCountLayout->addWidget(m_columnCount);

    freeForm = new QRadioButton(i18n("&Free-form"), m_disposition);
    dispLayout->addWidget(freeForm);
    bg->addButton(freeForm);

    mindMap = new QRadioButton(i18n("&Mind map"), m_disposition); // TODO: "Learn more..."
    dispLayout->addWidget(mindMap);
    bg->addButton(mindMap);

    int height = qMax(mindMap->sizeHint().height(), m_columnCount->sizeHint().height()); // Make all radioButtons vertically equaly-spaced!
    mindMap->setMinimumSize(mindMap->sizeHint().width(), height); // Because the m_columnCount can be heigher, and make radio1 and radio2 more spaced than radio2 and radio3.
    if (!m_basket->isFreeLayout())
        columnForm->setChecked(true);
    else if (m_basket->isMindMap())
        mindMap->setChecked(true);
    else
        freeForm->setChecked(true);
    topLayout->addWidget(m_disposition);

    mindMap->hide();

    QWidget *page2 = new QWidget(this);
    tabWidget->addTab(page2, i18n("&Keyboard Shortcut"));

    // Keyboard Shortcut:
    m_shortcutRoleLayout = new QVBoxLayout;
    QWidget *shortcutWidget = new QWidget;
    m_shortcutRoleLayout->addWidget(shortcutWidget);
    QHBoxLayout *shortcutLayout = new QHBoxLayout(shortcutWidget);
    m_shortcut = new KShortcutWidget(shortcutWidget);
    m_shortcut->setShortcut(m_basket->shortcut());
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
                                             "It is useful in addition to the configurable global shortcuts, eg. to paste the clipboard or the selection into the current basket from anywhere.</p>"),
                                         shortcutWidget);
    shortcutLayout->addWidget(m_shortcut);
    shortcutLayout->addStretch();
    shortcutLayout->addWidget(helpLabel);
    connect(m_shortcut, SIGNAL(shortcutChanged(const KShortcut&)), this, SLOT(capturedShortcut(const KShortcut&)));
    m_showButton = new QRadioButton(i18n("S&how this basket"));
    m_globalButton = new QRadioButton(i18n("Show this basket (&global shortcut)"));
    m_switchButton = new QRadioButton(i18n("S&witch to this basket (global shortcut)"));
    m_shortcutRoleLayout->addWidget(m_showButton);
    m_shortcutRoleLayout->addWidget(m_globalButton);
    m_shortcutRoleLayout->addWidget(m_switchButton);
    switch (m_basket->shortcutAction()) {
    default:
    case 0: m_showButton->setChecked(true); break;
    case 1: m_globalButton->setChecked(true); break;
    case 2: m_switchButton->setChecked(true); break;
    }

    page2->setLayout(m_shortcutRoleLayout);

    topLayout->addSpacing(marginHint());
    topLayout->addStretch(10);

    // Connect the Ok and Apply buttons to actually apply the changes
    connect(this, SIGNAL(okClicked()), this, SLOT(applyChanges()));
    connect(this, SIGNAL(applyClicked()), this, SLOT(applyChanges()));

    setMainWidget(tabWidget);
}

BasketPropertiesDialog::~BasketPropertiesDialog()
{
}

void BasketPropertiesDialog::ensurePolished()
{
    ensurePolished();
    m_name->setFocus();
}

void BasketPropertiesDialog::applyChanges()
{
    if (columnForm->isChecked()) {
        m_basket->setDisposition(0, m_columnCount->value());
    } else if (freeForm->isChecked()) {
        m_basket->setDisposition(1, m_columnCount->value());
    } else {
        m_basket->setDisposition(2, m_columnCount->value());
    }

    if (m_showButton->isChecked()) {
        m_basket->setShortcut(m_shortcut->shortcut(), 0);
    } else if (m_globalButton->isChecked()) {
        m_basket->setShortcut(m_shortcut->shortcut(), 1);
    } else if (m_switchButton->isChecked()) {
        m_basket->setShortcut(m_shortcut->shortcut(), 2);
    }

    // Should be called LAST, because it will emit the propertiesChanged() signal and the tree will be able to show the newly set Alt+Letter shortcut:
    m_basket->setAppearance(m_icon->icon(), m_name->text(), m_backgroundImagesMap[m_backgroundImage->currentIndex()], m_backgroundColor->color(), m_textColor->color());
    m_basket->save();
}

void BasketPropertiesDialog::capturedShortcut(const KShortcut &shortcut)
{
    // TODO: Validate it!
    m_shortcut->setShortcut(shortcut);
}

void BasketPropertiesDialog::selectColumnsLayout()
{
    columnForm->setChecked(true);
}


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
#include <QHBoxLayout>
#include <QPixmap>
#include <QVBoxLayout>
#include <QLabel>
#include <KDE/KLocale>
#include <KDE/KPushButton>
#include <KDE/KGuiItem>
#include <KDE/KMessageBox>
#include <QSize>
#include <QPainter>
#include <KApplication>
#include <KDE/KIconLoader>
#include <KDE/KMainWindow>

#include <QComboBox>

#include "kicondialog.h"
#include "newbasketdialog.h"
#include "basketfactory.h"
#include "basket.h"
#include "basketlistview.h"
#include "variouswidgets.h"
#include "kcolorcombo2.h"
#include "tools.h"
#include "global.h"
#include "bnpview.h"

/** class SingleSelectionKIconView: */

SingleSelectionKIconView::SingleSelectionKIconView(QWidget *parent)
        : QListWidget(parent), m_lastSelected(0)
{
    setViewMode(QListView::IconMode);
    connect(this, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(slotSelectionChanged(QListWidgetItem*)));
}

QMimeData* SingleSelectionKIconView::dragObject()
{
    return 0;
}

void SingleSelectionKIconView::slotSelectionChanged(QListWidgetItem *cur)
{
    if (cur)
        m_lastSelected = cur;
}

/** class NewBasketDefaultProperties: */

NewBasketDefaultProperties::NewBasketDefaultProperties()
        : icon("")
        , backgroundImage("")
        , backgroundColor()
        , textColor()
        , freeLayout(false)
        , columnCount(1)
{
}

/** class NewBasketDialog: */

NewBasketDialog::NewBasketDialog(Basket *parentBasket, const NewBasketDefaultProperties &defaultProperties, QWidget *parent)
        : KDialog(parent)
        , m_defaultProperties(defaultProperties)
{
    // KDialog options
    setCaption(i18n("New Basket"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setObjectName("NewBasket");
    setModal(true);
    showButtonSeparator(true);
    connect(this, SIGNAL(okClicked()), SLOT(slotOk()));

    QWidget *page = new QWidget(this);
    QVBoxLayout *topLayout = new QVBoxLayout(page);

    // Icon, Name and Background Color:
    QHBoxLayout *nameLayout = new QHBoxLayout;
    //QHBoxLayout *nameLayout = new QHBoxLayout(this);
    m_icon = new KIconButton(page);
    m_icon->setIconType(KIconLoader::NoGroup, KIconLoader::Action);
    m_icon->setIconSize(16);
    m_icon->setIcon(m_defaultProperties.icon.isEmpty() ? "basket" : m_defaultProperties.icon);

    int size = qMax(m_icon->sizeHint().width(), m_icon->sizeHint().height());
    m_icon->setFixedSize(size, size); // Make it square!

    m_icon->setToolTip(i18n("Icon"));
    m_name = new QLineEdit(/*i18n("Basket"), */page);
    m_name->setMinimumWidth(m_name->fontMetrics().maxWidth()*20);
    connect(m_name, SIGNAL(textChanged(const QString&)), this, SLOT(nameChanged(const QString&)));
    enableButtonOk(false);
    m_name->setToolTip(i18n("Name"));
    m_backgroundColor = new KColorCombo2(QColor(), palette().color(QPalette::Base), page);
    m_backgroundColor->setColor(QColor());
    m_backgroundColor->setFixedSize(m_backgroundColor->sizeHint());
    m_backgroundColor->setColor(m_defaultProperties.backgroundColor);
    m_backgroundColor->setToolTip(i18n("Background color"));
    nameLayout->addWidget(m_icon);
    nameLayout->addWidget(m_name);
    nameLayout->addWidget(m_backgroundColor);
    topLayout->addLayout(nameLayout);

    QHBoxLayout *layout = new QHBoxLayout;
    KPushButton *button = new KPushButton(KGuiItem(i18n("&Manage Templates..."), "configure"), page);
    connect(button, SIGNAL(clicked()), this, SLOT(manageTemplates()));
    button->hide();

    // Compute the right template to use as the default:
    QString defaultTemplate = "free";
    if (!m_defaultProperties.freeLayout) {
        if (m_defaultProperties.columnCount == 1)
            defaultTemplate = "1column";
        else if (m_defaultProperties.columnCount == 2)
            defaultTemplate = "2columns";
        else
            defaultTemplate = "3columns";
    }

    // Empty:
    // * * * * *
    // Personnal:
    // *To Do
    // Professionnal:
    // *Meeting Summary
    // Hobbies:
    // *
    m_templates = new SingleSelectionKIconView(page);
    m_templates->setSelectionMode(QAbstractItemView::SingleSelection);
    QListWidgetItem *lastTemplate = 0;
    QPixmap icon(40, 53);

    QPainter painter(&icon);
    painter.fillRect(0, 0, icon.width(), icon.height(), palette().color(QPalette::Base));
    painter.setPen(palette().color(QPalette::Text));
    painter.drawRect(0, 0, icon.width(), icon.height());
    painter.end();
    lastTemplate = new QListWidgetItem(icon, i18n("One column"), m_templates);

    if (defaultTemplate == "1column")
        m_templates->setCurrentItem(lastTemplate);

    painter.begin(&icon);
    painter.fillRect(0, 0, icon.width(), icon.height(), palette().color(QPalette::Base));
    painter.setPen(palette().color(QPalette::Text));
    painter.drawRect(0, 0, icon.width(), icon.height());
    painter.drawLine(icon.width() / 2, 0, icon.width() / 2, icon.height());
    painter.end();
    lastTemplate = new QListWidgetItem(icon, i18n("Two columns"), m_templates);

    if (defaultTemplate == "2columns")
        m_templates->setCurrentItem(lastTemplate);

    painter.begin(&icon);
    painter.fillRect(0, 0, icon.width(), icon.height(), palette().color(QPalette::Base));
    painter.setPen(palette().color(QPalette::Text));
    painter.drawRect(0, 0, icon.width(), icon.height());
    painter.drawLine(icon.width() / 3, 0, icon.width() / 3, icon.height());
    painter.drawLine(icon.width() * 2 / 3, 0, icon.width() * 2 / 3, icon.height());
    painter.end();
    lastTemplate = new QListWidgetItem(icon, i18n("Three columns"), m_templates);

    if (defaultTemplate == "3columns")
        m_templates->setCurrentItem(lastTemplate);

    painter.begin(&icon);
    painter.fillRect(0, 0, icon.width(), icon.height(), palette().color(QPalette::Base));
    painter.setPen(palette().color(QPalette::Text));
    painter.drawRect(0, 0, icon.width(), icon.height());
    painter.drawRect(icon.width() / 5, icon.width() / 5, icon.width() / 4, icon.height() / 8);
    painter.drawRect(icon.width() * 2 / 5, icon.width() * 2 / 5, icon.width() / 4, icon.height() / 8);
    painter.end();
    lastTemplate = new QListWidgetItem(icon, i18n("Free"), m_templates);

    if (defaultTemplate == "free")
        m_templates->setCurrentItem(lastTemplate);

    m_templates->setMinimumHeight(topLayout->minimumSize().width() * 9 / 16);

    QLabel *label = new QLabel(page);
    label->setText(i18n("&Template:"));
    label->setBuddy(m_templates);
    layout->addWidget(label, /*stretch=*/0, Qt::AlignBottom);
    layout->addStretch();
    layout->addWidget(button, /*stretch=*/0, Qt::AlignBottom);
    topLayout->addLayout(layout);
    topLayout->addWidget(m_templates);

    layout = new QHBoxLayout;
    m_createIn = new QComboBox(page);
    m_createIn->addItem(i18n("(Baskets)"));
    label = new QLabel(page);
    label->setText(i18n("C&reate in:"));
    label->setBuddy(m_createIn);
    HelpLabel *helpLabel = new HelpLabel(i18n("How is it useful?"), i18n(
                                             "<p>Creating baskets inside of other baskets to form a hierarchy allows you to be more organized by eg.:</p><ul>"
                                             "<li>Grouping baskets by themes or topics;</li>"
                                             "<li>Grouping baskets in folders for different projects;</li>"
                                             "<li>Making sections with sub-baskets representing chapters or pages;</li>"
                                             "<li>Making a group of baskets to export together (to eg. email them to people).</li></ul>"), page);
    layout->addWidget(label);
    layout->addWidget(m_createIn);
    layout->addWidget(helpLabel);
    layout->addStretch();
    topLayout->addLayout(layout);

    m_basketsMap.clear();
    int index;
    m_basketsMap.insert(/*index=*/0, /*basket=*/0L);
    index = 1;
    for (int i = 0; i < Global::bnpView->topLevelItemCount(); i++) {
        index = populateBasketsList(Global::bnpView->topLevelItem(i), /*indent=*/1, /*index=*/index);
    }

    connect(m_templates, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotOk()));
    connect(m_templates, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(returnPressed()));

    setMainWidget(page);

    if (parentBasket) {
        int index = 0;

        for (QMap<int, Basket*>::Iterator it = m_basketsMap.begin(); it != m_basketsMap.end(); ++it) {
            if (it.value() == parentBasket) {
                index = it.key();
                break;
            }
        }

        if (index <= 0)
            return;

        if (m_createIn->currentIndex() != index)
            m_createIn->setCurrentIndex(index);
    }

    m_name->setFocus();
}

void NewBasketDialog::returnPressed()
{
    button(Ok)->animateClick();
}

int NewBasketDialog::populateBasketsList(QTreeWidgetItem *item, int indent, int index)
{
    static const int ICON_SIZE = 16;
    // Get the basket data:
    Basket* basket = ((BasketListViewItem *)item)->basket();
    QPixmap icon = KIconLoader::global()->loadIcon(
                       basket->icon(), KIconLoader::NoGroup, ICON_SIZE,
                       KIconLoader::DefaultState, QStringList(), 0L,
                       /*canReturnNull=*/false
                   );
    icon = Tools::indentPixmap(icon, indent, 2 * ICON_SIZE / 3);
    m_createIn->addItem(icon, basket->basketName());
    m_basketsMap.insert(index, basket);
    ++index;

    for (int i = 0; i < item->childCount(); i++) {
        // Append children of item to the list:
        index = populateBasketsList(item->child(i), indent + 1, index);
    }

    return index;
}

NewBasketDialog::~NewBasketDialog()
{
}

void NewBasketDialog::ensurePolished()
{
    KDialog::ensurePolished();
    m_name->setFocus();
}

void NewBasketDialog::nameChanged(const QString &newName)
{
    enableButtonOk(!newName.isEmpty());
}

void NewBasketDialog::slotOk()
{
    QListWidgetItem *item = ((SingleSelectionKIconView*)m_templates)->selectedItem();
    QString templateName;
    if (!item)
        return;
    if (item->text() == i18n("One column"))
        templateName = "1column";
    if (item->text() == i18n("Two columns"))
        templateName = "2columns";
    if (item->text() == i18n("Three columns"))
        templateName = "3columns";
    if (item->text() == i18n("Free-form"))
        templateName = "free";
    if (item->text() == i18n("Mind map"))
        templateName = "mindmap";

    Global::bnpView->closeAllEditors();

    QString backgroundImage;
    QColor  textColor;
    if (m_backgroundColor->color() == m_defaultProperties.backgroundColor) {
        backgroundImage = m_defaultProperties.backgroundImage;
        textColor       = m_defaultProperties.textColor;
    }

    BasketFactory::newBasket(m_icon->icon(), m_name->text(), backgroundImage, m_backgroundColor->color(), textColor, templateName, m_basketsMap[m_createIn->currentIndex()]);

    if (Global::mainWindow()) Global::mainWindow()->show();

}

void NewBasketDialog::manageTemplates()
{
    KMessageBox::information(this, "Wait a minute! There is no template for now: they will come with time... :-D");
}


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

#include <kicondialog.h>
#include <qlineedit.h>
#include <kiconview.h>
#include <qlayout.h>
#include <qlabel.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kpushbutton.h>
#include <kguiitem.h>
#include <kmessagebox.h>
#include <qsize.h>
#include <qpainter.h>
#include <kapplication.h>
#include <kiconloader.h>

#include "newbasketdialog.h"
#include "basketfactory.h"
#include "container.h"
#include "basket.h"
#include "variouswidgets.h"
#include "kcolorcombo2.h"
#include "tools.h"
#include "global.h"

/** class SingleSelectionKIconView: */

SingleSelectionKIconView::SingleSelectionKIconView(QWidget *parent, const char *name, WFlags f)
 : KIconView(parent, name, f), m_lastSelected(0)
{
	connect( this, SIGNAL(selectionChanged(QIconViewItem*)), this, SLOT(slotSelectionChanged(QIconViewItem*)) );
	connect( this, SIGNAL(selectionChanged()),               this, SLOT(slotSelectionChanged())               );
}

QDragObject* SingleSelectionKIconView::dragObject()
{
	return 0;
}

void SingleSelectionKIconView::slotSelectionChanged(QIconViewItem *item)
{
	if (item)
		m_lastSelected = item;
}

void SingleSelectionKIconView::slotSelectionChanged()
{
	if (m_lastSelected && !m_lastSelected->isSelected())
		m_lastSelected->setSelected(true);
}

/** class NewBasketDialog: */

NewBasketDialog::NewBasketDialog(Basket *parentBasket, QWidget *parent)
 : KDialogBase(KDialogBase::Swallow, i18n("New Basket"), KDialogBase::Ok | KDialogBase::Cancel,
               KDialogBase::Ok, parent, /*name=*/0, /*modal=*/true, /*separator=*/true)
{
	QWidget *page = new QWidget(this);
	QVBoxLayout *topLayout = new QVBoxLayout(page, /*margin=*/0, spacingHint());

	// Icon, Name and Background Color:
	QHBoxLayout *nameLayout = new QHBoxLayout(0, marginHint()*2/3, spacingHint());
	m_icon = new KIconButton(page);
	m_icon->setIconType(KIcon::NoGroup, KIcon::Action);
	m_icon->setIconSize(16);
	m_icon->setIcon("basket");
	int size = QMAX(m_icon->sizeHint().width(), m_icon->sizeHint().height());
	m_icon->setFixedSize(size, size); // Make it square!
	QToolTip::add(m_icon, i18n("Icon"));
	m_name = new QLineEdit(i18n("Basket"), page);
	m_name->setMinimumWidth(m_name->fontMetrics().maxWidth()*20);
	QToolTip::add(m_name, i18n("Name"));
	m_backgroundColor = new KColorCombo2(QColor(), KGlobalSettings::baseColor(), page);
	QToolTip::add(m_backgroundColor, i18n("Background color"));
	nameLayout->addWidget(m_icon);
	nameLayout->addWidget(m_name);
	nameLayout->addWidget(m_backgroundColor);
	topLayout->addLayout(nameLayout);

	QHBoxLayout *layout = new QHBoxLayout(/*parent=*/0, /*margin=*/0, spacingHint());
	KPushButton *button = new KPushButton( KGuiItem(i18n("&Manage Templates..."), "configure"), page );
	connect( button, SIGNAL(clicked()), this, SLOT(manageTemplates()) );

	// Empty:
	// * * * * *
	// Personnal:
	// *To Do
	// Professionnal:
	// *Meeting Summary
	// Hobbies:
	// *
	m_templates = new SingleSelectionKIconView(page);
	m_templates->setItemsMovable(false);
	m_templates->setMode(KIconView::Select);
	m_templates->setGridX(m_templates->maxItemWidth());
	KIconViewItem *lastTemplate = 0;
	QPixmap icon(40, 53);
	QPainter painter(&icon);
	painter.fillRect(0, 0, icon.width(), icon.height(), KGlobalSettings::baseColor());
	painter.setPen(KGlobalSettings::textColor());
	painter.drawRect(0, 0, icon.width(), icon.height());
	painter.end();
	lastTemplate = new KIconViewItem(m_templates, lastTemplate, i18n("One column"), icon);
	m_templates->setSelected(lastTemplate, true);
	painter.begin(&icon);
	painter.fillRect(0, 0, icon.width(), icon.height(), KGlobalSettings::baseColor());
	painter.setPen(KGlobalSettings::textColor());
	painter.drawRect(0, 0, icon.width(), icon.height());
	painter.drawLine(icon.width() / 2, 0, icon.width() / 2, icon.height());
	painter.end();
	lastTemplate = new KIconViewItem(m_templates, lastTemplate, i18n("Two columns"), icon);
	painter.begin(&icon);
	painter.fillRect(0, 0, icon.width(), icon.height(), KGlobalSettings::baseColor());
	painter.setPen(KGlobalSettings::textColor());
	painter.drawRect(0, 0, icon.width(), icon.height());
	painter.drawLine(icon.width() / 3, 0, icon.width() / 3, icon.height());
	painter.drawLine(icon.width() * 2 / 3, 0, icon.width() * 2 / 3, icon.height());
	painter.end();
	lastTemplate = new KIconViewItem(m_templates, lastTemplate, i18n("Three columns"), icon);
	painter.begin(&icon);
	painter.fillRect(0, 0, icon.width(), icon.height(), KGlobalSettings::baseColor());
	painter.setPen(KGlobalSettings::textColor());
	painter.drawRect(0, 0, icon.width(), icon.height());
	painter.end();
	lastTemplate = new KIconViewItem(m_templates, lastTemplate, i18n("Free"), icon);
/*	painter.begin(&icon);
	painter.fillRect(0, 0, icon.width(), icon.height(), KGlobalSettings::baseColor());
	painter.setPen(KGlobalSettings::textColor());
	painter.drawRect(0, 0, icon.width(), icon.height());
	painter.drawRect(icon.width() * 2 / 5, icon.height() * 3 / 7, icon.width() / 5, icon.height() / 7);
	painter.end();
	lastTemplate = new KIconViewItem(m_templates, lastTemplate, i18n("Mind map"), icon);*/
	m_templates->setMinimumHeight(topLayout->minimumSize().width() * 9 / 16);

	QLabel *label = new QLabel(m_templates, i18n("&Template:"), page);
	layout->addWidget(label, /*stretch=*/0, Qt::AlignBottom);
	layout->addStretch();
	layout->addWidget(button, /*stretch=*/0, Qt::AlignBottom);
	topLayout->addLayout(layout);
	topLayout->addWidget(m_templates);

	layout = new QHBoxLayout(/*parent=*/0, /*margin=*/0, spacingHint());
	m_createIn = new QComboBox(page);
	m_createIn->insertItem(i18n("(Baskets)"));
	label = new QLabel(m_createIn, i18n("C&reate in:"), page);
	HelpLabel *helpLabel = new HelpLabel(i18n("How is it useful?"), i18n(
		"<p>Creating baskets inside of other baskets to form a hierarchy allows you to be more organized by eg.:</p><ul>"
		"<li>Groupping baskets by themes or topics;</li>"
		"<li>Groupping baskets in folders for different projects;</li>"
		"<li>Making sections with sub-baskets representing chapters or pages;</li>"
		"<li>Making group of baskets to export together (to eg. email them to people).</li></ul>"), page);
	layout->addWidget(label);
	layout->addWidget(m_createIn);
	layout->addWidget(helpLabel);
	layout->addStretch();
	topLayout->addLayout(layout);

	m_basketsMap.clear();
	m_basketsMap.insert(/*index=*/0, /*basket=*/0L);
	populateBasketsList(Global::basketTree->firstListViewItem(), /*indent=*/1, /*index=*/1);

	connect( m_templates, SIGNAL(doubleClicked(QIconViewItem*)), this, SLOT(slotOk())        );
	connect( m_templates, SIGNAL(returnPressed(QIconViewItem*)), this, SLOT(returnPressed()) );

	if (parentBasket) {
		int index = 0;

		for (QMap<int, Basket*>::Iterator it = m_basketsMap.begin(); it != m_basketsMap.end(); ++it)
			if (it.data() == parentBasket) {
				index = it.key();
				break;
			}
		if (index <= 0)
			return;

		if (m_createIn->currentItem() != index)
			m_createIn->setCurrentItem(index);
	}

	setMainWidget(page);
}

void NewBasketDialog::returnPressed()
{
	actionButton(KDialogBase::Ok)->animateClick();
}

int NewBasketDialog::populateBasketsList(QListViewItem *item, int indent, int index)
{
	static const int ICON_SIZE = 16;

	while (item) {
		// Get the basket data:
		Basket *basket = ((BasketListViewItem*)item)->basket();
		QPixmap icon = kapp->iconLoader()->loadIcon(basket->icon(), KIcon::NoGroup, ICON_SIZE, KIcon::DefaultState, 0L, /*canReturnNull=*/false);
		icon = Tools::indentPixmap(icon, indent, 2 * ICON_SIZE / 3);

		// Append item to the list:
		m_createIn->insertItem(icon, basket->basketName());
		m_basketsMap.insert(index, basket);
		++index;

		// Append childs of item to the list:
		index = populateBasketsList(item->firstChild(), indent + 1, index);

		// Add next sibling basket:
		item = item->nextSibling();
	}

	return index;
}

NewBasketDialog::~NewBasketDialog()
{
}

void NewBasketDialog::polish()
{
	KDialogBase::polish();
	m_name->setFocus();
}

void NewBasketDialog::slotOk()
{
	QIconViewItem *item = ((SingleSelectionKIconView*)m_templates)->selectedItem();
	QString templateName;
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

	Global::basketTree->closeAllEditors();

	BasketFactory::newBasket(m_icon->icon(), m_name->text(), m_backgroundColor->color(), templateName, m_basketsMap[m_createIn->currentItem()]);
	Global::mainContainer->show();

	KDialogBase::slotOk();
}

void NewBasketDialog::manageTemplates()
{
	KMessageBox::information(this, "Wait a minute! There is no template for now: they will come with time... :-D");
}

#include "newbasketdialog.moc"

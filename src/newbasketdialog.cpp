/***************************************************************************
 *   Copyright (C) 2003 by S�astien Laot                                 *
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
#include <k3iconview.h>
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
#include <kmainwindow.h>
#include <kdebug.h>
#include <kcolorscheme.h>

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

SingleSelectionKIconView::SingleSelectionKIconView(QWidget *parent, const char *name, Qt::WFlags f)
//TODO : KListWidget(parent, name, f), m_lastSelected(0)
{
	connect( this, SIGNAL(selectionChanged(QTreeWidgetItem*)), this, SLOT(slotSelectionChanged(QTreeWidgetItem*)) );
	connect( this, SIGNAL(selectionChanged()),               this, SLOT(slotSelectionChanged())               );
}

QMimeData* SingleSelectionKIconView::dragObject()
{
//TODO	return new QMimeData(this);
}

void SingleSelectionKIconView::slotSelectionChanged(QTreeWidgetItem *item)
{
//	if (item)
//		m_lastSelected = item;
}

void SingleSelectionKIconView::slotSelectionChanged()
{
//	if (m_lastSelected && !m_lastSelected->isSelected())
//		m_lastSelected->setSelected(true);
}

/** class NewBasketDefaultProperties: */

NewBasketDefaultProperties::NewBasketDefaultProperties()
// : icon("")
// , backgroundImage("")
// , backgroundColor()
// , textColor()
// , freeLayout(false)
// , columnCount(1)
{
}

/** class NewBasketDialog: */

NewBasketDialog::NewBasketDialog(Basket *parentBasket, const NewBasketDefaultProperties &defaultProperties, QWidget *parent)
 //: KDialog(KDialog::Swallow, i18n("New Basket"), KDialog::Ok | KDialog::Cancel,
 //              KDialog::Ok, parent, /*name=*/"NewBasket", /*modal=*/true, /*separator=*/true)
	: KDialog(parent)
	, m_defaultProperties(defaultProperties)
{
	this->setCaption( i18n("New Basket") );
	this->setButtons( KDialog::Ok | KDialog::Cancel );
	this->setModal( true );

	QWidget *page = new QWidget(this);
	//FIXME QVBoxLayout *topLayout = new QVBoxLayout(page, /*margin=*/0, spacingHint());
	QVBoxLayout *topLayout = new QVBoxLayout(page);

	// Icon, Name and Background Color:
	// QHBoxLayout(0, marginHin() * 2 / 3, spacingHint());
	QHBoxLayout *nameLayout = new QHBoxLayout();
	m_icon = new KIconButton(page);
	m_icon->setIconType(KIconLoader::NoGroup, KIconLoader::Action);
	m_icon->setIconSize(16);
	m_icon->setIcon(m_defaultProperties.icon.isEmpty() ? "basket" : m_defaultProperties.icon);

	//FIXME int size = qMax(m_icon->sizeHint().width(), m_icon->sizeHint().height());
	int size = 32;
	m_icon->setFixedSize(size, size); // Make it square!
	m_icon->setToolTip( i18n("Icon") );
	m_name = new QLineEdit( page );
	m_name->setMinimumWidth(m_name->fontMetrics().maxWidth()*20);
	connect( m_name, SIGNAL(textChanged(const QString&)), this, SLOT(nameChanged(const QString&)) );
	enableButtonOk(false);

	m_name->setToolTip( i18n("Name") );
//	m_backgroundColor = new KColorCombo2(QColor(), KColorScheme(KColorScheme::View).background().color(), page);
//	m_backgroundColor->setColor(QColor());
//	m_backgroundColor->setFixedSize(m_backgroundColor->sizeHint());
//	m_backgroundColor->setColor(m_defaultProperties.backgroundColor);
//	QToolTip::add(m_backgroundColor, i18n("Background color"));

	nameLayout->addWidget(m_icon);
	nameLayout->addWidget(m_name);
	//FIXME nameLayout->addWidget(m_backgroundColor);
	topLayout->addLayout(nameLayout);

	//FIXME QHBoxLayout *layout = new QHBoxLayout(/*parent=*/0, /*margin=*/0, spacingHint());
	QHBoxLayout *layout = new QHBoxLayout();
	/*KPushButton *button = new KPushButton( KGuiItem(i18n("&Manage Templates..."), "configure"), page );
	connect( button, SIGNAL(clicked()), this, SLOT(manageTemplates()) );
	button->hide();*/

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
	//FIXME m_templates = new SingleSelectionKIconView(page);
	m_templates = new SingleSelectionKIconView(page);
	//m_templates->setItemsMovable(false);
	//m_templates->setMode(K3IconView::Select);
	//m_templates->setGridX(m_templates->maxItemWidth());
	//K3IconViewItem *lastTemplate = 0;
	
	QPixmap icon(40, 53);
	QPainter painter(&icon);
	painter.fillRect(0, 0, icon.width(), icon.height(), KColorScheme(QPalette::Active, KColorScheme::View).background());
	//painter.setPen( KColorScheme(QPalette::Active, KColorScheme::View).foreground() );
	painter.drawRect( 0, 0, icon.width(), icon.height() );
	painter.end();

	

//	lastTemplate = new K3IconViewItem(m_templates, lastTemplate, i18n("One column"), icon);
//
//	if (defaultTemplate == "1column")
//		m_templates->setSelected(lastTemplate, true);

	painter.begin(&icon);
	painter.fillRect(0, 0, icon.width(), icon.height(), KColorScheme(QPalette::Active, KColorScheme::View).background() );
	//painter.setPen( KColorScheme(QPalette::Active, KColorScheme::View).foreground() );
	painter.drawRect(0, 0, icon.width(), icon.height());
	painter.drawLine(icon.width() / 2, 0, icon.width() / 2, icon.height());
	painter.end();

	//lastTemplate = new K3IconViewItem(m_templates, lastTemplate, i18n("Two columns"), icon);
//
//	if (defaultTemplate == "2columns")
//		m_templates->setSelected(lastTemplate, true);
//
//	painter.begin(&icon);
//	painter.fillRect(0, 0, icon.width(), icon.height(), KColorScheme(KColorScheme::View).background().color());
//	painter.setPen(KColorScheme(KColorScheme::View).foreground().color());
//	painter.drawRect(0, 0, icon.width(), icon.height());
//	painter.drawLine(icon.width() / 3, 0, icon.width() / 3, icon.height());
//	painter.drawLine(icon.width() * 2 / 3, 0, icon.width() * 2 / 3, icon.height());
//	painter.end();
//	lastTemplate = new K3IconViewItem(m_templates, lastTemplate, i18n("Three columns"), icon);
//
//	if (defaultTemplate == "3columns")
//		m_templates->setSelected(lastTemplate, true);
//
//	painter.begin(&icon);
//	painter.fillRect(0, 0, icon.width(), icon.height(), KColorScheme(KColorScheme::View).background().color());
//	painter.setPen(KColorScheme(KColorScheme::View).foreground().color());
//	painter.drawRect(0, 0, icon.width(), icon.height());
//	painter.drawRect(icon.width() / 5, icon.width() / 5, icon.width() / 4, icon.height() / 8);
//	painter.drawRect(icon.width() * 2 / 5, icon.width() * 2 / 5, icon.width() / 4, icon.height() / 8);
//	painter.end();
//	lastTemplate = new K3IconViewItem(m_templates, lastTemplate, i18n("Free"), icon);
//
//	if (defaultTemplate == "free")
//		m_templates->setSelected(lastTemplate, true);
//
///*	painter.begin(&icon);
//	painter.fillRect(0, 0, icon.width(), icon.height(), KColorScheme(KColorScheme::View).background().color());
//	painter.setPen(KColorScheme(KColorScheme::View).foreground().color());
//	painter.drawRect(0, 0, icon.width(), icon.height());
//	painter.drawRect(icon.width() * 2 / 5, icon.height() * 3 / 7, icon.width() / 5, icon.height() / 7);
//	painter.end();
//	lastTemplate = new K3IconViewItem(m_templates, lastTemplate, i18n("Mind map"), icon);*/

	m_templates->setMinimumHeight(topLayout->minimumSize().width() * 9 / 16);

	//FIXME QLabel *label = new QLabel(m_templates, i18n("&Template:"), page);
	QLabel *label = new QLabel( i18n("Template:"), page );
	layout->addWidget(label, /*stretch=*/0, Qt::AlignBottom);
	layout->addStretch();
	//layout->addWidget(button, /*stretch=*/0, Qt::AlignBottom);
	topLayout->addLayout(layout);
	topLayout->addWidget(m_templates);

//	layout = new QHBoxLayout(/*parent=*/0, /*margin=*/0, spacingHint());
	layout = new QHBoxLayout( page );
	m_createIn = new QComboBox( page );
	m_createIn->addItem(i18n("(Baskets)"));
//	label = new QLabel(m_createIn, i18n("C&reate in:"), page);
//	HelpLabel *helpLabel = new HelpLabel(i18n("How is it useful?"), i18n(
//		"<p>Creating baskets inside of other baskets to form a hierarchy allows you to be more organized by eg.:</p><ul>"
//		"<li>Grouping baskets by themes or topics;</li>"
//		"<li>Grouping baskets in folders for different projects;</li>"
//		"<li>Making sections with sub-baskets representing chapters or pages;</li>"
//		"<li>Making a group of baskets to export together (to eg. email them to people).</li></ul>"), page);
//	layout->addWidget(label);
	layout->addWidget(m_createIn);
//	layout->addWidget(helpLabel);
	layout->addStretch();
	topLayout->addLayout(layout);
//
//	m_basketsMap.clear();
//	m_basketsMap.insert(/*index=*/0, /*basket=*/0L);
//	populateBasketsList(Global::bnpView->firstListViewItem(), /*indent=*/1, /*index=*/1);

	connect( m_templates, SIGNAL(doubleClicked(QTreeWidgetItem*)), this, SLOT(slotOk())        );
	connect( m_templates, SIGNAL(returnPressed(QTreeWidgetItem*)), this, SLOT(returnPressed()) );

//	if (parentBasket) {
//		int index = 0;
//
//		for (QMap<int, Basket*>::Iterator it = m_basketsMap.begin(); it != m_basketsMap.end(); ++it)
//			if (it.data() == parentBasket) {
//				index = it.key();
//				break;
//			}
//		if (index <= 0)
//			return;
//
//		if (m_createIn->currentItem() != index)
//			m_createIn->setCurrentItem(index);
//	}
//
	setMainWidget(page);
}

void NewBasketDialog::returnPressed()
{
//	actionButton(KDialog::Ok)->animateClick();
}

int NewBasketDialog::populateBasketsList(QTreeWidget *item, int indent, int index)
{
//	static const int ICON_SIZE = 16;
//
//	while (item) {
//		// Get the basket data:
//		Basket *basket = ((BasketListViewItem*)item)->basket();
//		QPixmap icon = KIconLoader::global()->loadIcon(basket->icon(), K3Icon::NoGroup, ICON_SIZE, KIcon::DefaultState, 0L, /*canReturnNull=*/false);
//		icon = Tools::indentPixmap(icon, indent, 2 * ICON_SIZE / 3);
//
//		// Append item to the list:
//		m_createIn->insertItem(icon, basket->basketName());
//		m_basketsMap.insert(index, basket);
//		++index;
//
//		// Append childs of item to the list:
//		index = populateBasketsList(item->firstChild(), indent + 1, index);
//
//		// Add next sibling basket:
//		item = item->nextSibling();
//	}
//
//	return index;
}

NewBasketDialog::~NewBasketDialog()
{
}

void NewBasketDialog::polish()
{
//	KDialog::polish();
//	m_name->setFocus();
}

void NewBasketDialog::nameChanged(const QString &newName)
{
	enableButtonOk( !newName.isEmpty() );
}

void NewBasketDialog::slotOk()
{
//	QTreeWidgetItem *item = ((SingleSelectionKIconView*)m_templates)->selectedItem();
//	QString templateName;
//	if (item->text() == i18n("One column"))
//		templateName = "1column";
//	if (item->text() == i18n("Two columns"))
//		templateName = "2columns";
//	if (item->text() == i18n("Three columns"))
//		templateName = "3columns";
//	if (item->text() == i18n("Free-form"))
//		templateName = "free";
//	if (item->text() == i18n("Mind map"))
//		templateName = "mindmap";
//
//	Global::bnpView->closeAllEditors();
//
//	QString backgroundImage;
//	QColor  textColor;
//	if (m_backgroundColor->color() == m_defaultProperties.backgroundColor) {
//		backgroundImage = m_defaultProperties.backgroundImage;
//		textColor       = m_defaultProperties.textColor;
//	}
//
//	BasketFactory::newBasket(m_icon->icon(), m_name->text(), backgroundImage, m_backgroundColor->color(), textColor, templateName, m_basketsMap[m_createIn->currentItem()]);
//	if(Global::mainWindow()) Global::mainWindow()->show();
//
//	KDialog::slotOk();
}

void NewBasketDialog::manageTemplates()
{
//	KMessageBox::information(this, "Wait a minute! There is no template for now: they will come with time... :-D");
}

#include "newbasketdialog.moc"

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
#include <qcombobox.h>
#include <qvbuttongroup.h>
#include <knuminput.h>
#include <kkeybutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qstringlist.h>
#include <klocale.h>
#include <qstyle.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kglobalsettings.h>

#include "basketproperties.h"
#include "basket.h"
#include "kcolorcombo2.h"
#include "variouswidgets.h"
#include "global.h"
#include "backgroundmanager.h"

BasketPropertiesDialog::BasketPropertiesDialog(Basket *basket, QWidget *parent)
 : KDialog(KDialog::Swallow, i18n("Basket Properties"), KDialog::Ok | KDialog::Apply | KDialog::Cancel,
               KDialog::Ok, parent, /*name=*/"BasketProperties", /*modal=*/true, /*separator=*/false),
   m_basket(basket)
{
	QWidget *page = new QWidget(this);
	QVBoxLayout *topLayout = new QVBoxLayout(page, /*margin=*/0, spacingHint());

	// Icon and Name:
	QHBoxLayout *nameLayout = new QHBoxLayout(0, marginHint()*2/3, spacingHint());
	m_icon = new KIconButton(page);
	m_icon->setIconType(K3Icon::NoGroup, KIcon::Action);
	m_icon->setIconSize(16);
	m_icon->setIcon(m_basket->icon());
	int size = qMax(m_icon->sizeHint().width(), m_icon->sizeHint().height());
	m_icon->setFixedSize(size, size); // Make it square!
	QToolTip::add(m_icon, i18n("Icon"));
	m_name = new QLineEdit(m_basket->basketName(), page);
	m_name->setMinimumWidth(m_name->fontMetrics().maxWidth()*20);
	QToolTip::add(m_name, i18n("Name"));
	nameLayout->addWidget(m_icon);
	nameLayout->addWidget(m_name);
	topLayout->addLayout(nameLayout);

	// Appearance:
	QGroupBox *appearance = new QGroupBox(1, Qt::Horizontal, i18n("Appearance"), page);
	QWidget *appearanceWidget = new QWidget(appearance);
	QGridLayout *grid = new QGridLayout(appearanceWidget, /*nRows=*/3, /*nCols=*/2, /*margin=*/0, spacingHint());
	m_backgroundImage = new QComboBox(appearanceWidget);
	m_backgroundColor = new KColorCombo2(m_basket->backgroundColorSetting(), KColorScheme(KColorScheme::View).background().color(), appearanceWidget);
	m_textColor       = new KColorCombo2(m_basket->textColorSetting(),       KColorScheme(KColorScheme::View).foreground().color(), appearanceWidget);
		QLabel *label1 = new QLabel(m_backgroundImage, i18n("Background &image:"), appearanceWidget);
	QLabel *label2 = new QLabel(m_backgroundColor, i18n("&Background color:"), appearanceWidget);
	QLabel *label3 = new QLabel(m_textColor,       i18n("&Text color:"),       appearanceWidget);
	grid->addWidget(label1,            0, 0, Qt::AlignVCenter);
	grid->addWidget(label2,            1, 0, Qt::AlignVCenter);
	grid->addWidget(label3,            2, 0, Qt::AlignVCenter);
	grid->addWidget(m_backgroundImage, 0, 1, Qt::AlignVCenter);
	grid->addWidget(m_backgroundColor, 1, 1, Qt::AlignVCenter);
	grid->addWidget(m_textColor,       2, 1, Qt::AlignVCenter);
	topLayout->addWidget(appearance);

	m_backgroundImage->insertItem(i18n("(None)"), 0);
	m_backgroundImagesMap.insert(0, "");
	QStringList backgrounds = Global::backgroundManager->imageNames();
	int index = 1;
	for (QStringList::Iterator it = backgrounds.begin(); it != backgrounds.end(); ++it) {
		QPixmap *preview = Global::backgroundManager->preview(*it);
		if (preview) {
			m_backgroundImagesMap.insert(index, *it);
			m_backgroundImage->insertItem(*preview, index);
			if (m_basket->backgroundImageName() == *it)
				m_backgroundImage->setCurrentItem(index);
			index++;
		}
	}
//	m_backgroundImage->insertItem(i18n("Other..."), -1);
	int BUTTON_MARGIN = kapp->style().pixelMetric(QStyle::PM_ButtonMargin);
	m_backgroundImage->setSizeLimit(50/*75 * 6 / m_backgroundImage->sizeHint().height()*/);
	m_backgroundImage->setMinimumHeight(75 + 2 * BUTTON_MARGIN);

	// Disposition:
	m_disposition = new QVButtonGroup(i18n("Disposition"), page);
	QWidget *columnsWidget = new QWidget(m_disposition);
	QHBoxLayout *dispoLayout = new QHBoxLayout(columnsWidget, /*margin=*/0, spacingHint());
	QRadioButton *radio = new QRadioButton(i18n("Col&umns:"), columnsWidget);
	m_columnCount = new KIntNumInput(m_basket->columnsCount(), columnsWidget);
	m_columnCount->setRange(1, 20, /*step=*/1, /*slider=*/false);
	m_columnCount->setValue(m_basket->columnsCount());
	connect( m_columnCount, SIGNAL(valueChanged(int)), this, SLOT(selectColumnsLayout()) );
	dispoLayout->addWidget(radio);
	dispoLayout->addWidget(m_columnCount);
	m_disposition->insert(radio);
	new QRadioButton(i18n("&Free-form"), m_disposition);
	QRadioButton *mindMap = new QRadioButton(i18n("&Mind map"), m_disposition); // TODO: "Learn more..."
	int height = qMax(mindMap->sizeHint().height(), m_columnCount->sizeHint().height()); // Make all radioButtons vertically equaly-spaced!
	mindMap->setMinimumSize(mindMap->sizeHint().width(), height); // Because the m_columnCount can be heigher, and make radio1 and radio2 more spaced than radio2 and radio3.
	m_disposition->setButton(m_basket->isFreeLayout() ? (m_basket->isMindMap() ? 2 : 1) : 0);
	topLayout->addWidget(m_disposition);

	mindMap->hide();

	// Keyboard Shortcut:
	m_shortcutRole = new QVButtonGroup(i18n("&Keyboard Shortcut"), page);
	QWidget *shortcutWidget = new QWidget(m_shortcutRole);
	QHBoxLayout *shortcutLayout = new QHBoxLayout(shortcutWidget, /*margin=*/0, spacingHint());
	m_shortcut = new KKeyButton(shortcutWidget);
	m_shortcut->setShortcut(m_basket->shortcut(), /*bQtShortcut=*/true);
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
	connect( m_shortcut, SIGNAL(capturedShortcut(const KShortcut&)), this, SLOT(capturedShortcut(const KShortcut&)) );
	new QRadioButton(i18n("S&how this basket"),                        m_shortcutRole);
	new QRadioButton(i18n("Show this basket (&global shortcut)"),      m_shortcutRole);
	new QRadioButton(i18n("S&witch to this basket (global shortcut)"), m_shortcutRole);
	m_shortcutRole->setButton(m_basket->shortcutAction()/* + 1*/); // Id 0 is the KKeyButton!
	topLayout->addWidget(m_shortcutRole);

	topLayout->addSpacing(marginHint());
	topLayout->addStretch(10);

	setMainWidget(page);
}

BasketPropertiesDialog::~BasketPropertiesDialog()
{
}

void BasketPropertiesDialog::polish()
{
	KDialog::polish();
	m_name->setFocus();
}

void BasketPropertiesDialog::applyChanges()
{
	m_basket->setDisposition(m_disposition->selectedId(), m_columnCount->value());
	m_basket->setShortcut(m_shortcut->shortcut(), m_shortcutRole->selectedId());
	// Should be called LAST, because it will emit the propertiesChanged() signal and the tree will be able to show the newly set Alt+Letter shortcut:
	m_basket->setAppearance(m_icon->icon(), m_name->text(), m_backgroundImagesMap[m_backgroundImage->currentItem()], m_backgroundColor->color(), m_textColor->color());
	m_basket->save();
}

void BasketPropertiesDialog::slotApply()
{
	applyChanges();
	KDialog::slotApply();
}

void BasketPropertiesDialog::slotOk()
{
	applyChanges();
	KDialog::slotOk();
}

void BasketPropertiesDialog::capturedShortcut(const KShortcut &shortcut)
{
	// TODO: Validate it!
	m_shortcut->setShortcut(shortcut, /*bQtShortcut=*/true);
}

void BasketPropertiesDialog::selectColumnsLayout()
{
	m_disposition->setButton(0);
}

#include "basketproperties.moc"

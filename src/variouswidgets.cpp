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

#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qsizegrip.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qsizepolicy.h>

#include <KListWidget>

//Added by qt3to4:
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QList>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <KOpenWithDialog>
#include <klocale.h>
#include <QWhatsThis>
#include <kiconloader.h>
#include <QDrag>
#include <qfontdatabase.h>
#include <kpushbutton.h>

#include "variouswidgets.h"


/** class RunCommandRequester: */

RunCommandRequester::RunCommandRequester(const QString &runCommand, const QString &message, QWidget *parent)
 : QWidget(parent)
{
	m_message = message;

	QHBoxLayout *layout = new QHBoxLayout(this);
	m_runCommand        = new QLineEdit(runCommand, this);
	QPushButton *pb     = new QPushButton(/*"C&hoose..."*/i18n("..."), this);

	pb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	layout->addWidget(m_runCommand);
	layout->addWidget(pb);

	connect( pb, SIGNAL(clicked()), this, SLOT(slotSelCommand()) );
}

RunCommandRequester::~RunCommandRequester()
{
}

void RunCommandRequester::slotSelCommand()
{
	KOpenWithDialog *dlg =  new KOpenWithDialog(KUrl::List(), m_message, m_runCommand->text(), this);
	dlg->exec();
	if ( ! dlg->text().isEmpty() )
		m_runCommand->setText(dlg->text());
}

QString RunCommandRequester::runCommand()
{
	return m_runCommand->text();
}

void RunCommandRequester::setRunCommand(const QString &runCommand)
{
	m_runCommand->setText(runCommand);
}

/** class IconSizeCombo: */

IconSizeCombo::IconSizeCombo(QWidget *parent)
 : QComboBox(parent)
{
	addItem(i18n("16 by 16 pixels"));
	addItem(i18n("22 by 22 pixels"));
	addItem(i18n("32 by 32 pixels"));
	addItem(i18n("48 by 48 pixels"));
	addItem(i18n("64 by 64 pixels"));
	addItem(i18n("128 by 128 pixels"));
	setCurrentIndex(2);
}

IconSizeCombo::~IconSizeCombo()
{
}

int IconSizeCombo::iconSize()
{
	switch (currentIndex()) {
		default:
		case 0: return 16;
		case 1: return 22;
		case 2: return 32;
		case 3: return 48;
		case 4: return 64;
		case 5: return 128;
	}
}

void IconSizeCombo::setSize(int size)
{
	switch (size) {
		default:
		case 16:  setCurrentIndex(0); break;
		case 22:  setCurrentIndex(1); break;
		case 32:  setCurrentIndex(2); break;
		case 48:  setCurrentIndex(3); break;
		case 64:  setCurrentIndex(4); break;
		case 128: setCurrentIndex(5); break;
	}
}

/** class ViewSizeDialog: */

ViewSizeDialog::ViewSizeDialog(QWidget *parent, int w, int h)
 : QDialog(parent)
{
	QLabel *label = new QLabel(i18n(
		"Resize the window to select the image size\n"
		"and close it or press Escape to accept changes."), this);
	label->move(8, 8);
	label->setFixedSize( label->sizeHint() );

	// setSizeGripEnabled(true) doesn't work (the grip stay at the same place), so we emulate it:
	m_sizeGrip = new QSizeGrip(this);
	m_sizeGrip->setFixedSize( m_sizeGrip->sizeHint() );

	setGeometry(x(), y(), w, h);
}

ViewSizeDialog::~ViewSizeDialog()
{
}

void ViewSizeDialog::resizeEvent(QResizeEvent *)
{
	setWindowTitle( i18n("%1 by %2 pixels",QString::number(width()),QString::number(height())));
	m_sizeGrip->move( width() - m_sizeGrip->width(), height() - m_sizeGrip->height() );
}

/** class HelpLabel: */

HelpLabel::HelpLabel(const QString &text, const QString &message, QWidget *parent)
 : KUrlLabel(parent), m_message(message)
{
	setText(text);
	setWhatsThis(m_message);
	connect( this, SIGNAL(leftClickedUrl()), this, SLOT(display()) );
}

HelpLabel::~HelpLabel()
{
}

void HelpLabel::display(){
	QWhatsThis::showText(mapToGlobal( QPoint(width() / 2, height()) ), m_message);
}

/** class IconSizeDialog: */

class UndraggableKIconView : public KListWidget
{
  public:
	UndraggableKIconView(QWidget * parent = 0) : KListWidget(parent) { 
		this->setViewMode(QListView::IconMode);
		this->setMovement(QListView::Static); 
		this->setSelectionMode(QAbstractItemView::SingleSelection); 
		this->setWrapping(false);
	}
	QDrag* dragObject() { return 0; }
};

IconSizeDialog::IconSizeDialog(const QString &caption, const QString &message, const QString &icon, int iconSize, QWidget *parent)
     : KDialog(parent)
{
	// KDialog options
	setCaption(caption);
	setButtons(Ok | Cancel);
	setDefaultButton(Ok);
	setModal(true);
	showButtonSeparator(false);
	connect(this, SIGNAL(cancelClicked()), SLOT(slotCancel()));

	QWidget *page = new QWidget(this);
	QVBoxLayout *topLayout = new QVBoxLayout(page);

	QLabel *label = new QLabel(message, page);
	topLayout->addWidget(label);

	KListWidget *iconView = new UndraggableKIconView(page);

	m_size16  = new QListWidgetItem(DesktopIcon(icon,16), i18n("16 by 16 pixels"),iconView);
	m_size22  = new QListWidgetItem(DesktopIcon(icon,22), i18n("22 by 22 pixels"),iconView);
	m_size32  = new QListWidgetItem(DesktopIcon(icon,32), i18n("32 by 32 pixels"),iconView);
	m_size48  = new QListWidgetItem(DesktopIcon(icon,48), i18n("48 by 48 pixels"),iconView);
	m_size64  = new QListWidgetItem(DesktopIcon(icon,64), i18n("64 by 64 pixels"),iconView);
	m_size128 = new QListWidgetItem(DesktopIcon(icon,128), i18n("128 by 128 pixels"),iconView);
	iconView->setIconSize(QSize(128,128));
	iconView->setMinimumSize(QSize(128*6 + (6+2) * iconView->spacing() + 20, m_size128->sizeHint().height() + 2 * iconView->spacing() + 20));
	topLayout->addWidget(iconView);
	switch (iconSize) {
		case 16:  m_size16->setSelected(true); m_iconSize = 16;  break;
		case 22:  m_size22->setSelected(true); m_iconSize = 22;  break;
		default:
		case 32:  m_size32->setSelected(true); m_iconSize = 32;  break;
		case 48:  m_size48->setSelected(true); m_iconSize = 48;  break;
		case 64:  m_size64->setSelected(true); m_iconSize = 64;  break;
		case 128: m_size128->setSelected(true); m_iconSize = 128; break;
	}

	connect( iconView, SIGNAL(executed(QListWidgetItem*)),      this, SLOT(choose(QListWidgetItem*)) ); 
	connect( iconView, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(choose(QListWidgetItem*)) );
	connect( iconView, SIGNAL(itemSelectionChanged()),          this, SLOT(slotSelectionChanged()) );

	setMainWidget(page);
}

IconSizeDialog::~IconSizeDialog()
{
}

void IconSizeDialog::slotSelectionChanged()
{
	// Change m_iconSize to the new selected one:
	if (m_size16->isSelected())  { m_iconSize = 16;  return; }
	if (m_size22->isSelected())  { m_iconSize = 22;  return; }
	if (m_size32->isSelected())  { m_iconSize = 32;  return; }
	if (m_size48->isSelected())  { m_iconSize = 48;  return; }
	if (m_size64->isSelected())  { m_iconSize = 64;  return; }
	if (m_size128->isSelected()) { m_iconSize = 128; return; }

	// But if user unselected the item (by eg. right clicking a free space), reselect the last one:
	switch (m_iconSize) {
		case 16:  m_size16->setSelected(true);  m_iconSize = 16;  break;
		case 22:  m_size22->setSelected(true);  m_iconSize = 22;  break;
		default:
		case 32:  m_size32->setSelected(true);  m_iconSize = 32;  break;
		case 48:  m_size48->setSelected(true);  m_iconSize = 48;  break;
		case 64:  m_size64->setSelected(true);  m_iconSize = 64;  break;
		case 128: m_size128->setSelected(true); m_iconSize = 128; break;
	}
}

void IconSizeDialog::choose(QListWidgetItem *)
{
	button(Ok)->animateClick();
}

void IconSizeDialog::slotCancel()
{
	m_iconSize = -1;
}

/** class FontSizeCombo: */

FontSizeCombo::FontSizeCombo(bool rw, bool withDefault, QWidget *parent)
 : KComboBox(rw, parent), m_withDefault(withDefault)
{
	if (m_withDefault)
		addItem(i18n("(Default)"));

	QFontDatabase fontDB;
	QList<int> sizes = fontDB.standardSizes();
	for (QList<int>::Iterator it = sizes.begin(); it != sizes.end(); ++it)
		addItem(QString::number(*it));

//	connect( this, SIGNAL(acivated(const QString&)), this, SLOT(textChangedInCombo(const QString&)) );
	connect( this, SIGNAL(textChanged(const QString&)), this, SLOT(textChangedInCombo(const QString&)) );

	// TODO: 01617 void KFontSizeAction::setFontSize( int size )
}

FontSizeCombo::~FontSizeCombo()
{
}

void FontSizeCombo::textChangedInCombo(const QString &text)
{
	bool ok = false;
	int size = text.toInt(&ok);
	if (ok)
		emit sizeChanged(size);
}

void FontSizeCombo::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape)
		emit escapePressed();
	else if (event->key() == Qt::Key_Return)
		emit returnPressed2();
	else
		KComboBox::keyPressEvent(event);
}

void FontSizeCombo::setFontSize(qreal size)
{
	setItemText(currentIndex(), QString::number(size));

	// TODO: SEE KFontSizeAction::setFontSize( int size ) !!! for a more complete method!
}

qreal FontSizeCombo::fontSize()
{
	bool ok = false;
	int size = currentText().toInt(&ok);
	if (ok)
		return size;

	size = currentText().toInt(&ok);
	if (ok)
		return size;

	return font().pointSize();
}


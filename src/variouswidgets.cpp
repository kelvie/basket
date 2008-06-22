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
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QResizeEvent>
#include <Q3ValueList>
#include <QKeyEvent>
#include <Q3VBoxLayout>
#include <KOpenWithDialog>
#include <klocale.h>
#include <q3whatsthis.h>
#include <k3iconview.h>
#include <kiconloader.h>
#include <q3dragobject.h>
#include <qfontdatabase.h>
#include <kpushbutton.h>

#include "variouswidgets.h"


/** class RunCommandRequester: */

RunCommandRequester::RunCommandRequester(const QString &runCommand, const QString &message, QWidget *parent, const char *name)
 : QWidget(parent, name)
{
	m_message = message;

	Q3HBoxLayout *layout = new Q3HBoxLayout(this, /*margin=*/0, KDialog::spacingHint());
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

IconSizeCombo::IconSizeCombo(bool rw, QWidget *parent, const char *name)
 : QComboBox(rw, parent, name)
{
	insertItem(i18n("16 by 16 pixels"));
	insertItem(i18n("22 by 22 pixels"));
	insertItem(i18n("32 by 32 pixels"));
	insertItem(i18n("48 by 48 pixels"));
	insertItem(i18n("64 by 64 pixels"));
	insertItem(i18n("128 by 128 pixels"));
	setCurrentItem(2);
}

IconSizeCombo::~IconSizeCombo()
{
}

int IconSizeCombo::iconSize()
{
	switch (currentItem()) {
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
		case 16:  setCurrentItem(0); break;
		case 22:  setCurrentItem(1); break;
		case 32:  setCurrentItem(2); break;
		case 48:  setCurrentItem(3); break;
		case 64:  setCurrentItem(4); break;
		case 128: setCurrentItem(5); break;
	}
}

/** class ViewSizeDialog: */

ViewSizeDialog::ViewSizeDialog(QWidget *parent, int w, int h)
 : QDialog(parent, "ViewSizeDialog")
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
	setCaption( i18n("%1 by %2 pixels").arg(QString::number(width())).arg(QString::number(height())) );
	m_sizeGrip->move( width() - m_sizeGrip->width(), height() - m_sizeGrip->height() );
}

/** class HelpLabel: */

HelpLabel::HelpLabel(const QString &text, const QString &message, QWidget *parent)
 : KUrlLabel(parent), m_message(message)
{
	setText(text);
	connect( this, SIGNAL(leftClickedUrl()), this, SLOT(showMessage()) );
}

HelpLabel::~HelpLabel()
{
}

void HelpLabel::showMessage()
{
	Q3WhatsThis::display(m_message, mapToGlobal( QPoint(width() / 2, height()) ));
}

void HelpLabel::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Space)
		showMessage();
	else
		KUrlLabel::keyPressEvent(event);
}

/** class IconSizeDialog: */

class UndraggableKIconView : public K3IconView
{
  public:
	UndraggableKIconView(QWidget * parent = 0, const char * name = 0, Qt::WFlags f = 0) : K3IconView(parent, name, f) {}
	Q3DragObject* dragObject() { return 0; }
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
	Q3VBoxLayout *topLayout = new Q3VBoxLayout(page, /*margin=*/0, spacingHint());

	QLabel *label = new QLabel(message, page);
	topLayout->addWidget(label);

	K3IconView *iconView = new UndraggableKIconView(page);
	iconView->setItemsMovable(false);
	iconView->setSelectionMode(K3IconView::Single);
	m_size16  = new K3IconViewItem(iconView, 0,        i18n("16 by 16 pixels"),   DesktopIcon(icon, 16));
	m_size22  = new K3IconViewItem(iconView, m_size16, i18n("22 by 22 pixels"),   DesktopIcon(icon, 22));
	m_size32  = new K3IconViewItem(iconView, m_size22, i18n("32 by 32 pixels"),   DesktopIcon(icon, 32));
	m_size48  = new K3IconViewItem(iconView, m_size32, i18n("48 by 48 pixels"),   DesktopIcon(icon, 48));
	m_size64  = new K3IconViewItem(iconView, m_size48, i18n("64 by 64 pixels"),   DesktopIcon(icon, 64));
	m_size128 = new K3IconViewItem(iconView, m_size64, i18n("128 by 128 pixels"), DesktopIcon(icon, 128));
	iconView->setMinimumWidth(m_size16->width() + m_size22->width() + m_size32->width() + m_size48->width() + m_size64->width() + m_size128->width() +
	                          (6+2) * iconView->spacing() + 20);
	iconView->setMinimumHeight(m_size128->height() + 2 * iconView->spacing() + 20);
	topLayout->addWidget(iconView);
	switch (iconSize) {
		case 16:  iconView->setSelected(m_size16,  true); m_iconSize = 16;  break;
		case 22:  iconView->setSelected(m_size22,  true); m_iconSize = 22;  break;
		default:
		case 32:  iconView->setSelected(m_size32,  true); m_iconSize = 32;  break;
		case 48:  iconView->setSelected(m_size48,  true); m_iconSize = 48;  break;
		case 64:  iconView->setSelected(m_size64,  true); m_iconSize = 64;  break;
		case 128: iconView->setSelected(m_size128, true); m_iconSize = 128; break;
	}

	connect( iconView, SIGNAL(executed(Q3IconViewItem*)),      this, SLOT(choose(Q3IconViewItem*)) );
	connect( iconView, SIGNAL(returnPressed(Q3IconViewItem*)), this, SLOT(choose(Q3IconViewItem*)) );
	connect( iconView, SIGNAL(selectionChanged()),            this, SLOT(slotSelectionChanged()) );

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

void IconSizeDialog::choose(Q3IconViewItem*)
{
	button(Ok)->animateClick();
}

void IconSizeDialog::slotCancel()
{
	m_iconSize = -1;
}

/** class FontSizeCombo: */

FontSizeCombo::FontSizeCombo(bool rw, bool withDefault, QWidget *parent, const char *name)
 : KComboBox(rw, parent), m_withDefault(withDefault)
{
	if (m_withDefault)
		insertItem(i18n("(Default)"));

	QFontDatabase fontDB;
	Q3ValueList<int> sizes = fontDB.standardSizes();
	for (Q3ValueList<int>::Iterator it = sizes.begin(); it != sizes.end(); ++it)
		insertItem(QString::number(*it));

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

void FontSizeCombo::setFontSize(int size)
{
	setCurrentText(QString::number(size));

	// TODO: SEE KFontSizeAction::setFontSize( int size ) !!! for a more complete method!
}

int FontSizeCombo::fontSize()
{
	bool ok = false;
	int size = currentText().toInt(&ok);
	if (ok)
		return size;

	size = text(currentItem()).toInt(&ok);
	if (ok)
		return size;

	return font().pointSize();
}

#include "variouswidgets.moc"

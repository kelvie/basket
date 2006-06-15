/***************************************************************************
 *   Copyright (C) 2006 by Petri Damsten                                   *
 *   damu@iki.fi                                                           *
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

#include "password.h"

#ifdef HAVE_LIBGPGME

#include <qlayout.h>
#include <qtoolbutton.h>
#include <qbuttongroup.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kgpgme.h>
#include <basket.h>

PasswordDlg::PasswordDlg(QWidget *parent, const char *name)
	:KDialogBase(Plain, i18n("Password protection"), Ok|Cancel, Ok,
				 parent, name, true, false), w(0)
{
	QHBoxLayout* toplayout = new QHBoxLayout(plainPage(), 0, 0);
	w = new Password(plainPage());
	toplayout->addWidget(w, 1);
}

PasswordDlg::~PasswordDlg()
{
	delete w;
}

void PasswordDlg::slotOk()
{
	int n = w->buttonGroup->selectedId();
	QString key = w->keyLineEdit->text();
	if(n == Basket::PrivateKeyEncryption && key.isEmpty())
		KMessageBox::error(w, i18n("No private key selected."));
	else
		QDialog::accept();
}

Password::Password(QWidget *parent, const char *name)
 : PasswordLayout(parent, name)
{
	clearButton->setIconSet(SmallIconSet("clear_left"));
}


Password::~Password()
{
}

void Password::changeKey()
{
	KGpgMe gpg;
	QString key = gpg.selectKey(keyLineEdit->text());
	if(!key.isEmpty())
	{
		keyLineEdit->setText(key);
		buttonGroup->setButton(Basket::PrivateKeyEncryption);
	}
}

void Password::clearKey()
{
	int n = buttonGroup->selectedId();

	keyLineEdit->setText("");
	if(n == Basket::PrivateKeyEncryption)
		buttonGroup->setButton(Basket::PasswordEncryption);
}

#include "password.moc"

#endif

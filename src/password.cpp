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
#include <q3buttongroup.h>
#include <qradiobutton.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include "basket.h"
#include "kgpgme.h"

PasswordDlg::PasswordDlg(QWidget *parent, const char *name)
     : KDialog(parent)
     , w(0)
{
	// KDialog options
	setCaption(i18n("Password Protection"));
	setButtons(Ok | Cancel);
	setDefaultButton(Ok);
	setObjectName(name);
	setModal(true);
	showButtonSeparator(true);

	setMainWidget(new QWidget(this));
	Q3HBoxLayout* toplayout = new Q3HBoxLayout(mainWidget(), 0, 0);
	w = new Password(mainWidget());
	toplayout->addWidget(w, 1);
}

PasswordDlg::~PasswordDlg()
{
	delete w;
}

void PasswordDlg::accept()
{
	int n = type();
	if(n == Basket::PrivateKeyEncryption && key().isEmpty())
		KMessageBox::error(w, i18n("No private key selected."));
	else
		KDialog::accept();
}

QString PasswordDlg::key() const
{
	QString s = w->keyCombo->currentText();
	if(s.length() < 16)
		return "";
	int n = s.findRev(' ');
	if(n < 0)
		return "";
	return s.mid(n+1);
}

int PasswordDlg::type() const
{
	return w->buttonGroup->selectedId();
}

void PasswordDlg::setKey(const QString& key)
{
	for(int i = 0; i < w->keyCombo->count(); ++i)
	{
		if(w->keyCombo->text(i).find(key) >= 0)
		{
			w->keyCombo->setCurrentItem(i);
			return;
		}
	}
}

void PasswordDlg::setType(int type)
{
	w->buttonGroup->setButton(type);
}

Password::Password(QWidget *parent, const char *name)
 : PasswordLayout(parent, name)
{
	KGpgMe gpg;

	KGpgKeyList list = gpg.keys(true);
	for(KGpgKeyList::iterator it = list.begin(); it != list.end(); ++it) {
		QString name = gpg.checkForUtf8((*it).name);

		keyCombo->insertItem(QString("%1 <%2> %3").arg(name).arg((*it).email).arg((*it).id));
	}
	publicPrivateRadioButton->setEnabled(keyCombo->count() > 0);
	keyCombo->setEnabled(keyCombo->count() > 0);
}


Password::~Password()
{
}

#include "password.moc"

#endif

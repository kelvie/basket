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
#include <QGroupBox>
#include <qradiobutton.h>
//Added by qt3to4:
#include <QHBoxLayout>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>


#include "basket.h"
#include "kgpgme.h"

PasswordDlg::PasswordDlg(QWidget *parent)
        : KDialog(parent)
        , w(0)
{
    // KDialog options
    setWindowTitle(i18n("Password Protection"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setModal(true);
    showButtonSeparator(true);

    setMainWidget(new QWidget(this));
    QHBoxLayout* toplayout = new QHBoxLayout(mainWidget());
    w = new Password;
    toplayout->addWidget(w, 1);
}

PasswordDlg::~PasswordDlg()
{
    delete w;
}

void PasswordDlg::accept()
{
    int n = type();
    if (n == Basket::PrivateKeyEncryption && key().isEmpty())
        KMessageBox::error(w, i18n("No private key selected."));
    else
        KDialog::accept();
}

QString PasswordDlg::key() const
{
    QString s = w->keyCombo->currentText();
    if (s.length() < 16)
        return "";
    int n = s.lastIndexOf(' ');
    if (n < 0)
        return "";
    return s.mid(n + 1);
}

int PasswordDlg::type() const
{
    if (w->noPasswordRadioButton->isChecked())
        return Basket::NoEncryption;
    else if (w->passwordRadioButton->isChecked())
        return Basket::PasswordEncryption;
    else if (w->publicPrivateRadioButton->isChecked())
        return Basket::PrivateKeyEncryption;
    return -1;
}

void PasswordDlg::setKey(const QString& key)
{
    for (int i = 0; i < w->keyCombo->count(); ++i) {
        if (w->keyCombo->itemText(i).contains(key)) {
            w->keyCombo->setCurrentIndex(i);
            return;
        }
    }
}

void PasswordDlg::setType(int type)
{
    if (type == Basket::NoEncryption)
        w->noPasswordRadioButton->setChecked(true);
    else if (type == Basket::PasswordEncryption)
        w->passwordRadioButton->setChecked(true);
    else if (type == Basket::PrivateKeyEncryption)
        w->publicPrivateRadioButton->setChecked(true);
}

Password::Password(QWidget *parent)
        : QWidget(parent)
{
    // Setup from the UI file
    setupUi(this);

    KGpgMe gpg;

    KGpgKeyList list = gpg.keys(true);
    for (KGpgKeyList::iterator it = list.begin(); it != list.end(); ++it) {
        QString name = gpg.checkForUtf8((*it).name);

        keyCombo->addItem(QString("%1 <%2> %3").arg(name).arg((*it).email).arg((*it).id));
    }
    publicPrivateRadioButton->setEnabled(keyCombo->count() > 0);
    keyCombo->setEnabled(keyCombo->count() > 0);
}


Password::~Password()
{
}


#endif

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

#include "kgpgme.h"

#ifdef HAVE_LIBGPGME

#include <kapplication.h>
#include <kmessagebox.h>
#include <kpassdlg.h>
#include <kiconloader.h>
#include <klistview.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <klocale.h>
#include <locale.h>
#include <errno.h>

#define BUF_SIZE 512

// KGpgSelKey class based on class in KGpg with the same name

class KGpgSelKey : public KDialogBase
{
	private:
		KListView* keysListpr;

	public:

		KGpgSelKey(QWidget *parent, const char *name, QString preselected,
			const KGpgMe& gpg):
		KDialogBase( parent, name, true,i18n("Private Key List"),Ok | Cancel) {
			QString keyname;
			QVBoxLayout* vbox;
			QWidget* page = new QWidget(this);
			QLabel* labeltxt;
			KIconLoader* loader = KGlobal::iconLoader();
			QPixmap keyPair = loader->loadIcon("kgpg_key2", KIcon::Small, 20);

			setMinimumSize(350,100);
			keysListpr = new KListView(page);
			keysListpr->setRootIsDecorated(true);
			keysListpr->addColumn(i18n("Name"));
			keysListpr->addColumn(i18n("Email"));
			keysListpr->addColumn(i18n("ID"));
			keysListpr->setShowSortIndicator(true);
			keysListpr->setFullWidth(true);
			keysListpr->setAllColumnsShowFocus(true);

			labeltxt = new QLabel(i18n("Choose secret key:"),page);
			vbox = new QVBoxLayout(page);

			KGpgKeyList list = gpg.keys(true);

			for(KGpgKeyList::iterator it = list.begin(); it != list.end(); ++it) {
				QString name = gpg.checkForUtf8((*it).name);
				KListViewItem *item = new
					KListViewItem(keysListpr, name, (*it).email, (*it).id);
				item->setPixmap(0,keyPair);
				if(preselected == (*it).id) {
					keysListpr->setSelected(item, true);
					keysListpr->setCurrentItem(item);
				}
			}
			if(!keysListpr->selectedItem()) {
				keysListpr->setSelected(keysListpr->firstChild(), true);
				keysListpr->setCurrentItem(keysListpr->firstChild());
			}
			vbox->addWidget(labeltxt);
			vbox->addWidget(keysListpr);
			setMainWidget(page);
		};

		QString key() {
			QListViewItem* item = keysListpr->selectedItem();

			if(item)
				return item->text(2);
			return "";
		}
};

KGpgMe::KGpgMe(QString hint) : m_ctx(0), m_hint(hint)
{
	init(GPGME_PROTOCOL_OpenPGP);
	if(gpgme_new(&m_ctx)) {
		m_ctx = 0;
	}
	else {
		gpgme_set_armor(m_ctx, 1);
		setPassphraseCb();
	}
}

KGpgMe::~KGpgMe()
{
	if(m_ctx)
		gpgme_release(m_ctx);
}

void KGpgMe::init(gpgme_protocol_t proto)
{
	gpgme_error_t err;

	gpgme_check_version(NULL);
	setlocale(LC_ALL, "");
	gpgme_set_locale(NULL, LC_CTYPE, setlocale(LC_CTYPE, NULL));
	gpgme_set_locale(NULL, LC_MESSAGES, setlocale(LC_MESSAGES, NULL));

	err = gpgme_engine_check_version(proto);
	if(err) {
		KMessageBox::error(kapp->activeWindow(), QString("%1: %2")
			.arg(gpgme_strsource(err)).arg(gpgme_strerror(err)));
	}
}

QString KGpgMe::checkForUtf8(QString txt)
{
	// code borrowed from KGpg which borrowed it from gpa
	const char *s;

	// Make sure the encoding is UTF-8.
	// Test structure suggested by Werner Koch
	if(txt.isEmpty())
		return QString::null;

	for(s = txt.ascii(); *s && !(*s & 0x80); s++)
		;
	if (*s && !strchr (txt.ascii(), 0xc3) && (txt.find("\\x")==-1))
		return txt;

	// The string is not in UTF-8
	//if (strchr (txt.ascii(), 0xc3)) return (txt+" +++");
	if (txt.find("\\x")==-1)
		return QString::fromUtf8(txt.ascii());
	//        if (!strchr (txt.ascii(), 0xc3) || (txt.find("\\x")!=-1)) {
	for(int idx = 0 ; (idx = txt.find( "\\x", idx )) >= 0 ; ++idx) {
		char str[2] = "x";
		str[0] = (char)QString(txt.mid(idx + 2, 2)).toShort(0, 16);
		txt.replace(idx, 4, str);
	}
	if (!strchr (txt.ascii(), 0xc3))
		return QString::fromUtf8(txt.ascii());
	else
		return QString::fromUtf8(QString::fromUtf8(txt.ascii()).ascii());
	// perform Utf8 twice, or some keys display badly
	return txt;
}

QString KGpgMe::selectKey(QString previous) const
{
	KGpgSelKey dlg(kapp->activeWindow(), "", previous, *this);

	if(dlg.exec())
		return dlg.key();
	return "";
}

// Rest of the code is mainly based in gpgme examples

KGpgKeyList KGpgMe::keys(bool privateKeys /* = false */) const
{
	KGpgKeyList keys;
	gpgme_error_t err = 0, err2 = 0;
	gpgme_key_t key = 0;
	gpgme_keylist_result_t result = 0;

	if(m_ctx) {
		err = gpgme_op_keylist_start(m_ctx, NULL, privateKeys);
		if(!err) {
			while(!(err = gpgme_op_keylist_next(m_ctx, &key))) {
				KGpgKey gpgkey;

				if(!key->subkeys)
					continue;
				gpgkey.id = key->subkeys->keyid;
				if(key->uids) {
					gpgkey.name = key->uids->name;
					gpgkey.email = key->uids->email;
				}
				keys.append(gpgkey);
				gpgme_key_unref(key);
			}

			if (gpg_err_code (err) == GPG_ERR_EOF)
				err = 0;
			err2 = gpgme_op_keylist_end(m_ctx);
			if(!err)
				err = err2;
		}
	}

	if(err) {
		KMessageBox::error(kapp->activeWindow(), QString("%1: %2")
			.arg(gpgme_strsource(err)).arg(gpgme_strerror(err)));
	}
	else {
		result = gpgme_op_keylist_result(m_ctx);
		if (result->truncated) {
			KMessageBox::error(kapp->activeWindow(),
				i18n("Key listing unexpectedly truncated"));
		}
	}
	return keys;
}

bool KGpgMe::encrypt(const QString& inBuffer, QString* outBuffer,
QString keyid /* = QString::null */) const
{
	return encrypt(inBuffer.utf8(), outBuffer, keyid);
}

bool KGpgMe::encrypt(const QByteArray& inBuffer, QString* outBuffer,
QString keyid /* = QString::null */) const
{
	gpgme_error_t err = 0;
	gpgme_data_t in = 0, out = 0;
	gpgme_key_t keys[2] = { NULL, NULL };
	gpgme_key_t* key = NULL;
	gpgme_encrypt_result_t result = 0;

	*outBuffer = "";
	if(m_ctx) {
		err = gpgme_data_new_from_mem(&in, inBuffer.data(), inBuffer.size(), 1);
		if(!err) {
			err = gpgme_data_new(&out);
			if(!err) {
				if(keyid.isNull()) {
					key = NULL;
				}
				else {
					err = gpgme_get_key(m_ctx, keyid.ascii(), &keys[0], 0);
					key = keys;
				}

				if(!err) {
					err = gpgme_op_encrypt(m_ctx, key, GPGME_ENCRYPT_ALWAYS_TRUST,
						in, out);
					if(!err) {
						result = gpgme_op_encrypt_result(m_ctx);
						if (result->invalid_recipients) {
							KMessageBox::error(kapp->activeWindow(), QString("%1: %2")
								.arg(i18n("Invalid recipient encountered"))
								.arg(result->invalid_recipients->fpr));
						}
						else {
							char buf[BUF_SIZE + 2];
							int ret;

							ret = gpgme_data_seek(out, 0, SEEK_SET);
							if(ret) {
								err = gpgme_err_code_from_errno(errno);
							}
							else {
								while((ret = gpgme_data_read(out, buf, BUF_SIZE)) > 0) {
									buf[ret] = 0;
									*outBuffer += buf;
								}
								if(ret < 0)
									err = gpgme_err_code_from_errno(errno);
							}
						}
					}
				}
			}
		}
	}
	if(err) {
		KMessageBox::error(kapp->activeWindow(), QString("%1: %2")
			.arg(gpgme_strsource(err)).arg(gpgme_strerror(err)));
	}
	if(keys[0])
		gpgme_key_unref(keys[0]);
	if(in)
		gpgme_data_release(in);
	if(out)
		gpgme_data_release(out);
	return (err == 0);
}

bool KGpgMe::decrypt(const QString& inBuffer, QString* outBuffer) const
{
	QByteArray array;
	bool result;

	result = decrypt(inBuffer, &array);
	if(result)
		*outBuffer = QString::fromUtf8(array.data(), array.size());
	return result;
}

bool KGpgMe::decrypt(const QString& inBuffer, QByteArray* outBuffer) const
{
	gpgme_error_t err = 0;
	gpgme_data_t in = 0, out = 0;
	gpgme_decrypt_result_t result = 0;

	outBuffer->resize(0);
	if(m_ctx) {
		err = gpgme_data_new_from_mem(&in, inBuffer.ascii(), inBuffer.length(), 1);
		if(!err) {
			err = gpgme_data_new(&out);
			if(!err) {
				err = gpgme_op_decrypt(m_ctx, in, out);
				if(!err) {
					result = gpgme_op_decrypt_result(m_ctx);
					if(result->unsupported_algorithm) {
						KMessageBox::error(kapp->activeWindow(), QString("%1: %2")
							.arg(i18n("unsupported algorithm"))
							.arg(result->unsupported_algorithm));
					}
					else {
						char buf[BUF_SIZE + 2];
						int ret;

						ret = gpgme_data_seek(out, 0, SEEK_SET);
						if(ret) {
							err = gpgme_err_code_from_errno(errno);
						}
						else {
							while((ret = gpgme_data_read(out, buf, BUF_SIZE)) > 0) {
								uint size = outBuffer->size();
								if(outBuffer->resize(size + ret))
									memcpy(outBuffer->data() + size, buf, ret);
							}
							if(ret < 0)
								err = gpgme_err_code_from_errno(errno);
						}
					}
				}
			}
		}
	}
	if(err) {
		KMessageBox::error(kapp->activeWindow(), QString("%1: %2")
			.arg(gpgme_strsource(err)).arg(gpgme_strerror(err)));
	}
	if(in)
		gpgme_data_release(in);
	if(out)
		gpgme_data_release(out);
	return (err == 0);
}

void KGpgMe::setPassphraseCb()
{
	char *agent_info = 0;

	agent_info = getenv("GPG_AGENT_INFO");
	if (!(agent_info && strchr (agent_info, ':')))
		gpgme_set_passphrase_cb(m_ctx, passphraseCb, this);
}

gpgme_error_t KGpgMe::passphraseCb(void* hook, const char* uid_hint,
const char* /*passphrase_info*/,
int last_was_bad, int fd)
{
	QCString password;
	gpgme_error_t res = GPG_ERR_CANCELED;
	KGpgMe* gpg = static_cast<KGpgMe*>(hook);
	QString s;
	QString gpg_hint = checkForUtf8(uid_hint);

	s = gpg->hint();
	if(!s.isEmpty())
		s += "\n\n";
	if(last_was_bad)
		s += i18n("Password was not accepted.") + "\n\n";
	if(!gpg_hint.isEmpty())
		s += gpg_hint;

	if(KPasswordDialog::getPassword(password, s) == KPasswordDialog::Accepted) {
		write(fd, password.data(), password.length());
		res = 0;
	}
	write(fd, "\n", 1);
	return res;
}
#endif																  // HAVE_LIBGPGME

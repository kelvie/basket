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
#include <kdebug.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <klocale.h>
#include <locale.h>
#include <errno.h>

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

			labeltxt = new QLabel(i18n("Choose a secret key:"),page);
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

KGpgMe::KGpgMe() : m_ctx(0), m_useGnuPGAgent(true)
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
	clearCache();
}

void KGpgMe::clearCache()
{
	if(m_cache.size() > 0)
	{
		m_cache.fill('\0');
		m_cache.truncate(0);
	}
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

QString KGpgMe::selectKey(QString previous)
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
				i18n("Key listing unexpectedly truncated."));
		}
	}
	return keys;
}

bool KGpgMe::encrypt(const QByteArray& inBuffer, Q_ULONG length,
					 QByteArray* outBuffer, QString keyid /* = QString::null */)
{
	gpgme_error_t err = 0;
	gpgme_data_t in = 0, out = 0;
	gpgme_key_t keys[2] = { NULL, NULL };
	gpgme_key_t* key = NULL;
	gpgme_encrypt_result_t result = 0;

	outBuffer->resize(0);
	if(m_ctx) {
		err = gpgme_data_new_from_mem(&in, inBuffer.data(), length, 1);
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
							err = readToBuffer(out, outBuffer);
						}
					}
				}
			}
		}
	}
	if(err != GPG_ERR_NO_ERROR && err != GPG_ERR_CANCELED) {
		KMessageBox::error(kapp->activeWindow(), QString("%1: %2")
			.arg(gpgme_strsource(err)).arg(gpgme_strerror(err)));
	}
	if(err != GPG_ERR_NO_ERROR)
		clearCache();
	if(keys[0])
		gpgme_key_unref(keys[0]);
	if(in)
		gpgme_data_release(in);
	if(out)
		gpgme_data_release(out);
	return (err == GPG_ERR_NO_ERROR);
}

bool KGpgMe::decrypt(const QByteArray& inBuffer, QByteArray* outBuffer)
{
	gpgme_error_t err = 0;
	gpgme_data_t in = 0, out = 0;
	gpgme_decrypt_result_t result = 0;

	outBuffer->resize(0);
	if(m_ctx) {
		err = gpgme_data_new_from_mem(&in, inBuffer.data(), inBuffer.size(), 1);
		if(!err) {
			err = gpgme_data_new(&out);
			if(!err) {
				err = gpgme_op_decrypt(m_ctx, in, out);
				if(!err) {
					result = gpgme_op_decrypt_result(m_ctx);
					if(result->unsupported_algorithm) {
						KMessageBox::error(kapp->activeWindow(), QString("%1: %2")
							.arg(i18n("Unsupported algorithm"))
							.arg(result->unsupported_algorithm));
					}
					else {
						err = readToBuffer(out, outBuffer);
					}
				}
			}
		}
	}
	if(err != GPG_ERR_NO_ERROR && err != GPG_ERR_CANCELED) {
		KMessageBox::error(kapp->activeWindow(), QString("%1: %2")
			.arg(gpgme_strsource(err)).arg(gpgme_strerror(err)));
	}
	if(err != GPG_ERR_NO_ERROR)
		clearCache();
	if(in)
		gpgme_data_release(in);
	if(out)
		gpgme_data_release(out);
	return (err == GPG_ERR_NO_ERROR);
}

#define BUF_SIZE (32 * 1024)

gpgme_error_t KGpgMe::readToBuffer(gpgme_data_t in, QByteArray* outBuffer) const
{
	int ret;
	gpgme_error_t err = GPG_ERR_NO_ERROR;

	ret = gpgme_data_seek(in, 0, SEEK_SET);
	if(ret) {
		err = gpgme_err_code_from_errno(errno);
	}
	else {
		char* buf = new char[BUF_SIZE + 2];

		if(buf) {
			while((ret = gpgme_data_read(in, buf, BUF_SIZE)) > 0) {
				uint size = outBuffer->size();
				if(outBuffer->resize(size + ret))
					memcpy(outBuffer->data() + size, buf, ret);
			}
			if(ret < 0)
				err = gpgme_err_code_from_errno(errno);
			delete[] buf;
		}
	}
	return err;
}

bool KGpgMe::isGnuPGAgentAvailable()
{
	QString agent_info = getenv("GPG_AGENT_INFO");

	if (agent_info.find(':') > 0)
		return true;
	return false;
}

void KGpgMe::setPassphraseCb()
{
	bool agent = false;
	QString agent_info;

	agent_info = getenv("GPG_AGENT_INFO");

	if(m_useGnuPGAgent)
	{
		if (agent_info.find(':'))
			agent = true;
		if(agent_info.startsWith("disable:"))
			setenv("GPG_AGENT_INFO", agent_info.mid(8), 1);
	}
	else
	{
		if(!agent_info.startsWith("disable:"))
			setenv("GPG_AGENT_INFO", "disable:" + agent_info, 1);
	}
	if (agent)
		gpgme_set_passphrase_cb(m_ctx, 0, 0);
	else
		gpgme_set_passphrase_cb(m_ctx, passphraseCb, this);
}

gpgme_error_t KGpgMe::passphraseCb(void* hook, const char* uid_hint,
								   const char* passphrase_info,
								   int last_was_bad, int fd)
{
	KGpgMe* gpg = static_cast<KGpgMe*>(hook);
	return gpg->passphrase(uid_hint, passphrase_info, last_was_bad, fd);
}

gpgme_error_t KGpgMe::passphrase(const char* uid_hint,
								 const char* /*passphrase_info*/,
								 int last_was_bad, int fd)
{
	gpgme_error_t res = GPG_ERR_CANCELED;
	QString s;
	QString gpg_hint = checkForUtf8(uid_hint);
	int result;

	if(last_was_bad){
		s += "<b>" + i18n("Wrong password.") + "</b><br><br>\n\n";
		clearCache();
	}

	if(!m_text.isEmpty())
		s += m_text + "<br>";

	if(!gpg_hint.isEmpty())
		s += gpg_hint;

	if(m_cache.isEmpty()){
		QCString password;

		if(m_saving)
			result = KPasswordDialog::getNewPassword(password, s);
		else
			result = KPasswordDialog::getPassword(password, s);

		if(result == KPasswordDialog::Accepted)
			m_cache = password;
	}
	else
		result = KPasswordDialog::Accepted;

	if(result == KPasswordDialog::Accepted) {
		write(fd, m_cache.data(), m_cache.length());
		res = 0;
	}
	write(fd, "\n", 1);
	return res;
}
#endif	// HAVE_LIBGPGME

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
#ifndef KGPGME_H
#define KGPGME_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LIBGPGME

#include <gpgme.h>
#include <qstringlist.h>

/**
	@author Petri Damsten <damu@iki.fi>
*/

class KGpgKey
{
	public:
		QString id;
		QString name;
		QString email;
};

typedef QValueList< KGpgKey > KGpgKeyList;

class KGpgMe
{
	public:
		KGpgMe(QString hint = QString::null);
		~KGpgMe();

		QString selectKey(QString previous = QString::null) const;
		KGpgKeyList keys(bool privateKeys = false) const;
		void setHint(QString hint) { m_hint = hint; };
		QString hint() const { return m_hint; };

		bool encrypt(const QByteArray& inBuffer, QByteArray* outBuffer,
			QString keyid = QString::null) const;
		bool decrypt(const QByteArray& inBuffer, QByteArray* outBuffer) const;

		static QString checkForUtf8(QString txt);

	private:
		gpgme_ctx_t m_ctx;
		QString m_hint;

		void init(gpgme_protocol_t proto);
		void setPassphraseCb();
		static gpgme_error_t passphraseCb(void *hook, const char *uid_hint,
			const char *passphrase_info,
			int last_was_bad, int fd);
};
#endif																  // HAVE_LIBGPGME
#endif																  // KGPGME_H

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

#include <config.h>

#ifdef HAVE_LIBGPGME

#include <gpgme.h>

#include <QtCore/QList>
#include <QtCore/QString>

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

typedef QList< KGpgKey > KGpgKeyList;

class KGpgMe
{
public:
    KGpgMe();
    ~KGpgMe();

    QString selectKey(QString previous = QString::null);
    KGpgKeyList keys(bool privateKeys = false) const;
    void setText(QString text, bool saving) {
        m_text = text; m_saving = saving;
    };
    void setUseGnuPGAgent(bool use) {
        m_useGnuPGAgent = use; setPassphraseCb();
    };
    QString text() const {
        return m_text;
    };
    bool saving() const {
        return m_saving;
    };
    void clearCache();

    bool encrypt(const QByteArray& inBuffer, unsigned long length,
                 QByteArray* outBuffer, QString keyid = QString::null);
    bool decrypt(const QByteArray& inBuffer, QByteArray* outBuffer);

    static QString checkForUtf8(QString txt);
    static bool isGnuPGAgentAvailable();

private:
    gpgme_ctx_t m_ctx;
    QString m_text;
    bool m_saving;
    bool m_useGnuPGAgent;
    QString m_cache;

    void init(gpgme_protocol_t proto);
    gpgme_error_t readToBuffer(gpgme_data_t in, QByteArray* outBuffer) const;
    void setPassphraseCb();
    static gpgme_error_t passphraseCb(void *hook, const char *uid_hint,
                                      const char *passphrase_info,
                                      int last_was_bad, int fd);
    gpgme_error_t passphrase(const char *uid_hint,
                             const char *passphrase_info,
                             int last_was_bad, int fd);
};
#endif                                                                // HAVE_LIBGPGME
#endif                                                                // KGPGME_H

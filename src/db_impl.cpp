/*  Copyright (C) 2009 Kelvie Wong <kelvie@ieee.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "db_impl.h"
#include "db_object.h"
#include "global.h"

#include <QCryptographicHash>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>

// Utility functions

// Returns a unique has for any arbitrary "data"
static inline QString getHash(QByteArray data)
{
    return QString::fromAscii(QCryptographicHash::hash(data,
                                                       QCryptographicHash::Sha1).toHex());
}

// Returns a text representation of the object -- this is just an XML
// representation containing properties and the hash of its data.

static inline QByteArray getObjText(DatabaseObject object)
{
    QHash<QString, QString> props = object.properties();
    QStringList keys = props.keys();
    QDomDocument doc("object");
    QDomElement propElement = doc.createElement("properties");
    QDomElement dataElement = doc.createElement("data");

    keys.sort();
    foreach (QString key, keys)
        propElement.setAttribute(key, props[key]);

    dataElement.setAttribute("key", getHash(object.data()));
    
    doc.appendChild(propElement);
    doc.appendChild(dataElement);

    // Note that the indent for the XML is set to zero.
    return doc.toByteArray(0);
}

// Returns a unique hash of an entire object
static inline QString getHash(DatabaseObject object)
{
    return getHash(getObjText(object));
}

// Member functions
BasketDatabaseImpl::BasketDatabaseImpl()
{
    QDir basketDir = Global::basketsFolder();
    m_dataDir = basketDir;
    m_objDir = basketDir;
    m_rcDir = basketDir;

    m_dataDir.mkdir("data");
    m_objDir.mkdir("obj");
    m_rcDir.mkdir("rc");

    m_dataDir.cd("data");
    m_objDir.cd("obj");
    m_rcDir.cd("rc");

    if (basketDir.exists("root")) {
        QFile rootFile(basketDir.absoluteFilePath("root"));
        rootFile.open(QFile::ReadOnly | QFile::Text);
        m_root = QString::fromAscii(rootFile.readAll()).trimmed();
    }
}

DatabaseObject BasketDatabaseImpl::getObject(QString key) const
{
    // Empty object, gets returned if the item with the spcified key is not
    // found 
    DatabaseObject obj;
    if (m_dataDir.exists(key) && m_objDir.exists(key)) {
        QFile dataFile(m_dataDir.absoluteFilePath(key));
        QFile objFile(m_objDir.absoluteFilePath(key));
        QHash<QString, QString> props;

        dataFile.open(QFile::ReadOnly);
        objFile.open(QFile::ReadOnly | QFile::Text);
        
        obj.setData(dataFile.readAll());
        QDomDocument doc(objFile.readAll());
        QDomElement propElement =
            doc.elementsByTagName("properties").at(0).toElement();
        QDomNamedNodeMap attrs = propElement.attributes();
        for (int i = 0; i < attrs.count(); ++i) {
            QDomAttr attr = attrs.item(i).toAttr();
            obj.setProperty(attr.name(), attr.value());
        }
    }

    return obj;
}

QString BasketDatabaseImpl::addObject(DatabaseObject obj)
{
    // Be sure to increment the reference count for an object when adding it.
    QString hash = getHash(obj);
    if (!m_objDir.exists(hash)) {
        // Create the object
        QFile dataFile(m_dataDir.absoluteFilePath(hash));
        QFile objFile(m_objDir.absoluteFilePath(hash));

        dataFile.open(QFile::WriteOnly);
        dataFile.write(obj.data());

        objFile.open(QFile::WriteOnly | QFile::Text);
        objFile.write(getObjText(obj));
    }

    // Increment reference count
    changeRC(hash, 1);

    return hash;
}

void BasketDatabaseImpl::removeObject(QString key)
{
    // Decrement the reference count
    changeRC(key, -1);
}

bool BasketDatabaseImpl::hasObject(QString key) const
{
    return m_objDir.exists(key);
}

DatabaseObject BasketDatabaseImpl::getRootObject() const
{
    return getObject(m_root);
}

void BasketDatabaseImpl::setRootObject(DatabaseObject obj)
{
    m_root = addObject(obj);
}

BasketDatabaseImpl::~BasketDatabaseImpl()
{
    // pass
}

void BasketDatabaseImpl::changeRC(QString key, int d)
{
    QFile rcFile(m_rcDir.absoluteFilePath("key"));
    rcFile.open(QFile::ReadOnly | QFile::Text);
    int oldRC = QString::fromAscii(rcFile.readAll()).toInt();
    rcFile.close();

    int newRC = oldRC + d;

    rcFile.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);
    rcFile.write(QString::number(newRC).toAscii());

    // Clean up the object when the count goes to zero
    if (!newRC) {
        m_objDir.remove(key);
        m_dataDir.remove(key);
        m_rcDir.remove(key);
    }
}

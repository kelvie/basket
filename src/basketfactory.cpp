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

#include "basketfactory.h"

#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtGui/QGraphicsView>
#include <QtXml/QDomElement>

#include <KDE/KLocale>
#include <KDE/KMessageBox>

#include "global.h"
#include "basketscene.h"
#include "xmlwork.h"
#include "note.h" // For balanced column width computation
#include "bnpview.h"

/** BasketFactory */

// TODO: Don't create a basket with a name that already exists!

QString BasketFactory::newFolderName()
{
    QString folderName;
    QString fullPath;
    QDir    dir;


    int i = QDir(Global::basketsFolder()).count();
    QString time = QTime::currentTime().toString("hhmmss");

    for (; ; ++i) {
        folderName = QString("basket%1-%2/").arg(i).arg(time);
        fullPath   = Global::basketsFolder() + folderName;
        dir        = QDir(fullPath);
        if (! dir.exists())   // OK : The folder do not yet exists :
            break;            //  We've found one !
    }

    return folderName;
}

QString BasketFactory::unpackTemplate(const QString &templateName)
{
    // Find a name for a new folder and create it:
    QString folderName = newFolderName();
    QString fullPath   = Global::basketsFolder() + folderName;
    QDir dir;
    if (!dir.mkdir(fullPath)) {
        KMessageBox::error(/*parent=*/0, i18n("Sorry, but the folder creation for this new basket has failed."), i18n("Basket Creation Failed"));
        return "";
    }

    // Unpack the template file to that folder:
    // TODO: REALLY unpack (this hand-creation is temporary, or it could be used in case the template can't be found)
    QFile file(fullPath + "/.basket");
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        int nbColumns = (templateName == "mindmap" || templateName == "free" ? 0 : templateName.left(1).toInt());
        BasketScene *currentBasket = Global::bnpView->currentBasket();
        int columnWidth = (currentBasket && nbColumns > 0 ? (currentBasket->graphicsView()->viewport()->width() - (nbColumns - 1) * Note::RESIZER_WIDTH) / nbColumns : 0);
        stream << QString("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
                          "<!DOCTYPE basket>\n"
                          "<basket>\n"
                          " <properties>\n"
                          "  <disposition mindMap=\"%1\" columnCount=\"%2\" free=\"%3\" />\n"
                          " </properties>\n"
                          " <notes>\n").arg((templateName == "mindmap" ? "true" : "false"),
                                            QString::number(nbColumns),
                                            (templateName == "free" || templateName == "mindmap" ? "true" : "false"));
        if (nbColumns > 0)
            for (int i = 0; i < nbColumns; ++i)
                stream << QString("  <group width=\"%1\"></group>\n").arg(columnWidth);
        stream << " </notes>\n"
        "</basket>\n";
        file.close();
        return folderName;
    } else {
        KMessageBox::error(/*parent=*/0, i18n("Sorry, but the template copying for this new basket has failed."), i18n("Basket Creation Failed"));
        return "";
    }
}

void BasketFactory::newBasket(const QString &icon,
                              const QString &name,
                              const QString &backgroundImage,
                              const QColor  &backgroundColor,
                              const QColor  &textColor,
                              const QString &templateName,
                              BasketScene *parent)
{
    // Unpack the templateName file to a new basket folder:
    QString folderName = unpackTemplate(templateName);
    if (folderName.isEmpty())
        return;

    // Read the properties, change those that should be customized and save the result:
    QDomDocument *document  = XMLWork::openFile("basket", Global::basketsFolder() + folderName + "/.basket");
    if (!document) {
        KMessageBox::error(/*parent=*/0, i18n("Sorry, but the template customization for this new basket has failed."), i18n("Basket Creation Failed"));
        return;
    }
    QDomElement properties  = XMLWork::getElement(document->documentElement(), "properties");

    if (!icon.isEmpty()) {
        QDomElement iconElement = XMLWork::getElement(properties, "icon");
        if (!iconElement.tagName().isEmpty()) // If there is already an icon, remove it since we will add our own value below
            iconElement.removeChild(iconElement.firstChild());
        XMLWork::addElement(*document, properties, "icon", icon);
    }

    if (!name.isEmpty()) {
        QDomElement nameElement = XMLWork::getElement(properties, "name");
        if (!nameElement.tagName().isEmpty()) // If there is already a name, remove it since we will add our own value below
            nameElement.removeChild(nameElement.firstChild());
        XMLWork::addElement(*document, properties, "name", name);
    }

    if (backgroundColor.isValid()) {
        QDomElement appearanceElement = XMLWork::getElement(properties, "appearance");
        if (appearanceElement.tagName().isEmpty()) { // If there is not already an appearance tag, add it since we will access it below
            appearanceElement = document->createElement("appearance");
            properties.appendChild(appearanceElement);
        }
        appearanceElement.setAttribute("backgroundColor", backgroundColor.name());
    }

    if (!backgroundImage.isEmpty()) {
        QDomElement appearanceElement = XMLWork::getElement(properties, "appearance");
        if (appearanceElement.tagName().isEmpty()) { // If there is not already an appearance tag, add it since we will access it below
            appearanceElement = document->createElement("appearance");
            properties.appendChild(appearanceElement);
        }
        appearanceElement.setAttribute("backgroundImage", backgroundImage);
    }

    if (textColor.isValid()) {
        QDomElement appearanceElement = XMLWork::getElement(properties, "appearance");
        if (appearanceElement.tagName().isEmpty()) { // If there is not already an appearance tag, add it since we will access it below
            appearanceElement = document->createElement("appearance");
            properties.appendChild(appearanceElement);
        }
        appearanceElement.setAttribute("textColor", textColor.name());
    }

    // Load it in the parent basket (it will save the tree and switch to this new basket):
    Global::bnpView->loadNewBasket(folderName, properties, parent);
}

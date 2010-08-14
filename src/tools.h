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
//Added by qt3to4:
#include <QVector>
#include <QPixmap>

#ifndef TOOLS_H
#define TOOLS_H

#include <QVector>
#include <QPixmap>

class QString;
class QColor;
class QMimeData;
class HTMLExporter;

class StopWatch
{
public:
    static void start(int id);
    static void check(int id);
private:
    static QVector<QTime>  starts;
    static QVector<double> totals;
    static QVector<uint>   counts;
};

/** Some useful functions for that application.
  * @author Sébastien Laoût
  */
namespace Tools
{
// Text <-> HTML Conversions and Tools:
QString textToHTML(const QString &text);
QString textToHTMLWithoutP(const QString &text);
QString htmlToParagraph(const QString &html);
QString htmlToText(const QString &html);
QString tagURLs(const QString &test);
QString tagCrossReferences(const QString &text);
QString tagCrossReferencesForHtml(const QString &text, HTMLExporter *exporter);
QString convertToCrossReference(const QString &html);
QString cssFontDefinition(const QFont &font, bool onlyFontFamily = false);

// String Manipulations:
QString stripEndWhiteSpaces(const QString &string);

// Pixmap Manipulations:
/** @Return true if it is a Web color
  */
bool isWebColor(const QColor &color);
/** @Return a color that is 50% of @p color1 and 50% of @p color2.
  */
QColor mixColor(const QColor &color1, const QColor &color2);
/** @Return true if the color is too dark to be darkened one more time.
  */
bool tooDark(const QColor &color);
/** Make sure the @p pixmap is of the size (@p width, @p height) and @return a pixmap of this size.
  * If @p height <= 0, then width will be used to make the picture square.
  */
QPixmap normalizePixmap(const QPixmap &pixmap, int width, int height = 0);
/** @Return the pixmap @p source with depth*deltaX transparent pixels added to the left.\n
  * If @p deltaX is <= 0, then an indent delta is computed depending on the @p source width.
  */
QPixmap indentPixmap(const QPixmap &source, int depth, int deltaX = 0);

// File and Folder Manipulations:
/** Delete the folder @p folderOrFile recursively (to remove sub-folders and child files too).
  */
void deleteRecursively(const QString &folderOrFile);
/** @Return a new filename that doesn't already exist in @p destFolder.
  * If @p wantedName alread exist in @p destFolder, a dash and a number will be added before the extenssion.
  * Id there were already such a number in @p wantedName, it is incremented until a free filename is found.
  */
QString fileNameForNewFile(const QString &wantedName, const QString &destFolder);

// Other:
//void iconForURL(const KUrl &url);
/** @Return true if the source is from a file cutting in Konqueror.
  * @Return false if it was just a copy or if it was a drag.
  */
bool isAFileCut(const QMimeData *source);

// Debug
void printChildren(QObject* parent);
}

#endif // TOOLS_H

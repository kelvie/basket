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

#include <kdebug.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qregexp.h>
#include <QList>
#include <qfileinfo.h>
#include <qdir.h>
#include <qmime.h>
#include <qfont.h>
#include <qfontinfo.h>
#include <qobject.h>
//Added by qt3to4:
#include <QVector>

#include <QTextDocument>
#include <QTime>

#include "tools.h"

QVector<QTime>  StopWatch::starts;
QVector<double> StopWatch::totals;
QVector<uint>   StopWatch::counts;

void StopWatch::start(int id)
{
	if (id >= starts.size()) {
		totals.resize(id + 1);
		counts.resize(id + 1);
		for (int i = starts.size(); i <= id; i++) {
			totals[i] = 0;
			counts[i] = 0;
		}
		starts.resize(id + 1);
	}
	starts[id] = QTime::currentTime();
}

void StopWatch::check(int id)
{
	if (id >= starts.size())
		return;
	double time = starts[id].msecsTo(QTime::currentTime()) / 1000.0;
	totals[id] += time;
	counts[id]++;
	kDebug() << k_funcinfo << "Timer_" << id << ": " << time << " s    [" << counts[id] << " times, total: " << totals[id] << " s, average: " << totals[id] / counts[id] << " s]" <<  endl;
}

QString Tools::textToHTML(const QString &text)
{
	if (text.isEmpty())
		return "<p></p>";
	if (/*text.isEmpty() ||*/ text == " " || text == "&nbsp;")
		return "<p>&nbsp;</p>";

	// convertFromPlainText() replace "\n\n" by "</p>\n<p>": we don't want that
	QString htmlString = Qt::convertFromPlainText(text, Qt::WhiteSpaceNormal);
	return htmlString.replace("</p>\n", "<br>\n<br>\n").replace("\n<p>", "\n"); // Don't replace first and last tags
}

QString Tools::textToHTMLWithoutP(const QString &text)
{
	// textToHTML(text) return "<p>HTMLizedText</p>". We remove the strating "<p>" and ending </p>"
	QString HTMLizedText = textToHTML(text);
	return HTMLizedText.mid(3, HTMLizedText.length() - 3 - 4);
}

QString Tools::htmlToParagraph(const QString &html)
{
	QString result = html;
	bool startedBySpan = false;

	// Remove the <html> start tag, all the <head> and the <body> start
	// Because <body> can contain style="..." parameter, we transform it to <span>
	int pos = result.indexOf("<body");
	if (pos != -1) {
		result = "<span" + result.mid(pos + 5);
		startedBySpan = true;
	}

	// Remove the ending "</p>\n</body></html>", each tag can be separated by space characters (%s)
	// "</p>" can be omitted (eg. if the HTML doesn't contain paragraph but tables), as well as "</body>" (optinal)
	pos = result.indexOf(QRegExp("(?:(?:</p>[\\s\\n\\r\\t]*)*</body>[\\s\\n\\r\\t]*)*</html>", Qt::CaseInsensitive));
	if (pos != -1)
		result = result.left(pos);

	if (startedBySpan)
		result += "</span>";

	return result.replace("</p>", "<br><br>").replace("<p>", "");
}

// The following is adapted from KStringHanlder::tagURLs
// The adaptation lies in the change to urlEx
// Thanks to Richard Heck
QString Tools::tagURLs(const QString &text)
{
	QRegExp urlEx("<!DOCTYPE[^\"]+\"([^\"]+)\"[^\"]+\"([^\"]+)/([^/]+)\\.dtd\">");
	QString richText(text);
	int urlPos = 0;
	int urlLen;
	if((urlPos = urlEx.indexIn(richText, urlPos)) >= 0)
		urlPos+=urlEx.matchedLength();
	else
		urlPos=0;
	urlEx.setPattern("(www\\.(?!\\.)|([a-zA-z]+)://)[\\d\\w\\./,:_~\\?=&;#@\\-\\+\\%\\$]+[\\d\\w/]");
	while ((urlPos = urlEx.indexIn(richText, urlPos)) >= 0) {
		urlLen = urlEx.matchedLength();
		QString href = richText.mid(urlPos, urlLen);
		// Qt doesn't support (?<=pattern) so we do it here
		if ((urlPos > 0) && richText[urlPos-1].isLetterOrNumber()) {
			urlPos++;
			continue;
		}
		QString anchor = "<a href=\"" + href + "\">" + href + "</a>";
		richText.replace(urlPos, urlLen, anchor);
		urlPos += anchor.length();
	}
	return richText;
}

QString Tools::htmlToText(const QString &html)
{
	QString text = htmlToParagraph(html);
	text.remove("\n");
	text.replace("</h1>", "\n");
	text.replace("</h2>", "\n");
	text.replace("</h3>", "\n");
	text.replace("</h4>", "\n");
	text.replace("</h5>", "\n");
	text.replace("</h6>", "\n");
	text.replace("</li>", "\n");
	text.replace("</dt>", "\n");
	text.replace("</dd>", "\n");
	text.replace("<dd>",  "   ");
	text.replace("</div>","\n");
	text.replace("</blockquote>","\n");
	text.replace("</caption>","\n");
	text.replace("</tr>", "\n");
	text.replace("</th>", "  ");
	text.replace("</td>", "  ");
	text.replace("<br>",  "\n");
	text.replace("<br />","\n");
	// FIXME: Format <table> tags better, if possible
	// TODO: Replace &eacute; and co. by theire equivalent!

	// To manage tags:
	int pos = 0;
	int pos2;
	QString tag, tag3;
	// To manage lists:
	int deep = 0;            // The deep of the current line in imbriqued lists
	QList<bool> ul;    // true if current list is a <ul> one, false if it's an <ol> one
	QList<int>  lines; // The line number if it is an <ol> list
	// We're removing every other tags, or replace them in the case of li:
	while ( (pos = text.indexOf("<"), pos) != -1 ) {
		// What is the current tag?
		tag  = text.mid(pos + 1, 2);
		tag3 = text.mid(pos + 1, 3);
		// Lists work:
		if (tag == "ul") {
			deep++;
			ul.push_back(true);
			lines.push_back(-1);
		} else if (tag == "ol") {
			deep++;
			ul.push_back(false);
			lines.push_back(0);
		} else if (tag3 == "/ul" || tag3 == "/ol") {
			deep--;
			ul.pop_back();
			lines.pop_back();
		}
		// Where the tag closes?
		pos2 = text.indexOf(">");
		if (pos2 != -1) {
			// Remove the tag:
			text.remove(pos, pos2 - pos + 1);
			// And replace li with "* ", "x. "... without forbidding to indent that:
			if (tag == "li") {
				// How many spaces before the line (indentation):
				QString spaces = "";
				for (int i = 1; i < deep; i++)
					spaces += "  ";
				// The bullet or number of the line:
				QString bullet = "* ";
				if (ul.back() == false) {
					lines.push_back(lines.back() + 1);
					lines.pop_back();
					bullet = QString::number(lines.back()) + ". ";
				}
				// Insertion:
				text.insert(pos, spaces + bullet);
			}
			if ( (tag3 == "/ul" || tag3 == "/ol") && deep == 0 )
				text.insert(pos, "\n"); // Empty line before and after a set of lists
		}
		++pos;
	}

	text.replace("&gt;",   ">");
	text.replace("&lt;",   "<");
	text.replace("&quot;", "\"");
	text.replace("&nbsp;", " ");
	text.replace("&amp;",  "&"); // CONVERT IN LAST!!

	return text;
}

QString Tools::cssFontDefinition(const QFont &font, bool onlyFontFamily)
{
	// The font definition:
	QString definition = QString(font.italic() ? "italic " : "") +
	                     QString(font.bold()   ? "bold "   : "") +
	                     QString::number(QFontInfo(font).pixelSize()) + "px ";

	// Then, try to match the font name with a standard CSS font family:
	QString genericFont = "";
	if (definition.contains("serif", Qt::CaseInsensitive) || definition.contains("roman", Qt::CaseInsensitive))
		genericFont = "serif";
	// No "else if" because "sans serif" must be counted as "sans". So, the order between "serif" and "sans" is important
	if (definition.contains("sans", Qt::CaseInsensitive) || definition.contains("arial", Qt::CaseInsensitive) || definition.contains("helvetica", Qt::CaseInsensitive))
		genericFont = "sans-serif";
	if (definition.contains("mono", Qt::CaseInsensitive) || definition.contains("courier", Qt::CaseInsensitive) ||
	    definition.contains("typewriter", Qt::CaseInsensitive) || definition.contains("console", Qt::CaseInsensitive) ||
	    definition.contains("terminal", Qt::CaseInsensitive) || definition.contains("news", Qt::CaseInsensitive))
		genericFont = "monospace";

	// Eventually add the generic font family to the definition:
	QString fontDefinition = "\"" + font.family() + "\"";
	if (!genericFont.isEmpty())
		fontDefinition += ", " + genericFont;

	if (onlyFontFamily)
		return fontDefinition;

	return definition + fontDefinition;
}

QString Tools::stripEndWhiteSpaces(const QString &string)
{
	uint length = string.length();
	uint i;
	for (i = length; i > 0; --i)
		if (!string[i-1].isSpace())
			break;
	if (i == 0)
		return "";
	else
		return string.left(i);
}



bool Tools::isWebColor(const QColor &color)
{
	int r = color.red();   // The 216 web colors are those colors whose RGB (Red, Green, Blue)
	int g = color.green(); //  values are all in the set (0, 51, 102, 153, 204, 255).
	int b = color.blue();

	return ( ( r ==   0 || r ==  51 || r == 102 ||
			   r == 153 || r == 204 || r == 255    ) &&
			 ( g ==   0 || g ==  51 || g == 102 ||
			   g == 153 || g == 204 || g == 255    ) &&
			 ( b ==   0 || b ==  51 || b == 102 ||
			   b == 153 || b == 204 || b == 255    )    );
}

QColor Tools::mixColor(const QColor &color1, const QColor &color2)
{
	QColor mixedColor;
	mixedColor.setRgb( (color1.red()   + color2.red())   / 2,
	                   (color1.green() + color2.green()) / 2,
	                   (color1.blue()  + color2.blue())  / 2 );
	return mixedColor;
}

bool Tools::tooDark(const QColor &color)
{
	return color.value() < 175;
}


// TODO: Use it for all indentPixmap()
QPixmap Tools::normalizePixmap(const QPixmap &pixmap, int width, int height)
{
	if (height <= 0)
		height = width;

	if (pixmap.isNull() || (pixmap.width() == width && pixmap.height() == height))
		return pixmap;

	return pixmap;
}

QPixmap Tools::indentPixmap(const QPixmap &source, int depth, int deltaX)
{
	// Verify if it is possible:
	if (depth <= 0 || source.isNull())
		return source;

	// Compute the number of pixels to indent:
	if (deltaX <= 0)
		deltaX = 2 * source.width() / 3;
	int indent = depth * deltaX;

	// Create the images:
	QImage resultImage(indent + source.width(), source.height(), QImage::Format_ARGB32);
	resultImage.setNumColors(32);

	QImage sourceImage = source.toImage();

	// Clear the indent part (the left part) by making it fully transparent:
	uint *p;
	for (int row = 0; row < resultImage.height(); ++row) {
		for (int column = 0; column < resultImage.width(); ++column) {
			p = (uint *)resultImage.scanLine(row) + column;
			*p = 0; // qRgba(0, 0, 0, 0)
		}
	}

	// Copy the source image byte per byte to the right part:
	uint *q;
	for (int row = 0; row < sourceImage.height(); ++row) {
		for (int column = 0; column < sourceImage.width(); ++column) {
			p = (uint *)resultImage.scanLine(row) + indent + column;
			q = (uint *)sourceImage.scanLine(row) + column;
			*p = *q;
		}
	}

	// And return the result:
	QPixmap result = QPixmap::fromImage(resultImage);
	return result;
}

#include <QTextDocument>

void Tools::deleteRecursively(const QString &folderOrFile)
{
	if (folderOrFile.isEmpty())
		return;

	QFileInfo fileInfo(folderOrFile);
	if (fileInfo.isDir()) {
		// Delete the child files:
		QDir dir(folderOrFile, QString::null, QDir::Name | QDir::IgnoreCase, QDir::TypeMask | QDir::Hidden);
		QStringList list = dir.entryList();
		for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
			if ( *it != "." && *it != ".." )
				deleteRecursively(folderOrFile + "/" + *it);
		// And then delete the folder:
		dir.rmdir(folderOrFile);
	} else
		// Delete the file:
		QFile::remove(folderOrFile);
}

QString Tools::fileNameForNewFile(const QString &wantedName, const QString &destFolder)
{
	QString fileName  = wantedName;
	QString fullName  = destFolder + fileName;
	QString extension = "";
	int     number    = 2;
	QDir    dir;

	// First check if the file do not exists yet (simplier and more often case)
	dir = QDir(fullName);
	if ( ! dir.exists(fullName) )
		return fileName;

	// Find the file extension, if it exists : Split fileName in fileName and extension
	// Example : fileName == "note5-3.txt" => fileName = "note5-3" and extension = ".txt"
	int extIndex = fileName.lastIndexOf('.');
	if (extIndex != -1 && extIndex != int(fileName.length()-1))  { // Extension found and fileName do not ends with '.' !
		extension = fileName.mid(extIndex);
		fileName.truncate(extIndex);
	} // else fileName = fileName and extension = ""

	// Find the file number, if it exists : Split fileName in fileName and number
	// Example : fileName == "note5-3" => fileName = "note5" and number = 3
	int extNumber = fileName.lastIndexOf('-');
	if (extNumber != -1 && extNumber != int(fileName.length()-1))  { // Number found and fileName do not ends with '-' !
		bool isANumber;
		int  theNumber = fileName.mid(extNumber + 1).toInt(&isANumber);
		if (isANumber) {
			number = theNumber;
			fileName.truncate(extNumber);
		} // else :
	} // else fileName = fileName and number = 2 (because if the file already exists, the genereated name is at last the 2nd)

	QString finalName;
	for (/*int number = 2*/; ; ++number) { // TODO: FIXME: If overflow ???
		finalName = fileName + "-" + QString::number(number) + extension;
		fullName = destFolder + finalName;
		dir = QDir(fullName);
		if ( ! dir.exists(fullName) )
			break;
	}

	return finalName;
}


// TODO: Move it from NoteFactory
/*QString NoteFactory::iconForURL(const KUrl &url)
{
	QString icon = KMimeType::iconNameForUrl(url.url());
	if ( url.protocol() == "mailto" )
		icon = "message";
	return icon;
}*/

bool Tools::isAFileCut(const QMimeData *source)
{
	if (source->hasFormat("application/x-kde-cutselection")) {
		QByteArray array = source->data("application/x-kde-cutselection");
		return !array.isEmpty() && QByteArray(array.data(), array.size() + 1).at(0) == '1';
	} else
		return false;
}

void Tools::printChildren(QObject* parent)
{
	const QObjectList objs = parent->children();
    QObject * obj;
    for(int i = 0 ; i < objs.size() ; i++) {
        obj = objs.at(i);
        kDebug() << k_funcinfo << obj->metaObject()->className() << ": " << obj->objectName() << endl;
    }

}

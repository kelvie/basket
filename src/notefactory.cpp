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

#include <qstring.h>
//Added by qt3to4:
#include <QTextStream>
#include <QDropEvent>
#include <Q3MemArray>
#include <kurl.h>
#include <qpixmap.h>
#include <qcolor.h>
#include <qregexp.h>
#include <q3stylesheet.h>
#include <qdir.h>
#include <kmimetype.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdesktopfile.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <qfile.h>
#include <kfilemetainfo.h>
#include <kio/jobclasses.h>
#include <qtextcodec.h>
#include <KOpenWithDialog>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <qfileinfo.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kurifilter.h>

#include "basket.h"
#include "note.h"
#include "notefactory.h"
#include "notedrag.h"
#include "linklabel.h"
#include "global.h"
#include "settings.h"
#include "keyboard.h"
#include "variouswidgets.h"
#include "tools.h"
#include "kicondialog.h"

#include "debugwindow.h"

#include <QBitmap>
#include <KIO/CopyJob>

/** Create notes from scratch (just a content) */

Note* NoteFactory::createNoteText(const QString &text, Basket *parent, bool reallyPlainText/* = false*/)
{
	Note *note = new Note(parent);
	if (reallyPlainText) {
		TextContent *content = new TextContent(note, createFileForNewNote(parent, "txt"));
		content->setText(text);
		content->saveToFile();
	} else {
		HtmlContent *content = new HtmlContent(note, createFileForNewNote(parent, "html"));
		QString html = "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body>" + Tools::textToHTMLWithoutP(text) + "</body></html>";
		content->setHtml(html);
		content->saveToFile();
	}
	return note;
}

Note* NoteFactory::createNoteHtml(const QString &html, Basket *parent)
{
	Note *note = new Note(parent);
	HtmlContent *content = new HtmlContent(note, createFileForNewNote(parent, "html"));
	content->setHtml(html);
	content->saveToFile();
	return note;
}

Note* NoteFactory::createNoteLink(const KUrl &url, Basket *parent)
{
	Note *note = new Note(parent);
	new LinkContent(note, url, titleForURL(url), iconForURL(url), /*autoTitle=*/true, /*autoIcon=*/true);
	return note;
}

Note* NoteFactory::createNoteLink(const KUrl &url, const QString &title, Basket *parent)
{
	Note *note = new Note(parent);
	new LinkContent(note, url, title, iconForURL(url), /*autoTitle=*/false, /*autoIcon=*/true);
	return note;
}

Note* NoteFactory::createNoteImage(const QPixmap &image, Basket *parent)
{
	Note *note = new Note(parent);
	ImageContent *content = new ImageContent(note, createFileForNewNote(parent, "png"));
	content->setPixmap(image);
	content->saveToFile();
	return note;
}

Note* NoteFactory::createNoteColor(const QColor &color, Basket *parent)
{
	Note *note = new Note(parent);
	new ColorContent(note, color);
	return note;
}

/** Return a string list containing {url1, title1, url2, title2, url3, title3...}
 */
QStringList NoteFactory::textToURLList(const QString &text)
{
	// List to return:
	QStringList list;

	// Split lines:
	QStringList texts = QStringList::split('\n', text);

	// For each lines:
	QStringList::iterator it;
	for (it = texts.begin(); it != texts.end(); ++it) {
		// Strip white spaces:
		(*it) = (*it).trimmed();

		// Don't care of empty entries:
		if ((*it).isEmpty())
			continue;

		// Compute lower case equivalent:
		QString ltext = (*it).toLower();

		/* Search for mail address ("*@*.*" ; "*" can contain '_', '-', or '.') and add protocol to it */
		QString mailExpString = "[\\w-\\.]+@[\\w-\\.]+\\.[\\w]+";
		QRegExp mailExp("^"+mailExpString+"$");
		if (mailExp.search(ltext) != -1) {
			ltext.insert(0, "mailto:");
			(*it).insert(0, "mailto:");
		}

		// TODO: Recognize "<link>" (link between '<' and '>')
		// TODO: Replace " at " by "@" and " dot " by "." to look for e-mail addresses

		/* Search for mail address like "Name <address@provider.net>" */
		QRegExp namedMailExp("^([\\w\\s]+)\\s<("+mailExpString+")>$");
		//namedMailExp.setCaseSensitive(true); // For the name to be keeped with uppercases // DOESN'T WORK !
		if (namedMailExp.search(ltext) != -1) {
			QString name    = namedMailExp.cap(1);
			QString address = "mailto:" + namedMailExp.cap(2);
			// Threat it NOW, as it's an exception (it have a title):
			list.append(address);
			list.append(name);
			continue;
		}

		/* Search for an url and create an URL note */
		if ( (ltext.startsWith("/") && ltext[1] != '/' && ltext[1] != '*') || // Take files but not C/C++/... comments !
				   ltext.startsWith("file:")    ||
				   ltext.startsWith("http://")  ||
				   ltext.startsWith("https://") ||
				   ltext.startsWith("www.")     ||
				   ltext.startsWith("ftp.")     ||
				   ltext.startsWith("ftp://")   ||
				   ltext.startsWith("mailto:")    ) {

			// First, correct the text to use the good format for the url
			if (ltext.startsWith( "/"))
				(*it).insert(0, "file:");
			if (ltext.startsWith("www."))
				(*it).insert(0, "http://");
			if (ltext.startsWith("ftp."))
				(*it).insert(0, "ftp://");

			// And create the Url note (or launcher if URL point a .desktop file)
			list.append(*it);
			list.append(""); // We don't have any title
				   } else
					   return QStringList(); // FAILED: treat the text as a text, and not as a URL list!
	}
	return list;
}

Note* NoteFactory::createNoteFromText(const QString &text, Basket *parent)
{
	/* Search for a color (#RGB , #RRGGBB , #RRRGGGBBB , #RRRRGGGGBBBB) and create a color note */
	QRegExp exp("^#(?:[a-fA-F\\d]{3}){1,4}$");
	if ( exp.search(text) != -1 )
		return createNoteColor(QColor(text), parent);

	/* Try to convert the text as a URL or a list of URLs */
	QStringList uriList = textToURLList(text);
	if ( ! uriList.isEmpty() ) {
		// TODO: This code is almost duplicated from fropURLs()!
		Note *note;
		Note *firstNote = 0;
		Note *lastInserted = 0;
		QStringList::iterator it;
		for (it = uriList.begin(); it != uriList.end(); ++it) {
			QString url = (*it);
			++it;
			QString title = (*it);
			if (title.isEmpty())
				note = createNoteLinkOrLauncher(KUrl(url), parent);
			else
				note = createNoteLink(KUrl(url), title, parent);

			// If we got a new note, insert it in a linked list (we will return the first note of that list):
			if (note) {
//				kDebug() << "Drop URL: " << (*it).prettyUrl();
				if (!firstNote)
					firstNote = note;
				else {
					lastInserted->setNext(note);
					note->setPrev(lastInserted);
				}
				lastInserted = note;
			}

		}
		return firstNote; // It don't return ALL inserted notes !
	}

	//QString newText = text.trimmed(); // The text for a new note, without useless spaces
	/* Else, it's a text or an HTML note, so, create it */
	if (Qt::mightBeRichText(/*newT*/text))
		return createNoteHtml(/*newT*/text, parent);
	else
		return createNoteText(/*newT*/text, parent);
}

Note* NoteFactory::createNoteLauncher(const KUrl &url, Basket *parent)
{
	if (url.isEmpty())
		return createNoteLauncher("", "", "", parent);
	else
		return copyFileAndLoad(url, parent);
}

Note* NoteFactory::createNoteLauncher(const QString &command, const QString &name, const QString &icon, Basket *parent)
{
	QString fileName = createNoteLauncherFile(command, name, icon, parent);
	if (fileName.isEmpty())
		return 0L;
	else
		return loadFile(fileName, parent);
}

QString NoteFactory::createNoteLauncherFile(const QString &command, const QString &name, const QString &icon, Basket *parent)
{
	QString content = QString(
			"[Desktop Entry]\n"
			"Exec=%1\n"
			"Name=%2\n"
			"Icon=%3\n"
			"Encoding=UTF-8\n"
			"Type=Application\n").arg(command, name, icon.isEmpty() ? QString("exec") : icon);
	QString fileName = fileNameForNewNote(parent, "launcher.desktop");
	QString fullPath = parent->fullPathForFileName(fileName);
//	parent->dontCareOfCreation(fullPath);
	QFile file(fullPath);
	if ( file.open(QIODevice::WriteOnly) ) {
		QTextStream stream(&file);
		stream.setEncoding(QTextStream::UnicodeUTF8);
		stream << content;
		file.close();
		return fileName;
	} else
		return QString();
}

Note* NoteFactory::createNoteLinkOrLauncher(const KUrl &url, Basket *parent)
{
	// IMPORTANT: we create the service ONLY if the extension is ".desktop".
	//            Otherwise, KService take a long time to analyse all the file
	//            and output such things to stdout:
	//            "Invalid entry (missing '=') at /my/file.ogg:11984"
	//            "Invalid entry (missing ']') at /my/file.ogg:11984"...
	KService::Ptr service;
	if (url.fileName().endsWith(".desktop"))
		service = new KService(url.path());

	// If link point to a .desktop file then add a launcher, otherwise it's a link
	if (service && service->isValid())
		return createNoteLauncher(url, parent);
	else
		return createNoteLink(url, parent);
}

#include <q3strlist.h>
#include <qimage.h>

bool NoteFactory::movingNotesInTheSameBasket(const QMimeData *source, Basket *parent, QDropEvent::Action action)
{
	if (NoteDrag::canDecode(source))
		return action == QDropEvent::Move && NoteDrag::basketOf(source) == parent;
	else
		return false;
}

Note* NoteFactory::dropNote(const QMimeData *source, Basket *parent, bool fromDrop, QDropEvent::Action action, Note */*noteSource*/)
{
	Note *note = 0L;

	QStringList formats = source->formats();
	/* No data */
	if (formats.size() == 0) {
		// TODO: add a parameter to say if it's from a clipboard paste, a selection paste, or a drop
		//       To be able to say "The clipboard/selection/drop is empty".
//		KMessageBox::error(parent, i18n("There is no data to insert."), i18n("No Data"));
		return 0;
	}

	/* Debug */
	if (Global::debugWindow) {
		*Global::debugWindow << "<b>Drop :</b>";
		for (int i = 0; formats.size(); ++i)
			*Global::debugWindow << "\t[" + QString::number(i) + "] " + formats[i];
		switch (action) { // The source want that we:
			case QDropEvent::Copy:       *Global::debugWindow << ">> Drop action: Copy";       break;
			case QDropEvent::Move:       *Global::debugWindow << ">> Drop action: Move";       break;
			case QDropEvent::Link:       *Global::debugWindow << ">> Drop action: Link";       break;
			case QDropEvent::Private:    *Global::debugWindow << ">> Drop action: Private";    break; // What is it? (Copy?)
			default:                     *Global::debugWindow << ">> Drop action: Unknown";           //  supported by Qt!
		}
	}

	/* Copy or move a Note */
	if (NoteDrag::canDecode(source)) {
		bool moveFiles = fromDrop && action == QDropEvent::Move;
		bool moveNotes = moveFiles;
		return NoteDrag::decode(source, parent, moveFiles, moveNotes); // Filename will be kept
	}

	/* Else : Drop object to note */

	QImage image = qvariant_cast<QImage>(source->imageData()) ;
	if ( !image.isNull() )
		return createNoteImage(QPixmap::fromImage(image), parent);

	// K3ColorDrag::decode() is buggy and can trheat strings like "#include <foo.h>" as a black color
	// The correct "ideal" code:
	/*QColor color;
	if ( K3ColorDrag::decode(source, color) ) {
	createNoteColor(color, parent);
	return;
}*/
	// And then the hack (if provide color MIME type or a text that contains color), using createNote Color RegExp:
	QString hack;
	QRegExp exp("^#(?:[a-fA-F\\d]{3}){1,4}$");
	hack = source->text();
	if (source->hasFormat("application/x-color") || (!hack.isNull() && (exp.search(hack) != -1)) ) {
		QColor color = qvariant_cast<QColor>(source->colorData()) ;
		if (color.isValid())
			return createNoteColor(color, parent);
//			if ( (note = createNoteColor(color, parent)) )
//				return note;
//			// Theorically it should be returned. If not, continue by dropping other things
	}

	KUrl::List urls = KUrl::List::fromMimeData(source);
	if ( !urls.isEmpty() ){
		// If it's a Paste, we should know if files should be copied (copy&paste) or moved (cut&paste):
		if (!fromDrop && Tools::isAFileCut(source))
			action = QDropEvent::Move;
		return dropURLs(urls, parent, action, fromDrop);
	}

	// FIXME: use dropURLs() also from Mozilla?

	/*
	* Mozilla's stuff sometimes uses utf-16-le - little-endian UTF-16.
	*
	* This has the property that for the ASCII subset case (And indeed, the
	* ISO-8859-1 subset, I think), if you treat it as a C-style string,
	* it'll come out to one character long in most cases, since it looks
	 * like:
	*
	* "<\0H\0T\0M\0L\0>\0"
	*
	* A strlen() call on that will give you 1, which simply isn't correct.
	* That might, I suppose, be the answer, or something close.
	*
	* Also, Mozilla's drag/drop code predates the use of MIME types in XDnD
	* - hence it'll throw about STRING and UTF8_STRING quite happily, hence
	* the odd named types.
	*
	* Thanks to Dave Cridland for having said me that.
	*/
	if (source->hasFormat("text/x-moz-url")) { // FOR MOZILLA
		// Get the array and create a QChar array of 1/2 of the size
		QByteArray mozilla = source->data("text/x-moz-url");
		Q3MemArray<QChar> chars( mozilla.count() / 2 );
		// A small debug work to know the value of each bytes
		if (Global::debugWindow)
			for (int i = 0; i < mozilla.count(); i++)
				*Global::debugWindow << QString("'") + QChar(mozilla[i]) + "' " + QString::number(int(mozilla[i]));
		// text/x-moz-url give the URL followed by the link title and separed by OxOA (10 decimal: new line?)
		uint size   = 0;
		QChar *name = 0L;
		// For each little endian mozilla chars, copy it to the array of QChars
		for (int i = 0; i < mozilla.count(); i += 2) {
			chars[i/2] = QChar(mozilla[i], mozilla[i+1]);
			if (mozilla.at(i) == 0x0A) {
				size = i/2;
				name = &(chars[i/2+1]);
			}
		}
		// Create a QString that take the address of the first QChar and a length
		if (name == 0L) { // We haven't found name (FIXME: Is it possible ?)
			QString normalHtml(&(chars[0]), chars.size());
			return createNoteLink(normalHtml, parent);
		} else {
			QString normalHtml(  &(chars[0]), size               );
			QString normalTitle( name,        chars.size()-size-1);
			return createNoteLink(normalHtml, normalTitle, parent);
		}
	}

	if (source->hasFormat("text/html")) {
		QString html;
		QString subtype("html");
		// If the text/html comes from Mozilla or GNOME it can be UTF-16 encoded: we need ExtendedTextDrag to check that
		ExtendedTextDrag::decode(source, html, subtype);
		return createNoteHtml(html, parent);
	}

	QString text;
	// If the text/plain comes from GEdit or GNOME it can be empty: we need ExtendedTextDrag to check other MIME types
	if ( ExtendedTextDrag::decode(source, text) )
		return createNoteFromText(text, parent);

	/* Unsucceful drop */
	note = createNoteUnknown(source, parent);
	QString message = i18n("<p>%1 doesn't support the data you've dropped.<br>"
			"It however created a generic note, allowing you to drag or copy it to an application that understand it.</p>"
			"<p>If you want the support of these data, please contact developer or visit the "
			"<a href=\"http://basket.kde.org/dropdb.php\">BasKet Drop Database</a>.</p>",KGlobal::mainComponent().aboutData()->programName());
	KMessageBox::information(parent, message, i18n("Unsupported MIME Type(s)"),
							 "unsupportedDropInfo", KMessageBox::AllowLink);
	return note;
}

Note* NoteFactory::createNoteUnknown(const QMimeData *source, Basket *parent/*, const QString &annotations*/)
{
	// Save the MimeSource in a file: create and open the file:
	QString fileName = createFileForNewNote(parent, "unknown");
	QFile file(parent->fullPath() + fileName);
	if ( ! file.open(QIODevice::WriteOnly) )
		return 0L;
	QDataStream stream(&file);

	// Echo MIME types:
	QStringList formats = source->formats();
	for (int i = 0; formats.size(); ++i)
		stream << QString(formats[i]); // Output the '\0'-terminated format name string

	// Echo end of MIME types list delimiter:
	stream << "";

	// Echo the length (in bytes) and then the data, and then same for next MIME type:
	for (int i = 0; formats.size(); ++i){
		QByteArray data = source->data(formats[i]);
		stream << (quint32)data.count();
		stream.writeRawBytes(data.data(), data.count());
	}
	file.close();

	Note *note = new Note(parent);
	new UnknownContent(note, fileName);
	return note;
}

Note* NoteFactory::dropURLs(KUrl::List urls, Basket *parent, QDropEvent::Action action, bool fromDrop)
{
	int  shouldAsk    = 0; // shouldAsk==0: don't ask ; shouldAsk==1: ask for "file" ; shouldAsk>=2: ask for "files"
	bool shiftPressed = Keyboard::shiftPressed();
	bool ctrlPressed  = Keyboard::controlPressed();
	bool modified     = fromDrop && (shiftPressed || ctrlPressed);

	if (modified) // Then no menu + modified action
		; // action is already set: no work to do
	else if (fromDrop) { // Compute if user should be asked or not
		for ( KUrl::List::iterator it = urls.begin(); it != urls.end(); ++it )
			if ((*it).protocol() != "mailto") { // Do not ask when dropping mail address :-)
			shouldAsk++;
			if (shouldAsk == 1/*2*/) // Sufficient
				break;
			}
			if (shouldAsk) {
				KMenu menu(parent);
				QList<QAction *> actList;
				actList << new KAction(KIcon("go-jump"),
						       i18n("&Move Here\tShift"),
						       &menu)
					<< new KAction(KIcon("edit-copy"),
						       i18n("&Copy Here\tCtrl"),
						       &menu)
					<< new KAction(KIcon("insert-link"),
						       i18n("&Link Here\tCtrl+Shift"),
						       &menu);

				foreach (QAction *a, actList)
					menu.addAction(a);

				menu.insertSeparator();
				menu.addAction(KIcon("dialog-cancel"), i18n("C&ancel\tEscape"));
				int id = actList.indexOf(menu.exec(QCursor::pos()));
				switch (id) {
					case 0: action = QDropEvent::Move; break;
					case 1: action = QDropEvent::Copy; break;
					case 2: action = QDropEvent::Link; break;
					default: return 0;
				}
				modified = true;
			}
	} else { // fromPaste
		;
	}

	/* Policy of drops of URL:
	*   Email: [Modifier keys: Useless]
	+    - Link mail address
	*   Remote URL: [Modifier keys: {Copy,Link}]
	+    - Download as Image, Animation and Launcher
	+    - Link other URLs
	*   Local URL: [Modifier keys: {Copy,Move,Link}]
	*    - Copy as Image, Animation and Launcher [Modifier keys: {Copy,Move,Link}]
	*    - Link folder [Modifier keys: Useless]
	*    - Make Launcher of executable [Modifier keys: {Copy_exec,Move_exec,Link_Launcher}]
	*    - Ask for file (if use want to copy and it is a sound: make Sound)
	* Policy of pastes of URL: [NO modifier keys]
	*   - Same as drops
	*   - But copy when ask should be done
	*   - Unless cut-selection is true: move files instead
	* Policy of file created in the basket dir: [NO modifier keys]
	*   - View as Image, Animation, Sound, Launcher
	*   - View as File
	*/
	Note *note;
	Note *firstNote = 0;
	Note *lastInserted = 0;
	for (KUrl::List::iterator it = urls.begin(); it != urls.end(); ++it) {
		if ( ((*it).protocol() == "mailto") ||
					  (action == QDropEvent::Link)    )
			note = createNoteLinkOrLauncher(*it, parent);
		else if (!(*it).isLocalFile()) {
			if ( action != QDropEvent::Link && (maybeImageOrAnimation(*it)/* || maybeSound(*it)*/) )
				note = copyFileAndLoad(*it, parent);
			else
				note = createNoteLinkOrLauncher(*it, parent);
		} else {
			if (action == QDropEvent::Copy)
				note = copyFileAndLoad(*it, parent);
			else if (action == QDropEvent::Move)
				note = moveFileAndLoad(*it, parent);
			else
				note = createNoteLinkOrLauncher(*it, parent);
		}

		// If we got a new note, insert it in a linked list (we will return the first note of that list):
		if (note) {
			DEBUG_WIN << "Drop URL: " + (*it).prettyUrl();
			if (!firstNote)
				firstNote = note;
			else {
				lastInserted->setNext(note);
				note->setPrev(lastInserted);
			}
			lastInserted = note;
		}
	}
	return firstNote;
}

void NoteFactory::consumeContent(QDataStream &stream, NoteType::Id type)
{
	if (type == NoteType::Link) {
		KUrl url;
		QString title, icon;
		quint64 autoTitle64, autoIcon64;
		stream >> url >> title >> icon >> autoTitle64 >> autoIcon64;
	} else if (type == NoteType::Color) {
		QColor color;
		stream >> color;
	}
}

Note* NoteFactory::decodeContent(QDataStream &stream, NoteType::Id type, Basket *parent)
{
/*	if (type == NoteType::Text) {
	QString text;
	stream >> text;
	return NoteFactory::createNoteText(text, parent);
} else if (type == NoteType::Html) {
	QString html;
	stream >> html;
	return NoteFactory::createNoteHtml(html, parent);
} else if (type == NoteType::Image) {
	QPixmap pixmap;
	stream >> pixmap;
	return NoteFactory::createNoteImage(pixmap, parent);
} else */
	if (type == NoteType::Link) {
		KUrl url;
		QString title, icon;
		quint64 autoTitle64, autoIcon64;
		bool autoTitle, autoIcon;
		stream >> url >> title >> icon >> autoTitle64 >> autoIcon64;
		autoTitle = (bool)autoTitle64;
		autoIcon  = (bool)autoIcon64;
		Note *note = NoteFactory::createNoteLink(url, parent);
		((LinkContent*)(note->content()))->setLink(url, title, icon, autoTitle, autoIcon);
		return note;
	} else if (type == NoteType::Color) {
		QColor color;
		stream >> color;
		return NoteFactory::createNoteColor(color, parent);
	} else
		return 0; // NoteFactory::loadFile() is sufficient
}

// mayBeLauncher: url.url().endsWith(".desktop");

bool NoteFactory::maybeText(const KUrl &url)
{
	QString path = url.url().toLower();
	return path.endsWith(".txt");
}

bool NoteFactory::maybeHtml(const KUrl &url)
{
	QString path = url.url().toLower();
	return path.endsWith(".html") || path.endsWith(".htm");
}

bool NoteFactory::maybeImageOrAnimation(const KUrl &url)
{
	/* Examples on my machine:
	QImageDrag can understands
	{"image/png", "image/bmp", "image/jpeg", "image/pgm", "image/ppm", "image/xbm", "image/xpm"}
	QImageIO::inputFormats() returns
	{"BMP", "GIF", "JPEG", "MNG", "PBM", "PGM", "PNG", "PPM", "XBM", "XPM"}
		QImageDecoder::inputFormats():
	{"GIF", "MNG", "PNG"} */
	QList<QByteArray> formats = QImageReader::supportedImageFormats();
	formats << QMovie::supportedFormats();

	// Since QImageDrag return only "JPEG" and extensions can be "JPG";
	// preprend for heuristic optim.
	formats.prepend("jpg");

	QString path = url.url().toLower();
	foreach (QByteArray format, formats)
		if (path.endsWith(QString(".") + QString(format).toLower()))
			return true;
	// TODO: Search real MIME type for local files?
	return false;
}

bool NoteFactory::maybeAnimation(const KUrl &url)
{
	QString path = url.url().toLower();
	return path.endsWith(".mng") || path.endsWith(".gif");
}

bool NoteFactory::maybeSound(const KUrl &url)
{
	QString path = url.url().toLower();
	return path.endsWith(".mp3") || path.endsWith(".ogg");
}

bool NoteFactory::maybeLauncher(const KUrl &url)
{
	QString path = url.url().toLower();
	return path.endsWith(".desktop");
}

////////////// NEW:

Note* NoteFactory::copyFileAndLoad(const KUrl &url, Basket *parent)
{
	QString fileName = fileNameForNewNote(parent, url.fileName());
	QString fullPath = parent->fullPathForFileName(fileName);

	if (Global::debugWindow)
		*Global::debugWindow << "copyFileAndLoad: " + url.prettyUrl() + " to " + fullPath;

//	QString annotations = i18n("Original file: %1").arg(url.prettyUrl());
//	parent->dontCareOfCreation(fullPath);

	KIO::CopyJob *copyJob = KIO::copy(url, KUrl(fullPath),
					  KIO::Overwrite | KIO::Resume);
	parent->connect(copyJob,  SIGNAL(copyingDone(KIO::Job *, KUrl, KUrl, time_t, bool, bool)),
					parent, SLOT(slotCopyingDone2(KIO::Job *, KUrl, KUrl)));

	NoteType::Id type = typeForURL(url, parent); // Use the type of the original file because the target doesn't exist yet
	return loadFile(fileName, type, parent);
}

Note* NoteFactory::moveFileAndLoad(const KUrl &url, Basket *parent)
{
	// Globally the same as copyFileAndLoad() but move instead of copy (KIO::move())
	QString fileName = fileNameForNewNote(parent, url.fileName());
	QString fullPath = parent->fullPathForFileName(fileName);

	if (Global::debugWindow)
		*Global::debugWindow << "moveFileAndLoad: " + url.prettyUrl() + " to " + fullPath;

//	QString annotations = i18n("Original file: %1").arg(url.prettyUrl());
//	parent->dontCareOfCreation(fullPath);

	KIO::CopyJob *copyJob = KIO::move(url, KUrl(fullPath),
					  KIO::Overwrite | KIO::Resume);

	parent->connect(copyJob, SIGNAL(copyingDone(KIO::Job *, KUrl, KUrl, time_t, bool, bool)),
					parent, SLOT(slotCopyingDone2(KIO::Job *, KUrl, KUrl)));


	NoteType::Id type = typeForURL(url, parent); // Use the type of the original file because the target doesn't exist yet
	return loadFile(fileName, type, parent);
}

Note* NoteFactory::loadFile(const QString &fileName, Basket *parent)
{
	// The file MUST exists
	QFileInfo file( KUrl(parent->fullPathForFileName(fileName)).path() );
	if ( ! file.exists() )
		return 0L;

	NoteType::Id type = typeForURL(parent->fullPathForFileName(fileName), parent);
	Note *note = loadFile(fileName, type, parent);
	return note;
}

Note* NoteFactory::loadFile(const QString &fileName, NoteType::Id type, Basket *parent)
{
	Note *note = new Note(parent);
	switch (type) {
		case NoteType::Text:      new TextContent(      note, fileName ); break;
		case NoteType::Html:      new HtmlContent(      note, fileName ); break;
		case NoteType::Image:     new ImageContent(     note, fileName ); break;
		case NoteType::Animation: new AnimationContent( note, fileName ); break;
		case NoteType::Sound:     new SoundContent(     note, fileName ); break;
		case NoteType::File:      new FileContent(      note, fileName ); break;
		case NoteType::Launcher:  new LauncherContent(  note, fileName ); break;
		case NoteType::Unknown:   new UnknownContent(   note, fileName ); break;

		default:
		case NoteType::Link:
		case NoteType::Color:
			return 0;
	}

	return note;
}

NoteType::Id NoteFactory::typeForURL(const KUrl &url, Basket */*parent*/)
{
/*	KMimeType::Ptr kMimeType = KMimeType::findByUrl(url);
	if (Global::debugWindow)
		*Global::debugWindow << "typeForURL: " + kMimeType->parentMimeType();//property("MimeType").toString();*/
	bool viewText  = Settings::viewTextFileContent();
	bool viewHTML  = Settings::viewHtmlFileContent();
	bool viewImage = Settings::viewImageFileContent();
	bool viewSound = Settings::viewSoundFileContent();

	KFileMetaInfo metaInfo(url);
	if (Global::debugWindow && !metaInfo.isValid())
		*Global::debugWindow << "typeForURL: metaInfo is empty for " + url.prettyUrl();
	if (metaInfo.isValid()) { // metaInfo is empty for GIF files on my machine !
		if      (viewText  && maybeText(url))             return NoteType::Text;
		else if (viewHTML  && (maybeHtml(url)))           return NoteType::Html;
		else if (viewImage && maybeAnimation(url))        return NoteType::Animation; // See Note::movieStatus(int)
		else if (viewImage && maybeImageOrAnimation(url)) return NoteType::Image;     //  for more explanations
		else if (viewSound && maybeSound(url))            return NoteType::Sound;
		else if (maybeLauncher(url))                      return NoteType::Launcher;
		else                                              return NoteType::File;
	}
	QString mimeType = KMimeType::findByUrl(url)->name();

	if (Global::debugWindow)
		*Global::debugWindow << "typeForURL: " + url.prettyUrl() + " ; MIME type = " + mimeType;

	if      (mimeType == "application/x-desktop")            return NoteType::Launcher;
	else if (viewText  && mimeType.startsWith("text/plain")) return NoteType::Text;
	else if (viewHTML  && mimeType.startsWith("text/html"))  return NoteType::Html;
	else if (viewImage && mimeType == "movie/x-mng")         return NoteType::Animation;
	else if (viewImage && mimeType == "image/gif")           return NoteType::Animation;
	else if (viewImage && mimeType.startsWith("image/"))     return NoteType::Image;
	else if (viewSound && mimeType.startsWith("audio/"))     return NoteType::Sound;
	else                                                     return NoteType::File;
}

QString NoteFactory::fileNameForNewNote(Basket *parent, const QString &wantedName)
{
	return Tools::fileNameForNewFile(wantedName, parent->fullPath());
}

// Create a file to store a new note in Basket parent and with extension extension.
// If wantedName is provided, the function will first try to use this file name, or derive it if it's impossible
//  (extension willn't be used for that case)
QString NoteFactory::createFileForNewNote(Basket *parent, const QString &extension, const QString &wantedName)
{
	static int nb = 1;

	QString fileName;
	QString fullName;

	if (wantedName.isEmpty()) { // TODO: fileNameForNewNote(parent, "note1."+extension);
		QDir dir;
		for (/*int nb = 1*/; ; ++nb) { // TODO: FIXME: If overflow ???
			fileName = "note" + QString::number(nb)/*.rightJustified(5, '0')*/ + "." + extension;
			fullName = parent->fullPath() + fileName;
			dir = QDir(fullName);
			if ( ! dir.exists(fullName) )
				break;
		}
	} else {
		fileName = fileNameForNewNote(parent, wantedName);
		fullName = parent->fullPath() + fileName;
	}

	// Create the file
//	parent->dontCareOfCreation(fullName);
	QFile file(fullName);
	file.open(QIODevice::WriteOnly);
	file.close();

	return fileName;
}

KUrl NoteFactory::filteredURL(const KUrl &url)
{
	// KURIFilter::filteredURI() is slow if the URL contains only letters, digits and '-' or '+'.
	// So, we don't use that function is that case:
	bool isSlow = true;
	for (int i = 0; i < url.url().length(); ++i) {
		QChar c = url.url()[i];
		if (!c.isLetterOrNumber() && c != '-' && c != '+') {
			isSlow = false;
			break;
		}
	}
	if (isSlow)
		return url;
	else
		return KUriFilter::self()->filteredUri(url);
}

QString NoteFactory::titleForURL(const KUrl &url)
{
	QString title = url.prettyUrl();
	QString home  = "file:" + QDir::homePath() + "/";

	if (title.startsWith("mailto:"))
		return title.remove(0, 7);

	if (title.startsWith(home))
		title = "~/" + title.remove(0, home.length());

	if (title.startsWith("file://"))
		title = title.remove(0, 7); // 7 == QString("file://").length() - 1
	else if (title.startsWith("file:"))
		title = title.remove(0, 5); // 5 == QString("file:").length() - 1
	else if (title.startsWith("http://www."))
		title = title.remove(0, 11); // 11 == QString("http://www.").length() - 1
	else if (title.startsWith("http://"))
		title = title.remove(0, 7); // 7 == QString("http://").length() - 1

	if ( ! url.isLocalFile() ) {
		if (title.endsWith("/index.html") && title.length() > 11)
			title.truncate(title.length() - 11); // 11 == QString("/index.html").length()
		else if (title.endsWith("/index.htm") && title.length() > 10)
			title.truncate(title.length() - 10); // 10 == QString("/index.htm").length()
		else if (title.endsWith("/index.xhtml") && title.length() > 12)
			title.truncate(title.length() - 12); // 12 == QString("/index.xhtml").length()
		else if (title.endsWith("/index.php") && title.length() > 10)
			title.truncate(title.length() - 10); // 10 == QString("/index.php").length()
		else if (title.endsWith("/index.asp") && title.length() > 10)
			title.truncate(title.length() - 10); // 10 == QString("/index.asp").length()
		else if (title.endsWith("/index.php3") && title.length() > 11)
			title.truncate(title.length() - 11); // 11 == QString("/index.php3").length()
		else if (title.endsWith("/index.php4") && title.length() > 11)
			title.truncate(title.length() - 11); // 11 == QString("/index.php4").length()
		else if (title.endsWith("/index.php5") && title.length() > 11)
			title.truncate(title.length() - 11); // 11 == QString("/index.php5").length()
	}

	if (title.length() > 2 && title.endsWith("/")) // length > 2 because "/" and "~/" shouldn't be transformed to "" and "~"
		title.truncate(title.length() - 1); // eg. transform "www.kde.org/" to "www.kde.org"

	return title;
}

QString NoteFactory::iconForURL(const KUrl &url)
{
	QString icon = KMimeType::iconNameForUrl(url.url());
	if ( url.protocol() == "mailto" )
		icon = "message";
	return icon;
}

// TODO: Can I add "autoTitle" and "autoIcon" entries to .desktop files? or just store them in basket, as now...

/* Try our better to find an icon suited to the command line
 * eg. "/usr/bin/kwrite-3.2 ~/myfile.txt /home/other/file.xml"
 * will give the "kwrite" icon!
 */
QString NoteFactory::iconForCommand(const QString &command)
{
	QString icon;

	// 1. Use first word as icon (typically the program without argument)
	icon = QStringList::split(' ', command).first();
	// 2. If the command is a full path, take only the program file name
	icon = icon.mid(icon.findRev('/') + 1); // strip path if given [But it doesn't care of such
	                                                   // "myprogram /my/path/argument" -> return "argument". Would
	                                                   // must first strip first word and then strip path... Useful ??
	// 3. Use characters before any '-' (e.g. use "gimp" icon if run command is "gimp-1.3")
	if ( ! isIconExist(icon) )
		icon = QStringList::split('-', icon).first();
	// 4. If the icon still not findable, use a generic icon
	if ( ! isIconExist(icon) )
		icon = "exec";

	return icon;
}

bool NoteFactory::isIconExist(const QString &icon)
{
	return ! KIconLoader::global()->loadIcon(
        icon, KIconLoader::NoGroup, 16, KIconLoader::DefaultState,
        QStringList(), 0L, true
        ).isNull();
}

Note* NoteFactory::createEmptyNote(NoteType::Id type, Basket *parent)
{
	QPixmap *pixmap;
	switch (type) {
		case NoteType::Text:
			return NoteFactory::createNoteText("", parent, /*reallyPlainText=*/true);
		case NoteType::Html:
			return NoteFactory::createNoteHtml("", parent);
		case NoteType::Image:
			pixmap = new QPixmap( QSize(Settings::defImageX(), Settings::defImageY()) );
			pixmap->fill();
			pixmap->setMask(pixmap->createHeuristicMask());
			return NoteFactory::createNoteImage(*pixmap, parent);
		case NoteType::Link:
			return NoteFactory::createNoteLink(KUrl(), parent);
		case NoteType::Launcher:
			return NoteFactory::createNoteLauncher(KUrl(), parent);
		case NoteType::Color:
			return NoteFactory::createNoteColor(Qt::black, parent);
		default:
		case NoteType::Animation:
		case NoteType::Sound:
		case NoteType::File:
		case NoteType::Unknown:
			return 0; // Not possible!
	}
}

Note* NoteFactory::importKMenuLauncher(Basket *parent)
{
	KOpenWithDialog dialog(parent);
	dialog.setSaveNewApplications(true); // To create temp file, needed by createNoteLauncher()
	dialog.exec();
	if (dialog.service()) {
		// * locateLocal() return a local file even if it is a system wide one (local one doesn't exists)
		// * desktopEntryPath() returns the full path for system wide ressources, but relative path if in home
		QString serviceUrl = dialog.service()->entryPath();
		if ( ! serviceUrl.startsWith("/") )
			serviceUrl = dialog.service()->locateLocal(); //locateLocal("xdgdata-apps", serviceUrl);
		return createNoteLauncher(serviceUrl, parent);
	}
	return 0;
}

Note* NoteFactory::importIcon(Basket *parent)
{
	QString iconName = KIconDialog::getIcon( KIconLoader::Desktop, KIconLoader::Application, false, Settings::defIconSize() );
	if ( ! iconName.isEmpty() ) {
		IconSizeDialog dialog(i18n("Import Icon as Image"), i18n("Choose the size of the icon to import as an image:"), iconName, Settings::defIconSize(), 0);
		dialog.exec();
		if (dialog.iconSize() > 0) {
			Settings::setDefIconSize(dialog.iconSize());
			Settings::saveConfig();
			return createNoteImage( DesktopIcon(iconName, dialog.iconSize()), parent ); // TODO: wantedName = iconName !
		}
	}
	return 0;
}

Note* NoteFactory::importFileContent(Basket *parent)
{
	KUrl url = KFileDialog::getOpenUrl(KUrl(), "", parent,
                                       i18n("Load File Content into a Note"));
	if ( ! url.isEmpty() )
		return copyFileAndLoad(url, parent);
	return 0;
}

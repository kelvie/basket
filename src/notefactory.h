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

#ifndef NOTEFACTORY_H
#define NOTEFACTORY_H

#include <qevent.h>
//Added by qt3to4:
#include <QPixmap>
#include <QDropEvent>
#include <kurl.h>
#include <qstringlist.h>

class QString;
class QPixmap;
class QColor;

class Basket;
class Note;

enum NoteType::Id;

/** Factory class to create (new, drop, past) or load BasketIem, and eventuelly save them (?)
  * @author Sébastien Laoût
  */
namespace NoteFactory
{
	/** Functions to create a new note from a content.
	  * Content, if any, is saved to file but the note is not insterted in the basket, and the basket is not saved.
	  * Return 0 if the note has not been successfully created.
	  * In some cases, the returned note can be a group containing several notes or the first note of a chained list.
	  * The method Basket::TODO() can insert several grouped or chained notes without problem.
	  */
	Note* createNoteText(     const QString &text,     Basket *parent, bool reallyPlainText = false);
	Note* createNoteHtml(     const QString &html,     Basket *parent);
	Note* createNoteLink(     const KURL    &url,      Basket *parent);
	Note* createNoteLink(     const KURL    &url,      const QString &title, Basket *parent);
	Note* createNoteImage(    const QPixmap &image,    Basket *parent);
	Note* createNoteColor(    const QColor  &color,    Basket *parent);
	Note* createNoteFromText( const QString &content,  Basket *parent); // Find automatically the type from the text meaning  // TODO: Return Note::List?
	Note* createNoteLauncher( const KURL    &url,      Basket *parent);
	Note* createNoteLauncher( const QString &command,  const QString &name, const QString &icon, Basket *parent);
	Note* createNoteUnknown(  QMimeSource *source,     Basket *parent);
	/** Functions to create derived notes from a content */
	Note* createNoteLinkOrLauncher( const KURL &url,   Basket *parent);
	Note* copyFileAndLoad(    const KURL &url,         Basket *parent);
	Note* moveFileAndLoad(    const KURL &url,         Basket *parent);
	Note* loadFile(           const QString &fileName, Basket *parent); /// << Determine the content of the file (the file SHOULD exists) and return a note of the good type.
	Note* loadFile(           const QString &fileName, NoteType::Id type, Basket *parent ); /// <<  Create a note of type @p type. The file is not obliged to exist.
	/** Functions to create a new note from a drop or past event */
	Note* dropNote(QMimeSource *source, Basket *parent,
	               bool fromDrop = false, QDropEvent::Action action = QDropEvent::Copy, Note *noteSource = 0);
	bool movingNotesInTheSameBasket(QMimeSource *source, Basket *parent, QDropEvent::Action action);
	Note* dropURLs(KURL::List urls, Basket *parent, QDropEvent::Action action, bool fromDrop);
	Note* decodeContent(QDataStream &stream, NoteType::Id type, Basket *parent); /// << Decode the @p stream to a note or return 0 if a general loadFile() is sufficient.
	void consumeContent(QDataStream &stream, NoteType::Id type); /// << Decode the @p stream to a note or return 0 if a general loadFile() is sufficient.
	/** Functions to create a note file but not load it in a note object */
	QString createNoteLauncherFile(const QString &command, const QString &name, const QString &icon, Basket *parent);
	/** Other useful functions */
	NoteType::Id typeForURL(const KURL &url, Basket *parent);
	bool         maybeText(const KURL &url);
	bool         maybeHtml(const KURL &url);
	bool         maybeImageOrAnimation(const KURL &url);
	bool         maybeAnimation(const KURL &url);
	bool         maybeSound(const KURL &url);
	bool         maybeLauncher(const KURL &url);
	QString      fileNameForNewNote(Basket *parent, const QString &wantedName);
	QString      createFileForNewNote(Basket *parent, const QString &extension, const QString &wantedName = "");
	KURL         filteredURL(const KURL &url);
	QString      titleForURL(const KURL &url);
	QString      iconForURL(const KURL &url);
	QString      iconForCommand(const QString &command);
	bool         isIconExist(const QString &icon);
	QStringList  textToURLList(const QString &text); // @Return { url1, title1, url2, title2, url3, title3... }
	/** Insert GUI menu */
	Note* createEmptyNote(  NoteType::Id type, Basket *parent ); // Insert empty if of type Note::Type
	Note* importKMenuLauncher(Basket *parent);
	Note* importIcon(Basket *parent);
	Note* importFileContent(Basket *parent);
}

#endif // NOTEFACTORY_H

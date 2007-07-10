/***************************************************************************
 *   Copyright (C) 2003 by S�bastien Lao�t                                 *
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

#ifndef NOTEDRAG_H
#define NOTEDRAG_H

#include <qstring.h>
#include <qmimedata.h>
#include <qdatastream.h>
#include <qpixmap.h>
#include <qlist.h>
#include <k3multipledrag.h>
#include <k3urldrag.h>
class QDataStream;

class Basket;
class Note;
class NoteSelection;

/** Dragging/Copying/Cutting Scenario:
  * - User select some notes and cut them;
  * - NoteDrag::toMultipleDrag() is called with a tree of the selected notes (see Basket::toSelectionTree()):
  *   - This method create a new K3MultipleDrag object, create a stream,
  *   - And then browse all notes and call the virtual Note::serialize() with the stream as parameter for them to serialize theire content in the "native format".
  *   - This give the MIME type "application/x-basket-note" that will be used by the application to paste the notes exactly as they were.
  *   - Then the method try to set alterante formats for the dragged objects:
  *   - It call successively toText() for each notes and stack up the result so theire is ONE big text flavour to add to the K3MultipleDrag object
  *   - It do the same with toHtml(), toImage() and toLink() to have those flavours as well, if possible...
  *   - If there is only ONE copied note, addAlternateDragObjects() is called on it, so that Unknown objects can be dragged "as is".
  *   - It's OK for the flavours. The method finally set the drag feedback pixmap by asking every selected notes to draw the content to a small pixmap.
  *   - The pixmaps are joined to one big pixmap (but it should not exceed a defined size) and a border is drawn on this image.
  *
  * Pasting/Dropping Scenario:
  *
  * @author S�bastien Lao�t
  */
class NoteDrag
{
  protected:
	static void serializeNotes(     NoteSelection *noteList, QDataStream &stream,         bool cutting );
	static void serializeText(      NoteSelection *noteList, K3MultipleDrag *multipleDrag               );
	static void serializeHtml(      NoteSelection *noteList, K3MultipleDrag *multipleDrag               );
	static void serializeImage(     NoteSelection *noteList, K3MultipleDrag *multipleDrag               );
	static void serializeLinks(     NoteSelection *noteList, K3MultipleDrag *multipleDrag, bool cutting );
	static void setFeedbackPixmap(  NoteSelection *noteList, K3MultipleDrag *multipleDrag               );
	static Note* decodeHierarchy(QDataStream &stream, Basket *parent, bool moveFiles, bool moveNotes, Basket *originalBasket);
  public:
	static QPixmap feedbackPixmap(NoteSelection *noteList);
	static QMimeData* dragObject(NoteSelection *noteList, bool cutting, QWidget *source = 0);
	static bool canDecode(QMimeSource *source);
	static Note* decode(QMimeSource *source, Basket *parent, bool moveFiles, bool moveNotes);
	static Basket* basketOf(QMimeSource *source);
	static QList<Note*> notesOf(QMimeSource *source);
	static void createAndEmptyCuttingTmpFolder();

	static const char *NOTE_MIME_STRING;
};

/** QTextDrag with capabilities to drop GNOME and Mozilla texts
  * as well as UTF-16 texts even if it was supposed to be encoded
  * with local encoding!
  * @author S�bastien Lao�t
  */
class ExtendedTextDrag : public QMimeData
{
  Q_OBJECT
  public:
	static bool decode(const QMimeSource *e, QString &str);
	static bool decode(const QMimeSource *e, QString &str, QByteArray &subtype);
};

// Support KDE 3.3 and older PROTECTED K3URLDrag::encodedData()!

class K3URLDrag2 : public K3URLDrag
{
  Q_OBJECT
  public:
	K3URLDrag2(const KUrl::List &urls) : K3URLDrag(urls) {}
	QByteArray encodedData2(const char *mime) const
	{
		return encodedData(mime);
	}
};

#endif // NOTEDRAG_H

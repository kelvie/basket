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

#ifndef NOTEDRAG_H
#define NOTEDRAG_H

#include <QList>
#include <QDrag>
#include <QGraphicsSceneDragDropEvent>

class QDataStream;
class QDragEnterEvent;
class QPixmap;
class QString;

class BasketScene;
class Note;
class NoteSelection;

/** Dragging/Copying/Cutting Scenario:
  * - User select some notes and cut them;
  * - NoteDrag::toMultipleDrag() is called with a tree of the selected notes (see BasketScene::toSelectionTree()):
  *   - This method create a new QDrag object, create a stream,
  *   - And then browse all notes and call the virtual Note::serialize() with the stream as parameter for them to serialize theire content in the "native format".
  *   - This give the MIME type "application/x-basket-note" that will be used by the application to paste the notes exactly as they were.
  *   - Then the method try to set alterante formats for the dragged objects:
  *   - It call successively toText() for each notes and stack up the result so theire is ONE big text flavour to add to the QDrag object
  *   - It do the same with toHtml(), toImage() and toLink() to have those flavors as well, if possible...
  *   - If there is only ONE copied note, addAlternateDragObjects() is called on it, so that Unknown objects can be dragged "as is".
  *   - It's OK for the flavors. The method finally set the drag feedback pixmap by asking every selected notes to draw the content to a small pixmap.
  *   - The pixmaps are joined to one big pixmap (but it should not exceed a defined size) and a border is drawn on this image.
  *
  * Pasting/Dropping Scenario:
  *
  * @author Sébastien Laoût
  */
class NoteDrag
{
protected:
    static void serializeNotes(NoteSelection *noteList, QDataStream &stream,         bool cutting);
    static void serializeText(NoteSelection *noteList, QDrag *multipleDrag);
    static void serializeHtml(NoteSelection *noteList, QDrag *multipleDrag);
    static void serializeImage(NoteSelection *noteList, QDrag *multipleDrag);
    static void serializeLinks(NoteSelection *noteList, QDrag *multipleDrag, bool cutting);
    static void setFeedbackPixmap(NoteSelection *noteList, QDrag *multipleDrag);
    static Note* decodeHierarchy(QDataStream &stream, BasketScene *parent, bool moveFiles, bool moveNotes, BasketScene *originalBasket);
public:
    static QPixmap feedbackPixmap(NoteSelection *noteList);
    static QDrag* dragObject(NoteSelection *noteList, bool cutting, QWidget *source = 0);
    static bool canDecode(const QMimeData *source);
    static Note* decode(const QMimeData *source, BasketScene *parent, bool moveFiles, bool moveNotes);
    static BasketScene* basketOf(const QMimeData *source);
    static QList<Note*> notesOf(QGraphicsSceneDragDropEvent *source);
    static void createAndEmptyCuttingTmpFolder();

    static const char *NOTE_MIME_STRING;
};

/** QTextDrag with capabilities to drop GNOME and Mozilla texts
  * as well as UTF-16 texts even if it was supposed to be encoded
  * with local encoding!
  * @author Sébastien Laoût
  */
class ExtendedTextDrag : public QDrag
{
    Q_OBJECT
public:
    static bool decode(const QMimeData *e, QString &str);
    static bool decode(const QMimeData *e, QString &str, QString &subtype);
};

#endif // NOTEDRAG_H

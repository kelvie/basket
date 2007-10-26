/***************************************************************************
 *   Copyright (C) 2003 by S�astien Laot                                 *
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

//#include <qdragobject.h>
#include <qdom.h>
#include <qpainter.h>
#include <qstyle.h>
#include <kstyle.h>
#include <qtooltip.h>
#include <qlistview.h>
#include <qcursor.h>
#include <QTextDocument>
#include <qpushbutton.h>
#include <ktextedit.h>
#include <qpoint.h>
#include <qstringlist.h>
#include <kapplication.h>
#include <kglobalsettings.h>
//#include <kopenwith.h>
#include <kservice.h>
#include <klocale.h>
#include <kglobalaccel.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <kfiledialog.h>
#include <kaboutdata.h>
#include <klineedit.h>
#include <ksavefile.h>
#include <kdebug.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <kopenwithdialog.h>
#include <unistd.h> // For sleep()

#include <kmenu.h>
#include <kiconloader.h>
#include <krun.h>

#include <qtoolbar.h>
#include <qclipboard.h>

#include <kmessagebox.h>
#include <qinputdialog.h>

#include <qlayout.h>

#include <stdlib.h>     // rand() function
#include <qdatetime.h>  // seed for rand()

#include "basket.h"
#include "note.h"
#include "notedrag.h"
#include "notefactory.h"
#include "noteedit.h"
#include "tagsedit.h"
#include "xmlwork.h"
#include "global.h"
#include "backgroundmanager.h"
#include "settings.h"
#include "tools.h"
#include "debugwindow.h"
#include "exporterdialog.h"
//#include "config.h"
#include "popupmenu.h"
#include <QLabel>

#ifdef HAVE_LIBGPGME
#include "kgpgme.h"
#endif

#include <iostream>
#include <kcmdlineargs.h>
#include <kactioncollection.h>
#include <kio/copyjob.h>
#include <kcolorscheme.h>
#include <QScrollBar>
#include <kmountpoint.h>

/** Class NoteSelection: */

NoteSelection* NoteSelection::nextStacked()
{
	// First, search in the childs:
	if ( firstChild )
		if ( firstChild->note && firstChild->note->content() )
			return firstChild;
		else
			return firstChild->nextStacked();

	// Then, in the next:
	if ( next )
		if ( next->note && next->note->content() )
			return next;
		else
			return next->nextStacked();

	// And finally, in the parent:
	NoteSelection *node = parent;
	while ( node )
		if ( node->next )
			if ( node->next->note && node->next->note->content() )
				return node->next;
			else
				return node->next->nextStacked();
		else
			node = node->parent;

	// Not found:
	return 0;
}

NoteSelection* NoteSelection::firstStacked()
{
	if ( !this )
		return 0;

	if ( note && note->content() )
		return this;
	else
		return nextStacked();
}

void NoteSelection::append ( NoteSelection *node )
{
	if ( !this || !node )
		return;

	if ( firstChild )
	{
		NoteSelection *last = firstChild;
		while ( last->next )
			last = last->next;
		last->next = node;
	}
	else
		firstChild = node;

	while ( node )
	{
		node->parent = this;
		node = node->next;
	}
}

int NoteSelection::count()
{
	if ( !this )
		return 0;

	int count = 0;

	for ( NoteSelection *node = this; node; node = node->next )
		if ( node->note && node->note->content() )
			++count;
		else
			count += node->firstChild->count();

	return count;
}

QList<Note*> NoteSelection::parentGroups()
{
	QList<Note*> groups;

	// For each note:
	for ( NoteSelection *node = firstStacked(); node; node = node->nextStacked() )
		// For each parent groups of the note:
		for ( Note *note = node->note->parentNote(); note; note = note->parentNote() )
			// Add it (if it was not already in the list):
			if ( !note->isColumn() && !groups.contains ( note ) )
				groups.append ( note );

	return groups;
}

/** Class DecoratedBasket: */

DecoratedBasket::DecoratedBasket ( QWidget *parent, const QString &folderName, Qt::WFlags flags )
		: QWidget ( parent, flags )
{
	kDebug() << (int)parent << endl;
	kDebug() << folderName << endl;
	kDebug() << "---------" << endl;

	m_layout = new QVBoxLayout ( this );
	m_filter = new FilterBar ( this );
	m_basket = new Basket ( this, folderName );
	m_layout->addWidget ( m_basket );
	setFilterBarPosition ( Settings::filterOnTop() );

	m_filter->setShown ( true );
	m_basket->setFocus(); // To avoid the filter bar have focus on load

	connect ( m_filter, SIGNAL ( newFilter ( const FilterData& ) ), m_basket, SLOT ( newFilter ( const FilterData& ) ) );
	connect ( m_filter, SIGNAL ( escapePressed() ),              m_basket, SLOT ( cancelFilter() ) );
	connect ( m_filter, SIGNAL ( returnPressed() ),              m_basket, SLOT ( validateFilter() ) );

	connect ( m_basket, SIGNAL ( postMessage ( const QString& ) ),      Global::bnpView, SLOT ( postStatusbarMessage ( const QString& ) ) );
	connect ( m_basket, SIGNAL ( setStatusBarText ( const QString& ) ), Global::bnpView, SLOT ( setStatusBarHint ( const QString& ) ) );
	connect ( m_basket, SIGNAL ( resetStatusBarText() ),             Global::bnpView, SLOT ( updateStatusBarHint() ) );

	kDebug() << "returning ..." << endl;
}

DecoratedBasket::~DecoratedBasket()
{}

void DecoratedBasket::setFilterBarPosition ( bool onTop )
{
	m_layout->removeWidget ( m_filter );
	if ( onTop )
	{
		m_layout->insertWidget ( 0, m_filter );
		setTabOrder ( this/*(QWidget*)parent()*/, m_filter );
		setTabOrder ( m_filter, m_basket );
		setTabOrder ( m_basket, ( QWidget* ) parent() );
	}
	else
	{
		m_layout->addWidget ( m_filter );
		setTabOrder ( this/*(QWidget*)parent()*/, m_basket );
		setTabOrder ( m_basket, m_filter );
		setTabOrder ( m_filter, ( QWidget* ) parent() );
	}
}

void DecoratedBasket::setFilterBarShown ( bool show, bool switchFocus )
{
//	m_basket->setShowFilterBar(true);//show);
//	m_basket->save();
	// In this order (m_basket and then m_filter) because setShown(false)
	//  will call resetFilter() that will update actions, and then check the
	//  Ctrl+F action whereas it should be unchecked
	//  FIXME: It's very uggly all those things
	m_filter->setShown ( true );//show);
	if ( show )
	{
		if ( switchFocus )
			m_filter->setEditFocus();
	}
	else if ( m_filter->hasEditFocus() )
		m_basket->setFocus();
}

void DecoratedBasket::resetFilter()
{
	m_filter->reset();
}

/** Class TransparentWidget */

TransparentWidget::TransparentWidget ( Basket *basket )
		: QWidget ( basket->viewport() ), m_basket ( basket )
{
	setFocusPolicy ( Qt::NoFocus );
	setMouseTracking ( true ); // To receive mouseMoveEvents

	basket->viewport()->installEventFilter ( this );
}

/*void TransparentWidget::reparent(QWidget *parent, WFlags f, const QPoint &p, bool showIt)
{
	QWidget::reparent(parent, Qt::WNoAutoErase, p, showIt);
}*/

void TransparentWidget::setPosition ( int x, int y )
{
	m_x = x;
	m_y = y;
}

void TransparentWidget::paintEvent ( QPaintEvent*event )
{
	QWidget::paintEvent ( event );
	QPainter painter ( this );

//	painter.save();

	painter.translate ( -m_x, -m_y );
	m_basket->drawContents ( &painter, m_x, m_y, width(), height() );

//	painter.restore();
//	painter.setPen(Qt::blue);
//	painter.drawRect(0, 0, width(), height());
}

void TransparentWidget::mouseMoveEvent ( QMouseEvent *event )
{
	QMouseEvent *translated = new QMouseEvent ( QEvent::MouseMove, ( event->pos() + QPoint ( m_x, m_y ) ), event->button(), event->buttons(),0 );
	m_basket->contentsMouseMoveEvent ( translated );
	delete translated;
}

bool TransparentWidget::eventFilter ( QObject */*object*/, QEvent *event )
{
	// If the parent basket viewport has changed, we should change too:
	if ( event->type() == QEvent::Paint )
		update();

	return false; // Event not consumed, in every cases (because it's only a notification)!
}

/** Class Basket: */

const int Basket::FRAME_DELAY = 50/*1500*/; // Delay between two animation "frames" in milliseconds

/*
 * Convenient function (defined in note.cpp !):
 */
void drawGradient ( QPainter *p, const QColor &colorTop, const QColor & colorBottom,
                    int x, int y, int w, int h,
                    bool sunken, bool horz, bool flat );

/*
 * Defined in note.cpp:
 */
extern void substractRectOnAreas ( const QRect &rectToSubstract, QList<QRect> &areas, bool andRemove = true );

void debugZone ( int zone )
{
	QString s;
	switch ( zone )
	{
		case Note::Handle:        s = "Handle";              break;
		case Note::Group:         s = "Group";               break;
		case Note::TagsArrow:     s = "TagsArrow";           break;
		case Note::Custom0:       s = "Custom0";             break;
		case Note::GroupExpander: s = "GroupExpander";       break;
		case Note::Content:       s = "Content";             break;
		case Note::Link:          s = "Link";                break;
		case Note::TopInsert:     s = "TopInsert";           break;
		case Note::TopGroup:      s = "TopGroup";            break;
		case Note::BottomInsert:  s = "BottomInsert";        break;
		case Note::BottomGroup:   s = "BottomGroup";         break;
		case Note::BottomColumn:  s = "BottomColumn";        break;
		case Note::None:          s = "None";                break;
		default:
			if ( zone == Note::Emblem0 )
				s = "Emblem0";
			else
				s = "Emblem0+" + QString::number ( zone - Note::Emblem0 );
			break;
	}
	std::cout << s.toLatin1().data() << std::endl;
}

#define FOR_EACH_NOTE(noteVar) \
	for (Note *noteVar = firstNote(); noteVar; noteVar = noteVar->next())

void Basket::prependNoteIn ( Note *note, Note *in )
{
	if ( !note )
		// No note to prepend:
		return;

	if ( in )
	{
		// The normal case:
		preparePlug ( note );

		Note *last = note->lastSibling();

		for ( Note *n = note; n; n = n->next() )
			n->setParentNote ( in );
//		note->setPrev(0L);
		last->setNext ( in->firstChild() );

		if ( in->firstChild() )
			in->firstChild()->setPrev ( last );

		in->setFirstChild ( note );

		if ( m_loaded )
			signalCountsChanged();
	}
	else
		// Prepend it directly in the basket:
		appendNoteBefore ( note, firstNote() );
}

void Basket::appendNoteIn ( Note *note, Note *in )
{
	if ( !note )
		// No note to append:
		return;

	if ( in )
	{
		// The normal case:
		preparePlug ( note );

//		Note *last = note->lastSibling();
		Note *lastChild = in->lastChild();

		for ( Note *n = note; n; n = n->next() )
			n->setParentNote ( in );
		note->setPrev ( lastChild );
//		last->setNext(0L);

		if ( !in->firstChild() )
			in->setFirstChild ( note );

		if ( lastChild )
			lastChild->setNext ( note );

		if ( m_loaded )
			signalCountsChanged();
	}
	else
		// Prepend it directly in the basket:
		appendNoteAfter ( note, lastNote() );
}

void Basket::appendNoteAfter ( Note *note, Note *after )
{
	if ( !note )
		// No note to append:
		return;

	if ( !after )
		// By default, insert after the last note:
		after = lastNote();

	if ( m_loaded && after && !after->isFree() && !after->isColumn() )
		for ( Note *n = note; n; n = n->next() )
			n->inheritTagsOf ( after );

//	if (!alreadyInBasket)
	preparePlug ( note );

	Note *last = note->lastSibling();
	if ( after )
	{
		// The normal case:
		for ( Note *n = note; n; n = n->next() )
			n->setParentNote ( after->parentNote() );
		note->setPrev ( after );
		last->setNext ( after->next() );
		after->setNext ( note );
		if ( last->next() )
			last->next()->setPrev ( last );
	}
	else
	{
		// There is no note in the basket:
		for ( Note *n = note; n; n = n->next() )
			n->setParentNote ( 0 );
		m_firstNote = note;
//		note->setPrev(0);
//		last->setNext(0);
	}

//	if (!alreadyInBasket)
	if ( m_loaded )
		signalCountsChanged();
}

void Basket::appendNoteBefore ( Note *note, Note *before )
{
	if ( !note )
		// No note to append:
		return;

	if ( !before )
		// By default, insert before the first note:
		before = firstNote();

	if ( m_loaded && before && !before->isFree() && !before->isColumn() )
		for ( Note *n = note; n; n = n->next() )
			n->inheritTagsOf ( before );

	preparePlug ( note );

	Note *last = note->lastSibling();
	if ( before )
	{
		// The normal case:
		for ( Note *n = note; n; n = n->next() )
			n->setParentNote ( before->parentNote() );
		note->setPrev ( before->prev() );
		last->setNext ( before );
		before->setPrev ( last );
		if ( note->prev() )
			note->prev()->setNext ( note );
		else
		{
			if ( note->parentNote() )
				note->parentNote()->setFirstChild ( note );
			else
				m_firstNote = note;
		}
	}
	else
	{
		// There is no note in the basket:
		for ( Note *n = note; n; n = n->next() )
			n->setParentNote ( 0 );
		m_firstNote = note;
//		note->setPrev(0);
//		last->setNext(0);
	}

	if ( m_loaded )
		signalCountsChanged();
}

DecoratedBasket* Basket::decoration()
{
	return ( DecoratedBasket* ) parent();
}

void Basket::preparePlug ( Note *note )
{
	// Select only the new notes, compute the new notes count and the new number of found notes:
	if ( m_loaded )
		unselectAll();
	int count  = 0;
	int founds = 0;
	Note *last = 0;
	for ( Note *n = note; n; n = n->next() )
	{
		if ( m_loaded )
			n->setSelectedRecursivly ( true ); // Notes should have a parent basket (and they have, so that's OK).
		count  += n->count();
		founds += n->newFilter ( decoration()->filterData() );
		last = n;
	}
	m_count += count;
	m_countFounds += founds;

	// Focus the last inserted note:
	if ( m_loaded && last )
	{
		setFocusedNote ( last );
		m_startOfShiftSelectionNote = ( last->isGroup() ? last->lastRealChild() : last );
	}

	// If some notes don't match (are hidden), tell it to the user:
	if ( m_loaded && founds < count )
	{
		if ( count == 1 )          postMessage ( i18n ( "The new note does not match the filter and is hidden." ) );
		else if ( founds == count - 1 ) postMessage ( i18n ( "A new note does not match the filter and is hidden." ) );
		else if ( founds > 0 )          postMessage ( i18n ( "Some new notes do not match the filter and are hidden." ) );
		else                          postMessage ( i18n ( "The new notes do not match the filter and are hidden." ) );
	}
}

void Basket::unplugNote ( Note *note )
{
	// If there is nothing to do...
	if ( !note )
		return;

//	if (!willBeReplugged) {
	note->setSelectedRecursivly ( false ); // To removeSelectedNote() and decrease the selectedsCount.
	m_count -= note->count();
	m_countFounds -= note->newFilter ( decoration()->filterData() );
	signalCountsChanged();
//	}

	// If it was the first note, change the first note:
	if ( m_firstNote == note )
		m_firstNote = note->next();

	// Change previous and next notes:
	if ( note->prev() )
		note->prev()->setNext ( note->next() );
	if ( note->next() )
		note->next()->setPrev ( note->prev() );

	if ( note->parentNote() )
	{
		// If it was the first note of a group, change the first note of the group:
		if ( note->parentNote()->firstChild() == note )
			note->parentNote()->setFirstChild ( note->next() );

		if ( !note->parentNote()->isColumn() )
		{
			// Ungroup if still 0 note inside parent group:
			if ( ! note->parentNote()->firstChild() )
				unplugNote ( note->parentNote() ); // TODO delete

			// Ungroup if still 1 note inside parent group:
			else if ( ! note->parentNote()->firstChild()->next() )
				ungroupNote ( note->parentNote() );
		}
	}

	note->setParentNote ( 0 );
	note->setPrev ( 0 );
	note->setNext ( 0 );

//	recomputeBlankRects(); // FIXME: called too much time. It's here because when dragging and moving a note to another basket and then go back to the original basket, the note is deleted but the note rect is not painter anymore.
}

void Basket::ungroupNote ( Note *group )
{
	Note *note            = group->firstChild();
	Note *lastGroupedNote = group;
	Note *nextNote;

	// Move all notes after the group (not before, to avoid to change m_firstNote or group->m_firstChild):
	while ( note )
	{
		nextNote = note->next();

		if ( lastGroupedNote->next() )
			lastGroupedNote->next()->setPrev ( note );
		note->setNext ( lastGroupedNote->next() );
		lastGroupedNote->setNext ( note );
		note->setParentNote ( group->parentNote() );
		note->setPrev ( lastGroupedNote );

		note->setGroupWidth ( group->groupWidth() - Note::GROUP_WIDTH );
		lastGroupedNote = note;
		note = nextNote;
	}

	// Unplug the group:
	group->setFirstChild ( 0 );
	unplugNote ( group ); // TODO: delete

	relayoutNotes ( true );
}

void Basket::groupNoteBefore ( Note *note, Note *with )
{
	if ( !note || !with )
		// No note to group or nowhere to group it:
		return;

//	if (m_loaded && before && !with->isFree() && !with->isColumn())
	for ( Note *n = note; n; n = n->next() )
		n->inheritTagsOf ( with );

	preparePlug ( note );

	Note *last = note->lastSibling();

	Note *group = new Note ( this );
	group->setPrev ( with->prev() );
	group->setNext ( with->next() );
	group->setX ( with->x() );
	group->setY ( with->y() );
	if ( with->parentNote() && with->parentNote()->firstChild() == with )
		with->parentNote()->setFirstChild ( group );
	else if ( m_firstNote == with )
		m_firstNote = group;
	group->setParentNote ( with->parentNote() );
	group->setFirstChild ( note );
	group->setGroupWidth ( with->groupWidth() + Note::GROUP_WIDTH );

	if ( with->prev() )
		with->prev()->setNext ( group );
	if ( with->next() )
		with->next()->setPrev ( group );
	with->setParentNote ( group );
	with->setPrev ( last );
	with->setNext ( 0L );

	for ( Note *n = note; n; n = n->next() )
		n->setParentNote ( group );
//	note->setPrev(0L);
	last->setNext ( with );

	if ( m_loaded )
		signalCountsChanged();
}

void Basket::groupNoteAfter ( Note *note, Note *with )
{
	if ( !note || !with )
		// No note to group or nowhere to group it:
		return;

//	if (m_loaded && before && !with->isFree() && !with->isColumn())
	for ( Note *n = note; n; n = n->next() )
		n->inheritTagsOf ( with );

	preparePlug ( note );

//	Note *last = note->lastSibling();

	Note *group = new Note ( this );
	group->setPrev ( with->prev() );
	group->setNext ( with->next() );
	group->setX ( with->x() );
	group->setY ( with->y() );
	if ( with->parentNote() && with->parentNote()->firstChild() == with )
		with->parentNote()->setFirstChild ( group );
	else if ( m_firstNote == with )
		m_firstNote = group;
	group->setParentNote ( with->parentNote() );
	group->setFirstChild ( with );
	group->setGroupWidth ( with->groupWidth() + Note::GROUP_WIDTH );

	if ( with->prev() )
		with->prev()->setNext ( group );
	if ( with->next() )
		with->next()->setPrev ( group );
	with->setParentNote ( group );
	with->setPrev ( 0L );
	with->setNext ( note );

	for ( Note *n = note; n; n = n->next() )
		n->setParentNote ( group );
	note->setPrev ( with );
//	last->setNext(0L);

	if ( m_loaded )
		signalCountsChanged();
}

void Basket::loadNotes ( const QDomElement &notes, Note *parent )
{
	Note *note;
	for ( QDomNode n = notes.firstChild(); !n.isNull(); n = n.nextSibling() )
	{
		QDomElement e = n.toElement();
		if ( e.isNull() ) // Cannot handle that!
			continue;
		note = 0;
		// Load a Group:
		if ( e.tagName() == "group" )
		{
			note = new Note ( this );   // 1. Create the group...
			loadNotes ( e, note );      // 3. ... And populate it with child notes.
			int noteCount = note->count();
			if ( noteCount > 0 || ( parent == 0 && !isFreeLayout() ) )
			{ // But don't remove columns!
				appendNoteIn ( note, parent ); // 2. ... Insert it... FIXME: Initially, the if() the insrtion was the step 2. Was it on purpose?
				// The notes in the group are counted two times (it's why appendNoteIn() was called before loadNotes):
				m_count       -= noteCount;// TODO: Recompute note count every time noteCount() is emitted!
				m_countFounds -= noteCount;
			}
		}
		// Load a Content-Based Note:
		if ( e.tagName() == "note" || e.tagName() == "item" )
		{ // Keep compatible with 0.6.0 Alpha 1
			note = new Note ( this );   // Create the note...
			NoteFactory__loadNode ( XMLWork::getElement ( e, "content" ), e.attribute ( "type" ), note, /*lazyLoad=*/m_finishLoadOnFirstShow ); // ... Populate it with content...
			if ( e.attribute ( "type" ) == "text" )
				m_shouldConvertPlainTextNotes = true; // Convert Pre-0.6.0 baskets: plain text notes should be converted to rich text ones once all is loaded!
			appendNoteIn ( note, parent ); // ... And insert it.
			// Load dates:
			if ( e.hasAttribute ( "added" ) )
				note->setAddedDate ( QDateTime::fromString ( e.attribute ( "added" ),            Qt::ISODate ) );
			if ( e.hasAttribute ( "lastModification" ) )
				note->setLastModificationDate ( QDateTime::fromString ( e.attribute ( "lastModification" ), Qt::ISODate ) );
		}
		// If we successfully loaded a note:
		if ( note )
		{
			// Free Note Properties:
			if ( note->isFree() )
			{
				int x = e.attribute ( "x" ).toInt();
				int y = e.attribute ( "y" ).toInt();
				note->setX ( x < 0 ? 0 : x );
				note->setY ( y < 0 ? 0 : y );
			}
			// Resizeable Note Properties:
			if ( note->hasResizer() || note->isColumn() )
				note->setGroupWidth ( e.attribute ( "width", "200" ).toInt() );
			// Group Properties:
			if ( note->isGroup() && !note->isColumn() && XMLWork::trueOrFalse ( e.attribute ( "folded", "false" ) ) )
				note->toggleFolded ( false );
			// Tags:
			if ( note->content() )
			{
				QString tagsString = XMLWork::getElementText ( e, "tags", "" );
				QStringList tagsId = tagsString.split ( ";" );
				for ( QStringList::iterator it = tagsId.begin(); it != tagsId.end(); ++it )
				{
					State *state = Tag::stateForId ( *it );
					if ( state )
						note->addState ( state, /*orReplace=*/true );
				}
			}
		}
//		kapp->processEvents();
	}
}

void Basket::saveNotes ( QDomDocument &document, QDomElement &element, Note *parent )
{
	Note *note = ( parent ? parent->firstChild() : firstNote() );
	while ( note )
	{
		// Create Element:
		QDomElement noteElement = document.createElement ( note->isGroup() ? "group" : "note" );
		element.appendChild ( noteElement );
		// Free Note Properties:
		if ( note->isFree() )
		{
			noteElement.setAttribute ( "x", note->finalX() );
			noteElement.setAttribute ( "y", note->finalY() );
		}
		// Resizeable Note Properties:
		if ( note->hasResizer() )
			noteElement.setAttribute ( "width", note->groupWidth() );
		// Group Properties:
		if ( note->isGroup() && !note->isColumn() )
			noteElement.setAttribute ( "folded", XMLWork::trueOrFalse ( note->isFolded() ) );
		// Save Content:
		if ( note->content() )
		{
			// Save Dates:
			noteElement.setAttribute ( "added",            note->addedDate().toString ( Qt::ISODate ) );
			noteElement.setAttribute ( "lastModification", note->lastModificationDate().toString ( Qt::ISODate ) );
			// Save Content:
			noteElement.setAttribute ( "type", note->content()->lowerTypeName() );
			QDomElement content = document.createElement ( "content" );
			noteElement.appendChild ( content );
			note->content()->saveToNode ( document, content );
			// Save Tags:
			if ( note->states().count() > 0 )
			{
				QString tags;
				for ( State::List::iterator it = note->states().begin(); it != note->states().end(); ++it )
					tags += ( tags.isEmpty() ? "" : ";" ) + ( *it )->id();
				XMLWork::addElement ( document, noteElement, "tags", tags );
			}
		}
		else
			// Save Child Notes:
			saveNotes ( document, noteElement, note );
		// Go to the Next One:
		note = note->next();
	}
}

void Basket::loadProperties ( const QDomElement &properties )
{
	// Compute Default Values for When Loading the Properties:
	QString defaultBackgroundColor = ( backgroundColorSetting().isValid() ? backgroundColorSetting().name() : "" );
	QString defaultTextColor       = ( textColorSetting().isValid()       ? textColorSetting().name()       : "" );

	// Load the Properties:
	QString icon = XMLWork::getElementText ( properties, "icon", this->icon() );
	QString name = XMLWork::getElementText ( properties, "name", basketName() );

	QDomElement appearance = XMLWork::getElement ( properties, "appearance" );
	// In 0.6.0-Alpha versions, there was a typo error: "backround" instead of "background"
	QString backgroundImage       = appearance.attribute ( "backgroundImage", appearance.attribute ( "backroundImage", backgroundImageName() ) );
	QString backgroundColorString = appearance.attribute ( "backgroundColor", appearance.attribute ( "backroundColor", defaultBackgroundColor ) );
	QString textColorString       = appearance.attribute ( "textColor",      defaultTextColor );
	QColor  backgroundColor = ( backgroundColorString.isEmpty() ? QColor() : QColor ( backgroundColorString ) );
	QColor  textColor       = ( textColorString.isEmpty()       ? QColor() : QColor ( textColorString ) );

	QDomElement disposition = XMLWork::getElement ( properties, "disposition" );
	bool free        = XMLWork::trueOrFalse ( disposition.attribute ( "free",        XMLWork::trueOrFalse ( isFreeLayout() ) ) );
	int  columnCount =                       disposition.attribute ( "columnCount", QString::number ( this->columnsCount() ) ).toInt();
	bool mindMap     = XMLWork::trueOrFalse ( disposition.attribute ( "mindMap",     XMLWork::trueOrFalse ( isMindMap() ) ) );

	QDomElement shortcut = XMLWork::getElement ( properties, "shortcut" );
	QString actionStrings[] = { "show", "globalShow", "globalSwitch" };
	//FIXME 1.5 KShortcut combination  = KShortcut ( shortcut.attribute ( "combination", m_action->shortcut().toString() ) );
	QString   actionString =            shortcut.attribute ( "action" );
	int action = shortcutAction();
	if ( actionString == actionStrings[0] ) action = 0;
	if ( actionString == actionStrings[1] ) action = 1;
	if ( actionString == actionStrings[2] ) action = 2;

	QDomElement protection = XMLWork::getElement ( properties, "protection" );
	m_encryptionType = protection.attribute ( "type" ).toInt();
	m_encryptionKey = protection.attribute ( "key" );

	// Apply the Properties:
	setDisposition ( ( free ? ( mindMap ? 2 : 1 ) : 0 ), columnCount );
	//FIXME 1.5 setShortcut ( combination, action );
	setAppearance ( icon, name, backgroundImage, backgroundColor, textColor ); // Will emit propertiesChanged(this)
}

void Basket::saveProperties ( QDomDocument &document, QDomElement &properties )
{
	XMLWork::addElement ( document, properties, "name", basketName() );
	XMLWork::addElement ( document, properties, "icon", icon() );

	QDomElement appearance = document.createElement ( "appearance" );
	properties.appendChild ( appearance );
	appearance.setAttribute ( "backgroundImage", backgroundImageName() );
	appearance.setAttribute ( "backgroundColor", backgroundColorSetting().isValid() ? backgroundColorSetting().name() : "" );
	appearance.setAttribute ( "textColor",       textColorSetting().isValid()       ? textColorSetting().name()       : "" );

	QDomElement disposition = document.createElement ( "disposition" );
	properties.appendChild ( disposition );
	disposition.setAttribute ( "free",        XMLWork::trueOrFalse ( isFreeLayout() ) );
	disposition.setAttribute ( "columnCount", QString::number ( columnsCount() ) );
	disposition.setAttribute ( "mindMap",     XMLWork::trueOrFalse ( isMindMap() ) );

	QDomElement shortcut = document.createElement ( "shortcut" );
	properties.appendChild ( shortcut );
	QString actionStrings[] = { "show", "globalShow", "globalSwitch" };
	shortcut.setAttribute ( "combination", m_action->shortcut().toString() );
	shortcut.setAttribute ( "action",      actionStrings[shortcutAction() ] );

	QDomElement protection = document.createElement ( "protection" );
	properties.appendChild ( protection );
	protection.setAttribute ( "type", m_encryptionType );
	protection.setAttribute ( "key",  m_encryptionKey );
}

void Basket::subscribeBackgroundImages()
{
	if ( !m_backgroundImageName.isEmpty() )
	{
		Global::backgroundManager->subscribe ( m_backgroundImageName );
		Global::backgroundManager->subscribe ( m_backgroundImageName, this->backgroundColor() );
		Global::backgroundManager->subscribe ( m_backgroundImageName, selectionRectInsideColor() );
		m_backgroundPixmap         = Global::backgroundManager->pixmap ( m_backgroundImageName );
		m_opaqueBackgroundPixmap   = Global::backgroundManager->opaquePixmap ( m_backgroundImageName, this->backgroundColor() );
		m_selectedBackgroundPixmap = Global::backgroundManager->opaquePixmap ( m_backgroundImageName, selectionRectInsideColor() );
		m_backgroundTiled          = Global::backgroundManager->tiled ( m_backgroundImageName );
	}
}

void Basket::unsubscribeBackgroundImages()
{
	if ( hasBackgroundImage() )
	{
		Global::backgroundManager->unsubscribe ( m_backgroundImageName );
		Global::backgroundManager->unsubscribe ( m_backgroundImageName, this->backgroundColor() );
		Global::backgroundManager->unsubscribe ( m_backgroundImageName, selectionRectInsideColor() );
		m_backgroundPixmap         = 0;
		m_opaqueBackgroundPixmap   = 0;
		m_selectedBackgroundPixmap = 0;
	}
}

void Basket::setAppearance ( const QString &icon, const QString &name, const QString &backgroundImage, const QColor &backgroundColor, const QColor &textColor )
{
	unsubscribeBackgroundImages();

	m_icon                   = icon;
	m_basketName             = name;
	m_backgroundImageName    = backgroundImage;
	m_backgroundColorSetting = backgroundColor;
	m_textColorSetting       = textColor;

	m_action->setText ( "BASKET SHORTCUT: " + name );

	// Basket should ALWAYS have an icon (the "basket" icon by default):
	QPixmap iconTest = KIconLoader::global()->loadIcon ( m_icon, KIconLoader::NoGroup, 16, KIconLoader::DefaultState, QStringList(), 0L, /*canReturnNull=*/true );
	if ( iconTest.isNull() )
		m_icon = "basket";

	// We don't request the background images if it's not loaded yet (to make the application startup fast).
	// When the basket is loading (because requested by the user: he/she want to access it)
	// it load the properties, subscribe to (and then load) the images, update the "Loading..." message with the image,
	// load all the notes and it's done!
	if ( m_loadingLaunched )
		subscribeBackgroundImages();

	recomputeAllStyles(); // If a note have a tag with the same background color as the basket one, then display a "..."
	recomputeBlankRects(); // See the drawing of blank areas in Basket::drawContents()
	unbufferizeAll();
//FIXME 1.5	updateContents();

	if ( isDuringEdit() && m_editor->widget() )
	{
		QPalette palette;
		palette.setColor ( m_editor->widget()->backgroundRole(), m_editor->note()->backgroundColor() );
		palette.setColor ( m_editor->widget()->foregroundRole(), m_editor->note()->textColor() );
		m_editor->widget()->setPalette ( palette );
	}

	emit propertiesChanged ( this );
}

void Basket::setDisposition ( int disposition, int columnCount )
{
	static const int COLUMNS_LAYOUT  = 0;
	static const int FREE_LAYOUT     = 1;
	static const int MINDMAPS_LAYOUT = 2;

	int currentDisposition = ( isFreeLayout() ? ( isMindMap() ? MINDMAPS_LAYOUT : FREE_LAYOUT ) : COLUMNS_LAYOUT );

	if ( currentDisposition == COLUMNS_LAYOUT && disposition == COLUMNS_LAYOUT )
	{
		if ( firstNote() && columnCount > m_columnsCount )
		{
			// Insert each new columns:
			for ( int i = m_columnsCount; i < columnCount; ++i )
			{
				Note *newColumn = new Note ( this );
				insertNote ( newColumn, /*clicked=*/lastNote(), /*zone=*/Note::BottomInsert, QPoint(), /*animateNewPosition=*/false );
			}
		}
		else if ( firstNote() && columnCount < m_columnsCount )
		{
			Note *column = firstNote();
			Note *cuttedNotes = 0;
			for ( int i = 1; i <= m_columnsCount; ++i )
			{
				Note *columnToRemove = column;
				column = column->next();
				if ( i > columnCount )
				{
					// Remove the columns that are too much:
					unplugNote ( columnToRemove );
					// "Cut" the content in the columns to be deleted:
					if ( columnToRemove->firstChild() )
					{
						for ( Note *it = columnToRemove->firstChild(); it; it = it->next() )
							it->setParentNote ( 0 );
						if ( !cuttedNotes )
							cuttedNotes = columnToRemove->firstChild();
						else
						{
							Note *lastCuttedNote = cuttedNotes;
							while ( lastCuttedNote->next() )
								lastCuttedNote = lastCuttedNote->next();
							lastCuttedNote->setNext ( columnToRemove->firstChild() );
							columnToRemove->firstChild()->setPrev ( lastCuttedNote );
						}
						columnToRemove->setFirstChild ( 0 );
					}
				}
			}
			// Paste the content in the last column:
			if ( cuttedNotes )
				insertNote ( cuttedNotes, /*clicked=*/lastNote(), /*zone=*/Note::BottomColumn, QPoint(), /*animateNewPosition=*/true );
			unselectAll();
		}
		if ( columnCount != m_columnsCount )
		{
			m_columnsCount = ( columnCount <= 0 ? 1 : columnCount );
			equalizeColumnSizes(); // Will relayoutNotes()
		}
	}
	else if ( currentDisposition == COLUMNS_LAYOUT && ( disposition == FREE_LAYOUT || disposition == MINDMAPS_LAYOUT ) )
	{
		Note *column = firstNote();
		m_columnsCount = 0; // Now, so relayoutNotes() will not relayout the free notes as if they were columns!
		while ( column )
		{
			// Move all childs on the first level:
			Note *nextColumn = column->next();
			ungroupNote ( column );
			column = nextColumn;
		}
		unselectAll();
		m_mindMap = ( disposition == MINDMAPS_LAYOUT );
		relayoutNotes ( true );
	}
	else if ( ( currentDisposition == FREE_LAYOUT || currentDisposition == MINDMAPS_LAYOUT ) && disposition == COLUMNS_LAYOUT )
	{
		if ( firstNote() )
		{
			// TODO: Reorder notes!
			// Remove all notes (but keep a reference to them, we're not crazy ;-) ):
			Note *notes = m_firstNote;
			m_firstNote = 0;
			m_count = 0;
			m_countFounds = 0;
			// Insert the number of columns that is needed:
			Note *lastInsertedColumn = 0;
			for ( int i = 0; i < columnCount; ++i )
			{
				Note *column = new Note ( this );
				if ( lastInsertedColumn )
					insertNote ( column, /*clicked=*/lastInsertedColumn, /*zone=*/Note::BottomInsert, QPoint(), /*animateNewPosition=*/false );
				else
					m_firstNote = column;
				lastInsertedColumn = column;
			}
			// Reinsert the old notes in the first column:
			insertNote ( notes, /*clicked=*/firstNote(), /*zone=*/Note::BottomColumn, QPoint(), /*animateNewPosition=*/true );
			unselectAll();
		}
		else
		{
			// Insert the number of columns that is needed:
			Note *lastInsertedColumn = 0;
			for ( int i = 0; i < columnCount; ++i )
			{
				Note *column = new Note ( this );
				if ( lastInsertedColumn )
					insertNote ( column, /*clicked=*/lastInsertedColumn, /*zone=*/Note::BottomInsert, QPoint(), /*animateNewPosition=*/false );
				else
					m_firstNote = column;
				lastInsertedColumn = column;
			}
		}
		m_columnsCount = ( columnCount <= 0 ? 1 : columnCount );
		equalizeColumnSizes(); // Will relayoutNotes()
	}
}

void Basket::equalizeColumnSizes()
{
	if ( !firstNote() )
		return;

	// Necessary to know the available space;
	relayoutNotes ( true );

	int availableSpace = width();
	int columnWidth = ( width() - ( columnsCount()-1 ) *Note::GROUP_WIDTH ) / columnsCount();
	int columnCount = columnsCount();
	Note *column = firstNote();
	while ( column )
	{
		int minGroupWidth = column->minRight() - column->x();
		if ( minGroupWidth > columnWidth )
		{
			availableSpace -= minGroupWidth;
			--columnCount;
		}
		column = column->next();
	}
	columnWidth = ( availableSpace - ( columnsCount()-1 ) *Note::GROUP_WIDTH ) / columnCount;

	column = firstNote();
	while ( column )
	{
		int minGroupWidth = column->minRight() - column->x();
		if ( minGroupWidth > columnWidth )
			column->setGroupWidth ( minGroupWidth );
		else
			column->setGroupWidth ( columnWidth );
		column = column->next();
	}

	relayoutNotes ( true );
}

void Basket::enableActions()
{
	Global::bnpView->enableActions();
	setFocusPolicy ( isLocked() ? Qt::NoFocus : Qt::StrongFocus );
	if ( isLocked() )
		viewport()->setCursor ( Qt::ArrowCursor ); // When locking, the cursor stays the last form it was
}

bool Basket::save()
{
	if ( !m_loaded )
		return false;

	DEBUG_WIN << "Basket[" + folderName() + "]: Saving...";

	// Create Document:
	QDomDocument document ( /*doctype=*/"basket" );
	QDomElement root = document.createElement ( "basket" );
	document.appendChild ( root );

	// Create Properties Element and Populate It:
	QDomElement properties = document.createElement ( "properties" );
	saveProperties ( document, properties );
	root.appendChild ( properties );

	// Create Notes Element and Populate It:
	QDomElement notes = document.createElement ( "notes" );
	saveNotes ( document, notes, 0 );
	root.appendChild ( notes );

	// Write to Disk:
	if ( !saveToFile ( fullPath() + ".basket", "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n" + document.toString() ) )
	{
		DEBUG_WIN << "Basket[" + folderName() + "]: <font color=red>FAILED to save</font>!";
		return false;
	}

	Global::bnpView->setUnsavedStatus ( false );
	return true;
}

void Basket::aboutToBeActivated()
{
	if ( m_finishLoadOnFirstShow )
	{
		FOR_EACH_NOTE ( note )
		note->finishLazyLoad();

		//relayoutNotes(/*animate=*/false);
		setFocusedNote ( 0 ); // So that during the focusInEvent that will come shortly, the FIRST note is focused.

		if ( Settings::playAnimations() && !decoration()->filterBar()->filterData().isFiltering && Global::bnpView->currentBasket() == this ) // No animation when filtering all!
			animateLoad();//QTimer::singleShot( 0, this, SLOT(animateLoad()) );

		m_finishLoadOnFirstShow = false;
	}
}

void Basket::load()
{
	// Load only once:
	if ( m_loadingLaunched )
		return;
	m_loadingLaunched = true;

//	StopWatch::start(10);

	DEBUG_WIN << "Basket[" + folderName() + "]: Loading...";
	QDomDocument *doc = 0;
	QString content;

//	StopWatch::start(0);

	if ( loadFromFile ( fullPath() + ".basket", &content ) )
	{
		doc = new QDomDocument ( "basket" );
		if ( ! doc->setContent ( content ) )
		{
			DEBUG_WIN << "Basket[" + folderName() + "]: <font color=red>FAILED to parse XML</font>!";
			delete doc;
			doc = 0;
		}
	}
	if ( isEncrypted() )
		DEBUG_WIN << "Basket is encrypted.";
	if ( ! doc )
	{
		DEBUG_WIN << "Basket[" + folderName() + "]: <font color=red>FAILED to load</font>!";
		m_loadingLaunched = false;
		if ( isEncrypted() )
			m_locked = true;
		Global::bnpView->notesStateChanged(); // Show "Locked" instead of "Loading..." in the statusbar
		return;
	}
	m_locked = false;

	QDomElement docElem = doc->documentElement();
	QDomElement properties = XMLWork::getElement ( docElem, "properties" );

	loadProperties ( properties ); // Since we are loading, this time the background image will also be loaded!
	// Now that the background image is loaded and subscribed, we display it during the load process:
	delete doc;
//FIXME 1.5	updateContents();
//	kapp->processEvents();

	//BEGIN Compatibility with 0.6.0 Pre-Alpha versions:
	QDomElement notes = XMLWork::getElement ( docElem, "notes" );
	if ( notes.isNull() )
		notes = XMLWork::getElement ( docElem, "items" );
	m_watcher->stopScan();
	m_shouldConvertPlainTextNotes = false; // Convert Pre-0.6.0 baskets: plain text notes should be converted to rich text ones once all is loaded!
//	StopWatch::check(0);

//	StopWatch::start(1);
	m_finishLoadOnFirstShow = ( Global::bnpView->currentBasket() != this );
	loadNotes ( notes, 0L );
//	StopWatch::check(1);
//	StopWatch::start(2);

	if ( m_shouldConvertPlainTextNotes )
		convertTexts();
	m_watcher->startScan();
	//loadNotes(XMLWork::getElement(docElem, "notes"), 0L);
	//END

//	StopWatch::check(0);

	signalCountsChanged();
	if ( isColumnsLayout() )
	{
		// Count the number of columns:
		int columnsCount = 0;
		Note *column = firstNote();
		while ( column )
		{
			++columnsCount;
			column = column->next();
		}
		m_columnsCount = columnsCount;
	}

	relayoutNotes ( false );

	// On application start, the current basket is not focused yet, so the focus rectangle is not shown when calling focusANote():
	if ( Global::bnpView->currentBasket() == this )
		setFocus();
	focusANote();

	if ( Settings::playAnimations() && !decoration()->filterBar()->filterData().isFiltering && Global::bnpView->currentBasket() == this ) // No animation when filtering all!
		animateLoad();//QTimer::singleShot( 0, this, SLOT(animateLoad()) );
	else
		m_loaded = true;
	enableActions();
//	StopWatch::check(2);

//	StopWatch::check(10);
}

void Basket::filterAgain ( bool andEnsureVisible/* = true*/ )
{
	newFilter ( decoration()->filterData(), andEnsureVisible );
}

void Basket::filterAgainDelayed()
{
	QTimer::singleShot ( 0, this, SLOT ( filterAgain() ) );
}

void Basket::newFilter ( const FilterData &data, bool andEnsureVisible/* = true*/ )
{
	if ( !isLoaded() )
		return;

//StopWatch::start(20);

	m_countFounds = 0;
	for ( Note *note = firstNote(); note; note = note->next() )
		m_countFounds += note->newFilter ( data );

	relayoutNotes ( true );
	signalCountsChanged();

	if ( hasFocus() ) // if (!hasFocus()), focusANote() will be called at focusInEvent()
		focusANote(); //  so, we avoid de-focus a note if it will be re-shown soon
	if ( andEnsureVisible && m_focusedNote != 0L )
		ensureNoteVisible ( m_focusedNote );

	Global::bnpView->setFiltering ( data.isFiltering );

//StopWatch::check(20);
}

void Basket::cancelFilter()
{
	decoration()->filterBar()->reset();
	validateFilter();
}

void Basket::validateFilter()
{
	if ( isDuringEdit() )
		m_editor->widget()->setFocus();
	else
		setFocus();
}

bool Basket::isFiltering()
{
	return decoration()->filterBar()->filterData().isFiltering;
}



QString Basket::fullPath()
{
	return Global::basketsFolder() + folderName();
}

QString Basket::fullPathForFileName ( const QString &fileName )
{
	return fullPath() + fileName;
}

/*static*/ QString Basket::fullPathForFolderName ( const QString &folderName )
{
	return Global::basketsFolder() + folderName;
}


void Basket::setShortcut ( KShortcut shortcut, int action )
{
// /*FIXME 1.5	if ( !Global::globalAccel )
// 		return;
// 	QString sAction = "global_basket_activate_" + folderName();
// 	Global::globalAccel->remove ( sAction );
// 	Global::globalAccel->updateConnections();
// 
// 	m_action->setShortcut ( shortcut );
// 	m_shortcutAction = action;
// 
// 	if ( action > 0 )
// 		Global::globalAccel->insert ( sAction, m_action->text(), /*whatsThis=*/"", m_action->shortcut(), KShortcut(), this, SLOT ( activatedShortcut() ), /*configurable=*/false );
// 	Global::globalAccel->updateConnections();*/
}

void Basket::activatedShortcut()
{
	Global::bnpView->setCurrentBasket ( this );

	if ( m_shortcutAction == 1 )
		Global::bnpView->setActive ( true );
}

void Basket::signalCountsChanged()
{
	if ( !m_timerCountsChanged.isActive() ){
		m_timerCountsChanged.setSingleShot ( true );
		m_timerCountsChanged.start ( 0/*ms*/);
}
}

void Basket::countsChangedTimeOut()
{
	emit countsChanged ( this );
}


Basket::Basket ( QWidget *parent, const QString &folderName )
		: QScrollArea ( parent ),
		//QToolTip ( widget() ),
		m_noActionOnMouseRelease ( false ), m_ignoreCloseEditorOnNextMouseRelease ( false ), m_pressPos ( -100, -100 ), m_canDrag ( false ),
		m_firstNote ( 0 ), m_columnsCount ( 1 ), m_mindMap ( false ), m_resizingNote ( 0L ), m_pickedResizer ( 0 ), m_movingNote ( 0L ), m_pickedHandle ( 0, 0 ),
		m_clickedToInsert ( 0 ), m_zoneToInsert ( 0 ), m_posToInsert ( -1, -1 ),
		m_isInsertPopupMenu ( false ),
		m_loaded ( false ), m_loadingLaunched ( false ), m_locked ( false ), m_decryptBox ( 0 ), m_button ( 0 ), m_encryptionType ( NoEncryption ),
#ifdef HAVE_LIBGPGME
		m_gpg ( 0 ),
#endif
		m_backgroundPixmap ( 0 ), m_opaqueBackgroundPixmap ( 0 ), m_selectedBackgroundPixmap ( 0 ),
		m_action ( 0 ), m_shortcutAction ( 0 ),
		m_hoveredNote ( 0 ), m_hoveredZone ( Note::None ), m_lockedHovering ( false ), m_underMouse ( false ),
		m_inserterRect(), m_inserterShown ( false ), m_inserterSplit ( true ), m_inserterTop ( false ), m_inserterGroup ( false ),
		m_isSelecting ( false ), m_selectionStarted ( false ),
		m_count ( 0 ), m_countFounds ( 0 ), m_countSelecteds ( 0 ),
		m_folderName ( folderName ),
		m_editor ( 0 ), m_leftEditorBorder ( 0 ), m_rightEditorBorder ( 0 ), m_redirectEditActions ( false ), m_editorWidth ( -1 ), m_editorHeight ( -1 ),
		m_doNotCloseEditor ( false ), m_editParagraph ( 0 ), m_editIndex ( 0 ),
		m_isDuringDrag ( false ), m_draggedNotes(),
		m_focusedNote ( 0 ), m_startOfShiftSelectionNote ( 0 ),
		m_finishLoadOnFirstShow ( false ), m_relayoutOnNextShow ( false )
{
	kDebug() << "Creating real basket" << endl;
	this->setWidget( new QWidget() );

	QString sAction = "local_basket_activate_" + folderName;
	
	m_action = new KAction ( this);
	m_action->setText(i18n("FAKE TEXT"));
	m_action->setIcon(KIcon("FAKE ICON"));
	m_action->setShortcut( KShortcut());
	Global::bnpView->actionCollection()->addAction(sAction,m_action);
	connect(m_action, SIGNAL(triggered(bool)), SLOT ( activatedShortcut() ));
	m_action->setShortcutConfigurable ( false ); // We do it in the basket properties dialog (and keep it in sync with the global one)

	if ( !m_folderName.endsWith ( "/" ) )
		m_folderName += "/";

	setFocusPolicy ( Qt::StrongFocus );
//FIXME 1.5	setDragAutoScroll ( true );

	// By default, there is no corner widget: we set one for the corner area to be painted!
	// If we don't set one and there are two scrollbars present, slowly resizing up the window show graphical glitches in that area!
	//FIXME m_cornerWidget = new QWidget ( this );
	//FIXME setCornerWidget ( m_cornerWidget );

	widget()->setAcceptDrops ( true );
	widget()->setMouseTracking ( true );

	// File Watcher:
	m_watcher = new KDirWatch ( this );
	connect ( m_watcher,       SIGNAL ( dirty ( const QString& ) ),   this, SLOT ( watchedFileModified ( const QString& ) ) );
	connect ( m_watcher,       SIGNAL ( deleted ( const QString& ) ), this, SLOT ( watchedFileDeleted ( const QString& ) ) );
	connect ( &m_watcherTimer, SIGNAL ( timeout() ),               this, SLOT ( updateModifiedNotes() ) );

	// Various Connections:
	connect ( &m_animationTimer,           SIGNAL ( timeout() ),   this, SLOT ( animateObjects() ) );
	connect ( &m_autoScrollSelectionTimer, SIGNAL ( timeout() ),   this, SLOT ( doAutoScrollSelection() ) );
	connect ( &m_timerCountsChanged,       SIGNAL ( timeout() ),   this, SLOT ( countsChangedTimeOut() ) );
	connect ( &m_inactivityAutoSaveTimer,  SIGNAL ( timeout() ),   this, SLOT ( inactivityAutoSaveTimeout() ) );
	connect ( &m_inactivityAutoLockTimer,  SIGNAL ( timeout() ),   this, SLOT ( inactivityAutoLockTimeout() ) );
	connect ( this, SIGNAL ( contentsMoving ( int, int ) ), this, SLOT ( contentsMoved() ) );
#ifdef HAVE_LIBGPGME
	m_gpg = new KGpgMe();
#endif
	m_locked = isFileEncrypted();

	kDebug() << "exiting" << endl;
}

void Basket::contentsMoved()
{
	// This slot is called BEFORE the content move, so we delay the hover effects:
	QTimer::singleShot ( 0, this, SLOT ( doHoverEffects() ) );
}

void Basket::enterEvent ( QEvent * )
{
	m_underMouse = true;
	doHoverEffects();
}

void Basket::leaveEvent ( QEvent * )
{
	m_underMouse = false;
	doHoverEffects();

	if ( m_lockedHovering )
		return;

	removeInserter();
	if ( m_hoveredNote )
	{
		m_hoveredNote->setHovered ( false );
		m_hoveredNote->setHoveredZone ( Note::None );
		updateNote ( m_hoveredNote );
	}
	m_hoveredNote = 0;
}

void Basket::setFocusIfNotInPopupMenu()
{
	if ( !kapp->activePopupWidget() )
		if ( isDuringEdit() )
			m_editor->widget()->setFocus();
		else
			setFocus();
}

void Basket::contentsMousePressEvent ( QMouseEvent *event )
{
	// If user click the basket, focus it!
	// The focus is delayed because if the click results in showing a popup menu,
	// the interface flicker by showing the focused rectangle (as the basket gets focus)
	// and immediatly removing it (because the popup menu now have focus).
	if ( !isDuringEdit() )
		QTimer::singleShot ( 0, this, SLOT ( setFocusIfNotInPopupMenu() ) );

	// Convenient variables:
	bool controlPressed = event->buttons() & Qt::ControlModifier;
	bool shiftPressed   = event->buttons() & Qt::ShiftModifier;

	// Do nothing if we disabled the click some milliseconds sooner.
	// For instance when a popup menu has been closed with click, we should not do action:
	if ( event->button() == Qt::LeftButton && ( kapp->activePopupWidget() || m_lastDisableClick.msecsTo ( QTime::currentTime() ) <= 80 ) )
	{
		doHoverEffects();
		m_noActionOnMouseRelease = true;
		// But we allow to select:
		// The code is the same as at the bottom of this method:
		if ( event->button() == Qt::LeftButton )
		{
			m_selectionStarted = true;
			m_selectionBeginPoint = event->pos();
			m_selectionInvert = controlPressed || shiftPressed;
		}
		return;
	}

	// Figure out what is the clicked note and zone:
	Note *clicked = noteAt ( event->pos().x(), event->pos().y() );
	Note::Zone zone = ( clicked ? clicked->zoneAt ( event->pos() - QPoint ( clicked->x(), clicked->y() ) ) : Note::None );

	// Popup Tags menu:
	if ( zone == Note::TagsArrow && !controlPressed && !shiftPressed && event->button() != Qt::MidButton )
	{
		if ( !clicked->isSelected() )
			unselectAllBut ( clicked );
		setFocusedNote ( clicked ); /// /// ///
		m_startOfShiftSelectionNote = clicked;
		m_noActionOnMouseRelease = true;
		popupTagsMenu ( clicked );
		return;
	}

	if ( event->button() == Qt::LeftButton )
	{
		// Prepare to allow drag and drop when moving mouse further:
		if ( ( zone == Note::Handle || zone == Note::Group ) ||
		        ( clicked && clicked->isSelected() &&
		          ( zone == Note::TagsArrow || zone == Note::Custom0 || zone == Note::Content || zone == Note::Link /**/ || zone >= Note::Emblem0 /**/ ) ) )
		{
			if ( !shiftPressed && !controlPressed )
			{
				m_pressPos = event->pos(); // TODO: Allow to drag emblems to assign them to other notes. Then don't allow drag at Emblem0!!
				m_canDrag  = true;
				// Saving where we were editing, because during a drag, the mouse can fly over the text edit and move the cursor position:
				if ( m_editor && m_editor->textEdit() )
				{
					QTextEdit *editor = m_editor->textEdit();
					m_editParagraph=editor->textCursor().anchor();
					m_editIndex=editor->textCursor().position();
				}
			}
		}

		// Initializing Resizer move:
		if ( zone == Note::Resizer )
		{
			m_resizingNote  = clicked;
			m_pickedResizer = event->pos().x() - clicked->rightLimit();
			m_noActionOnMouseRelease = true;
			m_lockedHovering = true;
			return;
		}

		// Select note(s):
		if ( zone == Note::Handle || zone == Note::Group || ( zone == Note::GroupExpander && ( controlPressed || shiftPressed ) ) )
		{
			Note *end = clicked;
			if ( clicked->isGroup() && shiftPressed )
			{
				if ( clicked->contains ( m_startOfShiftSelectionNote ) )
				{
					m_startOfShiftSelectionNote = clicked->firstRealChild();
					end = clicked->lastRealChild();
				}
				else if ( clicked->firstRealChild()->isAfter ( m_startOfShiftSelectionNote ) )
					end = clicked->lastRealChild();
				else
					end = clicked->firstRealChild();
			}
			if ( controlPressed && shiftPressed )
				selectRange ( m_startOfShiftSelectionNote, end, /*unselectOthers=*/false );
			else if ( shiftPressed )
				selectRange ( m_startOfShiftSelectionNote, end );
			else if ( controlPressed )
				clicked->setSelectedRecursivly ( !clicked->allSelected() );
			else if ( !clicked->allSelected() )
				unselectAllBut ( clicked );
			setFocusedNote ( end ); /// /// ///
			m_startOfShiftSelectionNote = ( end->isGroup() ? end->firstRealChild() : end );
			//m_noActionOnMouseRelease = false;
			m_noActionOnMouseRelease = true;
			return;
		}
		// MOVED TO RELEASE EVENT:
		/* else if (clicked && zone != Note::None && zone != Note::BottomColumn && zone != Note::Resizer && (controlPressed || shiftPressed)) {
			if (controlPressed && shiftPressed)
				selectRange(m_startOfShiftSelectionNote, clicked, / *unselectOthers=* /false);
			else if (shiftPressed)
				selectRange(m_startOfShiftSelectionNote, clicked);
			else if (controlPressed)
				clicked->setSelectedRecursivly(!clicked->allSelected());
			setFocusedNote(clicked); /// /// ///
			m_startOfShiftSelectionNote = (clicked->isGroup() ? clicked->firstRealChild() : clicked);
			m_noActionOnMouseRelease = true;
			return;
		}*/

		// Initializing Note move:
		/*		if ((zone == Note::Group || zone == Note::Handle) && clicked->isFree()) {
					m_movingNote   = clicked;
					m_pickedHandle = QPoint(event->pos().x() - clicked->x(), event->pos().y() - clicked->y());
					m_noActionOnMouseRelease = true;
					m_lockedHovering = true;
					return;
				}
		*/

		// Folding/Unfolding group:
		if ( zone == Note::GroupExpander )
		{
			clicked->toggleFolded ( Settings::playAnimations() );
			relayoutNotes ( true );
			m_noActionOnMouseRelease = true;
			return;
		}
	}

	// Popup menu for tag emblems:
	if ( event->button() == Qt::RightButton && zone >= Note::Emblem0 )
	{
		if ( !clicked->isSelected() )
			unselectAllBut ( clicked );
		setFocusedNote ( clicked ); /// /// ///
		m_startOfShiftSelectionNote = clicked;
		popupEmblemMenu ( clicked, zone - Note::Emblem0 );
		m_noActionOnMouseRelease = true;
		return;
	}

	// Insertion Popup Menu:
	if ( ( event->button() == Qt::RightButton ) &&
	        ( ( !clicked && isFreeLayout() ) ||
	          ( clicked && ( zone == Note::TopInsert || zone == Note::TopGroup || zone == Note::BottomInsert || zone == Note::BottomGroup || zone == Note::BottomColumn ) ) ) )
	{
		unselectAll();
		m_clickedToInsert = clicked;
		m_zoneToInsert    = zone;
		m_posToInsert     = event->pos();
		KMenu* menu = ( KMenu* ) ( Global::bnpView->popupMenu ( "insert_popup" ) );
		foreach(QAction* a, menu->actions()){
		if(a->text()==i18nc ( "The verb (Group New Note)", "Group" ) || a->text()==i18nc ( "The verb (Insert New Note)", "Insert" )){
}
			 // If we already added a title, remove it because it would be kept and then added several times:
			menu->removeAction ( a );
			delete a;
		}
		menu->addAction ( ( zone == Note::TopGroup || zone == Note::BottomGroup ? i18nc ( "The verb (Group New Note)", "Group" ) : i18nc ( "The verb (Insert New Note)", "Insert" )  ));
		setInsertPopupMenu();
		connect ( menu, SIGNAL ( aboutToHide() ),  this, SLOT ( delayedCancelInsertPopupMenu() ) );
		connect ( menu, SIGNAL ( aboutToHide() ),  this, SLOT ( unlockHovering() ) );
		connect ( menu, SIGNAL ( aboutToHide() ),  this, SLOT ( disableNextClick() ) );
		connect ( menu, SIGNAL ( aboutToHide() ),  this, SLOT ( hideInsertPopupMenu() ) );
		doHoverEffects ( clicked, zone ); // In the case where another popup menu was open, we should do that manually!
		m_lockedHovering = true;
		menu->exec ( QCursor::pos() );
		m_noActionOnMouseRelease = true;
		return;
	}

	// Note Context Menu:
	if ( event->button() == Qt::RightButton && clicked && !clicked->isColumn() && zone != Note::Resizer )
	{
		if ( !clicked->isSelected() )
			unselectAllBut ( clicked );
		setFocusedNote ( clicked ); /// /// ///
		if ( editedNote() == clicked )
		{
			closeEditor();
			clicked->setSelected ( true );
		}
		m_startOfShiftSelectionNote = ( clicked->isGroup() ? clicked->firstRealChild() : clicked );
		QMenu* menu = Global::bnpView->popupMenu ( "note_popup" );
		connect ( menu, SIGNAL ( aboutToHide() ),  this, SLOT ( unlockHovering() ) );
		connect ( menu, SIGNAL ( aboutToHide() ),  this, SLOT ( disableNextClick() ) );
		doHoverEffects ( clicked, zone ); // In the case where another popup menu was open, we should do that manually!
		m_lockedHovering = true;
		menu->exec ( QCursor::pos() );
		m_noActionOnMouseRelease = true;
		return;
	}

	// Paste selection under cursor (but not "create new primary note under cursor" because this is on moveRelease):
	if ( event->button() == Qt::MidButton && zone != Note::Resizer && ( !isDuringEdit() || clicked != editedNote() ) )
	{
		if ( ( Settings::middleAction() != 0 ) && ( event->modifiers () == Qt::ShiftModifier ) )
		{
			m_clickedToInsert = clicked;
			m_zoneToInsert    = zone;
			m_posToInsert     = event->pos();
			closeEditor();
			removeInserter();                     // If clicked at an insertion line and the new note shows a dialog for editing,
			NoteType::Id type = ( NoteType::Id ) 0;  //  hide that inserter before the note edition instead of after the dialog is closed
			switch ( Settings::middleAction() )
			{
				case 1:
					m_isInsertPopupMenu = true;
					pasteNote();
					break;
				case 2: type = NoteType::Image;    break;
				case 3: type = NoteType::Link;     break;
				case 4: type = NoteType::Launcher; break;
				default:
					m_noActionOnMouseRelease = false;
					return; // Other options should be done on mouse release (to avoid mouse release to cancel them!)
					/*				case 5: type = NoteType::Color;    break;
									case 6:
										Global::bnpView->grabScreenshot();
										break;
									case 7:
										Global::bnpView->slotColorFromScreen();
										break;
									case 8:
										Global::bnpView->insertWizard(3); // loadFromFile
										break;
									case 9:
										Global::bnpView->insertWizard(1); // importKMenuLauncher
										break;
									case 10:
										Global::bnpView->insertWizard(2); // importIcon
										break;
					*/
			}
			if ( type != 0 )
			{
				m_ignoreCloseEditorOnNextMouseRelease = true;
				Global::bnpView->insertEmpty ( type );
			}
		}
		else
		{
			if ( clicked )
				zone = clicked->zoneAt ( event->pos() - QPoint ( clicked->x(), clicked->y() ), true );
			closeEditor();
			clickedToInsert ( event, clicked, zone );
			save();
		}
		m_noActionOnMouseRelease = true;
		return;
	}

	// Finally, no action has been done durint pressEvent, so an action can be done on releaseEvent:
	m_noActionOnMouseRelease = false;

	/* Selection scenario:
	 * On contentsMousePressEvent, put m_selectionStarted to true and init Begin and End selection point.
	 * On contentsMouseMoveEvent, if m_selectionStarted, update End selection point, update selection rect,
	 * and if it's larger, switching to m_isSelecting mode: we can draw the selection rectangle.
	 */
	// Prepare selection:
	if ( event->button() == Qt::LeftButton )
	{
		m_selectionStarted = true;
		m_selectionBeginPoint = event->pos();
		// We usualy invert the selection with the Ctrl key, but some environements (like GNOME or The Gimp) do it with the Shift key.
		// Since the Shift key has no specific usage, we allow to invert selection ALSO with Shift for Gimp people
		m_selectionInvert = controlPressed || shiftPressed;
	}
}

void Basket::delayedCancelInsertPopupMenu()
{
	QTimer::singleShot ( 0, this, SLOT ( cancelInsertPopupMenu() ) );
}

void Basket::contentsContextMenuEvent ( QContextMenuEvent *event )
{
	if ( event->reason() == QContextMenuEvent::Keyboard )
	{
		if ( countFounds/*countShown*/() == 0 )
		{ // TODO: Count shown!!
			QRect basketRect ( mapToGlobal ( QPoint ( 0,0 ) ), size() );
			QMenu *menu = Global::bnpView->popupMenu ( "insert_popup" );
			setInsertPopupMenu();
			connect ( menu, SIGNAL ( aboutToHide() ),  this, SLOT ( delayedCancelInsertPopupMenu() ) );
			connect ( menu, SIGNAL ( aboutToHide() ),  this, SLOT ( unlockHovering() ) );
			connect ( menu, SIGNAL ( aboutToHide() ),  this, SLOT ( disableNextClick() ) );
			removeInserter();
			m_lockedHovering = true;
			PopupMenu::execAtRectCenter ( *menu, basketRect ); // Popup at center or the basket
		}
		else
		{
			if ( ! m_focusedNote->isSelected() )
				unselectAllBut ( m_focusedNote );
			setFocusedNote ( m_focusedNote ); /// /// ///
			m_startOfShiftSelectionNote = ( m_focusedNote->isGroup() ? m_focusedNote->firstRealChild() : m_focusedNote );
			// Popup at bottom (or top) of the focused note, if visible :
			QMenu *menu = Global::bnpView->popupMenu ( "note_popup" );
			connect ( menu, SIGNAL ( aboutToHide() ),  this, SLOT ( unlockHovering() ) );
			connect ( menu, SIGNAL ( aboutToHide() ),  this, SLOT ( disableNextClick() ) );
			doHoverEffects ( m_focusedNote, Note::Content ); // In the case where another popup menu was open, we should do that manually!
			m_lockedHovering = true;
			PopupMenu::execAtRectBottom ( *menu, noteVisibleRect ( m_focusedNote ), true );
		}
	}
}

QRect Basket::noteVisibleRect ( Note *note )
{
//FIXME 1.5	QRect rect ( contentsToViewport ( QPoint ( note->x(), note->y() ) ), QSize ( note->width(),note->height() ) );
QRect rect;
	QPoint basketPoint = mapToGlobal ( QPoint ( 0,0 ) );
	rect.moveTopLeft ( rect.topLeft() + basketPoint + QPoint ( frameWidth(), frameWidth() ) );

	// Now, rect contain the global note rectangle on the screen.
	// We have to clip it by the basket widget :
	if ( rect.bottom() > basketPoint.y() + height() + 1 )
	{ // Bottom too... bottom
		rect.setBottom ( basketPoint.y() + height() + 1 );
		if ( rect.height() <= 0 ) // Have at least one visible pixel of height
			rect.setTop ( rect.bottom() );
	}
	if ( rect.top() < basketPoint.y() + frameWidth() )
	{ // Top too... top
		rect.setTop ( basketPoint.y() + frameWidth() );
		if ( rect.height() <= 0 )
			rect.setBottom ( rect.top() );
	}
	if ( rect.right() > basketPoint.x() + width() + 1 )
	{ // Right too... right
		rect.setRight ( basketPoint.x() + width() + 1 );
		if ( rect.width() <= 0 ) // Have at least one visible pixel of width
			rect.setLeft ( rect.right() );
	}
	if ( rect.left() < basketPoint.x() + frameWidth() )
	{ // Left too... left
		rect.setLeft ( basketPoint.x() + frameWidth() );
		if ( rect.width() <= 0 )
			rect.setRight ( rect.left() );
	}

	return rect;
}

void Basket::disableNextClick()
{
	m_lastDisableClick = QTime::currentTime();
}

void Basket::recomputeAllStyles()
{
	FOR_EACH_NOTE ( note )
	note->recomputeAllStyles();
}

void Basket::removedStates ( const QList<State*> &deletedStates )
{
	bool modifiedBasket = false;

	FOR_EACH_NOTE ( note )
	if ( note->removedStates ( deletedStates ) )
		modifiedBasket = true;

	if ( modifiedBasket )
		save();
}

void Basket::insertNote ( Note *note, Note *clicked, int zone, const QPoint &pos, bool animateNewPosition )
{
	if ( !note )
	{
		std::cout << "Wanted to insert NO note" << std::endl;
		return;
	}

	if ( clicked && zone == Note::BottomColumn )
	{
		// When inserting at the bottom of a column, it's obvious the new note SHOULD inherit tags.
		// We ensure that by changing the insertion point after the last note of the column:
		Note *last = clicked->lastChild();
		if ( last )
		{
			clicked = last;
			zone = Note::BottomInsert;
		}
	}

	/// Insertion at the bottom of a column:
	if ( clicked && zone == Note::BottomColumn )
	{
		note->setWidth ( clicked->rightLimit() - clicked->x() );
		Note *lastChild = clicked->lastChild();
		if ( !animateNewPosition || !Settings::playAnimations() )
			for ( Note *n = note; n; n = n->next() )
			{
				n->setXRecursivly ( clicked->x() );
				n->setYRecursivly ( ( lastChild ? lastChild : clicked )->bottom() + 1 );
			}
		appendNoteIn ( note, clicked );

		/// Insertion relative to a note (top/bottom, insert/group):
	}
	else if ( clicked )
	{
		note->setWidth ( clicked->width() );
		if ( !animateNewPosition || !Settings::playAnimations() )
			for ( Note *n = note; n; n = n->next() )
			{
				if ( zone == Note::TopGroup || zone == Note::BottomGroup )
					n->setXRecursivly ( clicked->x() + Note::GROUP_WIDTH );
				else
					n->setXRecursivly ( clicked->x() );
				if ( zone == Note::TopInsert || zone == Note::TopGroup )
					n->setYRecursivly ( clicked->y() );
				else
					n->setYRecursivly ( clicked->bottom() + 1 );
			}

	if ( zone == Note::TopInsert )    { appendNoteBefore ( note, clicked ); }
		else if ( zone == Note::BottomInsert ) { appendNoteAfter ( note,  clicked ); }
		else if ( zone == Note::TopGroup )     { groupNoteBefore ( note,  clicked ); }
		else if ( zone == Note::BottomGroup )  { groupNoteAfter ( note,   clicked ); }

		/// Free insertion:
	}
	else if ( isFreeLayout() )
	{
		// Group if note have siblings:
		if ( note->next() )
		{
			Note *group = new Note ( this );
			for ( Note *n = note; n; n = n->next() )
				n->setParentNote ( group );
			group->setFirstChild ( note );
			note = group;
		}
		// Insert at cursor position:
		const int initialWidth = 250;
		note->setWidth ( note->isGroup() ? Note::GROUP_WIDTH : initialWidth );
		if ( note->isGroup() && note->firstChild() )
			note->setInitialHeight ( note->firstChild()->height() );
		//note->setGroupWidth(initialWidth);
		if ( animateNewPosition && Settings::playAnimations() )
			note->setFinalPosition ( pos.x(), pos.y() );
		else
		{
			note->setXRecursivly ( pos.x() );
			note->setYRecursivly ( pos.y() );
		}
		appendNoteAfter ( note, lastNote() );
	}

	relayoutNotes ( true );
}

void Basket::clickedToInsert ( QMouseEvent *event, Note *clicked, /*Note::Zone*/int zone )
{
	Note *note;
	if ( event->button() == Qt::MidButton )
		note = NoteFactory::dropNote ( KApplication::clipboard()->mimeData ( QClipboard::Selection ), this );
	else
		note = NoteFactory::createNoteText ( "", this );

	if ( !note )
		return;

	insertNote ( note, clicked, zone, event->pos(), /*animateNewPosition=*/false );

//	ensureNoteVisible(lastInsertedNote()); // TODO: in insertNote()

	if ( event->button() != Qt::MidButton )
	{
		removeInserter(); // Case: user clicked below a column to insert, the note is inserted and doHoverEffects() put a new inserter below. We don't want it.
		closeEditor();
		noteEdit ( note, /*justAdded=*/true );
	}
}

void Basket::contentsDragEnterEvent ( QDragEnterEvent *event )
{
	m_isDuringDrag = true;
	Global::bnpView->updateStatusBarHint();
	if ( NoteDrag::basketOf ( event ) == this )
		m_draggedNotes = NoteDrag::notesOf ( event );
}

void Basket::contentsDragMoveEvent ( QDragMoveEvent *event )
{
//	m_isDuringDrag = true;

//	if (isLocked())
//		return;

//	FIXME: viewportToContents does NOT work !!!
//	QPoint pos = viewportToContents(event->pos());
//	QPoint pos( event->pos().x() + contentsX(), event->pos().y() + contentsY() );

//	if (insertAtCursorPos())
//		computeInsertPlace(pos);
	doHoverEffects ( event->pos() );

//	showFrameInsertTo();
	if ( isFreeLayout() || noteAt ( event->pos().x(), event->pos().y() ) ) // Cursor before rightLimit() or hovering the dragged source notes
		acceptDropEvent ( event );
	else
	{
		event->ignore ( );
	}

	/*	Note *hoveredNote = noteAt(event->pos().x(), event->pos().y());
		if ( (isColumnsLayout() && !hoveredNote) || (draggedNotes().contains(hoveredNote)) ) {
			event->acceptAction(false);
			event->accept(false);
		} else
			acceptDropEvent(event);*/

	// A workarround since QScrollView::dragAutoScroll seem to have no effect :
//	ensureVisible(event->pos().x() + contentsX(), event->pos().y() + contentsY(), 30, 30);
//	QScrollView::dragMoveEvent(event);
}

void Basket::contentsDragLeaveEvent ( QDragLeaveEvent* )
{
//	resetInsertTo();
	m_isDuringDrag = false;
	m_draggedNotes.clear();
	m_noActionOnMouseRelease = true;
	emit resetStatusBarText();
	doHoverEffects();
}

void Basket::contentsDropEvent ( QDropEvent *event )
{
	QPoint pos = event->pos();
	std::cout << "Contents Drop Event at position " << pos.x() << ":" << pos.y() << std::endl;

	m_isDuringDrag = false;
	emit resetStatusBarText();

//	if (isLocked())
//		return;

	// Do NOT check the bottom&right borders.
	// Because imagine someone drag&drop a big note from the top to the bottom of a big basket (with big vertical scrollbars),
	// the note is first removed, and relayoutNotes() compute the new height that is smaller
	// Then noteAt() is called for the mouse pointer position, because the basket is now smaller, the cursor is out of boundaries!!!
	// Should, of course, not return 0:
	Note *clicked = noteAt ( event->pos().x(), event->pos().y() );

	if ( NoteFactory::movingNotesInTheSameBasket ( event, this, event->dropAction() ) && event->dropAction() == Qt::MoveAction )
	{
		m_doNotCloseEditor = true;
	}

	Note *note = NoteFactory::dropNote ( event->mimeData(), this, true, event->dropAction(), dynamic_cast<Note*> ( event->source() ) );

	if ( note )
	{
		Note::Zone zone = ( clicked ? clicked->zoneAt ( event->pos() - QPoint ( clicked->x(), clicked->y() ), /*toAdd=*/true ) : Note::None );
		bool animateNewPosition = NoteFactory::movingNotesInTheSameBasket ( event, this, event->dropAction() );
		if ( animateNewPosition )
		{
			FOR_EACH_NOTE ( n )
			n->setOnTop ( false );
			// FOR_EACH_NOTE_IN_CHUNK(note)
			for ( Note *n = note; n; n = n->next() )
				n->setOnTop ( true );
		}

		insertNote ( note, clicked, zone, event->pos(), animateNewPosition );

		// If moved a note on bottom, contentsHeight has been disminished, then view scrolled up, and we should re-scroll the view down:
		ensureNoteVisible ( note );

//		if (event->button() != Qt::MidButton) {
//			removeInserter(); // Case: user clicked below a column to insert, the note is inserted and doHoverEffects() put a new inserter below. We don't want it.
//		}

//		resetInsertTo();
//		doHoverEffects(); called by insertNote()
		save();
	}

	m_draggedNotes.clear();

	m_doNotCloseEditor = false;
	// When starting the drag, we saved where we were editing.
	// This is because during a drag, the mouse can fly over the text edit and move the cursor position, and even HIDE the cursor.
	// So we re-show the cursor, and re-position it at the right place:
	if ( m_editor && m_editor->textEdit() )
	{
		QTextEdit *editor = m_editor->textEdit();
//FIXME 1.5		editor->setCursorPosition ( m_editParagraph, m_editIndex );
	}
}

// handles dropping of a note to basket that is not shown
// (usually through its entry in the basket list)
void Basket::blindDrop ( QDropEvent* event )
{
	if ( !m_isInsertPopupMenu && redirectEditActions() )
	{
		if ( m_editor->textEdit() )
			m_editor->textEdit()->paste();
		else if ( m_editor->lineEdit() )
			m_editor->lineEdit()->paste();
	}
	else
	{
		if ( !isLoaded() )
		{
			Global::bnpView->showPassiveLoading ( this );
			load();
		}
		closeEditor();
		unselectAll();
		Note *note = NoteFactory::dropNote ( event->mimeData(), this, true, event->dropAction(),
		                                     dynamic_cast<Note*> ( event->source() ) );
		if ( note )
		{
			insertCreatedNote ( note );
			//unselectAllBut(note);
			if ( Settings::usePassivePopup() )
				Global::bnpView->showPassiveDropped ( i18n ( "Dropped to basket <i>%1</i>" ) );
		}
	}
	save();
}

void Basket::insertEmptyNote ( int type )
{
	if ( !isLoaded() )
		load();
	if ( isDuringEdit() )
		closeEditor();
	Note *note = NoteFactory::createEmptyNote ( ( NoteType::Id ) type, this );
	insertCreatedNote ( note/*, / *edit=* /true*/ );
	noteEdit ( note, /*justAdded=*/true );
}

void Basket::insertWizard ( int type )
{
	saveInsertionData();
	Note *note = 0;
	switch ( type )
	{
		default:
		case 1: note = NoteFactory::importKMenuLauncher ( this ); break;
		case 2: note = NoteFactory::importIcon ( this );          break;
		case 3: note = NoteFactory::importFileContent ( this );   break;
	}
	if ( !note )
		return;
	restoreInsertionData();
	insertCreatedNote ( note );
	unselectAllBut ( note );
	resetInsertionData();
}

void Basket::insertColor ( const QColor &color )
{
	Note *note = NoteFactory::createNoteColor ( color, this );
	restoreInsertionData();
	insertCreatedNote ( note );
	unselectAllBut ( note );
	resetInsertionData();
}

void Basket::insertImage ( const QPixmap &image )
{
	Note *note = NoteFactory::createNoteImage ( image, this );
	restoreInsertionData();
	insertCreatedNote ( note );
	unselectAllBut ( note );
	resetInsertionData();
}

void Basket::pasteNote ( QClipboard::Mode mode )
{
	if ( !m_isInsertPopupMenu && redirectEditActions() )
	{
		if ( m_editor->textEdit() )
			m_editor->textEdit()->paste();
		else if ( m_editor->lineEdit() )
			m_editor->lineEdit()->paste();
	}
	else
	{
		if ( !isLoaded() )
		{
			Global::bnpView->showPassiveLoading ( this );
			load();
		}
		closeEditor();
		unselectAll();
		Note *note = NoteFactory::dropNote ( KApplication::clipboard()->mimeData ( mode ), this );
		if ( note )
		{
			insertCreatedNote ( note );
			//unselectAllBut(note);
		}
	}
}

void Basket::insertCreatedNote ( Note *note )
{
	// Get the insertion data if the user clicked inside the basket:
	Note *clicked = m_clickedToInsert;
	int zone      = m_zoneToInsert;
	QPoint pos    = m_posToInsert;

	// If it isn't the case, use the default position:
	if ( !clicked && ( pos.x() < 0 || pos.y() < 0 ) )
	{
		// Insert right after the focused note:
		focusANote();
		if ( m_focusedNote )
		{
			clicked = m_focusedNote;
			zone    = ( m_focusedNote->isFree() ? Note::BottomGroup : Note::BottomInsert );
			pos     = QPoint ( m_focusedNote->x(), m_focusedNote->finalBottom() );
			// Insert at the end of the last column:
		}
		else if ( isColumnsLayout() )
		{
			Note *column = /*(Settings::newNotesPlace == 0 ?*/ firstNote() /*: lastNote())*/;
			/*if (Settings::newNotesPlace == 0 && column->firstChild()) { // On Top, if at least one child in the column
				clicked = column->firstChild();
				zone    = Note::TopInsert;
			} else { // On Bottom*/
			clicked = column;
			zone    = Note::BottomColumn;
			/*}*/
			// Insert at free position:
		}
		else
		{
			pos = QPoint ( 0, 0 );
		}
	}

	insertNote ( note, clicked, zone, pos );
//	ensureNoteVisible(lastInsertedNote());
	removeInserter(); // Case: user clicked below a column to insert, the note is inserted and doHoverEffects() put a new inserter below. We don't want it.
//	resetInsertTo();
	save();
}

void Basket::saveInsertionData()
{
	m_savedClickedToInsert = m_clickedToInsert;
	m_savedZoneToInsert    = m_zoneToInsert;
	m_savedPosToInsert     = m_posToInsert;
}

void Basket::restoreInsertionData()
{
	m_clickedToInsert = m_savedClickedToInsert;
	m_zoneToInsert    = m_savedZoneToInsert;
	m_posToInsert     = m_savedPosToInsert;
}

void Basket::resetInsertionData()
{
	m_clickedToInsert = 0;
	m_zoneToInsert    = 0;
	m_posToInsert     = QPoint ( -1, -1 );
}

void Basket::hideInsertPopupMenu()
{
	QTimer::singleShot ( 50/*ms*/, this, SLOT ( timeoutHideInsertPopupMenu() ) );
}

void Basket::timeoutHideInsertPopupMenu()
{
	resetInsertionData();
}

void Basket::acceptDropEvent ( QDropEvent *event, bool preCond )
{
	// FIXME: Should not accept all actions! Or not all actions (link not supported?!)
	event->setAccepted(preCond);
}

void Basket::contentsMouseReleaseEvent ( QMouseEvent *event )
{
	// Now disallow drag:
	m_canDrag = false;

	// Cancel Resizer move:
	if ( m_resizingNote )
	{
		m_resizingNote  = 0;
		m_pickedResizer = 0;
		m_lockedHovering = false;
		doHoverEffects();
		save();
	}

	// Cancel Note move:
	/*	if (m_movingNote) {
			m_movingNote   = 0;
			m_pickedHandle = QPoint(0, 0);
			m_lockedHovering = false;
			//doHoverEffects();
			save();
		}
	*/

	// Cancel Selection rectangle:
	if ( m_isSelecting )
	{
		m_isSelecting = false;
		stopAutoScrollSelection();
		resetWasInLastSelectionRect();
		doHoverEffects();
//FIXME 1.5		updateContents ( m_selectionRect );
	}
	m_selectionStarted = false;

	Note *clicked = noteAt ( event->pos().x(), event->pos().y() );
	Note::Zone zone = ( clicked ? clicked->zoneAt ( event->pos() - QPoint ( clicked->x(), clicked->y() ) ) : Note::None );
	if ( ( zone == Note::Handle || zone == Note::Group ) && editedNote() && editedNote() == clicked )
	{
		if ( m_ignoreCloseEditorOnNextMouseRelease )
			m_ignoreCloseEditorOnNextMouseRelease = false;
		else
		{
			bool editedNoteStillThere = closeEditor();
			if ( editedNoteStillThere )
				//clicked->setSelected(true);
				unselectAllBut ( clicked );
		}
	}


	if ( event->buttons() == 0 && ( zone == Note::Group || zone == Note::Handle ) )
	{
		closeEditor();
		unselectAllBut ( clicked );
	}


	// Do nothing if an action has already been made during mousePressEvent,
	// or if user made a selection and canceled it by regressing to a very small rectangle.
	if ( m_noActionOnMouseRelease )
		return;
	// We immediatly set it to true, to avoid actions set on mouseRelease if NO mousePress event has been triggered.
	// This is the case when a popup menu is shown, and user click to the basket area to close it:
	// the menu then receive the mousePress event and the basket area ONLY receive the mouseRelease event.
	// Obviously, nothing should be done in this case:
	m_noActionOnMouseRelease = true;



	if ( event->button() == Qt::MidButton && zone != Note::Resizer && ( !isDuringEdit() || clicked != editedNote() ) )
	{
		if ( ( Settings::middleAction() != 0 ) && ( event->buttons() == Qt::ShiftModifier ) )
		{
			m_clickedToInsert = clicked;
			m_zoneToInsert    = zone;
			m_posToInsert     = event->pos();
			closeEditor();
			removeInserter();                     // If clicked at an insertion line and the new note shows a dialog for editing,
			NoteType::Id type = ( NoteType::Id ) 0;  //  hide that inserter before the note edition instead of after the dialog is closed
			switch ( Settings::middleAction() )
			{
				case 5: type = NoteType::Color;    break;
				case 6:
					Global::bnpView->grabScreenshot();
					return;
				case 7:
					Global::bnpView->slotColorFromScreen();
					return;
				case 8:
					Global::bnpView->insertWizard ( 3 ); // loadFromFile
					return;
				case 9:
					Global::bnpView->insertWizard ( 1 ); // importKMenuLauncher
					return;
				case 10:
					Global::bnpView->insertWizard ( 2 ); // importIcon
					return;
			}
			if ( type != 0 )
			{
				m_ignoreCloseEditorOnNextMouseRelease = true;
				Global::bnpView->insertEmpty ( type );
				return;
			}
		}
	}

//	Note *clicked = noteAt(event->pos().x(), event->pos().y());
	if ( ! clicked )
	{
		if ( isFreeLayout() && event->button() == Qt::LeftButton )
		{
			clickedToInsert ( event );
			save();
		}
		return;
	}
//	Note::Zone zone = clicked->zoneAt( event->pos() - QPoint(clicked->x(), clicked->y()) );

	// Convenient variables:
	bool controlPressed = event->buttons() & Qt::ControlModifier;
	bool shiftPressed   = event->buttons() & Qt::ShiftModifier;

	if ( clicked && zone != Note::None && zone != Note::BottomColumn && zone != Note::Resizer && ( controlPressed || shiftPressed ) )
	{
		if ( controlPressed && shiftPressed )
			selectRange ( m_startOfShiftSelectionNote, clicked, /*unselectOthers=*/false );
		else if ( shiftPressed )
			selectRange ( m_startOfShiftSelectionNote, clicked );
		else if ( controlPressed )
			clicked->setSelectedRecursivly ( !clicked->allSelected() );
		setFocusedNote ( clicked ); /// /// ///
		m_startOfShiftSelectionNote = ( clicked->isGroup() ? clicked->firstRealChild() : clicked );
		m_noActionOnMouseRelease = true;
		return;
	}

	// Switch tag states:
	if ( zone >= Note::Emblem0 )
	{
		if ( event->button() == Qt::LeftButton )
		{
			int icons = -1;
			for ( State::List::iterator it = clicked->states().begin(); it != clicked->states().end(); ++it )
			{
				if ( ! ( *it )->emblem().isEmpty() )
					icons++;
				if ( icons == zone - Note::Emblem0 )
				{
					State *state = ( *it )->nextState();
					if ( !state )
						return;
					it = clicked->states().insert ( it, state );
					++it;
					clicked->states().erase ( it );
					clicked->recomputeStyle();
					clicked->unbufferize();
					updateNote ( clicked );
					updateEditorAppearance();
					filterAgain();
					save();
					break;
				}
			}
			return;
		}/* else if (event->button() == Qt::RightButton) {
				popupEmblemMenu(clicked, zone - Note::Emblem0);
				return;
			}*/
	}

	// Insert note or past clipboard:
	QString  text;
//	Note *note;
	QString  link;
	//int zone = zone;
	if ( event->button() == Qt::MidButton && zone == Note::Resizer )
		return; //zone = clicked->zoneAt( event->pos() - QPoint(clicked->x(), clicked->y()), true );
	if ( event->button() == Qt::RightButton && ( clicked->isColumn() || zone == Note::Resizer ) )
		return;
	if ( clicked->isGroup() && zone == Note::None )
		return;
	switch ( zone )
	{
		case Note::Handle:
		case Note::Group:
			// We select note on mousePress if it was unselected or Ctrl is pressed.
			// But the user can want to drag select_s_ notes, so it the note is selected, we only select it alone on mouseRelease:
			if ( event->buttons() == 0 )
			{
				std::cout << "EXEC" << std::endl;
				if ( ! ( event->buttons() & Qt::ControlModifier ) && clicked->allSelected() )
					unselectAllBut ( clicked );
				if ( zone == Note::Handle && isDuringEdit() && editedNote() == clicked )
				{
					closeEditor();
					clicked->setSelected ( true );
				}
			}
			break;

		case Note::Custom0:
			//unselectAllBut(clicked);
			setFocusedNote ( clicked );
			noteOpen ( clicked );
			break;

		case Note::GroupExpander:
		case Note::TagsArrow:
			break;

		case Note::Link:
			link = clicked->linkAt ( event->pos() - QPoint ( clicked->x(), clicked->y() ) );
			if ( ! link.isEmpty() )
			{
				if ( link == "basket-internal-remove-basket" )
				{
					// TODO: ask confirmation: "Do you really want to delete the welcome baskets?\n You can re-add them at any time in the Help menu."
					Global::bnpView->doBasketDeletion ( this );
				}
				else if ( link == "basket-internal-import" )
				{
					QMenu *menu = Global::bnpView->popupMenu ( "file-import" );
					menu->exec ( event->globalPos() );
				}
				else
				{
					KRun *run = new KRun ( KUrl(link),this); //  open the URL.
					run->setAutoDelete ( true );
				}
				break;
			} // If there is no link, edit note content
		case Note::Content:
			closeEditor();
			unselectAllBut ( clicked );
			noteEdit ( clicked, /*justAdded=*/false, event->pos() );
			break;

		case Note::TopInsert:
		case Note::TopGroup:
		case Note::BottomInsert:
		case Note::BottomGroup:
		case Note::BottomColumn:
			clickedToInsert ( event, clicked, zone );
			save();
			break;

		case Note::None:
		default:
			KMessageBox::information ( viewport(),
			                           i18n ( "This message should never appear. If it does, this program is buggy! "
			                                  "Please report the bug to the developer." ) );
			break;
	}
}

void Basket::contentsMouseDoubleClickEvent ( QMouseEvent *event )
{
	Note *clicked = noteAt ( event->pos().x(), event->pos().y() );
	Note::Zone zone = ( clicked ? clicked->zoneAt ( event->pos() - QPoint ( clicked->x(), clicked->y() ) ) : Note::None );

	if ( event->button() == Qt::LeftButton && ( zone == Note::Group || zone == Note::Handle ) )
	{
		doCopy ( CopyToSelection );
		m_noActionOnMouseRelease = true;
	}
	else
		contentsMousePressEvent ( event );
}

void Basket::contentsMouseMoveEvent ( QMouseEvent *event )
{
	// Drag the notes:
	if ( m_canDrag && ( m_pressPos - event->pos() ).manhattanLength() > KApplication::startDragDistance() )
	{
		m_canDrag          = false;
		m_isSelecting      = false; // Don't draw selection rectangle ater drag!
		m_selectionStarted = false;
		NoteSelection *selection = selectedNotes();
		if ( selection->firstStacked() )
		{
			QMimeData *d = NoteDrag::dragObject ( selection, /*cutting=*/false, /*source=*/this ); // d will be deleted by QT
			/*bool shouldRemove = */
//FIXME 1.5			d->drag();
//		delete selection;

			// Never delete because URL is dragged and the file must be available for the extern appliation
//		if (shouldRemove && d->target() == 0) // If target is another application that request to remove the note
//			emit wantDelete(this);
		}
		return;
	}

	// Moving a Resizer:
	if ( m_resizingNote )
	{
		int groupWidth = event->pos().x() - m_resizingNote->x() - m_pickedResizer;
		int minRight   = m_resizingNote->minRight();
		int maxRight   = 100 * widget()->width(); // A big enough value (+infinity) for free layouts.
		Note *nextColumn = m_resizingNote->next();
		if ( m_resizingNote->isColumn() )
		{
			if ( nextColumn )
				maxRight = nextColumn->x() + nextColumn->rightLimit() - nextColumn->minRight() - Note::RESIZER_WIDTH;
			else
				maxRight = widget()->width();
		}
		if ( groupWidth > maxRight - m_resizingNote->x() )
			groupWidth = maxRight - m_resizingNote->x();
		if ( groupWidth < minRight - m_resizingNote->x() )
			groupWidth = minRight - m_resizingNote->x();
		int delta = groupWidth - m_resizingNote->groupWidth();
		m_resizingNote->setGroupWidth ( groupWidth );
		// If resizing columns:
		if ( m_resizingNote->isColumn() )
		{
			Note *column = m_resizingNote;
			if ( ( column = column->next() ) )
			{
				// Next columns should not have them X coordinate animated, because it would flicker:
				column->setXRecursivly ( column->x() + delta );
				// And the resizer should resize the TWO sibling columns, and not push the other columns on th right:
				column->setGroupWidth ( column->groupWidth() - delta );
			}
		}
		relayoutNotes ( true );
	}

	// Moving a Note:
	/*	if (m_movingNote) {
			int x = event->pos().x() - m_pickedHandle.x();
			int y = event->pos().y() - m_pickedHandle.y();
			if (x < 0) x = 0;
			if (y < 0) y = 0;
			m_movingNote->setX(x);
			m_movingNote->setY(y);
			m_movingNote->relayoutAt(x, y, / *animate=* /false);
			relayoutNotes(true);
		}
	*/

	// Dragging the selection rectangle:
	if ( m_selectionStarted )
		doAutoScrollSelection();

	doHoverEffects ( event->pos() );
}

void Basket::doAutoScrollSelection()
{
	static const int AUTO_SCROLL_MARGIN = 50;  // pixels
	static const int AUTO_SCROLL_DELAY  = 100; // milliseconds

	QPoint pos = widget()->mapFromGlobal ( QCursor::pos() );

	// Do the selection:

/*FIXME 1.5	if ( m_isSelecting )
		updateContents ( m_selectionRect );
*/
	m_selectionEndPoint =  pos ;
	m_selectionRect = QRect ( m_selectionBeginPoint, m_selectionEndPoint ).normalized();
	if ( m_selectionRect.left() < 0 )                    m_selectionRect.setLeft ( 0 );
	if ( m_selectionRect.top() < 0 )                     m_selectionRect.setTop ( 0 );
	if ( m_selectionRect.right() >= widget()->width() )    m_selectionRect.setRight ( widget()->width() - 1 );
	if ( m_selectionRect.bottom() >= widget()->height() )  m_selectionRect.setBottom ( widget()->height() - 1 );

	if ( ( m_selectionBeginPoint - m_selectionEndPoint ).manhattanLength() > QApplication::startDragDistance() )
	{
		m_isSelecting = true;
		selectNotesIn ( m_selectionRect, m_selectionInvert );
//FIXME 1.5		updateContents ( m_selectionRect );
		m_noActionOnMouseRelease = true;
	}
	else
	{
		// If the user was selecting but cancel by making the rectangle too small, cancel it really!!!
		if ( m_isSelecting )
		{
			if ( m_selectionInvert )
				selectNotesIn ( QRect(), m_selectionInvert );
			else
				unselectAllBut ( 0 ); // TODO: unselectAll();
		}
		if ( m_isSelecting )
			resetWasInLastSelectionRect();
		m_isSelecting = false;
		stopAutoScrollSelection();
		return;
	}

	// Do the auto-scrolling:
	// FIXME: It's still flickering

	QRect insideRect ( AUTO_SCROLL_MARGIN, AUTO_SCROLL_MARGIN, width() - 2*AUTO_SCROLL_MARGIN, height() - 2*AUTO_SCROLL_MARGIN );

	int dx = 0;
	int dy = 0;

	if ( pos.y() < AUTO_SCROLL_MARGIN )
		dy = pos.y() - AUTO_SCROLL_MARGIN;
	else if ( pos.y() > height() - AUTO_SCROLL_MARGIN )
		dy = pos.y() - height() + AUTO_SCROLL_MARGIN;

	if ( pos.x() < AUTO_SCROLL_MARGIN )
		dx = pos.x() - AUTO_SCROLL_MARGIN;
	else if ( pos.x() > width() - AUTO_SCROLL_MARGIN )
		dx = pos.x() - width() + AUTO_SCROLL_MARGIN;

	if ( dx || dy )
	{
		kapp->sendPostedEvents(); // Do the repaints, because the scrolling will make the area to repaint to be wrong
		scrollContentsBy ( dx, dy );
		if ( !m_autoScrollSelectionTimer.isActive() )
			m_autoScrollSelectionTimer.start ( AUTO_SCROLL_DELAY );
	}
	else
		stopAutoScrollSelection();
}

void Basket::stopAutoScrollSelection()
{
	m_autoScrollSelectionTimer.stop();
}

void Basket::resetWasInLastSelectionRect()
{
	Note *note = m_firstNote;
	while ( note )
	{
		note->resetWasInLastSelectionRect();
		note = note->next();
	}
}

void Basket::selectAll()
{
	if ( redirectEditActions() )
	{
		if ( m_editor->textEdit() )
			m_editor->textEdit()->selectAll();
		else if ( m_editor->lineEdit() )
			m_editor->lineEdit()->selectAll();
	}
	else
	{
		// First select all in the group, then in the parent group...
		Note *child  = m_focusedNote;
		Note *parent = ( m_focusedNote ? m_focusedNote->parentNote() : 0 );
		while ( parent )
		{
			if ( !parent->allSelected() )
			{
				parent->setSelectedRecursivly ( true );
				return;
			}
			child  = parent;
			parent = parent->parentNote();
		}
		// Then, select all:
		FOR_EACH_NOTE ( note )
		note->setSelectedRecursivly ( true );
	}
}

void Basket::unselectAll()
{
	if ( redirectEditActions() )
	{
		if ( m_editor->textEdit() )
		{
			m_editor->textEdit()->textCursor().clearSelection ();
			selectionChangedInEditor(); // THIS IS NOT EMITED BY Qt!!!
		}
		else if ( m_editor->lineEdit() )
			m_editor->lineEdit()->deselect();
	}
	else
	{
		if ( countSelecteds() > 0 ) // Optimisation
			FOR_EACH_NOTE ( note )
			note->setSelectedRecursivly ( false );
	}
}

void Basket::invertSelection()
{
	FOR_EACH_NOTE ( note )
	note->invertSelectionRecursivly();
}

void Basket::unselectAllBut ( Note *toSelect )
{
	FOR_EACH_NOTE ( note )
	note->unselectAllBut ( toSelect );
}

void Basket::invertSelectionOf ( Note *toSelect )
{
	FOR_EACH_NOTE ( note )
	note->invertSelectionOf ( toSelect );
}

void Basket::selectNotesIn ( const QRect &rect, bool invertSelection, bool unselectOthers /*= true*/ )
{
	FOR_EACH_NOTE ( note )
	note->selectIn ( rect, invertSelection, unselectOthers );
}

void Basket::doHoverEffects()
{
	doHoverEffects ( widget()->mapFromGlobal ( QCursor::pos()  ) );
}

void Basket::doHoverEffects ( Note *note, Note::Zone zone, const QPoint &pos )
{
	// Inform the old and new hovered note (if any):
	Note *oldHoveredNote = m_hoveredNote;
	if ( note != m_hoveredNote )
	{
		if ( m_hoveredNote )
		{
			m_hoveredNote->setHovered ( false );
			m_hoveredNote->setHoveredZone ( Note::None );
			updateNote ( m_hoveredNote );
		}
		m_hoveredNote = note;
		if ( note )
			note->setHovered ( true );
	}

	// If we are hovering a note, compute which zone is hovered and inform the note:
	if ( m_hoveredNote )
	{
		if ( zone != m_hoveredZone || oldHoveredNote != m_hoveredNote )
		{
			m_hoveredZone = zone;
			m_hoveredNote->setCursor ( zone );
			updateNote ( m_hoveredNote );
		}
		m_hoveredNote->setHoveredZone ( zone );
		// If we are hovering an insert line zone, update this thing:
		if ( zone == Note::TopInsert || zone == Note::TopGroup || zone == Note::BottomInsert || zone == Note::BottomGroup || zone == Note::BottomColumn )
			placeInserter ( m_hoveredNote, zone );
		else
			removeInserter();
		// If we are hovering an embedded link in a rich text element, show the destination in the statusbar:
		if ( zone == Note::Link )
			emit setStatusBarText ( m_hoveredNote->linkAt ( pos - QPoint ( m_hoveredNote->x(), m_hoveredNote->y() ) ) );
		else if ( m_hoveredNote->content() )
			emit setStatusBarText ( m_hoveredNote->content()->statusBarMessage ( m_hoveredZone ) );//resetStatusBarText();
		// If we aren't hovering a note, reset all:
	}
	else
	{
		if ( isFreeLayout() && !isSelecting() )
			viewport()->setCursor ( Qt::CrossCursor );
		else
			viewport()->unsetCursor();
		m_hoveredZone = Note::None;
		removeInserter();
		emit resetStatusBarText();
	}
}

void Basket::doHoverEffects ( const QPoint &pos )
{
//	if (isDuringEdit())
//		viewport()->unsetCursor();

	// Do we have the right to do hover effects?
	if ( ! m_loaded || m_lockedHovering )
		return;

	// enterEvent() (mouse enter in the widget) set m_underMouse to true, and leaveEvent() make it false.
	// But some times the enterEvent() is not trigerred: eg. when dragging the scrollbar:
	// Ending the drag INSIDE the basket area will make NO hoverEffects() because m_underMouse is false.
	// User need to leave the area and re-enter it to get effects.
	// This hack solve that by dismissing the m_underMouse variable:
	bool underMouse = Global::bnpView->currentBasket() == this && QRect ( widget()->pos().x(), widget()->pos().y(), width(), height() ).contains ( pos );

	// Don't do hover effects when a popup menu is opened.
	// Primarily because the basket area will only receive mouseEnterEvent and mouveLeaveEvent.
	// It willn't be noticed of mouseMoveEvent, which would result in a apparently broken application state:
	if ( kapp->activePopupWidget() )
		underMouse = false;

	// Compute which note is hovered:
	Note       *note = ( m_isSelecting || !underMouse ? 0 : noteAt ( pos.x(), pos.y() ) );
	Note::Zone  zone = ( note ? note->zoneAt ( pos - QPoint ( note->x(), note->y() ), isDuringDrag() ) : Note::None );

	// Inform the old and new hovered note (if any) and update the areas:
	doHoverEffects ( note, zone, pos );
}

void Basket::mouseEnteredEditorWidget()
{
	if ( !m_lockedHovering && !kapp->activePopupWidget() )
		doHoverEffects ( editedNote(), Note::Content, QPoint() );
}

void Basket::removeInserter()
{
	if ( m_inserterShown )
	{ // Do not hide (and then update/repaint the view) if it is already hidden!
		m_inserterShown = false;
//FIXME 1.5		updateContents ( m_inserterRect );
	}
}

void Basket::placeInserter ( Note *note, int zone )
{
	// Remove the inserter:
	if ( !note )
	{
		removeInserter();
		return;
	}

	// Update the old position:
/*FIXME 1.5	if ( inserterShown() )
		updateContents ( m_inserterRect );
*/	// Some comodities:
	m_inserterShown = true;
	m_inserterTop   = ( zone == Note::TopGroup || zone == Note::TopInsert );
	m_inserterGroup = ( zone == Note::TopGroup || zone == Note::BottomGroup );
	// X and width:
	int groupIndent = ( note->isGroup() ? note->width() : Note::HANDLE_WIDTH );
	int x     = note->x();
	int width = ( note->isGroup() ? note->rightLimit() - note->x() : note->width() );
	if ( m_inserterGroup )
	{
		x     += groupIndent;
		width -= groupIndent;
	}
	m_inserterSplit = ( Settings::groupOnInsertionLine() && note && !note->isGroup() && !note->isFree() && !note->isColumn() );
//	if (note->isGroup())
//		width = note->rightLimit() - note->x() - (m_inserterGroup ? groupIndent : 0);
	// Y:
	int y = note->y() - ( m_inserterGroup && m_inserterTop ? 1 : 3 );
	if ( !m_inserterTop )
		y += ( note->isColumn() ? note->finalHeight() : note->height() );
	// Assigning result:
	m_inserterRect = QRect ( x, y, width, 6 - ( m_inserterGroup ? 2 : 0 ) );
	// Update the new position:
//FIXME 1.5	updateContents ( m_inserterRect );
}

inline void drawLineByRect ( QPainter &painter, int x, int y, int width, int height )
{
	painter.drawLine ( x, y, x + width - 1, y + height - 1 );
}

void Basket::drawInserter ( QPainter &painter, int xPainter, int yPainter )
{
	if ( !m_inserterShown )
		return;

	QRect rect = m_inserterRect; // For shorter code-lines when drawing!
	rect.translate ( -xPainter, -yPainter );
	int lineY  = ( m_inserterGroup && m_inserterTop ? 0 : 2 );
	int roundY = ( m_inserterGroup && m_inserterTop ? 0 : 1 );

	QColor dark  = KColorScheme(QPalette::Active, KColorScheme::View).background().color().darker();
	QColor light = dark.light().light();
	if ( m_inserterGroup && Settings::groupOnInsertionLine() )
		light = Tools::mixColor ( light, KColorScheme(QPalette::Active, KColorScheme::Selection).background().color() );
	painter.setPen ( dark );
	// The horizontal line:
	//painter.drawRect(       rect.x(),                    rect.y() + lineY,  rect.width(), 2);
	int width = rect.width() - 4;
	drawGradient ( &painter, dark,  light, rect.x() + 2,           rect.y() + lineY, width/2,         2, /*sunken=*/false, /*horz=*/false, /*flat=*/false );
	drawGradient ( &painter, light, dark,  rect.x() + 2 + width/2, rect.y() + lineY, width - width/2, 2, /*sunken=*/false, /*horz=*/false, /*flat=*/false );
	// The left-most and right-most edges (biggest vertical lines):
	drawLineByRect ( painter, rect.x(),                    rect.y(),          1, ( m_inserterGroup ? 4 : 6 ) );
	drawLineByRect ( painter, rect.x() + rect.width() - 1, rect.y(),          1, ( m_inserterGroup ? 4 : 6 ) );
	// The left and right mid vertical lines:
	drawLineByRect ( painter, rect.x() + 1,                rect.y() + roundY, 1, ( m_inserterGroup ? 3 : 4 ) );
	drawLineByRect ( painter, rect.x() + rect.width() - 2, rect.y() + roundY, 1, ( m_inserterGroup ? 3 : 4 ) );
	// Draw the split as a feedback to know where is the limit between insert and group:
	if ( m_inserterSplit )
	{
		int noteWidth = rect.width() + ( m_inserterGroup ? Note::HANDLE_WIDTH : 0 );
		int xSplit = rect.x() - ( m_inserterGroup ? Note::HANDLE_WIDTH : 0 ) + noteWidth / 2;
		painter.setPen ( Tools::mixColor ( dark, light ) );
		painter.drawRect ( xSplit - 2, rect.y() + lineY, 4, 2 );
		painter.setPen ( dark );
		painter.drawRect ( xSplit - 1, rect.y() + lineY, 2, 2 );
	}
}

void Basket::maybeTip ( const QPoint &pos )
{
	if ( !m_loaded || !Settings::showNotesToolTip() )
		return;

	QString message;
	QRect   rect;

	QPoint contentPos = widget()->mapFromGlobal ( pos );
	Note *note = noteAt ( contentPos.x(), contentPos.y() );

	if ( !note && isFreeLayout() )
	{
		message = i18n ( "Insert note here\nRight click for more options" );
		QRect itRect;
		for ( QList<QRect>::iterator it = m_blankAreas.begin(); it != m_blankAreas.end(); ++it )
		{
			itRect = QRect ( 0, 0, width(), height() ).intersect ( *it );
			if ( itRect.contains ( contentPos ) )
			{
				rect = itRect;
				rect.moveLeft ( rect.left() - widget()->pos().x() );
				rect.moveTop ( rect.top()  - widget()->pos().y() );
				break;
			}
		}
	}
	else
	{
		if ( !note )
			return;

		Note::Zone zone = note->zoneAt ( contentPos - QPoint ( note->x(), note->y() ) );
		switch ( zone )
		{
			case Note::Resizer:       message = ( note->isColumn() ?
				                                      i18n ( "Resize those columns" ) :
				                                      ( note->isGroup() ?
				                                        i18n ( "Resize this group" ) :
				                                        i18n ( "Resize this note" ) ) );                 break;
			case Note::Handle:        message = i18n ( "Select or move this note" );           break;
			case Note::Group:         message = i18n ( "Select or move this group" );          break;
			case Note::TagsArrow:     message = i18n ( "Assign or remove tags from this note" );
				if ( note->states().count() > 0 )
				{
					message = "<qt><nobr>" + message + "</nobr><br>" + i18n ( "<b>Assigned Tags</b>: %1" );
					QString tagsString = "";
					for ( State::List::iterator it = note->states().begin(); it != note->states().end(); ++it )
					{
						QString tagName = "<nobr>" + Tools::textToHTMLWithoutP ( ( *it )->fullName() ) + "</nobr>";
						if ( tagsString.isEmpty() )
							tagsString = tagName;
						else
							tagsString = i18n ( "%1, %2" ).arg ( tagsString, tagName );
					}
					message = message.arg ( tagsString );
				}
				break;
			case Note::Custom0:       message = note->content()->zoneTip ( zone );             break; //"Open this link/Open this file/Open this sound file/Launch this application"
			case Note::GroupExpander: message = ( note->isFolded() ?
				                                      i18n ( "Expand this group" ) :
				                                      i18n ( "Collapse this group" ) );               break;
			case Note::Link:
			case Note::Content:       message = note->content()->editToolTipText();         break;
			case Note::TopInsert:
			case Note::BottomInsert:  message = i18n ( "Insert note here\nRight click for more options" );              break;
			case Note::TopGroup:      message = i18n ( "Group note with the one below\nRight click for more options" ); break;
			case Note::BottomGroup:   message = i18n ( "Group note with the one above\nRight click for more options" ); break;
			case Note::BottomColumn:  message = i18n ( "Insert note here\nRight click for more options" );              break;
			case Note::None:          message = "** Zone NONE: internal error **";                                   break;
			default:
				if ( zone >= Note::Emblem0 )
					message = note->stateForEmblemNumber ( zone - Note::Emblem0 )->fullName();
				else
					message = "";
				break;
		}

		if ( zone == Note::Content || zone == Note::Link || zone == Note::Custom0 )
		{
			QStringList keys;
			QStringList values;

			note->content()->toolTipInfos ( &keys, &values );
			keys.append ( i18n ( "Added" ) );
			keys.append ( i18n ( "Last Modification" ) );
			values.append ( note->addedStringDate() );
			values.append ( note->lastModificationStringDate() );

			message = "<qt><nobr>" + message;
			QStringList::iterator key;
			QStringList::iterator value;
			for ( key = keys.begin(), value = values.begin(); key != keys.end() && value != values.end(); ++key, ++value )
				message += "<br>" + i18nc ( "of the form 'key: value'", "<b>%1</b>: %2" ).arg ( *key, *value );
			message += "</nobr></qt>";
		}
		else if ( m_inserterSplit && ( zone == Note::TopInsert || zone == Note::BottomInsert ) )
			message += "\n" + i18n ( "Click on the right to group instead of insert" );
		else if ( m_inserterSplit && ( zone == Note::TopGroup || zone == Note::BottomGroup ) )
			message += "\n" + i18n ( "Click on the left to insert instead of group" );

		rect = note->zoneRect ( zone, contentPos - QPoint ( note->x(), note->y() ) );

		rect.moveLeft ( rect.left() - widget()->pos().x() );
		rect.moveTop ( rect.top()  - widget()->pos().y() );

		rect.moveLeft ( rect.left() + note->x() );
		rect.moveTop ( rect.top()  + note->y() );
	}

	setToolTip ( message );
}

Note* Basket::lastNote()
{
	Note *note = firstNote();
	while ( note && note->next() )
		note = note->next();
	return note;
}

void Basket::deleteNotes()
{
	Note *note = m_firstNote;

	while ( note )
	{
		Note *tmp = note->next();
		delete note;
		note = tmp;
	}
	m_firstNote = 0;
	m_resizingNote = 0;
	m_movingNote = 0;
	m_focusedNote = 0;
	m_startOfShiftSelectionNote = 0;
	m_tagPopupNote = 0;
	m_clickedToInsert = 0;
	m_savedClickedToInsert = 0;
	m_hoveredNote = 0;
	m_count = 0;
	m_countFounds = 0;
	m_countSelecteds = 0;

	emit resetStatusBarText();
	emit countsChanged ( this );
}

Note* Basket::noteAt ( int x, int y )
{
//NO:
// 	// Do NOT check the bottom&right borders.
// 	// Because imagine someone drag&drop a big note from the top to the bottom of a big basket (with big vertical scrollbars),
// 	// the note is first removed, and relayoutNotes() compute the new height that is smaller
// 	// Then noteAt() is called for the mouse pointer position, because the basket is now smaller, the cursor is out of boundaries!!!
// 	// Should, of course, not return 0:
	if ( x < 0 || x > widget()->width() || y < 0 || y > widget()->height() )
		return 0;

	// When resizing a note/group, keep it highlighted:
	if ( m_resizingNote )
		return m_resizingNote;

	// Search and return the hovered note:
	Note *note = m_firstNote;
	Note *possibleNote;
	while ( note )
	{
		possibleNote = note->noteAt ( x, y );
		if ( possibleNote )
		{
			if ( draggedNotes().contains ( possibleNote ) )
				return 0;
			else
				return possibleNote;
		}
		note = note->next();
	}

	// If the basket is layouted in columns, return one of the columns to be able to add notes in them:
	if ( isColumnsLayout() )
	{
		Note *column = m_firstNote;
		while ( column )
		{
			if ( x >= column->x() && x < column->rightLimit() )
				return column;
			column = column->next();
		}
	}

	// Nothing found, no note is hovered:
	return NULL;
}

Basket::~Basket()
{
	//delete m_action;
	if ( m_decryptBox )
		delete m_decryptBox;
#ifdef HAVE_LIBGPGME
	delete m_gpg;
#endif
	deleteNotes();
}

void Basket::viewportResizeEvent ( QResizeEvent *event )
{
	relayoutNotes ( true );
	//cornerWidget()->setShown(horizontalScrollBar()->isVisible() && verticalScrollBar()->isVisible());
	if ( horizontalScrollBar()->isVisible() && verticalScrollBar()->isVisible() )
	{
		if ( !cornerWidget() )
			setCornerWidget ( m_cornerWidget );
	}
	else
	{
		if ( cornerWidget() )
			setCornerWidget ( 0 );
	}
//	if (isDuringEdit())
//		ensureNoteVisible(editedNote());
//FIXME 1.5	QScrollView::viewportResizeEvent ( event );
}

void Basket::animateLoad()
{
	const int viewHeight = widget()->pos().y() + widget()->height();

	QTime t = QTime::currentTime(); // Set random seed
	srand ( t.hour() *12 + t.minute() *60 + t.second() *60 );

	Note *note = firstNote();
	while ( note )
	{
		if ( ( note->finalY() < viewHeight ) && note->matching() )
			note->initAnimationLoad();
		note = note->next();
	}

	m_loaded = true;
}

QColor Basket::selectionRectInsideColor()
{
	return Tools::mixColor ( Tools::mixColor ( backgroundColor(), KColorScheme(QPalette::Active, KColorScheme::Selection).background().color() ), backgroundColor() );
}

QColor alphaBlendColors ( const QColor &bgColor, const QColor &fgColor, const int a )
{
	// normal button...
	QRgb rgb = bgColor.rgb();
	QRgb rgb_b = fgColor.rgb();
	int alpha = a;
	if ( alpha>255 ) alpha = 255;
	if ( alpha<0 ) alpha = 0;
	int inv_alpha = 255 - alpha;
	QColor result = QColor ( qRgb ( qRed ( rgb_b ) *inv_alpha/255 + qRed ( rgb ) *alpha/255,
	                                qGreen ( rgb_b ) *inv_alpha/255 + qGreen ( rgb ) *alpha/255,
	                                qBlue ( rgb_b ) *inv_alpha/255 + qBlue ( rgb ) *alpha/255 ) );

	return result;
}

void Basket::unlock()
{
	QTimer::singleShot ( 0, this, SLOT ( load() ) );
}

void Basket::inactivityAutoLockTimeout()
{
	lock ();
}

void Basket::drawContents ( QPainter *painter, int clipX, int clipY, int clipWidth, int clipHeight )
{
	// Start the load the first time the basket is shown:
	if ( !m_loadingLaunched )
	{
		if ( !m_locked )
			QTimer::singleShot ( 0, this, SLOT ( load() ) );
		else
		{
			Global::bnpView->notesStateChanged(); // Show "Locked" instead of "Loading..." in the statusbar
		}
	}

	QBrush brush ( backgroundColor() ); // FIXME: share it for all the basket?
	QRect clipRect ( clipX, clipY, clipWidth, clipHeight );

	if ( m_locked )
	{
		if ( !m_decryptBox )
		{
			m_decryptBox = new QFrame ( this);
			m_decryptBox->setObjectName( "m_decryptBox" );
			m_decryptBox->setFrameShape ( QFrame::StyledPanel );
			m_decryptBox->setFrameShadow ( QFrame::Plain );
			m_decryptBox->setLineWidth ( 1 );

			QGridLayout* layout = new QGridLayout ( m_decryptBox);
			layout->setObjectName("decryptBoxLayout" );

#ifdef HAVE_LIBGPGME
			m_button = new QPushButton ( m_decryptBox, "button" );
			m_button->setText ( i18n ( "&Unlock" ) );
			layout->addWidget ( m_button, 1, 2 );
			connect ( m_button, SIGNAL ( clicked() ), this, SLOT ( unlock() ) );
#endif
			QLabel* label = new QLabel ( m_decryptBox);
			label->setObjectName("label" );
			QString text = "<b>" + i18n ( "Password protected basket." ) + "</b><br/>";
#ifdef HAVE_LIBGPGME
			label->setText ( text + i18n ( "Press Unlock to access it." ) );
#else
			label->setText ( text + i18n ( "Encryption is not supported by<br/>this version of %1." ).arg ( KCmdLineArgs::aboutData( )->programName() ) );
#endif
			label->setAlignment ( Qt::AlignTop );
			layout->addWidget ( label, 0, 1, 1, 2 );
			QLabel* pixmap = new QLabel ( m_decryptBox);
			pixmap->setObjectName( "pixmap" );
			pixmap->setPixmap ( KIcon("encrypted").pixmap(32,32) );
			layout->addWidget ( pixmap, 0, 0, 2, 1 );

			QSpacerItem* spacer = new QSpacerItem ( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
			layout->addItem ( spacer, 1, 1 );

			label = new QLabel ( "<small>" +
			                     i18n ( "To make baskets stay unlocked, change the automatic<br>"
			                            "locking duration in the application settings." ) + "</small>",
			                     m_decryptBox );
			//label->setFixedWidth(label->sizeHint().width() / 2);
			label->setAlignment (  Qt::AlignTop );
			layout->addWidget ( label, 2,0,1,3 );

			m_decryptBox->resize ( layout->sizeHint() );
		}
		if ( m_decryptBox->isHidden() )
		{
			m_decryptBox->show();
		}
#ifdef HAVE_LIBGPGME
		m_button->setFocus();
#endif
		m_decryptBox->move ( ( width() - m_decryptBox->width() ) / 2,
		                     ( height() - m_decryptBox->height() ) / 2 );
	}
	else
	{
		if ( m_decryptBox && m_decryptBox->isVisible() )
			m_decryptBox->hide();
	}

	// Draw notes (but not when it's not loaded or locked yet):
	Note *note = ( ( m_loaded || m_locked ) ? m_firstNote : 0 );
	while ( note )
	{
		note->draw ( painter, clipRect );
		note = note->next();
	}
	enableActions();

	// Draw loading message:
	if ( !m_loaded )
	{
		QPixmap pixmap ( width(), height() ); // TODO: Clip it to asked size only!
		QPainter painter2 ( &pixmap );
		QTextDocument rt ( QString ( "<center>%1</center>" ).arg ( i18n ( "Loading..." ) ));
		rt.setDefaultFont(QScrollArea::font() );
		QSizeF pageSize=rt.pageSize();
		pageSize.setWidth(width());
		rt.setPageSize(pageSize);
		int hrt = rt.pageSize().height();
		painter2.fillRect ( 0, 0, width(), height(), brush );
		blendBackground ( painter2, QRect ( 0, 0, width(), height() ), -1, -1, /*opaque=*/true );
		QPalette cg = palette();
		cg.setColor ( QPalette::Text, textColor() );
		//rt.drawContents ( &painter2, 0, ( height() - hrt ) / 2, QRect(), cg );
		rt.drawContents ( &painter2 );
		painter2.end();
		painter->drawPixmap ( 0, 0, pixmap );
		return; // TODO: Clip to the wanted rectangle
	}

	// We will draw blank areas below.
	// For each rectangle to be draw there is three ways to do so:
	// - The rectangle is full of the background COLOR  => we fill a rect directly on screen
	// - The rectangle is full of the background PIXMAP => we draw it directly on screen (we draw m_opaqueBackgroundPixmap that is not transparent)
	// - The rectangle contains the resizer             => We draw it on an offscreen buffer and then paint the buffer on screen
	// If the background image is not tiled, we know that recomputeBlankRects() broken rects so that they are full of either background pixmap or color, but not a mix.

	// Draw blank areas (see the last preparation above):
	QPainter painter2;
	QRect    rect;
	for ( QList<QRect>::iterator it = m_blankAreas.begin(); it != m_blankAreas.end(); ++it )
	{
		rect = clipRect.intersect ( *it );
		if ( rect.width() > 0 && rect.height() > 0 )
		{
			// If there is an inserter to draw, draw the image off screen,
			// apply the inserter and then draw the image on screen:
			if ( ( inserterShown() && rect.intersects ( inserterRect() ) )  || ( m_isSelecting && rect.intersects ( m_selectionRect ) ) )
			{
				QPixmap pixmap( rect.width(), rect.height() );
				painter2.begin ( &pixmap );
				painter2.fillRect ( 0, 0, rect.width(), rect.height(), backgroundColor() );
				blendBackground ( painter2, rect, -1, -1, /*opaque=*/true );
				// Draw inserter:
				if ( inserterShown() && rect.intersects ( inserterRect() ) )
					drawInserter ( painter2, rect.x(), rect.y() );
				// Draw selection rect:
				if ( m_isSelecting && rect.intersects ( m_selectionRect ) )
				{
					QRect selectionRect = m_selectionRect;
					selectionRect.moveTo ( -rect.x(), -rect.y() );
					QRect selectionRectInside ( selectionRect.x() + 1, selectionRect.y() + 1, selectionRect.width() - 2, selectionRect.height() - 2 );
					if ( selectionRectInside.width() > 0 && selectionRectInside.height() > 0 )
					{
						QColor insideColor = selectionRectInsideColor();
						painter2.fillRect ( selectionRectInside, insideColor );
						selectionRectInside.moveTo ( rect.x(), rect.y() );
						blendBackground ( painter2, selectionRectInside, rect.x(), rect.y(), true, /*&*/m_selectedBackgroundPixmap );
					}
					painter2.setPen ( KColorScheme(QPalette::Active,KColorScheme::Selection).background().color().darker() );
					painter2.drawRect ( selectionRect );
					painter2.setPen ( Tools::mixColor ( KColorScheme(QPalette::Active, KColorScheme::Selection).background().color().darker(), backgroundColor() ) );
					painter2.drawPoint ( selectionRect.topLeft() );
					painter2.drawPoint ( selectionRect.topRight() );
					painter2.drawPoint ( selectionRect.bottomLeft() );
					painter2.drawPoint ( selectionRect.bottomRight() );
				}
				painter2.end();
				painter->drawPixmap ( rect.x(), rect.y(), pixmap );
				// If it's only a blank rectangle to draw, draw it directly on screen (faster!!!):
			}
			else if ( ! hasBackgroundImage() )
			{
				painter->fillRect ( rect, backgroundColor() );
				// It's either a background pixmap to draw or a background color to fill:
			}
			else
			{
				if ( isTiledBackground() || ( rect.x() < backgroundPixmap()->width() && rect.y() < backgroundPixmap()->height() ) )
					blendBackground ( *painter, rect, 0, 0, /*opaque=*/true );
				else
					painter->fillRect ( rect, backgroundColor() );
			}
		}
	}
}

/*  rect(x,y,width,height)==(xBackgroundToDraw,yBackgroundToDraw,widthToDraw,heightToDraw)
 */
void Basket::blendBackground ( QPainter &painter, const QRect &rect, int xPainter, int yPainter, bool opaque, QPixmap *bg )
{
	if ( xPainter == -1 && yPainter == -1 )
	{
		xPainter = rect.x();
		yPainter = rect.y();
	}

	if ( hasBackgroundImage() )
	{
		const QPixmap *bgPixmap = ( bg ? /* * */bg : ( opaque ? m_opaqueBackgroundPixmap : m_backgroundPixmap ) );
		if ( isTiledBackground() )
			painter.drawTiledPixmap ( rect.x() - xPainter, rect.y() - yPainter, rect.width(), rect.height(), *bgPixmap, rect.x(), rect.y() );
		else
			painter.drawPixmap ( rect.x() - xPainter, rect.y() - yPainter, *bgPixmap, rect.x(), rect.y(), rect.width(), rect.height() );
	}
}

void Basket::recomputeBlankRects()
{
	m_blankAreas.clear();
	m_blankAreas.append ( QRect ( 0, 0, widget()->width(), widget()->height() ) );

	FOR_EACH_NOTE ( note )
	note->recomputeBlankRects ( m_blankAreas );

	// See the drawing of blank areas in Basket::drawContents()
	if ( hasBackgroundImage() && ! isTiledBackground() )
		substractRectOnAreas ( QRect ( 0, 0, backgroundPixmap()->width(), backgroundPixmap()->height() ), m_blankAreas, false );
}

void Basket::addAnimatedNote ( Note *note )
{
	if ( m_animatedNotes.isEmpty() )
	{
		m_animationTimer.start ( FRAME_DELAY );
		m_lastFrameTime = QTime::currentTime();
	}

	m_animatedNotes.append ( note );
}

void Basket::unsetNotesWidth()
{
	Note *note = m_firstNote;
	while ( note )
	{
		note->unsetWidth();
		note = note->next();
	}
}

void Basket::relayoutNotes ( bool animate )
{
	if ( Global::bnpView->currentBasket() != this )
		return; // Optimize load time, and basket will be relaid out when activated, anyway

	if ( !Settings::playAnimations() )
		animate = false;

	if ( !animate )
	{
		m_animatedNotes.clear();
		m_animationTimer.stop();
	}

	int h     = 0;
	tmpWidth  = 0;
	tmpHeight = 0;
	Note *note = m_firstNote;
	while ( note )
	{
		if ( note->matching() )
		{
			note->relayoutAt ( 0, h, animate );
			if ( note->hasResizer() )
			{
				int minGroupWidth = note->minRight() - note->finalX();
				if ( note->groupWidth() < minGroupWidth )
				{
					note->setGroupWidth ( minGroupWidth );
					relayoutNotes ( animate ); // Redo the thing, but this time it should not recurse
					return;
				}
			}
			h += note->finalHeight();
		}
		note = note->next();
	}

	if ( isFreeLayout() )
		tmpHeight += 100;
	else
		tmpHeight += 15;

	resize ( qMax ( tmpWidth, width() ), qMax ( tmpHeight, height() ) );
	recomputeBlankRects();
	placeEditor();
	doHoverEffects();
	updateGeometry();
}

void Basket::updateNote ( Note *note )
{
	update ( note->rect() );
	if ( note->hasResizer() )
		update ( note->resizerRect() );
}

void Basket::animateObjects()
{
	QList<Note*>::iterator it;
	for ( it = m_animatedNotes.begin(); it != m_animatedNotes.end(); )
//		if ((*it)->y() >= contentsY() && (*it)->bottom() <= contentsY() + contentsWidth())
//			updateNote(*it);
		if ( ( *it )->advance() )
			it = m_animatedNotes.erase ( it );
		else
		{
//			if ((*it)->y() >= contentsY() && (*it)->bottom() <= contentsY() + contentsWidth())
//				updateNote(*it);
			++it;
		}

	if ( m_animatedNotes.isEmpty() )
	{
		// Stop animation timer:
		m_animationTimer.stop();
		// Reset all onTop notes:
		Note* note = m_firstNote;
		while ( note )
		{
			note->setOnTop ( false );
			note = note->next();
		}
	}

	if ( m_focusedNote )
		ensureNoteVisible ( m_focusedNote );

	// We refresh content if it was the last frame,
	// or if the drawing of the last frame was not too long.
	if ( !m_animationTimer.isActive() || ( m_lastFrameTime.msecsTo ( QTime::currentTime() ) < FRAME_DELAY*11/10 ) )
	{ // *11/10 == *1.1 : We keep a 0.1 security margin
		m_lastFrameTime = m_lastFrameTime.addMSecs ( FRAME_DELAY );                                            // because timers are not accurate and can trigger late
		//m_lastFrameTime = QTime::currentTime();
//std::cout << ">>" << m_lastFrameTime.toString("hh:mm:ss.zzz") << std::endl;
		if ( m_underMouse )
			doHoverEffects();
		recomputeBlankRects();
		//relayoutNotes(true); // In case an animated note was to the contents view boundaries, resize the view!
		updateGeometry();
		// If the drawing of the last frame was too long, we skip the drawing of the current and do the next one:
	}
	else
	{
		m_lastFrameTime = m_lastFrameTime.addMSecs ( FRAME_DELAY );
//std::cout << "+=" << m_lastFrameTime.toString("hh:mm:ss.zzz") << std::endl;
		animateObjects();
	}

	doHoverEffects();
	placeEditor();

	/*	int delta = m_deltaY / 3;
		if (delta == 0       && m_deltaY != 0)
		delta = (m_deltaY > 0 ? 1 : -1);
		m_deltaY -= delta;
		resizeContents(contentsWidth(), contentsHeight() + delta); //m_lastNote->y() + m_lastNote->height()
	*/
}

void Basket::popupEmblemMenu ( Note *note, int emblemNumber )
{
	m_tagPopupNote = note;
	State *state = note->stateForEmblemNumber ( emblemNumber );
	State *nextState = state->nextState ( /*cycle=*/false );
	Tag *tag = state->parentTag();
	m_tagPopup = tag;

	QKeySequence sequence = tag->shortcut().primary();
	bool sequenceOnDelete = ( nextState == 0 && !tag->shortcut().isEmpty() );

	QAction* tmpAction;
	KMenu menu ( this );
	if ( tag->countStates() == 1 )
	{
		menu.addMenu ( /*SmallIcon(state->icon()), */tag->name() );
		tmpAction = menu.addTitle( i18n ( "&Remove" ) );
		tmpAction->setIcon(KIcon ("edit-delete" ));
//TODO 		menu.insertItem ( KIcon ( "configure" ),  i18n ( "&Customize..." ),       2 );
 		menu.insertSeparator(0);
		//TODO		menu.insertItem ( KIcon ( "search-filter" ),     i18n ( "&Filter by this Tag" ), 3 );
	}
	else
	{
		menu.addTitle ( tag->name() );
		QList<State*>::iterator it;
		State *currentState;

		int i = 10;
		for ( it = tag->states().begin(); it != tag->states().end(); ++it )
		{
			currentState = *it;
			QKeySequence sequence;
			if ( currentState == nextState && !tag->shortcut().isEmpty() )
				sequence = tag->shortcut().primary();
			//TODO menu.insertItem ( StateMenuItem::radioButtonIconSet ( state == currentState, menu.colorGroup() ), new StateMenuItem ( currentState, sequence, false ), i );
			if ( currentState == nextState && !tag->shortcut().isEmpty() )
			  //TODO				menu.setAccel ( sequence, i );
			++i;
		}
		//TODO		menu.insertSeparator();
		//TODO	menu.insertItem ( new IndentedMenuItem ( i18n ( "&Remove" ),               "edit-delete", ( sequenceOnDelete ? sequence : QKeySequence() ) ), 1 );
		//TODO		menu.insertItem ( new IndentedMenuItem ( i18n ( "&Customize..." ),         "configure" ),  2 );
		//TODO		menu.insertSeparator();
		//TODO	menu.insertItem ( new IndentedMenuItem ( i18n ( "&Filter by this Tag" ),   "search-filter" ),     3 );
		//TODO		menu.insertItem ( new IndentedMenuItem ( i18n ( "Filter by this &State" ), "search-filter" ),     4 );
	}
	if ( sequenceOnDelete )
	  //TODO menu.setAccel ( sequence, 1 );

	connect ( &menu, SIGNAL ( activated ( int ) ), this, SLOT ( toggledStateInMenu ( int ) ) );
	connect ( &menu, SIGNAL ( aboutToHide() ),  this, SLOT ( unlockHovering() ) );
	connect ( &menu, SIGNAL ( aboutToHide() ),  this, SLOT ( disableNextClick() ) );

	m_lockedHovering = true;
	menu.exec ( QCursor::pos() );
}

void Basket::toggledStateInMenu ( int id )
{
	if ( id == 1 )
	{
		removeTagFromSelectedNotes ( m_tagPopup );
		//m_tagPopupNote->removeTag(m_tagPopup);
		//m_tagPopupNote->setWidth(0); // To force a new layout computation
		updateEditorAppearance();
		filterAgain();
		save();
		return;
	}
	if ( id == 2 )
	{ // Customize this State:
		TagsEditDialog dialog ( this, m_tagPopupNote->stateOfTag ( m_tagPopup ) );
		dialog.exec();
		return;
	}
	if ( id == 3 )
	{ // Filter by this Tag
		decoration()->filterBar()->filterTag ( m_tagPopup );
		return;
	}
	if ( id == 4 )
	{ // Filter by this State
		decoration()->filterBar()->filterState ( m_tagPopupNote->stateOfTag ( m_tagPopup ) );
		return;
	}

	/*addStateToSelectedNotes*/changeStateOfSelectedNotes ( m_tagPopup->states() [id - 10] /*, orReplace=true*/ );
	//m_tagPopupNote->addState(m_tagPopup->states()[id - 10], true);
	filterAgain();
	save();
}

State* Basket::stateForTagFromSelectedNotes ( Tag *tag )
{
	State *state = 0;

	FOR_EACH_NOTE ( note )
	if ( note->stateForTagFromSelectedNotes ( tag, &state ) && state == 0 )
		return 0;
	return state;
}

void Basket::activatedTagShortcut ( Tag *tag )
{
	// Compute the next state to set:
	State *state = stateForTagFromSelectedNotes ( tag );
	if ( state )
		state = state->nextState ( /*cycle=*/false );
	else
		state = tag->states().first();

	// Set or unset it:
	if ( state )
	{
		FOR_EACH_NOTE ( note )
		note->addStateToSelectedNotes ( state, /*orReplace=*/true );
		updateEditorAppearance();
	}
	else
		removeTagFromSelectedNotes ( tag );

	filterAgain();
	save();
}

void Basket::popupTagsMenu ( Note *note )
{
	m_tagPopupNote = note;

	KMenu menu ( this );
	menu.addTitle ( i18n ( "Tags" ) );
// 	QList<Tag*>::iterator it;
// 	Tag *currentTag;
// 	State *currentState;
// 	int i = 10;
// 	for (it = Tag::all.begin(); it != Tag::all.end(); ++it) {
// 		// Current tag and first state of it:
// 		currentTag = *it;
// 		currentState = currentTag->states().first();
// 		QKeySequence sequence;
// 		if (!currentTag->shortcut().isNull())
// 			sequence = currentTag->shortcut().operator QKeySequence();
// 		menu.insertItem(StateMenuItem::checkBoxIconSet(note->hasTag(currentTag), menu.colorGroup()), new StateMenuItem(currentState, sequence, true), i );
// 		if (!currentTag->shortcut().isNull())
// 			menu.setAccel(sequence, i);
// 		++i;
// 	}
//
// 	menu.insertSeparator();
// //	menu.insertItem( /*SmallIconSet("edit-delete"),*/ "&Assign New Tag...", 1 );
// 	//id = menu.insertItem( SmallIconSet("edit-delete"), "&Remove All", -2 );
// 	//if (note->states().isEmpty())
// 	//	menu.setItemEnabled(id, false);
// //	menu.insertItem( SmallIconSet("configure"),  "&Customize...", 3 );
// 	menu.insertItem( new IndentedMenuItem(i18n("&Assign New Tag...")),          1 );
// 	menu.insertItem( new IndentedMenuItem(i18n("&Remove All"),   "edit-delete"), 2 );
// 	menu.insertItem( new IndentedMenuItem(i18n("&Customize..."), "configure"),  3 );
//
// 	if (!selectedNotesHaveTags())//note->states().isEmpty())
// 		menu.setItemEnabled(2, false);
//
// 	connect( &menu, SIGNAL(activated(int)), this, SLOT(toggledTagInMenu(int)) );
// 	connect( &menu, SIGNAL(aboutToHide()),  this, SLOT(unlockHovering())      );
// 	connect( &menu, SIGNAL(aboutToHide()),  this, SLOT(disableNextClick())    );

	Global::bnpView->populateTagsMenu ( menu, note );

	m_lockedHovering = true;
	menu.exec ( QCursor::pos() );
}

void Basket::unlockHovering()
{
	m_lockedHovering = false;
	doHoverEffects();
}

void Basket::toggledTagInMenu ( int id )
{
	if ( id == 1 )
	{ // Assign new Tag...
		TagsEditDialog dialog ( this, /*stateToEdit=*/0, /*addNewTag=*/true );
		dialog.exec();
		if ( !dialog.addedStates().isEmpty() )
		{
			State::List states = dialog.addedStates();
			for ( State::List::iterator itState = states.begin(); itState != states.end(); ++itState )
				FOR_EACH_NOTE ( note )
				note->addStateToSelectedNotes ( *itState );
			updateEditorAppearance();
			filterAgain();
			save();
		}
		return;
	}
	if ( id == 2 )
	{ // Remove All
		removeAllTagsFromSelectedNotes();
		filterAgain();
		save();
		return;
	}
	if ( id == 3 )
	{ // Customize...
		TagsEditDialog dialog ( this );
		dialog.exec();
		return;
	}

	Tag *tag = Tag::all[id - 10];
	if ( !tag )
		return;

	if ( m_tagPopupNote->hasTag ( tag ) )
		removeTagFromSelectedNotes ( tag );
	else
		addTagToSelectedNotes ( tag );
	m_tagPopupNote->setWidth ( 0 ); // To force a new layout computation
	filterAgain();
	save();
}

void Basket::addTagToSelectedNotes ( Tag *tag )
{
	FOR_EACH_NOTE ( note )
	note->addTagToSelectedNotes ( tag );
	updateEditorAppearance();
}

void Basket::removeTagFromSelectedNotes ( Tag *tag )
{
	FOR_EACH_NOTE ( note )
	note->removeTagFromSelectedNotes ( tag );
	updateEditorAppearance();
}

void Basket::addStateToSelectedNotes ( State *state )
{
	FOR_EACH_NOTE ( note )
	note->addStateToSelectedNotes ( state );
	updateEditorAppearance();
}

void Basket::updateEditorAppearance()
{
	if ( isDuringEdit() && m_editor->widget() )
	{
		m_editor->widget()->setFont ( m_editor->note()->font() );



		//		m_editor->widget()->setPaletteBackgroundColor ( m_editor->note()->backgroundColor() );
		QPalette palette;
		palette.setColor(m_editor->widget()->backgroundRole(), m_editor->note()->backgroundColor() );
		m_editor->widget()->setPalette(palette);


		//		m_editor->widget()->setPaletteForegroundColor ( m_editor->note()->textColor() );
		palette.setColor(m_editor->widget()->foregroundRole(), m_editor->note()->textColor() );
		m_editor->widget()->setPalette(palette);

		// Uggly Hack arround Qt bugs: placeCursor() don't call any signal:
// 		HtmlEditor *htmlEditor = dynamic_cast<HtmlEditor*> ( m_editor );
// 		if ( htmlEditor )
// 		{
// 			int para, index;
// 			m_editor->textEdit()->getCursorPosition ( &para, &index );
// 			if ( para == 0 && index == 0 )
// 			{
// 			  m_editor->textEdit()->moveCursor ( QTextEdit::MoveForward,  /*select=*/false );
			  

// 			  m_editor->textEdit()->moveCursor ( QTextEdit::MoveBackward, /*select=*/false );
// 			}
// 			else
// 			{
// 				m_editor->textEdit()->moveCursor ( QTextEdit::MoveBackward, /*select=*/false );
// 				m_editor->textEdit()->moveCursor ( QTextEdit::MoveForward,  /*select=*/false );
// 			}
// 			htmlEditor->cursorPositionChanged(); // Does not work anyway :-( (when clicking on a red bold text, the toolbar still show black normal text)
// 		}
	}
}

void Basket::editorPropertiesChanged()
{
	if ( isDuringEdit() && m_editor->note()->content()->type() == NoteType::Html )
	{
		m_editor->textEdit()->setAutoFormatting ( Settings::autoBullet() ? QTextEdit::AutoAll : QTextEdit::AutoNone );
	}
}

void Basket::changeStateOfSelectedNotes ( State *state )
{
	FOR_EACH_NOTE ( note )
	note->changeStateOfSelectedNotes ( state );
	updateEditorAppearance();
}

void Basket::removeAllTagsFromSelectedNotes()
{
	FOR_EACH_NOTE ( note )
	note->removeAllTagsFromSelectedNotes();
	updateEditorAppearance();
}

bool Basket::selectedNotesHaveTags()
{
	FOR_EACH_NOTE ( note )
	if ( note->selectedNotesHaveTags() )
		return true;
	return false;
}

QColor Basket::backgroundColor()
{
	if ( m_backgroundColorSetting.isValid() )
		return m_backgroundColorSetting;
	else
		return KColorScheme(QPalette::Active, KColorScheme::View).background().color();
}

QColor Basket::textColor()
{
	if ( m_textColorSetting.isValid() )
		return m_textColorSetting;
	else
		return KColorScheme(QPalette::Active, KColorScheme::View).foreground().color();
}

void Basket::unbufferizeAll()
{
	FOR_EACH_NOTE ( note )
	note->unbufferizeAll();
}

Note* Basket::editedNote()
{
	if ( m_editor )
		return m_editor->note();
	else
		return 0;
}

bool Basket::hasTextInEditor()
{
	if ( !isDuringEdit() || !redirectEditActions() )
		return false;

	if ( m_editor->textEdit() )
		return ! m_editor->textEdit()->toPlainText().isEmpty();
	else if ( m_editor->lineEdit() )
		return ! m_editor->lineEdit()->text().isEmpty();
	else
		return false;
}

bool Basket::hasSelectedTextInEditor()
{
	if ( !isDuringEdit() || !redirectEditActions() )
		return false;

	if ( m_editor->textEdit() )
	{
		// The following line does NOT work if one letter is selected and the user press Shift+Left or Shift+Right to unselect than letter:
		// Qt misteriously tell us there is an invisible selection!!
		//return m_editor->textEdit()->hasSelectedText();
	  return !m_editor->textEdit()->textCursor().selectedText().isEmpty();
	}
	else if ( m_editor->lineEdit() )
		return m_editor->lineEdit()->hasSelectedText();
	else
		return false;
}

bool Basket::selectedAllTextInEditor()
{
	if ( !isDuringEdit() || !redirectEditActions() )
		return false;

	if ( m_editor->textEdit() )
		return m_editor->textEdit()->toPlainText().isEmpty() || m_editor->textEdit()->toPlainText() == m_editor->textEdit()->textCursor().selectedText();
	else if ( m_editor->lineEdit() )
		return m_editor->lineEdit()->text().isEmpty() || m_editor->lineEdit()->text() == m_editor->lineEdit()->selectedText();
	else
		return false;
}

void Basket::selectionChangedInEditor()
{
	Global::bnpView->notesStateChanged();
}

void Basket::contentChangedInEditor()
{
	// Do not wait 3 seconds, because we need the note to expand as needed (if a line is too wider... the note should grow wider):
	if ( m_editor->textEdit() )
		m_editor->autoSave ( /*toFileToo=*/false );
//	else {
	if ( m_inactivityAutoSaveTimer.isActive() )
		m_inactivityAutoSaveTimer.stop();
	m_inactivityAutoSaveTimer.setSingleShot(true);
	m_inactivityAutoSaveTimer.start ( 3 * 1000);
	Global::bnpView->setUnsavedStatus ( true );
//	}
}

void Basket::inactivityAutoSaveTimeout()
{
	if ( m_editor )
		m_editor->autoSave ( /*toFileToo=*/true );
}

void Basket::placeEditorAndEnsureVisible()
{
	placeEditor ( /*andEnsureVisible=*/true );
}

void Basket::placeEditor ( bool /*andEnsureVisible*/ /*= false*/ )
{
	if ( !isDuringEdit() )
		return;

	QFrame    *editorQFrame = dynamic_cast<QFrame*> ( m_editor->widget() );
	KTextEdit *textEdit     = m_editor->textEdit();
//	QLineEdit *lineEdit     = m_editor->lineEdit();
	Note      *note         = m_editor->note();

	int frameWidth = ( editorQFrame ? editorQFrame->frameWidth() : 0 );
	int x          = note->x() + note->contentX() + note->content()->xEditorIndent() - frameWidth;
	int y;
	//TODO int maxHeight  = qMax ( this->height(), contentsHeight() );
	int maxHeight = this->height();
	int height, width;

	if ( textEdit )
	{
		x -= 4;
		// Need to do it 2 times, because it's wrong overwise
		// (sometimes, width depends on height, and sometimes, height depends on with):
		for ( int i = 0; i < 2; i++ )
		{
			// FIXME: CRASH: Select all text, press Del or [<--] and editor->removeSelectedText() is called:
			//        editor->sync() CRASH!!
			//		editor->sync();
			y = note->y() + Note::NOTE_MARGIN - frameWidth;
			//TODO height = textEdit->contentsHeight() + 2*frameWidth;
			height = textEdit->height() + 2 * frameWidth;
//			height = /*qMax(*/height/*, note->height())*/;
//			height = qMin(height, visibleHeight());
			width  = note->x() + note->width() - x + 1;//      /*note->x() + note->width()*/note->rightLimit() - x + 2*frameWidth + 1;
//width=qMax(width,textEdit->contentsWidth()+2*frameWidth);
			if ( y + height > maxHeight )
				y = maxHeight - height;
			textEdit->setFixedSize ( width, height );
		}
	}
	else
	{
		height = note->height() - 2*Note::NOTE_MARGIN + 2*frameWidth;
		width  = note->x() + note->width() - x;//note->rightLimit() - x + 2*frameWidth;
		m_editor->widget()->setFixedSize ( width, height );
		x -= 1;
		y = note->y() + Note::NOTE_MARGIN - frameWidth;
	}
	if ( ( m_editorWidth > 0 && m_editorWidth != width ) || ( m_editorHeight > 0 && m_editorHeight != height ) )
	{
		m_editorWidth  = width; // Avoid infinite recursion!!!
		m_editorHeight = height;
		m_editor->autoSave ( /*toFileToo=*/true );
	}
	m_editorWidth  = width;
	m_editorHeight = height;
	//TODO addChild ( m_editor->widget(), x, y );
	m_editorX = x;
	m_editorY = y;

	m_leftEditorBorder->setFixedSize ( ( m_editor->textEdit() ? 3 : 0 ), height );
//	m_leftEditorBorder->raise();
	//TODO addChild ( m_leftEditorBorder,     x, y );
	m_leftEditorBorder->setPosition ( x, y );

	m_rightEditorBorder->setFixedSize ( 3, height );
//	m_rightEditorBorder->raise();
//	addChild(m_rightEditorBorder,     note->rightLimit() - Note::NOTE_MARGIN, note->y() + Note::NOTE_MARGIN );
//	m_rightEditorBorder->setPosition( note->rightLimit() - Note::NOTE_MARGIN, note->y() + Note::NOTE_MARGIN );
	//TODO addChild ( m_rightEditorBorder,     note->x() + note->width() - Note::NOTE_MARGIN, note->y() + Note::NOTE_MARGIN );
	m_rightEditorBorder->setPosition ( note->x() + note->width() - Note::NOTE_MARGIN, note->y() + Note::NOTE_MARGIN );

//	if (andEnsureVisible)
//		ensureNoteVisible(note);
}

#include <iostream>
//#include <private/qrichtext_p.h>
void Basket::editorCursorPositionChanged()
{
	if ( !isDuringEdit() )
		return;

	FocusedTextEdit *textEdit = ( FocusedTextEdit* ) m_editor->textEdit();
	const QTextCursor *cursor = textEdit->textCursor();
//	std::cout << cursor->x() << ";" << cursor->y() << "      "
//			  << cursor->globalX() << ";" << cursor->globalY() << "          "
//			  << cursor->offsetX() << ";" << cursor->offsetY() << ";" << std::endl;

	//TODO ensureVisible ( m_editorX + cursor->globalX(), m_editorY + cursor->globalY(), 50, 50 );
}

void Basket::closeEditorDelayed()
{
	setFocus();
	QTimer::singleShot ( 0, this, SLOT ( closeEditor() ) );
}

bool Basket::closeEditor()
{
	if ( !isDuringEdit() )
		return true;

	if ( m_doNotCloseEditor )
		return true;

	if ( m_redirectEditActions )
	{
		disconnect ( m_editor->widget(), SIGNAL ( selectionChanged() ), this, SLOT ( selectionChangedInEditor() ) );
		if ( m_editor->textEdit() )
		{
			disconnect ( m_editor->textEdit(), SIGNAL ( textChanged() ),               this, SLOT ( selectionChangedInEditor() ) );
			disconnect ( m_editor->textEdit(), SIGNAL ( textChanged() ),               this, SLOT ( contentChangedInEditor() ) );
		}
		else if ( m_editor->lineEdit() )
		{
			disconnect ( m_editor->lineEdit(), SIGNAL ( textChanged ( const QString& ) ), this, SLOT ( selectionChangedInEditor() ) );
			disconnect ( m_editor->lineEdit(), SIGNAL ( textChanged ( const QString& ) ), this, SLOT ( contentChangedInEditor() ) );
		}
	}
	m_editor->widget()->disconnect();
	m_editor->widget()->hide();
	m_editor->validate();

	delete m_leftEditorBorder;
	delete m_rightEditorBorder;
	m_leftEditorBorder  = 0;
	m_rightEditorBorder = 0;

	Note *note = m_editor->note();
	note->setWidth ( 0 ); // For relayoutNotes() to succeed to take care of the change

	// Delete the editor BEFORE unselecting the note because unselecting the note would trigger closeEditor() recursivly:
	bool isEmpty = m_editor->isEmpty();
	delete m_editor;
	m_editor = 0;
	m_redirectEditActions = false;
	m_editorWidth  = -1;
	m_editorHeight = -1;
	m_inactivityAutoSaveTimer.stop();

	// Delete the note if it is now empty:
	if ( isEmpty )
	{
		focusANonSelectedNoteAboveOrThenBelow();
		note->setSelected ( true );
		note->deleteSelectedNotes();
		save();
		note = 0;
	}

	unlockHovering();
	filterAgain ( /*andEnsureVisible=*/false );

// Does not work:
//	if (Settings::playAnimations())
//		note->setOnTop(true); // So if it grew, do not obscure it temporarily while the notes below it are moving

	if ( note )
		note->setSelected ( false );//unselectAll();
	doHoverEffects();
//	save();

	Global::bnpView->m_actEditNote->setEnabled ( !isLocked() && countSelecteds() == 1 /*&& !isDuringEdit()*/ );

	emit resetStatusBarText(); // Remove the "Editing. ... to validate." text.

	//if (kapp->activeWindow() == Global::mainContainer)

	// Set focus to the basket, unless the user pressed a letter key in the filter bar and the currently edited note came hidden, then editing closed:
	if ( !decoration()->filterBar()->lineEdit()->hasFocus() )
		setFocus();

	// Return true if the note is still there:
	return ( note != 0 );
}

void Basket::closeBasket()
{
	closeEditor();
	unbufferizeAll(); // Keep the memory footprint low
	if ( isEncrypted() )
	{
		if ( Settings::enableReLockTimeout() )
		{
			int seconds = Settings::reLockTimeoutMinutes() * 60;
			m_inactivityAutoLockTimer.setSingleShot(true);
			m_inactivityAutoLockTimer.start ( seconds * 1000 );
		}
	}
}

void Basket::openBasket()
{
	if ( m_inactivityAutoLockTimer.isActive() )
		m_inactivityAutoLockTimer.stop();
}

Note* Basket::theSelectedNote()
{
	if ( countSelecteds() != 1 )
	{
		std::cout << "NO SELECTED NOTE !!!!" << std::endl;
		return 0;
	}

	Note *selectedOne;
	FOR_EACH_NOTE ( note )
	{
		selectedOne = note->theSelectedNote();
		if ( selectedOne )
			return selectedOne;
	}

	std::cout << "One selected note, BUT NOT FOUND !!!!" << std::endl;

	return 0;
}

void debugSel ( NoteSelection* sel, int n = 0 )
{
	QString debugStr;
	for ( NoteSelection *node = sel; node; node = node->next )
	{
		debugStr=QString();
		for ( int i = 0; i < n; i++ )
			QTextStream(&debugStr) << "-";
		QTextStream(&debugStr) << ( node->firstChild ? "Group" : node->note->content()->toText ( "" ));
		qDebug()<<debugStr;
		if ( node->firstChild )
			debugSel ( node->firstChild, n+1 );
	}
}

NoteSelection* Basket::selectedNotes()
{
	NoteSelection selection;

	FOR_EACH_NOTE ( note )
	selection.append ( note->selectedNotes() );

	if ( !selection.firstChild )
		return 0;

	for ( NoteSelection *node = selection.firstChild; node; node = node->next )
		node->parent = 0;

	// If the top-most groups are columns, export only childs of those groups
	// (because user is not consciencious that columns are groups, and don't care: it's not what she want):
	if ( selection.firstChild->note->isColumn() )
	{
		NoteSelection tmpSelection;
		NoteSelection *nextNode;
		NoteSelection *nextSubNode;
		for ( NoteSelection *node = selection.firstChild; node; node = nextNode )
		{
			nextNode = node->next;
			if ( node->note->isColumn() )
			{
				for ( NoteSelection *subNode = node->firstChild; subNode; subNode = nextSubNode )
				{
					nextSubNode = subNode->next;
					tmpSelection.append ( subNode );
					subNode->parent = 0;
					subNode->next = 0;
				}
			}
			else
			{
				tmpSelection.append ( node );
				node->parent = 0;
				node->next = 0;
			}
		}
//		debugSel(tmpSelection.firstChild);
		return tmpSelection.firstChild;
	}
	else
	{
//		debugSel(selection.firstChild);
		return selection.firstChild;
	}
}

void Basket::showEditedNoteWhileFiltering()
{
	if ( m_editor )
	{
		Note *note = m_editor->note();
		filterAgain();
		note->setSelected ( true );
		relayoutNotes ( false );
		note->setX ( note->finalX() );
		note->setY ( note->finalY() );
		filterAgainDelayed();
	}
}

void Basket::noteEdit ( Note *note, bool justAdded, const QPoint &clickedPoint ) // TODO: Remove the first parameter!!!
{
	if ( !note )
		note = theSelectedNote(); // TODO: Or pick the focused note!
	if ( !note )
		return;

	if ( isDuringEdit() )
	{
		closeEditor(); // Validate the noteeditors in KLineEdit that does not intercept Enter key press (and edit is triggered with Enter too... Can conflict)
		return;
	}

	if ( note != m_focusedNote )
	{
		setFocusedNote ( note );
		m_startOfShiftSelectionNote = note;
	}

	if ( justAdded && isFiltering() )
	{
		QTimer::singleShot ( 0, this, SLOT ( showEditedNoteWhileFiltering() ) );
	}

	doHoverEffects ( note, Note::Content ); // Be sure (in the case Edit was triggered by menu or Enter key...): better feedback!
	//m_lockedHovering = true;

	//m_editorWidget = note->content()->launchEdit(this);
	NoteEditor *editor = NoteEditor::editNoteContent ( note->content(), this );

	if ( editor->widget() )
	{
		m_editor = editor;
		QWidget* tmpWidget;
		m_leftEditorBorder  = new TransparentWidget ( this );
		m_rightEditorBorder = new TransparentWidget ( this );
		tmpWidget=m_editor->widget();
		tmpWidget->setParent ( viewport());
		tmpWidget->move(QPoint ( 0,0 ));
		tmpWidget->show();

		m_leftEditorBorder->setParent ( viewport());
		m_leftEditorBorder->move(QPoint ( 0,0 ));
		m_leftEditorBorder->show();
		m_rightEditorBorder->setParent ( viewport());
		m_rightEditorBorder->move(QPoint ( 0,0 ));
		m_rightEditorBorder->show();

		//TODO addChild ( m_editor->widget(), 0, 0 );
		placeEditorAndEnsureVisible(); //       placeEditor(); // FIXME: After?
		m_redirectEditActions = m_editor->lineEdit() || m_editor->textEdit();
		if ( m_redirectEditActions )
		{
			connect ( m_editor->widget(), SIGNAL ( selectionChanged() ), this, SLOT ( selectionChangedInEditor() ) );
			// In case there is NO text, "Select All" is disabled. But if the user press a key the there is now a text:
			// selection has not changed but "Select All" should be re-enabled:
			if ( m_editor->textEdit() )
			{
				connect ( m_editor->textEdit(), SIGNAL ( textChanged() ),               this, SLOT ( selectionChangedInEditor() ) );
				connect ( m_editor->textEdit(), SIGNAL ( textChanged() ),               this, SLOT ( contentChangedInEditor() ) );
			}
			else if ( m_editor->lineEdit() )
			{
				connect ( m_editor->lineEdit(), SIGNAL ( textChanged ( const QString& ) ), this, SLOT ( selectionChangedInEditor() ) );
				connect ( m_editor->lineEdit(), SIGNAL ( textChanged ( const QString& ) ), this, SLOT ( contentChangedInEditor() ) );
			}
		}
		m_editor->widget()->show();
		//m_editor->widget()->raise();
		m_editor->widget()->setFocus();
		connect ( m_editor, SIGNAL ( askValidation() ),            this, SLOT ( closeEditorDelayed() ) );
		connect ( m_editor, SIGNAL ( mouseEnteredEditorWidget() ), this, SLOT ( mouseEnteredEditorWidget() ) );
		if ( m_editor->textEdit() )
		{
			connect ( m_editor->textEdit(), SIGNAL ( textChanged() ), this, SLOT ( placeEditorAndEnsureVisible() ) );
			if ( clickedPoint != QPoint() )
			{
				QPoint pos ( clickedPoint.x() - note->x() - note->contentX() + m_editor->textEdit()->frameWidth() + 4   - m_editor->textEdit()->frameWidth(),
				             clickedPoint.y() - note->y()   - m_editor->textEdit()->frameWidth() );
				// Do it right before the kapp->processEvents() to not have the cursor to quickly flicker at end (and sometimes stay at end AND where clicked):

				m_editor->textEdit()->moveCursor ( QTextCursor::StartOfLine,QTextCursor::MoveAnchor);
				m_editor->textEdit()->ensureCursorVisible();

				m_editor->textEdit()->cursorForPosition( pos );
				updateEditorAppearance();
			}
		}
//		kapp->processEvents();     // Show the editor toolbar before ensuring the note is visible
		ensureNoteVisible ( note );   //  because toolbar can create a new line and then partially hide the note
		m_editor->widget()->setFocus(); // When clicking in the basket, a QTimer::singleShot(0, ...) focus the basket! So we focus the the widget after kapp->processEvents()
		emit resetStatusBarText(); // Display "Editing. ... to validate."
	}
	else
	{
		// Delete the note user have canceled the addition:
		if ( ( justAdded && editor->canceled() ) || editor->isEmpty() /*) && editor->note()->states().count() <= 0*/ )
		{
			focusANonSelectedNoteAboveOrThenBelow();
			editor->note()->setSelected ( true );
			editor->note()->deleteSelectedNotes();
			save();
		}
		delete editor;
		unlockHovering();
		filterAgain();
		unselectAll();
	}
	Global::bnpView->m_actEditNote->setEnabled ( false );
}

void Basket::noteDelete()
{
	if ( redirectEditActions() )
	{
		if ( m_editor->textEdit() )
			m_editor->textEdit()->cut();
		else if ( m_editor->lineEdit() )
			m_editor->lineEdit()->cut();
		return;
	}

	if ( countSelecteds() <= 0 )
		return;
	int really = KMessageBox::Yes;
	if ( Settings::confirmNoteDeletion() )
		really = KMessageBox::questionYesNo ( this,
		                                      i18nc ( "<qt>Do you really want to delete this note?</qt>",
		                                               "<qt>Do you really want to delete those <b>%n</b> notes?</qt>",
		                                               countSelecteds() ),
		                                      i18nc ( "Delete Note", "Delete Notes", countSelecteds() )
#if KDE_IS_VERSION( 3, 2, 90 )   // KDE 3.3.x
		                                      , KStandardGuiItem::del(), KStandardGuiItem::cancel() );
#else
		                                    );
#endif
	if ( really == KMessageBox::No )
		return;

	noteDeleteWithoutConfirmation();
}

void Basket::focusANonSelectedNoteBelow ( bool inSameColumn )
{
	// First focus another unselected one below it...:
	if ( m_focusedNote && m_focusedNote->isSelected() )
	{
		Note *next = m_focusedNote->nextShownInStack();
		while ( next && next->isSelected() )
			next = next->nextShownInStack();
		if ( next )
		{
			if ( inSameColumn && isColumnsLayout() && m_focusedNote->parentPrimaryNote() == next->parentPrimaryNote() )
			{
				setFocusedNote ( next );
				m_startOfShiftSelectionNote = next;
			}
		}
	}
}

void Basket::focusANonSelectedNoteAbove ( bool inSameColumn )
{
	// ... Or above it:
	if ( m_focusedNote && m_focusedNote->isSelected() )
	{
		Note *prev = m_focusedNote->prevShownInStack();
		while ( prev && prev->isSelected() )
			prev = prev->prevShownInStack();
		if ( prev )
		{
			if ( inSameColumn && isColumnsLayout() && m_focusedNote->parentPrimaryNote() == prev->parentPrimaryNote() )
			{
				setFocusedNote ( prev );
				m_startOfShiftSelectionNote = prev;
			}
		}
	}
}

void Basket::focusANonSelectedNoteBelowOrThenAbove()
{
	focusANonSelectedNoteBelow ( /*inSameColumn=*/true );
	focusANonSelectedNoteAbove ( /*inSameColumn=*/true );
	focusANonSelectedNoteBelow ( /*inSameColumn=*/false );
	focusANonSelectedNoteAbove ( /*inSameColumn=*/false );
}

void Basket::focusANonSelectedNoteAboveOrThenBelow()
{
	focusANonSelectedNoteAbove ( /*inSameColumn=*/true );
	focusANonSelectedNoteBelow ( /*inSameColumn=*/true );
	focusANonSelectedNoteAbove ( /*inSameColumn=*/false );
	focusANonSelectedNoteBelow ( /*inSameColumn=*/false );
}

void Basket::noteDeleteWithoutConfirmation ( bool deleteFilesToo )
{
	// If the currently focused note is selected, it will be deleted.
	focusANonSelectedNoteBelowOrThenAbove();

	// Do the deletion:
	Note *note = firstNote();
	Note *next;
	while ( note )
	{
		next = note->next(); // If we delete 'note' on the next line, note->next() will be 0!
		note->deleteSelectedNotes ( deleteFilesToo );
		note = next;
	}

	relayoutNotes ( true ); // FIXME: filterAgain()?
	save();
}

void Basket::doCopy ( CopyMode copyMode )
{
	QClipboard *cb = KApplication::clipboard();
	QClipboard::Mode mode = ( copyMode == CopyToSelection ? QClipboard::Selection : QClipboard::Clipboard );

	NoteSelection *selection = selectedNotes();
	int countCopied = countSelecteds();
	if ( selection->firstStacked() )
	{
		//TODO QDragObject *d = NoteDrag::dragObject ( selection, copyMode == CutToClipboard, /*source=*/0 ); // d will be deleted by QT
//		/*bool shouldRemove = */d->drag();
//		delete selection;
		//TODO cb->setData ( d, mode ); // NoteMultipleDrag will be deleted by QT
//		if (copyMode == CutToClipboard && !note->useFile()) // If useFile(), NoteDrag::dragObject() will delete it TODO
//			note->slotDelete();

		if ( copyMode == CutToClipboard )
			noteDeleteWithoutConfirmation ( /*deleteFilesToo=*/false );

		switch ( copyMode )
		{
			default:
			case CopyToClipboard: emit postMessage ( i18nc ( "Copied note to clipboard.", "Copied notes to clipboard.", countCopied ) ); break;
			case CutToClipboard:  emit postMessage ( i18nc ( "Cut note to clipboard.",    "Cut notes to clipboard.",    countCopied ) ); break;
			case CopyToSelection: emit postMessage ( i18nc ( "Copied note to selection.", "Copied notes to selection.", countCopied ) ); break;
		}
	}
}

void Basket::noteCopy()
{
	if ( redirectEditActions() )
	{
		if ( m_editor->textEdit() )
			m_editor->textEdit()->copy();
		else if ( m_editor->lineEdit() )
			m_editor->lineEdit()->copy();
	}
	else
		doCopy ( CopyToClipboard );
}

void Basket::noteCut()
{
	if ( redirectEditActions() )
	{
		if ( m_editor->textEdit() )
			m_editor->textEdit()->cut();
		else if ( m_editor->lineEdit() )
			m_editor->lineEdit()->cut();
	}
	else
		doCopy ( CutToClipboard );
}

void Basket::noteOpen ( Note *note )
{
	/*
	GetSelectedNotes
	NoSelectedNote || Count == 0 ? return
	AllTheSameType ?
	Get { url, message(count) }
	*/

	// TODO: Open ALL selected notes!
	if ( !note )
		note = theSelectedNote();
	if ( !note )
		return;

	KUrl    url     = note->content()->urlToOpen ( /*with=*/false );
	QString message = note->content()->messageWhenOpenning ( NoteContent::OpenOne /*NoteContent::OpenSeveral*/ );
	if ( url.isEmpty() )
	{
		if ( message.isEmpty() )
			emit postMessage ( i18n ( "Unable to open this note." ) /*"Unable to open those notes."*/ );
		else
		{
			int result = KMessageBox::warningContinueCancel ( this, message, /*caption=*/QString::null, KGuiItem ( i18n ( "&Edit" ), "edit" ) );
			if ( result == KMessageBox::Continue )
				noteEdit ( note );
		}
	}
	else
	{
		emit postMessage ( message ); // "Openning link target..." / "Launching application..." / "Openning note file..."
		// Finally do the opening job:
		QString customCommand = note->content()->customOpenCommand();
		if ( customCommand.isEmpty() )
		{
			KRun *run = new KRun ( url, this );
			run->setAutoDelete ( true );
		}
		else
			KRun::run ( customCommand, url, this );
	}
}

/** Code from bool KRun::displayOpenWithDialog(const KUrl::List& lst, bool tempFiles)
  * It does not allow to set a text, so I ripped it to do that:
  */
//BIG TODO

bool KRun__displayOpenWithDialog ( const KUrl::List& lst, bool tempFiles, const QString &text )
{
	//TODO if ( kapp && !KAuthorized::authorizeKAction ( "openwith" ) )
	if (0)
	{
		KMessageBox::sorry ( 0L, i18n ( "You are not authorized to open this file." ) ); // TODO: Better message, i18n freeze :-(
		return false;
	}
	KOpenWithDialog l ( lst, text, QString::null, 0L );
	if ( l.exec() )
	{
		KService::Ptr service = l.service();
		//TODO if ( !!service )
			//return KRun::run ( *service, lst, this, tempFiles );
		//kDebug(250) << "No service set, running " << l.text() << endl;
		//		return KRun::run ( l.text(), lst ); // TODO handle tempFiles
		
	}
	return false;
}

void Basket::noteOpenWith ( Note *note )
{
	if ( !note )
		note = theSelectedNote();
	if ( !note )
		return;

	KUrl    url     = note->content()->urlToOpen ( /*with=*/true );
	QString message = note->content()->messageWhenOpenning ( NoteContent::OpenOneWith /*NoteContent::OpenSeveralWith*/ );
	QString text    = note->content()->messageWhenOpenning ( NoteContent::OpenOneWithDialog /*NoteContent::OpenSeveralWithDialog*/ );
	if ( url.isEmpty() )
		emit postMessage ( i18n ( "Unable to open this note." ) /*"Unable to open those notes."*/ );
	else if ( KRun__displayOpenWithDialog ( url, false, text ) )
		emit postMessage ( message ); // "Openning link target with..." / "Openning note file with..."
}

void Basket::noteSaveAs()
{
//	if (!note)
//		note = theSelectedNote();
	Note *note = theSelectedNote();
	if ( !note )
		return;

	KUrl url = note->content()->urlToOpen ( /*with=*/false );
	if ( url.isEmpty() )
		return;

	QString fileName = KFileDialog::getSaveFileName ( url.fileName(), note->content()->saveAsFilters(), this, i18n ( "Save to File" ) );
	// TODO: Ask to overwrite !
	if ( fileName.isEmpty() )
		return;

	// TODO: Convert format, etc. (use NoteContent::saveAs(fileName))
	KIO::copy ( url, KUrl ( fileName ) );
}

Note* Basket::selectedGroup()
{
	FOR_EACH_NOTE ( note )
	{
		Note *selectedGroup = note->selectedGroup();
		if ( selectedGroup )
		{
			// If the selected group is one group in a column, then return that group, and not the column,
			// because the column is not ungrouppage, and the Ungroup action would be disabled.
			if ( selectedGroup->isColumn() && selectedGroup->firstChild() && !selectedGroup->firstChild()->next() )
			{
				return selectedGroup->firstChild();
			}
			return selectedGroup;
		}
	}
	return 0;
}

bool Basket::selectionIsOneGroup()
{
	return ( selectedGroup() != 0 );
}

Note* Basket::firstSelected()
{
	Note *first = 0;
	FOR_EACH_NOTE ( note )
	{
		first = note->firstSelected();
		if ( first )
			return first;
	}
	return 0;
}

Note* Basket::lastSelected()
{
	Note *last = 0, *tmp = 0;
	FOR_EACH_NOTE ( note )
	{
		tmp = note->lastSelected();
		if ( tmp )
			last = tmp;
	}
	return last;
}

bool Basket::convertTexts()
{
	m_watcher->stopScan();
	bool convertedNotes = false;

	if ( !isLoaded() )
		load();

	FOR_EACH_NOTE ( note )
	if ( note->convertTexts() )
		convertedNotes = true;

	if ( convertedNotes )
		save();
	m_watcher->startScan();
	return convertedNotes;
}

void Basket::noteGroup()
{
	/*	// Nothing to do?
		if (isLocked() || countSelecteds() <= 1)
			return;

		// If every selected notes are ALREADY in one group, then don't touch anything:
		Note *selectedGroup = this->selectedGroup();
		if (selectedGroup && !selectedGroup->isColumn())
			return;
	*/

	// Copied from BNPView::updateNotesActions()
	bool severalSelected = countSelecteds() >= 2;
	Note *selectedGroup = ( severalSelected ? this->selectedGroup() : 0 );
	bool enabled = !isLocked() && severalSelected && ( !selectedGroup || selectedGroup->isColumn() );
	if ( !enabled )
		return;

	// Get the first selected note: we will group selected items just before:
	Note *first = firstSelected();
//	if (selectedGroup != 0 || first == 0)
//		return;

	m_loaded = false; // Hack to avoid notes to be unselected and new notes to be selected:

	// Create and insert the receiving group:
	Note *group = new Note ( this );
	if ( first->isFree() )
	{
		insertNote ( group, 0L, Note::BottomColumn, QPoint ( first->finalX(), first->finalY() ), /*animateNewPosition=*/false );
	}
	else
	{
		insertNote ( group, first, Note::TopInsert, QPoint(), /*animateNewPosition=*/false );
	}

	// Put a FAKE UNSELECTED note in the new group, so if the new group is inside an allSelected() group, the parent group is not moved inside the new group!
	Note *fakeNote = NoteFactory::createNoteColor ( Qt::red, this );
	insertNote ( fakeNote, group, Note::BottomColumn, QPoint(), /*animateNewPosition=*/false );

	// Group the notes:
	Note *nextNote;
	Note *note = firstNote();
	while ( note )
	{
		nextNote = note->next();
		note->groupIn ( group );
		note = nextNote;
	}

	m_loaded = true; // Part 2 / 2 of the workarround!

	// Do cleanup:
	unplugNote ( fakeNote );
	unselectAll();
	group->setSelectedRecursivly ( true ); // Notes were unselected by unplugging

	relayoutNotes ( true );
	save();
}

void Basket::noteUngroup()
{
	Note *group = selectedGroup();
	if ( group && !group->isColumn() )
		ungroupNote ( group );
	save();
}

void Basket::unplugSelection ( NoteSelection *selection )
{
	for ( NoteSelection *toUnplug = selection->firstStacked(); toUnplug; toUnplug = toUnplug->nextStacked() )
		unplugNote ( toUnplug->note );
}

void Basket::insertSelection ( NoteSelection *selection, Note *after )
{
	for ( NoteSelection *toUnplug = selection->firstStacked(); toUnplug; toUnplug = toUnplug->nextStacked() )
	{
		if ( toUnplug->note->isGroup() )
		{
			Note *group = new Note ( this );
			insertNote ( group, after, Note::BottomInsert, QPoint(), /*animateNewPosition=*/false );
			Note *fakeNote = NoteFactory::createNoteColor ( Qt::red, this );
			insertNote ( fakeNote, group, Note::BottomColumn, QPoint(), /*animateNewPosition=*/false );
			insertSelection ( toUnplug->firstChild, fakeNote );
			unplugNote ( fakeNote );
			after = group;
		}
		else
		{
			Note *note = toUnplug->note;
			note->setPrev ( 0 );
			note->setNext ( 0 );
			insertNote ( note, after, Note::BottomInsert, QPoint(), /*animateNewPosition=*/true );
			after = note;
		}
	}
}

void Basket::selectSelection ( NoteSelection *selection )
{
	for ( NoteSelection *toUnplug = selection->firstStacked(); toUnplug; toUnplug = toUnplug->nextStacked() )
	{
		if ( toUnplug->note->isGroup() )
			selectSelection ( toUnplug );
		else
			toUnplug->note->setSelected ( true );
	}
}

void Basket::noteMoveOnTop()
{
	// TODO: Get the group containing the selected notes and first move inside the group, then inside parent group, then in the basket
	// TODO: Move on top/bottom... of the column or basjet

	NoteSelection *selection = selectedNotes();
	unplugSelection ( selection );
	// Replug the notes:
	Note *fakeNote = NoteFactory::createNoteColor ( Qt::red, this );
	if ( isColumnsLayout() )
	{
		if ( firstNote()->firstChild() )
			insertNote ( fakeNote, firstNote()->firstChild(), Note::TopInsert, QPoint(), /*animateNewPosition=*/false );
		else
			insertNote ( fakeNote, firstNote(), Note::BottomColumn, QPoint(), /*animateNewPosition=*/false );
	}
	else
	{
		// TODO: Also allow to move notes on top of a group!!!!!!!
		insertNote ( fakeNote, 0, Note::BottomInsert, QPoint ( 0, 0 ), /*animateNewPosition=*/false );
	}
	insertSelection ( selection, fakeNote );
	unplugNote ( fakeNote );
	selectSelection ( selection );
	relayoutNotes ( true );
	save();
}

void Basket::noteMoveOnBottom()
{

	// TODO: Duplicate code: void noteMoveOn();

	// TODO: Get the group containing the selected notes and first move inside the group, then inside parent group, then in the basket
	// TODO: Move on top/bottom... of the column or basjet

	NoteSelection *selection = selectedNotes();
	unplugSelection ( selection );
	// Replug the notes:
	Note *fakeNote = NoteFactory::createNoteColor ( Qt::red, this );
	if ( isColumnsLayout() )
		insertNote ( fakeNote, firstNote(), Note::BottomColumn, QPoint(), /*animateNewPosition=*/false );
	else
	{
		// TODO: Also allow to move notes on top of a group!!!!!!!
		insertNote ( fakeNote, 0, Note::BottomInsert, QPoint ( 0, 0 ), /*animateNewPosition=*/false );
	}
	insertSelection ( selection, fakeNote );
	unplugNote ( fakeNote );
	selectSelection ( selection );
	relayoutNotes ( true );
	save();
}

void Basket::moveSelectionTo ( Note *here, bool below/* = true*/ )
{
	NoteSelection *selection = selectedNotes();
	unplugSelection ( selection );
	// Replug the notes:
	Note *fakeNote = NoteFactory::createNoteColor ( Qt::red, this );
//	if (isColumnsLayout())
	insertNote ( fakeNote, here, ( below ? Note::BottomInsert : Note::TopInsert ), QPoint(), /*animateNewPosition=*/false );
//	else {
//		// TODO: Also allow to move notes on top of a group!!!!!!!
//		insertNote(fakeNote, 0, Note::BottomInsert, QPoint(0, 0), /*animateNewPosition=*/false);
//	}
	insertSelection ( selection, fakeNote );
	unplugNote ( fakeNote );
	selectSelection ( selection );
	relayoutNotes ( true );
	save();
}

void Basket::noteMoveNoteUp()
{

	// TODO: Move between columns, even if they are empty !!!!!!!

	// TODO: if first note of a group, move just above the group! And let that even if there is no note before that group!!!

	Note *first    = firstSelected();
	Note *previous = first->prevShownInStack();
	if ( previous )
		moveSelectionTo ( previous, /*below=*/false );
}

void Basket::noteMoveNoteDown()
{
	Note *first = lastSelected();
	Note *next  = first->nextShownInStack();
	if ( next )
		moveSelectionTo ( next, /*below=*/true );
}

void Basket::wheelEvent ( QWheelEvent *event )
{
	QScrollArea::wheelEvent ( event );
}

void Basket::linkLookChanged()
{
	Note *note = m_firstNote;
	while ( note )
	{
		note->linkLookChanged();
		note = note->next();
	}
	relayoutNotes ( true );
}

void Basket::slotCopyingDone2 ( KIO::Job *job )
{
	if ( job->error() )
	{
		DEBUG_WIN << "Copy finished, ERROR";
		return;
	}
	KIO::FileCopyJob *fileCopyJob = ( KIO::FileCopyJob* ) job;
	Note *note = noteForFullPath ( fileCopyJob->destUrl().path() );
	DEBUG_WIN << "Copy finished, load note: " + fileCopyJob->destUrl().path() + ( note ? "" : " --- NO CORRESPONDING NOTE" );
	if ( note != 0L )
	{
		note->content()->loadFromFile ( /*lazyLoad=*/false );
		if ( isEncrypted() )
			note->content()->saveToFile();
		if ( m_focusedNote == note ) // When inserting a new note we ensure it visble
			ensureNoteVisible ( note ); //  But after loading it has certainly grown and if it was
	}                                //  on bottom of the basket it's not visible entirly anymore
}

Note* Basket::noteForFullPath ( const QString &path )
{
	Note *note = firstNote();
	Note *found;
	while ( note )
	{
		found = note->noteForFullPath ( path );
		if ( found )
			return found;
		note = note->next();
	}
	return 0;
}

void Basket::deleteFiles()
{
	m_watcher->stopScan();
	Tools::deleteRecursively ( fullPath() );
}

QList<State*> Basket::usedStates()
{
	QList<State*> states;
	FOR_EACH_NOTE ( note )
	note->usedStates ( states );
	return states;
}

QString Basket::saveGradientBackground ( const QColor &color, const QFont &font, const QString &folder )
{
	// Construct file name and return if the file already exists:
	QString fileName = "note_background_" + color.name().toLower().mid ( 1 ) + ".png";
	QString fullPath = folder + fileName;
	if ( QFile::exists ( fullPath ) )
		return fileName;

	// Get the gradient top and bottom colors:
	QColor topBgColor;
	QColor bottomBgColor;
	Note::getGradientColors ( color, &topBgColor, &bottomBgColor );

	// Draw and save the gradient image:
	int sampleTextHeight = QFontMetrics ( font )
	                       .boundingRect ( 0, 0, /*width=*/10000, /*height=*/0, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, "Test text" )
	                       .height();
	QPixmap noteGradient ( 100, sampleTextHeight + Note::NOTE_MARGIN );
	QPainter painter ( &noteGradient );
	drawGradient ( &painter, topBgColor, bottomBgColor, 0, 0, noteGradient.width(), noteGradient.height(), /*sunken=*/false, /*horz=*/true, /*flat=*/false );
	painter.end();
	noteGradient.save ( fullPath, "PNG" );

	// Return the name of the created file:
	return fileName;
}

void Basket::listUsedTags ( QList<Tag*> &list )
{
	if ( !isLoaded() )
	{
		load();
	}

	FOR_EACH_NOTE ( child )
	child->listUsedTags ( list );
}


/** Unfocus the previously focused note (unless it was null)
  * and focus the new @param note (unless it is null) if hasFocus()
  * Update m_focusedNote to the new one
  */
void Basket::setFocusedNote ( Note *note ) // void Basket::changeFocusTo(Note *note)
{
	// Don't focus an hidden note:
	if ( note != 0L && !note->isVisible() )
		return;
	// When clicking a group, this group gets focused. But only content-based notes should be focused:
	if ( note && note->isGroup() )
		note = note->firstRealChild();
	// The first time a note is focused, it becomes the start of the Shift selection:
	if ( m_startOfShiftSelectionNote == 0 )
		m_startOfShiftSelectionNote = note;
	// Unfocus the old focused note:
	if ( m_focusedNote != 0L )
		m_focusedNote->setFocused ( false );
	// Notify the new one to draw a focus rectangle... only if the basket is focused:
	if ( hasFocus() && note != 0L )
		note->setFocused ( true );
	// Save the new focused note:
	m_focusedNote = note;
}

/** If no shown note is currently focused, try to find a shown note and focus it
  * Also update m_focusedNote to the new one (or null if there isn't)
  */
void Basket::focusANote()
{
	if ( countFounds() == 0 )
	{ // No note to focus
		setFocusedNote ( 0L );
//		m_startOfShiftSelectionNote = 0;
		return;
	}

	if ( m_focusedNote == 0L )
	{ // No focused note yet : focus the first shown
		Note *toFocus = ( isFreeLayout() ? noteOnHome() : firstNoteShownInStack() );
		setFocusedNote ( toFocus );
//		m_startOfShiftSelectionNote = m_focusedNote;
		return;
	}

	// Search a visible note to focus if the focused one isn't shown :
	Note *toFocus = m_focusedNote;
	if ( toFocus && !toFocus->isVisible() )
		toFocus = toFocus->nextShownInStack();
	if ( !toFocus && m_focusedNote )
		toFocus = m_focusedNote->prevShownInStack();
	setFocusedNote ( toFocus );
//	m_startOfShiftSelectionNote = toFocus;
}

Note* Basket::firstNoteInStack()
{
	if ( !firstNote() )
		return 0;

	if ( firstNote()->content() )
		return firstNote();
	else
		return firstNote()->nextInStack();
}

Note* Basket::lastNoteInStack()
{
	Note *note = lastNote();
	while ( note )
	{
		if ( note->content() )
			return note;
		Note *possibleNote = note->lastRealChild();
		if ( possibleNote && possibleNote->content() )
			return possibleNote;
		note = note->prev();
	}
	return 0;
}

Note* Basket::firstNoteShownInStack()
{
	Note *first = firstNoteInStack();
	while ( first && !first->isVisible() )
		first = first->nextInStack();
	return first;
}

Note* Basket::lastNoteShownInStack()
{
	Note *last = lastNoteInStack();
	while ( last && !last->isVisible() )
		last = last->prevInStack();
	return last;
}

inline int abs ( int n )
{
	return ( n < 0 ? -n : n );
}

Note* Basket::noteOn ( NoteOn side )
{
	Note *bestNote = 0;
	int   distance = -1;
	int   bestDistance = widget()->width() * widget()->height() * 10;

	Note *note    = firstNoteShownInStack();
	Note *primary = m_focusedNote->parentPrimaryNote();
	while ( note )
	{
		switch ( side )
		{
			case LEFT_SIDE:   distance = m_focusedNote->distanceOnLeftRight ( note, LEFT_SIDE );   break;
			case RIGHT_SIDE:  distance = m_focusedNote->distanceOnLeftRight ( note, RIGHT_SIDE );  break;
			case TOP_SIDE:    distance = m_focusedNote->distanceOnTopBottom ( note, TOP_SIDE );    break;
			case BOTTOM_SIDE: distance = m_focusedNote->distanceOnTopBottom ( note, BOTTOM_SIDE ); break;
		}
		if ( ( side == TOP_SIDE || side == BOTTOM_SIDE || primary != note->parentPrimaryNote() ) && note != m_focusedNote && distance > 0 && distance < bestDistance )
		{
			bestNote     = note;
			bestDistance = distance;
		}
		note = note ->nextShownInStack();
	}

	return bestNote;
}

Note* Basket::firstNoteInGroup()
{
	Note *child  = m_focusedNote;
	Note *parent = ( m_focusedNote ? m_focusedNote->parentNote() : 0 );
	while ( parent )
	{
		if ( parent->firstChild() != child && !parent->isColumn() )
			return parent->firstRealChild();
		child  = parent;
		parent = parent->parentNote();
	}
	return 0;
}

Note* Basket::noteOnHome()
{
	// First try to find the first note of the group containing the focused note:
	Note *child  = m_focusedNote;
	Note *parent = ( m_focusedNote ? m_focusedNote->parentNote() : 0 );
	while ( parent )
	{
		if ( parent->nextShownInStack() != m_focusedNote )
			return parent->nextShownInStack();
		child  = parent;
		parent = parent->parentNote();
	}

	// If it was not found, then focus the very first note in the basket:
	if ( isFreeLayout() )
	{
		Note *first = firstNoteShownInStack(); // The effective first note found
		Note *note  = first; // The current note, to conpare with the previous first note, if this new note is more on top
		if ( note )
			note = note->nextShownInStack();
		while ( note )
		{
			if ( note->finalY() < first->finalY() || ( note->finalY() == first->finalY() && note->finalX() < first->finalX() ) )
				first = note;
			note = note->nextShownInStack();
		}
		return first;
	}
	else
		return firstNoteShownInStack();
}

Note* Basket::noteOnEnd()
{
	Note *child     = m_focusedNote;
	Note *parent    = ( m_focusedNote ? m_focusedNote->parentNote() : 0 );
	Note *lastChild;
	while ( parent )
	{
		lastChild = parent->lastRealChild();
		if ( lastChild && lastChild != m_focusedNote )
		{
			if ( lastChild->isVisible() )
				return lastChild;
			lastChild = lastChild->prevShownInStack();
			if ( lastChild && lastChild->isVisible() && lastChild != m_focusedNote )
				return lastChild;
		}
		child  = parent;
		parent = parent->parentNote();
	}
	if ( isFreeLayout() )
	{
		Note *last;
		Note *note;
		last = note = firstNoteShownInStack();
		note = note->nextShownInStack();
		while ( note )
		{
			if ( note->finalBottom() > last->finalBottom() || ( note->finalBottom() == last->finalBottom() && note->finalX() > last->finalX() ) )
				last = note;
			note = note->nextShownInStack();
		}
		return last;
	}
	else
		return lastNoteShownInStack();
}


void Basket::keyPressEvent ( QKeyEvent *event )
{
	if ( isDuringEdit() && event->key() == Qt::Key_Return )
	{
		//if (m_editor->lineEdit())
		//	closeEditor();
		//else
		m_editor->widget()->setFocus();
	}
	else if ( event->key() == Qt::Key_Escape )
	{
		if ( isDuringEdit() )
			closeEditor();
		else if ( decoration()->filterData().isFiltering )
			cancelFilter();
		else
			unselectAll();
	}

	if ( countFounds() == 0 )
		return;

	if ( !m_focusedNote )
		return;

	Note *toFocus = 0L;

	switch ( event->key() )
	{
		case Qt::Key_Down:
			toFocus = ( isFreeLayout() ? noteOn ( BOTTOM_SIDE ) : m_focusedNote->nextShownInStack() );
			if ( toFocus )
				break;
			scrollContentsBy ( 0, 30 ); // This cases do not move focus to another note...
			return;
		case Qt::Key_Up:
			toFocus = ( isFreeLayout() ? noteOn ( TOP_SIDE ) : m_focusedNote->prevShownInStack() );
			if ( toFocus )
				break;
			scrollContentsBy ( 0, -30 ); // This cases do not move focus to another note...
			return;
		case Qt::Key_PageDown:
			if ( isFreeLayout() )
			{
				Note *lastFocused = m_focusedNote;
				for ( int i = 0; i < 10 && m_focusedNote; ++i )
					m_focusedNote = noteOn ( BOTTOM_SIDE );
				toFocus = m_focusedNote;
				m_focusedNote = lastFocused;
			}
			else
			{
				toFocus = m_focusedNote;
				for ( int i = 0; i < 10 && toFocus; ++i )
					toFocus = toFocus->nextShownInStack();
			}
			if ( toFocus == 0L )
				toFocus = ( isFreeLayout() ? noteOnEnd() : lastNoteShownInStack() );
			if ( toFocus && toFocus != m_focusedNote )
				break;
			scrollContentsBy ( 0, height() / 2 ); // This cases do not move focus to another note...
			return;
		case Qt::Key_PageUp:
			if ( isFreeLayout() )
			{
				Note *lastFocused = m_focusedNote;
				for ( int i = 0; i < 10 && m_focusedNote; ++i )
					m_focusedNote = noteOn ( TOP_SIDE );
				toFocus = m_focusedNote;
				m_focusedNote = lastFocused;
			}
			else
			{
				toFocus = m_focusedNote;
				for ( int i = 0; i < 10 && toFocus; ++i )
					toFocus = toFocus->prevShownInStack();
			}
			if ( toFocus == 0L )
				toFocus = ( isFreeLayout() ? noteOnHome() : firstNoteShownInStack() );
			if ( toFocus && toFocus != m_focusedNote )
				break;
			scrollContentsBy ( 0, - height() / 2 ); // This cases do not move focus to another note...
			return;
		case Qt::Key_Home:
			toFocus = noteOnHome();
			break;
		case Qt::Key_End:
			toFocus = noteOnEnd();
			break;
		case Qt::Key_Left:
			if ( m_focusedNote->tryFoldParent() )
				return;
			if ( ( toFocus = noteOn ( LEFT_SIDE ) ) )
				break;
			if ( ( toFocus = firstNoteInGroup() ) )
				break;
			scrollContentsBy ( -30, 0 ); // This cases do not move focus to another note...
			return;
		case Qt::Key_Right:
			if ( m_focusedNote->tryExpandParent() )
				return;
			if ( ( toFocus = noteOn ( RIGHT_SIDE ) ) )
				break;
			scrollContentsBy ( 30, 0 ); // This cases do not move focus to another note...
			return;
		case Qt::Key_Space:  // This case do not move focus to another note...
			if ( m_focusedNote )
			{
				m_focusedNote->setSelected ( ! m_focusedNote->isSelected() );
				event->accept();
			}
			else
				event->ignore();
			return;          // ... so we return after the process
		default:
			return;
	}

	if ( toFocus == 0L )
	{ // If no direction keys have been pressed OR reached out the begin or end
		event->ignore(); // Important !!
		return;
	}

	//	if ( QInputEvent::modifiers() & Qt::ShiftModifier )
	if ( (event)->modifiers() & Qt::ShiftModifier )
	{ // Shift+arrowKeys selection
		if ( m_startOfShiftSelectionNote == 0L )
			m_startOfShiftSelectionNote = toFocus;
		ensureNoteVisible ( toFocus ); // Important: this line should be before the other ones because else repaint would be done on the wrong part!
		selectRange ( m_startOfShiftSelectionNote, toFocus );
		setFocusedNote ( toFocus );
		event->accept();
		return;
	}
	else /*if (toFocus != m_focusedNote)*/
	{  // Move focus to ANOTHER note...
		ensureNoteVisible ( toFocus ); // Important: this line should be before the other ones because else repaint would be done on the wrong part!
		setFocusedNote ( toFocus );
		m_startOfShiftSelectionNote = toFocus;
		if ( ! ((event)->modifiers() & Qt::ControlModifier ) )     // ... select only current note if Control
			unselectAllBut ( m_focusedNote );
		event->accept();
		return;
	}

	event->ignore(); // Important !!
}

/** Select a range of notes and deselect the others.
  * The order between start and end has no importance (end could be before start)
  */
void Basket::selectRange ( Note *start, Note *end, bool unselectOthers /*= true*/ )
{
	Note *cur;
	Note *realEnd = 0L;

	// Avoid crash when start (or end) is null
	if ( start == 0L )
		start = end;
	else if ( end == 0L )
		end = start;
	// And if *both* are null
	if ( start == 0L )
	{
		if ( unselectOthers )
			unselectAll();
		return;
	}
	// In case there is only one note to select
	if ( start == end )
	{
		if ( unselectOthers )
			unselectAllBut ( start );
		else
			start->setSelected ( true );
		return;
	}

	// Free layout baskets should select range as if we were drawing a rectangle between start and end:
	if ( isFreeLayout() )
	{
		QRect startRect ( start->finalX(), start->finalY(), start->width(), start->finalHeight() );
		QRect endRect ( end->finalX(),   end->finalY(),   end->width(),   end->finalHeight() );
		QRect toSelect = startRect.unite ( endRect );
		selectNotesIn ( toSelect, /*invertSelection=*/false, unselectOthers );
		return;
	}

	// Search the REAL first (and deselect the others before it) :
	for ( cur = firstNoteInStack(); cur != 0L; cur = cur->nextInStack() )
	{
		if ( cur == start || cur == end )
			break;
		if ( unselectOthers )
			cur->setSelected ( false );
	}

	// Select the notes after REAL start, until REAL end :
	if ( cur == start )
		realEnd = end;
	else if ( cur == end )
		realEnd = start;

	for ( /*cur = cur*/; cur != 0L; cur = cur->nextInStack() )
	{
		cur->setSelected ( cur->isVisible() ); // Select all notes in the range, but only if they are shown
		if ( cur == realEnd )
			break;
	}

	if ( !unselectOthers )
		return;

	// Deselect the remaining notes :
	if ( cur )
		cur = cur->nextInStack();
	for ( /*cur = cur*/; cur != 0L; cur = cur->nextInStack() )
		cur->setSelected ( false );
}

void Basket::focusInEvent ( QFocusEvent* )
{
	// Focus cannot be get with Tab when locked, but a click can focus the basket!
	if ( isLocked() )
	{
		if ( m_button )
			QTimer::singleShot ( 0, m_button, SLOT ( setFocus() ) );
	}
	else
		focusANote();      // hasFocus() is true at this stage, note will be focused
}

void Basket::focusOutEvent ( QFocusEvent* )
{
	if ( m_focusedNote != 0L )
		m_focusedNote->setFocused ( false );
}

void Basket::ensureNoteVisible ( Note *note )
{
	if ( !note->isVisible() ) // Logical!
		return;

	if ( note == editedNote() ) // HACK: When filtering while editing big notes, etc... cause unwanted scrolls
		return;

	int finalBottom = note->finalY() + qMin ( note->finalHeight(),                                             height() );
	int finalRight  = note->finalX() + qMin ( note->width() + ( note->hasResizer() ? Note::RESIZER_WIDTH : 0 ),  width() );
	ensureVisible ( finalRight,     finalBottom,    0,0 );
	ensureVisible ( note->finalX(), note->finalY(), 0,0 );
}

void Basket::addWatchedFile ( const QString &fullPath )
{
//	DEBUG_WIN << "Watcher>Add Monitoring Of : <font color=blue>" + fullPath + "</font>";
	m_watcher->addFile ( fullPath );
}

void Basket::removeWatchedFile ( const QString &fullPath )
{
//	DEBUG_WIN << "Watcher>Remove Monitoring Of : <font color=blue>" + fullPath + "</font>";
	m_watcher->removeFile ( fullPath );
}

void Basket::watchedFileModified ( const QString &fullPath )
{
	if ( !m_modifiedFiles.contains ( fullPath ) )
		m_modifiedFiles.append ( fullPath );
	// If a big file is saved by an application, notifications are send several times.
	// We wait they are not sent anymore to considere the file complete!
	m_watcherTimer.setSingleShot(true);
	m_watcherTimer.start ( 200/*ms*/ );
	DEBUG_WIN << "Watcher>Modified : <font color=blue>" + fullPath + "</font>";
}

void Basket::watchedFileDeleted ( const QString &fullPath )
{
	Note *note = noteForFullPath ( fullPath );
	removeWatchedFile ( fullPath );
	if ( note )
	{
		NoteSelection *selection = selectedNotes();
		unselectAllBut ( note );
		noteDeleteWithoutConfirmation();
		while ( selection )
		{
			selection->note->setSelected ( true );
			selection = selection->nextStacked();
		}
	}
	DEBUG_WIN << "Watcher>Removed : <font color=blue>" + fullPath + "</font>";
}

void Basket::updateModifiedNotes()
{
	for ( QList<QString>::iterator it = m_modifiedFiles.begin(); it != m_modifiedFiles.end(); ++it )
	{
		Note *note = noteForFullPath ( *it );
		if ( note )
			note->content()->loadFromFile ( /*lazyLoad=*/false );
	}
	m_modifiedFiles.clear();
}

bool Basket::setProtection ( int type, QString key )
{
#ifdef HAVE_LIBGPGME
	if ( type == PasswordEncryption || // Ask a new password
	        m_encryptionType != type || m_encryptionKey != key )
	{
		int savedType = m_encryptionType;
		QString savedKey = m_encryptionKey;

		m_encryptionType = type;
		m_encryptionKey = key;
		m_gpg->clearCache();

		if ( saveAgain() )
		{
			emit propertiesChanged ( this );
		}
		else
		{
			m_encryptionType = savedType;
			m_encryptionKey = savedKey;
			m_gpg->clearCache();
			return false;
		}
	}
	return true;
#else
	m_encryptionType = type;
	m_encryptionKey = key;
	return false;
#endif
}

bool Basket::saveAgain()
{
	bool result = false;

	m_watcher->stopScan();
	// Re-encrypt basket file:
	result = save();
	// Re-encrypt every note files recursively:
	if ( result )
	{
		FOR_EACH_NOTE ( note )
		{
			result = note->saveAgain();
			if ( !result )
				break;
		}
	}
	m_watcher->startScan();
	return result;
}

bool Basket::loadFromFile ( const QString &fullPath, QString *string, bool isLocalEncoding )
{
	QByteArray array;

	if ( loadFromFile ( fullPath, &array ) )
	{
		if ( isLocalEncoding )
			*string = QString::fromLocal8Bit ( array.data(), array.size() );
		else
			*string = QString::fromUtf8 ( array.data(), array.size() );
		return true;
	}
	else
		return false;
}

bool Basket::isEncrypted()
{
	return ( m_encryptionType != NoEncryption );
}

bool Basket::isFileEncrypted()
{
	QFile file ( fullPath() + ".basket" );

	if ( file.open ( QIODevice::WriteOnly ) )
	{
		QString line;

		line=file.readLine (32);
		if ( line.startsWith ( "-----BEGIN PGP MESSAGE-----" ) )
			return true;
	}
	return false;
}

bool Basket::loadFromFile ( const QString &fullPath, QByteArray *array )
{
	QFile file ( fullPath );
	bool encrypted = false;

	if ( file.open ( QIODevice::WriteOnly ) )
	{
		*array = file.readAll();
		const char* magic = "-----BEGIN PGP MESSAGE-----";
		uint i = 0;

		if ( array->size() > strlen ( magic ) )
			for ( i = 0; array->at ( i ) == magic[i]; ++i )
				;
		if ( i == strlen ( magic ) )
		{
			encrypted = true;
		}
		file.close();
#ifdef HAVE_LIBGPGME
		if ( encrypted )
		{
			QByteArray tmp ( *array );

			tmp.detach();
			// Only use gpg-agent for private key encryption since it doesn't
			// cache password used in symmetric encryption.
			m_gpg->setUseGnuPGAgent ( Settings::useGnuPGAgent() && m_encryptionType == PrivateKeyEncryption );
			if ( m_encryptionType == PrivateKeyEncryption )
				m_gpg->setText ( i18n ( "Please enter the password for the following private key:" ), false );
			else
				m_gpg->setText ( i18n ( "Please enter the password for the basket <b>%1</b>:" ).arg ( basketName() ), false ); // Used when decrypting
			return m_gpg->decrypt ( tmp, array );
		}
#else
		if ( encrypted )
		{
			return false;
		}
#endif
		return true;
	}
	else
		return false;
}

bool Basket::saveToFile ( const QString& fullPath, const QString& string, bool isLocalEncoding )
{
	QByteArray bytes = ( isLocalEncoding ? string.toLocal8Bit() : string.toUtf8() );
	return saveToFile ( fullPath, bytes, bytes.length() );
}

bool Basket::saveToFile ( const QString& fullPath, const QByteArray& array )
{
	return saveToFile ( fullPath, array, array.size() );
}

bool Basket::saveToFile ( const QString& fullPath, const QByteArray& array, qulonglong length )
{
	bool success = true;
	QByteArray tmp;

#ifdef HAVE_LIBGPGME
	if ( isEncrypted() )
	{
		QString key = QString::null;

		// We only use gpg-agent for private key encryption and saving without
		// public key doesn't need one.
		m_gpg->setUseGnuPGAgent ( false );
		if ( m_encryptionType == PrivateKeyEncryption )
		{
			key = m_encryptionKey;
			// public key doesn't need password
			m_gpg->setText ( "", false );
		}
		else
			m_gpg->setText ( i18n ( "Please assign a password to the basket <b>%1</b>:" ).arg ( basketName() ), true ); // Used when defining a new password

		success = m_gpg->encrypt ( array, length, &tmp, key );
		length = tmp.size();
	}
	else
		tmp = array;

#else
	success = !isEncrypted();
	if ( success )
		tmp = array;
#endif
	/*if (success && (success = file.open(QIODevice::WriteOnly))){
		success = (file.write(tmp) == (Q_LONG)tmp.size());
		file.close();
	}*/

	if ( success )
		return safelySaveToFile ( fullPath, tmp, length );
	else
		return false;
}

/** Same as saveToFile(), but it is static, and does not crypt the data if needed.
  * Basically, to save a file owned by a basket (a basket or a note file), use saveToFile().
  * But to save another file (eg. the basket hierarchy), use this safelySaveToFile() static method.
  */
/*static*/ bool Basket::safelySaveToFile ( const QString& fullPath, const QByteArray& array, qulonglong length )
{
	// Here, we take a double protection:
	// - We use KSaveFile to write atomically to the file (either it's a success or the file is untouched)
	// - We show a modal dialog to the user when no disk space is left or access is denied and retry every couple of seconds

	// Static, because safelySaveToFile() can be called a second time while blocked.
	// Example:
	// User type something and press Enter: safelySaveToFile() is called and block.
	// Three seconds later, a timer ask to save changes, and this second safelySaveToFile() block too.
	// Do not show the dialog twice in this case!
	static DiskErrorDialog *dialog = 0;

	//std::cout << "---------- Saving " << fullPath << ":" << std::endl;
	bool openSuccess;
	bool closeSuccess;
	bool errorWhileWritting;
	do
	{
		KSaveFile saveFile ( fullPath );
		//std::cout << "==>>" << std::endl << "SAVE FILE CREATED: " << strerror(saveFile.status()) << std::endl;
		openSuccess = ( saveFile.error() ==  QFile::NoError );
		if ( openSuccess )
		{
			saveFile.write ( array, length );
			//std::cout << "FILE WRITTEN: " << strerror(saveFile.status()) << std::endl;
			closeSuccess = saveFile.finalize();
			//std::cout << "FILE CLOSED: " << (closeSuccess ? "well" : "erroneous") << std::endl;
		}
		errorWhileWritting = ( !openSuccess || !closeSuccess || saveFile.error() != QFile::NoError );
		if ( errorWhileWritting )
		{
			//std::cout << "ERROR DETECTED" << std::endl;
			if ( dialog == 0 )
			{
				//std::cout << "Opening dialog for " << fullPath << std::endl;
				dialog = new DiskErrorDialog (
				             ( openSuccess
				               ? i18n ( "Insufficient Disk Space to Save Basket Data" )
				               : i18n ( "Wrong Basket File Permissions" )
				             ),
				             ( openSuccess
				               ? i18n ( "Please remove files on the disk <b>%1</b> to let the application safely save your changes." )
					       .arg( KMountPoint::currentMountPoints().findByPath( fullPath ))
				               : i18n ( "File permissions are bad for <b>%1</b>. Please check that you have write access to it and the parent folders." )
				               .arg ( fullPath )
				             ),
				             kapp->activeWindow()
				         );
			}
			if ( !dialog->isVisible() )
				dialog->show();
			const int retryDelay = 1000/*ms*/;
			const int sleepDelay = 50/*ms*/;
			for ( int i = 0; i < retryDelay / sleepDelay; ++i )
			{
				kapp->processEvents();
				usleep ( sleepDelay );
			}
		}
	}
	while ( errorWhileWritting );
	if ( dialog )
	{
		delete dialog;
		dialog = 0;
	}

	return true; // Hum...?!
}

/*static*/ bool Basket::safelySaveToFile ( const QString& fullPath, const QString& string, bool isLocalEncoding )
{
	QByteArray bytes = ( isLocalEncoding ? string.toLocal8Bit() : string.toUtf8() );
	return safelySaveToFile ( fullPath, bytes, bytes.length() - 1 );
}

/*static*/ bool Basket::safelySaveToFile ( const QString& fullPath, const QByteArray& array )
{
	return safelySaveToFile ( fullPath, array, array.size() );
}

DiskErrorDialog::DiskErrorDialog ( const QString &titleMessage, const QString &message, QWidget *parent )
// 		: KDialog ( ( "Save Error" ),
// 		            ( KDialog::ButtonCode ) 0, ( KDialog::ButtonCode ) 0, parent, /*name=*/"DiskError" )



{
KDialog dlg( parent );
dlg.setCaption( i18n("Save error") );
dlg.setButtons( Ok );
dlg.setDefaultButton( Ok );

	//enableButtonCancel(false);
	//enableButtonClose(false);
	//enableButton(Close, false);
	//enableButtonOk(false);
	setModal ( true );
	QHBoxLayout *layout = new QHBoxLayout;// ( plainPage() );
	                      layout->setMargin ( 0 );
	layout->setSpacing ( spacingHint() );
	QPixmap icon = KIconLoader::global()->loadIcon ( "hdd_unmount", KIconLoader::NoGroup, 64, KIconLoader::DefaultState,QStringList(), /*path_store=*/0L, /*canReturnNull=*/true );
	QLabel *iconLabel  = new QLabel;// ( plainPage() ); TODO not sure if the right way
	iconLabel->setPixmap ( icon );
	iconLabel->setFixedSize ( iconLabel->sizeHint() );
	QLabel *label = new QLabel ( "<p><nobr><b><font size='+1'>" + titleMessage + "</font></b></nobr></p><p>" + message + "</p>",0,0);//TOdo not sure, plainPage() );
	if ( !icon.isNull() )
		layout->addWidget ( iconLabel );
	layout->addWidget ( label );
}

DiskErrorDialog::~DiskErrorDialog()
{}

void DiskErrorDialog::closeEvent ( QCloseEvent *event )
{
	event->ignore();
}

void DiskErrorDialog::keyPressEvent ( QKeyEvent* )
{
	// Escape should not close the window...
}

void Basket::lock ()
{
#ifdef HAVE_LIBGPGME
	closeEditor();
	m_gpg->clearCache();
	m_locked = true;
	enableActions();
	deleteNotes();
	m_loaded = false;
	m_loadingLaunched = false;
	updateContents();
#endif
}

#if 0

#include <qlayout.h>
#include <qvbox.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qcolor.h>
#include <kmenu.h>
#include <kurllabel.h>
#include <qcheckbox.h>
#include <qpalette.h>
#include <qcursor.h>
#include <qaction.h>
#include <kstdaccel.h>
#include <kglobalsettings.h>
#include <qevent.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <qinputdialog.h>
#include <qdragobject.h>
#include <kurldrag.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kfiledialog.h>
#include <qdir.h>
#include <kiconloader.h>
#include <qregexp.h>
#include <qfileinfo.h>

#include <qstringlist.h>
#include <qdir.h>
#include <kurl.h>
#include <krun.h>
#include <kmessagebox.h>
#include <kdeversion.h>

#include "kdirwatch.h"
#include <qstringlist.h>
#include <klineedit.h>

#include <config.h>
#include <qtextcodec.h>
#include <kauthorized.h>

#include "basket.h"
#include "note.h"
#include "notefactory.h"
#include "variouswidgets.h"
#include "linklabel.h"
#include "global.h"
#include "container.h"
#include "xmlwork.h"
#include "settings.h"
#include "popupmenu.h"
#include "debugwindow.h"
#include "exporterdialog.h"


/** Basket */

const int Basket::c_updateTime = 200;



// Remove the note from the basket and delete the associated file
// If the note mirror a file, it will ask before deleting or not the file
// But if askForMirroredFile is false, it willn't ask NOR delete the MIRRORED file
//  (it will anyway delete the file if it is not a mirror)
void Basket::delNote ( Note *note, bool askForMirroredFile )
{
//...
	if ( hasFocus() )
		focusANote();       // We need note->next() and note->previous() here [BUT deleted note should be hidden]
	if ( note->isSelected() )
		note->setSelected ( false ); //removeSelectedNote();

	relayoutNotes();
	recolorizeNotes();
	resetInsertTo();         // If we delete the first or the last, pointer to it is invalid
	save();

	if ( note == m_startOfShiftSelectionNote )
		m_startOfShiftSelectionNote = 0L;

	if ( isDuringEdit() && m_editor->editedNote() == note )
		closeEditor ( false );
//...
}


// Calculate where to paste or drop
void Basket::computeInsertPlace ( const QPoint &cursorPosition )
{
	int y = cursorPosition.y();

	if ( countShown() == 0 )
		return;

	// TODO: Memorize the last hovered note to avoid a new computation on dragMoveEvent !!
	//       If the mouse is not over the last note, compute which new is :
	// TODO: Optimization : start from m_insertAtNote and compare y position to search before or after (or the same)
	for ( Note *it = firstNote(); it != 0L; it = it->next() )
		if ( ( it->isVisible() ) && ( it->y() + it->height() >= y ) && ( it->y() < y ) )
		{
			int center = it->y() + ( it->height() / 2 );
			m_insertAtNote = it;
			m_insertAfter  = y > center;
			return;
		}
	// Else, there is at least one shown note but cursor hover NO note, so we are after the last shown note
	m_insertAtNote = lastShownNote();
	m_insertAfter  = true;

	// Code for rectangular notes :
	/*QRect globalRect = it->rect();
	globalRect.moveTopLeft(it->pos() + contentsY());
	if ( globalRect.contains(curPos) ) {
		it->doInterestingThing();
	}*/
}

void Basket::dragMoveEvent ( QDragMoveEvent* event )
{
//	m_isDuringDrag = true;

	if ( isLocked() )
		return;

//	FIXME: viewportToContents does NOT work !!!
//	QPoint pos = viewportToContents(event->pos());
	QPoint pos ( event->pos().x() + contentsX(), event->pos().y() + contentsY() );

//	if (insertAtCursorPos())
	computeInsertPlace ( pos );

	showFrameInsertTo();
	acceptDropEvent ( event );

	// A workarround since QScrollView::dragAutoScroll seem to have no effect :
	ensureVisible ( event->pos().x() + contentsX(), event->pos().y() + contentsY(), 30, 30 );
//	QScrollView::dragMoveEvent(event);
}

void Basket::dropEvent ( QDropEvent *event )
{
	QPoint pos = event->pos();
	std::cout << "Drop Event at position " << pos.x() << ":" << pos.y() << std::endl;
	m_isDuringDrag = false;
	emit resetStatusBarText();

	if ( isLocked() )
		return;

	NoteFactory::dropNote ( event, this, true, event->action(), dynamic_cast<Note*> ( event->source() ) );
	// TODO: need to know if we really inserted an (or several!!!!) note !!!
	ensureNoteVisible ( lastInsertedNote() );
	unselectAllBut ( lastInsertedNote() );
	setFocusedNote ( lastInsertedNote() );

	resetInsertTo();
}

void Basket::moveOnTop()
{
	if ( m_countSelecteds == 0 )
		return;

	Note *endOfBrowse = firstShownNote();
	Note *topNote     = firstNote();
	Note *prev;
	for ( Note *it = lastShownNote(); it != 0L; )
	{
		prev = it->previous();
		if ( it->isSelected() )
		{
			m_insertAtNote = topNote;
			m_insertAfter  = false;
			changeNotePlace ( it );
			topNote = it;
		}
		if ( it == endOfBrowse )
			break;
		it = prev;
	}
	ensureNoteVisible ( firstShownNote() );
	ensureNoteVisible ( m_focusedNote );
}

void Basket::moveOnBottom()
{
	if ( m_countSelecteds == 0 )
		return;

	Note *endOfBrowse = lastShownNote();
	Note *bottomNote  = lastNote();
	Note *next;
	for ( Note *it = firstShownNote(); it != 0L; )
	{
		next = it->next();
		if ( it->isSelected() )
		{
			m_insertAtNote = bottomNote;
			m_insertAfter  = true;
			changeNotePlace ( it );
			bottomNote = it;
		}
		if ( it == endOfBrowse )
			break;
		it = next;
	}
	ensureNoteVisible ( lastShownNote() );
	ensureNoteVisible ( m_focusedNote );
}

void Basket::moveNoteUp()
{
	if ( m_countSelecteds == 0 )
		return;

	// Begin from the top (important move all selected notes one note up
	// AND to quit early if a selected note is the first shown one
	for ( Note *it = firstShownNote(); it != 0L; it = it->next() )
	{
		if ( it->isSelected() && it->isVisible() )
		{ // it->isVisible() not necessary, but in case...
			if ( it == firstShownNote() )
				return; // No way...
			m_insertAtNote = nextShownNoteFrom ( it, -1 ); // Previous shown note
			if ( m_insertAtNote == 0L )
			{ // Should not appends, since it's not the first shown note,
				resetInsertTo();        // there SHOULD be one before
				return;
			}
			m_insertAfter  = false;
			changeNotePlace ( it );
		}
		if ( it == lastShownNote() )
			break;
	}
	ensureNoteVisible ( m_focusedNote );
}

void Basket::moveNoteDown()
{
	if ( m_countSelecteds == 0 )
		return;

	// Begin from the bottom (important move all selected notes one note down
	// AND to quit early if a selected note is the last shown one
	for ( Note *it = lastShownNote(); it != 0L; it = it->previous() )
	{
		if ( it->isSelected() && it->isVisible() )
		{ // it->isVisible() not necessary, but in case...
			if ( it == lastShownNote() )
				return; // No way...
			m_insertAtNote = nextShownNoteFrom ( it, 1 ); // Next shown note
			if ( m_insertAtNote == 0L )
			{ // Should not appends, since it's not the last shown note,
				resetInsertTo();        // there SHOULD be one before
				return;
			}
			m_insertAfter  = true;
			changeNotePlace ( it );
		}
		if ( it == firstShownNote() )
			break;
	}
	ensureNoteVisible ( m_focusedNote );
}

#endif // #if 0

#include "basket.moc"

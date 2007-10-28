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

#include <qpainter.h>
#include <kglobalsettings.h>
#include <qstyle.h>
#include <kapplication.h>
#include <kstyle.h>
#include <qcursor.h>
#include <kiconloader.h>
//#include <kpixmapeffect.h>
#include <qpixmap.h>
#include <kglobal.h>
#include <klocale.h>
#include <kurifilter.h>
#include <qfile.h>
#include <qtextstream.h>
#include <kdebug.h>

#include <stdlib.h> // rand() function
#include <math.h> // sqrt() and pow() functions

#include <iostream>

#ifdef None
#undef None
#endif

#include "basket.h"
#include "tag.h"
#include "note.h"
#include "tools.h"
#include "settings.h"
#include "notefactory.h" // For NoteFactory::filteredURL()

/** class Note: */

#define FOR_EACH_CHILD(childVar) \
	for (Note *childVar = firstChild(); childVar; childVar = childVar->next())

// TODO:
#define FOR_EACH_VISIBLE_CHILD(childVar) \
	for (...)

int Note::NOTE_MARGIN      = 2;
int Note::INSERTION_HEIGHT = 5;
int Note::EXPANDER_WIDTH   = 9;
int Note::EXPANDER_HEIGHT  = 9;
int Note::GROUP_WIDTH      = 2*NOTE_MARGIN + EXPANDER_WIDTH;
int Note::HANDLE_WIDTH     = GROUP_WIDTH;
int Note::RESIZER_WIDTH    = GROUP_WIDTH;
int Note::TAG_ARROW_WIDTH  = 5;
int Note::EMBLEM_SIZE      = 16;
int Note::MIN_HEIGHT       = 2*NOTE_MARGIN + EMBLEM_SIZE;

Note::Note ( Basket *parent )
		: m_prev ( 0 ), m_next ( 0 ),
		m_x ( 0 ), m_y ( -1 ), m_width ( -1 ), m_height ( -1 ),
		m_groupWidth ( 250 ),
		m_isFolded ( false ), m_firstChild ( 0L ), m_parentNote ( 0 ),
		m_basket ( parent ), m_content ( 0 ), m_addedDate ( QDateTime::currentDateTime() ), m_lastModificationDate ( QDateTime::currentDateTime() ),
		m_computedAreas ( false ), m_onTop ( false ),
		m_deltaX ( 0 ), m_deltaY ( 0 ), m_deltaHeight ( 0 ), m_collapseFinished ( true ), m_expandingFinished ( true ),
		m_hovered ( false ), m_hoveredZone ( Note::None ), m_focused ( false ), m_selected ( false ), m_wasInLastSelectionRect ( false ),
		m_computedState(), m_emblemsCount ( 0 ), m_haveInvisibleTags ( false ),
		m_matching ( true )
{
	kDebug() << "Create Note" << endl;
}

Note::~Note()
{
	delete m_content;
	deleteChilds();
}

QString Note::addedStringDate()
{
	return KGlobal::locale()->formatDateTime ( m_addedDate );
}

QString Note::lastModificationStringDate()
{
	return KGlobal::locale()->formatDateTime ( m_lastModificationDate );
}

QString Note::toText ( const QString &cuttedFullPath )
{
	if ( content() )
	{
		// Convert note to text:
		QString text = content()->toText ( cuttedFullPath );
		// If we should not export tags with the text, return immediatly:
		if ( !Settings::exportTextTags() )
			return text;
		// Compute the text equivalent of the tag states:
		QString firstLine;
		QString otherLines;
		for ( State::List::Iterator it = m_states.begin(); it != m_states.end(); ++it )
		{
			if ( ! ( *it )->textEquivalent().isEmpty() )
			{
				firstLine += ( *it )->textEquivalent() + " ";
				if ( ( *it )->onAllTextLines() )
					otherLines += ( *it )->textEquivalent() + " ";
			}
		}
		// Merge the texts:
		if ( firstLine.isEmpty() )
			return text;
		if ( otherLines.isEmpty() )
			return firstLine + text;
		QStringList lines = text.split ( '\n' );
		QString result = firstLine + lines[0] + ( lines.count() > 1 ? "\n" : "" );
		for ( uint i = 1/*Skip the first line*/; i < lines.count(); ++i )
			result += otherLines + lines[i] + ( i < lines.count() - 1 ? "\n" : "" );
		return result;
	}
	else
		return "";
}

bool Note::computeMatching ( const FilterData &data )
{
	// Groups are always matching:
	if ( !content() )
		return true;

	// If we were editing this note and there is a save operation in the middle, then do not hide it suddently:
	if ( basket()->editedNote() == this )
		return true;

	bool matching;
	// First match tags (they are fast to compute):
	switch ( data.tagFilterType )
	{
		default:
		case FilterData::DontCareTagsFilter: matching = true;                   break;
		case FilterData::NotTaggedFilter:    matching = m_states.count() <= 0;  break;
		case FilterData::TaggedFilter:       matching = m_states.count() > 0;   break;
		case FilterData::TagFilter:          matching = hasTag ( data.tag );       break;
		case FilterData::StateFilter:        matching = hasState ( data.state );   break;
	}

	// Don't try to match the content text if we are not matching now (the filter is of 'AND' type) or if we shouldn't try to match the string:
	if ( matching && !data.string.isEmpty() )
		matching = content()->match ( data );

	return matching;
}

int Note::newFilter ( const FilterData &data )
{
	bool wasMatching = matching();
	m_matching = computeMatching ( data );
	setOnTop ( wasMatching && matching() );
	if ( !matching() )
		setSelected ( false );

	int countMatches = ( content() && matching() ? 1 : 0 );

	FOR_EACH_CHILD ( child )
	countMatches += child->newFilter ( data );

	return countMatches;
}

void Note::deleteSelectedNotes ( bool deleteFilesToo )
{
	if ( content() && isSelected() )
	{
		basket()->unplugNote ( this );
		if ( deleteFilesToo && content() && content()->useFile() )
			Tools::deleteRecursively ( fullPath() );//basket()->deleteFiles(fullPath()); // Also delete the folder if it's a folder
		//delete this;
		return;
	}

	Note *child = firstChild();
	Note *next;
	while ( child )
	{
		next = child->next(); // If we delete 'child' on the next line, child->next() will be 0!
		child->deleteSelectedNotes ( deleteFilesToo );
		child = next;
	}
}

int Note::count()
{
	if ( content() )
		return 1;

	int count = 0;
	FOR_EACH_CHILD ( child )
	count += child->count();
	return count;
}

int Note::countDirectChilds()
{
	int count = 0;
	FOR_EACH_CHILD ( child )
	++count;
	return count;
}

QString Note::fullPath()
{
	if ( content() )
		return basket()->fullPath() + content()->fileName();
	else
		return "";
}

void Note::update()
{
	basket()->updateNote ( this );
}

void Note::setFocused ( bool focused )
{
	if ( m_focused == focused )
		return;

	m_focused = focused;
	unbufferize();
	update(); // FIXME: ???
}

void Note::setSelected ( bool selected )
{
	if ( isGroup() )
		selected = false; // A group cannot be selected!

	if ( m_selected == selected )
		return;

	if ( !selected && basket()->editedNote() == this )
	{
		basket()->closeEditor();
		return; // To avoid a bug that would count 2 less selected notes instead of 1 less! Because m_selected is modified only below.
	}

	if ( selected )
		basket()->addSelectedNote();
	else
		basket()->removeSelectedNote();

	m_selected = selected;
	unbufferize();
	update(); // FIXME: ???
}

void Note::resetWasInLastSelectionRect()
{
	m_wasInLastSelectionRect = false;

	FOR_EACH_CHILD ( child )
	child->resetWasInLastSelectionRect();
}

void Note::finishLazyLoad()
{
	if ( content() )
		content()->finishLazyLoad();

	FOR_EACH_CHILD ( child )
	child->finishLazyLoad();
}

void Note::selectIn ( const QRect &rect, bool invertSelection, bool unselectOthers /*= true*/ )
{
//	QRect myRect(x(), y(), width(), height());

//	bool intersects = myRect.intersects(rect);

	// Only intersects with visible areas.
	// If the note is not visible, the user don't think it will be selected while selecting the note(s) that hide this, so act like the user think:
	bool intersects = false;
	for ( QList<QRect>::iterator it = m_areas.begin(); it != m_areas.end(); ++it )
	{
		QRect &r = *it;
		if ( r.intersects ( rect ) )
		{
			intersects = true;
			break;
		}
	}

	bool toSelect = intersects || ( !unselectOthers && isSelected() );
	if ( invertSelection )
	{
		if ( m_wasInLastSelectionRect == intersects )
			toSelect = isSelected();
		else if ( intersects xor m_wasInLastSelectionRect )
			toSelect = !isSelected();// xor intersects;
	}
	setSelected ( toSelect );
	m_wasInLastSelectionRect = intersects;

	Note *child = firstChild();
	bool first = true;
	while ( child )
	{
		if ( ( showSubNotes() || first ) && child->matching() )
			child->selectIn ( rect, invertSelection, unselectOthers );
		else
			child->setSelectedRecursivly ( false );
		child = child->next();
		first = false;
	}
}

bool Note::allSelected()
{
	if ( isGroup() )
	{
		Note *child = firstChild();
		bool first = true;
		while ( child )
		{
			if ( ( showSubNotes() || first ) && child->matching() )
				if ( !child->allSelected() )
					return false;;
			child = child->next();
			first = false;
		}
		return true;
	}
	else
		return isSelected();
}

void Note::setSelectedRecursivly ( bool selected )
{
	setSelected ( selected && matching() );

	FOR_EACH_CHILD ( child )
	child->setSelectedRecursivly ( selected );
}

void Note::invertSelectionRecursivly()
{
	if ( content() )
		setSelected ( !isSelected() && matching() );

	FOR_EACH_CHILD ( child )
	child->invertSelectionRecursivly();
}

void Note::unselectAllBut ( Note *toSelect )
{
	if ( this == toSelect )
		setSelectedRecursivly ( true );
	else
	{
		setSelected ( false );

		Note *child = firstChild();
		bool first = true;
		while ( child )
		{
			if ( ( showSubNotes() || first ) && child->matching() )
				child->unselectAllBut ( toSelect );
			else
				child->setSelectedRecursivly ( false );
			child = child->next();
			first = false;
		}
	}
}

void Note::invertSelectionOf ( Note *toSelect )
{
	if ( this == toSelect )
		setSelectedRecursivly ( !isSelected() );
	else
	{
		Note *child = firstChild();
		bool first = true;
		while ( child )
		{
			if ( ( showSubNotes() || first ) && child->matching() )
				child->invertSelectionOf ( toSelect );
			child = child->next();
			first = false;
		}
	}
}

Note* Note::theSelectedNote()
{
	if ( !isGroup() && isSelected() )
		return this;

	Note *selectedOne;
	Note *child = firstChild();
	while ( child )
	{
		selectedOne = child->theSelectedNote();
		if ( selectedOne )
			return selectedOne;
		child = child->next();
	}

	return 0;
}

NoteSelection* Note::selectedNotes()
{
	if ( content() )
		if ( isSelected() )
			return new NoteSelection ( this );
		else
			return 0;

	NoteSelection *selection = new NoteSelection ( this );

	FOR_EACH_CHILD ( child )
	selection->append ( child->selectedNotes() );

	if ( selection->firstChild )
	{
		if ( selection->firstChild->next )
			return selection;
		else
		{
			// If 'selection' is a groupe with only one content, return directly that content:
			NoteSelection *reducedSelection = selection->firstChild;
// 			delete selection; // TODO: Cut all connexions of 'selection' before deleting it!
			for ( NoteSelection *node = reducedSelection; node; node = node->next )
				node->parent = 0;
			return reducedSelection;
		}
	}
	else
	{
		delete selection;
		return 0;
	}
}

bool Note::isAfter ( Note *note )
{
	if ( this == 0 || note == 0 )
		return true;

	Note *next = this;
	while ( next )
	{
		if ( next == note )
			return false;
		next = next->nextInStack();
	}
	return true;
}

bool Note::contains ( Note *note )
{
//	if (this == note)
//		return true;

	while ( note )
		if ( note == this )
			return true;
		else
			note = note->parentNote();

//	FOR_EACH_CHILD (child)
//		if (child->contains(note))
//			return true;
	return false;
}

Note* Note::firstRealChild()
{
	Note *child = m_firstChild;
	while ( child )
	{
		if ( !child->isGroup() /*&& child->matching()*/ )
			return child;
		child = child->firstChild();
	}
	// Empty group:
	return 0;
}

Note* Note::lastRealChild()
{
	Note *child = lastChild();
	while ( child )
	{
		if ( child->content() )
			return child;
		Note *possibleChild = child->lastRealChild();
		if ( possibleChild && possibleChild->content() )
			return possibleChild;
		child = child->prev();
	}
	return 0;
}

Note* Note::lastChild()
{
	Note *child = m_firstChild;
	while ( child && child->next() )
		child = child->next();

	return child;
}

Note* Note::lastSibling()
{
	Note *last = this;
	while ( last && last->next() )
		last = last->next();

	return last;
}

int Note::yExpander()
{
	Note *child = firstRealChild();
	if ( child && !child->isVisible() )
		child = child->nextShownInStack(); // FIXME: Restrict scope to 'this'

	if ( child )
		return ( child->height() - EXPANDER_HEIGHT ) / 2 + ! ( child->height() %2 );
	else // Groups always have at least 2 notes, except for columns which can have no child (but should exists anyway):
		return 0;
}

bool Note::isFree()
{
	return parentNote() == 0 && basket()->isFreeLayout();
}

bool Note::isColumn()
{
	return parentNote() == 0 && basket()->isColumnsLayout();
}

bool Note::hasResizer()
{
	// "isFree" || "isColmun but not the last"
	return parentNote() == 0 && ( basket()->isFreeLayout() || m_next != 0L );
}

int Note::resizerHeight()
{
	//TODO return ( isColumn() ? basket()->contentsHeight() : height() );
	return ( isColumn() ? basket()->height() : height() );
}

void Note::setHoveredZone ( Zone zone ) // TODO: Remove setHovered(bool) and assume it is hovered if zone != None !!!!!!!
{
	if ( m_hoveredZone != zone )
	{
		if ( content() )
			content()->setHoveredZone ( m_hoveredZone, zone );
		m_hoveredZone = zone;
		unbufferize();
	}
}

Note::Zone Note::zoneAt ( const QPoint &pos, bool toAdd )
{
	// Keep the resizer highlighted when resizong, even if the cursor is over another note:
	if ( basket()->resizingNote() == this )
		return Resizer;

	// When dropping/pasting something on a column resizer, add it at the bottom of the column, and don't group it whith the whole column:
	if ( toAdd && isColumn() && hasResizer() )
	{
		int right = rightLimit() - x();
		if ( ( pos.x() >= right ) && ( pos.x() < right + RESIZER_WIDTH ) && ( pos.y() >= 0 ) && ( pos.y() < resizerHeight() ) ) // Code copied from below
			return BottomColumn;
	}

	// Below a column:
	if ( isColumn() )
	{
		if ( pos.y() >= height() && pos.x() < rightLimit() - x() )
			return BottomColumn;
	}

	// If toAdd, return only TopInsert, TopGroup, BottomInsert or BottomGroup
	// (by spanning those areas in 4 equal rectangles in the note):
	if ( toAdd )
	{
		if ( !isFree() && !Settings::groupOnInsertionLine() )
			return ( pos.y() < height() / 2 ? TopInsert : BottomInsert );
		if ( isColumn() && pos.y() >= height() )
			return BottomGroup;
		if ( pos.y() < height() / 2 )
			if ( pos.x() < width() / 2 && !isFree() )
				return TopInsert;
			else
				return TopGroup;
		else
			if ( pos.x() < width() / 2 && !isFree() )
				return BottomInsert;
			else
				return BottomGroup;
	}

	// If in the resizer:
	if ( hasResizer() )
	{
		int right = rightLimit() - x();
		if ( ( pos.x() >= right ) && ( pos.x() < right + RESIZER_WIDTH ) && ( pos.y() >= 0 ) && ( pos.y() < resizerHeight() ) )
			return Resizer;
	}

	// If isGroup, return only Group, GroupExpander, TopInsert or BottomInsert:
	if ( isGroup() )
	{
		if ( pos.y() < INSERTION_HEIGHT )
			return ( isFree() ? TopGroup : TopInsert );
		if ( pos.y() >= height() - INSERTION_HEIGHT )
			return ( isFree() ? BottomGroup : BottomInsert );

		if ( pos.x() >= NOTE_MARGIN  &&  pos.x() < NOTE_MARGIN + EXPANDER_WIDTH )
		{
			int yExp = yExpander();
			if ( pos.y() >= yExp  &&  pos.y() < yExp + EXPANDER_HEIGHT )
				return GroupExpander;
		}
		if ( pos.x() < width() )
			return Group;
		else
			return Note::None;
	}

	// Else, it's a normal note:

	if ( pos.x() < HANDLE_WIDTH )
		return Handle;

	if ( pos.y() < INSERTION_HEIGHT )
		if ( ( !isFree() && !Settings::groupOnInsertionLine() ) || pos.x() < width() / 2 && !isFree() )
			return TopInsert;
		else
			return TopGroup;

	if ( pos.y() >= height() - INSERTION_HEIGHT )
		if ( ( !isFree() && !Settings::groupOnInsertionLine() ) || pos.x() < width() / 2 && !isFree() )
			return BottomInsert;
		else
			return BottomGroup;

	for ( int i =0; i < m_emblemsCount; i++ )
	{
		if ( pos.x() >= HANDLE_WIDTH + ( NOTE_MARGIN+EMBLEM_SIZE ) *i  &&
		        pos.x() <  HANDLE_WIDTH + ( NOTE_MARGIN+EMBLEM_SIZE ) *i + NOTE_MARGIN+EMBLEM_SIZE )
			return ( Zone ) ( Emblem0 + i );
	}

	if ( pos.x() < HANDLE_WIDTH + ( NOTE_MARGIN+EMBLEM_SIZE ) *m_emblemsCount + NOTE_MARGIN + TAG_ARROW_WIDTH + NOTE_MARGIN )
		return TagsArrow;

	if ( !linkAt ( pos ).isEmpty() )
		return Link;

	int customZone = content()->zoneAt ( pos - QPoint ( contentX(), NOTE_MARGIN ) );
	if ( customZone )
		return ( Note::Zone ) customZone;

	return Content;
}

QString Note::linkAt ( const QPoint &pos )
{
	QString link = m_content->linkAt ( pos - QPoint ( contentX(), NOTE_MARGIN ) );
	if ( link.isEmpty() )
		return link;
	else
		return NoteFactory::filteredURL ( KUrl ( link ) ).prettyUrl();//KURIFilter::self()->filteredURI(link);
}

int Note::contentX()
{
	return HANDLE_WIDTH + NOTE_MARGIN + ( EMBLEM_SIZE+NOTE_MARGIN ) *m_emblemsCount + TAG_ARROW_WIDTH + NOTE_MARGIN;
}

QRect Note::zoneRect ( Note::Zone zone, const QPoint &pos )
{
	if ( zone >= Emblem0 )
		return QRect ( HANDLE_WIDTH + ( NOTE_MARGIN+EMBLEM_SIZE ) * ( zone-Emblem0 ),
		               INSERTION_HEIGHT,
		               NOTE_MARGIN + EMBLEM_SIZE,
		               height() - 2*INSERTION_HEIGHT );

	int yExp;
	int right;
	int xGroup = ( isFree() ? ( isGroup() ? 0 : GROUP_WIDTH ) : width() / 2 );
	QRect rect;
	int insertSplit = ( Settings::groupOnInsertionLine() ? 2 : 1 );
	switch ( zone )
	{
		case Note::Handle:        return QRect ( 0, 0, HANDLE_WIDTH, height() );
		case Note::Group:
			yExp = yExpander();
			if ( pos.y() < yExp )                   return QRect ( 0,                     INSERTION_HEIGHT,       width(),     yExp - INSERTION_HEIGHT );
			if ( pos.y() > yExp + EXPANDER_HEIGHT ) return QRect ( 0,                     yExp + EXPANDER_HEIGHT, width(),     height() - yExp - EXPANDER_HEIGHT - INSERTION_HEIGHT );
			if ( pos.x() < NOTE_MARGIN )            return QRect ( 0,                     0,                      NOTE_MARGIN, height() );
			else                                  return QRect ( width() - NOTE_MARGIN, 0,                      NOTE_MARGIN, height() );
		case Note::TagsArrow:     return QRect ( HANDLE_WIDTH + ( NOTE_MARGIN+EMBLEM_SIZE ) *m_emblemsCount,
			                                 INSERTION_HEIGHT,
			                                 NOTE_MARGIN + TAG_ARROW_WIDTH + NOTE_MARGIN,
			                                 height() - 2*INSERTION_HEIGHT );
		case Note::Custom0:
		case Note::Content:       rect = content()->zoneRect ( zone, pos - QPoint ( contentX(), NOTE_MARGIN ) );
			rect.moveTo ( contentX(), NOTE_MARGIN );
			//TODO rect.moveBy ( contentX(), NOTE_MARGIN );
			return rect.intersect ( QRect ( contentX(), INSERTION_HEIGHT, width() - contentX(), height() - 2*INSERTION_HEIGHT ) ); // Only IN contentRect
		case Note::GroupExpander: return QRect ( NOTE_MARGIN, yExpander(), EXPANDER_WIDTH, EXPANDER_HEIGHT );
		case Note::Resizer:       right = rightLimit();
			return QRect ( right - x(), 0, RESIZER_WIDTH, resizerHeight() );
		case Note::Link:
		case Note::TopInsert:     if ( isGroup() ) return QRect ( 0,            0,                           width(),                              INSERTION_HEIGHT );
			else           return QRect ( HANDLE_WIDTH, 0,                           width() / insertSplit - HANDLE_WIDTH, INSERTION_HEIGHT );
		case Note::TopGroup:                     return QRect ( xGroup,       0,                           width() - xGroup,                     INSERTION_HEIGHT );
		case Note::BottomInsert:  if ( isGroup() ) return QRect ( 0,            height() - INSERTION_HEIGHT, width(),                              INSERTION_HEIGHT );
			else           return QRect ( HANDLE_WIDTH, height() - INSERTION_HEIGHT, width() / insertSplit - HANDLE_WIDTH, INSERTION_HEIGHT );
		case Note::BottomGroup:                  return QRect ( xGroup,       height() - INSERTION_HEIGHT, width() - xGroup,                     INSERTION_HEIGHT );
		case Note::BottomColumn:  return QRect ( 0, height(), rightLimit() - x(), basket()->height() - height() );
		case Note::None:          return QRect ( /*0, 0, -1, -1*/ );
		default:                  return QRect ( /*0, 0, -1, -1*/ );
	}
}

void Note::setCursor ( Zone zone )
{
	switch ( zone )
	{
		case Note::Handle:
		case Note::Group:         basket()->viewport()->setCursor ( Qt::SizeAllCursor );      break;
		case Note::Resizer:       if ( isColumn() )
				basket()->viewport()->setCursor ( Qt::SplitHCursor );
			else
				basket()->viewport()->setCursor ( Qt::SizeHorCursor );  break;

		case Note::Custom0:       content()->setCursor ( basket()->viewport(), zone );        break;

		case Note::Link:
		case Note::TagsArrow:
		case Note::GroupExpander: basket()->viewport()->setCursor ( Qt::PointingHandCursor ); break;

		case Note::Content:       basket()->viewport()->setCursor ( Qt::IBeamCursor );        break;

		case Note::TopInsert:
		case Note::TopGroup:
		case Note::BottomInsert:
		case Note::BottomGroup:
		case Note::BottomColumn:  basket()->viewport()->setCursor ( Qt::CrossCursor );        break;
		case Note::None:          basket()->viewport()->unsetCursor(); break;
		default:
			State *state = stateForEmblemNumber ( zone - Emblem0 );
			if ( state && state->parentTag()->states().count() > 1 )
				basket()->viewport()->setCursor ( Qt::PointingHandCursor );
			else
				basket()->viewport()->unsetCursor();
	}
}

void Note::addAnimation ( int deltaX, int deltaY, int deltaHeight )
{
	// Don't process animation that make the note stay in place!
	if ( deltaX == 0 && deltaY == 0 && deltaHeight == 0 )
		return;

	// If it was not animated previsouly, make it animated:
	if ( m_deltaX == 0 && m_deltaY == 0 && m_deltaHeight == 0 )
		basket()->addAnimatedNote ( this );

	// Configure the animation:
	m_deltaX      += deltaX;
	m_deltaY      += deltaY;
	m_deltaHeight += deltaHeight;
}

void Note::setFinalPosition ( int x, int y )
{
	addAnimation ( x - finalX(), y - finalY() );
}

void Note::initAnimationLoad()
{
	kDebug() << "animation load" << endl;
	// TODO
/*	int x, y;
	switch ( rand() % 4 )
	{
		case 0: // Put it on top:
			x = basket()->contentsX() + rand() % basket()->contentsWidth();
			y = -height();
			break;
		case 1: // Put it on bottom:
			x = basket()->contentsX() + rand() % basket()->contentsWidth();
			y = basket()->contentsY() + basket()->visibleHeight();
			break;
		case 2: // Put it on left:
			x = -width() - ( hasResizer() ? Note::RESIZER_WIDTH : 0 );
			y = basket()->contentsY() + rand() % basket()->visibleHeight();
			break;
		case 3: // Put it on right:
		default: // In the case of...
			x = basket()->contentsX() + basket()->visibleWidth();
			y = basket()->contentsY() + rand() % basket()->visibleHeight();
			break;
	}
	cancelAnimation();
	addAnimation ( finalX() - x, finalY() - y );
	setX ( x );
	setY ( y );

	if ( isGroup() )
	{
		const int viewHeight = basket()->contentsY() + basket()->visibleHeight();
		Note *child = firstChild();
		bool first = true;
		while ( child )
		{
			if ( child->finalY() < viewHeight )
			{
				if ( ( showSubNotes() || first ) && child->matching() )
					child->initAnimationLoad();
			}
			else
				break; // 'child' are not a free notes (because child of at least one note, 'this'), so 'child' is ordered vertically.
			child = child->next();
			first = false;
		}
	}*/
}


bool Note::advance()
{
	// Animate X:
	if ( m_deltaX != 0 )
	{
		int deltaX = m_deltaX / 3;
		if ( deltaX == 0 )
			deltaX = ( m_deltaX > 0 ? 1 : -1 );
		setX ( m_x + deltaX );
		m_deltaX -= deltaX;
	}

	// Animate Y:
	if ( m_deltaY != 0 )
	{
		int deltaY = m_deltaY / 3;
		if ( deltaY == 0 )
			deltaY = ( m_deltaY > 0 ? 1 : -1 );
		setY ( m_y + deltaY );
		m_deltaY -= deltaY;
	}

	// Animate Height:
	if ( m_deltaHeight != 0 )
	{
		int deltaHeight = m_deltaHeight / 3;
		if ( deltaHeight == 0 )
			deltaHeight = ( m_deltaHeight > 0 ? 1 : -1 );
		m_height += deltaHeight;
		unbufferize();
		m_deltaHeight -= deltaHeight;
	}

	if ( m_deltaHeight == 0 )
	{
		m_collapseFinished  = true;
		m_expandingFinished = true;
	}

	// Return true if the animation is finished:
	return ( m_deltaX == 0 && m_deltaY == 0 && m_deltaHeight == 0 );
}

void Note::unsetWidth()
{
	m_width = 0;
	unbufferize();

	FOR_EACH_CHILD ( child )
	child->unsetWidth();
}

void Note::requestRelayout()
{
	m_width = 0;
	unbufferize();
	basket()->relayoutNotes ( true ); // TODO: A signal that will relayout ONCE and DELAYED if called several times
}

void Note::setWidth ( int width ) // TODO: inline ?
{
	if ( m_width != width )
		setWidthForceRelayout ( width );
}

void Note::setWidthForceRelayout ( int width )
{
	unbufferize();
	m_width = ( width < minWidth() ? minWidth() : width );
	int contentWidth = width - contentX() - NOTE_MARGIN;
	if ( m_content )
	{ ///// FIXME: is this OK?
		if ( contentWidth < 1 )
			contentWidth = 1;
		if ( contentWidth < m_content->minWidth() )
			contentWidth = m_content->minWidth();
		m_height = m_content->setWidthAndGetHeight ( contentWidth/* < 1 ? 1 : contentWidth*/ ) + 2 * NOTE_MARGIN;
		if ( m_height < 3 * INSERTION_HEIGHT ) // Assure a minimal size...
			m_height = 3 * INSERTION_HEIGHT;
	}
}

int Note::minWidth()
{
	if ( m_content )
		return contentX() + m_content->minWidth() + NOTE_MARGIN;
	else
		return GROUP_WIDTH; ///// FIXME: is this OK?
}

int Note::minRight()
{
	if ( isGroup() )
	{
		int right = finalX() + width();
		Note* child = firstChild();
		bool first = true;
		while ( child )
		{
			if ( ( showSubNotes() || first ) && child->matching() )
				right = qMax ( right, child->minRight() );
			child = child->next();
			first = false;
		}
		if ( isColumn() )
		{
			int minColumnRight = finalX() + 2*HANDLE_WIDTH;
			if ( right < minColumnRight )
				return minColumnRight;
		}
		return right;
	}
	else
		return finalX() + minWidth();
}

void Note::setX ( int x )
{
	if ( m_x == x )
		return;

	if ( isBufferized() && basket()->hasBackgroundImage() )
	{
		// Unbufferize only if the background change:
		if ( basket()->isTiledBackground() )
			unbufferize();
		else
		{
			int bgw = basket()->backgroundPixmap()->width();
			if ( m_x >= bgw && x < bgw ) // Was not in the background image and is now inside it:
				unbufferize();
			else if ( m_x < bgw ) // Was in the background image and is now at another position of the background image or is now outside:
				unbufferize();
		}
	}

	m_x = x;
}

void Note::setY ( int y )
{
	if ( m_y == y )
		return;

	if ( isBufferized() && basket()->hasBackgroundImage() )
	{
		// Unbufferize only if the background change:
		if ( basket()->isTiledBackground() )
			unbufferize();
		else
		{
			int bgh = basket()->backgroundPixmap()->height();
			if ( m_y >= bgh && y < bgh ) // Was not in the background image and is now inside it:
				unbufferize();
			else if ( m_y < bgh ) // Was in the background image and is now at another position of the background image or is now outside:
				unbufferize();
		}
	}

	m_y = y;
}


void Note::toggleFolded ( bool animate )
{
	// Close the editor if it was editing a note that we are about to hide after collapsing:
	if ( !m_isFolded && basket() && basket()->isDuringEdit() )
	{
		if ( contains ( basket()->editedNote() ) && firstRealChild() != basket()->editedNote() )
			basket()->closeEditor();
	}

	// Important to close the editor FIRST, because else, the last edited note would not show during folding animation (don't ask me why ;-) ):
	m_isFolded = ! m_isFolded;

	unbufferize();

	if ( animate )
	{
		// We animate collapsing (so sub-notes fluidly go under the first note)
		// We don't animate expanding: we place sub-notes directly under the first note (and the next relayout will animate the expanding)
		// But if user quickly collapsed and then expand (while the collapsing animation isn't finished), we animate anyway
		bool animateSetUnder = ( m_isFolded || !m_collapseFinished );
//		std::cout << "fold:" << m_isFolded << " collapseFinished:"  << m_collapseFinished << " animateSetUnder:" << animateSetUnder << std::endl;

		if ( m_isFolded )
			m_collapseFinished = false;
		else
			m_expandingFinished = false;

		Note* note = firstChild();
		if ( note )
		{
			note->setOnTop ( true );
			while ( ( note = note->next() ) )
			{ // Don't process the first child: it is OK
				note->setRecursivelyUnder ( /*firstRealChild*/firstChild(), animateSetUnder );
				note->setOnTop ( false );
			}
		}
	}

	//if (basket()->focusedNote() && !basket()->focusedNote()->isVisible()) {
	if ( basket()->isLoaded() )
	{
		basket()->setFocusedNote ( firstRealChild() );
		basket()->m_startOfShiftSelectionNote = firstRealChild();
	}

	if ( basket()->isLoaded() && !m_isFolded )
	{
		//basket()->setFocusedNote(this);
		basket()->relayoutNotes ( true );
		basket()->ensureNoteVisible ( this );
	}

	basket()->save(); // FIXME: SHOULD WE ALWAYS SAVE ????????
}

void Note::setRecursivelyUnder ( Note *under, bool animate )
{
	int y = /*finalHeight() > under->finalHeight() ? under->finalY() :*/ under->finalBottom() - finalHeight() + 1;
	if ( animate )
		setFinalPosition ( finalX(), y );
	else
	{
		setY ( y );
		cancelAnimation();
	}

	if ( isGroup() )
		FOR_EACH_CHILD ( child )
		child->setRecursivelyUnder ( under, animate );
}


Note* Note::noteAt ( int x, int y )
{
	if ( matching() && hasResizer() )
	{
		int right = rightLimit();
		// TODO: This code is dupliacted 3 times: !!!!
		if ( ( x >= right ) && ( x < right + RESIZER_WIDTH ) && ( y >= m_y ) && ( y < m_y + resizerHeight() ) )
		{
			if ( ! m_computedAreas )
				recomputeAreas();
			for ( QList<QRect>::iterator it = m_areas.begin(); it != m_areas.end(); ++it )
			{
				QRect &rect = *it;
				if ( rect.contains ( x, y ) )
					return this;
			}
		}
	}

	if ( isGroup() )
	{
		if ( ( x >= m_x ) && ( x < m_x + width() ) && ( y >= m_y ) && ( y < m_y + m_height ) )
		{
			if ( ! m_computedAreas )
				recomputeAreas();
			for ( QList<QRect>::iterator it = m_areas.begin(); it != m_areas.end(); ++it )
			{
				QRect &rect = *it;
				if ( rect.contains ( x, y ) )
					return this;
			}
			return NULL;
		}
		Note *child = firstChild();
		Note *found;
		bool first = true;
		while ( child )
		{
			if ( ( showSubNotes() || first ) && child->matching() )
			{
				found = child->noteAt ( x, y );
				if ( found )
					return found;
			}
			child = child->next();
			first = false;
		}
	}
	else if ( matching() && y >= m_y && y < m_y + m_height && x >= m_x && x < m_x + m_width )
	{
		if ( ! m_computedAreas )
			recomputeAreas();
		for ( QList<QRect>::iterator it = m_areas.begin(); it != m_areas.end(); ++it )
		{
			QRect &rect = *it;
			if ( rect.contains ( x, y ) )
				return this;
		}
		return NULL;
	}

	return NULL;
}

QRect Note::rect()
{
	return QRect ( x(), y(), width(), height() );
}

QRect Note::resizerRect()
{
	return QRect ( rightLimit(), y(), RESIZER_WIDTH, resizerHeight() );
}


bool Note::showSubNotes()
{
	return !m_isFolded || !m_collapseFinished || basket()->isFiltering();
}

void Note::relayoutAt ( int x, int y, bool animate )
{
	if ( !matching() )
		return;

	m_computedAreas = false;
	m_areas.clear();

	// Don't relayout free notes one under the other, because by definition they are freely positionned!
	if ( isFree() )
	{
		x = finalX();
		y = finalY();
		// If it's a column, it always have the same "fixed" position (no animation):
	}
	else if ( isColumn() )
	{
		x = ( prev() ? prev()->rightLimit() + RESIZER_WIDTH : 0 );
		y = 0;
		cancelAnimation();
		setX ( x );
		setY ( y );
		// But relayout others vertically if they are inside such primary groups or if it is a "normal" basket:
	}
	else
	{
		if ( animate )
			setFinalPosition ( x, y );
		else
		{
			cancelAnimation();
			setX ( x );
			setY ( y );
		}
	}

	// Then, relayout sub-notes (only the first, if the group is folded) and so, assign an height to the group:
	if ( isGroup() )
	{
		int h = 0;
		Note *child = firstChild();
		bool first = true;
		while ( child )
		{
			if ( child->matching() && ( !m_isFolded || first || basket()->isFiltering() ) )
			{ // Don't use showSubNotes() but use !m_isFolded because we don't want a relayout for the animated collapsing notes
				child->relayoutAt ( x + width(), y+h, animate );
				h += child->finalHeight();
			}
			else                                  // In case the user collapse a group, then move it and then expand it:
				child->setXRecursivly ( x + width() ); //  notes SHOULD have a good X coordonate, and not the old one!
			// For future animation when re-match, but on bottom of already matched notes!
			// Find parent primary note and set the Y to THAT y:
			if ( !child->matching() )
				child->setY ( parentPrimaryNote()->y() );
			child = child->next();
			first = false;
		}
		if ( finalHeight() != h || m_height != h )
		{
			unbufferize();
			if ( animate )
				addAnimation ( 0, 0, h - finalHeight() );
			else
			{
				m_height = h;
				unbufferize();
			}
		}
	}
	else
	{
		setWidth ( finalRightLimit() - x );
		// If rightLimit is excedded, set the top-level right limit!!!
		// and NEED RELAYOUT
	}

	// Set the basket area limits (but not for child notes: no need, because they will look for theire parent note):
	if ( !parentNote() )
	{
		if ( basket()->tmpWidth < finalRightLimit() + ( hasResizer() ? RESIZER_WIDTH : 0 ) )
			basket()->tmpWidth = finalRightLimit() + ( hasResizer() ? RESIZER_WIDTH : 0 );
		if ( basket()->tmpHeight < finalY() + finalHeight() )
			basket()->tmpHeight = finalY() + finalHeight();
		// However, if the note exceed the allowed size, let it! :
	}
	else if ( !isGroup() )
	{
		if ( basket()->tmpWidth < finalX() + width() )
			basket()->tmpWidth = finalX() + width();
		if ( basket()->tmpHeight < finalY() + finalHeight() )
			basket()->tmpHeight = finalY() + finalHeight();
	}
}

void Note::setXRecursivly ( int x )
{
	m_deltaX = 0;
	setX ( x );

	FOR_EACH_CHILD ( child )
	child->setXRecursivly ( x + width() );
}

void Note::setYRecursivly ( int y )
{
	m_deltaY = 0;
	setY ( y );

	FOR_EACH_CHILD ( child )
	child->setYRecursivly ( y );
}

void Note::setGroupWidth ( int width )
{
	m_groupWidth = width;
}

int Note::groupWidth()
{
	if ( hasResizer() )
		return m_groupWidth;
	else
		return rightLimit() - x();
}

int Note::rightLimit()
{
	if ( isColumn() && m_next == 0L ) // The last column
		return x() + minWidth();
		//TODO return qMax ( x() + minWidth(), basket()->visibleWidth() );
	else if ( parentNote() )
		return parentNote()->rightLimit();
	else
		return m_x + m_groupWidth;
}

int Note::finalRightLimit()
{
	if ( isColumn() && m_next == 0L ) // The last column
		return finalX() + minWidth();
		//TODO return qMax ( finalX() + minWidth(), basket()->visibleWidth() );
	else if ( parentNote() )
		return parentNote()->finalRightLimit();
	else
		return finalX() + m_groupWidth;
}

/*
 * This code is derivated from drawMetalGradient() from the Qt documentation:
 */
void drawGradient ( QPainter *p, const QColor &colorTop, const QColor & colorBottom,
                    int x, int y, int w, int h,
                    bool sunken, bool horz, bool flat ) /*const*/
{
	QColor highlight ( colorBottom );
	QColor subh1 ( colorTop );
	QColor subh2 ( colorTop );

	QColor topgrad ( colorTop );
	QColor botgrad ( colorBottom );


	if ( flat && !sunken )
		p->fillRect ( x, y, w, h, colorTop );
	else
	{
		int i  = 0;
		int x1 = x;
		int y1 = y;
		int x2 = x + w - 1;
		int y2 = y + h - 1;
		if ( horz )
			x2 = x2;
		else
			y2 = y2;

#define DRAWLINE if (horz) \
                    p->drawLine( x1, y1+i, x2, y1+i ); \
                 else \
                    p->drawLine( x1+i, y1, x1+i, y2 ); \
                 i++;

		// Gradient:
		int ng = ( horz ? h : w ); // how many lines for the gradient?

		int h1, h2, s1, s2, v1, v2;
		if ( !sunken )
		{
			//TODO topgrad.hsv ( &h1, &s1, &v1 );
			//TODO botgrad.hsv ( &h2, &s2, &v2 );
		}
		else
		{
			//TODO botgrad.hsv ( &h1, &s1, &v1 );
			//TODO topgrad.hsv ( &h2, &s2, &v2 );
		}

		if ( ng > 1 )
		{
			for ( int j =0; j < ng; j++ )
			{
				/*TODO p->setPen ( QColor ( h1 + ( ( h2-h1 ) *j ) / ( ng-1 ),
				                     s1 + ( ( s2-s1 ) *j ) / ( ng-1 ),
				                     v1 + ( ( v2-v1 ) *j ) / ( ng-1 ),  QColor::Hsv ) );*/
				DRAWLINE;
			}
		}
		else if ( ng == 1 )
		{
			//TODO p->setPen ( QColor ( ( h1+h2 ) /2, ( s1+s2 ) /2, ( v1+v2 ) /2, QColor::Hsv ) );
			DRAWLINE;
		}
	}
}

void Note::drawExpander ( QPainter *painter, int x, int y, const QColor &background, bool expand, Basket *basket )
{
	kDebug() << "enter" << endl;
	//TODO
/*
	// If the current style is a KStyle, use it to draw the expander (plus or minus):
	if ( dynamic_cast<KStyle*> ( & ( kapp->style() ) ) != NULL )
	{
		// Set the 4 rounded corners background to background color:
		QPalette cg ( basket->colorGroup() );
		cg.setColor ( QPalette::Base, background );

		// Fill the inside of the expander in white, typically:
		QBrush brush ( KColorScheme(KColorScheme::View).background().color() );
		painter->fillRect ( x, y, 9, 9, brush );

		// Draw it:
		( ( KStyle& ) ( kapp->style() ) ).drawKStylePrimitive ( KStyle::KPE_ListViewExpander,
		        painter,
		        basket->viewport(),
		        QRect ( x, y, 9, 9 ),
		        cg,
		        ( expand ? QStyle::Style_On : QStyle::Style_Off ) );
		// Else, QStyle does not provide easy way to do so (if it's doable at all...)
		// So, I'm drawing it myself my immitating Plastik (pretty style)...
		// After all, the note/group handles are all non-QStyle aware so that doesn't matter if the expander is a custom one too.
	}
	else
	{
		int width  = EXPANDER_WIDTH;
		int height = EXPANDER_HEIGHT;
		const QPalette &cg = basket->colorGroup();

		// Fill white area:
		painter->fillRect ( x + 1, y + 1, width - 2, height - 2, cg.base() );
		// Draw contour lines:
		painter->setPen ( cg.dark() );
		painter->drawLine ( x + 2,         y,              x + width - 3, y );
		painter->drawLine ( x + 2,         y + height - 1, x + width - 3, y + height - 1 );
		painter->drawLine ( x,             y + 2,          x,             y + height - 3 );
		painter->drawLine ( x + width - 1, y + 2,          x + width - 1, y + height - 3 );
		// Draw edge points:
		painter->drawPoint ( x + 1,         y + 1 );
		painter->drawPoint ( x + width - 2, y + 1 );
		painter->drawPoint ( x + 1,         y + height - 2 );
		painter->drawPoint ( x + width - 2, y + height - 2 );
		// Draw anti-aliased points:
		painter->setPen ( Tools::mixColor ( cg.dark(), background ) );
		painter->drawPoint ( x + 1,         y );
		painter->drawPoint ( x + width - 2, y );
		painter->drawPoint ( x,             y + 1 );
		painter->drawPoint ( x + width - 1, y + 1 );
		painter->drawPoint ( x,             y + height - 2 );
		painter->drawPoint ( x + width - 1, y + height - 2 );
		painter->drawPoint ( x + 1,         y + height - 1 );
		painter->drawPoint ( x + width - 2, y + height - 1 );
		// Draw plus / minus:
		painter->setPen ( cg.text() );
		painter->drawLine ( x + 2, y + height / 2, x + width - 3, y + height / 2 );
		if ( expand )
			painter->drawLine ( x + width / 2, y + 2, x + width / 2, y + height - 3 );
	}*/
}

QColor expanderBackground ( int height, int y, const QColor &foreground )
{
	// We will divide height per two, substract one and use that below a division bar:
	// To avoid division by zero error, height should be bigger than 3.
	// And to avoid y errors or if y is on the borders, we return the border color: the background color.
	if ( height <= 3 || y <= 0 || y >= height - 1 )
		return foreground;

	QColor dark     = foreground.dark ( 110 );  // 1/1.1 of brightness
	QColor light    = foreground.light ( 150 ); // 50% brighter

	int h1, h2, s1, s2, v1, v2;
	int ng;
	if ( y <= ( height-2 ) /2 )
	{
		//TODO light.hsv ( &h1, &s1, &v1 );
		//TODO dark.hsv ( &h2, &s2, &v2 );
		ng = ( height-2 ) /2;
		y -= 1;
	}
	else
	{
		//TODO dark.hsv ( &h1, &s1, &v1 );
		//TODO foreground.hsv ( &h2, &s2, &v2 );
		ng = ( height-2 )- ( height-2 ) /2;
		y -= 1 + ( height-2 ) /2;
	}
	/*TODO return QColor ( h1 + ( ( h2-h1 ) *y ) / ( ng-1 ),
	                s1 + ( ( s2-s1 ) *y ) / ( ng-1 ),
	                v1 + ( ( v2-v1 ) *y ) / ( ng-1 ), QColor::Hsv );
					*/
	return QColor();
}

void Note::drawHandle ( QPainter *painter, int x, int y, int width, int height, const QColor &background, const QColor &foreground )
{
	QPen backgroundPen ( background );
	QPen foregroundPen ( foreground );

	QColor dark     = foreground.dark ( 110 );  // 1/1.1 of brightness
	QColor light    = foreground.light ( 150 ); // 50% brighter

	// Draw the surrounding rectangle:
	painter->setPen ( foregroundPen );
	painter->drawLine ( 0,         0,          width - 1, 0 );
	painter->drawLine ( 0,         0,          0,         height - 1 );
	painter->drawLine ( width - 1, 0,          width - 1, height - 1 );
	painter->drawLine ( 0,         height - 1, width - 1, height - 1 );

	// Draw the gradients:
	drawGradient ( painter, light, dark,       1 + x, 1 + y,                width-2, ( height-2 ) /2,            /*sunken=*/false, /*horz=*/true, /*flat=*/false );
	drawGradient ( painter, dark,  foreground, 1 + x, 1 + y + ( height-2 ) /2, width-2, ( height-2 )- ( height-2 ) /2, /*sunken=*/false, /*horz=*/true, /*flat=*/false );

	// Round the top corner with background color:
	painter->setPen ( backgroundPen );
	painter->drawLine ( 0, 0, 0, 3 );
	painter->drawLine ( 1, 0, 3, 0 );
	painter->drawPoint ( 1, 1 );
	// Round the bottom corner with background color:
	painter->drawLine ( 0, height-1, 0, height-4 );
	painter->drawLine ( 1, height-1, 3, height-1 );
	painter->drawPoint ( 1, height-2 );

	// Surrounding line of the rounded top-left corner:
	painter->setPen ( foregroundPen );
	painter->drawLine ( 1, 2, 1, 3 );
	painter->drawLine ( 2, 1, 3, 1 );

	// Anti-aliased rounded top corner (1/2):
	painter->setPen ( Tools::mixColor ( foreground, background ) );
	painter->drawPoint ( 0, 3 );
	painter->drawPoint ( 3, 0 );
	// Anti-aliased rounded bottom corner:
	painter->drawPoint ( 0, height - 4 );
	painter->drawPoint ( 3, height - 1 );
	// Anti-aliased rounded top corner (2/2):
	painter->setPen ( Tools::mixColor ( foreground, light ) );
	painter->drawPoint ( 2, 2 );

	// Draw the grips:
	int xGrips             = 4;
	int marginedHeight = ( height * 80 / 100 ); // 10% empty on top, and 10% empty on bottom, so 20% of the height should be empty of any grip, and 80% should be in the grips
	int nbGrips            = ( marginedHeight - 3 ) / 6;
	if ( nbGrips < 2 )
		nbGrips = 2;
	int yGrips             = ( height + 1 - nbGrips * 6 - 3 ) / 2; // +1 to avoid rounding errors, -nbGrips*6-3 the size of the grips
	QColor darker  = foreground.dark ( 130 );
	QColor lighter = foreground.light ( 130 );
	for ( int i = 0; i < nbGrips; ++i )
	{
		/// Dark color:
		painter->setPen ( darker );
		// Top-left point:
		painter->drawPoint ( xGrips,     yGrips );
		painter->drawPoint ( xGrips + 1, yGrips );
		painter->drawPoint ( xGrips,     yGrips + 1 );
		// Bottom-right point:
		painter->drawPoint ( xGrips + 4, yGrips + 3 );
		painter->drawPoint ( xGrips + 5, yGrips + 3 );
		painter->drawPoint ( xGrips + 4, yGrips + 4 );
		/// Light color:
		painter->setPen ( lighter );
		// Top-left point:
		painter->drawPoint ( xGrips + 1, yGrips + 1 );
		// Bottom-right point:
		painter->drawPoint ( xGrips + 5, yGrips + 4 );
		yGrips += 6;
	}
	// The remaining point:
	painter->setPen ( darker );
	painter->drawPoint ( xGrips,     yGrips );
	painter->drawPoint ( xGrips + 1, yGrips );
	painter->drawPoint ( xGrips,     yGrips + 1 );
	painter->setPen ( lighter );
	painter->drawPoint ( xGrips + 1, yGrips + 1 );
}

void Note::drawResizer ( QPainter *painter, int x, int y, int width, int height, const QColor &background, const QColor &foreground, bool rounded )
{
	QPen backgroundPen ( background );
	QPen foregroundPen ( foreground );

	QColor dark     = foreground.dark ( 110 );  // 1/1.1 of brightness
	QColor light    = foreground.light ( 150 ); // 50% brighter
	QColor midLight = foreground.light ( 105 ); // 5% brighter

	// Draw the surrounding rectangle:
	painter->setPen ( foregroundPen );
	painter->drawRect ( 0, 0, width, height );

	// Draw the gradients:
	drawGradient ( painter, light, dark,       1 + x, 1 + y,                width-2, ( height-2 ) /2,            /*sunken=*/false, /*horz=*/true, /*flat=*/false );
	drawGradient ( painter, dark,  foreground, 1 + x, 1 + y + ( height-2 ) /2, width-2, ( height-2 )- ( height-2 ) /2, /*sunken=*/false, /*horz=*/true, /*flat=*/false );

	if ( rounded )
	{
		// Round the top corner with background color:
		painter->setPen ( backgroundPen );
		painter->drawLine ( width - 1, 0, width - 3, 0 );
		painter->drawLine ( width - 1, 1, width - 1, 2 );
		painter->drawPoint ( width - 2, 1 );
		// Round the bottom corner with background color:
		painter->drawLine ( width - 1, height - 1, width - 1, height - 4 );
		painter->drawLine ( width - 2, height - 1, width - 4, height - 1 );
		painter->drawPoint ( width - 2, height-2 );

		// Surrounding line of the rounded top-left corner:
		painter->setPen ( foregroundPen );
		painter->drawLine ( width-2, 2, width-2, 3 );
		painter->drawLine ( width-3, 1, width-4, 1 );

		// Anti-aliased rounded top corner (1/2):
		painter->setPen ( Tools::mixColor ( foreground, background ) );
		painter->drawPoint ( width - 1, 3 );
		painter->drawPoint ( width - 4, 0 );
		// Anti-aliased rounded bottom corner:
		painter->drawPoint ( width - 1, height - 4 );
		painter->drawPoint ( width - 4, height - 1 );
		// Anti-aliased rounded top corner (2/2):
		painter->setPen ( Tools::mixColor ( foreground, light ) );
		painter->drawPoint ( width - 3, 2 );
	}

	// Draw the arows:
	int xArrow  = 2;
	int hMargin = 9;
	int countArrows = ( height >= hMargin*4 + 6*3 ? 3 : ( height >= hMargin*3 + 6*2 ? 2 : 1 ) );
	QColor darker  = foreground.dark ( 130 );
	QColor lighter = foreground.light ( 130 );
	for ( int i = 0; i < countArrows; ++i )
	{
		int yArrow;
		switch ( countArrows )
		{
			default:
			case 1: yArrow = ( height-6 ) / 2;                                                        break;
			case 2: yArrow = ( i == 1 ? hMargin : height - hMargin - 6 );                             break;
			case 3: yArrow = ( i == 1 ? hMargin : ( i == 2 ? ( height-6 ) / 2 : height - hMargin - 6 ) ); break;
		}
		/// Dark color:
		painter->setPen ( darker );
		// Left arrow:
		painter->drawLine ( xArrow, yArrow + 2, xArrow + 2, yArrow );
		painter->drawLine ( xArrow, yArrow + 2, xArrow + 2, yArrow + 4 );
		// Right arrow:
		painter->drawLine ( width - 1 - xArrow, yArrow + 2, width - 1 - xArrow - 2, yArrow );
		painter->drawLine ( width - 1 - xArrow, yArrow + 2, width - 1 - xArrow - 2, yArrow + 4 );
		/// Light color:
		painter->setPen ( lighter );
		// Left arrow:
		painter->drawLine ( xArrow, yArrow + 2 + 1, xArrow + 2, yArrow + 1 );
		painter->drawLine ( xArrow, yArrow + 2 + 1, xArrow + 2, yArrow + 4 + 1 );
		// Right arrow:
		painter->drawLine ( width - 1 - xArrow, yArrow + 2 + 1, width - 1 - xArrow - 2, yArrow + 1 );
		painter->drawLine ( width - 1 - xArrow, yArrow + 2 + 1, width - 1 - xArrow - 2, yArrow + 4 + 1 );
	}
}

void Note::drawInactiveResizer ( QPainter *painter, int x, int y, int height, const QColor &background, bool column )
{
	// If background color is too dark, we compute a lighter color instead of a darker:
	QColor darkBgColor = ( Tools::tooDark ( background ) ? background.light ( 120 ) : background.dark ( 105 ) );
	if ( column )
	{
		int halfWidth = RESIZER_WIDTH / 2;
		drawGradient ( painter, darkBgColor, background,  x,         y, halfWidth,                 height, /*sunken=*/false, /*horz=*/false, /*flat=*/false );
		drawGradient ( painter, background,  darkBgColor, halfWidth, y, RESIZER_WIDTH - halfWidth, height, /*sunken=*/false, /*horz=*/false, /*flat=*/false );
	}
	else
		drawGradient ( painter, darkBgColor, background,  x,         y, RESIZER_WIDTH,             height, /*sunken=*/false, /*horz=*/false, /*flat=*/false );
}


#include <qimage.h>
//#include <kimageeffect.h>

/* type: 1: topLeft
 *       2: bottomLeft
 *       3: topRight
 *       4: bottomRight
 *       5: fourCorners
 *       6: noteInsideAndOutsideCorners
 * (x,y) relate to the painter origin
 * (width,height) only used for 5:fourCorners type
 */
void Note::drawRoundings ( QPainter *painter, int x, int y, int type, int width, int height )
{
	int right;

	switch ( type )
	{
		case 1:
			x += this->x();
			y += this->y();
			basket()->blendBackground ( *painter, QRect ( x, y,     4, 1 ), this->x(), this->y() );
			basket()->blendBackground ( *painter, QRect ( x, y + 1, 2, 1 ), this->x(), this->y() );
			basket()->blendBackground ( *painter, QRect ( x, y + 2, 1, 1 ), this->x(), this->y() );
			basket()->blendBackground ( *painter, QRect ( x, y + 3, 1, 1 ), this->x(), this->y() );
			break;
		case 2:
			x += this->x();
			y += this->y();
			basket()->blendBackground ( *painter, QRect ( x, y - 1, 1, 1 ), this->x(), this->y() );
			basket()->blendBackground ( *painter, QRect ( x, y,     1, 1 ), this->x(), this->y() );
			basket()->blendBackground ( *painter, QRect ( x, y + 1, 2, 1 ), this->x(), this->y() );
			basket()->blendBackground ( *painter, QRect ( x, y + 2, 4, 1 ), this->x(), this->y() );
			break;
		case 3:
			right = rightLimit();
			x += right;
			y += this->y();
			basket()->blendBackground ( *painter, QRect ( x - 1, y,     4, 1 ), right, this->y() );
			basket()->blendBackground ( *painter, QRect ( x + 1, y + 1, 2, 1 ), right, this->y() );
			basket()->blendBackground ( *painter, QRect ( x + 2, y + 2, 1, 1 ), right, this->y() );
			basket()->blendBackground ( *painter, QRect ( x + 2, y + 3, 1, 1 ), right, this->y() );
			break;
		case 4:
			right = rightLimit();
			x += right;
			y += this->y();
			basket()->blendBackground ( *painter, QRect ( x + 2, y - 1, 1, 1 ), right, this->y() );
			basket()->blendBackground ( *painter, QRect ( x + 2, y,     1, 1 ), right, this->y() );
			basket()->blendBackground ( *painter, QRect ( x + 1, y + 1, 2, 1 ), right, this->y() );
			basket()->blendBackground ( *painter, QRect ( x - 1, y + 2, 4, 1 ), right, this->y() );
			break;
		case 5:
			// First make sure the corners are white (depending on the widget style):
			painter->setPen ( basket()->backgroundColor() );
			painter->drawPoint ( x,             y );
			painter->drawPoint ( x + width - 1, y );
			painter->drawPoint ( x + width - 1, y + height - 1 );
			painter->drawPoint ( x,             y + height - 1 );
			// And then blend corners:
			x += this->x();
			y += this->y();
			basket()->blendBackground ( *painter, QRect ( x,             y,              1, 1 ), this->x(), this->y() );
			basket()->blendBackground ( *painter, QRect ( x + width - 1, y,              1, 1 ), this->x(), this->y() );
			basket()->blendBackground ( *painter, QRect ( x + width - 1, y + height - 1, 1, 1 ), this->x(), this->y() );
			basket()->blendBackground ( *painter, QRect ( x,             y + height - 1, 1, 1 ), this->x(), this->y() );
			break;
		case 6:
			x += this->x();
			y += this->y();
			//if (!isSelected()) {
			// Inside left corners:
			basket()->blendBackground ( *painter, QRect ( x + HANDLE_WIDTH + 1, y + 1,          1, 1 ), this->x(), this->y() );
			basket()->blendBackground ( *painter, QRect ( x + HANDLE_WIDTH,     y + 2,          1, 1 ), this->x(), this->y() );
			basket()->blendBackground ( *painter, QRect ( x + HANDLE_WIDTH + 1, y + height - 2, 1, 1 ), this->x(), this->y() );
			basket()->blendBackground ( *painter, QRect ( x + HANDLE_WIDTH,     y + height - 3, 1, 1 ), this->x(), this->y() );
			// Inside right corners:
			basket()->blendBackground ( *painter, QRect ( x + width - 4,        y + 1,          1, 1 ), this->x(), this->y() );
			basket()->blendBackground ( *painter, QRect ( x + width - 3,        y + 2,          1, 1 ), this->x(), this->y() );
			basket()->blendBackground ( *painter, QRect ( x + width - 4,        y + height - 2, 1, 1 ), this->x(), this->y() );
			basket()->blendBackground ( *painter, QRect ( x + width - 3,        y + height - 3, 1, 1 ), this->x(), this->y() );
			//}
			// Outside right corners:
			basket()->blendBackground ( *painter, QRect ( x + width - 1,        y,              1, 1 ), this->x(), this->y() );
			basket()->blendBackground ( *painter, QRect ( x + width - 1,        y + height - 1, 1, 1 ), this->x(), this->y() );
			break;
	}
}

/// Blank Spaces Drawing:

void Note::setOnTop ( bool onTop )
{
	m_onTop = onTop;

	Note *note = firstChild();
	while ( note )
	{
		note->setOnTop ( onTop );
		note = note->next();
	}
}

void substractRectOnAreas ( const QRect &rectToSubstract, QList<QRect> &areas, bool andRemove )
{
	for ( QList<QRect>::iterator it = areas.begin(); it != areas.end(); )
	{
		QRect &rect = *it;
		// Split the rectangle if it intersects with rectToSubstract:
		if ( rect.intersects ( rectToSubstract ) )
		{
			// Create the top rectangle:
			if ( rectToSubstract.top() > rect.top() )
			{
				areas.insert ( it, QRect ( rect.left(), rect.top(), rect.width(), rectToSubstract.top() - rect.top() ) );
				rect.setTop ( rectToSubstract.top() );
			}
			// Create the bottom rectangle:
			if ( rectToSubstract.bottom() < rect.bottom() )
			{
				areas.insert ( it, QRect ( rect.left(), rectToSubstract.bottom() + 1, rect.width(), rect.bottom() - rectToSubstract.bottom() ) );
				rect.setBottom ( rectToSubstract.bottom() );
			}
			// Create the left rectangle:
			if ( rectToSubstract.left() > rect.left() )
			{
				areas.insert ( it, QRect ( rect.left(), rect.top(), rectToSubstract.left() - rect.left(), rect.height() ) );
				rect.setLeft ( rectToSubstract.left() );
			}
			// Create the right rectangle:
			if ( rectToSubstract.right() < rect.right() )
			{
				areas.insert ( it, QRect ( rectToSubstract.right() + 1, rect.top(), rect.right() - rectToSubstract.right(), rect.height() ) );
				rect.setRight ( rectToSubstract.right() );
			}
			// Remove the rectangle if it's entirely contained:
			if ( andRemove && rectToSubstract.contains ( rect ) )
				it = areas.erase ( it );
			else
				++it;
		}
		else
			++it;
	}
}

void Note::recomputeAreas()
{
	// Initialize the areas with the note rectangle(s):
	m_areas.clear();
	m_areas.append ( visibleRect() );
	if ( hasResizer() )
		m_areas.append ( resizerRect() );

	// Cut the areas where other notes are on top of this note:
	Note *note = basket()->firstNote();
	bool noteIsAfterThis = false;
	while ( note )
	{
		noteIsAfterThis = recomputeAreas ( note, noteIsAfterThis );
		note = note->next();
	}
}

bool Note::recomputeAreas ( Note *note, bool noteIsAfterThis )
{
	if ( note == this )
		noteIsAfterThis = true;
	// Only compute overlapping of notes AFTER this, or ON TOP this:
	//else if ( note->matching() && noteIsAfterThis && (!isOnTop() || (isOnTop() && note->isOnTop())) || (!isOnTop() && note->isOnTop()) ) {
	else if ( note->matching() && noteIsAfterThis && ( ! ( isOnTop() || isEditing() ) || ( ( isOnTop() || isEditing() ) && ( note->isOnTop() || note->isEditing() ) ) ) ||
	          ( ! ( isOnTop() || isEditing() ) && ( note->isOnTop() || note->isEditing() ) ) )
	{
		//if (!(isSelected() && !note->isSelected())) { // FIXME: FIXME: FIXME: FIXME: This last condition was added LATE, so we should look if it's ALWAYS good:
		substractRectOnAreas ( note->visibleRect(), m_areas, true );
		if ( note->hasResizer() )
			substractRectOnAreas ( note->resizerRect(), m_areas, true );
		//}
	}

	if ( note->isGroup() )
	{
		Note *child = note->firstChild();
		bool first = true;
		while ( child )
		{
			if ( ( note->showSubNotes() || first ) && note->matching() )
				noteIsAfterThis = recomputeAreas ( child, noteIsAfterThis );
			child = child->next();
			first = false;
		}
	}

	return noteIsAfterThis;
}

bool Note::isEditing()
{
	return basket()->editedNote() == this;
}

void Note::getGradientColors ( const QColor &originalBackground, QColor *colorTop, QColor *colorBottom )
{
	bool wasTooDark = Tools::tooDark ( originalBackground );
	if ( wasTooDark )
	{
		*colorTop    = originalBackground;
		*colorBottom = originalBackground.light ( 120 );
	}
	else
	{
		*colorTop    = originalBackground.dark ( 105 );
		*colorBottom = originalBackground;
	}
}

/* Drawing policy:
 * ==============
 * - Draw the note on a pixmap and then draw the pixmap on screen is faster and flicker-free, rather than drawing directly on screen
 * - The next time the pixmap can be directly redrawn on screen without (relatively low, for small texts) time-consuming text-drawing
 * - To keep memory footprint low, we can destruct the bufferPixmap because redrawing it offscreen and applying it onscreen is nearly as fast as just drawing the pixmap onscreen
 * - But as drawing the pixmap offscreen is little time consuming we can keep last visible notes buffered and then the redraw of the entire window is INSTANTANEOUS
 * - We keep bufferized note/group draws BUT NOT the resizer: such objects are small and fast to draw, so we don't complexify code for that
 */
void Note::draw ( QPainter *painter, const QRect &clipRect )
{
	kDebug() << "enter" << endl;
	if ( !matching() )
		return;

	/** Paint childs: */
	if ( isGroup() )
	{
		Note *child = firstChild();
		bool first = true;
		while ( child )
		{
			if ( ( showSubNotes() || first ) && child->matching() )
				child->draw ( painter, clipRect );
			child = child->next();
			first = false;
		}
	}

	QRect myRect ( x(), y(), width(), height() );
	/** Paint the resizer if needed: */
	if ( hasResizer() )
	{
		int right = rightLimit();
		QRect resizerRect ( right, y(), RESIZER_WIDTH, resizerHeight() );
		if ( resizerRect.intersects ( clipRect ) )
		{
			// Prepare to draw the resizer:
			QPixmap pixmap ( RESIZER_WIDTH, resizerHeight() );
			QPainter painter2 ( &pixmap );
			// Draw gradient or resizer:
			if ( m_hovered && m_hoveredZone == Resizer )
			{
				QColor baseColor ( basket()->backgroundColor() );
				//TODO QColor highColor ( KGlobalSettings::highlightColor() );
				QRgb highColor;
				drawResizer ( &painter2, 0, 0, RESIZER_WIDTH, resizerHeight(), baseColor, highColor, /*rounded=*/!isColumn() );
				if ( !isColumn() )
				{
					drawRoundings ( &painter2, RESIZER_WIDTH - 3, 0,                   /*type=*/3 );
					drawRoundings ( &painter2, RESIZER_WIDTH - 3, resizerHeight() - 3, /*type=*/4 );
				}
			}
			else
			{
				drawInactiveResizer ( &painter2, /*x=*/0, /*y=*/0, /*height=*/resizerHeight(), basket()->backgroundColor(), isColumn() );
				basket()->blendBackground ( painter2, resizerRect );
			}
			// Draw inserter:
			if ( basket()->inserterShown() && resizerRect.intersects ( basket()->inserterRect() ) )
				basket()->drawInserter ( painter2, right, y() );
			// Draw selection rect:
			if ( basket()->isSelecting() && resizerRect.intersects ( basket()->selectionRect() ) )
			{
				QRect selectionRect = basket()->selectionRect();
				selectionRect.moveTo ( -right, -y() );
				QRect selectionRectInside ( selectionRect.x() + 1, selectionRect.y() + 1, selectionRect.width() - 2, selectionRect.height() - 2 );
				if ( selectionRectInside.width() > 0 && selectionRectInside.height() > 0 )
				{
					QColor insideColor = basket()->selectionRectInsideColor();
					QColor darkInsideColor ( insideColor.dark ( 105 ) );
					painter2.setClipRect ( selectionRectInside );
					if ( isColumn() )
					{
						int halfWidth = RESIZER_WIDTH / 2;
						drawGradient ( &painter2, darkInsideColor, insideColor,     0,         0, halfWidth,               resizerHeight(), /*sunken=*/false, /*horz=*/false, /*flat=*/false );
						drawGradient ( &painter2, insideColor,     darkInsideColor, halfWidth, 0, RESIZER_WIDTH-halfWidth, resizerHeight(), /*sunken=*/false, /*horz=*/false, /*flat=*/false );
					}
					else
						drawGradient ( &painter2, darkInsideColor, insideColor, 0, 0, RESIZER_WIDTH, resizerHeight(), /*sunken=*/false, /*horz=*/false, /*flat=*/false );
					painter2.setClipping ( false );
					selectionRectInside.moveTo ( right, y() );
					basket()->blendBackground ( painter2, selectionRectInside, right, y(), false );
				}
				//TODO painter2.setPen ( KGlobalSettings::highlightColor().dark() );
				painter2.drawRect ( selectionRect );
				//TODO painter2.setPen ( Tools::mixColor ( KGlobalSettings::highlightColor().dark(), basket()->backgroundColor() ) );
				painter2.drawPoint ( selectionRect.topLeft() );
				painter2.drawPoint ( selectionRect.topRight() );
				painter2.drawPoint ( selectionRect.bottomLeft() );
				painter2.drawPoint ( selectionRect.bottomRight() );
			}
			// Draw on screen:
			painter2.end();
			/** Compute visible areas: */
			if ( ! m_computedAreas )
				recomputeAreas();
			if ( m_areas.isEmpty() )
				return;
			for ( QList<QRect>::iterator it = m_areas.begin(); it != m_areas.end(); ++it )
			{
				QRect &rect = *it;
				painter->drawPixmap ( rect.x(), rect.y(), pixmap, rect.x() - right, rect.y() - y(), rect.width(), rect.height() );
			}
		}
	}

	/** Then, draw the note/group ONLY if needed: */
	if ( ! myRect.intersects ( clipRect ) )
		return;

	/** Compute visible areas: */
	if ( ! m_computedAreas )
		recomputeAreas();
	if ( m_areas.isEmpty() )
		return;

	/** Directly draw pixmap on screen if it is already buffered: */
	if ( isBufferized() )
	{
		drawBufferOnScreen ( painter, m_bufferedPixmap );
		return;
	}

	/** Initialise buffer painter: */
	//TODO m_bufferedPixmap.resize ( width(), height() );
	QPainter painter2 ( &m_bufferedPixmap );

	/** Initialise colors: */
	QColor baseColor ( basket()->backgroundColor() );
	//TODO QColor highColor ( KGlobalSettings::highlightColor() );
	QColor highColor;
	//TODO QColor midColor = Tools::mixColor ( baseColor, highColor );
	QColor midColor;

	/** Initialise brushs and pens: */
	QBrush baseBrush ( baseColor );
	QBrush highBrush ( highColor );
	QPen   basePen ( baseColor );
	QPen   highPen ( highColor );
	QPen   midPen ( midColor );

	/** Figure out the state of the note: */
	bool hovered = m_hovered && m_hoveredZone != TopInsert && m_hoveredZone != BottomInsert && m_hoveredZone != Resizer;

	/** And then draw the group: */
	if ( isGroup() )
	{
		// Draw background or handle:
		if ( hovered )
		{
			drawHandle ( &painter2, 0, 0, width(), height(), baseColor, highColor );
			drawRoundings ( &painter2, 0, 0,            /*type=*/1 );
			drawRoundings ( &painter2, 0, height() - 3, /*type=*/2 );
		}
		else
		{
			painter2.fillRect ( 0, 0, width(), height(), baseBrush );
			basket()->blendBackground ( painter2, myRect, -1, -1, /*opaque=*/true );
		}

		// Draw expander:
		int yExp = yExpander();
		drawExpander ( &painter2, NOTE_MARGIN, yExp, ( hovered ? expanderBackground ( height(), yExp+EXPANDER_HEIGHT/2, highColor ) : baseColor ), m_isFolded, basket() );
		// Draw expander rounded edges:
		if ( hovered )
		{
			QColor color1 = expanderBackground ( height(), yExp,                       highColor );
			QColor color2 = expanderBackground ( height(), yExp + EXPANDER_HEIGHT - 1, highColor );
			painter2.setPen ( color1 );
			painter2.drawPoint ( NOTE_MARGIN,         yExp );
			painter2.drawPoint ( NOTE_MARGIN + 9 - 1, yExp );
			painter2.setPen ( color2 );
			painter2.drawPoint ( NOTE_MARGIN,         yExp + 9 - 1 );
			painter2.drawPoint ( NOTE_MARGIN + 9 - 1, yExp + 9 - 1 );
		}
		else
			drawRoundings ( &painter2, NOTE_MARGIN, yExp, /*type=*/5, 9, 9 );

		// Draw on screen:
		painter2.end();
		drawBufferOnScreen ( painter, m_bufferedPixmap );
		return;
	}

	/** Or draw the note: */
	// What are the background colors:
	QColor background = basket()->backgroundColor();
	if ( isSelected() )
		/*TODO if ( m_computedState.backgroundColor().isValid() )
			background = Tools::mixColor ( Tools::mixColor ( m_computedState.backgroundColor(), KGlobalSettings::highlightColor() ), KGlobalSettings::highlightColor() );
		else
			background = KGlobalSettings::highlightColor();*/
		;
	else if ( m_computedState.backgroundColor().isValid() )
		background = m_computedState.backgroundColor();
	QColor bgColor;
	QColor darkBgColor;
	getGradientColors ( background, &darkBgColor, &bgColor );
	// Draw background (color, gradient and pixmap):
	drawGradient ( &painter2, bgColor, darkBgColor, 0, 0, width(), height(), /*sunken=*/!hovered, /*horz=*/true, /*flat=*/false );
	if ( !hovered )
	{
		painter2.setPen ( Tools::mixColor ( bgColor, darkBgColor ) );
		painter2.drawLine ( 0, height() - 1, width(), height() - 1 );
	}
	basket()->blendBackground ( painter2, myRect );

	if ( hovered )
	{
		// Top/Bottom lines:
		painter2.setPen ( highPen );
		painter2.drawLine ( 0, height() - 1, width(), height() - 1 );
		painter2.drawLine ( 0, 0,            width(), 0 );
		// The handle:
		drawHandle ( &painter2, 0, 0, HANDLE_WIDTH, height(), baseColor, highColor );
		drawRoundings ( &painter2, 0, 0,            /*type=*/1 );
		drawRoundings ( &painter2, 0, height() - 3, /*type=*/2 );
		// Round handle-right-side border:
		painter2.setPen ( highPen );
		painter2.drawPoint ( HANDLE_WIDTH, 1 );
		painter2.drawPoint ( HANDLE_WIDTH, height() - 2 );
		// Light handle top-right round corner:
		painter2.setPen ( QPen ( highColor.light ( 150 ) ) );
		painter2.drawPoint ( HANDLE_WIDTH - 1, 1 );
		// Handle anti-aliased rounded handle-right-side corners:
		QColor insideMidColor = Tools::mixColor ( bgColor, highColor );
		painter2.setPen ( insideMidColor );
		// Left inside round corners:
		painter2.drawPoint ( HANDLE_WIDTH + 1, 1 );
		painter2.drawPoint ( HANDLE_WIDTH,     2 );
		painter2.drawPoint ( HANDLE_WIDTH + 1, height() - 2 );
		painter2.drawPoint ( HANDLE_WIDTH,     height() - 3 );
		// Right inside round corners:
		painter2.drawPoint ( width() - 4, 1 );
		painter2.drawPoint ( width() - 3, 2 );
		painter2.drawPoint ( width() - 4, height() - 2 );
		painter2.drawPoint ( width() - 3, height() - 3 );
		// Right rounded edge:
		painter2.setPen ( highPen );
		painter2.fillRect ( width() - 2, 0, 2, height(), highBrush );
		painter2.drawPoint ( width() - 3, 1 );
		painter2.drawPoint ( width() - 3, height() - 2 );
		// Right anti-aliased rounded edge:
		painter2.setPen ( midPen );
		painter2.drawPoint ( width() - 1, 0 );
		painter2.drawPoint ( width() - 1, height() - 1 );
		// Blend background pixmap:
		drawRoundings ( &painter2, 0, 0, /*type=*/6, width(), height() );
	}

	if ( isFocused() )
	{
		QRect focusRect ( HANDLE_WIDTH, NOTE_MARGIN - 1, width() - HANDLE_WIDTH - 2, height() - 2*NOTE_MARGIN + 2 );
		//painter2.drawWinFocusRect ( focusRect );
	}

	// Draw the Emblems:
	int yIcon = ( height() - EMBLEM_SIZE ) / 2;
	int xIcon = HANDLE_WIDTH + NOTE_MARGIN;
	for ( State::List::Iterator it = m_states.begin(); it != m_states.end(); ++it )
	{
		if ( ! ( *it )->emblem().isEmpty() )
		{
			//TODO QPixmap stateEmblem = KIconLoader::global()->loadIcon ( ( *it )->emblem(), KIconLoader::NoGroup, 16, KIconLoader::DefaultState, 0L, false );
			QPixmap stateEmblem = KIconLoader::global()->loadIcon( ( *it )->emblem(), KIconLoader::NoGroup );
			painter2.drawPixmap ( xIcon, yIcon, stateEmblem );
			xIcon += NOTE_MARGIN + EMBLEM_SIZE;
		}
	}

	// Determine the colors (for the richText drawing) and the text color (for the tags arrow too):
	//TODO QPalette cg ( basket()->colorGroup() );
	QPalette cg;
	cg.setColor ( QPalette::Text, ( m_computedState.textColor().isValid() ? m_computedState.textColor() : basket()->textColor() ) );
	cg.setColor ( QPalette::Background, bgColor );
	if ( isSelected() )
		;//TODO cg.setColor ( QPalette::Text, KGlobalSettings::highlightedTextColor() );

	// Draw the Tags Arrow:
	if ( hovered )
	{
		QColor textColor = cg.color ( QPalette::Text );
		QColor light     = Tools::mixColor ( textColor, bgColor );
		QColor mid       = Tools::mixColor ( textColor, light );
		painter2.setPen ( light );//QPen(basket()->colorGroup().dark().light(150)));
		painter2.drawLine ( xIcon,      yIcon + 6, xIcon + 4, yIcon + 6 );
		painter2.setPen ( mid );//QPen(basket()->colorGroup().dark()));
		painter2.drawLine ( xIcon + 1,  yIcon + 7, xIcon + 3, yIcon + 7 );
		painter2.setPen ( textColor );//QPen(basket()->colorGroup().foreground()));
		painter2.drawPoint ( xIcon + 2, yIcon + 8 );
	}
	else if ( m_haveInvisibleTags )
	{
		painter2.setPen ( cg.color ( QPalette::Text ) /*QPen(basket()->colorGroup().foreground())*/ );
		painter2.drawPoint ( xIcon,     yIcon + 7 );
		painter2.drawPoint ( xIcon + 2, yIcon + 7 );
		painter2.drawPoint ( xIcon + 4, yIcon + 7 );
	}

	// Draw content:
	// Optimization: do not draw text notes because it is time consuming and should be done nearly at each tetx modification.
	if ( basket()->editedNote() != this || basket()->editedNote()->content()->type() != NoteType::Html )
	{
		painter2.translate ( contentX(), NOTE_MARGIN );
		painter2.setFont ( m_computedState.font ( painter2.font() ) );
		m_content->paint ( &painter2, width() - contentX() - NOTE_MARGIN, height() - 2*NOTE_MARGIN, cg, !m_computedState.textColor().isValid(), isSelected(), hovered );
	}

	// Draw on screen:
	painter2.end();
	drawBufferOnScreen ( painter, m_bufferedPixmap );
}

void Note::drawBufferOnScreen ( QPainter *painter, const QPixmap &contentPixmap )
{
	kDebug() << "enter" << endl;
	for ( QList<QRect>::iterator it = m_areas.begin(); it != m_areas.end(); ++it )
	{
		QRect &rect = *it;
		if ( rect.x() >= x() + width() ) // It's a rect of the resizer, don't draw it!
			continue;
		// If the inserter is above the note, draw it, BUT NOT in the buffer pixmap,
		// we copy the rectangle in a new pixmap, apply the inserter and then draw this new pixmap on screen:
		if ( ( basket()->inserterShown() && rect.intersects ( basket()->inserterRect() ) )  ||
		        ( basket()->isSelecting()   && rect.intersects ( basket()->selectionRect() ) ) )
		{
			QPixmap pixmap3 ( rect.width(), rect.height() );
			QPainter painter3 ( &pixmap3 );
			painter3.drawPixmap ( 0, 0, contentPixmap, rect.x() - x(), rect.y() - y(), rect.width(), rect.height() );
			// Draw inserter:
			if ( basket()->inserterShown() && rect.intersects ( basket()->inserterRect() ) )
				basket()->drawInserter ( painter3, rect.x(), rect.y() );
			// Draw selection rect:
			if ( basket()->isSelecting() && rect.intersects ( basket()->selectionRect() ) )
			{
				QRect selectionRect = basket()->selectionRect();
				selectionRect.moveTo ( -rect.x(), -rect.y() );

				QRect selectionRectInside ( selectionRect.x() + 1, selectionRect.y() + 1, selectionRect.width() - 2, selectionRect.height() - 2 );
				if ( selectionRectInside.width() > 0 && selectionRectInside.height() > 0 )
				{
					bufferizeSelectionPixmap();
					selectionRectInside.moveTo ( rect.x(), rect.y() );
					QRect rectToPaint = rect.intersect ( selectionRectInside );
					rectToPaint.moveTo ( -x(), -y() );
					painter3.drawPixmap ( rectToPaint.topLeft() + QPoint ( x(), y() ) - rect.topLeft(), m_bufferedSelectionPixmap, rectToPaint );
					//blendBackground(painter2, selectionRectInside, rect.x(), rect.y(), true, &m_selectedBackgroundPixmap);
				}

				//TODO painter3.setPen ( KGlobalSettings::highlightColor().dark() );
				painter3.drawRect ( selectionRect );
				if ( isGroup() )
					; //TODO painter3.setPen ( Tools::mixColor ( KGlobalSettings::highlightColor().dark(), basket()->backgroundColor() ) );
				else
				{
					// What are the background colors:
					QColor bgColor = basket()->backgroundColor();
					if ( isSelected() )
						;//TODO bgColor = ( m_computedState.backgroundColor().isValid() ? Tools::mixColor ( Tools::mixColor ( m_computedState.backgroundColor(), KGlobalSettings::highlightColor() ), KGlobalSettings::highlightColor() ) : KGlobalSettings::highlightColor() );
					else if ( m_computedState.backgroundColor().isValid() )
						bgColor = m_computedState.backgroundColor();
					//TODO painter3.setPen ( Tools::mixColor ( KGlobalSettings::highlightColor().dark(), bgColor ) );
				}
				painter3.drawPoint ( selectionRect.topLeft() );
				painter3.drawPoint ( selectionRect.topRight() );
				painter3.drawPoint ( selectionRect.bottomLeft() );
				painter3.drawPoint ( selectionRect.bottomRight() );
			}
			painter3.end();
			painter->drawPixmap ( rect.x(), rect.y(), pixmap3 );
			// Else, draw the rect pixmap directly on screen:
		}
		else
			painter->drawPixmap ( rect.x(), rect.y(), contentPixmap, rect.x() - x(), rect.y() - y(), rect.width(), rect.height() );
	}
}

void Note::setContent ( NoteContent *content )
{
	m_content = content;
}

/*const */State::List& Note::states() const
{
	return ( State::List& ) m_states;
}

void Note::addState ( State *state, bool orReplace )
{
	if ( !content() )
		return;

	Tag *tag = state->parentTag();
	State::List::iterator itStates = m_states.begin();
	// Browse all tags, see if the note has it, increment itSates if yes, and then insert the state at this position...
	// For each existing tags:
	for ( Tag::List::iterator it = Tag::all.begin(); it != Tag::all.end(); ++it )
	{
		// If the current tag isn't the one to assign or the current one on the note, go to the next tag:
		if ( *it != tag && itStates != m_states.end() && *it != ( *itStates )->parentTag() )
			continue;
		// We found the tag to insert:
		if ( *it == tag )
		{
			// And the note already have the tag:
			if ( itStates != m_states.end() && *it == ( *itStates )->parentTag() )
			{
				// We replace the state if wanted:
				if ( orReplace )
				{
					itStates = m_states.insert ( itStates, state );
					++itStates;
					m_states.erase ( itStates );
					recomputeStyle();
				}
			}
			else
			{
				m_states.insert ( itStates, state );
				recomputeStyle();
			}
			return;
		}
		// The note has this tag:
		if ( itStates != m_states.end() && *it == ( *itStates )->parentTag() )
			++itStates;
	}
}

QFont Note::font()
{
	//TODO return m_computedState.font ( basket()->QScrollView::font() );
	return m_computedState.font( basket()->font() );
}

QColor Note::backgroundColor()
{
	if ( m_computedState.backgroundColor().isValid() )
		return m_computedState.backgroundColor();
	else
		return basket()->backgroundColor();
}

QColor Note::textColor()
{
	if ( m_computedState.textColor().isValid() )
		return m_computedState.textColor();
	else
		return basket()->textColor();
}

void Note::recomputeStyle()
{
	State::merge ( m_states, &m_computedState, &m_emblemsCount, &m_haveInvisibleTags, basket()->backgroundColor() );
//	unsetWidth();
	if ( content() )
		content()->fontChanged();
//	requestRelayout(); // TODO!
}

void Note::recomputeAllStyles()
{
	if ( content() ) // We do the merge ourself, without calling recomputeStyle(), so there is no infinite recursion:
		//State::merge(m_states, &m_computedState, &m_emblemsCount, &m_haveInvisibleTags, basket()->backgroundColor());
		recomputeStyle();
	else if ( isGroup() )
		FOR_EACH_CHILD ( child )
		child->recomputeAllStyles();
}

bool Note::removedStates ( const QList<State*> &deletedStates )
{
	bool modifiedBasket = false;

	if ( !states().isEmpty() )
	{
		for ( QList<State*>::const_iterator it = deletedStates.begin(); it != deletedStates.end(); ++it )
			if ( hasState ( *it ) )
			{
				removeState ( *it );
				modifiedBasket = true;
			}
	}

	FOR_EACH_CHILD ( child )
	if ( child->removedStates ( deletedStates ) )
		modifiedBasket = true;

	return modifiedBasket;
}


void Note::addTag ( Tag *tag )
{
	addState ( tag->states().first(), /*but do not replace:*/false );
}

void Note::removeState ( State *state )
{
	for ( State::List::iterator it = m_states.begin(); it != m_states.end(); ++it )
		if ( *it == state )
		{
			m_states.erase ( it );
			recomputeStyle();
			return;
		}
}

void Note::removeTag ( Tag *tag )
{
	for ( State::List::iterator it = m_states.begin(); it != m_states.end(); ++it )
		if ( ( *it )->parentTag() == tag )
		{
			m_states.erase ( it );
			recomputeStyle();
			return;
		}
}

void Note::removeAllTags()
{
	m_states.clear();
	recomputeStyle();
}

void Note::addTagToSelectedNotes ( Tag *tag )
{
	if ( content() && isSelected() )
		addTag ( tag );

	FOR_EACH_CHILD ( child )
	child->addTagToSelectedNotes ( tag );
}

void Note::removeTagFromSelectedNotes ( Tag *tag )
{
	if ( content() && isSelected() )
	{
		if ( hasTag ( tag ) )
			setWidth ( 0 );
		removeTag ( tag );
	}

	FOR_EACH_CHILD ( child )
	child->removeTagFromSelectedNotes ( tag );
}

void Note::removeAllTagsFromSelectedNotes()
{
	if ( content() && isSelected() )
	{
		if ( m_states.count() > 0 )
			setWidth ( 0 );
		removeAllTags();
	}

	FOR_EACH_CHILD ( child )
	child->removeAllTagsFromSelectedNotes();
}

void Note::addStateToSelectedNotes ( State *state, bool orReplace )
{
	if ( content() && isSelected() )
		addState ( state, orReplace );

	FOR_EACH_CHILD ( child )
	child->addStateToSelectedNotes ( state, orReplace ); // TODO: Basket::addStateToSelectedNotes() does not have orReplace
}

void Note::changeStateOfSelectedNotes ( State *state )
{
	if ( content() && isSelected() && hasTag ( state->parentTag() ) )
		addState ( state );

	FOR_EACH_CHILD ( child )
	child->changeStateOfSelectedNotes ( state );
}

bool Note::selectedNotesHaveTags()
{
	if ( content() && isSelected() && m_states.count() > 0 )
		return true;

	FOR_EACH_CHILD ( child )
	if ( child->selectedNotesHaveTags() )
		return true;
	return false;
}

bool Note::hasState ( State *state )
{
	for ( State::List::iterator it = m_states.begin(); it != m_states.end(); ++it )
		if ( *it == state )
			return true;
	return false;
}

bool Note::hasTag ( Tag *tag )
{
	for ( State::List::iterator it = m_states.begin(); it != m_states.end(); ++it )
		if ( ( *it )->parentTag() == tag )
			return true;
	return false;
}

State* Note::stateOfTag ( Tag *tag )
{
	for ( State::List::iterator it = m_states.begin(); it != m_states.end(); ++it )
		if ( ( *it )->parentTag() == tag )
			return *it;
	return 0;
}

State* Note::stateForEmblemNumber ( int number )
{
	int i = -1;
	for ( State::List::Iterator it = m_states.begin(); it != m_states.end(); ++it )
		if ( ! ( *it )->emblem().isEmpty() )
		{
			++i;
			if ( i == number )
				return *it;
		}
	return 0;
}

bool Note::stateForTagFromSelectedNotes ( Tag *tag, State **state )
{
	if ( content() && isSelected() )
	{
		// What state is the tag on this note?
		State* stateOfTag = this->stateOfTag ( tag );
		// This tag is not assigned to this note, the action will assign it, then:
		if ( stateOfTag == 0 )
			*state = 0;
		else
		{
			// Take the LOWEST state of all the selected notes:
			// Say the two selected notes have the state "Done" and "To Do".
			// The first note set *state to "Done".
			// When reaching the second note, we should recognize "To Do" is first in the tag states, then take it
			// Because pressing the tag shortcut key should first change state before removing the tag!
			if ( *state == 0 )
				*state = stateOfTag;
			else
			{
				bool stateIsFirst = true;
				for ( State *nextState = stateOfTag->nextState(); nextState; nextState = nextState->nextState ( /*cycle=*/false ) )
					if ( nextState == *state )
						stateIsFirst = false;
				if ( !stateIsFirst )
					*state = stateOfTag;
			}
		}
		return true; // We encountered a selected note
	}

	bool encounteredSelectedNote = false;
	FOR_EACH_CHILD ( child )
	{
		bool encountered = child->stateForTagFromSelectedNotes ( tag, state );
		if ( encountered && *state == 0 )
			return true;
		if ( encountered )
			encounteredSelectedNote = true;
	}
	return encounteredSelectedNote;
}


void Note::inheritTagsOf ( Note *note )
{
	if ( !note || !content() )
		return;

	for ( State::List::iterator it = note->states().begin(); it != note->states().end(); ++it )
		if ( ( *it )->parentTag() && ( *it )->parentTag()->inheritedBySiblings() )
			addTag ( ( *it )->parentTag() );
}

void Note::unbufferizeAll()
{
	unbufferize();

	if ( isGroup() )
	{
		Note *child = firstChild();
		while ( child )
		{
			child->unbufferizeAll();
			child = child->next();
		}
	}
}

void Note::bufferizeSelectionPixmap()
{
	if ( m_bufferedSelectionPixmap.isNull() )
	{
		QColor insideColor; //TODO = KGlobalSettings::highlightColor();
		QPixmap kpixmap ( m_bufferedPixmap );
		//TODO m_bufferedSelectionPixmap = QPixmapEffect::fade ( kpixmap, 0.25, insideColor );
	}
}

QRect Note::visibleRect()
{
	QList<QRect> areas;
	areas.append ( rect() );

	// When we are folding a parent group, if this note is bigger than the first real note of the group, cut the top of this:
	Note *parent = parentNote();
	while ( parent )
	{
		if ( parent->expandingOrCollapsing() )
			substractRectOnAreas ( QRect ( x(), parent->y() - height(), width(), height() ), areas, true );
		parent = parent->parentNote();
	}

	if ( areas.count() > 0 )
		return areas[0];
	else
		return QRect();
}

void Note::recomputeBlankRects ( QList<QRect> &blankAreas )
{
	if ( !matching() )
		return;

	// visibleRect() instead of rect() because if we are folding/expanding a smaller parent group, then some part is hidden!
	// But anyway, a resizer is always a primary note and is never hidden by a parent group, so no visibleResizerRect() method!
	substractRectOnAreas ( visibleRect(), blankAreas, true );
	if ( hasResizer() )
		substractRectOnAreas ( resizerRect(), blankAreas, true );

	if ( isGroup() )
	{
		Note *child = firstChild();
		bool first = true;
		while ( child )
		{
			if ( ( showSubNotes() || first ) && child->matching() )
				child->recomputeBlankRects ( blankAreas );
			child = child->next();
			first = false;
		}
	}
}

void Note::linkLookChanged()
{
	if ( isGroup() )
	{
		Note *child = firstChild();
		while ( child )
		{
			child->linkLookChanged();
			child = child->next();
		}
	}
	else
		content()->linkLookChanged();
}

Note* Note::noteForFullPath ( const QString &path )
{
	if ( content() && fullPath() == path )
		return this;

	Note *child = firstChild();
	Note *found;
	while ( child )
	{
		found = child->noteForFullPath ( path );
		if ( found )
			return found;
		child = child->next();
	}
	return 0;
}

void Note::listUsedTags ( QList<Tag*> &list )
{
	for ( State::List::Iterator it = m_states.begin(); it != m_states.end(); ++it )
	{
		Tag *tag = ( *it )->parentTag();
		if ( !list.contains ( tag ) )
			list.append ( tag );
	}

	FOR_EACH_CHILD ( child )
	child->listUsedTags ( list );
}


void Note::usedStates ( QList<State*> &states )
{
	if ( content() )
		for ( State::List::Iterator it = m_states.begin(); it != m_states.end(); ++it )
			if ( !states.contains ( *it ) )
				states.append ( *it );

	FOR_EACH_CHILD ( child )
	child->usedStates ( states );
}

Note* Note::nextInStack()
{
	// First, search in the childs:
	if ( firstChild() )
		if ( firstChild()->content() )
			return firstChild();
		else
			return firstChild()->nextInStack();

	// Then, in the next:
	if ( next() )
		if ( next()->content() )
			return next();
		else
			return next()->nextInStack();

	// And finally, in the parent:
	Note *note = parentNote();
	while ( note )
		if ( note->next() )
			if ( note->next()->content() )
				return note->next();
			else
				return note->next()->nextInStack();
		else
			note = note->parentNote();

	// Not found:
	return 0;
}

Note* Note::prevInStack()
{
	// First, search in the previous:
	if ( prev() && prev()->content() )
		return prev();

	// Else, it's a group, get the last item in that group:
	if ( prev() )
	{
		Note *note = prev()->lastRealChild();
		if ( note )
			return note;
	}

	if ( parentNote() )
		return parentNote()->prevInStack();
	else
		return 0;
}

Note* Note::nextShownInStack()
{
	Note *next = nextInStack();
	while ( next && !next->isVisible() )
		next = next->nextInStack();
	return next;
}

Note* Note::prevShownInStack()
{
	Note *prev = prevInStack();
	while ( prev && !prev->isVisible() )
		prev = prev->prevInStack();
	return prev;
}

bool Note::isVisible()
{
	// First, the easy one: groups are always shown:
	if ( isGroup() )
		return true;

	// And another easy part: non-matching notes are hidden:
	if ( !matching() )
		return false;

	if ( basket()->isFiltering() ) // And isMatching() because of the line above!
		return true;

	// So, here we go to the complexe case: if the note is inside a collapsed group:
	Note *group = parentNote();
	Note *child = this;
	while ( group )
	{
		if ( group->isFolded() && group->firstChild() != child )
			return false;
		child = group;
		group = group->parentNote();
	}
	return true;
}

void Note::debug()
{
	QString debugStr;
	QTextStream(&debugStr) << "Note@" << this;
	if ( this!=0 )
	{
		if ( isColumn() )
			QTextStream(&debugStr) << ": Column";
		else if ( isGroup() )
			QTextStream(&debugStr) << ": Group";
		else
			QTextStream(&debugStr) << ": Content[" << content()->lowerTypeName() << "]: " << toText ( "" );
	}

	kDebug() << debugStr;
}

Note* Note::firstSelected()
{
	if ( isSelected() )
		return this;

	Note *first = 0;
	FOR_EACH_CHILD ( child )
	{
		first = child->firstSelected();
		if ( first )
			break;
	}
	return first;
}

Note* Note::lastSelected()
{
	if ( isSelected() )
		return this;

	Note *last = 0, *tmp = 0;
	FOR_EACH_CHILD ( child )
	{
		tmp = child->lastSelected();
		if ( tmp )
			last = tmp;
	}
	return last;
}

Note* Note::selectedGroup()
{
	if ( isGroup() && allSelected() && count() == basket()->countSelecteds() )
		return this;

	FOR_EACH_CHILD ( child )
	{
		Note *selectedGroup = child->selectedGroup();
		if ( selectedGroup )
			return selectedGroup;
	}

	return 0;
}

void Note::groupIn ( Note *group )
{
	if ( this == group )
		return;

	if ( allSelected() && !isColumn() )
	{
		basket()->unplugNote ( this );
		basket()->insertNote ( this, group, Note::BottomColumn, QPoint(), /*animateNewPosition=*/true );
	}
	else
	{
		Note *next;
		Note *child = firstChild();
		while ( child )
		{
			next = child->next();
			child->groupIn ( group );
			child = next;
		}
	}
}

bool Note::tryExpandParent()
{
	Note *parent = parentNote();
	Note *child  = this;
	while ( parent )
	{
		if ( parent->firstChild() != child )
			return false;
		if ( parent->isColumn() )
			return false;
		if ( parent->isFolded() )
		{
			parent->toggleFolded ( true );
			basket()->relayoutNotes ( true );
			return true;
		}
		child  = parent;
		parent = parent->parentNote();
	}
	return false;
}

bool Note::tryFoldParent()        // TODO: withCtrl  ? withShift  ?
{
	Note *parent = parentNote();
	Note *child  = this;
	while ( parent )
	{
		if ( parent->firstChild() != child )
			return false;
		if ( parent->isColumn() )
			return false;
		if ( !parent->isFolded() )
		{
			parent->toggleFolded ( true );
			basket()->relayoutNotes ( true );
			return true;
		}
		child  = parent;
		parent = parent->parentNote();
	}
	return false;
}


int Note::distanceOnLeftRight ( Note *note, int side )
{
	if ( side == Basket::RIGHT_SIDE )
	{
		// If 'note' is on left of 'this': cannot switch from this to note by pressing Right key:
		if ( finalX() > note->finalX() || finalRightLimit() > note->finalRightLimit() )
			return -1;
	}
	else /*LEFT_SIDE:*/
	{
		// If 'note' is on left of 'this': cannot switch from this to note by pressing Right key:
		if ( finalX() < note->finalX() || finalRightLimit() < note->finalRightLimit() )
			return -1;
	}
	if ( finalX() == note->finalX() && finalRightLimit() == note->finalRightLimit() )
		return -1;

	float thisCenterX = finalX() + ( side == Basket::LEFT_SIDE ? width() : /*RIGHT_SIDE:*/ 0 );
	float thisCenterY = finalY() + finalHeight() / 2;
	float noteCenterX = note->finalX() + note->width() / 2;
	float noteCenterY = note->finalY() + note->finalHeight() / 2;

	if ( thisCenterY > note->finalBottom() )
		noteCenterY = note->finalBottom();
	else if ( thisCenterY < note->finalY() )
		noteCenterY = note->finalY();
	else
		noteCenterY = thisCenterY;

	float angle = 0;
	if ( noteCenterX - thisCenterX != 0 )
		angle = 1000 * ( ( noteCenterY - thisCenterY ) / ( noteCenterX - thisCenterX ) );
	if ( angle < 0 )
		angle = -angle;

	return int ( sqrt ( pow ( noteCenterX - thisCenterX, 2 ) + pow ( noteCenterY - thisCenterY, 2 ) ) + angle );
}

int Note::distanceOnTopBottom ( Note *note, int side )
{
	if ( side == Basket::BOTTOM_SIDE )
	{
		// If 'note' is on left of 'this': cannot switch from this to note by pressing Right key:
		if ( finalY() > note->finalY() || finalBottom() > note->finalBottom() )
			return -1;
	}
	else /*TOP_SIDE:*/
	{
		// If 'note' is on left of 'this': cannot switch from this to note by pressing Right key:
		if ( finalY() < note->finalY() || finalBottom() < note->finalBottom() )
			return -1;
	}
	if ( finalY() == note->finalY() && finalBottom() == note->finalBottom() )
		return -1;

	float thisCenterX = finalX() + width() / 2;
	float thisCenterY = finalY() + ( side == Basket::TOP_SIDE ? finalHeight() : /*BOTTOM_SIDE:*/ 0 );
	float noteCenterX = note->finalX() + note->width() / 2;
	float noteCenterY = note->finalY() + note->finalHeight() / 2;

	if ( thisCenterX > note->finalRightLimit() )
		noteCenterX = note->finalRightLimit();
	else if ( thisCenterX < note->finalX() )
		noteCenterX = note->finalX();
	else
		noteCenterX = thisCenterX;

	float angle = 0;
	if ( noteCenterX - thisCenterX != 0 )
		angle = 1000 * ( ( noteCenterY - thisCenterY ) / ( noteCenterX - thisCenterX ) );
	if ( angle < 0 )
		angle = -angle;

	return int ( sqrt ( pow ( noteCenterX - thisCenterX, 2 ) + pow ( noteCenterY - thisCenterY, 2 ) ) + angle );
}

Note* Note::parentPrimaryNote()
{
	Note *primary = this;
	while ( primary->parentNote() )
		primary = primary->parentNote();
	return primary;
}

void Note::deleteChilds()
{
	Note *child = firstChild();

	while ( child )
	{
		Note *tmp = child->next();
		delete child;
		child = tmp;
	}
}

bool Note::saveAgain()
{
	if ( content() )
	{
		if ( !content()->saveToFile() )
			return false;
	}
	FOR_EACH_CHILD ( child )
	{
		if ( !child->saveAgain() )
			return false;
	}
	return true;
}

bool Note::convertTexts()
{
	bool convertedNotes = false;

	if ( content() && content()->lowerTypeName() == "text" )
	{
		QString text = ( ( TextContent* ) content() )->text();
		QString html = "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body>" + Tools::textToHTMLWithoutP ( text ) + "</body></html>";
		basket()->saveToFile ( fullPath(), html, /*isLocalEncoding=*/true );
		setContent ( new HtmlContent ( this, content()->fileName() ) );
		convertedNotes = true;
	}

	FOR_EACH_CHILD ( child )
	if ( child->convertTexts() )
		convertedNotes = true;

	return convertedNotes;
}

#if 0

/** Note */

QString Note::toHtml ( const QString &imageName )
{
	switch ( m_type )
	{
		case Text:
			/*return "<font color=" + backgroundColor().name() + + font().name() + font + ">" +
			Tools::textToHTMLWithoutP(text()) + "</font>";*/
			return Tools::textToHTMLWithoutP ( text() );
		case Html:
			return Tools::htmlToParagraph ( html() );
		case Image:
		case Animation:
		{
			if ( ( m_type == Image     && pixmap() == 0L ) ||
			        ( m_type == Animation && movie()  == 0L ) )
			{
				QMimeSourceFactory::defaultFactory()->setData ( imageName, 0L );
				return i18n ( "(Image)" ); // Image or animation not yet loaded!!
			}

			QImage image;
			if ( m_type == Image )
				image = pixmap()->convertToImage();
			else
				image = movie()->framePixmap().convertToImage();
			image = image.smoothScale ( 200, 150, QImage::ScaleMin );
			QPixmap pixmap = QPixmap ( image );
			QMimeSourceFactory::defaultFactory()->setPixmap ( imageName, pixmap );
			return "<img src=" + imageName + ">"; ///

			/*				// FIXME: movie isn't loaded yet: CRASH!
							return i18n("(Image)");
							// Not executed, because don't work:
							QImage image;
							if (m_type == Image)
								image = pixmap()->convertToImage();
							else
								image = movie()->framePixmap().convertToImage();
							image = image.smoothScale(200, 150, QImage::ScaleMin);
							QPixmap pixmap = QPixmap(image);
							QMimeSourceFactory::defaultFactory()->setPixmap(imageName, pixmap);
							return "<img src=" + imageName + ">"; ///
				*/			//TODO?: QMimeSourceFactory::defaultFactory()->setData(imageName, 0L);
		}
		case Sound:
		case File:
		{
			/// FIXME: Since fullPath() doesn't exist yet, the icon rely on the extension.
			///        Bad if there isn't one or if it's a wrong one.
			/*QPixmap icon = DesktopIcon(
				NoteFactory::iconForURL(fullPath()),
				(m_type == Sound ? LinkLook::soundLook : LinkLook::fileLook)->iconSize());
			QMimeSourceFactory::defaultFactory()->setPixmap(imageName, icon);
			return "<img src=" + imageName + "> " + fileName(); */ ///
			return m_linkLabel->toHtml ( imageName );
		}
		case Link:
		{
			QString link = m_linkLabel->toHtml ( imageName );
			if ( !autoTitle() && title() != NoteFactory::titleForURL ( url().prettyUrl() ) )
				link += "<br><i>" + url().prettyUrl() + "</i>"; ///
			return link;
		}
		case Launcher:
		{
			return m_linkLabel->toHtml ( imageName );
			//KService service(fullPath()); // service.icon()
			//return service.name() + "<br><i>" + service.exec() + "</i>"; ///
		}
		case Color:
			return "<b><font color=" + color().name() + ">" + color().name() + "</font></b>";
		case Unknown:
			return text();
	}
	return QString();
}

#endif // #if 0

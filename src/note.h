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

#ifndef NOTE_H
#define NOTE_H

#include <qstring.h>
#include <qpixmap.h>
#include <qdatetime.h>

#include "notecontent.h"
#include "tag.h"

class Basket;
class FilterData;
class HtmlExportData;

class NoteSelection;

class QPainter;
class QTextDocument;

/** Handle basket notes and groups!\n
  * After creation, the note is a group. You should create a NoteContent with this Note
  * as constructor parameter to transform it to a note with content. eg:
  * @code
  * Note *note = new Note(basket);   // note is a group!
  * new TextContent(note, fileName); // note is now a note with a text content!
  * new ColorContent(note, Qt::red); // Should never be done!!!!! the old Content should be deleted...
  * @endcode
  * @author S�astien Laot
  */
class Note
{
/// CONSTRUCTOR AND DESTRUCTOR:
  public:
	Note(Basket *parent);
	~Note();

/// DOUBLY LINKED LIST:
  private:
	Note *m_prev;
	Note *m_next;
  public:
	inline void  setNext(Note *next) { m_next = next; }
	inline void  setPrev(Note *prev) { m_prev = prev; }
	inline Note* next()              { return m_next; }
	inline Note* prev()              { return m_prev; }

/// GEOMETRY MANAGEMENT:
  private:
	int m_x;
	int m_y;
	int m_width;
	int m_height;
//	int m_minContentWidth;
  public:
	void setWidth(int width);
	void setWidthForceRelayout(int width);
	void setInitialHeight(int height) { m_height = height; } /// << Do NEVER use it unless you know what you do!
	void setX(int x);
	void setY(int y);
	void setXRecursivly(int x);
	void setYRecursivly(int y);
	inline int  x()      { return m_x;                }
	inline int  y()      { return m_y;                }
	inline int  width()  { return (isGroup() ? (isColumn() ? 0 : GROUP_WIDTH) : m_width); }
	inline int  height() { return m_height;           }
	inline int  bottom() { return m_y + m_height - 1; }
	QRect rect();
	QRect resizerRect();
	QRect visibleRect();
	void relayoutAt(int x, int y, bool animate);
	int contentX();
	int minWidth();
	int minRight();
	void unsetWidth();
	void requestRelayout();
	void setHeight(int height) { m_height = height; } /// << DO NEVER USE IT!!! Only available when moving notes, groups should be recreated with the exact same state as before!

/// FREE AND COLUMN LAYOUTS MANAGEMENT:
  private:
	int m_groupWidth;
  public:
	int  groupWidth();
	void setGroupWidth(int width);
	int  rightLimit();
	int  finalRightLimit();
	bool isFree();
	bool isColumn();
	bool hasResizer();
	int  resizerHeight();

/// GROUPS MANAGEMENT:
  private:
	bool  m_isFolded;
	Note *m_firstChild;
	Note *m_parentNote;
  public:
	inline bool isGroup()                 { return m_content == 0L;  }
	inline bool isFolded()                { return m_isFolded;       }
	inline Note* firstChild()             { return m_firstChild;     }
	inline Note* parentNote()             { return m_parentNote;     }
	/*inline*/ bool showSubNotes();//            { return !m_isFolded || !m_collapseFinished; }
	inline void setParentNote(Note *note) { m_parentNote = note;     }
	inline void setFirstChild(Note *note) { m_firstChild = note;     }
	bool isVisible();
	void  toggleFolded(bool animate);
	Note* noteAt(int x, int y);
	Note* firstRealChild();
	Note* lastRealChild();
	Note* lastChild();
	Note* lastSibling();
	int   yExpander();
	bool  isAfter(Note *note);
	bool  contains(Note *note);

/// NOTES VARIOUS PROPERTIES:       // CONTENT MANAGEMENT?
  private:
	Basket      *m_basket;
	NoteContent *m_content;
	QDateTime    m_addedDate;
	QDateTime    m_lastModificationDate;
  public:
	inline Basket*      basket()  { return m_basket;  }
	inline NoteContent* content() { return m_content; }
	inline void setAddedDate(const QDateTime &dateTime)            { m_addedDate            = dateTime; }
	inline void setLastModificationDate(const QDateTime &dateTime) { m_lastModificationDate = dateTime; }
	inline void setParentBasket(Basket *basket) { m_basket = basket; }
	QDateTime addedDate()            { return m_addedDate;            }
	QDateTime lastModificationDate() { return m_lastModificationDate; }
	QString addedStringDate();
	QString lastModificationStringDate();
	QString toText(const QString &cuttedFullPath);
	bool saveAgain();
	void deleteChilds();

  protected:
	void setContent(NoteContent *content);
	friend class NoteContent;
	friend class AnimationContent;

/// DRAWING:
  private:
	QPixmap m_bufferedPixmap;
	QPixmap m_bufferedSelectionPixmap;
  public:
	void draw(QPainter *painter, const QRect &clipRect);
	void drawBufferOnScreen(QPainter *painter, const QPixmap &contentPixmap);
	static void getGradientColors(const QColor &originalBackground, QColor *colorTop, QColor *colorBottom);
	static void drawExpander(QPainter *painter, int x, int y, const QColor &background, bool expand, Basket *basket);
	void drawHandle(   QPainter *painter, int x, int y, int width, int height, const QColor &background, const QColor &foreground);
	void drawResizer(  QPainter *painter, int x, int y, int width, int height, const QColor &background, const QColor &foreground, bool rounded);
	void drawRoundings(QPainter *painter, int x, int y, int type, int width = 0, int height = 0);
	void unbufferizeAll();
	void bufferizeSelectionPixmap();
	inline void unbufferize()  { m_bufferedPixmap.scaled(0, 0); m_bufferedSelectionPixmap.scaled(0, 0); }
	inline bool isBufferized() { return !m_bufferedPixmap.isNull(); }
	void recomputeBlankRects(QList<QRect> &blankAreas);
	static void drawInactiveResizer(QPainter *painter, int x, int y, int height, const QColor &background, bool column);

/// VISIBLE AREAS COMPUTATION:
  private:
	QList<QRect> m_areas;
	bool              m_computedAreas;
	bool              m_onTop;
	void recomputeAreas();
	bool recomputeAreas(Note *note, bool noteIsAfterThis);
  public:
	void setOnTop(bool onTop);
	inline bool isOnTop() { return m_onTop; }
	bool isEditing();

/// MANAGE ANIMATION:
  private:
	int  m_deltaX;
	int  m_deltaY;
	int  m_deltaHeight;
	bool m_collapseFinished;
	bool m_expandingFinished;
	public:
	inline int  deltaX()          { return m_deltaX;                 }
	inline int  deltaY()          { return m_deltaY;                 }
	inline int  deltaHeight()     { return m_deltaHeight;            }
	inline int  finalX()          { return m_x + m_deltaX;           }
	inline int  finalY()          { return m_y + m_deltaY;           }
	inline int  finalHeight()     { return m_height + m_deltaHeight; }
	inline int  finalBottom()     { return finalY() + finalHeight() - 1; }
	inline void cancelAnimation() { m_deltaX = 0; m_deltaY = 0; m_deltaHeight = 0; }
	inline bool expandingOrCollapsing() { return !m_collapseFinished || !m_expandingFinished; }
	void addAnimation(int deltaX, int deltaY, int deltaHeight = 0);
	void setFinalPosition(int x, int y); /// << Convenient method for addAnimation()
	bool advance();
	void initAnimationLoad();
	void setRecursivelyUnder(Note *under, bool animate);

/// USER INTERACTION:
  public:
	enum Zone { None = 0,
	            Handle, TagsArrow, Custom0, /*CustomContent1, CustomContent2, */Content, Link,
	            TopInsert, TopGroup, BottomInsert, BottomGroup, BottomColumn,
	            Resizer,
	            Group, GroupExpander,
	            Emblem0 }; // Emblem0 should be at end, because we add 1 to get Emblem1, 2 to get Emblem2...
	inline void setHovered(bool hovered)  { m_hovered     = hovered; unbufferize(); }
	void        setHoveredZone(Zone zone);
	inline bool hovered()                 { return m_hovered;     }
	inline Zone hoveredZone()             { return m_hoveredZone; }
	Zone zoneAt(const QPoint &pos, bool toAdd = false);
	QRect zoneRect(Zone zone, const QPoint &pos);
	void setCursor(Zone zone);
	QString linkAt(const QPoint &pos);
  private:
	bool m_hovered;
	Zone m_hoveredZone;

/// SELECTION:
  public:
	NoteSelection* selectedNotes();
	void setSelected(bool selected);
	void setSelectedRecursivly(bool selected);
	void invertSelectionRecursivly();
	void selectIn(const QRect &rect, bool invertSelection, bool unselectOthers = true);
	void setFocused(bool focused);
	inline bool isFocused()  { return m_focused; }
	inline bool isSelected() { return m_selected; }
	bool allSelected();
	void resetWasInLastSelectionRect();
	void unselectAllBut(Note *toSelect);
	void invertSelectionOf(Note *toSelect);
	Note* theSelectedNote();
  private:
	bool m_focused;
	bool m_selected;
	bool m_wasInLastSelectionRect;

/// TAGS:
  private:
	State::List m_states;
	State       m_computedState;
	int         m_emblemsCount;
	bool        m_haveInvisibleTags;
  public:
	/*const */State::List& states() const;
	inline int emblemsCount() { return m_emblemsCount; }
	void addState(State *state, bool orReplace = true);
	void addTag(Tag *tag);
	void removeState(State *state);
	void removeTag(Tag *tag);
	void removeAllTags();
	void addTagToSelectedNotes(Tag *tag);
	void removeTagFromSelectedNotes(Tag *tag);
	void removeAllTagsFromSelectedNotes();
	void addStateToSelectedNotes(State *state, bool orReplace = true);
	void changeStateOfSelectedNotes(State *state);
	bool selectedNotesHaveTags();
	void inheritTagsOf(Note *note);
	bool hasTag(Tag *tag);
	bool hasState(State *state);
	State* stateOfTag(Tag *tag);
	State* stateForEmblemNumber(int number);
	bool stateForTagFromSelectedNotes(Tag *tag, State **state);
	void   recomputeStyle();
	void   recomputeAllStyles();
	bool   removedStates(const QList<State*> &deletedStates);
	QFont  font(); // Computed!
	QColor backgroundColor(); // Computed!
	QColor textColor(); // Computed!

/// FILTERING:
  private:
	bool m_matching;
  public:
	bool computeMatching(const FilterData &data);
	int  newFilter(const FilterData &data);
	bool matching() { return m_matching; }

/// ADDED:
  public:
	void deleteSelectedNotes(bool deleteFilesToo = true);
	int count();
	int countDirectChilds();

	QString fullPath();
	Note* noteForFullPath(const QString &path);

	void update();
	void linkLookChanged();

	void usedStates(QList<State*> &states);

	void listUsedTags(QList<Tag*> &list);


	Note* nextInStack();
	Note* prevInStack();
	Note* nextShownInStack();
	Note* prevShownInStack();

	Note* parentPrimaryNote(); // TODO: There are places in the code where this methods is hand-copied / hand-inlined instead of called.

	Note* firstSelected();
	Note* lastSelected();
	Note* selectedGroup();
	void groupIn(Note *group);

	bool tryExpandParent();
	bool tryFoldParent();

	int distanceOnLeftRight(Note *note, int side);
	int distanceOnTopBottom(Note *note, int side);

	bool convertTexts();

	void debug();

/// SPEED OPTIMIZATION
  public:
	void finishLazyLoad();

  public:
	// Values are provided here as info:
	// Please see Settings::setBigNotes() to know whats values are assigned.
	static int NOTE_MARGIN      /*= 2*/;
	static int INSERTION_HEIGHT /*= 5*/;
	static int EXPANDER_WIDTH   /*= 9*/;
	static int EXPANDER_HEIGHT  /*= 9*/;
	static int GROUP_WIDTH      /*= 2*NOTE_MARGIN + EXPANDER_WIDTH*/;
	static int HANDLE_WIDTH     /*= GROUP_WIDTH*/;
	static int RESIZER_WIDTH    /*= GROUP_WIDTH*/;
	static int TAG_ARROW_WIDTH  /*= 5*/;
	static int EMBLEM_SIZE      /*= 16*/;
	static int MIN_HEIGHT       /*= 2*NOTE_MARGIN + EMBLEM_SIZE*/;
};

/*class InsertionData
{
  public:
	enum Type { Free = 0, NoteRelative, ColumnBottom };
	Type type;

	InsertionData(int _x, int _y)
	 : type(Free),
	   x(_x), y(_y),
	   note(0), group(false), onTop(false),
	   column(0)
	{}
	int x;
	int y;

	InsertionData(Note *_note, bool _group, bool _onTop)
	 : type(NoteRelative),
	   x(0), y(0),
	   note(_note), group(_group), onTop(_onTop),
	   column(0)
	{}
	Note *note;
	bool group;
	bool onTop;

	InsertionData(Note *_column)
	 : type(NoteRelative),
	   x(0), y(0),
	   note(0), group(false), onTop(false),
	   column(_column)
	{}
	Note *column;
};*/

#endif // NOTE_H

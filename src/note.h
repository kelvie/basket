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

#ifndef NOTE_H
#define NOTE_H

#include <QtCore/QList>
#include <QtCore/QDateTime>
#include <QtGui/QGraphicsItemGroup>

#include "basket_export.h"
#include "tag.h"

class BasketScene;
class FilterData;

class NoteContent;
class NoteSelection;

class QPainter;
class QPixmap;
class QString;

class QGraphicsItemAnimation;
class QTimeLine;

class NotePrivate;

/** Handle basket notes and groups!\n
  * After creation, the note is a group. You should create a NoteContent with this Note
  * as constructor parameter to transform it to a note with content. eg:
  * @code
  * Note *note = new Note(basket);   // note is a group!
  * new TextContent(note, fileName); // note is now a note with a text content!
  * new ColorContent(note, Qt::red); // Should never be done!!!!! the old Content should be deleted...
  * @endcode
  * @author Sébastien Laoût
  */
class BASKET_EXPORT Note : public QGraphicsItemGroup
{
/// CONSTRUCTOR AND DESTRUCTOR:
public:
    Note(BasketScene *parent = 0);
    ~Note();

private:
    NotePrivate* d;

/// DOUBLY LINKED LIST:
public:
    void  setNext(Note *next);
    void  setPrev(Note *prev);
    Note* next() const;
    Note* prev() const;

public:
    void setWidth(qreal width);
    void setWidthForceRelayout(qreal width);
    //! Do not use it unless you know what you do!
    void setInitialHeight(qreal height);

    void setXRecursively(qreal ax);
    void setYRecursively(qreal ay);
    qreal width() const;
    qreal height() const;
    qreal bottom() const;
    QRectF rect();
    QRectF resizerRect();
    QRectF visibleRect();
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                QWidget *widget);
    void relayoutAt(qreal ax, qreal ay, bool animate);
    qreal contentX() const;
    qreal minWidth() const;
    qreal minRight();
    void unsetWidth();
    void requestRelayout();
    /** << DO NEVER USE IT!!! Only available when moving notes, groups should be recreated with the exact same state as before! */
    void setHeight(qreal height);

/// FREE AND COLUMN LAYOUTS MANAGEMENT:
private:
    qreal m_groupWidth;
public:
    qreal  groupWidth() const;
    void setGroupWidth(qreal width);
    qreal rightLimit() const;
    qreal  finalRightLimit() const;
    bool isFree() const;
    bool isColumn() const;
    bool hasResizer() const;
    qreal resizerHeight() const;

/// GROUPS MANAGEMENT:
private:
    bool  m_isFolded;
    Note *m_firstChild;
    Note *m_parentNote;
public:
    inline bool isGroup() const           {
        return m_content == 0L;
    }
    inline bool isFolded()                {
        return m_isFolded;
    }
    inline Note* firstChild()             {
        return m_firstChild;
    }
    inline Note* parentNote() const       {
        return m_parentNote;
    }
    /*inline*/ bool showSubNotes();//            { return !m_isFolded || !m_collapseFinished; }
    inline void setParentNote(Note *note) {
        m_parentNote = note;
    }
    inline void setFirstChild(Note *note) {
        m_firstChild = note;
    }
    bool isShown();
    bool  toggleFolded();
    
    Note* noteAt(QPointF pos);
    Note* firstRealChild();
    Note* lastRealChild();
    Note* lastChild();
    Note* lastSibling();
    qreal yExpander();
    bool  isAfter(Note *note);
    bool  containsNote(Note *note);

/// NOTES VARIOUS PROPERTIES:       // CONTENT MANAGEMENT?
private:
    BasketScene  *m_basket;
    NoteContent *m_content;
    QDateTime    m_addedDate;
    QDateTime    m_lastModificationDate;
public:
    inline BasketScene*      basket() const {
        return m_basket;
    }
    inline NoteContent* content() {
        return m_content;
    }
    inline void setAddedDate(const QDateTime &dateTime)            {
        m_addedDate            = dateTime;
    }
    inline void setLastModificationDate(const QDateTime &dateTime) {
        m_lastModificationDate = dateTime;
    }
    
    void setParentBasket(BasketScene *basket);
    
    QDateTime addedDate()            {
        return m_addedDate;
    }
    QDateTime lastModificationDate() {
        return m_lastModificationDate;
    }
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
    void draw(QPainter *painter, const QRectF &clipRect);
    void drawBufferOnScreen(QPainter *painter, const QPixmap &contentPixmap);
    static void getGradientColors(const QColor &originalBackground, QColor *colorTop, QColor *colorBottom);
    static void drawExpander(QPainter *painter, qreal x, qreal y, const QColor &background, bool expand, BasketScene *basket);
    void drawHandle(QPainter *painter, qreal x, qreal y, qreal width, qreal height, const QColor &background, const QColor &foreground);
    void drawResizer(QPainter *painter, qreal x, qreal y, qreal width, qreal height, const QColor &background, const QColor &foreground, bool rounded);
    void drawRoundings(QPainter *painter, qreal x, qreal y, int type, qreal width = 0, qreal height = 0);
    void unbufferizeAll();
    void bufferizeSelectionPixmap();
    inline void unbufferize()  {
        m_bufferedPixmap = QPixmap(); m_bufferedSelectionPixmap = QPixmap();
    }
    inline bool isBufferized() {
        return !m_bufferedPixmap.isNull();
    }
    void recomputeBlankRects(QList<QRectF> &blankAreas);
    static void drawInactiveResizer(QPainter *painter, qreal x, qreal y, qreal height, const QColor &background, bool column);
    QPalette palette() const;

/// VISIBLE AREAS COMPUTATION:
private:
    QList<QRectF> m_areas;
    bool              m_computedAreas;
    bool              m_onTop;
    void recomputeAreas();
    bool recomputeAreas(Note *note, bool noteIsAfterThis);
public:
    void setOnTop(bool onTop);
    inline bool isOnTop() {
        return m_onTop;
    }
    bool isEditing();

/// MANAGE ANIMATION:
private:
    QGraphicsItemAnimation *m_animation;
public:
    bool initAnimationLoad(QTimeLine *timeLine);
    void animationFinished();
    
/// USER INTERACTION:
public:
    enum Zone { None = 0,
                Handle, TagsArrow, Custom0, /*CustomContent1, CustomContent2, */Content, Link,
                TopInsert, TopGroup, BottomInsert, BottomGroup, BottomColumn,
                Resizer,
                Group, GroupExpander,
                Emblem0
              }; // Emblem0 should be at end, because we add 1 to get Emblem1, 2 to get Emblem2...
    inline void setHovered(bool hovered)  {
        m_hovered     = hovered; unbufferize();
    }
    void        setHoveredZone(Zone zone);
    inline bool hovered()                 {
        return m_hovered;
    }
    inline Zone hoveredZone()             {
        return m_hoveredZone;
    }
    Zone zoneAt(const QPointF &pos, bool toAdd = false);
    QRectF zoneRect(Zone zone, const QPointF &pos);
    Qt::CursorShape cursorFromZone(Zone zone) const;
    QString linkAt(const QPointF &pos);
private:
    bool m_hovered;
    Zone m_hoveredZone;

/// SELECTION:
public:
    NoteSelection* selectedNotes();
    void setSelected(bool selected);
    void setSelectedRecursively(bool selected);
    void invertSelectionRecursively();
    void selectIn(const QRectF &rect, bool invertSelection, bool unselectOthers = true);
    void setFocused(bool focused);
    inline bool isFocused()  {
        return m_focused;
    }
    inline bool isSelected() {
        return m_selected;
    }
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
    inline int emblemsCount() {
        return m_emblemsCount;
    }
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
    State* stateForEmblemNumber(int number) const;
    bool stateForTagFromSelectedNotes(Tag *tag, State **state);
    void   recomputeStyle();
    void   recomputeAllStyles();
    bool   removedStates(const QList<State*> &deletedStates);
    QFont  font(); // Computed!
    QColor backgroundColor(); // Computed!
    QColor textColor(); // Computed!
    bool allowCrossReferences();

/// FILTERING:
private:
    bool m_matching;
public:
    bool computeMatching(const FilterData &data);
    int  newFilter(const FilterData &data);
    bool matching() {
        return m_matching;
    }

/// ADDED:
public:
    /**
     * @return true if this note could be deleted
     **/
    bool deleteSelectedNotes(bool deleteFilesToo = true);
    int count();
    int countDirectChilds();

    QString fullPath();
    Note* noteForFullPath(const QString &path);

    //void update();
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

    qreal distanceOnLeftRight(Note *note, int side);
    qreal distanceOnTopBottom(Note *note, int side);

    bool convertTexts();

    void debug();

/// SPEED OPTIMIZATION
public:
    void finishLazyLoad();

public:
    // Values are provided here as info:
    // Please see Settings::setBigNotes() to know whats values are assigned.
    static qreal NOTE_MARGIN      /*= 2*/;
    static qreal INSERTION_HEIGHT /*= 5*/;
    static qreal EXPANDER_WIDTH   /*= 9*/;
    static qreal EXPANDER_HEIGHT  /*= 9*/;
    static qreal GROUP_WIDTH      /*= 2*NOTE_MARGIN + EXPANDER_WIDTH*/;
    static qreal HANDLE_WIDTH     /*= GROUP_WIDTH*/;
    static qreal RESIZER_WIDTH    /*= GROUP_WIDTH*/;
    static qreal TAG_ARROW_WIDTH  /*= 5*/;
    static qreal EMBLEM_SIZE      /*= 16*/;
    static qreal MIN_HEIGHT       /*= 2*NOTE_MARGIN + EMBLEM_SIZE*/;
};

/*
 * Convenience functions:
 */
void drawGradient(QPainter *p, const QColor &colorTop, const QColor & colorBottom,
		  qreal x, qreal y, qreal w, qreal h,
		  bool sunken, bool horz, bool flat);

extern void substractRectOnAreas(const QRectF &rectToSubstract,
		QList<QRectF> &areas, bool andRemove = true);


#endif // NOTE_H

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

#ifndef BASKET_H
#define BASKET_H

#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtCore/QTimer>
#include <QtGui/QTextCursor>
#include <QtGui/QClipboard>
#include <QtGui/QGraphicsScene>

#include "note.h" // For Note::Zone
#include "config.h"

class QFrame;
class QPixmap;
class QPushButton;

class QDomDocument;
class QDomElement;

class QContextMenuEvent;
class QDragLeaveEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QEvent;
class QFocusEvent;
class QHelpEvent;
class QKeyEvent;
class QMouseEvent;
class QResizeEvent;
class QWheelEvent;

class KAction;
class KDirWatch;
class KShortcut;
class KUrl;

namespace KIO { 
    class Job; 
}

class DecoratedBasket;
class Note;
class NoteEditor;
class Tag;
class TransparentWidget;

#ifdef HAVE_LIBGPGME
class KGpgMe;
#endif

/**
  * @author Sébastien Laoût
  */
class BasketScene : public QGraphicsScene
{
    Q_OBJECT
public:
    enum EncryptionTypes {
        NoEncryption         = 0,
        PasswordEncryption   = 1,
        PrivateKeyEncryption = 2
    };

public:
/// CONSTRUCTOR AND DESTRUCTOR:
    BasketScene(QWidget *parent, const QString &folderName);
    ~BasketScene();

/// USER INTERACTION:
private:
    bool   m_noActionOnMouseRelease;
    bool   m_ignoreCloseEditorOnNextMouseRelease;
    QPointF m_pressPos;
    bool   m_canDrag;

public:
    void drawBackground ( QPainter * painter, const QRectF & rect );
    void drawForeground ( QPainter * painter, const QRectF & rect );

    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void clickedToInsert(QGraphicsSceneMouseEvent *event, Note *clicked = 0, int zone = 0);
private slots:
    void setFocusIfNotInPopupMenu();
signals:
    void crossReference(QString link);

/// LAYOUT:
private:
    Note   *m_firstNote;
    int     m_columnsCount;
    bool    m_mindMap;
    Note   *m_resizingNote;
    int     m_pickedResizer;
    Note   *m_movingNote;
    QPoint  m_pickedHandle;
    QSet<Note*> m_notesToBeDeleted;
    
public:
    qreal tmpWidth;
    qreal tmpHeight;
public:
    void unsetNotesWidth();
    void relayoutNotes(bool animate);
    Note* noteAt(QPointF pos);
    inline Note* firstNote()       {
        return m_firstNote;
    }
    inline int   columnsCount()    {
        return m_columnsCount;
    }
    inline bool  isColumnsLayout() {
        return m_columnsCount > 0;
    }
    inline bool  isFreeLayout()    {
        return m_columnsCount <= 0;
    }
    inline bool  isMindMap()       {
        return isFreeLayout() && m_mindMap;
    }
    Note* resizingNote()           {
        return m_resizingNote;
    }
    void  deleteNotes();
    Note* lastNote();
    void setDisposition(int disposition, int columnCount);
    void equalizeColumnSizes();

/// NOTES INSERTION AND REMOVAL:
public:
    /// The following methods assume that the note(s) to insert already all have 'this' as the parent basket:
    void prependNoteIn(Note *note, Note *in);        /// << Add @p note (and the next linked notes) as the first note(s) of the group @p in.
    void appendNoteIn(Note *note, Note *in);         /// << Add @p note (and the next linked notes) as the last note(s) of the group @p in.
    void appendNoteAfter(Note *note, Note *after);   /// << Add @p note (and the next linked notes) just after (just below) the note @p after.
    void appendNoteBefore(Note *note, Note *before); /// << Add @p note (and the next linked notes) just before (just above) the note @p before.
    void groupNoteAfter(Note *note, Note *with);     /// << Add a group at @p with place, move @p with in it, and add @p note (and the next linked notes) just after the group.
    void groupNoteBefore(Note *note, Note *with);    /// << Add a group at @p with place, move @p with in it, and add @p note (and the next linked notes) just before the group.
    void unplugNote(Note *note);                     /// << Unplug @p note (and its child notes) from the basket (and also decrease counts...).
    /// <<  After that, you should delete the notes yourself. Do not call prepend/append/group... functions two times: unplug and ok
    void ungroupNote(Note *group);                   /// << Unplug @p group but put child notes at its place.
    /// And this one do almost all the above methods depending on the context:
    void insertNote(Note *note, Note *clicked, int zone, const QPointF &pos = QPointF(), bool animateNewPosition = false);
    void insertCreatedNote(Note *note);
    /// And working with selections:
    void unplugSelection(NoteSelection *selection);
    void insertSelection(NoteSelection *selection, Note *after);
    void selectSelection(NoteSelection *selection);
protected slots:
    void doCleanUp();
  
private:
    void preparePlug(Note *note);
private:
    Note  *m_clickedToInsert;
    int    m_zoneToInsert;
    QPointF m_posToInsert;
    Note  *m_savedClickedToInsert;
    int    m_savedZoneToInsert;
    QPointF m_savedPosToInsert;
    bool   m_isInsertPopupMenu;
    QAction *m_insertMenuTitle;
public:
    void saveInsertionData();
    void restoreInsertionData();
    void resetInsertionData();
public slots:
    void insertEmptyNote(int type);
    void insertWizard(int type);
    void insertColor(const QColor &color);
    void insertImage(const QPixmap &image);
    void pasteNote(QClipboard::Mode mode = QClipboard::Clipboard);
    void delayedCancelInsertPopupMenu();
    void setInsertPopupMenu()    {
        m_isInsertPopupMenu = true;
    }
    void cancelInsertPopupMenu() {
        m_isInsertPopupMenu = false;
    }
private slots:
    void hideInsertPopupMenu();
    void timeoutHideInsertPopupMenu();

/// TOOL TIPS:
protected:
    void helpEvent(QGraphicsSceneHelpEvent* event);

/// ANIMATIONS:
private:
    QTimeLine *m_animationTimeLine;
    static const int ANIMATION_DELAY;
    
public slots:
    void animationFrameChanged(int);
    void animateLoad();
    void finishAnimation();
    
/// LOAD AND SAVE:
private:
    bool m_loaded;
    bool m_loadingLaunched;
    bool m_locked;
    bool m_shouldConvertPlainTextNotes;
    QFrame* m_decryptBox;
    QPushButton* m_button;
    int m_encryptionType;
    QString m_encryptionKey;
#ifdef HAVE_LIBGPGME
    KGpgMe* m_gpg;
#endif
    QTimer      m_inactivityAutoLockTimer;
    void enableActions();

private slots:
    void loadNotes(const QDomElement &notes, Note *parent);
    void saveNotes(QDomDocument &document, QDomElement &element, Note *parent);
    void unlock();
protected slots:
    void inactivityAutoLockTimeout();
public slots:
    void load();
    void loadProperties(const QDomElement &properties);
    void saveProperties(QDomDocument &document, QDomElement &properties);
    bool save();
    void reload();
public:
    bool isEncrypted();
    bool isFileEncrypted();
    bool isLocked()        {
        return m_locked;
    };
    void lock();
    bool isLoaded()        {
        return m_loaded;
    };
    bool loadingLaunched() {
        return m_loadingLaunched;
    };
    bool loadFromFile(const QString &fullPath, QString* string);
    bool loadFromFile(const QString &fullPath, QByteArray* array);
    bool saveToFile(const QString& fullPath, const QString& string);
    static bool safelySaveToFile(const QString& fullPath, const QByteArray& array, unsigned long length);
    static bool safelySaveToFile(const QString& fullPath, const QString& string);
    bool setProtection(int type, QString key);
    int  encryptionType()  {
        return m_encryptionType;
    };
    QString encryptionKey() {
        return m_encryptionKey;
    };
    bool saveAgain();

/// BACKGROUND:
private:
    QColor   m_backgroundColorSetting;
    QString  m_backgroundImageName;
    QPixmap *m_backgroundPixmap;
    QPixmap *m_opaqueBackgroundPixmap;
    QPixmap *m_selectedBackgroundPixmap;
    bool     m_backgroundTiled;
    QColor   m_textColorSetting;
public:
    inline bool           hasBackgroundImage()     {
        return m_backgroundPixmap != 0;
    }
    inline const QPixmap* backgroundPixmap()       {
        return m_backgroundPixmap;
    }
    inline bool           isTiledBackground()      {
        return m_backgroundTiled;
    }
    inline QString        backgroundImageName()    {
        return m_backgroundImageName;
    }
    inline QColor         backgroundColorSetting() {
        return m_backgroundColorSetting;
    }
    inline QColor         textColorSetting()       {
        return m_textColorSetting;
    }
    QColor backgroundColor() const;
    QColor textColor() const;
    void setAppearance(const QString &icon, const QString &name, const QString &backgroundImage, const QColor &backgroundColor, const QColor &textColor);
    void blendBackground(QPainter &painter, const QRectF &rect, qreal xPainter = -1, qreal yPainter = -1, bool opaque = false, QPixmap *bg = 0);
    void blendBackground(QPainter &painter, const QRectF &rect, bool opaque, QPixmap *bg );
    void unbufferizeAll();
    void subscribeBackgroundImages();
    void unsubscribeBackgroundImages();

/// KEYBOARD SHORTCUT:
public:
    KAction *m_action;
private:
    int      m_shortcutAction;
private slots:
    void activatedShortcut();
public:
    KShortcut shortcut() {
        return m_action->shortcut();
    }
    int shortcutAction() {
        return m_shortcutAction;
    }
    void setShortcut(KShortcut shortcut, int action);

/// USER INTERACTION:
private:
    Note  *m_hoveredNote;
    int    m_hoveredZone;
    bool   m_lockedHovering;
    bool   m_underMouse;
    QRectF  m_inserterRect;
    bool   m_inserterShown;
    bool   m_inserterSplit;
    bool   m_inserterTop;
    bool   m_inserterGroup;
    void placeInserter(Note *note, int zone);
    void removeInserter();
public:
//  bool inserterShown() { return m_inserterShown; }
    bool inserterSplit() {
        return m_inserterSplit;
    }
    bool inserterGroup() {
        return m_inserterGroup;
    }
public slots:
    void doHoverEffects(Note *note, Note::Zone zone, const QPointF &pos = QPointF(0, 0)); /// << @p pos is optionnal and only used to show the link target in the statusbar
    void doHoverEffects(const QPointF &pos);
    void doHoverEffects(); // The same, but using the current cursor position
    void mouseEnteredEditorWidget();
public:
    void popupTagsMenu(Note *note);
    void popupEmblemMenu(Note *note, int emblemNumber);
    void addTagToSelectedNotes(Tag *tag);
    void removeTagFromSelectedNotes(Tag *tag);
    void removeAllTagsFromSelectedNotes();
    void addStateToSelectedNotes(State *state);
    void changeStateOfSelectedNotes(State *state);
    bool selectedNotesHaveTags();
    const QRectF& inserterRect()  {
        return m_inserterRect;
    }
    bool         inserterShown() {
        return m_inserterShown;
    }
    void drawInserter(QPainter &painter, qreal xPainter, qreal yPainter);
    DecoratedBasket* decoration();
    State *stateForTagFromSelectedNotes(Tag *tag);
public slots:
    void activatedTagShortcut(Tag *tag);
    void recomputeAllStyles();
    void removedStates(const QList<State*> &deletedStates);
private slots:
    void toggledTagInMenu(QAction *act);
    void toggledStateInMenu(QAction *act);
    void unlockHovering();
    void disableNextClick();
public:
    Note  *m_tagPopupNote;
private:
    Tag   *m_tagPopup;
    QTime  m_lastDisableClick;

/// SELECTION:
private:
    bool   m_isSelecting;
    bool   m_selectionStarted;
    bool   m_selectionInvert;
    QPointF m_selectionBeginPoint;
    QPointF m_selectionEndPoint;
    QRectF  m_selectionRect;
    QTimer m_autoScrollSelectionTimer;
    void stopAutoScrollSelection();
private slots:
    void doAutoScrollSelection();
public:
    inline bool isSelecting() {
        return m_isSelecting;
    }
    inline const QRectF& selectionRect() {
        return m_selectionRect;
    }
    void selectNotesIn(const QRectF &rect, bool invertSelection, bool unselectOthers = true);
    void resetWasInLastSelectionRect();
    void selectAll();
    void unselectAll();
    void invertSelection();
    void unselectAllBut(Note *toSelect);
    void invertSelectionOf(Note *toSelect);
    QColor selectionRectInsideColor();
    Note* theSelectedNote();
    NoteSelection* selectedNotes();

/// BLANK SPACES DRAWING:
private:
    QList<QRectF> m_blankAreas;
    void recomputeBlankRects();
    QWidget *m_cornerWidget;

/// COMMUNICATION WITH ITS CONTAINER:
signals:
    void postMessage(const QString &message);      /// << Post a temporar message in the statusBar.
    void setStatusBarText(const QString &message); /// << Set the permanent statusBar text or reset it if message isEmpty().
    void resetStatusBarText();                     /// << Equivalent to setStatusBarText("").
    void propertiesChanged(BasketScene *basket);
    void countsChanged(BasketScene *basket);
public slots:
    void linkLookChanged();
    void signalCountsChanged();
private:
    QTimer m_timerCountsChanged;
private slots:
    void countsChangedTimeOut();

/// NOTES COUNTING:
public:
    void addSelectedNote()    {
        ++m_countSelecteds;   signalCountsChanged();
    }
    void removeSelectedNote() {
        --m_countSelecteds;   signalCountsChanged();
    }
    void resetSelectedNote()  {
        m_countSelecteds = 0; signalCountsChanged();
    } // FIXME: Useful ???
    int count()               {
        return m_count;
    }
    int countFounds()         {
        return m_countFounds;
    }
    int countSelecteds()      {
        return m_countSelecteds;
    }
private:
    int m_count;
    int m_countFounds;
    int m_countSelecteds;

/// PROPERTIES:
public:
    QString basketName() {
        return m_basketName;
    }
    QString icon()       {
        return m_icon;
    }
    QString folderName() {
        return m_folderName;
    }
    QString fullPath();
    QString fullPathForFileName(const QString &fileName); // Full path of an [existing or not] note in this basket
    static QString fullPathForFolderName(const QString &folderName);
private:
    QString m_basketName;
    QString m_icon;
    QString m_folderName;

/// ACTIONS ON SELECTED NOTES FROM THE INTERFACE:
public slots:
    void noteEdit(Note *note = 0L, bool justAdded = false, const QPointF &clickedPoint = QPointF());
    void showEditedNoteWhileFiltering();
    void noteDelete();
    void noteDeleteWithoutConfirmation(bool deleteFilesToo = true);
    void noteCopy();
    void noteCut();
    void noteOpen(Note *note = 0L);
    void noteOpenWith(Note *note = 0L);
    void noteSaveAs();
    void noteGroup();
    void noteUngroup();
    void noteMoveOnTop();
    void noteMoveOnBottom();
    void noteMoveNoteUp();
    void noteMoveNoteDown();
    void moveSelectionTo(Note *here, bool below);
public:
    enum CopyMode { CopyToClipboard, CopyToSelection, CutToClipboard };
    void doCopy(CopyMode copyMode);
    bool selectionIsOneGroup();
    Note* selectedGroup();
    Note* firstSelected();
    Note* lastSelected();

/// NOTES EDITION:
private:
    NoteEditor *m_editor;
    //QWidget    *m_rightEditorBorder;
    TransparentWidget *m_leftEditorBorder;
    TransparentWidget *m_rightEditorBorder;
    bool        m_redirectEditActions;
    bool	m_editorTrackMouseEvent;
    qreal       m_editorWidth;
    qreal       m_editorHeight;
    QTimer      m_inactivityAutoSaveTimer;
    bool        m_doNotCloseEditor;
    QTextCursor m_textCursor;
public:
    bool isDuringEdit()        {
        return m_editor;
    }
    bool redirectEditActions() {
        return m_redirectEditActions;
    }
    bool hasTextInEditor();
    bool hasSelectedTextInEditor();
    bool selectedAllTextInEditor();
    Note* editedNote();
protected slots:
    void selectionChangedInEditor();
    void contentChangedInEditor();
    void inactivityAutoSaveTimeout();
public slots:
    void editorCursorPositionChanged();
private:
    qreal m_editorX;
    qreal m_editorY;
public slots:
    void placeEditor(bool andEnsureVisible = false);
    void placeEditorAndEnsureVisible();
    bool closeEditor();
    void closeEditorDelayed();
    void updateEditorAppearance();
    void editorPropertiesChanged();
    void openBasket();
    void closeBasket();

/// FILTERING:
public slots:
    void newFilter(const FilterData &data, bool andEnsureVisible = true);
    void filterAgain(bool andEnsureVisible = true);
    void filterAgainDelayed();
    bool isFiltering();

/// DRAG AND DROP:
private:
    bool m_isDuringDrag;
    QList<Note*> m_draggedNotes;
public:
    static void acceptDropEvent(QGraphicsSceneDragDropEvent *event, bool preCond = true);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void blindDrop(QGraphicsSceneDragDropEvent * event);
    void blindDrop(const QMimeData *mimeData, Qt::DropAction dropAction, QWidget *source);
    bool isDuringDrag() {
        return m_isDuringDrag;
    }
    QList<Note*> draggedNotes() {
        return m_draggedNotes;
    }
protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent *);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *);
public slots:
    void slotCopyingDone2(KIO::Job *job, const KUrl &from, const KUrl &to);
public:
    Note* noteForFullPath(const QString &path);

/// EXPORTATION:
public:
    QList<State*> usedStates();
    static QString saveGradientBackground(const QColor &color, const QFont &font, const QString &folder);

public:
    void listUsedTags(QList<Tag*> &list);

/// MANAGE FOCUS:
private:
    Note *m_focusedNote;
public:
    void setFocusedNote(Note *note);
    void focusANote();
    void focusANonSelectedNoteAbove(bool inSameColumn);
    void focusANonSelectedNoteBelow(bool inSameColumn);
    void focusANonSelectedNoteBelowOrThenAbove();
    void focusANonSelectedNoteAboveOrThenBelow();
    Note* focusedNote() {
        return m_focusedNote;
    }
    Note* firstNoteInStack();
    Note* lastNoteInStack();
    Note* firstNoteShownInStack();
    Note* lastNoteShownInStack();
    void selectRange(Note *start, Note *end, bool unselectOthers = true); /// FIXME: Not really a focus related method!
    void ensureNoteVisible(Note *note);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void focusInEvent(QFocusEvent*);
    virtual void focusOutEvent(QFocusEvent*);
    QRectF noteVisibleRect(Note *note); // clipped global (desktop as origin) rectangle
    Note* firstNoteInGroup();
    Note *noteOnHome();
    Note *noteOnEnd();

    enum NoteOn { LEFT_SIDE = 1, RIGHT_SIDE, TOP_SIDE, BOTTOM_SIDE };
    Note* noteOn(NoteOn side);

/// REIMPLEMENTED:
public:
    void deleteFiles();
    bool convertTexts();


public:
    void wheelEvent(QGraphicsSceneWheelEvent *event);



public:
    Note *m_startOfShiftSelectionNote;


/// THE NEW FILE WATCHER:
private:
    KDirWatch           *m_watcher;
    QTimer               m_watcherTimer;
    QList<QString>  m_modifiedFiles;
public:
    void addWatchedFile(const QString &fullPath);
    void removeWatchedFile(const QString &fullPath);
private slots:
    void watchedFileModified(const QString &fullPath);
    void watchedFileDeleted(const QString &fullPath);
    void updateModifiedNotes();


/// FROM OLD ARCHITECTURE **********************

public slots:

    void showFrameInsertTo() {}
    void resetInsertTo() {}

    void  computeInsertPlace(const QPointF &/*cursorPosition*/)    { }
public:

    friend class SystemTray;

/// SPEED OPTIMIZATION
private:
    bool m_finishLoadOnFirstShow;
    bool m_relayoutOnNextShow;
public:
    void aboutToBeActivated();

    
    QGraphicsView *graphicsView() { return m_view; }
private:
    QGraphicsView *m_view;
};

#endif // BASKET_H

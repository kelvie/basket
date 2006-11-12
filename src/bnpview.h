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

#ifndef BNPVIEW_H
#define BNPVIEW_H

#include <klistview.h>
#include <kxmlguiclient.h>
#include <qtimer.h>
#include <qclipboard.h>
#include <qsplitter.h>
#include <qlistview.h>
#include <dcopref.h>
#include "global.h"
#include "basketdcopiface.h"

 /// NEW:

class QWidgetStack;
class QDomDocument;
class QDomElement;
class KToggleAction;
class KPassivePopup;
class QPopupMenu;
class KPopupMenu;
class KTar;

class DesktopColorPicker;
class RegionGrabber;

class Basket;
class DecoratedBasket;
class BasketListViewItem;
class NoteSelection;
class BasketStatusBar;
class Tag;
class State;
class Note;

class BNPView : public QSplitter, virtual public BasketDcopInterface
{
	Q_OBJECT
	public:
	/// CONSTRUCTOR AND DESTRUCTOR:
		BNPView(QWidget *parent, const char *name, KXMLGUIClient *aGUIClient,
				KActionCollection *actionCollection, BasketStatusBar *bar);
		~BNPView();
	/// MANAGE CONFIGURATION EVENTS!:
		void setTreePlacement(bool onLeft);
		void relayoutAllBaskets();
		void recomputeAllStyles();
		void removedStates(const QValueList<State*> &deletedStates);
		void linkLookChanged();
		void filterPlacementChanged(bool onTop);
	/// MANAGE BASKETS:
		BasketListViewItem* listViewItemForBasket(Basket *basket);
		Basket* currentBasket();
		Basket* parentBasketOf(Basket *basket);
		void setCurrentBasket(Basket *basket);
		void removeBasket(Basket *basket);
	/// For NewBasketDialog (and later some other classes):
		QListViewItem* firstListViewItem();
		///
		BasketListViewItem* lastListViewItem();
		int basketCount(QListViewItem *parent = 0);
		bool canFold();
		bool canExpand();
		void enableActions();

	private:
		QDomElement basketElement(QListViewItem *item, QDomDocument &document, QDomElement &parentElement);
	public slots:
		void countsChanged(Basket *basket);
		void notesStateChanged();
		bool convertTexts();

		void updateBasketListViewItem(Basket *basket);
		void save();
		void save(QListViewItem *firstItem, QDomDocument &document, QDomElement &parentElement);
		void saveSubHierarchy(QListViewItem *item, QDomDocument &document, QDomElement &parentElement, bool recursive);
		void load();
		void load(KListView *listView, QListViewItem *item, const QDomElement &baskets);
		void loadNewBasket(const QString &folderName, const QDomElement &properties, Basket *parent);
		void goToPreviousBasket();
		void goToNextBasket();
		void foldBasket();
		void expandBasket();
		void closeAllEditors();
		///
		void toggleFilterAllBaskets(bool doFilter);
		void newFilter();
		void newFilterFromFilterBar();
		bool isFilteringAllBaskets();
		// From main window
		void importKNotes();
		void importKJots();
		void importKnowIt();
		void importTuxCards();
		void importStickyNotes();
		void importTomboy();

		/** Note */
		void activatedTagShortcut();
		void exportToHTML();
		void editNote();
		void cutNote();
		void copyNote();
		void delNote();
		void openNote();
		void openNoteWith();
		void saveNoteAs();
		void noteGroup();
		void noteUngroup();
		void moveOnTop();
		void moveOnBottom();
		void moveNoteUp();
		void moveNoteDown();
		void slotSelectAll();
		void slotUnselectAll();
		void slotInvertSelection();
		void slotResetFilter();

		void slotColorFromScreen(bool global = false);
		void slotColorFromScreenGlobal();
		void colorPicked(const QColor &color);
		void colorPickingCanceled();
		void slotConvertTexts();

		/** Global shortcuts */
		void addNoteText();
		void addNoteHtml();
		void addNoteImage();
		void addNoteLink();
		void addNoteColor();
		/** Passive Popups for Global Actions */
		void showPassiveDropped(const QString &title);
		void showPassiveDroppedDelayed(); // Do showPassiveDropped(), but delayed
		void showPassiveContent(bool forceShow = false);
		void showPassiveContentForced();
		void showPassiveImpossible(const QString &message);
		void showPassiveLoading(Basket *basket);
		// For GUI :
		void setFiltering(bool filtering);
		/** Edit */
		void undo();
		void redo();
		void globalPasteInCurrentBasket();
		void pasteInCurrentBasket();
		void pasteSelInCurrentBasket();
		void pasteToBasket(int index, QClipboard::Mode mode = QClipboard::Clipboard);
		void showHideFilterBar(bool show, bool switchFocus = true);
		/** Insert **/
		void insertEmpty(int type);
		void insertWizard(int type);
		void grabScreenshot(bool global = false);
		void grabScreenshotGlobal();
		void screenshotGrabbed(const QPixmap &pixmap);
		/** Basket */
		void askNewBasket();
		void askNewBasket(Basket *parent, Basket *pickProperties);
		void askNewSubBasket();
		void askNewSiblingBasket();
		void aboutToHideNewBasketPopup();
		void setNewBasketPopup();
		void cancelNewBasketPopup();
		void propBasket();
		void delBasket();
		void doBasketDeletion(Basket *basket);
		void password();
		void saveAsArchive();
		void openArchive();
		void lockBasket();
		void hideOnEscape();

		void changedSelectedNotes();
		void timeoutTryHide();
		void timeoutHide();
	private:
		void saveBasketToArchive(Basket *basket, bool recursive, KTar *tar);

	private slots:
		void updateNotesActions();
		void slotBasketNumberChanged(int number);
		void slotBasketChanged();
		void currentBasketChanged();
		void isLockedChanged();
		void lateInit();
		void onFirstShow();
		void showGlobalShortcutsSettingsDialog();

	public:
		KAction       *m_actEditNote;
		KAction       *m_actOpenNote;
		KAction       *m_actPaste;
		KAction       *m_actGrabScreenshot;
		KAction       *m_actColorPicker;
		KAction       *m_actLockBasket;
		KAction       *m_actPassBasket;
		KAction       *actNewBasket;
		KAction       *actNewSubBasket;
		KAction       *actNewSiblingBasket;
		KAction       *m_actHideWindow;
		KAction       *m_actExportToHtml;
		KAction       *m_actPropBasket;
		KAction       *m_actDelBasket;
		KToggleAction *m_actFilterAllBaskets;

	private:
		// Basket actions:
		KAction       *m_actSaveAsArchive;
		KAction       *m_actOpenArchive;
		// Notes actions :
		KAction       *m_actOpenNoteWith;
		KAction       *m_actSaveNoteAs;
		KAction       *m_actGroup;
		KAction       *m_actUngroup;
		KAction       *m_actMoveOnTop;
		KAction       *m_actMoveNoteUp;
		KAction       *m_actMoveNoteDown;
		KAction       *m_actMoveOnBottom;
		// Edit actions :
		KAction       *m_actUndo;
		KAction       *m_actRedo;
		KAction       *m_actCutNote;
		KAction       *m_actCopyNote;
		KAction       *m_actDelNote;
		KAction       *m_actSelectAll;
		KAction       *m_actUnselectAll;
		KAction       *m_actInvertSelection;
		// Insert actions :
//		KAction       *m_actInsertText;
		KAction       *m_actInsertHtml;
		KAction       *m_actInsertLink;
		KAction       *m_actInsertImage;
		KAction       *m_actInsertColor;
		KAction       *m_actImportKMenu;
		KAction       *m_actInsertLauncher;
		KAction       *m_actImportIcon;
		KAction       *m_actLoadFile;
		QPtrList<KAction> m_insertActions;
		// Basket actions :
		KToggleAction *m_actShowFilter;
		KAction       *m_actResetFilter;
		// Go actions :
		KAction       *m_actPreviousBasket;
		KAction       *m_actNextBasket;
		KAction       *m_actFoldBasket;
		KAction       *m_actExpandBasket;
//		KAction       *m_convertTexts; // FOR_BETA_PURPOSE
		KAction       *actConfigGlobalShortcuts;

		Basket* loadBasket(const QString &folderName);
		BasketListViewItem* appendBasket(Basket *basket, QListViewItem *parentItem);
		void setupActions();
		void setupGlobalShortcuts();
		DecoratedBasket* currentDecoratedBasket();

	public:
		Basket* basketForFolderName(const QString &folderName);
		QPopupMenu* popupMenu(const QString &menuName);
		bool isPart();
		bool isMainWindowActive();
		void showMainWindow();

		// dcop calls
		virtual void newBasket();
		virtual void handleCommandLine();

	public slots:
		void setCaption(QString s);
		void updateStatusBarHint();
		void setSelectionStatus(QString s);
		void setLockStatus(bool isLocked);
		void postStatusbarMessage(const QString&);
		void setStatusBarHint(const QString&);
		void setUnsavedStatus(bool isUnsaved);
		void setActive(bool active = true);
		KActionCollection *actionCollection() { return m_actionCollection; };

		void populateTagsMenu();
		void populateTagsMenu(KPopupMenu &menu, Note *referenceNote);
		void connectTagsMenu();
		void disconnectTagsMenu();
		void disconnectTagsMenuDelayed();
	protected:
		void showEvent(QShowEvent*);
		void hideEvent(QHideEvent*);
	private:
		KPopupMenu *m_lastOpenedTagsMenu;

	private slots:
		void slotPressed(QListViewItem *item, const QPoint &/*pos*/ = QPoint(), int /*column*/ = 0);
		void needSave(QListViewItem*);
		void slotContextMenu(KListView *listView, QListViewItem *item, const QPoint &pos);
		void slotMouseButtonPressed(int button, QListViewItem *item, const QPoint &pos, int column);
		void slotShowProperties(QListViewItem *item, const QPoint&, int);
		void initialize();

	signals:
		void basketNumberChanged(int number);
		void basketChanged();
		void setWindowCaption(const QString &s);
		void showPart();

	protected:
		void enterEvent(QEvent*);
		void leaveEvent(QEvent*);

	private:
		KListView    *m_tree;
		QWidgetStack *m_stack;
		bool          m_loading;
		bool          m_newBasketPopup;
		bool          m_firstShow;
		DesktopColorPicker *m_colorPicker;
		bool                m_colorPickWasShown;
		bool                m_colorPickWasGlobal;
		RegionGrabber      *m_regionGrabber;
		QString m_passiveDroppedTitle;
		NoteSelection *m_passiveDroppedSelection;
		KPassivePopup *m_passivePopup;
		static const int c_delayTooltipTime;
		KActionCollection *m_actionCollection;
		KXMLGUIClient *m_guiClient;
		BasketStatusBar *m_statusbar;
		QTimer             *m_tryHideTimer;
		QTimer             *m_hideTimer;
};

#endif // BNPVIEW_H

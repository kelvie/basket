/***************************************************************************
 *   Copyright (C) 2005 by S�astien Laot                                 *
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

#ifndef TAGEDIT_H
#define TAGEDIT_H

#include <kdialog.h>
#include <kcombobox.h>
#include <qlistview.h>
#include <QList>

class QGroupBox;
class QLineEdit;
class QCheckBox;
class KPushButton;
class KKeyButton;
class KIconButton;
class KFontCombo;
class QLabel;
class KShortcut;

class KColorCombo2;

class FontSizeCombo;

class Tag;
class State;

class StateCopy
{
  public:
	typedef QList<StateCopy*> List;
	StateCopy(State *old = 0);
	~StateCopy();
	State *oldState;
	State *newState;
	void copyBack();
};

class TagCopy
{
  public:
	typedef QList<TagCopy*> List;
	TagCopy(Tag *old = 0);
	~TagCopy();
	Tag *oldTag;
	Tag *newTag;
	StateCopy::List stateCopies;
	void copyBack();
	bool isMultiState();
};

class TagListViewItem : public QListViewItem
{
  public:
	TagListViewItem(QListWidgetItem     *parent, TagCopy *tagCopy);
	TagListViewItem(QListViewItem *parent, TagCopy *tagCopy);
	TagListViewItem(QListWidgetItem     *parent, QListViewItem *after, TagCopy *tagCopy);
	TagListViewItem(QListViewItem *parent, QListViewItem *after, TagCopy *tagCopy);
	TagListViewItem(QListWidgetItem     *parent, StateCopy *stateCopy);
	TagListViewItem(QListViewItem *parent, StateCopy *stateCopy);
	TagListViewItem(QListWidgetItem     *parent, QListViewItem *after, StateCopy *stateCopy);
	TagListViewItem(QListViewItem *parent, QListViewItem *after, StateCopy *stateCopy);
	~TagListViewItem();
	TagCopy*   tagCopy()   { return m_tagCopy;   }
	StateCopy* stateCopy() { return m_stateCopy; }
	bool isEmblemObligatory();
	TagListViewItem* lastChild();
	TagListViewItem* prevSibling();
	TagListViewItem* parent() const; // Reimplemented to cast the return value
	int width(const QFontMetrics &fontMetrics, const QListWidgetItem *listView, int column) const;
	void setup();
	void paintCell(QPainter *painter, const QColorGroup &colorGroup, int column, int width, int align);

  private:
	TagCopy   *m_tagCopy;
	StateCopy *m_stateCopy;
};

class TagListView : public QListView
{
  Q_OBJECT
  public:
	TagListView(QWidget *parent = 0, const char *name = 0, WFlags flags = 0);
	~TagListView();
	void keyPressEvent(QKeyEvent *event);
	void contentsMouseDoubleClickEvent(QMouseEvent *event);
	void contentsMousePressEvent(QMouseEvent *event);
	void contentsMouseReleaseEvent(QMouseEvent *event);
	TagListViewItem* currentItem() const; // Reimplemented to cast the return value
	TagListViewItem* firstChild() const; // Reimplemented to cast the return value
	TagListViewItem* lastItem() const; // Reimplemented to cast the return value
  signals:
	void deletePressed();
	void doubleClickedItem();
};

/**
  * @author S�astien Laot
  */
class TagsEditDialog : public KDialog
{
  Q_OBJECT
  public:
	TagsEditDialog(QWidget *parent = 0, State *stateToEdit = 0, bool addNewTag = false);
	~TagsEditDialog();
	State::List deletedStates() { return m_deletedStates; }
	State::List addedStates()   { return m_addedStates;   }
	TagListViewItem* itemForState(State *state);
  private slots:
	void newTag();
	void newState();
	void moveUp();
	void moveDown();
	void deleteTag();
	void renameIt();
	void capturedShortcut(const KShortcut &shortcut);
	void removeShortcut();
	void removeEmblem();
	void modified();
	void currentItemChanged(QListViewItem *item);
	void slotCancel();
	void slotOk();
	void selectUp();
	void selectDown();
	void selectLeft();
	void selectRight();
	void resetTreeSizeHint();
  private:
	void loadBlankState();
	void loadStateFrom(State *state);
	void loadTagFrom(Tag *tag);
	void saveStateTo(State *state);
	void saveTagTo(Tag *tag);
	void ensureCurrentItemVisible();
	TagListView   *m_tags;
	KPushButton   *m_moveUp;
	KPushButton   *m_moveDown;
	KPushButton   *m_deleteTag;
	QLineEdit     *m_tagName;
	KKeyButton    *m_shortcut;
	QPushButton   *m_removeShortcut;
	QCheckBox     *m_inherit;
	QGroupBox     *m_tagBox;
	QGroupBox     *m_stateBox;
	QLabel        *m_stateNameLabel;
	QLineEdit     *m_stateName;
	KIconButton   *m_emblem;
	QPushButton   *m_removeEmblem;
	QPushButton   *m_bold;
	QPushButton   *m_underline;
	QPushButton   *m_italic;
	QPushButton   *m_strike;
	KColorCombo2  *m_textColor;
	KFontCombo    *m_font;
	FontSizeCombo *m_fontSize;
	KColorCombo2  *m_backgroundColor;
	QLineEdit     *m_textEquivalent;
	QCheckBox     *m_onEveryLines;

	TagCopy::List m_tagCopies;
	State::List   m_deletedStates;
	State::List   m_addedStates;

	bool m_loading;
};

#endif // TAGEDIT_H

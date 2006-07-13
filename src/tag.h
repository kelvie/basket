/***************************************************************************
 *   Copyright (C) 2005 by Sébastien Laoût                                 *
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

#ifndef TAG_H
#define TAG_H

#include <qstring.h>
#include <qcolor.h>
#include <qfont.h>
#include <qvaluelist.h>

#include <kaction.h>
#include <kshortcut.h>

class QPainter;

class Tag;

/**
  * @author Sébastien Laoût
  */
class State
{
  public:
	/// LIST OF STATES:
	typedef QValueList<State*> List;

  public:
	/// CONSTRUCTOR AND DESTRUCTOR:
	State(const QString &id = QString(), Tag *tag = 0);
	~State();
	/// SET PROPERTIES:
	void setId(const QString &id)                { m_id              = id;        }
	void setName(const QString &name)            { m_name            = name;      }
	void setEmblem(const QString &emblem)        { m_emblem          = emblem;    }
	void setBold(bool bold)                      { m_bold            = bold;      }
	void setItalic(bool italic)                  { m_italic          = italic;    }
	void setUnderline(bool underline)            { m_underline       = underline; }
	void setStrikeOut(bool strikeOut)            { m_strikeOut       = strikeOut; }
	void setTextColor(const QColor &color)       { m_textColor       = color;     }
	void setFontName(const QString &font)        { m_fontName        = font;      }
	void setFontSize(int size)                   { m_fontSize        = size;      }
	void setBackgroundColor(const QColor &color) { m_backgroundColor = color;     }
	void setTextEquivalent(const QString &text)  { m_textEquivalent  = text;      }
	void setOnAllTextLines(bool yes)             { m_onAllTextLines  = yes;       }
	void setParentTag(Tag *tag)                  { m_parentTag       = tag;       }
	/// GET PROPERTIES:
	QString id()              const { return m_id;              }
	QString name()            const { return m_name;            }
	QString emblem()          const { return m_emblem;          }
	bool    bold()            const { return m_bold;            }
	bool    italic()          const { return m_italic;          }
	bool    underline()       const { return m_underline;       }
	bool    strikeOut()       const { return m_strikeOut;       }
	QColor  textColor()       const { return m_textColor;       }
	QString fontName()        const { return m_fontName;        }
	int     fontSize()        const { return m_fontSize;        }
	QColor  backgroundColor() const { return m_backgroundColor; }
	QString textEquivalent()  const { return m_textEquivalent;  }
	bool    onAllTextLines()  const { return m_onAllTextLines;  }
	Tag*    parentTag()       const { return m_parentTag;       }
	/// HELPING FUNCTIONS:
	State *nextState(bool cycle = true);
	QString fullName();
	QFont font(QFont base);
	QString toCSS(const QString &gradientFolderPath, const QString &gradientFolderName, const QFont &baseFont);
	static void merge(const List &states, State *result, int *emblemsCount, bool *haveInvisibleTags, const QColor &backgroundColor);
	void copyTo(State *other);
  private:
	/// PROPERTIES:
	QString  m_id;
	QString  m_name;
	QString  m_emblem;
	bool     m_bold;
	bool     m_italic;
	bool     m_underline;
	bool     m_strikeOut;
	QColor   m_textColor;
	QString  m_fontName;
	int      m_fontSize;
	QColor   m_backgroundColor;
	QString  m_textEquivalent;
	bool     m_onAllTextLines;
	Tag     *m_parentTag;
};

/** A Tag is a category of Notes.
  * A Note can have 0, 1 or more Tags.
  * A Tag can have a unique State or several States.
  * @author Sébastien Laoût
  */
class Tag
{
  public:
	/// LIST OF ALL TAGS IN THE APPLICATION:
	typedef QValueList<Tag*> List;
	static Tag::List all;
	static State* stateForId(const QString &id);
	static Tag* tagForKAction(KAction *action);
	static void loadTags(/*const QString &path = QString(), bool merge*/); /// << Load the tags contained in the XML file @p path or those in the application settings if @p path isEmpty(). If @p merge is true and a tag with the id of a tag that should be loaded already exist, the tag will get a new id. Otherwise, the tag will be dismissed.
	static void saveTags();
	static void createDefaultTagsSet(const QString &file);

  public:
	/// CONSTRUCTOR AND DESTRUCTOR:
	Tag(/*State *firstState, const QString &name, bool inheritedBySiblings*/);
	~Tag();
	/// SET PROPERTIES:
	void setName(const QString &name)           { m_name = name;                                     }
	void setShortcut(const KShortcut &shortcut) { m_action->setShortcut(shortcut);                   }
	void setInheritedBySiblings(bool inherited) { m_inheritedBySiblings = inherited;                 }
	void appendState(State *state)              { m_states.append(state); state->setParentTag(this); }
	void removeState(State *state)              { m_states.remove(state); state->setParentTag(0);    }
	/// GET PROPERTIES:
	QString      name()                const { return m_name;                 }
	KShortcut    shortcut()            const { return m_action->shortcut();   }
	bool         inheritedBySiblings() const { return m_inheritedBySiblings;  }
	State::List& states()              const { return (State::List&)m_states; }
	int          countStates()         const { return m_states.count();       }
	void copyTo(Tag *other);
  private:
	/// PROPERTIES:
	QString      m_name;
	KAction     *m_action;
	bool         m_inheritedBySiblings;
	State::List  m_states;
};

#include <qiconset.h>
#include <qmenudata.h>
#include <qstring.h>

class QPainter;

/** A menu item to indent icon and text (to keep place for a checkbox or a radiobutton on left).
  * You should not set any icon when adding this entry to the menu.
  * Instead, the constructor take the icon and the item take care to draw it itself.
  * Better suited to be used with StateMenuItem (or TagMenuItem).
  * @author Sébastien Laoût
  */
class IndentedMenuItem : public QCustomMenuItem
{
  public:
	IndentedMenuItem(const QString &text, const QString &icon = "", const QString &shortcut = "");
	~IndentedMenuItem();
	void paint(QPainter *painter, const QColorGroup &cg, bool active, bool enabled, int x, int y, int w, int h);
	QSize sizeHint();
	bool fullSpan() { return true; }
  private:
	QString  m_text;
	QString  m_icon;
	QString  m_shortcut;
};

/** A menu item representing a State or a Tag.
  * @author Sébastien Laoût
  */
class StateMenuItem : public QCustomMenuItem
{
  public:
	StateMenuItem(State *state, const QString &shortcut, bool withTagName = false);
	~StateMenuItem();
	void paint(QPainter *painter, const QColorGroup &cg, bool active, bool enabled, int x, int y, int w, int h);
	QSize sizeHint();
	bool fullSpan() { return true; }
  private:
	State   *m_state;
	QString  m_name;
	QString  m_shortcut;
  public:
	static QIconSet checkBoxIconSet(bool checked, QColorGroup cg);
	static QIconSet radioButtonIconSet(bool checked, QColorGroup cg);
	static int iconMargin() { return 5; }
};

#endif // TAG_H

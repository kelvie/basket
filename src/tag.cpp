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

#include <kapplication.h>
#include <kstyle.h>
#include <kiconloader.h>
#include <qpainter.h>
#include <qfont.h>
#include <qdom.h>
#include <qdir.h>
//Added by qt3to4:
#include <QTextStream>
#include <Q3ValueList>
#include <QPixmap>
#include <klocale.h>

#include <kactioncollection.h>

#include "tag.h"
#include "xmlwork.h"
#include "global.h"
#include "debugwindow.h"
#include "bnpview.h"
#include "tools.h"
#include "basket.h"

/** class State: */

State::State(const QString &id, Tag *tag)
 : m_id(id), m_name(), m_emblem(), m_bold(false), m_italic(false), m_underline(false), m_strikeOut(false),
   m_textColor(), m_fontName(), m_fontSize(-1), m_backgroundColor(), m_textEquivalent(), m_onAllTextLines(false), m_parentTag(tag)
{
}

State::~State()
{
}

State* State::nextState(bool cycle /*= true*/)
{
	if (!parentTag())
		return 0;

	List states = parentTag()->states();
	// The tag contains only one state:
	if (states.count() == 1)
		return 0;
	// Find the next state:
	for (List::iterator it = states.begin(); it != states.end(); ++it)
		// Found the current state in the list:
		if (*it == this) {
			// Find the next state:
			State *next = *(++it);
			if (it == states.end())
				return (cycle ? states.first() : 0);
			return next;
		}
	// Should not happens:
	Q_ASSERT(false);
	return 0;
}

QString State::fullName()
{
	if (!parentTag() || parentTag()->states().count() == 1)
		return (name().isEmpty() && parentTag() ? parentTag()->name() : name());
	return QString(i18n("%1: %2",parentTag()->name(), name()));
}

QFont State::font(QFont base)
{
	if (bold())
		base.setBold(true);
	if (italic())
		base.setItalic(true);
	if (underline())
		base.setUnderline(true);
	if (strikeOut())
		base.setStrikeOut(true);
	if (!fontName().isEmpty())
		base.setFamily(fontName());
	if (fontSize() > 0)
		base.setPointSize(fontSize());
	return base;
}

QString State::toCSS(const QString &gradientFolderPath, const QString &gradientFolderName, const QFont &baseFont)
{
	QString css;
	if (bold())
		css += " font-weight: bold;";
	if (italic())
		css += " font-style: italic;";
	if (underline() && strikeOut())
		css += " text-decoration: underline line-through;";
	else if (underline())
		css += " text-decoration: underline;";
	else if (strikeOut())
		css += " text-decoration: line-through;";
	if (textColor().isValid())
		css += " color: " + textColor().name() + ";";
	if (!fontName().isEmpty()) {
		QString fontFamily = Tools::cssFontDefinition(fontName(), /*onlyFontFamily=*/true);
		css += " font-family: " + fontFamily + ";";
	}
	if (fontSize() > 0)
		css += " font-size: " + QString::number(fontSize()) + "px;";
	if (backgroundColor().isValid()) {
		// Get the colors of the gradient and the border:
		QColor topBgColor;
		QColor bottomBgColor;
		Note::getGradientColors(backgroundColor(), &topBgColor, &bottomBgColor);
		// Produce the CSS code:
		QString gradientFileName = Basket::saveGradientBackground(backgroundColor(), font(baseFont), gradientFolderPath);
		css += " background: " + bottomBgColor.name() + " url('" + gradientFolderName + gradientFileName + "') repeat-x;";
		css += " border-top: solid " + topBgColor.name() + " 1px;";
		css += " border-bottom: solid " + Tools::mixColor(topBgColor, bottomBgColor).name() + " 1px;";
	}

	if (css.isEmpty())
		return "";
	else
		return "   .tag_" + id() + " {" + css + " }\n";
}

void State::merge(const List &states, State *result, int *emblemsCount, bool *haveInvisibleTags, const QColor &backgroundColor)
{
	*result            = State(); // Reset to default values.
	*emblemsCount      = 0;
	*haveInvisibleTags = false;

	for (List::const_iterator it = states.begin(); it != states.end(); ++it) {
		State *state = *it;
		bool isVisible = false;
		// For each propertie, if that properties have a value (is not default) is the current state of the list,
		// and if it haven't been set to the result state by a previous state, then it's visible and we assign the propertie to the result state.
		if (!state->emblem().isEmpty()) {
			++*emblemsCount;
			isVisible = true;
		}
		if (state->bold() && !result->bold()) {
			result->setBold(true);
			isVisible = true;
		}
		if (state->italic() && !result->italic()) {
			result->setItalic(true);
			isVisible = true;
		}
		if (state->underline() && !result->underline()) {
			result->setUnderline(true);
			isVisible = true;
		}
		if (state->strikeOut() && !result->strikeOut()) {
			result->setStrikeOut(true);
			isVisible = true;
		}
		if (state->textColor().isValid() && !result->textColor().isValid()) {
			result->setTextColor(state->textColor());
			isVisible = true;
		}
		if (!state->fontName().isEmpty() && result->fontName().isEmpty()) {
			result->setFontName(state->fontName());
			isVisible = true;
		}
		if (state->fontSize() > 0 && result->fontSize() <= 0) {
			result->setFontSize(state->fontSize());
			isVisible = true;
		}
		if (state->backgroundColor().isValid() && !result->backgroundColor().isValid() && state->backgroundColor() != backgroundColor) { // vv
			result->setBackgroundColor(state->backgroundColor()); // This is particular: if the note background color is the same as the basket one, don't use that.
			isVisible = true;
		}
		// If it's not visible, well, at least one tag is not visible: the note will display "..." at the tags arrow place to show that:
		if (!isVisible)
			*haveInvisibleTags = true;
	}
}

void State::copyTo(State *other)
{
	other->m_id              = m_id;
	other->m_name            = m_name;
	other->m_emblem          = m_emblem;
	other->m_bold            = m_bold;
	other->m_italic          = m_italic;
	other->m_underline       = m_underline;
	other->m_strikeOut       = m_strikeOut;
	other->m_textColor       = m_textColor;
	other->m_fontName        = m_fontName;
	other->m_fontSize        = m_fontSize;
	other->m_backgroundColor = m_backgroundColor;
	other->m_textEquivalent  = m_textEquivalent;
	other->m_onAllTextLines  = m_onAllTextLines; // TODO
	//TODO: other->m_parentTag;
}

/** class Tag: */

Tag::List Tag::all = Tag::List();

long Tag::nextStateUid = 1;

long Tag::getNextStateUid()
{
	return nextStateUid++; // Return the next Uid and THEN increment the Uid
}

Tag::Tag()
{
	static int tagNumber = 0;
	++tagNumber;
	QString sAction = "tag_shortcut_number_" + QString::number(tagNumber);

	KActionCollection *ac = Global::bnpView->actionCollection();
	m_action = ac->addAction(sAction, Global::bnpView,
				 SLOT(activatedTagShortcut()));
	m_action->setText("FAKE TEXT");
	m_action->setIcon(KIcon("FAKE ICON"));

	m_action->setShortcutConfigurable(false); // We do it in the tag properties dialog

	m_inheritedBySiblings = false;
}

Tag::~Tag()
{
	delete m_action;
}

void Tag::setName(const QString &name)
{
	m_name = name;
	m_action->setText("TAG SHORTCUT: " + name); // TODO: i18n  (for debug purpose only by now).
}

State* Tag::stateForId(const QString &id)
{
	for (List::iterator it = all.begin(); it != all.end(); ++it)
		for (State::List::iterator it2 = (*it)->states().begin(); it2 != (*it)->states().end(); ++it2)
			if ((*it2)->id() == id)
				return *it2;
	return 0;
}

Tag* Tag::tagForKAction(KAction *action)
{
	for (List::iterator it = all.begin(); it != all.end(); ++it)
		if ((*it)->m_action == action)
			return *it;
	return 0;
}

QMap<QString, QString> Tag::loadTags(const QString &path/* = QString()*//*, bool merge = false*/)
{
	QMap<QString, QString> mergedStates;

	bool merge = !path.isEmpty();
	QString fullPath = (merge ? path : Global::savesFolder() + "tags.xml");
	QString doctype  = "basketTags";

	QDir dir;
	if (!dir.exists(fullPath)) {
		if (merge)
			return mergedStates;
		DEBUG_WIN << "Tags file does not exist: Creating it...";
		createDefaultTagsSet(fullPath);
	}

	QDomDocument *document = XMLWork::openFile(doctype, fullPath);
	if (!document) {
		DEBUG_WIN << "<font color=red>FAILED to read the tags file</font>";
		return mergedStates;
	}

	QDomElement docElem = document->documentElement();
	if (!merge)
		nextStateUid = docElem.attribute("nextStateUid", QString::number(nextStateUid)).toLong();

	QDomNode node = docElem.firstChild();
	while (!node.isNull()) {
		QDomElement element = node.toElement();
		if ( (!element.isNull()) && element.tagName() == "tag" ) {
			Tag *tag = new Tag();
			// Load properties:
			QString name      = XMLWork::getElementText(element, "name");
			QString shortcut  = XMLWork::getElementText(element, "shortcut");
			QString inherited = XMLWork::getElementText(element, "inherited", "false");
			tag->setName(name);
			tag->setShortcut(KShortcut(shortcut));
			tag->setInheritedBySiblings(XMLWork::trueOrFalse(inherited));
			// Load states:
			QDomNode subNode = element.firstChild();
			while (!subNode.isNull()) {
				QDomElement subElement = subNode.toElement();
				if ( (!subElement.isNull()) && subElement.tagName() == "state" ) {
					State *state = new State(subElement.attribute("id"), tag);
					state->setName(   XMLWork::getElementText(subElement, "name")   );
					state->setEmblem( XMLWork::getElementText(subElement, "emblem") );
					QDomElement textElement = XMLWork::getElement(subElement, "text");
					state->setBold(      XMLWork::trueOrFalse(textElement.attribute("bold",      "false")) );
					state->setItalic(    XMLWork::trueOrFalse(textElement.attribute("italic",    "false")) );
					state->setUnderline( XMLWork::trueOrFalse(textElement.attribute("underline", "false")) );
					state->setStrikeOut( XMLWork::trueOrFalse(textElement.attribute("strikeOut", "false")) );
					QString textColor = textElement.attribute("color", "");
					state->setTextColor(textColor.isEmpty() ? QColor() : QColor(textColor));
					QDomElement fontElement = XMLWork::getElement(subElement, "font");
					state->setFontName(fontElement.attribute("name", ""));
					QString fontSize = fontElement.attribute("size", "");
					state->setFontSize(fontSize.isEmpty() ? -1 : fontSize.toInt());
					QString backgroundColor = XMLWork::getElementText(subElement, "backgroundColor", "");
					state->setBackgroundColor(backgroundColor.isEmpty() ? QColor() : QColor(backgroundColor));
					QDomElement textEquivalentElement = XMLWork::getElement(subElement, "textEquivalent");
					state->setTextEquivalent( textEquivalentElement.attribute("string", "") );
					state->setOnAllTextLines( XMLWork::trueOrFalse(textEquivalentElement.attribute("onAllTextLines", "false")) );
					tag->appendState(state);
				}
				subNode = subNode.nextSibling();
			}
			// If the Tag is Valid:
			if (tag->countStates() > 0) {
				// Rename Things if Needed:
				State *firstState = tag->states().first();
				if (tag->countStates() == 1 && firstState->name().isEmpty())
					firstState->setName(tag->name());
				if (tag->name().isEmpty())
					tag->setName(firstState->name());
				// Add or Merge the Tag:
				if (!merge) {
					all.append(tag);
				} else {
					Tag *similarTag = tagSimilarTo(tag);
					// Tag does not exists, add it:
					if (similarTag == 0) {
						// We are merging the new states, so we should choose new and unique (on that computer) ids for those states:
						for (State::List::iterator it = tag->states().begin(); it != tag->states().end(); ++it) {
							State *state = *it;
							QString uid    = state->id();
							QString newUid = "tag_state_" + QString::number(getNextStateUid());
							state->setId(newUid);
							mergedStates[uid] = newUid;
						}
						// TODO: if shortcut is already assigned to a previous note, do not import it, keep the user settings!
						all.append(tag);
					// Tag already exists, rename to theire ids:
					} else {
						State::List::iterator it2 = similarTag->states().begin();
						for (State::List::iterator it = tag->states().begin(); it != tag->states().end(); ++it, ++it2) {
							State *state        = *it;
							State *similarState = *it2;
							QString uid    = state->id();
							QString newUid = similarState->id();
							if (uid != newUid)
								mergedStates[uid] = newUid;
						}
						delete tag; // Already exists, not to be merged. Delete the shortcut and all.
					}
				}
			}
		}
		node = node.nextSibling();
	}

	return mergedStates;
}

Tag* Tag::tagSimilarTo(Tag *tagToTest)
{
	// Tags are considered similar if they have the same name, the same number of states, in the same order, and the same look.
	// Keyboard shortcut, text equivalent and onEveryLines are user settings, and thus not considered during the comparision.
	// Default tags (To Do, Important, Idea...) do not take into account the name of the tag and states during the comparision.
	// Default tags are equal only if they have the same number of states, in the same order, and the same look.
	// This is because default tag names are translated differently in every countries, but they are essentialy the same!
	// User tags begins with "tag_state_" followed by a number. Default tags are the other ones.

	// Browse all tags:
	for (List::iterator it = all.begin(); it != all.end(); ++it) {
		Tag *tag = *it;
		bool same = true;
		bool sameName;
		bool defaultTag = true;
		// We test only name and look. Shorcut and whenever it is inherited by sibling new notes are user settings only!
		sameName = tag->name() == tagToTest->name();
		if (tag->countStates() != tagToTest->countStates())
			continue; // Tag is different!
		// We found a tag with same name, check if every states/look are same too:
		State::List::iterator itTest = tagToTest->states().begin();
		for (State::List::iterator it2 = (*it)->states().begin(); it2 != (*it)->states().end(); ++it2, ++itTest) {
			State *state       = *it2;
			State *stateToTest = *itTest;
			if (state->id().startsWith("tag_state_") || stateToTest->id().startsWith("tag_state_")) { defaultTag = false; }
			if (state->name()            != stateToTest->name())            { sameName = false;    }
			if (state->emblem()          != stateToTest->emblem())          { same = false; break; }
			if (state->bold()            != stateToTest->bold())            { same = false; break; }
			if (state->italic()          != stateToTest->italic())          { same = false; break; }
			if (state->underline()       != stateToTest->underline())       { same = false; break; }
			if (state->strikeOut()       != stateToTest->strikeOut())       { same = false; break; }
			if (state->textColor()       != stateToTest->textColor())       { same = false; break; }
			if (state->fontName()        != stateToTest->fontName())        { same = false; break; }
			if (state->fontSize()        != stateToTest->fontSize())        { same = false; break; }
			if (state->backgroundColor() != stateToTest->backgroundColor()) { same = false; break; }
			// Text equivalent (as well as onAllTextLines) is also a user setting!
		}
		// We found an existing tag that is "exactly" the same:
		if (same && (sameName || defaultTag))
			return tag;
	}

	// Not found:
	return 0;
}

void Tag::saveTags()
{
	DEBUG_WIN << "Saving tags...";
	saveTagsTo(all, Global::savesFolder() + "tags.xml");
}

void Tag::saveTagsTo(Q3ValueList<Tag*> &list, const QString &fullPath)
{
	// Create Document:
	QDomDocument document(/*doctype=*/"basketTags");
	QDomElement root = document.createElement("basketTags");
	root.setAttribute("nextStateUid", static_cast<long long int>(nextStateUid) );
	document.appendChild(root);

	// Save all tags:
	for (List::iterator it = list.begin(); it != list.end(); ++it) {
		Tag *tag = *it;
		// Create tag node:
		QDomElement tagNode = document.createElement("tag");
		root.appendChild(tagNode);
		// Save tag properties:
		XMLWork::addElement( document, tagNode, "name",      tag->name()                                      );
		XMLWork::addElement( document, tagNode, "shortcut", tag->shortcut().primary().toString());
		XMLWork::addElement( document, tagNode, "inherited", XMLWork::trueOrFalse(tag->inheritedBySiblings()) );
		// Save all states:
		for (State::List::iterator it2 = (*it)->states().begin(); it2 != (*it)->states().end(); ++it2) {
			State *state = *it2;
			// Create state node:
			QDomElement stateNode = document.createElement("state");
			tagNode.appendChild(stateNode);
			// Save state properties:
			stateNode.setAttribute("id", state->id());
			XMLWork::addElement( document, stateNode, "name",   state->name()   );
			XMLWork::addElement( document, stateNode, "emblem", state->emblem() );
			QDomElement textNode = document.createElement("text");
			stateNode.appendChild(textNode);
			QString textColor = (state->textColor().isValid() ? state->textColor().name() : "");
			textNode.setAttribute( "bold",      XMLWork::trueOrFalse(state->bold())      );
			textNode.setAttribute( "italic",    XMLWork::trueOrFalse(state->italic())    );
			textNode.setAttribute( "underline", XMLWork::trueOrFalse(state->underline()) );
			textNode.setAttribute( "strikeOut", XMLWork::trueOrFalse(state->strikeOut()) );
			textNode.setAttribute( "color",     textColor                                );
			QDomElement fontNode = document.createElement("font");
			stateNode.appendChild(fontNode);
			fontNode.setAttribute( "name", state->fontName() );
			fontNode.setAttribute( "size", state->fontSize() );
			QString backgroundColor = (state->backgroundColor().isValid() ? state->backgroundColor().name() : "");
			XMLWork::addElement( document, stateNode, "backgroundColor", backgroundColor );
			QDomElement textEquivalentNode = document.createElement("textEquivalent");
			stateNode.appendChild(textEquivalentNode);
			textEquivalentNode.setAttribute( "string",         state->textEquivalent()                       );
			textEquivalentNode.setAttribute( "onAllTextLines", XMLWork::trueOrFalse(state->onAllTextLines()) );
		}
	}

	// Write to Disk:
	if (!Basket::safelySaveToFile(fullPath, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n" + document.toString()))
		DEBUG_WIN << "<font color=red>FAILED to save tags</font>!";
}

void Tag::copyTo(Tag *other)
{
	other->m_name = m_name;
	other->m_action->setShortcut(m_action->shortcut());
	other->m_inheritedBySiblings =  m_inheritedBySiblings;
}

void Tag::createDefaultTagsSet(const QString &fullPath)
{
	QString xml = QString(
		"<!DOCTYPE basketTags>\n"
		"<basketTags>\n"
		"  <tag>\n"
		"    <name>%1</name>\n" // "To Do"
		"    <shortcut>Ctrl+1</shortcut>\n"
		"    <inherited>true</inherited>\n"
		"    <state id=\"todo_unchecked\">\n"
		"      <name>%2</name>\n" // "Unchecked"
		"      <emblem>tag_checkbox</emblem>\n"
		"      <text bold=\"false\" italic=\"false\" underline=\"false\" strikeOut=\"false\" color=\"\" />\n"
		"      <font name=\"\" size=\"\" />\n"
		"      <backgroundColor></backgroundColor>\n"
		"      <textEquivalent string=\"[ ]\" onAllTextLines=\"false\" />\n"
		"    </state>\n"
		"    <state id=\"todo_done\">\n"
		"      <name>%3</name>\n" // "Done"
		"      <emblem>tag_checkbox_checked</emblem>\n"
		"      <text bold=\"false\" italic=\"false\" underline=\"false\" strikeOut=\"true\" color=\"\" />\n"
		"      <font name=\"\" size=\"\" />\n"
		"      <backgroundColor></backgroundColor>\n"
		"      <textEquivalent string=\"[x]\" onAllTextLines=\"false\" />\n"
		"    </state>\n"
		"  </tag>\n"
		"\n"
		"  <tag>\n"
		"    <name>%4</name>\n" // "Progress"
		"    <shortcut>Ctrl+2</shortcut>\n"
		"    <inherited>true</inherited>\n"
		"    <state id=\"progress_000\">\n"
		"      <name>%5</name>\n" // "0 %"
		"      <emblem>tag_progress_000</emblem>\n"
		"      <textEquivalent string=\"[    ]\" />\n"
		"    </state>\n"
		"    <state id=\"progress_025\">\n"
		"      <name>%6</name>\n" // "25 %"
		"      <emblem>tag_progress_025</emblem>\n"
		"      <textEquivalent string=\"[=   ]\" />\n"
		"    </state>\n"
		"    <state id=\"progress_050\">\n"
		"      <name>%7</name>\n" // "50 %"
		"      <emblem>tag_progress_050</emblem>\n"
		"      <textEquivalent string=\"[==  ]\" />\n"
		"    </state>\n"
		"    <state id=\"progress_075\">\n"
		"      <name>%8</name>\n" // "75 %"
		"      <emblem>tag_progress_075</emblem>\n"
		"      <textEquivalent string=\"[=== ]\" />\n"
		"    </state>\n"
		"    <state id=\"progress_100\">\n"
		"      <name>%9</name>\n" // "100 %"
		"      <emblem>tag_progress_100</emblem>\n"
		"      <textEquivalent string=\"[====]\" />\n"
		"    </state>\n"
		"  </tag>\n"
		"\n")
			.arg( i18n("To Do"),     i18n("Unchecked"),      i18n("Done")        )  // %1 %2 %3
			.arg( i18n("Progress"),  i18n("0 %"),            i18n("25 %")        )  // %4 %5 %6
			.arg( i18n("50 %"),      i18n("75 %"),           i18n("100 %")       )  // %7 %8 %9
	+ QString(
		"  <tag>\n"
		"    <name>%1</name>\n" // "Priority"
		"    <shortcut>Ctrl+3</shortcut>\n"
		"    <inherited>true</inherited>\n"
		"    <state id=\"priority_low\">\n"
		"      <name>%2</name>\n" // "Low"
		"      <emblem>tag_priority_low</emblem>\n"
		"      <textEquivalent string=\"{1}\" />\n"
		"    </state>\n"
		"    <state id=\"priority_medium\">\n"
		"      <name>%3</name>\n" // "Medium
		"      <emblem>tag_priority_medium</emblem>\n"
		"      <textEquivalent string=\"{2}\" />\n"
		"    </state>\n"
		"    <state id=\"priority_high\">\n"
		"      <name>%4</name>\n" // "High"
		"      <emblem>tag_priority_high</emblem>\n"
		"      <textEquivalent string=\"{3}\" />\n"
		"    </state>\n"
		"  </tag>\n"
		"\n"
		"  <tag>\n"
		"    <name>%5</name>\n" // "Preference"
		"    <shortcut>Ctrl+4</shortcut>\n"
		"    <inherited>true</inherited>\n"
		"    <state id=\"preference_bad\">\n"
		"      <name>%6</name>\n" // "Bad"
		"      <emblem>tag_preference_bad</emblem>\n"
		"      <textEquivalent string=\"(*  )\" />\n"
		"    </state>\n"
		"    <state id=\"preference_good\">\n"
		"      <name>%7</name>\n" // "Good"
		"      <emblem>tag_preference_good</emblem>\n"
		"      <textEquivalent string=\"(** )\" />\n"
		"    </state>\n"
		"    <state id=\"preference_excelent\">\n"
		"      <name>%8</name>\n" // "Excellent"
		"      <emblem>tag_preference_excelent</emblem>\n" // "excelent": typo error, but we should keep compatibility with old versions.
		"      <textEquivalent string=\"(***)\" />\n"
		"    </state>\n"
		"  </tag>\n"
		"\n"
		"  <tag>\n"
		"    <name>%9</name>\n" // "Highlight"
		"    <shortcut>Ctrl+5</shortcut>\n"
		"    <state id=\"highlight\">\n"
		"      <backgroundColor>#ffffcc</backgroundColor>\n"
		"      <textEquivalent string=\"=>\" />\n"
		"    </state>\n"
		"  </tag>\n"
		"\n")
			.arg( i18n("Priority"),  i18n("Low"),            i18n("Medium")      )  // %1 %2 %3
			.arg( i18n("High"),      i18n("Preference"),     i18n("Bad")         )  // %4 %5 %6
			.arg( i18n("Good"),      i18n("Excellent"),      i18n("Highlight")   )  // %7 %8 %9
	+ QString(
		"  <tag>\n"
		"    <name>%1</name>\n" // "Important"
		"    <shortcut>Ctrl+6</shortcut>\n"
		"    <state id=\"important\">\n"
		"      <emblem>tag_important</emblem>\n"
		"      <backgroundColor>#ffcccc</backgroundColor>\n"
		"      <textEquivalent string=\"!!\" />\n"
		"    </state>\n"
		"  </tag>\n"
		"\n"
		"  <tag>\n"
		"    <name>%2</name>\n" // "Very Important"
		"    <shortcut>Ctrl+7</shortcut>\n"
		"    <state id=\"very_important\">\n"
		"      <emblem>tag_important</emblem>\n"
		"      <text color=\"#ffffff\" />\n"
		"      <backgroundColor>#ff0000</backgroundColor>\n"
		"      <textEquivalent string=\"/!\\\" />\n"
		"    </state>\n"
		"  </tag>\n"
		"\n"
		"  <tag>\n"
		"    <name>%3</name>\n" // "Information"
		"    <shortcut>Ctrl+8</shortcut>\n"
		"    <state id=\"information\">\n"
		"      <emblem>messagebox_info</emblem>\n"
		"      <textEquivalent string=\"(i)\" />\n"
		"    </state>\n"
		"  </tag>\n"
		"\n"
		"  <tag>\n"
		"    <name>%4</name>\n" // "Idea"
		"    <shortcut>Ctrl+9</shortcut>\n"
		"    <state id=\"idea\">\n"
		"      <emblem>ktip</emblem>\n"
		"      <textEquivalent string=\"%5\" />\n" // I.
		"    </state>\n"
		"  </tag>""\n"
		"\n"
		"  <tag>\n"
		"    <name>%6</name>\n" // "Title"
		"    <shortcut>Ctrl+0</shortcut>\n"
		"    <state id=\"title\">\n"
		"      <text bold=\"true\" />\n"
		"      <textEquivalent string=\"##\" />\n"
		"    </state>\n"
		"  </tag>\n"
		"\n"
		"  <tag>\n"
		"    <name>%7</name>\n" // "Code"
		"    <state id=\"code\">\n"
		"      <font name=\"monospace\" />\n"
		"      <textEquivalent string=\"|\" onAllTextLines=\"true\" />\n"
		"    </state>\n"
		"  </tag>\n"
		"\n"
		"  <tag>\n"
		"    <state id=\"work\">\n"
		"      <name>%8</name>\n" // "Work"
		"      <text color=\"#ff8000\" />\n"
		"      <textEquivalent string=\"%9\" />\n" // W.
		"    </state>\n"
		"  </tag>""\n"
		"\n")
			.arg( i18n("Important"), i18n("Very Important"),              i18n("Information")                 ) // %1 %2 %3
			.arg( i18n("Idea"),      i18nc("The initial of 'Idea'", "I."), i18n("Title")                       ) // %4 %5 %6
			.arg( i18n("Code"),      i18n("Work"),                        i18nc("The initial of 'Work'", "W.") ) // %7 %8 %9
	+ QString(
		"  <tag>\n"
		"    <state id=\"personal\">\n"
		"      <name>%1</name>\n" // "Personal"
		"      <text color=\"#008000\" />\n"
		"      <textEquivalent string=\"%2\" />\n" // P.
		"    </state>\n"
		"  </tag>\n"
		"\n"
		"  <tag>\n"
		"    <state id=\"funny\">\n"
		"      <name>%3</name>\n" // "Funny"
		"      <emblem>tag_fun</emblem>\n"
		"    </state>\n"
		"  </tag>\n"
		"</basketTags>\n"
		"")
			.arg( i18n("Personal"), i18nc("The initial of 'Personal'", "P."), i18n("Funny") ); // %1 %2 %3

	// Write to Disk:
	QFile file(fullPath);
	if (file.open(QIODevice::WriteOnly)) {
		QTextStream stream(&file);
		stream.setEncoding(QTextStream::UnicodeUTF8);
		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
		stream << xml;
		file.close();
	} else
		DEBUG_WIN << "<font color=red>FAILED to create the tags file</font>!";
}

#include <kapplication.h>
#include <qrect.h>
#include <qstyle.h>
#include <qcheckbox.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qradiobutton.h>
#include <kiconeffect.h>


// StateAction
StateAction::StateAction(State *state, const KShortcut &shortcut, QWidget* parent, bool withTagName) 
    : KToggleAction(parent)
    , m_state(state)
{
    setText(m_state->name());

    if (withTagName && m_state->parentTag())
	setText(m_state->parentTag()->name());

    setIcon(KIconLoader::global()->loadIcon(m_state->emblem(),
					    KIconLoader::Small,
					    16,
					    KIconLoader::DefaultState,
					    QStringList(),
					    /*path_store=*/0L,
					    /*canReturnNull=*/true
					    )
	    );

    setShortcut(shortcut);
}

StateAction::~StateAction()
{
    // pass
}

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

#include <qtooltip.h>
#include <qlabel.h>
#include <qlineedit.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QKeyEvent>
#include <Q3GridLayout>
#include <QPixmap>
#include <QMouseEvent>
#include <QEvent>
#include <Q3VBoxLayout>
#include <kfontcombo.h>
#include <qlayout.h>
#include <kkeybutton.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <qcheckbox.h>
#include <kpushbutton.h>
#include <q3groupbox.h>
#include <q3header.h>
#include <q3valuelist.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kseparator.h>
#include <kstringhandler.h>
#include <qpainter.h>
#include <qaction.h>
#include <kmessagebox.h>
#include <qtimer.h>

#include <iostream>

#include "kicondialog.h"
#include "tag.h"
#include "tagsedit.h"
#include "kcolorcombo2.h"
#include "variouswidgets.h"
#include "global.h"
#include "bnpview.h"

/** class StateCopy: */

StateCopy::StateCopy(State *old/* = 0*/)
{
	oldState = old;
	newState = new State();
	if (oldState)
		oldState->copyTo(newState);
}

StateCopy::~StateCopy()
{
}

void StateCopy::copyBack()
{
}

/** class TagCopy: */

TagCopy::TagCopy(Tag *old/* = 0*/)
{
	oldTag = old;
	newTag = new Tag();
	if (oldTag)
		oldTag->copyTo(newTag);

	if (old)
		for (State::List::iterator it = old->states().begin(); it != old->states().end(); ++it)
			stateCopies.append(new StateCopy(*it));
	else
		stateCopies.append(new StateCopy());
}

TagCopy::~TagCopy()
{
}

void TagCopy::copyBack()
{
}

bool TagCopy::isMultiState()
{
	return (stateCopies.count() > 1);
}

/** class TagListViewItem: */

TagListViewItem::TagListViewItem(Q3ListView *parent, TagCopy *tagCopy)
 : Q3ListViewItem(parent), m_tagCopy(tagCopy), m_stateCopy(0)
{
	setText(0, tagCopy->newTag->name());
}

TagListViewItem::TagListViewItem(Q3ListViewItem *parent, TagCopy *tagCopy)
 : Q3ListViewItem(parent), m_tagCopy(tagCopy), m_stateCopy(0)
{
	setText(0, tagCopy->newTag->name());
}

TagListViewItem::TagListViewItem(Q3ListView *parent, Q3ListViewItem *after, TagCopy *tagCopy)
 : Q3ListViewItem(parent, after), m_tagCopy(tagCopy), m_stateCopy(0)
{
	setText(0, tagCopy->newTag->name());
}

TagListViewItem::TagListViewItem(Q3ListViewItem *parent, Q3ListViewItem *after, TagCopy *tagCopy)
 : Q3ListViewItem(parent, after), m_tagCopy(tagCopy), m_stateCopy(0)
{
	setText(0, tagCopy->newTag->name());
}

/* */

TagListViewItem::TagListViewItem(Q3ListView *parent, StateCopy *stateCopy)
 : Q3ListViewItem(parent), m_tagCopy(0), m_stateCopy(stateCopy)
{
	setText(0, stateCopy->newState->name());
}

TagListViewItem::TagListViewItem(Q3ListViewItem *parent, StateCopy *stateCopy)
 : Q3ListViewItem(parent), m_tagCopy(0), m_stateCopy(stateCopy)
{
	setText(0, stateCopy->newState->name());
}

TagListViewItem::TagListViewItem(Q3ListView *parent, Q3ListViewItem *after, StateCopy *stateCopy)
 : Q3ListViewItem(parent, after), m_tagCopy(0), m_stateCopy(stateCopy)
{
	setText(0, stateCopy->newState->name());
}

TagListViewItem::TagListViewItem(Q3ListViewItem *parent, Q3ListViewItem *after, StateCopy *stateCopy)
 : Q3ListViewItem(parent, after), m_tagCopy(0), m_stateCopy(stateCopy)
{
	setText(0, stateCopy->newState->name());
}

/* */

TagListViewItem::~TagListViewItem()
{
}

TagListViewItem* TagListViewItem::lastChild()
{
	TagListViewItem *child = (TagListViewItem*)firstChild();
	while (child) {
		if (child->nextSibling())
			child = (TagListViewItem*)(child->nextSibling());
		else
			return child;
	}
	return 0;
}

bool TagListViewItem::isEmblemObligatory()
{
	return m_stateCopy != 0; // It's a state of a multi-state
}

TagListViewItem* TagListViewItem::prevSibling()
{
	TagListViewItem *item = this;
	while (item) {
		if (item->nextSibling() == this)
			return item;
		item = (TagListViewItem*)(item->itemAbove());
	}
	return 0;
}

TagListViewItem* TagListViewItem::parent() const
{
	return (TagListViewItem*) Q3ListViewItem::parent();
}

// TODO: TagListViewItem::
int TAG_ICON_SIZE = 16;
int TAG_MARGIN = 1;

int TagListViewItem::width(const QFontMetrics &/* fontMetrics */, const Q3ListView */*listView*/, int /* column */) const
{
	return listView()->visibleWidth();
}

void TagListViewItem::setup()
{
	QString text = (m_tagCopy ? m_tagCopy->newTag->name() : m_stateCopy->newState->name());
	State *state = (m_tagCopy ? m_tagCopy->stateCopies[0]->newState : m_stateCopy->newState);

	if (m_tagCopy && !m_tagCopy->newTag->shortcut().isNull())
		text = i18nc("Tag name (shortcut)", "%1 (%2)").arg(text, m_tagCopy->newTag->shortcut().toString());

	QFont font = state->font(listView()->font());

	QRect textRect = QFontMetrics(font).boundingRect(0, 0, /*width=*/1, 500000, Qt::AlignLeft | Qt::AlignTop, text);

	widthChanged();
	int height = TAG_MARGIN + qMax(TAG_ICON_SIZE, textRect.height()) + TAG_MARGIN;
	setHeight(height);

	repaint();
}

void TagListViewItem::paintCell(QPainter *painter, const QColorGroup &/*colorGroup*/, int /*column*/, int width, int /*align*/)
{
	bool withIcon = m_stateCopy || (m_tagCopy && !m_tagCopy->isMultiState());
	QString text = (m_tagCopy ? m_tagCopy->newTag->name() : m_stateCopy->newState->name());
	State *state = (m_tagCopy ? m_tagCopy->stateCopies[0]->newState : m_stateCopy->newState);

	if (m_tagCopy && !m_tagCopy->newTag->shortcut().isNull())
		text = i18nc("Tag name (shortcut)", "%1 (%2)").arg(text, m_tagCopy->newTag->shortcut().toString());

	QFont font = (withIcon ? state->font(listView()->font()) : listView()->font());

	QFontMetrics fontMetrics(font);
	QRect textRect = fontMetrics.boundingRect(0, 0, /*width=*/1, 500000, Qt::AlignLeft | Qt::AlignTop, text);

	QPixmap emblem = QPixmap();
    if (withIcon)
        empblem = KIconLoader::global()->loadIcon(
            state->emblem(), KIconLoader::NoGroup, 16);

	QColor backgroundColor = (isSelected() ? palette().color(QPalette::Highlight)
	                                       : (withIcon && state->backgroundColor().isValid() ? state->backgroundColor()
	                                                                                         : listView()->paletteBackgroundColor()));
	QColor textColor = (isSelected() ? palette().color(QPalette::HighlightedText)
	                                 : (withIcon && state->textColor().isValid() ? state->textColor()
	                                                                             : listView()->paletteForegroundColor()));

	// Bufferize the drawing of items (otherwize, resizing the splitter make the tree act like a Christmas Tree ;-D ):
	QPixmap theBuffer(width, height());
	theBuffer.fill(backgroundColor);
	QPainter thePainter(&theBuffer);

	if (withIcon)
		thePainter.drawPixmap(TAG_MARGIN, (height() - emblem.height()) / 2, emblem);

	int xText = TAG_MARGIN + (withIcon ? TAG_ICON_SIZE + TAG_MARGIN : 0);
	thePainter.setPen(textColor);
	thePainter.setFont(font);
	int textWidth = width - xText;
	if (thePainter.fontMetrics().width(text) > textWidth)
		text = KStringHandler::rPixelSqueeze(text, fontMetrics, textWidth);
	thePainter.drawText(xText, 0, textWidth, height(), Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic, text);

	// Apply the buffer:
	thePainter.end();
	painter->drawPixmap(0, 0, theBuffer);
}

/** class TagListView: */

TagListView::TagListView(QWidget *parent, const char *name, Qt::WFlags flags)
 : Q3ListView(parent, name, flags)
{
}

TagListView::~TagListView()
{
}

void TagListView::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Delete)
		emit deletePressed();
	else if (event->key() != Qt::Key_Left || (selectedItem() && selectedItem()->parent()))
		// Do not allow to open/close first-level items
		Q3ListView::keyPressEvent(event);
}

void TagListView::contentsMouseDoubleClickEvent(QMouseEvent *event)
{
	// Ignore this event! Do not open/close first-level items!

	// But trigger edit (change focus to name) when double-click an item:
	if (itemAt(contentsToViewport(event->pos())) != 0)
		emit doubleClickedItem();
}

void TagListView::contentsMousePressEvent(QMouseEvent *event)
{
	// When clicking on an empty space, QListView would unselect the current item! We forbid that!
	if (itemAt(contentsToViewport(event->pos())) != 0)
		Q3ListView::contentsMousePressEvent(event);
}

void TagListView::contentsMouseReleaseEvent(QMouseEvent *event)
{
	// When clicking on an empty space, QListView would unselect the current item! We forbid that!
	if (itemAt(contentsToViewport(event->pos())) != 0)
		Q3ListView::contentsMouseReleaseEvent(event);
}

TagListViewItem* TagListView::currentItem() const
{
	return (TagListViewItem*) Q3ListView::currentItem();
}

TagListViewItem* TagListView::firstChild() const
{
	return (TagListViewItem*) Q3ListView::firstChild();
}

TagListViewItem* TagListView::lastItem() const
{
	return (TagListViewItem*) Q3ListView::lastItem();
}

/** class TagsEditDialog: */

TagsEditDialog::TagsEditDialog(QWidget *parent, State *stateToEdit, bool addNewTag)
 : KDialog(parent)
 ,  m_loading(false)
{
	// KDialog options
	setCaption(i18n("Customize Tags"));
	setButtons(Ok | Cancel);
	setDefaultButton(Ok);
	setObjectName("CustomizeTags");
	setModal(true);
	showButtonSeparator(true);
	connect(this, SIGNAL(okClicked()), SLOT(slotOk()));
	connect(this, SIGNAL(cancelClicked()), SLOT(slotCancel()));

	setMainWidget(new QWidget(this));

	Q3HBoxLayout *layout = new Q3HBoxLayout(mainWidget(), /*margin=*/0, spacingHint());

	/* Left part: */

	QPushButton *newTag     = new QPushButton(i18n("Ne&w Tag"),   mainWidget());
	QPushButton *newState   = new QPushButton(i18n("New St&ate"), mainWidget());

	connect( newTag,   SIGNAL(clicked()), this, SLOT(newTag())   );
	connect( newState, SIGNAL(clicked()), this, SLOT(newState()) );

	m_tags = new TagListView(mainWidget());
	m_tags->header()->hide();
	m_tags->setRootIsDecorated(false);
	m_tags->addColumn("");
	m_tags->setSorting(-1); // Sort column -1, so disabled sorting
	m_tags->setResizeMode(Q3ListView::LastColumn);

	m_moveUp    = new KPushButton( KGuiItem("", "1uparrow"),   mainWidget() );
	m_moveDown  = new KPushButton( KGuiItem("", "1downarrow"), mainWidget() );
	m_deleteTag = new KPushButton( KGuiItem("", "editdelete"), mainWidget() );

	QToolTip::add( m_moveUp,    i18n("Move Up (Ctrl+Shift+Up)")     );
	QToolTip::add( m_moveDown,  i18n("Move Down (Ctrl+Shift+Down)") );
	QToolTip::add( m_deleteTag, i18n("Delete")                      );

	connect( m_moveUp,    SIGNAL(clicked()), this, SLOT(moveUp())    );
	connect( m_moveDown,  SIGNAL(clicked()), this, SLOT(moveDown())  );
	connect( m_deleteTag, SIGNAL(clicked()), this, SLOT(deleteTag()) );

	Q3HBoxLayout *topLeftLayout = new Q3HBoxLayout(0, /*margin=*/0, spacingHint());
	topLeftLayout->addWidget(m_moveUp);
	topLeftLayout->addWidget(m_moveDown);
	topLeftLayout->addWidget(m_deleteTag);

	Q3VBoxLayout *leftLayout = new Q3VBoxLayout(0, /*margin=*/0, spacingHint());
	leftLayout->addWidget(newTag);
	leftLayout->addWidget(newState);
	leftLayout->addWidget(m_tags);
	leftLayout->addLayout(topLeftLayout);

	layout->addLayout(leftLayout);

	/* Right part: */

	QWidget *rightWidget = new QWidget(mainWidget());

	m_tagBox             = new Q3GroupBox(1, Qt::Horizontal, i18n("Tag"), rightWidget);
	QWidget   *tagWidget = new QWidget(m_tagBox);

	m_tagName = new QLineEdit(tagWidget);
	QLabel *tagNameLabel = new QLabel(m_tagName, i18n("&Name:"), tagWidget);

	m_shortcut = new KKeyButton(tagWidget);
	m_removeShortcut = new QPushButton(i18nc("Remove tag shortcut", "&Remove"), tagWidget);
	QLabel *shortcutLabel = new QLabel(m_shortcut, i18n("S&hortcut:"), tagWidget);
	connect( m_shortcut,       SIGNAL(capturedShortcut(const KShortcut&)), this, SLOT(capturedShortcut(const KShortcut&)) );
	connect( m_removeShortcut, SIGNAL(clicked()),                          this, SLOT(removeShortcut())                   );

	m_inherit = new QCheckBox(i18n("&Inherited by new sibling notes"), tagWidget);

	Q3GridLayout *tagGrid = new Q3GridLayout(tagWidget, /*rows=*/3, /*cols=*/4, /*border=*/0, /*spacing=*/spacingHint());
	tagGrid->addWidget(tagNameLabel,     0, 0);
	tagGrid->addMultiCellWidget(m_tagName, /*fromRow=*/0, /*toRow=*/0, /*fromCol=*/1, /*toCol=*/3);
	tagGrid->addWidget(shortcutLabel,    1, 0);
	tagGrid->addWidget(m_shortcut,       1, 1);
	tagGrid->addWidget(m_removeShortcut, 1, 2);
	tagGrid->addMultiCellWidget(m_inherit, /*fromRow=*/2, /*toRow=*/2, /*fromCol=*/0, /*toCol=*/3);
	tagGrid->setColStretch(/*col=*/3, /*stretch=*/255);

	m_stateBox           = new Q3GroupBox(1, Qt::Horizontal, i18n("State"), rightWidget);
	QWidget *stateWidget = new QWidget(m_stateBox);

	m_stateName = new QLineEdit(stateWidget);
	m_stateNameLabel = new QLabel(m_stateName, i18n("Na&me:"), stateWidget);

	QWidget *emblemWidget = new QWidget(stateWidget);
	m_emblem = new KIconButton(emblemWidget);
	m_emblem->setIconType(KIconLoader::NoGroup, KIcon::Action);
	m_emblem->setIconSize(16);
	m_emblem->setIcon("editdelete");
	m_removeEmblem = new QPushButton(i18nc("Remove tag emblem", "Remo&ve"), emblemWidget);
	QLabel *emblemLabel = new QLabel(m_emblem, i18n("&Emblem:"), stateWidget);
	connect( m_removeEmblem, SIGNAL(clicked()), this, SLOT(removeEmblem()) ); // m_emblem.resetIcon() is not a slot!

	// Make the icon button and the remove button the same height:
	int height = qMax(m_emblem->sizeHint().width(), m_emblem->sizeHint().height());
	height = qMax(height, m_removeEmblem->sizeHint().height());
	m_emblem->setFixedSize(height, height); // Make it square
	m_removeEmblem->setFixedHeight(height);
	m_emblem->resetIcon();

	Q3HBoxLayout *emblemLayout = new Q3HBoxLayout(emblemWidget, /*margin=*/0, spacingHint());
	emblemLayout->addWidget(m_emblem);
	emblemLayout->addWidget(m_removeEmblem);
	emblemLayout->addStretch();

	m_backgroundColor = new KColorCombo2(QColor(), palette().color(QPalette::Base), stateWidget);
	QLabel *backgroundColorLabel = new QLabel(m_backgroundColor, i18n("&Background:"), stateWidget);

	Q3HBoxLayout *backgroundColorLayout = new Q3HBoxLayout(0, /*margin=*/0, spacingHint());
	backgroundColorLayout->addWidget(m_backgroundColor);
	backgroundColorLayout->addStretch();

	QIcon boldIconSet = KIconLoader::global()->loadIconSet("text_bold", KIcon::Small);
	m_bold = new QPushButton(boldIconSet, "", stateWidget);
	m_bold->setToggleButton(true);
	int size = qMax(m_bold->sizeHint().width(), m_bold->sizeHint().height());
	m_bold->setFixedSize(size, size); // Make it square!
	QToolTip::add(m_bold, i18n("Bold"));

	QIcon underlineIconSet = KIconLoader::global()->loadIconSet("text_under", KIcon::Small);
	m_underline = new QPushButton(underlineIconSet, "", stateWidget);
	m_underline->setToggleButton(true);
	m_underline->setFixedSize(size, size); // Make it square!
	QToolTip::add(m_underline, i18n("Underline"));

	QIcon italicIconSet = KIconLoader::global()->loadIconSet("text_italic", KIcon::Small);
	m_italic = new QPushButton(italicIconSet, "", stateWidget);
	m_italic->setToggleButton(true);
	m_italic->setFixedSize(size, size); // Make it square!
	QToolTip::add(m_italic, i18n("Italic"));

	QIcon strikeIconSet = KIconLoader::global()->loadIconSet("text_strike", KIcon::Small);
	m_strike = new QPushButton(strikeIconSet, "", stateWidget);
	m_strike->setToggleButton(true);
	m_strike->setFixedSize(size, size); // Make it square!
	QToolTip::add(m_strike, i18n("Strike Through"));

	QLabel *textLabel = new QLabel(m_bold, i18n("&Text:"), stateWidget);

	Q3HBoxLayout *textLayout = new Q3HBoxLayout(0, /*margin=*/0, spacingHint());
	textLayout->addWidget(m_bold);
	textLayout->addWidget(m_underline);
	textLayout->addWidget(m_italic);
	textLayout->addWidget(m_strike);
	textLayout->addStretch();

	m_textColor = new KColorCombo2(QColor(), palette().color(QPalette::Text), stateWidget);
	QLabel *textColorLabel = new QLabel(m_textColor, i18n("Co&lor:"), stateWidget);

	m_font = new KFontCombo(stateWidget);
	m_font->insertItem(i18n("(Default)"), 0);
	QLabel *fontLabel = new QLabel(m_font, i18n("&Font:"), stateWidget);

	m_fontSize = new FontSizeCombo(/*rw=*/true, /*withDefault=*/true, stateWidget);
	QLabel *fontSizeLabel = new QLabel(m_fontSize, i18n("&Size:"), stateWidget);

	m_textEquivalent = new QLineEdit(stateWidget);
	QLabel *textEquivalentLabel = new QLabel(m_textEquivalent, i18n("Te&xt equivalent:"), stateWidget);
	QFont font = m_textEquivalent->font();
	font.setFamily("monospace");
	m_textEquivalent->setFont(font);

	QPixmap textEquivalentPixmap(KGlobal::dirs()->findResource("data", "basket/images/tag_export_help.png"));
	Q3MimeSourceFactory::defaultFactory()->setPixmap("__resource_help_tag_export.png", textEquivalentPixmap);
	HelpLabel *textEquivalentHelp = new HelpLabel(
		i18n("When does this apply?"),
		"<p>" + i18n("It does apply when you copy and paste, or drag and drop notes to a text editor.") + "</p>" +
		"<p>" + i18n("If filled, this property lets you paste this tag or this state as textual equivalent.") + "<br>" +
		i18n("For instance, a list of notes with the <b>To Do</b> and <b>Done</b> tags are exported as lines preceded by <b>[ ]</b> or <b>[x]</b>, "
		     "representing an empty checkbox and a checked box.") + "</p>" +
		"<p align='center'><img src=\"__resource_help_tag_export.png\"></p>",
		stateWidget);
	Q3HBoxLayout *textEquivalentHelpLayout = new Q3HBoxLayout((QWidget*)0, /*border=*/0, spacingHint());
	textEquivalentHelpLayout->addWidget(textEquivalentHelp);
	textEquivalentHelpLayout->addStretch(255);

	m_onEveryLines = new QCheckBox(i18n("On ever&y line"), stateWidget);

	QPixmap onEveryLinesPixmap(KGlobal::dirs()->findResource("data", "basket/images/tag_export_on_every_lines_help.png"));
	Q3MimeSourceFactory::defaultFactory()->setPixmap("__resource_help_tag_export_on_every_lines.png", onEveryLinesPixmap);
	HelpLabel *onEveryLinesHelp = new HelpLabel(
		i18n("What does it mean?"),
		"<p>" + i18n("When a note has several lines, you can choose to export the tag or the state on the first line or on every line of the note.") + "</p>" +
		"<p align='center'><img src=\"__resource_help_tag_export_on_every_lines.png\"></p>" +
		"<p>" + i18n("In the example above, the tag of the top note is only exported on the first line, while the tag of the bottom note is exported on every line of the note."),
		stateWidget);
	Q3HBoxLayout *onEveryLinesHelpLayout = new Q3HBoxLayout((QWidget*)0, /*border=*/0, spacingHint());
	onEveryLinesHelpLayout->addWidget(onEveryLinesHelp);
	onEveryLinesHelpLayout->addStretch(255);

	Q3GridLayout *textEquivalentGrid = new Q3GridLayout(0, /*rows=*/2, /*cols=*/4, /*border=*/0, /*spacing=*/spacingHint());
	textEquivalentGrid->addWidget(textEquivalentLabel,      0, 0);
	textEquivalentGrid->addWidget(m_textEquivalent,         0, 1);
	textEquivalentGrid->addLayout(textEquivalentHelpLayout, 0, 2);
	textEquivalentGrid->addWidget(m_onEveryLines,           1, 1);
	textEquivalentGrid->addLayout(onEveryLinesHelpLayout,   1, 2);
	textEquivalentGrid->setColStretch(/*col=*/3, /*stretch=*/255);

	KSeparator *separator = new KSeparator(Qt::Horizontal, stateWidget);

	Q3GridLayout *stateGrid = new Q3GridLayout(stateWidget, /*rows=*/6, /*cols=*/7, /*border=*/0, /*spacing=*/spacingHint());
	stateGrid->addWidget(m_stateNameLabel,     0, 0);
	stateGrid->addMultiCellWidget(m_stateName,            /*fromRow=*/0, /*toRow=*/0, /*fromCol=*/1, /*toCol=*/6);
	stateGrid->addWidget(emblemLabel,          1, 0);
	stateGrid->addMultiCellWidget(emblemWidget,           /*fromRow=*/1, /*toRow=*/1, /*fromCol=*/1, /*toCol=*/6);
	stateGrid->addWidget(backgroundColorLabel, 1, 5);
	stateGrid->addMultiCellLayout(backgroundColorLayout,  /*fromRow=*/1, /*toRow=*/1, /*fromCol=*/6, /*toCol=*/6);
	stateGrid->addWidget(textLabel,            2, 0);
	stateGrid->addMultiCellLayout(textLayout,             /*fromRow=*/2, /*toRow=*/2, /*fromCol=*/1, /*toCol=*/4);
	stateGrid->addWidget(textColorLabel,       2, 5);
	stateGrid->addWidget(m_textColor,          2, 6);
	stateGrid->addWidget(fontLabel,            3, 0);
	stateGrid->addMultiCellWidget(m_font,                 /*fromRow=*/3, /*toRow=*/3, /*fromCol=*/1, /*toCol=*/4);
	stateGrid->addWidget(fontSizeLabel,        3, 5);
	stateGrid->addWidget(m_fontSize,           3, 6);
	stateGrid->addMultiCellWidget(separator,              /*fromRow=*/4, /*toRow=*/4, /*fromCol=*/0, /*toCol=*/6);
	stateGrid->addMultiCellLayout(textEquivalentGrid,     /*fromRow=*/5, /*toRow=*/5, /*fromCol=*/0, /*toCol=*/6);

	Q3VBoxLayout *rightLayout = new Q3VBoxLayout(rightWidget, /*margin=*/0, spacingHint());
	rightLayout->addWidget(m_tagBox);
	rightLayout->addWidget(m_stateBox);
	rightLayout->addStretch();

	layout->addWidget(rightWidget);
	rightWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);

	// Equalize the width of the first column of the two grids:
	int maxWidth = tagNameLabel->sizeHint().width();
	maxWidth = qMax(maxWidth, shortcutLabel->sizeHint().width());
	maxWidth = qMax(maxWidth, m_stateNameLabel->sizeHint().width());
	maxWidth = qMax(maxWidth, emblemLabel->sizeHint().width());
	maxWidth = qMax(maxWidth, textLabel->sizeHint().width());
	maxWidth = qMax(maxWidth, fontLabel->sizeHint().width());
	maxWidth = qMax(maxWidth, backgroundColorLabel->sizeHint().width());
	maxWidth = qMax(maxWidth, textEquivalentLabel->sizeHint().width());

	tagNameLabel->setFixedWidth(maxWidth);
	m_stateNameLabel->setFixedWidth(maxWidth);
	textEquivalentLabel->setFixedWidth(maxWidth);

	// Load Tags:
	for (Tag::List::iterator tagIt = Tag::all.begin(); tagIt != Tag::all.end(); ++tagIt)
		m_tagCopies.append(new TagCopy(*tagIt));

	TagListViewItem *lastInsertedItem = 0;
	TagListViewItem *lastInsertedSubItem;
	TagListViewItem *item;
	TagListViewItem *subItem;
	for (TagCopy::List::iterator tagCopyIt = m_tagCopies.begin(); tagCopyIt != m_tagCopies.end(); ++tagCopyIt) {
		// New List View Item:
		if (lastInsertedItem)
			item = new TagListViewItem(m_tags, lastInsertedItem, *tagCopyIt);
		else
			item = new TagListViewItem(m_tags, *tagCopyIt);
		item->setOpen(true);
		lastInsertedItem = item;
		// Load
		if ((*tagCopyIt)->isMultiState()) {
			lastInsertedSubItem = 0;
			StateCopy::List stateCopies = item->tagCopy()->stateCopies;
			for (StateCopy::List::iterator stateCopyIt = stateCopies.begin(); stateCopyIt != stateCopies.end(); ++stateCopyIt) {
				if (lastInsertedSubItem)
					subItem = new TagListViewItem(item, lastInsertedSubItem, *stateCopyIt);
				else
					subItem = new TagListViewItem(item, *stateCopyIt);
				lastInsertedSubItem = subItem;
			}
		}
	}

	// Connect Signals:
	connect( m_tagName,         SIGNAL(textChanged(const QString&)),        this, SLOT(modified()) );
	connect( m_shortcut,        SIGNAL(capturedShortcut(const KShortcut&)), this, SLOT(modified()) );
	connect( m_inherit,         SIGNAL(stateChanged(int)),                  this, SLOT(modified()) );
	connect( m_stateName,       SIGNAL(textChanged(const QString&)),        this, SLOT(modified()) );
	connect( m_emblem,          SIGNAL(iconChanged(QString)),               this, SLOT(modified()) );
	connect( m_backgroundColor, SIGNAL(changed(const QColor&)),             this, SLOT(modified()) );
	connect( m_bold,            SIGNAL(toggled(bool)),                      this, SLOT(modified()) );
	connect( m_underline,       SIGNAL(toggled(bool)),                      this, SLOT(modified()) );
	connect( m_italic,          SIGNAL(toggled(bool)),                      this, SLOT(modified()) );
	connect( m_strike,          SIGNAL(toggled(bool)),                      this, SLOT(modified()) );
	connect( m_textColor,       SIGNAL(changed(const QColor&)),             this, SLOT(modified()) );
	connect( m_font,            SIGNAL(textChanged(const QString&)),        this, SLOT(modified()) );
	connect( m_fontSize,        SIGNAL(textChanged(const QString&)),        this, SLOT(modified()) );
	connect( m_textEquivalent,  SIGNAL(textChanged(const QString&)),        this, SLOT(modified()) );
	connect( m_onEveryLines,    SIGNAL(stateChanged(int)),                  this, SLOT(modified()) );

	connect( m_tags,            SIGNAL(currentChanged(Q3ListViewItem*)),     this, SLOT(currentItemChanged(Q3ListViewItem*)) );
	connect( m_tags,            SIGNAL(deletePressed()),                    this, SLOT(deleteTag())                        );
	connect( m_tags,            SIGNAL(doubleClickedItem()),                this, SLOT(renameIt())                         );

	Q3ListViewItem *firstItem = m_tags->firstChild();
	if (stateToEdit != 0) {
		TagListViewItem *item = itemForState(stateToEdit);
		if (item)
			firstItem = item;
	}
	// Select the first tag unless the first tag is a multi-state tag.
	// In this case, select the first state, as it let customize the state AND the associated tag.
	if (firstItem) {
		if (firstItem->firstChild())
			firstItem = firstItem->firstChild();
		firstItem->setSelected(true);
		m_tags->setCurrentItem(firstItem);
		currentItemChanged(firstItem);
		if (stateToEdit == 0)
			m_tags->ensureVisible(0, 0);
		m_tags->setFocus();
	} else {
		m_moveUp->setEnabled(false);
		m_moveDown->setEnabled(false);
		m_deleteTag->setEnabled(false);
		m_tagBox->setEnabled(false);
		m_stateBox->setEnabled(false);
	}
	// TODO: Disabled both boxes if no tag!!!

	// Some keyboard shortcuts:       // Ctrl+arrows instead of Alt+arrows (same as Go menu in the main window) because Alt+Down is for combo boxes
	QAction *selectAbove = new QAction(this);
	selectAbove->setAccel(QString("Ctrl+Up"));
	connect( selectAbove, SIGNAL(activated()), this, SLOT(selectUp()) );

	QAction *selectBelow = new QAction(this);
	selectBelow->setAccel(QString("Ctrl+Down"));
	connect( selectBelow, SIGNAL(activated()), this, SLOT(selectDown()) );

	QAction *selectLeft = new QAction(this);
	selectLeft->setAccel(QString("Ctrl+Left"));
	connect( selectLeft, SIGNAL(activated()), this, SLOT(selectLeft()) );

	QAction *selectRight = new QAction(this);
	selectRight->setAccel(QString("Ctrl+Right"));
	connect( selectRight, SIGNAL(activated()), this, SLOT(selectRight()) );

	QAction *moveAbove = new QAction(this);
	moveAbove->setAccel(QString("Ctrl+Shift+Up"));
	connect( moveAbove, SIGNAL(activated()), this, SLOT(moveUp()) );

	QAction *moveBelow = new QAction(this);
	moveBelow->setAccel(QString("Ctrl+Shift+Down"));
	connect( moveBelow, SIGNAL(activated()), this, SLOT(moveDown()) );

	QAction *rename = new QAction(this);
	rename->setAccel(QString("F2"));
	connect( rename, SIGNAL(activated()), this, SLOT(renameIt()) );

	m_tags->setMinimumSize(
		m_tags->sizeHint().width() * 2,
		m_tagBox->sizeHint().height() + m_stateBox->sizeHint().height()
	);

	if (addNewTag)
		QTimer::singleShot(0, this, SLOT(newTag()) );
	else
		// Once the window initial size is computed and the window show, allow the user to resize it down:
		QTimer::singleShot(0, this, SLOT(resetTreeSizeHint()) );
}

TagsEditDialog::~TagsEditDialog()
{
}

void TagsEditDialog::resetTreeSizeHint()
{
	m_tags->setMinimumSize(m_tags->sizeHint());
}

TagListViewItem* TagsEditDialog::itemForState(State *state)
{
	// Browse all tags:
	Q3ListViewItemIterator it(m_tags);
	while (it.current()) {
		Q3ListViewItem *item = it.current();

		// Return if we found the tag item:
		TagListViewItem *tagItem = (TagListViewItem*)item;
		if (tagItem->tagCopy() && tagItem->tagCopy()->oldTag && tagItem->tagCopy()->stateCopies[0]->oldState == state)
			return tagItem;

		// Browser all sub-states:
		Q3ListViewItemIterator it2(item);
		while (it2.current()) {
			Q3ListViewItem *subItem = it2.current();

			// Return if we found the state item:
			TagListViewItem *stateItem = (TagListViewItem*)subItem;
			if (stateItem->stateCopy() && stateItem->stateCopy()->oldState && stateItem->stateCopy()->oldState == state)
				return stateItem;

			++it2;
		}

		++it;
	}
	return 0;
}

void TagsEditDialog::newTag()
{
	// Add to the "model":
	TagCopy *newTagCopy = new TagCopy();
	newTagCopy->stateCopies[0]->newState->setId("tag_state_" + QString::number( Tag::getNextStateUid() )); //TODO: Check if it's really unique
	m_tagCopies.append(newTagCopy);
	m_addedStates.append(newTagCopy->stateCopies[0]->newState);

	// Add to the "view":
	TagListViewItem *item;
	if (m_tags->firstChild()) {
		// QListView::lastItem is the last item in the tree. If we the last item is a state item, the new tag gets appended to the begin of the list.
		TagListViewItem *last = m_tags->lastItem();
		if (last->parent())
			last = last->parent();
		item = new TagListViewItem(m_tags, last, newTagCopy);
	} else
		item = new TagListViewItem(m_tags, newTagCopy);

	m_deleteTag->setEnabled(true);
	m_tagBox->setEnabled(true);

	// Add to the "controler":
	m_tags->setCurrentItem(item);
	currentItemChanged(item);
	item->setSelected(true);
	m_tagName->setFocus();
}

void TagsEditDialog::newState()
{
	TagListViewItem *tagItem = m_tags->currentItem();
	if (tagItem->parent())
		tagItem = ((TagListViewItem*)(tagItem->parent()));
	tagItem->setOpen(true); // Show sub-states if we're adding them for the first time!

	State *firstState = tagItem->tagCopy()->stateCopies[0]->newState;

	// Add the first state to the "view". From now on, it's a multi-state tag:
	if (tagItem->firstChild() == 0) {
		firstState->setName( tagItem->tagCopy()->newTag->name() );
		// Force emblem to exists for multi-state tags:
		if (firstState->emblem().isEmpty())
			firstState->setEmblem("empty");
		new TagListViewItem(tagItem, tagItem->tagCopy()->stateCopies[0]);
	}

	// Add to the "model":
	StateCopy *newStateCopy = new StateCopy();
	firstState->copyTo(newStateCopy->newState);
	newStateCopy->newState->setId("tag_state_" + QString::number( Tag::getNextStateUid() )); //TODO: Check if it's really unique
	newStateCopy->newState->setName(""); // We copied it too but it's likely the name will not be the same
	tagItem->tagCopy()->stateCopies.append(newStateCopy);
	m_addedStates.append(newStateCopy->newState);

	// Add to the "view":
	TagListViewItem *item = new TagListViewItem(tagItem, tagItem->lastChild(), newStateCopy);

	// Add to the "controler":
	m_tags->setCurrentItem(item);
	currentItemChanged(item);
	m_stateName->setFocus();
}

void TagsEditDialog::moveUp()
{
	if (!m_moveUp->isEnabled()) // Trggered by keyboard shortcut
		return;

	TagListViewItem *tagItem     = m_tags->currentItem();
	TagListViewItem *prevTagItem = ((TagListViewItem*)( tagItem->prevSibling() ));

	// Move in the list view:
	prevTagItem->moveItem(tagItem);

	// Move in the value list:
	if (tagItem->tagCopy()) {
		int pos = m_tagCopies.findIndex(tagItem->tagCopy());
		m_tagCopies.remove(tagItem->tagCopy());
		int i = 0;
		for (TagCopy::List::iterator it = m_tagCopies.begin(); it != m_tagCopies.end(); ++it, ++i)
			if (i == pos - 1) {
				m_tagCopies.insert(it, tagItem->tagCopy());
				break;
			}
	} else {
		StateCopy::List &stateCopies = ((TagListViewItem*)( tagItem->parent() ))->tagCopy()->stateCopies;
		int pos = stateCopies.findIndex(tagItem->stateCopy());
		stateCopies.remove(tagItem->stateCopy());
		int i = 0;
		for (StateCopy::List::iterator it = stateCopies.begin(); it != stateCopies.end(); ++it, ++i)
			if (i == pos - 1) {
				stateCopies.insert(it, tagItem->stateCopy());
				break;
			}
	}

	ensureCurrentItemVisible();

	m_moveDown->setEnabled( tagItem->nextSibling() != 0 );
	m_moveUp->setEnabled(   tagItem->prevSibling() != 0 );
}

void TagsEditDialog::moveDown()
{
	if (!m_moveDown->isEnabled()) // Trggered by keyboard shortcut
		return;

	TagListViewItem *tagItem = m_tags->currentItem();

	// Move in the list view:
	tagItem->moveItem(tagItem->nextSibling());

	// Move in the value list:
	if (tagItem->tagCopy()) {
		uint pos = m_tagCopies.findIndex(tagItem->tagCopy());
		m_tagCopies.remove(tagItem->tagCopy());
		if (pos == m_tagCopies.count() - 1) // Insert at end: iterator does not go there
			m_tagCopies.append(tagItem->tagCopy());
		else {
			uint i = 0;
			for (TagCopy::List::iterator it = m_tagCopies.begin(); it != m_tagCopies.end(); ++it, ++i)
				if (i == pos + 1) {
					m_tagCopies.insert(it, tagItem->tagCopy());
					break;
				}
		}
	} else {
		StateCopy::List &stateCopies = ((TagListViewItem*)( tagItem->parent() ))->tagCopy()->stateCopies;
		uint pos = stateCopies.findIndex(tagItem->stateCopy());
		stateCopies.remove(tagItem->stateCopy());
		if (pos == stateCopies.count() - 1) // Insert at end: iterator does not go there
			stateCopies.append(tagItem->stateCopy());
		else {
			uint i = 0;
			for (StateCopy::List::iterator it = stateCopies.begin(); it != stateCopies.end(); ++it, ++i)
				if (i == pos + 1) {
					stateCopies.insert(it, tagItem->stateCopy());
					break;
				}
		}
	}

	ensureCurrentItemVisible();

	m_moveDown->setEnabled( tagItem->nextSibling() != 0 );
	m_moveUp->setEnabled(   tagItem->prevSibling() != 0 );
}

void TagsEditDialog::selectUp()
{
	QKeyEvent* keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, 0, 0);
	QApplication::postEvent(m_tags, keyEvent);
}

void TagsEditDialog::selectDown()
{
	QKeyEvent* keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, 0, 0);
	QApplication::postEvent(m_tags, keyEvent);
}

void TagsEditDialog::selectLeft()
{
	QKeyEvent* keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, 0, 0);
	QApplication::postEvent(m_tags, keyEvent);
}

void TagsEditDialog::selectRight()
{
	QKeyEvent* keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, 0, 0);
	QApplication::postEvent(m_tags, keyEvent);
}

void TagsEditDialog::deleteTag()
{
	if (!m_deleteTag->isEnabled())
		return;

	TagListViewItem *item = m_tags->currentItem();

	int result = KMessageBox::Continue;
	if (item->tagCopy() && item->tagCopy()->oldTag)
		result = KMessageBox::warningContinueCancel(
			this,
			i18n("Deleting the tag will remove it from every note it is currently assigned to."),
			i18n("Confirm Delete Tag"),
			KGuiItem(i18n("Delete Tag"), "editdelete")
		);
	else if (item->stateCopy() && item->stateCopy()->oldState)
		result = KMessageBox::warningContinueCancel(
			this,
			i18n("Deleting the state will remove the tag from every note the state is currently assigned to."),
			i18n("Confirm Delete State"),
			KGuiItem(i18n("Delete State"), "editdelete")
		);
	if (result != KMessageBox::Continue)
		return;

	if (item->tagCopy()) {
		StateCopy::List stateCopies = item->tagCopy()->stateCopies;
		for (StateCopy::List::iterator stateCopyIt = stateCopies.begin(); stateCopyIt != stateCopies.end(); ++stateCopyIt) {
			StateCopy *stateCopy = *stateCopyIt;
			if (stateCopy->oldState) {
				m_deletedStates.append(stateCopy->oldState);
				m_addedStates.remove(stateCopy->oldState);
			}
			m_addedStates.remove(stateCopy->newState);
		}
		m_tagCopies.remove(item->tagCopy());
		// Remove the new tag, to avoid keyboard-shortcut clashes:
		delete item->tagCopy()->newTag;
	} else {
		TagListViewItem *parentItem = item->parent();
		// Remove the state:
		parentItem->tagCopy()->stateCopies.remove(item->stateCopy());
		if (item->stateCopy()->oldState) {
			m_deletedStates.append(item->stateCopy()->oldState);
			m_addedStates.remove(item->stateCopy()->oldState);
		}
		m_addedStates.remove(item->stateCopy()->newState);
		delete item;
		item = 0;
		// Transform to single-state tag if needed:
		if (parentItem->childCount() == 1) {
			delete parentItem->firstChild();
			m_tags->setCurrentItem(parentItem);
		}
	}

	delete item;
	if (m_tags->currentItem())
		m_tags->currentItem()->setSelected(true);

	if (!m_tags->firstChild()) {
		m_deleteTag->setEnabled(false);
		m_tagBox->setEnabled(false);
		m_stateBox->setEnabled(false);
	}
}

void TagsEditDialog::renameIt()
{
	if (m_tags->currentItem()->tagCopy())
		m_tagName->setFocus();
	else
		m_stateName->setFocus();
}

void TagsEditDialog::capturedShortcut(const KShortcut &shortcut)
{
	// TODO: Validate it!
	m_shortcut->setShortcut(shortcut, /*bQtShortcut=*/true);
}

void TagsEditDialog::removeShortcut()
{
	m_shortcut->setShortcut(KShortcut(), /*bQtShortcut=*/true);
	modified();
}

void TagsEditDialog::removeEmblem()
{
	m_emblem->resetIcon();
	modified();
}

void TagsEditDialog::modified()
{
	if (m_loading)
		return;

	TagListViewItem *tagItem = m_tags->currentItem();
	if (tagItem == 0)
		return;

	if (tagItem->tagCopy()) {
		if (tagItem->tagCopy()->isMultiState()) {
			saveTagTo(tagItem->tagCopy()->newTag);
		} else {
			saveTagTo(tagItem->tagCopy()->newTag);
			saveStateTo(tagItem->tagCopy()->stateCopies[0]->newState);
		}
	} else if (tagItem->stateCopy()) {
		saveTagTo(((TagListViewItem*)(tagItem->parent()))->tagCopy()->newTag);
		saveStateTo(tagItem->stateCopy()->newState);
	}

	m_tags->currentItem()->setup();
	if (m_tags->currentItem()->parent())
		m_tags->currentItem()->parent()->setup();

	m_removeShortcut->setEnabled(!m_shortcut->shortcut().isNull());
	m_removeEmblem->setEnabled(!m_emblem->icon().isEmpty() && !m_tags->currentItem()->isEmblemObligatory());
	m_onEveryLines->setEnabled(!m_textEquivalent->text().isEmpty());
}

void TagsEditDialog::currentItemChanged(Q3ListViewItem *item)
{
	if (item == 0)
		return;

	m_loading = true;

	TagListViewItem *tagItem = (TagListViewItem*)item;
	if (tagItem->tagCopy()) {
		if (tagItem->tagCopy()->isMultiState()) {
			loadTagFrom(tagItem->tagCopy()->newTag);
			loadBlankState();
			m_stateBox->setEnabled(false);
			m_stateBox->setTitle(i18n("State"));
			m_stateNameLabel->setEnabled(true);
			m_stateName->setEnabled(true);
		} else {
			loadTagFrom(tagItem->tagCopy()->newTag); // TODO: No duplicat
			loadStateFrom(tagItem->tagCopy()->stateCopies[0]->newState);
			m_stateBox->setEnabled(true);
			m_stateBox->setTitle(i18n("Appearance"));
			m_stateName->setText("");
			m_stateNameLabel->setEnabled(false);
			m_stateName->setEnabled(false);
		}
	} else if (tagItem->stateCopy()) {
		loadTagFrom(((TagListViewItem*)(tagItem->parent()))->tagCopy()->newTag);
		loadStateFrom(tagItem->stateCopy()->newState);
		m_stateBox->setEnabled(true);
		m_stateBox->setTitle(i18n("State"));
		m_stateNameLabel->setEnabled(true);
		m_stateName->setEnabled(true);
	}

	ensureCurrentItemVisible();

	m_loading = false;
}

void TagsEditDialog::ensureCurrentItemVisible()
{
	TagListViewItem *tagItem = m_tags->currentItem();

	// Ensure the tag and the states (if available) to be visible, but if there is a looooot of states,
	// ensure the tag is still visible, even if the last states are not...
	int y = m_tags->itemPos(tagItem);
	int height = tagItem->totalHeight();
	int bottom = y + qMin(height, m_tags->visibleHeight());
	int xMiddle = m_tags->contentsX() + m_tags->visibleWidth() / 2;
	m_tags->ensureVisible( xMiddle, bottom, 0,0 );
	m_tags->ensureVisible( xMiddle, y,      0,0 );

	m_moveDown->setEnabled( tagItem->nextSibling() != 0 );
	m_moveUp->setEnabled(   tagItem->prevSibling() != 0 );
}

void TagsEditDialog::loadBlankState()
{
	m_stateName->setText("");
	m_emblem->resetIcon();
	m_removeEmblem->setEnabled(false);
	m_backgroundColor->setColor(QColor());
	m_bold->setOn(false);
	m_underline->setOn(false);
	m_italic->setOn(false);
	m_strike->setOn(false);
	m_textColor->setColor(QColor());
	m_font->setCurrentItem(0);
	m_fontSize->setCurrentItem(0);
	m_textEquivalent->setText("");
	m_onEveryLines->setChecked(false);
}

void TagsEditDialog::loadStateFrom(State *state)
{
	m_stateName->setText(state->name());
	if (state->emblem().isEmpty())
		m_emblem->resetIcon();
	else
		m_emblem->setIcon(state->emblem());
	m_removeEmblem->setEnabled(!state->emblem().isEmpty() && !m_tags->currentItem()->isEmblemObligatory());
	m_backgroundColor->setColor(state->backgroundColor());
	m_bold->setOn(state->bold());
	m_underline->setOn(state->underline());
	m_italic->setOn(state->italic());
	m_strike->setOn(state->strikeOut());
	m_textColor->setColor(state->textColor());
	m_textEquivalent->setText(state->textEquivalent());
	m_onEveryLines->setChecked(state->onAllTextLines());

	if (state->fontName().isEmpty())
		m_font->setCurrentItem(0);
	else
		m_font->setCurrentFont(state->fontName());

	if (state->fontSize() == -1)
		m_fontSize->setCurrentItem(0);
	else
		m_fontSize->setCurrentText(QString::number(state->fontSize()));
}

void TagsEditDialog::loadTagFrom(Tag *tag)
{
	m_tagName->setText(tag->name());
	m_shortcut->setShortcut(tag->shortcut(), /*bQtShortcut=*/false);
	m_removeShortcut->setEnabled(!tag->shortcut().isNull());
	m_inherit->setChecked(tag->inheritedBySiblings());
}

void TagsEditDialog::saveStateTo(State *state)
{
	state->setName(m_stateName->text());
	state->setEmblem(m_emblem->icon());
	state->setBackgroundColor(m_backgroundColor->color());
	state->setBold(m_bold->isOn());
	state->setUnderline(m_underline->isOn());
	state->setItalic(m_italic->isOn());
	state->setStrikeOut(m_strike->isOn());
	state->setTextColor(m_textColor->color());
	state->setTextEquivalent(m_textEquivalent->text());
	state->setOnAllTextLines(m_onEveryLines->isChecked());

	if (m_font->currentItem() == 0)
		state->setFontName("");
	else
		state->setFontName(m_font->currentFont());

	bool conversionOk;
	int fontSize = m_fontSize->currentText().toInt(&conversionOk);
	if (conversionOk)
		state->setFontSize(fontSize);
	else
		state->setFontSize(-1);
}

void TagsEditDialog::saveTagTo(Tag *tag)
{
	tag->setName(m_tagName->text());
	tag->setShortcut(m_shortcut->shortcut());
	tag->setInheritedBySiblings(m_inherit->isChecked());
}

void TagsEditDialog::slotCancel()
{
	// All copies of tag have a shortcut, that is stored as a QAction.
	// So, shortcuts are duplicated, and if the user press one tag keyboard-shortcut in the main window, there is a conflict.
	// We then should delete every copies:
	for (TagCopy::List::iterator tagCopyIt = m_tagCopies.begin(); tagCopyIt != m_tagCopies.end(); ++tagCopyIt) {
		delete (*tagCopyIt)->newTag;
	}
}

void TagsEditDialog::slotOk()
{
	Tag::all.clear();
	for (TagCopy::List::iterator tagCopyIt = m_tagCopies.begin(); tagCopyIt != m_tagCopies.end(); ++tagCopyIt) {
		TagCopy *tagCopy = *tagCopyIt;
		// Copy changes to the tag and append in the list of tags::
		if (tagCopy->oldTag) {
			tagCopy->newTag->copyTo(tagCopy->oldTag);
			delete tagCopy->newTag;
		}
		Tag *tag = (tagCopy->oldTag ? tagCopy->oldTag : tagCopy->newTag);
		Tag::all.append(tag);
		// And change all states:
		State::List &states = tag->states();
		StateCopy::List &stateCopies = tagCopy->stateCopies;
		states.clear();
		for (StateCopy::List::iterator stateCopyIt = stateCopies.begin(); stateCopyIt != stateCopies.end(); ++stateCopyIt) {
			StateCopy *stateCopy = *stateCopyIt;
			// Copy changes to the state and append in the list of tags:
			if (stateCopy->oldState)
				stateCopy->newState->copyTo(stateCopy->oldState);
			State *state = (stateCopy->oldState ? stateCopy->oldState : stateCopy->newState);
			states.append(state);
			state->setParentTag(tag);
		}
	}
	Tag::saveTags();

	// Notify removed states and tags, and then remove them:
	if (!m_deletedStates.isEmpty())
		Global::bnpView->removedStates(m_deletedStates);

	// Update every note (change colors, size because of font change or added/removed emblems...):
	Global::bnpView->relayoutAllBaskets();
	Global::bnpView->recomputeAllStyles();
}

#include "tagsedit.moc"

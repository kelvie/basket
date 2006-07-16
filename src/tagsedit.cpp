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

#include <qtooltip.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <kfontcombo.h>
#include <qlayout.h>
#include <kkeybutton.h>
#include <kicondialog.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kglobalsettings.h>
#include <qcheckbox.h>
#include <kpushbutton.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qvaluelist.h>
#include <klocale.h>
#include <qfontdatabase.h>

#include <iostream>

#include "tag.h"
#include "tagsedit.h"
#include "kcolorcombo2.h"

/** class FontSizeCombo: */

FontSizeCombo::FontSizeCombo(bool rw, bool withDefault, QWidget *parent, const char *name)
 : KComboBox(rw, parent, name), m_withDefault(withDefault)
{
	if (m_withDefault)
		insertItem(i18n("(Default)"));

	QFontDatabase fontDB;
	QValueList<int> sizes = fontDB.standardSizes();
	for (QValueList<int>::Iterator it = sizes.begin(); it != sizes.end(); ++it)
		insertItem(QString::number(*it));

	// TODO: 01617 void KFontSizeAction::setFontSize( int size )
}

FontSizeCombo::~FontSizeCombo()
{
}

/** class StateCopy: */

StateCopy::StateCopy(State *old/* = 0*/)
{
	oldState = old;
	newState = new State();
	if (oldState)
		oldState->copyTo(newState);
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
			states.append(new StateCopy(*it));
	else
		states.append(new StateCopy());
}

void TagCopy::copyBack()
{
}

bool TagCopy::isMultiState()
{
	return (states.count() > 1);
}

/** class TagListViewItem: */

TagListViewItem::TagListViewItem(QListView *parent, TagCopy *tagCopy)
 : QListViewItem(parent), m_tagCopy(tagCopy), m_stateCopy(0)
{
	setText(0, tagCopy->newTag->name());
}

TagListViewItem::TagListViewItem(QListViewItem *parent, TagCopy *tagCopy)
 : QListViewItem(parent), m_tagCopy(tagCopy), m_stateCopy(0)
{
	setText(0, tagCopy->newTag->name());
}

TagListViewItem::TagListViewItem(QListView *parent, QListViewItem *after, TagCopy *tagCopy)
 : QListViewItem(parent, after), m_tagCopy(tagCopy), m_stateCopy(0)
{
	setText(0, tagCopy->newTag->name());
}

TagListViewItem::TagListViewItem(QListViewItem *parent, QListViewItem *after, TagCopy *tagCopy)
 : QListViewItem(parent, after), m_tagCopy(tagCopy), m_stateCopy(0)
{
	setText(0, tagCopy->newTag->name());
}

/* */

TagListViewItem::TagListViewItem(QListView *parent, StateCopy *stateCopy)
 : QListViewItem(parent), m_tagCopy(0), m_stateCopy(stateCopy)
{
	setText(0, stateCopy->newState->name());
}

TagListViewItem::TagListViewItem(QListViewItem *parent, StateCopy *stateCopy)
 : QListViewItem(parent), m_tagCopy(0), m_stateCopy(stateCopy)
{
	setText(0, stateCopy->newState->name());
}

TagListViewItem::TagListViewItem(QListView *parent, QListViewItem *after, StateCopy *stateCopy)
 : QListViewItem(parent, after), m_tagCopy(0), m_stateCopy(stateCopy)
{
	setText(0, stateCopy->newState->name());
}

TagListViewItem::TagListViewItem(QListViewItem *parent, QListViewItem *after, StateCopy *stateCopy)
 : QListViewItem(parent, after), m_tagCopy(0), m_stateCopy(stateCopy)
{
	setText(0, stateCopy->newState->name());
}

/* */

TagListViewItem::~TagListViewItem()
{
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

/** class TagListView: */

TagListView::TagListView(QWidget *parent, const char *name, WFlags flags)
 : QListView(parent, name, flags)
{
}

TagListView::~TagListView()
{
}

void TagListView::keyPressEvent(QKeyEvent *event)
{
	// Do not allow to open/close first-level items
	if (event->key() != Qt::Key_Left || (selectedItem() && selectedItem()->parent()))
		QListView::keyPressEvent(event);
}

void TagListView::contentsMouseDoubleClickEvent(QMouseEvent*)
{
	// Ignore this event! Do not open/close first-level items!
}

/** class TagsEditDialog: */

TagsEditDialog::TagsEditDialog(QWidget *parent, const char */*name*/)
 : KDialogBase(KDialogBase::Plain, i18n("Customize Tags"), KDialogBase::Ok | KDialogBase::Cancel,
               KDialogBase::Close, parent, /*name=*/"CustomizeTags", /*modal=*/true, /*separator=*/true),
   m_loading(false)
{
	QHBoxLayout *layout = new QHBoxLayout(plainPage(), /*margin=*/0, spacingHint());

	/* Left part: */

	QPushButton *newTag     = new QPushButton(i18n("Ne&w Tag"),   plainPage());
	QPushButton *newState   = new QPushButton(i18n("New St&ate"), plainPage());

	m_tags = new TagListView(plainPage());
	m_tags->header()->hide();
	m_tags->setRootIsDecorated(false);
	m_tags->addColumn("");
	m_tags->setSorting(-1); // Sort column -1, so disabled sorting
	m_tags->setResizeMode(QListView::LastColumn);

	m_moveUp    = new KPushButton( KGuiItem("", "1uparrow"),   plainPage() );
	m_moveDown  = new KPushButton( KGuiItem("", "1downarrow"), plainPage() );
	m_deleteTag = new KPushButton( KGuiItem("", "editdelete"), plainPage() );

	QToolTip::add( m_moveUp,    i18n("Move Up")   );
	QToolTip::add( m_moveDown,  i18n("Move Down") );
	QToolTip::add( m_deleteTag, i18n("Delete")    );

	connect( m_moveUp,    SIGNAL(clicked()), this, SLOT(moveUp())    );
	connect( m_moveDown,  SIGNAL(clicked()), this, SLOT(moveDown())  );
	connect( m_deleteTag, SIGNAL(clicked()), this, SLOT(deleteTag()) );

	QHBoxLayout *topLeftLayout = new QHBoxLayout(0, /*margin=*/0, spacingHint());
	topLeftLayout->addWidget(m_moveUp);
	topLeftLayout->addWidget(m_moveDown);
	topLeftLayout->addWidget(m_deleteTag);

	QVBoxLayout *leftLayout = new QVBoxLayout(0, /*margin=*/0, spacingHint());
	leftLayout->addWidget(newTag);
	leftLayout->addWidget(newState);
	leftLayout->addWidget(m_tags);
	leftLayout->addLayout(topLeftLayout);

	layout->addLayout(leftLayout);

	/* Right part: */

	QWidget *rightWidget = new QWidget(plainPage());

	QGroupBox *tagBox    = new QGroupBox(1, Qt::Horizontal, i18n("Tag"), rightWidget);
	QWidget   *tagWidget = new QWidget(tagBox);

	m_tagName = new QLineEdit(tagWidget);
	QLabel *tagNameLabel = new QLabel(m_tagName, i18n("&Name:"), tagWidget);

	m_shortcut = new KKeyButton(tagWidget);
	m_removeShortcut = new QPushButton(i18n("&Remove"), tagWidget);
	QLabel *shortcutLabel = new QLabel(m_tagName, i18n("S&hortcut:"), tagWidget);
	connect( m_shortcut,       SIGNAL(capturedShortcut(const KShortcut&)), this, SLOT(capturedShortcut(const KShortcut&)) );
	connect( m_removeShortcut, SIGNAL(clicked()),                          this, SLOT(removeShortcut())                   );

	m_inherit = new QCheckBox(i18n("&Inherited by new sibling notes"), tagWidget);

	QGridLayout *tagGrid = new QGridLayout(tagWidget, /*rows=*/3, /*cols=*/4, /*border=*/0, /*spacing=*/spacingHint());
	tagGrid->addWidget(tagNameLabel,     0, 0);
	tagGrid->addMultiCellWidget(m_tagName, /*fromRow=*/0, /*toRow=*/0, /*fromCol=*/1, /*toCol=*/3);
	tagGrid->addWidget(shortcutLabel,    1, 0);
	tagGrid->addWidget(m_shortcut,       1, 1);
	tagGrid->addWidget(m_removeShortcut, 1, 2);
	tagGrid->addMultiCellWidget(m_inherit, /*fromRow=*/2, /*toRow=*/2, /*fromCol=*/0, /*toCol=*/3);
	tagGrid->setColStretch(/*col=*/3, /*stretch=*/255);

	m_stateBox           = new QGroupBox(1, Qt::Horizontal, i18n("State"), rightWidget);
	QWidget *stateWidget = new QWidget(m_stateBox);

	m_stateName = new QLineEdit(stateWidget);
	m_stateNameLabel = new QLabel(m_stateName, i18n("Na&me:"), stateWidget);

	QWidget *emblemWidget = new QWidget(stateWidget);
	m_emblem = new KIconButton(emblemWidget);
	m_emblem->setIconType(KIcon::NoGroup, KIcon::Action);
	m_emblem->setIconSize(16);
	m_emblem->setIcon("editdelete");
	m_removeEmblem = new QPushButton(i18n("Remo&ve"), emblemWidget);
	QLabel *emblemLabel = new QLabel(m_emblem, i18n("&Emblem:"), stateWidget);
	connect( m_removeEmblem, SIGNAL(clicked()), this, SLOT(removeEmblem()) ); // m_emblem.resetIcon() is not a slot!

	// Make the icon button and the remove button the same height:
	int height = QMAX(m_emblem->sizeHint().width(), m_emblem->sizeHint().height());
	height = QMAX(height, m_removeEmblem->sizeHint().height());
	m_emblem->setFixedSize(height, height); // Make it square
	m_removeEmblem->setFixedHeight(height);
	m_emblem->resetIcon();

	QHBoxLayout *emblemLayout = new QHBoxLayout(emblemWidget, /*margin=*/0, spacingHint());
	emblemLayout->addWidget(m_emblem);
	emblemLayout->addWidget(m_removeEmblem);
	emblemLayout->addStretch();

	QIconSet boldIconSet = kapp->iconLoader()->loadIconSet("text_bold", KIcon::Small);
	m_bold = new QPushButton(boldIconSet, "", stateWidget);
	m_bold->setToggleButton(true);
	int size = QMAX(m_bold->sizeHint().width(), m_bold->sizeHint().height());
	m_bold->setFixedSize(size, size); // Make it square!
	QToolTip::add(m_bold, i18n("Bold"));

	QIconSet underlineIconSet = kapp->iconLoader()->loadIconSet("text_under", KIcon::Small);
	m_underline = new QPushButton(underlineIconSet, "", stateWidget);
	m_underline->setToggleButton(true);
	m_underline->setFixedSize(size, size); // Make it square!
	QToolTip::add(m_underline, i18n("Underline"));

	QIconSet italicIconSet = kapp->iconLoader()->loadIconSet("text_italic", KIcon::Small);
	m_italic = new QPushButton(italicIconSet, "", stateWidget);
	m_italic->setToggleButton(true);
	m_italic->setFixedSize(size, size); // Make it square!
	QToolTip::add(m_italic, i18n("Italic"));

	QIconSet strikeIconSet = kapp->iconLoader()->loadIconSet("text_strike", KIcon::Small);
	m_strike = new QPushButton(strikeIconSet, "", stateWidget);
	m_strike->setToggleButton(true);
	m_strike->setFixedSize(size, size); // Make it square!
	QToolTip::add(m_strike, i18n("Strike Through"));

	QLabel *textLabel = new QLabel(m_bold, i18n("&Text:"), stateWidget);

	QHBoxLayout *textLayout = new QHBoxLayout(0, /*margin=*/0, spacingHint());
	textLayout->addWidget(m_bold);
	textLayout->addWidget(m_underline);
	textLayout->addWidget(m_italic);
	textLayout->addWidget(m_strike);
	textLayout->addStretch();

	m_textColor = new KColorCombo2(QColor(), KGlobalSettings::textColor(), stateWidget);
	QLabel *textColorLabel = new QLabel(m_textColor, i18n("Co&lor:"), stateWidget);

	m_font = new KFontCombo(stateWidget);
	m_font->insertItem(i18n("(Default)"), 0);
	QLabel *fontLabel = new QLabel(m_font, i18n("&Font:"), stateWidget);

	m_fontSize = new FontSizeCombo(/*rw=*/true, /*withDefault=*/true, stateWidget);
	QLabel *fontSizeLabel = new QLabel(m_font, i18n("&Size:"), stateWidget);

	m_backgroundColor = new KColorCombo2(QColor(), KGlobalSettings::baseColor(), stateWidget);
	QLabel *backgroundColorLabel = new QLabel(m_backgroundColor, i18n("&Background:"), stateWidget);

	QHBoxLayout *backgroundColorLayout = new QHBoxLayout(0, /*margin=*/0, spacingHint());
	backgroundColorLayout->addWidget(m_backgroundColor);
	backgroundColorLayout->addStretch();

	QGridLayout *stateGrid = new QGridLayout(stateWidget, /*rows=*/5, /*cols=*/7, /*border=*/0, /*spacing=*/spacingHint());
	stateGrid->addWidget(m_stateNameLabel,     0, 0);
	stateGrid->addMultiCellWidget(m_stateName,            /*fromRow=*/0, /*toRow=*/0, /*fromCol=*/1, /*toCol=*/6);
	stateGrid->addWidget(emblemLabel,          1, 0);
	stateGrid->addMultiCellWidget(emblemWidget,           /*fromRow=*/1, /*toRow=*/1, /*fromCol=*/1, /*toCol=*/6);
	stateGrid->addWidget(textLabel,            2, 0);
	stateGrid->addMultiCellLayout(textLayout,             /*fromRow=*/2, /*toRow=*/2, /*fromCol=*/1, /*toCol=*/4);
	stateGrid->addWidget(textColorLabel,       2, 5);
	stateGrid->addWidget(m_textColor,          2, 6);
	stateGrid->addWidget(fontLabel,            3, 0);
	stateGrid->addMultiCellWidget(m_font,                 /*fromRow=*/3, /*toRow=*/3, /*fromCol=*/1, /*toCol=*/4);
	stateGrid->addWidget(fontSizeLabel,        3, 5);
	stateGrid->addWidget(m_fontSize,           3, 6);
	stateGrid->addWidget(backgroundColorLabel, 1, 5);
	stateGrid->addMultiCellLayout(backgroundColorLayout,  /*fromRow=*/1, /*toRow=*/1, /*fromCol=*/6, /*toCol=*/6);

	QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget, /*margin=*/0, spacingHint());
	rightLayout->addWidget(tagBox);
	rightLayout->addWidget(m_stateBox);
	rightLayout->addStretch();

	layout->addWidget(rightWidget);
	rightWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);

	// Equalize the width of the first column of the two grids:
	int maxWidth = tagNameLabel->sizeHint().width();
	maxWidth = QMAX(maxWidth, shortcutLabel->sizeHint().width());
	maxWidth = QMAX(maxWidth, m_stateNameLabel->sizeHint().width());
	maxWidth = QMAX(maxWidth, emblemLabel->sizeHint().width());
	maxWidth = QMAX(maxWidth, textLabel->sizeHint().width());
	maxWidth = QMAX(maxWidth, fontLabel->sizeHint().width());
	maxWidth = QMAX(maxWidth, backgroundColorLabel->sizeHint().width());

	tagNameLabel->setFixedWidth(maxWidth);
	m_stateNameLabel->setFixedWidth(maxWidth);

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
			StateCopy::List stateCopies = item->tagCopy()->states;
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

	connect( m_tags,            SIGNAL(currentChanged(QListViewItem*)),     this, SLOT(currentItemChanged(QListViewItem*)) );

	if (m_tags->firstChild()) {
		m_tags->firstChild()->setSelected(true);
		currentItemChanged(m_tags->firstChild());
		m_tags->setFocus();
	}
	// TODO: Disabled both boxes if no tag!!!
}

TagsEditDialog::~TagsEditDialog()
{
}

void TagsEditDialog::newTag()
{
}

void TagsEditDialog::newState()
{
}

void TagsEditDialog::moveUp()
{
	TagListViewItem *tagItem     = ((TagListViewItem*)( m_tags->currentItem()  ));
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
		StateCopy::List &stateCopies = ((TagListViewItem*)( tagItem->parent() ))->tagCopy()->states;
		int pos = stateCopies.findIndex(tagItem->stateCopy());
		stateCopies.remove(tagItem->stateCopy());
		int i = 0;
		for (StateCopy::List::iterator it = stateCopies.begin(); it != stateCopies.end(); ++it, ++i)
			if (i == pos - 1) {
				stateCopies.insert(it, tagItem->stateCopy());
				break;
			}
	}

	m_moveDown->setEnabled( tagItem->nextSibling() != 0 );
	m_moveUp->setEnabled(   tagItem->prevSibling() != 0 );
}

void TagsEditDialog::moveDown()
{
	TagListViewItem *tagItem = ((TagListViewItem*)( m_tags->currentItem() ));

	// Move in the list view:
	tagItem->moveItem(tagItem->nextSibling());

	// Move in the value list:
	if (tagItem->tagCopy()) {
		int pos = m_tagCopies.findIndex(tagItem->tagCopy());
		m_tagCopies.remove(tagItem->tagCopy());
		if (pos == m_tagCopies.count() - 1) // Insert at end: iterator does not go there
			m_tagCopies.append(tagItem->tagCopy());
		else {
			int i = 0;
			for (TagCopy::List::iterator it = m_tagCopies.begin(); it != m_tagCopies.end(); ++it, ++i)
				if (i == pos + 1) {
					m_tagCopies.insert(it, tagItem->tagCopy());
					break;
				}
		}
	} else {
		StateCopy::List &stateCopies = ((TagListViewItem*)( tagItem->parent() ))->tagCopy()->states;
		int pos = stateCopies.findIndex(tagItem->stateCopy());
		stateCopies.remove(tagItem->stateCopy());
		if (pos == stateCopies.count() - 1) // Insert at end: iterator does not go there
			stateCopies.append(tagItem->stateCopy());
		else {
			int i = 0;
			for (StateCopy::List::iterator it = stateCopies.begin(); it != stateCopies.end(); ++it, ++i)
				if (i == pos + 1) {
					stateCopies.insert(it, tagItem->stateCopy());
					break;
				}
		}
	}

	m_moveDown->setEnabled( tagItem->nextSibling() != 0 );
	m_moveUp->setEnabled(   tagItem->prevSibling() != 0 );
}

void TagsEditDialog::deleteTag()
{
	//TagListViewItem *item = ((TagListViewItem*)(m_tags->currentItem()));

	//if (item->tagCopy())
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

	static int cpt = 0;
	std::cout << "modified " << cpt << std::endl;
	cpt++;

	TagListViewItem *tagItem = (TagListViewItem*)m_tags->currentItem();
	if (tagItem == 0)
		return;

	if (tagItem->tagCopy()) {
		if (tagItem->tagCopy()->isMultiState()) {
			saveTagTo(tagItem->tagCopy()->newTag);
		} else {
			saveTagTo(tagItem->tagCopy()->newTag);
			saveStateTo(tagItem->tagCopy()->states[0]->newState);
		}
	} else if (tagItem->stateCopy()) {
		saveTagTo(((TagListViewItem*)(tagItem->parent()))->tagCopy()->newTag);
		saveStateTo(tagItem->stateCopy()->newState);
	}

	m_removeShortcut->setEnabled(!m_shortcut->shortcut().isNull());
	m_removeEmblem->setEnabled(!m_emblem->icon().isEmpty());
}

void TagsEditDialog::currentItemChanged(QListViewItem *item)
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
			loadStateFrom(tagItem->tagCopy()->states[0]->newState);
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

	m_moveDown->setEnabled( tagItem->nextSibling() != 0 );
	m_moveUp->setEnabled(   tagItem->prevSibling() != 0 );

	m_loading = false;
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
}

void TagsEditDialog::loadStateFrom(State *state)
{
	m_stateName->setText(state->name());
	if (state->emblem().isEmpty())
		m_emblem->resetIcon();
	else
		m_emblem->setIcon(state->emblem());
	m_removeEmblem->setEnabled(!state->emblem().isEmpty());
	m_backgroundColor->setColor(state->backgroundColor());
	m_bold->setOn(state->bold());
	m_underline->setOn(state->underline());
	m_italic->setOn(state->italic());
	m_strike->setOn(state->strikeOut());
	m_textColor->setColor(state->textColor());

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

	if (m_font->currentItem() == 0)
		state->setFontName("");
	else
		state->setFontName(m_font->currentFont());

	if (m_fontSize->currentItem() == 0)
		state->setFontSize(-1);
	else
		state->setFontSize(m_fontSize->currentText().toInt());
}

void TagsEditDialog::saveTagTo(Tag *tag)
{
	tag->setName(m_tagName->text());
	tag->setShortcut(m_shortcut->shortcut());
	tag->setInheritedBySiblings(m_inherit->isChecked());
}

void TagsEditDialog::slotOk()
{
	// TODO: Notify removed states and tags, and then remove them

	Tag::all.clear();
	for (TagCopy::List::iterator tagCopyIt = m_tagCopies.begin(); tagCopyIt != m_tagCopies.end(); ++tagCopyIt) {
//	for (QListViewItem *tagItem = m_tags->firstChild(); tagItem; tagItem = tagItem->nextSibling()) {
		TagCopy *tagCopy = *tagCopyIt;
//		TagCopy *tagCopy = ((TagListViewItem*)tagItem)->tagCopy();
		// Copy changes to the tag and append in the list of tags::
		if (tagCopy->oldTag)
			tagCopy->newTag->copyTo(tagCopy->oldTag);
		Tag *tag = (tagCopy->oldTag ? tagCopy->oldTag : tagCopy->newTag);
std::cout << "Tag " << tag->name() << std::endl;
		Tag::all.append(tag);
		// And change all states:
		State::List &states = tag->states();
		StateCopy::List &stateCopies = tagCopy->states;
		states.clear();
		for (StateCopy::List::iterator stateCopyIt = stateCopies.begin(); stateCopyIt != stateCopies.end(); ++stateCopyIt) {
//		for (QListViewItem *stateItem = tagItem->firstChild(); stateItem; stateItem = stateItem->nextSibling()) {
			StateCopy *stateCopy = *stateCopyIt;
//			StateCopy *stateCopy = ((TagListViewItem*)stateItem)->stateCopy();
			// Copy changes to the state and append in the list of tags:
			if (stateCopy->oldState)
				stateCopy->newState->copyTo(stateCopy->oldState);
			State *state = (stateCopy->oldState ? stateCopy->oldState : stateCopy->newState);
std::cout << "  State " << state->name() << std::endl;
			states.append(state);
			state->setParentTag(tag);
		}
	}
	Tag::saveTags();

	KDialogBase::slotOk();
}

#include "tagsedit.moc"

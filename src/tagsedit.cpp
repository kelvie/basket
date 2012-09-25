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

#include "tagsedit.h"

#include <QtCore/QEvent>
#include <QtCore/QList>
#include <QtCore/QTimer>
#include <QtGui/QAction>
#include <QtGui/QCheckBox>
#include <QtGui/QFontComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>    //For m_tags->header()
#include <QtGui/QHBoxLayout>
#include <QtGui/QKeyEvent>
#include <QtGui/QLabel>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QVBoxLayout>

#include <KDE/KApplication>
#include <KDE/KLineEdit>
#include <KDE/KIconButton>
#include <KDE/KIconLoader>
#include <KDE/KLocale>
#include <KDE/KMessageBox>
#include <KDE/KPushButton>
#include <KDE/KSeparator>
#include <kshortcutwidget.h>

#include "tag.h"
#include "kcolorcombo2.h"
#include "variouswidgets.h"         //For FontSizeCombo
#include "global.h"
#include "bnpview.h"

#include <KDE/KDebug>

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

TagListViewItem::TagListViewItem(QTreeWidget *parent, TagCopy *tagCopy)
        : QTreeWidgetItem(parent), m_tagCopy(tagCopy), m_stateCopy(0)
{
    setText(0, tagCopy->newTag->name());
}

TagListViewItem::TagListViewItem(QTreeWidgetItem *parent, TagCopy *tagCopy)
        : QTreeWidgetItem(parent), m_tagCopy(tagCopy), m_stateCopy(0)
{
    setText(0, tagCopy->newTag->name());
}

TagListViewItem::TagListViewItem(QTreeWidget *parent, QTreeWidgetItem *after, TagCopy *tagCopy)
        : QTreeWidgetItem(parent, after), m_tagCopy(tagCopy), m_stateCopy(0)
{
    setText(0, tagCopy->newTag->name());
}

TagListViewItem::TagListViewItem(QTreeWidgetItem *parent, QTreeWidgetItem *after, TagCopy *tagCopy)
        : QTreeWidgetItem(parent, after), m_tagCopy(tagCopy), m_stateCopy(0)
{
    setText(0, tagCopy->newTag->name());
}

/* */

TagListViewItem::TagListViewItem(QTreeWidget *parent, StateCopy *stateCopy)
        : QTreeWidgetItem(parent), m_tagCopy(0), m_stateCopy(stateCopy)
{
    setText(0, stateCopy->newState->name());
}

TagListViewItem::TagListViewItem(QTreeWidgetItem *parent, StateCopy *stateCopy)
        : QTreeWidgetItem(parent), m_tagCopy(0), m_stateCopy(stateCopy)
{
    setText(0, stateCopy->newState->name());
}

TagListViewItem::TagListViewItem(QTreeWidget *parent, QTreeWidgetItem *after, StateCopy *stateCopy)
        : QTreeWidgetItem(parent, after), m_tagCopy(0), m_stateCopy(stateCopy)
{
    setText(0, stateCopy->newState->name());
}

TagListViewItem::TagListViewItem(QTreeWidgetItem *parent, QTreeWidgetItem *after, StateCopy *stateCopy)
        : QTreeWidgetItem(parent, after), m_tagCopy(0), m_stateCopy(stateCopy)
{
    setText(0, stateCopy->newState->name());
}

/* */

TagListViewItem::~TagListViewItem()
{
}

TagListViewItem* TagListViewItem::lastChild()
{
    if (childCount() <= 0)
        return 0;
    return (TagListViewItem *)child(childCount() - 1);
}

bool TagListViewItem::isEmblemObligatory()
{
    return m_stateCopy != 0; // It's a state of a multi-state
}

TagListViewItem* TagListViewItem::prevSibling()
{
    TagListViewItem *item = this;
    int idx = 0;
    if (!parent()) {
        idx = treeWidget()->indexOfTopLevelItem(item);
        if (idx <= 0) return NULL;
        return (TagListViewItem *)treeWidget()->topLevelItem(idx - 1);
    } else {
        idx = parent()->indexOfChild(item);
        if (idx <= 0) return NULL;
        return (TagListViewItem *)parent()->child(idx - 1);
    }
}

TagListViewItem* TagListViewItem::nextSibling()
{
    TagListViewItem *item = this;
    int idx = 0;
    if (!parent()) {
        idx = treeWidget()->indexOfTopLevelItem(item);
        if (idx >= treeWidget()->topLevelItemCount()) return NULL;
        return (TagListViewItem *)treeWidget()->topLevelItem(idx + 1);
    } else {
        idx = parent()->indexOfChild(item);
        if (idx >= parent()->childCount()) return NULL;
        return (TagListViewItem *)parent()->child(idx + 1);
    }
}


TagListViewItem* TagListViewItem::parent() const
{
    return (TagListViewItem*) QTreeWidgetItem::parent();
}

// TODO: TagListViewItem::
int TAG_ICON_SIZE = 16;
int TAG_MARGIN = 1;

int TagListViewItem::width(const QFontMetrics &/* fontMetrics */, const QTreeWidget* /*listView*/, int /* column */) const
{
    return treeWidget()->width();
}

void TagListViewItem::setup()
{
    QString text = (m_tagCopy ? m_tagCopy->newTag->name() : m_stateCopy->newState->name());
    State *state = (m_tagCopy ? m_tagCopy->stateCopies[0]->newState : m_stateCopy->newState);

    if (m_tagCopy && !m_tagCopy->newTag->shortcut().isEmpty())
        text = i18nc("Tag name (shortcut)", "%1 (%2)", text, m_tagCopy->newTag->shortcut().toString());

    QFont font = state->font(treeWidget()->font());

    QRect textRect = QFontMetrics(font).boundingRect(0, 0, /*width=*/1, 500000, Qt::AlignLeft | Qt::AlignTop, text);

    //widthChanged();
    //int height = TAG_MARGIN + qMax(TAG_ICON_SIZE, textRect.height()) + TAG_MARGIN;
    //setHeight(height);

    //repaint();

    QBrush brush;

    bool withIcon = m_stateCopy || (m_tagCopy && !m_tagCopy->isMultiState());
    brush.setColor(isSelected() ? kapp->palette().color(QPalette::Highlight)  :
                   (withIcon && state->backgroundColor().isValid() ? state->backgroundColor() :
                    treeWidget()->viewport()->palette().color(treeWidget()->viewport()->backgroundRole())));
    setBackground(1, brush);
}

/** class TagListView: */

TagListView::TagListView(QWidget *parent)
        : QTreeWidget(parent)
{
    setItemDelegate(new TagListDelegate);
}

TagListView::~TagListView()
{
}

void TagListView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete)
        emit deletePressed();
    else if (event->key() != Qt::Key_Left || (currentItem() && currentItem()->parent()))
        // Do not allow to open/close first-level items
        QTreeWidget::keyPressEvent(event);
}

void TagListView::mouseDoubleClickEvent(QMouseEvent *event)
{
    // Ignore this event! Do not open/close first-level items!

    // But trigger edit (change focus to name) when double-click an item:
    if (itemAt(event->pos()) != 0)
        emit doubleClickedItem();
}

void TagListView::mousePressEvent(QMouseEvent *event)
{
    // When clicking on an empty space, QListView would unselect the current item! We forbid that!
    if (itemAt(event->pos()) != 0)
        QTreeWidget::mousePressEvent(event);
}

void TagListView::mouseReleaseEvent(QMouseEvent *event)
{
    // When clicking on an empty space, QListView would unselect the current item! We forbid that!
    if (itemAt(event->pos()) != 0)
        QTreeWidget::mouseReleaseEvent(event);
}

TagListViewItem* TagListView::currentItem() const
{
    return (TagListViewItem*) QTreeWidget::currentItem();
}

TagListViewItem* TagListView::firstChild() const
{
    if (topLevelItemCount() <= 0)
        return NULL;
    return (TagListViewItem*)topLevelItem(0);
}

TagListViewItem* TagListView::lastItem() const
{
    if (topLevelItemCount() <= 0)
        return NULL;
    return (TagListViewItem*)topLevelItem(topLevelItemCount() - 1);
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

    QHBoxLayout *layout = new QHBoxLayout(mainWidget());

    /* Left part: */

    QPushButton *newTag     = new QPushButton(i18n("Ne&w Tag"),   mainWidget());
    QPushButton *newState   = new QPushButton(i18n("New St&ate"), mainWidget());

    connect(newTag,   SIGNAL(clicked()), this, SLOT(newTag()));
    connect(newState, SIGNAL(clicked()), this, SLOT(newState()));

    m_tags = new TagListView(mainWidget());
    m_tags->header()->hide();
    m_tags->setRootIsDecorated(false);
    //m_tags->addColumn("");
    //m_tags->setSorting(-1); // Sort column -1, so disabled sorting
    //m_tags->setResizeMode(QTreeWidget::LastColumn);

    m_tags->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_moveUp    = new KPushButton(KGuiItem("", "arrow-up"),   mainWidget());
    m_moveDown  = new KPushButton(KGuiItem("", "arrow-down"), mainWidget());
    m_deleteTag = new KPushButton(KGuiItem("", "edit-delete"), mainWidget());

    m_moveUp->setToolTip(i18n("Move Up (Ctrl+Shift+Up)"));
    m_moveDown->setToolTip(i18n("Move Down (Ctrl+Shift+Down)"));
    m_deleteTag->setToolTip(i18n("Delete"));

    connect(m_moveUp,    SIGNAL(clicked()), this, SLOT(moveUp()));
    connect(m_moveDown,  SIGNAL(clicked()), this, SLOT(moveDown()));
    connect(m_deleteTag, SIGNAL(clicked()), this, SLOT(deleteTag()));

    QHBoxLayout *topLeftLayout = new QHBoxLayout;
    topLeftLayout->addWidget(m_moveUp);
    topLeftLayout->addWidget(m_moveDown);
    topLeftLayout->addWidget(m_deleteTag);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(newTag);
    leftLayout->addWidget(newState);
    leftLayout->addWidget(m_tags);
    leftLayout->addLayout(topLeftLayout);

    layout->addLayout(leftLayout);

    /* Right part: */

    QWidget *rightWidget = new QWidget(mainWidget());

    m_tagBox             = new QGroupBox(i18n("Tag"), rightWidget);
    m_tagBoxLayout       = new QHBoxLayout;
    m_tagBox->setLayout(m_tagBoxLayout);

    QWidget   *tagWidget = new QWidget;
    m_tagBoxLayout->addWidget(tagWidget);

    m_tagName = new KLineEdit(tagWidget);
    QLabel *tagNameLabel = new QLabel(i18n("&Name:"), tagWidget);
    tagNameLabel->setBuddy(m_tagName);

    m_shortcut = new KShortcutWidget(tagWidget);
    m_removeShortcut = new QPushButton(i18nc("Remove tag shortcut", "&Remove"), tagWidget);
    QLabel *shortcutLabel = new QLabel(i18n("S&hortcut:"), tagWidget);
    shortcutLabel->setBuddy(m_shortcut);
    //connect( m_shortcut,       SIGNAL(shortcutChanged(const KShortcut&)), this, SLOT(capturedShortcut(const KShortcut&)) );
    connect(m_removeShortcut, SIGNAL(clicked()),                          this, SLOT(removeShortcut()));

    m_inherit = new QCheckBox(i18n("&Inherited by new sibling notes"), tagWidget);

    m_allowCrossRefernce = new QCheckBox(i18n("Allow Cross Reference Links"), tagWidget);

    HelpLabel *allowCrossReferenceHelp = new HelpLabel(
        i18n("What does this do?"),
        "<p>" + i18n("This option will enable you to type a cross reference link directly into a text note. Cross Reference links can have the following sytax:") + "</p>" +
        "<p>" + i18n("From the top of the tree (Absolute path):") + "<br />" + i18n("[[/top level item/child|optional title]]") + "<p>" +
        "<p>" + i18n("Relative to the current basket:") + "<br />" + i18n("[[../sibling|optional title]]") + "<br />" +
        i18n("[[child|optional title]]") + "<br />" + i18n("[[./child|optional title]]") + "<p>",
        tagWidget);

    QGridLayout *tagGrid = new QGridLayout(tagWidget);
    tagGrid->addWidget(tagNameLabel, 0, 0);
    tagGrid->addWidget(m_tagName, 0, 1, 1, 3);
    tagGrid->addWidget(shortcutLabel, 1, 0);
    tagGrid->addWidget(m_shortcut, 1, 1);
    tagGrid->addWidget(m_removeShortcut, 1, 2);
    tagGrid->addWidget(m_inherit, 2, 0, 1, 4);
    tagGrid->addWidget(m_allowCrossRefernce, 3, 0);
    tagGrid->addWidget(allowCrossReferenceHelp, 3, 1);
    tagGrid->setColumnStretch(/*col=*/3, /*stretch=*/255);

    m_stateBox           = new QGroupBox(i18n("State"), rightWidget);
    m_stateBoxLayout = new QHBoxLayout;
    m_stateBox->setLayout(m_stateBoxLayout);

    QWidget *stateWidget = new QWidget;
    m_stateBoxLayout->addWidget(stateWidget);

    m_stateName = new KLineEdit(stateWidget);
    m_stateNameLabel = new QLabel(i18n("Na&me:"), stateWidget);
    m_stateNameLabel->setBuddy(m_stateName);

    QWidget *emblemWidget = new QWidget(stateWidget);
    m_emblem = new KIconButton(emblemWidget);
    m_emblem->setIconType(KIconLoader::NoGroup, KIconLoader::Action);
    m_emblem->setIconSize(16);
    m_emblem->setIcon("edit-delete");
    m_removeEmblem = new QPushButton(i18nc("Remove tag emblem", "Remo&ve"), emblemWidget);
    QLabel *emblemLabel = new QLabel(i18n("&Emblem:"), stateWidget);
    emblemLabel->setBuddy(m_emblem);
    connect(m_removeEmblem, SIGNAL(clicked()), this, SLOT(removeEmblem()));   // m_emblem.resetIcon() is not a slot!

    // Make the icon button and the remove button the same height:
    int height = qMax(m_emblem->sizeHint().width(), m_emblem->sizeHint().height());
    height = qMax(height, m_removeEmblem->sizeHint().height());
    m_emblem->setFixedSize(height, height); // Make it square
    m_removeEmblem->setFixedHeight(height);
    m_emblem->resetIcon();

    QHBoxLayout *emblemLayout = new QHBoxLayout(emblemWidget);
    emblemLayout->addWidget(m_emblem);
    emblemLayout->addWidget(m_removeEmblem);
    emblemLayout->addStretch();

    m_backgroundColor = new KColorCombo2(QColor(), palette().color(QPalette::Base), stateWidget);
    QLabel *backgroundColorLabel = new QLabel(i18n("&Background:"), stateWidget);
    backgroundColorLabel->setBuddy(m_backgroundColor);

    QHBoxLayout *backgroundColorLayout = new QHBoxLayout(0);
    backgroundColorLayout->addWidget(m_backgroundColor);
    backgroundColorLayout->addStretch();

    //QIcon boldIconSet = KIconLoader::global()->loadIconSet("format-text-bold", KIconLoader::Small);
    KIcon boldIconSet("format-text-bold", KIconLoader::global());
    m_bold = new QPushButton(boldIconSet, "", stateWidget);
    m_bold->setCheckable(true);
    int size = qMax(m_bold->sizeHint().width(), m_bold->sizeHint().height());
    m_bold->setFixedSize(size, size); // Make it square!
    m_bold->setToolTip(i18n("Bold"));

    //QIcon underlineIconSet = KIconLoader::global()->loadIconSet("format-text-underline", KIconLoader::Small);
    KIcon underlineIconSet("format-text-underline", KIconLoader::global());
    m_underline = new QPushButton(underlineIconSet, "", stateWidget);
    m_underline->setCheckable(true);
    m_underline->setFixedSize(size, size); // Make it square!
    m_underline->setToolTip(i18n("Underline"));

    //QIcon italicIconSet = KIconLoader::global()->loadIconSet("format-text-italic", KIconLoader::Small);
    KIcon italicIconSet("format-text-italic", KIconLoader::global());
    m_italic = new QPushButton(italicIconSet, "", stateWidget);
    m_italic->setCheckable(true);
    m_italic->setFixedSize(size, size); // Make it square!
    m_italic->setToolTip(i18n("Italic"));

    KIcon strikeIconSet("format-text-strikethrough", KIconLoader::global());
    m_strike = new QPushButton(strikeIconSet, "", stateWidget);
    m_strike->setCheckable(true);
    m_strike->setFixedSize(size, size); // Make it square!
    m_strike->setToolTip(i18n("Strike Through"));

    QLabel *textLabel = new QLabel(i18n("&Text:"), stateWidget);
    textLabel->setBuddy(m_bold);

    QHBoxLayout *textLayout = new QHBoxLayout;
    textLayout->addWidget(m_bold);
    textLayout->addWidget(m_underline);
    textLayout->addWidget(m_italic);
    textLayout->addWidget(m_strike);
    textLayout->addStretch();

    m_textColor = new KColorCombo2(QColor(), palette().color(QPalette::Text), stateWidget);
    QLabel *textColorLabel = new QLabel(i18n("Co&lor:"), stateWidget);
    textColorLabel->setBuddy(m_textColor);

    m_font = new QFontComboBox(stateWidget);
    m_font->addItem(i18n("(Default)"), 0);
    QLabel *fontLabel = new QLabel(i18n("&Font:"), stateWidget);
    fontLabel->setBuddy(m_font);

    m_fontSize = new FontSizeCombo(/*rw=*/true, /*withDefault=*/true, stateWidget);
    QLabel *fontSizeLabel = new QLabel(i18n("&Size:"), stateWidget);
    fontSizeLabel->setBuddy(m_fontSize);

    m_textEquivalent = new KLineEdit(stateWidget);
    QLabel *textEquivalentLabel = new QLabel(i18n("Te&xt equivalent:"), stateWidget);
    textEquivalentLabel->setBuddy(m_textEquivalent);
    QFont font = m_textEquivalent->font();
    font.setFamily("monospace");
    m_textEquivalent->setFont(font);

    HelpLabel *textEquivalentHelp = new HelpLabel(
        i18n("What is this for?"),
        "<p>" + i18n("When you copy and paste or drag and drop notes to a text editor, this text will be inserted as a textual equivalent of the tag.") + "</p>" +
//      "<p>" + i18n("If filled, this property lets you paste this tag or this state as textual equivalent.") + "<br>" +
        i18n("For instance, a list of notes with the <b>To Do</b> and <b>Done</b> tags are exported as lines preceded by <b>[ ]</b> or <b>[x]</b>, "
             "representing an empty checkbox and a checked box.") + "</p>" +
        "<p align='center'><img src=\":images/tag_export_help.png\"></p>",
        stateWidget);
    QHBoxLayout *textEquivalentHelpLayout = new QHBoxLayout;
    textEquivalentHelpLayout->addWidget(textEquivalentHelp);
    textEquivalentHelpLayout->addStretch(255);

    m_onEveryLines = new QCheckBox(i18n("On ever&y line"), stateWidget);

    HelpLabel *onEveryLinesHelp = new HelpLabel(
        i18n("What does this mean?"),
        "<p>" + i18n("When a note has several lines, you can choose to export the tag or the state on the first line or on every line of the note.") + "</p>" +
        "<p align='center'><img src=\":images/tag_export_on_every_lines_help.png\"></p>" +
        "<p>" + i18n("In the example above, the tag of the top note is only exported on the first line, while the tag of the bottom note is exported on every line of the note."),
        stateWidget);
    QHBoxLayout *onEveryLinesHelpLayout = new QHBoxLayout;
    onEveryLinesHelpLayout->addWidget(onEveryLinesHelp);
    onEveryLinesHelpLayout->addStretch(255);

    QGridLayout *textEquivalentGrid = new QGridLayout;
    textEquivalentGrid->addWidget(textEquivalentLabel,      0, 0);
    textEquivalentGrid->addWidget(m_textEquivalent,         0, 1);
    textEquivalentGrid->addLayout(textEquivalentHelpLayout, 0, 2);
    textEquivalentGrid->addWidget(m_onEveryLines,           1, 1);
    textEquivalentGrid->addLayout(onEveryLinesHelpLayout,   1, 2);
    textEquivalentGrid->setColumnStretch(/*col=*/3, /*stretch=*/255);

    KSeparator *separator = new KSeparator(Qt::Horizontal, stateWidget);

    QGridLayout *stateGrid = new QGridLayout(stateWidget);
    stateGrid->addWidget(m_stateNameLabel, 0, 0);
    stateGrid->addWidget(m_stateName, 0, 1, 1, 6);
    stateGrid->addWidget(emblemLabel, 1, 0);
    stateGrid->addWidget(emblemWidget, 1, 1, 1, 6);
    stateGrid->addWidget(backgroundColorLabel, 1, 5);
    stateGrid->addLayout(backgroundColorLayout, 1, 6, 1, 1);
    stateGrid->addWidget(textLabel, 2, 0);
    stateGrid->addLayout(textLayout, 2, 1, 1, 4);
    stateGrid->addWidget(textColorLabel, 2, 5);
    stateGrid->addWidget(m_textColor, 2, 6);
    stateGrid->addWidget(fontLabel, 3, 0);
    stateGrid->addWidget(m_font, 3, 1, 1, 4);
    stateGrid->addWidget(fontSizeLabel, 3, 5);
    stateGrid->addWidget(m_fontSize, 3, 6);
    stateGrid->addWidget(separator, 4, 0, 1, 7);
    stateGrid->addLayout(textEquivalentGrid, 5, 0, 1, 7);

    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
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
        item->setExpanded(true);
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
    connect(m_tagName,         SIGNAL(editTextChanged(const QString&)),        this, SLOT(modified()));
    connect(m_shortcut,        SIGNAL(shortcutChanged(const KShortcut&)), this, SLOT(modified()));
    connect(m_inherit,         SIGNAL(stateChanged(int)),                  this, SLOT(modified()));
    connect(m_allowCrossRefernce, SIGNAL(clicked(bool)),                   this, SLOT(modified()));
    connect(m_stateName,       SIGNAL(editTextChanged(const QString&)),        this, SLOT(modified()));
    connect(m_emblem,          SIGNAL(iconChanged(QString)),               this, SLOT(modified()));
    connect(m_backgroundColor, SIGNAL(changed(const QColor&)),             this, SLOT(modified()));
    connect(m_bold,            SIGNAL(toggled(bool)),                      this, SLOT(modified()));
    connect(m_underline,       SIGNAL(toggled(bool)),                      this, SLOT(modified()));
    connect(m_italic,          SIGNAL(toggled(bool)),                      this, SLOT(modified()));
    connect(m_strike,          SIGNAL(toggled(bool)),                      this, SLOT(modified()));
    connect(m_textColor,       SIGNAL(activated(int)),             this, SLOT(modified()));
    connect(m_font,            SIGNAL(editTextChanged(const QString&)),        this, SLOT(modified()));
    connect(m_fontSize,        SIGNAL(editTextChanged(const QString&)),        this, SLOT(modified()));
    connect(m_textEquivalent,  SIGNAL(editTextChanged(const QString&)),        this, SLOT(modified()));
    connect(m_onEveryLines,    SIGNAL(stateChanged(int)),                  this, SLOT(modified()));

    connect(m_tags,            SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),     this,
            SLOT(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
    connect(m_tags,            SIGNAL(deletePressed()),                    this, SLOT(deleteTag()));
    connect(m_tags,            SIGNAL(doubleClickedItem()),                this, SLOT(renameIt()));

    QTreeWidgetItem *firstItem = m_tags->firstChild();
    if (stateToEdit != 0) {
        TagListViewItem *item = itemForState(stateToEdit);
        if (item)
            firstItem = item;
    }
    // Select the first tag unless the first tag is a multi-state tag.
    // In this case, select the first state, as it let customize the state AND the associated tag.
    if (firstItem) {
        if (firstItem->childCount() > 0)
            firstItem = firstItem->child(0);
        firstItem->setSelected(true);
        m_tags->setCurrentItem(firstItem);
        currentItemChanged(firstItem);
        if (stateToEdit == 0)
            m_tags->scrollToItem(firstItem);
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
    selectAbove->setShortcut(Qt::CTRL + Qt::Key_Up);
    connect(selectAbove, SIGNAL(triggered()), this, SLOT(selectUp()));

    QAction *selectBelow = new QAction(this);
    selectBelow->setShortcut(Qt::CTRL + Qt::Key_Down);
    connect(selectBelow, SIGNAL(triggered()), this, SLOT(selectDown()));

    QAction *selectLeft = new QAction(this);
    selectLeft->setShortcut(Qt::CTRL + Qt::Key_Left);
    connect(selectLeft, SIGNAL(triggered()), this, SLOT(selectLeft()));

    QAction *selectRight = new QAction(this);
    selectRight->setShortcut(Qt::CTRL + Qt::Key_Right);
    connect(selectRight, SIGNAL(triggered()), this, SLOT(selectRight()));

    QAction *moveAbove = new QAction(this);
    moveAbove->setShortcut(Qt::CTRL + Qt::Key_Up);
    connect(moveAbove, SIGNAL(triggered()), this, SLOT(moveUp()));

    QAction *moveBelow = new QAction(this);
    moveBelow->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Down);
    connect(moveBelow, SIGNAL(triggered()), this, SLOT(moveDown()));

    QAction *rename = new QAction(this);
    rename->setShortcut(Qt::Key_F2);
    connect(rename, SIGNAL(triggered()), this, SLOT(renameIt()));

    m_tags->setMinimumSize(
        m_tags->sizeHint().width() * 2,
        m_tagBox->sizeHint().height() + m_stateBox->sizeHint().height()
    );

    if (addNewTag)
        QTimer::singleShot(0, this, SLOT(newTag()));
    else
        // Once the window initial size is computed and the window show, allow the user to resize it down:
        QTimer::singleShot(0, this, SLOT(resetTreeSizeHint()));
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
    QTreeWidgetItemIterator it(m_tags);
    while (*it) {
        QTreeWidgetItem *item = *it;

        // Return if we found the tag item:
        TagListViewItem *tagItem = (TagListViewItem*)item;
        if (tagItem->tagCopy() && tagItem->tagCopy()->oldTag && tagItem->tagCopy()->stateCopies[0]->oldState == state)
            return tagItem;

        // Browser all sub-states:
        QTreeWidgetItemIterator it2(item);
        while (*it2) {
            QTreeWidgetItem *subItem = *it2;

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
    newTagCopy->stateCopies[0]->newState->setId("tag_state_" + QString::number(Tag::getNextStateUid()));   //TODO: Check if it's really unique
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

    // Add to the "controller":
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
    tagItem->setExpanded(true); // Show sub-states if we're adding them for the first time!

    State *firstState = tagItem->tagCopy()->stateCopies[0]->newState;

    // Add the first state to the "view". From now on, it's a multi-state tag:
    if (tagItem->childCount() <= 0) {
        firstState->setName(tagItem->tagCopy()->newTag->name());
        // Force emblem to exists for multi-state tags:
        if (firstState->emblem().isEmpty())
            firstState->setEmblem("empty");
        new TagListViewItem(tagItem, tagItem->tagCopy()->stateCopies[0]);
    }

    // Add to the "model":
    StateCopy *newStateCopy = new StateCopy();
    firstState->copyTo(newStateCopy->newState);
    newStateCopy->newState->setId("tag_state_" + QString::number(Tag::getNextStateUid()));   //TODO: Check if it's really unique
    newStateCopy->newState->setName(""); // We copied it too but it's likely the name will not be the same
    tagItem->tagCopy()->stateCopies.append(newStateCopy);
    m_addedStates.append(newStateCopy->newState);

    // Add to the "view":
    TagListViewItem *item = new TagListViewItem(tagItem, tagItem->lastChild(), newStateCopy);

    // Add to the "controller":
    m_tags->setCurrentItem(item);
    currentItemChanged(item);
    m_stateName->setFocus();
}

void TagsEditDialog::moveUp()
{
    if (!m_moveUp->isEnabled()) // Trggered by keyboard shortcut
        return;

    TagListViewItem *tagItem     = m_tags->currentItem();

    // Move in the list view:
    int idx = 0;
    QList<QTreeWidgetItem *> children;
    if (tagItem->parent()) {
        TagListViewItem* parentItem = tagItem->parent();
        idx = parentItem->indexOfChild(tagItem);
        if (idx > 0) {
            tagItem = (TagListViewItem *)parentItem->takeChild(idx);
            children = tagItem->takeChildren();
            parentItem->insertChild(idx - 1, tagItem);
            tagItem->insertChildren(0, children);
            tagItem->setExpanded(true);
        }
    } else {
        idx = m_tags->indexOfTopLevelItem(tagItem);
        if (idx > 0) {
            tagItem = (TagListViewItem *)m_tags->takeTopLevelItem(idx);
            children = tagItem->takeChildren();
            m_tags->insertTopLevelItem(idx - 1, tagItem);
            tagItem->insertChildren(0, children);
            tagItem->setExpanded(true);
        }
    }

    m_tags->setCurrentItem(tagItem);

    // Move in the value list:
    if (tagItem->tagCopy()) {
        int pos = m_tagCopies.indexOf(tagItem->tagCopy());
        m_tagCopies.removeAll(tagItem->tagCopy());
        int i = 0;
        for (TagCopy::List::iterator it = m_tagCopies.begin(); it != m_tagCopies.end(); ++it, ++i)
            if (i == pos - 1) {
                m_tagCopies.insert(it, tagItem->tagCopy());
                break;
            }
    } else {
        StateCopy::List &stateCopies = ((TagListViewItem*)(tagItem->parent()))->tagCopy()->stateCopies;
        int pos = stateCopies.indexOf(tagItem->stateCopy());
        stateCopies.removeAll(tagItem->stateCopy());
        int i = 0;
        for (StateCopy::List::iterator it = stateCopies.begin(); it != stateCopies.end(); ++it, ++i)
            if (i == pos - 1) {
                stateCopies.insert(it, tagItem->stateCopy());
                break;
            }
    }

    ensureCurrentItemVisible();

    m_moveDown->setEnabled(tagItem->nextSibling() != 0);
    m_moveUp->setEnabled(tagItem->prevSibling() != 0);
}

void TagsEditDialog::moveDown()
{
    if (!m_moveDown->isEnabled()) // Trggered by keyboard shortcut
        return;

    TagListViewItem *tagItem = m_tags->currentItem();

    // Move in the list view:
    int idx = 0;
    QList<QTreeWidgetItem *> children;
    if (tagItem->parent()) {
        TagListViewItem* parentItem = tagItem->parent();
        idx = parentItem->indexOfChild(tagItem);
        if (idx < parentItem->childCount() - 1) {
            children = tagItem->takeChildren();
            tagItem = (TagListViewItem *)parentItem->takeChild(idx);
            parentItem->insertChild(idx + 1, tagItem);
            tagItem->insertChildren(0, children);
            tagItem->setExpanded(true);
        }
    } else {
        idx = m_tags->indexOfTopLevelItem(tagItem);
        if (idx < m_tags->topLevelItemCount() - 1) {
            children = tagItem->takeChildren();
            tagItem = (TagListViewItem *)m_tags->takeTopLevelItem(idx);
            m_tags->insertTopLevelItem(idx + 1, tagItem);
            tagItem->insertChildren(0, children);
            tagItem->setExpanded(true);
        }
    }

    m_tags->setCurrentItem(tagItem);

    // Move in the value list:
    if (tagItem->tagCopy()) {
        uint pos = m_tagCopies.indexOf(tagItem->tagCopy());
        m_tagCopies.removeAll(tagItem->tagCopy());
        if (pos == (uint)m_tagCopies.count() - 1) // Insert at end: iterator does not go there
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
        StateCopy::List &stateCopies = ((TagListViewItem*)(tagItem->parent()))->tagCopy()->stateCopies;
        uint pos = stateCopies.indexOf(tagItem->stateCopy());
        stateCopies.removeAll(tagItem->stateCopy());
        if (pos == (uint)stateCopies.count() - 1) // Insert at end: iterator does not go there
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

    m_moveDown->setEnabled(tagItem->nextSibling() != 0);
    m_moveUp->setEnabled(tagItem->prevSibling() != 0);
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
                     KGuiItem(i18n("Delete Tag"), "edit-delete")
                 );
    else if (item->stateCopy() && item->stateCopy()->oldState)
        result = KMessageBox::warningContinueCancel(
                     this,
                     i18n("Deleting the state will remove the tag from every note the state is currently assigned to."),
                     i18n("Confirm Delete State"),
                     KGuiItem(i18n("Delete State"), "edit-delete")
                 );
    if (result != KMessageBox::Continue)
        return;

    if (item->tagCopy()) {
        StateCopy::List stateCopies = item->tagCopy()->stateCopies;
        for (StateCopy::List::iterator stateCopyIt = stateCopies.begin(); stateCopyIt != stateCopies.end(); ++stateCopyIt) {
            StateCopy *stateCopy = *stateCopyIt;
            if (stateCopy->oldState) {
                m_deletedStates.append(stateCopy->oldState);
                m_addedStates.removeAll(stateCopy->oldState);
            }
            m_addedStates.removeAll(stateCopy->newState);
        }
        m_tagCopies.removeAll(item->tagCopy());
        // Remove the new tag, to avoid keyboard-shortcut clashes:
        delete item->tagCopy()->newTag;
    } else {
        TagListViewItem *parentItem = item->parent();
        // Remove the state:
        parentItem->tagCopy()->stateCopies.removeAll(item->stateCopy());
        if (item->stateCopy()->oldState) {
            m_deletedStates.append(item->stateCopy()->oldState);
            m_addedStates.removeAll(item->stateCopy()->oldState);
        }
        m_addedStates.removeAll(item->stateCopy()->newState);
        delete item;
        item = 0;
        // Transform to single-state tag if needed:
        if (parentItem->childCount() == 1) {
            delete parentItem->child(0);
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
    Q_UNUSED(shortcut);
    // TODO: Validate it!
    //m_shortcut->setShortcut(shortcut);
}

void TagsEditDialog::removeShortcut()
{
    //m_shortcut->setShortcut(KShortcut());
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

    m_removeShortcut->setEnabled(!m_shortcut->shortcut().isEmpty());
    m_removeEmblem->setEnabled(!m_emblem->icon().isEmpty() && !m_tags->currentItem()->isEmblemObligatory());
    m_onEveryLines->setEnabled(!m_textEquivalent->text().isEmpty());
}

void TagsEditDialog::currentItemChanged(QTreeWidgetItem *item, QTreeWidgetItem* nextItem)
{
    Q_UNUSED(nextItem);
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
    m_tags->scrollToItem(tagItem);

    int idx = 0;

    if (tagItem->parent()) {
        idx = ((QTreeWidgetItem *)tagItem->parent())->indexOfChild(tagItem);
        m_moveDown->setEnabled(idx < ((QTreeWidgetItem *)tagItem->parent())->childCount());
    } else {
        idx = m_tags->indexOfTopLevelItem(tagItem);
        m_moveDown->setEnabled(idx < m_tags->topLevelItemCount());
    }

    m_moveUp->setEnabled(idx > 0);
}

void TagsEditDialog::loadBlankState()
{
    QFont defaultFont;
    m_stateName->setText("");
    m_emblem->resetIcon();
    m_removeEmblem->setEnabled(false);
    m_backgroundColor->setColor(QColor());
    m_bold->setChecked(false);
    m_underline->setChecked(false);
    m_italic->setChecked(false);
    m_strike->setChecked(false);
    m_textColor->setColor(QColor());
    //m_font->setCurrentIndex(0);
    m_font->setCurrentFont(defaultFont.family());
    m_fontSize->setCurrentIndex(0);
    m_textEquivalent->setText("");
    m_onEveryLines->setChecked(false);
    m_allowCrossRefernce->setChecked(false);
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
    m_bold->setChecked(state->bold());
    m_underline->setChecked(state->underline());
    m_italic->setChecked(state->italic());
    m_strike->setChecked(state->strikeOut());
    m_textColor->setColor(state->textColor());
    m_textEquivalent->setText(state->textEquivalent());
    m_onEveryLines->setChecked(state->onAllTextLines());
    m_allowCrossRefernce->setChecked(state->allowCrossReferences());

    QFont defaultFont;
    if (state->fontName().isEmpty())
        m_font->setCurrentFont(defaultFont.family() );
    else
        m_font->setCurrentFont(state->fontName());

    if (state->fontSize() == -1)
        m_fontSize->setCurrentIndex(0);
    else
        m_fontSize->setItemText(m_fontSize->currentIndex(), QString::number(state->fontSize()));
}

void TagsEditDialog::loadTagFrom(Tag *tag)
{
    m_tagName->setText(tag->name());
    m_shortcut->setShortcut(tag->shortcut());
    m_removeShortcut->setEnabled(!tag->shortcut().isEmpty());
    m_inherit->setChecked(tag->inheritedBySiblings());
}

void TagsEditDialog::saveStateTo(State *state)
{
    state->setName(m_stateName->text());
    state->setEmblem(m_emblem->icon());
    state->setBackgroundColor(m_backgroundColor->color());
    state->setBold(m_bold->isChecked());
    state->setUnderline(m_underline->isChecked());
    state->setItalic(m_italic->isChecked());
    state->setStrikeOut(m_strike->isChecked());
    state->setTextColor(m_textColor->color());
    state->setTextEquivalent(m_textEquivalent->text());
    state->setOnAllTextLines(m_onEveryLines->isChecked());
    state->setAllowCrossReferences(m_allowCrossRefernce->isChecked());

    if (m_font->currentIndex() == 0)
        state->setFontName("");
    else
        state->setFontName(m_font->currentFont().family());

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
        delete(*tagCopyIt)->newTag;
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

void TagListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                            const QModelIndex &index) const
{
    TagListViewItem* thisItem  = qvariant_cast<TagListViewItem*>(index.data());
//    kDebug() << "Pointer is: " << thisItem << endl;
    QItemDelegate::paint(painter, option, index);
}

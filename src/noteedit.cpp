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

#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kicondialog.h>
#include <kcolordialog.h>
#include <kservice.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <ktoolbar.h>
#include <kaction.h>
#include <kurifilter.h>

#include "noteedit.h"
#include "notecontent.h"
#include "notefactory.h" // Use Tools::
#include "note.h"
#include "basket.h"
#include "settings.h"
#include "tools.h"
#include "variouswidgets.h"
#include "focusedwidgets.h"

#include <iostream>

/** class NoteEditor: */

NoteEditor::NoteEditor(NoteContent *noteContent)
{
	m_isEmpty  = false;
	m_canceled = false;
	m_widget   = 0;
	m_textEdit = 0;
	m_lineEdit = 0;
	m_noteContent = noteContent;
}

Note* NoteEditor::note()
{
	return m_noteContent->note();
}

NoteEditor* NoteEditor::editNoteContent(NoteContent *noteContent, QWidget *parent)
{
	TextContent *textContent = dynamic_cast<TextContent*>(noteContent);
	if (textContent)
		return new TextEditor(textContent, parent);

	HtmlContent *htmlContent = dynamic_cast<HtmlContent*>(noteContent);
	if (htmlContent)
		return new HtmlEditor(htmlContent, parent);

	ImageContent *imageContent = dynamic_cast<ImageContent*>(noteContent);
	if (imageContent)
		return new ImageEditor(imageContent, parent);

	AnimationContent *animationContent = dynamic_cast<AnimationContent*>(noteContent);
	if (animationContent)
		return new AnimationEditor(animationContent, parent);

	FileContent *fileContent = dynamic_cast<FileContent*>(noteContent); // Same for SoundContent
	if (fileContent)
		return new FileEditor(fileContent, parent);

	LinkContent *linkContent = dynamic_cast<LinkContent*>(noteContent);
	if (linkContent)
		return new LinkEditor(linkContent, parent);

	LauncherContent *launcherContent = dynamic_cast<LauncherContent*>(noteContent);
	if (launcherContent)
		return new LauncherEditor(launcherContent, parent);

	ColorContent *colorContent = dynamic_cast<ColorContent*>(noteContent);
	if (colorContent)
		return new ColorEditor(colorContent, parent);

	UnknownContent *unknownContent = dynamic_cast<UnknownContent*>(noteContent);
	if (unknownContent)
		return new UnknownEditor(unknownContent, parent);

	return 0;
}

void NoteEditor::setInlineEditor(QWidget *inlineEditor)
{
	m_widget   = inlineEditor;
	m_textEdit = 0;
	m_lineEdit = 0;

	KTextEdit *textEdit = dynamic_cast<KTextEdit*>(inlineEditor);
	if (textEdit)
		m_textEdit = textEdit;
	else {
		QLineEdit *lineEdit = dynamic_cast<QLineEdit*>(inlineEditor);
		if (lineEdit)
			m_lineEdit = lineEdit;
	}
}

#include <iostream>

/** Several possible scenarii to leave or not the edit mode:
  * - The user click on the basket, the edit widget then have a focusOut() event.
  *   The new focused widget (the basket) is not in the m_secondaryInlineWidgets list, so we leave the mode
  * - The user click on the font or color combobox of the rich text edit: the edit widget then have a focusOut() event.
  *   But the new focused widget is in the m_secondaryInlineWidgets list, so we do nothing.
  *   Then the user can reswitch focus to the edit widget or click the basket.
  *   The font or color combobox will then also send focusOut() event, this will be threated here like the event from the editbox.
  * - The user click another window from another application.
  *   kapp->focusWidget() and kapp->activeWindow() are both equal to 0 because out of our application.
  *   We close the editor. But only after a small laps of time: see below.
  * - The user click "Custom..." in the color combobox. A new KColorDialog is open.
  *   But kapp->focusWidget() and kapp->activeWindow() are both equal to 0 for a short laps of time.
  *   After the window is created and displayed, those values are then different from 0.
  *   To avoid to close the editor in this case (the application could believe the focus is now in another application, like in the previous case),
  *   we temporize and then re-test a second time: if the active window is a KColorDialog, it's OK.
  */
void NoteEditor::slotFocusOut()
{
	QValueList<QWidget*> inlineWidgets = m_secondaryInlineWidgets;
	inlineWidgets.append(widget());

	QWidget *focusWidget      = kapp->focusWidget();
	QObject *focusParent      = (focusWidget ? focusWidget->parent() : 0);
	QObject *focusGrandParent = (focusParent ? focusParent->parent() : 0);

//	std::cout << "------" << std::endl;
//	if (focusWidget)
//		std::cout << "FOCUS: " << focusWidget->className() << std::endl;
//	if (focusParent)
//		std::cout << "FOCUS PARENT: " << focusParent->className() << std::endl;
//	if (focusGrandParent)
//		std::cout << "FOCUS GRAND PARENT: " << focusGrandParent->className() << std::endl;

	for (QValueList<QWidget*>::iterator it = inlineWidgets.begin(); it != inlineWidgets.end(); ++it) {
//		std::cout << (*it)->className() << std::endl;
		// If current focused widget is this KTextEdit or the popup menu of the KTextEdit, do not consider that as a focus loss:
		if ( focusWidget == *it || (focusParent && focusParent == *it) || (focusGrandParent && focusGrandParent == *it) )
			return;
	}

	// When color dialog is called, focusWidget == 0L, but also when another application widget stealed focus. We need to differenciate those two cases:
	if (!focusWidget && !kapp->activeWindow())
		QTimer::singleShot(100, this, SLOT(slotFocusOut2()));
	else
		emit askValidation();
}

void NoteEditor::slotFocusOut2()
{
	if (!kapp->activeWindow())
		emit askValidation();
}

/** class TextEditor: */

TextEditor::TextEditor(TextContent *textContent, QWidget *parent)
 : NoteEditor(textContent), m_textContent(textContent)
{
	FocusedTextEdit *textEdit = new FocusedTextEdit(Settings::enterValidateInline(), /*disableUpdatesOnKeyPress=*/true, parent);
	textEdit->setLineWidth(0);
	textEdit->setMidLineWidth(0);
	textEdit->setTextFormat(Qt::PlainText);
	textEdit->setPaletteBackgroundColor(note()->backgroundColor());
	textEdit->setPaletteForegroundColor(note()->textColor());
	textEdit->setFont(note()->font());
	textEdit->setHScrollBarMode(QScrollView::AlwaysOff);
	if (Settings::spellCheckTextNotes())
		textEdit->setCheckSpellingEnabled(true);
	textEdit->setText(m_textContent->text());
	textEdit->moveCursor(KTextEdit::MoveEnd, false); // FIXME: Sometimes, the cursor flicker at ends before being positionned where clicked (because kapp->processEvents() I think)
	textEdit->verticalScrollBar()->setCursor(Qt::ArrowCursor);
	setInlineEditor(textEdit);
	connect( textEdit, SIGNAL(escapePressed()), this, SIGNAL(askValidation()) );
	connect( textEdit, SIGNAL(focusOut()),      this, SLOT(slotFocusOut())    );
}

TextEditor::~TextEditor()
{
	delete widget(); // TODO: delete that in validate(), so we can remove one method
}

void TextEditor::autoSave()
{
	if (Settings::spellCheckTextNotes() != textEdit()->checkSpellingEnabled()) {
		Settings::setSpellCheckTextNotes(textEdit()->checkSpellingEnabled());
		Settings::saveConfig();
	}

	bool autoSpellCheck = textEdit()->checkSpellingEnabled();
	textEdit()->setCheckSpellingEnabled(false);
//	if (textEdit()->text().isEmpty())
//		setEmpty();
	m_textContent->setText(textEdit()->text());
	m_textContent->saveToFile();
	m_textContent->setEdited();

//	delete widget();

	textEdit()->setCheckSpellingEnabled(autoSpellCheck);
}

void TextEditor::validate()
{
	if (Settings::spellCheckTextNotes() != textEdit()->checkSpellingEnabled()) {
		Settings::setSpellCheckTextNotes(textEdit()->checkSpellingEnabled());
		Settings::saveConfig();
	}

	textEdit()->setCheckSpellingEnabled(false);
	if (textEdit()->text().isEmpty())
		setEmpty();
	m_textContent->setText(textEdit()->text());
	m_textContent->saveToFile();
	m_textContent->setEdited();

//	delete widget();
}

/** class HtmlEditor: */

HtmlEditor::HtmlEditor(HtmlContent *htmlContent, QWidget *parent)
 : NoteEditor(htmlContent), m_htmlContent(htmlContent)
{
	FocusedTextEdit *textEdit = new FocusedTextEdit(Settings::enterValidateInline(), /*disableUpdatesOnKeyPress=*/true, parent);
	textEdit->setLineWidth(0);
	textEdit->setMidLineWidth(0);
	textEdit->setTextFormat(Qt::RichText);
	textEdit->setPaletteBackgroundColor(note()->backgroundColor());
	textEdit->setPaletteForegroundColor(note()->textColor());
	textEdit->setFont(note()->font());
	textEdit->setHScrollBarMode(QScrollView::AlwaysOff);
	textEdit->setText(m_htmlContent->html());
	textEdit->moveCursor(KTextEdit::MoveEnd, false);
	textEdit->verticalScrollBar()->setCursor(Qt::ArrowCursor);
	setInlineEditor(textEdit);

	QValueList<QWidget*> widgets;
	widgets.append(InlineEditors::instance()->richTextFont);
	widgets.append(InlineEditors::instance()->richTextColor);
	widgets.append(InlineEditors::instance()->richTextToolBar());
	setSecondaryInlineWidgets(widgets);

	connect( textEdit,                                 SIGNAL(escapePressed()), this, SIGNAL(askValidation()) );
	connect( textEdit,                                 SIGNAL(focusOut()),      this, SLOT(slotFocusOut())    );
//	connect( InlineEditors::instance()->richTextFont,  SIGNAL(focusOut()),      this, SLOT(slotFocusOut())    );
//	connect( InlineEditors::instance()->richTextColor, SIGNAL(focusOut()),      this, SLOT(slotFocusOut())    );

	connect( InlineEditors::instance()->richTextFont,  SIGNAL(textChanged(const QString&)), textEdit, SLOT(setFamily(const QString&)) );
	connect( InlineEditors::instance()->richTextColor, SIGNAL(activated(const QColor&)),    textEdit, SLOT(setColor(const QColor&))   );

	connect( textEdit,  SIGNAL(cursorPositionChanged(int, int)), this, SLOT(cursorPositionChanged()) );
	connect( textEdit,  SIGNAL(clicked(int, int)),               this, SLOT(cursorPositionChanged()) );
//	connect( textEdit,  SIGNAL(currentVerticalAlignmentChanged(VerticalAlignment)), this, SLOT(slotVerticalAlignmentChanged()) );

	connect( InlineEditors::instance()->richTextBold,      SIGNAL(activated()), this, SLOT(setBold())      );
	connect( InlineEditors::instance()->richTextItalic,    SIGNAL(activated()), this, SLOT(setItalic())    );
	connect( InlineEditors::instance()->richTextUnderline, SIGNAL(activated()), this, SLOT(setUnderline()) );
	connect( InlineEditors::instance()->richTextLeft,      SIGNAL(activated()), this, SLOT(setLeft())      );
	connect( InlineEditors::instance()->richTextCenter,    SIGNAL(activated()), this, SLOT(setCentered())  );
	connect( InlineEditors::instance()->richTextRight,     SIGNAL(activated()), this, SLOT(setRight())     );
	connect( InlineEditors::instance()->richTextJustified, SIGNAL(activated()), this, SLOT(setBlock())     );

	InlineEditors::instance()->richTextToolBar()->show();
	cursorPositionChanged();
	//QTimer::singleShot( 0, this, SLOT(cursorPositionChanged()) );
	InlineEditors::instance()->enableRichTextToolBar();
}

void HtmlEditor::cursorPositionChanged()
{
	InlineEditors::instance()->richTextFont->setCurrentFont(  textEdit()->currentFont().family() );
	if (InlineEditors::instance()->richTextColor->color() != textEdit()->color())
		InlineEditors::instance()->richTextColor->setColor(   textEdit()->color()                );
	InlineEditors::instance()->richTextBold->setChecked(      textEdit()->bold()                 );
	InlineEditors::instance()->richTextItalic->setChecked(    textEdit()->italic()               );
	InlineEditors::instance()->richTextUnderline->setChecked( textEdit()->underline()            );

	switch (textEdit()->alignment()) {
		default:
		case 1/*Qt::AlignLeft*/:     InlineEditors::instance()->richTextLeft->setChecked(true);      break;
		case 4/*Qt::AlignCenter*/:   InlineEditors::instance()->richTextCenter->setChecked(true);    break;
		case 2/*Qt::AlignRight*/:    InlineEditors::instance()->richTextRight->setChecked(true);     break;
		case -8/*Qt::AlignJustify*/: InlineEditors::instance()->richTextJustified->setChecked(true); break;
	}
}

/*void HtmlEditor::slotVe<rticalAlignmentChanged(QTextEdit::VerticalAlignment align)
{
	QTextEdit::VerticalAlignment align = textEdit()
	switch (align) {
		case KTextEdit::AlignSuperScript:
			InlineEditors::instance()->richTextSuper->setChecked(true);
			InlineEditors::instance()->richTextSub->setChecked(false);
			break;
		case KTextEdit::AlignSubScript:
			InlineEditors::instance()->richTextSuper->setChecked(false);
			InlineEditors::instance()->richTextSub->setChecked(true);
			break;
		default:
			InlineEditors::instance()->richTextSuper->setChecked(false);
			InlineEditors::instance()->richTextSub->setChecked(false);
	}

	NoteHtmlEditor::buttonToggled(int id) :
		case 106:
			if (isOn && m_toolbar->isButtonOn(107))
				m_toolbar->setButton(107, false);
			m_text->setVerticalAlignment(isOn ? KTextEdit::AlignSuperScript : KTextEdit::AlignNormal);
			break;
		case 107:
			if (isOn && m_toolbar->isButtonOn(106))
				m_toolbar->setButton(106, false);
			m_text->setVerticalAlignment(isOn ? KTextEdit::AlignSubScript   : KTextEdit::AlignNormal);
			break;
}*/

void HtmlEditor::setBold()      { textEdit()->setBold(      InlineEditors::instance()->richTextBold->isChecked()      ); }
void HtmlEditor::setItalic()    { textEdit()->setItalic(    InlineEditors::instance()->richTextItalic->isChecked()    ); }
void HtmlEditor::setUnderline() { textEdit()->setUnderline( InlineEditors::instance()->richTextUnderline->isChecked() ); }
void HtmlEditor::setLeft()      { textEdit()->setAlignment(Qt::AlignLeft);    }
void HtmlEditor::setCentered()  { textEdit()->setAlignment(Qt::AlignCenter);  }
void HtmlEditor::setRight()     { textEdit()->setAlignment(Qt::AlignRight);   }
void HtmlEditor::setBlock()     { textEdit()->setAlignment(Qt::AlignJustify); }

HtmlEditor::~HtmlEditor()
{
	delete widget();
}

void HtmlEditor::autoSave()
{
	m_htmlContent->setHtml(textEdit()->text());
	m_htmlContent->saveToFile();
	m_htmlContent->setEdited();
}

void HtmlEditor::validate()
{
	if (Tools::htmlToText(textEdit()->text()).isEmpty())
		setEmpty();
	m_htmlContent->setHtml(textEdit()->text());
	m_htmlContent->saveToFile();
	m_htmlContent->setEdited();

	disconnect();
	widget()->disconnect();
	InlineEditors::instance()->disableRichTextToolBar();

	InlineEditors::instance()->richTextToolBar()->hide();
	delete widget();
	setInlineEditor(0);
}

/** class ImageEditor: */

ImageEditor::ImageEditor(ImageContent *imageContent, QWidget *parent)
 : NoteEditor(imageContent)
{
	int choice = KMessageBox::questionYesNo(parent, i18n(
		"Images can not be edited here at the moment (the next version of BasKet Note Pads will include an image editor).\n"
		"Do you want to open it with an application that understands it?"),
		i18n("Edit Image Note"),
		KStdGuiItem::open(),
		KStdGuiItem::cancel());

	if (choice == KMessageBox::Yes)
		note()->basket()->noteOpen(note());
}

/** class AnimationEditor: */

AnimationEditor::AnimationEditor(AnimationContent *animationContent, QWidget *parent)
 : NoteEditor(animationContent)
{
	int choice = KMessageBox::questionYesNo(parent, i18n(
		"This animated image can not be edited here.\n"
		"Do you want to open it with an application that understands it?"),
		i18n("Edit Animation Note"),
		KStdGuiItem::open(),
		KStdGuiItem::cancel());

	if (choice == KMessageBox::Yes)
		note()->basket()->noteOpen(note());
}

/** class FileEditor: */

FileEditor::FileEditor(FileContent *fileContent, QWidget *parent)
 : NoteEditor(fileContent), m_fileContent(fileContent)
{
	FocusedLineEdit *lineEdit = new FocusedLineEdit(parent);
	lineEdit->setLineWidth(0);
	lineEdit->setMidLineWidth(0);
	lineEdit->setPaletteBackgroundColor(note()->backgroundColor());
	lineEdit->setPaletteForegroundColor(note()->textColor());
	lineEdit->setFont(note()->font());
	lineEdit->setText(m_fileContent->fileName());
	lineEdit->selectAll();
	setInlineEditor(lineEdit);
	connect( lineEdit, SIGNAL(returnPressed()), this, SIGNAL(askValidation()) );
	connect( lineEdit, SIGNAL(escapePressed()), this, SIGNAL(askValidation()) );
//	connect( lineEdit, SIGNAL(focusOut()),      this, SLOT(slotFocusOut())    );
}

FileEditor::~FileEditor()
{
	delete widget();
}

void FileEditor::autoSave()
{
	// FIXME: How to detect cancel?
	if (!lineEdit()->text().isEmpty() && m_fileContent->trySetFileName(lineEdit()->text())) {
		m_fileContent->setFileName(lineEdit()->text());
		m_fileContent->setEdited();
	}
}

void FileEditor::validate()
{
	autoSave();
}

/** class LinkEditor: */

LinkEditor::LinkEditor(LinkContent *linkContent, QWidget *parent)
 : NoteEditor(linkContent)
{
	LinkEditDialog dialog(linkContent, parent);
	if (dialog.exec() == QDialog::Rejected)
		cancel();
	if (linkContent->url().isEmpty() && linkContent->title().isEmpty())
		setEmpty();
}

/** class LauncherEditor: */

LauncherEditor::LauncherEditor(LauncherContent *launcherContent, QWidget *parent)
 : NoteEditor(launcherContent)
{
	LauncherEditDialog dialog(launcherContent, parent);
	if (dialog.exec() == QDialog::Rejected)
		cancel();
	if (launcherContent->name().isEmpty() && launcherContent->exec().isEmpty())
		setEmpty();
}

/** class ColorEditor: */

ColorEditor::ColorEditor(ColorContent *colorContent, QWidget *parent)
 : NoteEditor(colorContent)
{
	KColorDialog dialog(parent, /*name=*/0, /*modal=*/true);
	dialog.setColor(colorContent->color());
	dialog.setCaption(i18n("Edit Color Note"));
	if (dialog.exec() == QDialog::Accepted) {
		if (dialog.color() != colorContent->color()) {
			colorContent->setColor(dialog.color());
			colorContent->setEdited();
		}
	} else
		cancel();

	/* This code don't allow to set a caption to the dialog:
	QColor color = colorContent()->color();
	if (KColorDialog::getColor(color, parent) == QDialog::Accepted && color != m_color) {
		colorContent()->setColor(color);
		setEdited();
	}*/
}

/** class UnknownEditor: */

UnknownEditor::UnknownEditor(UnknownContent *unknownContent, QWidget *parent)
 : NoteEditor(unknownContent)
{
	KMessageBox::information(parent, i18n(
		"The type of this note is unknown and can not be edited here.\n"
		"You however can drag or copy the note into an application that understands it."),
		i18n("Edit Unknown Note"));
}

/*********************************************************************/


/** class DebuggedLineEdit: */

DebuggedLineEdit::DebuggedLineEdit(const QString &text, QWidget *parent)
 : QLineEdit(text, parent)
{
}

DebuggedLineEdit::~DebuggedLineEdit()
{
}

void DebuggedLineEdit::keyPressEvent(QKeyEvent *event)
{
	QString oldText = text();
	QLineEdit::keyPressEvent(event);
	if (oldText != text())
		emit textChanged(text());
}


/** class LinkEditDialog: */

LinkEditDialog::LinkEditDialog(LinkContent *contentNote, QWidget *parent/*, QKeyEvent *ke*/)
 : KDialogBase(KDialogBase::Plain, i18n("Edit Link Note"), KDialogBase::Ok | KDialogBase::Cancel,
               KDialogBase::Ok, parent, /*name=*/0, /*modal=*/true, /*separator=*/true),
   m_noteContent(contentNote)
{
	QWidget     *page   = plainPage();
	QGridLayout *layout = new QGridLayout(page, /*nRows=*/4, /*nCols=*/2, /*margin=*/0, spacingHint());

	m_url = new KURLRequester(m_noteContent->url().url(), page);

	QWidget *wid1 = new QWidget(page);
	QHBoxLayout *titleLay = new QHBoxLayout(wid1, /*margin=*/0, spacingHint());
	m_title = new DebuggedLineEdit(m_noteContent->title(), wid1);
	m_autoTitle = new QPushButton(i18n("Auto"), wid1);
	m_autoTitle->setToggleButton(true);
	m_autoTitle->setOn(m_noteContent->autoTitle());
	titleLay->addWidget(m_title);
	titleLay->addWidget(m_autoTitle);

	QWidget *wid = new QWidget(page);
	QHBoxLayout *hLay = new QHBoxLayout(wid, /*margin=*/0, spacingHint());
	m_icon = new KIconButton(wid);
	QLabel *label3 = new QLabel(m_icon, i18n("&Icon:"), page);
	KURL filteredURL = NoteFactory::filteredURL(KURL(m_url->lineEdit()->text()));//KURIFilter::self()->filteredURI(KURL(m_url->lineEdit()->text()));
	m_icon->setIconType(KIcon::NoGroup, KIcon::MimeType);
	m_icon->setIconSize(LinkLook::lookForURL(filteredURL)->iconSize());
	m_autoIcon = new QPushButton(i18n("Auto"), wid); // Create before to know size here:
	/* Icon button: */
	m_icon->setIcon(m_noteContent->icon());
	int minSize = m_autoIcon->sizeHint().height();
	// Make the icon button at least the same heigh than the other buttons for a better alignment (nicer to the eyes):
	if (m_icon->sizeHint().height() < minSize)
		m_icon->setFixedSize(minSize, minSize);
	else
		m_icon->setFixedSize(m_icon->sizeHint().height(), m_icon->sizeHint().height()); // Make it square
	/* Auto button: */
	m_autoIcon->setToggleButton(true);
	m_autoIcon->setOn(m_noteContent->autoIcon());
	hLay->addWidget(m_icon);
	hLay->addWidget(m_autoIcon);
	hLay->addStretch();

	m_url->lineEdit()->setMinimumWidth(m_url->lineEdit()->fontMetrics().maxWidth()*20);
	m_title->setMinimumWidth(m_title->fontMetrics().maxWidth()*20);

	//m_url->setShowLocalProtocol(true);
	QLabel      *label1 = new QLabel(m_url,   i18n("Ta&rget:"),    page);
	QLabel      *label2 = new QLabel(m_title, i18n("&Title:"),  page);
	layout->addWidget(label1,  0, 0, Qt::AlignVCenter);
	layout->addWidget(label2,  1, 0, Qt::AlignVCenter);
	layout->addWidget(label3,  2, 0, Qt::AlignVCenter);
	layout->addWidget(m_url,   0, 1, Qt::AlignVCenter);
	layout->addWidget(wid1,    1, 1, Qt::AlignVCenter);
	layout->addWidget(wid,     2, 1, Qt::AlignVCenter);

	m_isAutoModified = false;
	connect( m_url,   SIGNAL(textChanged(const QString&)), this, SLOT(urlChanged(const QString&))     );
	connect( m_title, SIGNAL(textChanged(const QString&)), this, SLOT(doNotAutoTitle(const QString&)) );
	connect( m_icon,  SIGNAL(iconChanged(QString))       , this, SLOT(doNotAutoIcon(QString))         );
	connect( m_autoTitle, SIGNAL(clicked()), this, SLOT(guessTitle()) );
	connect( m_autoIcon,  SIGNAL(clicked()), this, SLOT(guessIcon())  );

	QWidget *stretchWidget = new QWidget(page);
	stretchWidget->setSizePolicy(QSizePolicy(/*hor=*/QSizePolicy::Fixed, /*ver=*/QSizePolicy::Expanding, /*hStretch=*/1, /*vStretch=*/5000)); // Make it fill ALL vertical space
	layout->addWidget(stretchWidget, 3, 1, Qt::AlignVCenter);


	urlChanged("");

//	if (ke)
//		kapp->postEvent(m_url->lineEdit(), ke);
}

LinkEditDialog::~LinkEditDialog()
{
}

void LinkEditDialog::polish()
{
	KDialogBase::polish();
	if (m_url->lineEdit()->text().isEmpty()) {
		m_url->setFocus();
		m_url->lineEdit()->end(false);
	} else {
		m_title->setFocus();
		m_title->end(false);
	}
}


void LinkEditDialog::urlChanged(const QString&)
{
	m_isAutoModified = true;
//	guessTitle();
//	guessIcon();
	// Optimization (filter only once):
	KURL filteredURL = NoteFactory::filteredURL(KURL(m_url->url()));//KURIFilter::self()->filteredURI(KURL(m_url->url()));
	if (m_autoIcon->isOn())
		m_icon->setIcon(NoteFactory::iconForURL(filteredURL));
	if (m_autoTitle->isOn()) {
		m_title->setText(NoteFactory::titleForURL(filteredURL));
		m_autoTitle->setOn(true); // Because the setText() will disable it!
	}
}

void LinkEditDialog::doNotAutoTitle(const QString&)
{
	if (m_isAutoModified)
		m_isAutoModified = false;
	else
		m_autoTitle->setOn(false);
}

void LinkEditDialog::doNotAutoIcon(QString)
{
	m_autoIcon->setOn(false);
}

void LinkEditDialog::guessIcon()
{
	if (m_autoIcon->isOn()) {
		KURL filteredURL = NoteFactory::filteredURL(KURL(m_url->url()));//KURIFilter::self()->filteredURI(KURL(m_url->url()));
		m_icon->setIcon(NoteFactory::iconForURL(filteredURL));
	}
}

void LinkEditDialog::guessTitle()
{
	if (m_autoTitle->isOn()) {
		KURL filteredURL = NoteFactory::filteredURL(KURL(m_url->url()));//KURIFilter::self()->filteredURI(KURL(m_url->url()));
		m_title->setText(NoteFactory::titleForURL(filteredURL));
		m_autoTitle->setOn(true); // Because the setText() will disable it!
	}
}

void LinkEditDialog::slotOk()
{
	KURL filteredURL = NoteFactory::filteredURL(KURL(m_url->url()));//KURIFilter::self()->filteredURI(KURL(m_url->url()));
	m_noteContent->setLink(filteredURL, m_title->text(), m_icon->icon(), m_autoTitle->isOn(), m_autoIcon->isOn());
	m_noteContent->setEdited();

	/* Change icon size if link look have changed */
	LinkLook *linkLook = LinkLook::lookForURL(filteredURL);
	QString icon = m_icon->icon();             // When we change size, icon isn't changed and keep it's old size
	m_icon->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum); // Reset size policy
	m_icon->setIconSize(linkLook->iconSize()); //  So I store it's name and reload it after size change !
	m_icon->setIcon(icon);
	int minSize = m_autoIcon->sizeHint().height();
	// Make the icon button at least the same heigh than the other buttons for a better alignment (nicer to the eyes):
	if (m_icon->sizeHint().height() < minSize)
		m_icon->setFixedSize(minSize, minSize);
	else
		m_icon->setFixedSize(m_icon->sizeHint().height(), m_icon->sizeHint().height()); // Make it square

	KDialogBase::slotOk();
}

/** class LauncherEditDialog: */

LauncherEditDialog::LauncherEditDialog(LauncherContent *contentNote, QWidget *parent)
 : KDialogBase(KDialogBase::Plain, i18n("Edit Launcher Note"), KDialogBase::Ok | KDialogBase::Cancel,
               KDialogBase::Ok, parent, /*name=*/0, /*modal=*/true, /*separator=*/true),
   m_noteContent(contentNote)
{
	QWidget     *page   = plainPage();
	QGridLayout *layout = new QGridLayout(page, /*nRows=*/4, /*nCols=*/2, /*margin=*/0, spacingHint());

	KService service(contentNote->fullPath());

	m_command = new RunCommandRequester(service.exec(), i18n("Choose a command to run:"), page);
	m_name    = new QLineEdit(service.name(), page);

	QWidget *wid = new QWidget(page);
	QHBoxLayout *hLay = new QHBoxLayout(wid, /*margin=*/0, spacingHint());
	m_icon = new KIconButton(wid);
	QLabel *label = new QLabel(m_icon, i18n("&Icon:"), page);
	m_icon->setIconType(KIcon::NoGroup, KIcon::Application);
	m_icon->setIconSize(LinkLook::launcherLook->iconSize());
	QPushButton *guessButton = new QPushButton(i18n("&Guess"), wid);
	/* Icon button: */
	m_icon->setIcon(service.icon());
	int minSize = guessButton->sizeHint().height();
	// Make the icon button at least the same heigh than the other buttons for a better alignment (nicer to the eyes):
	if (m_icon->sizeHint().height() < minSize)
		m_icon->setFixedSize(minSize, minSize);
	else
		m_icon->setFixedSize(m_icon->sizeHint().height(), m_icon->sizeHint().height()); // Make it square
	/* Guess button: */
	hLay->addWidget(m_icon);
	hLay->addWidget(guessButton);
	hLay->addStretch();

	m_command->lineEdit()->setMinimumWidth(m_command->lineEdit()->fontMetrics().maxWidth()*20);

	QLabel *label1 = new QLabel(m_command->lineEdit(), i18n("Comman&d:"), page);
	QLabel *label2 = new QLabel(m_name,                i18n("&Name:"),    page);
	layout->addWidget(label1,    0, 0, Qt::AlignVCenter);
	layout->addWidget(label2,    1, 0, Qt::AlignVCenter);
	layout->addWidget(label,     2, 0, Qt::AlignVCenter);
	layout->addWidget(m_command, 0, 1, Qt::AlignVCenter);
	layout->addWidget(m_name,    1, 1, Qt::AlignVCenter);
	layout->addWidget(wid,       2, 1, Qt::AlignVCenter);

	QWidget *stretchWidget = new QWidget(page);
	stretchWidget->setSizePolicy(QSizePolicy(/*hor=*/QSizePolicy::Fixed, /*ver=*/QSizePolicy::Expanding, /*hStretch=*/1, /*vStretch=*/5000)); // Make it fill ALL vertical space
	layout->addWidget(stretchWidget, 3, 1, Qt::AlignVCenter);

	connect( guessButton, SIGNAL(clicked()), this, SLOT(guessIcon()) );
}

LauncherEditDialog::~LauncherEditDialog()
{
}

void LauncherEditDialog::polish()
{
	KDialogBase::polish();
	if (m_command->runCommand().isEmpty()) {
		m_command->lineEdit()->setFocus();
		m_command->lineEdit()->end(false);
	} else {
		m_name->setFocus();
		m_name->end(false);
	}
}

void LauncherEditDialog::slotOk()
{
	// TODO: Remember if a string has been modified AND IS DIFFERENT FROM THE ORIGINAL!

	KConfig conf(m_noteContent->fullPath());
	conf.setGroup("Desktop Entry");
	conf.writeEntry("Exec", m_command->runCommand());
	conf.writeEntry("Name", m_name->text());
	conf.writeEntry("Icon", m_icon->icon());

	// Just for faster feedback: conf object will save to disk (and then m_note->loadContent() called)
	m_noteContent->setLauncher(m_name->text(), m_icon->icon(), m_command->runCommand());
	m_noteContent->setEdited();

	KDialogBase::slotOk();
}

void LauncherEditDialog::guessIcon()
{
	m_icon->setIcon( NoteFactory::iconForCommand(m_command->runCommand()) );
}

/** class InlineEditors: */

InlineEditors::InlineEditors()
{
	m_mainWindow = 0;
}

InlineEditors::~InlineEditors()
{
}

InlineEditors* InlineEditors::instance()
{
	static InlineEditors *instance = 0;
	if (!instance)
		instance = new InlineEditors();
	return instance;
}

void InlineEditors::initToolBars(KMainWindow *mainWindow)
{
	m_mainWindow = mainWindow;

	// Init the RichTextEditor Toolbar:

	richTextFont = new FocusedFontCombo(mainWindow);
	richTextFont->setFixedWidth(richTextFont->sizeHint().width() * 2 / 3);
	KWidgetAction *action = new KWidgetAction(richTextFont, i18n("Font"), Qt::Key_F6,
	                                          /*receiver=*/0, /*slot=*/"", mainWindow->actionCollection(), "richtext_font");
	richTextColor = new FocusedColorCombo(mainWindow);
	richTextColor->setFixedWidth(richTextColor->sizeHint().height() * 2);
	action = new KWidgetAction(richTextColor, i18n("Color"), KShortcut(), 0, SLOT(), mainWindow->actionCollection(), "richtext_color");

	richTextBold      = new KToggleAction( i18n("Bold"),        "text_bold",   "Ctrl+B", mainWindow->actionCollection(), "richtext_bold"      );
	richTextItalic    = new KToggleAction( i18n("Italic"),      "text_italic", "Ctrl+I", mainWindow->actionCollection(), "richtext_italic"    );
	richTextUnderline = new KToggleAction( i18n("Underline"),   "text_under",  "Ctrl+U", mainWindow->actionCollection(), "richtext_underline" );

//	richTextSuper     = new KToggleAction( i18n("Superscript"), "text_super",  "",       mainWindow->actionCollection(), "richtext_super"     );
//	richTextSub       = new KToggleAction( i18n("Subscript"),   "text_sub",    "",       mainWindow->actionCollection(), "richtext_sub"       );

	richTextLeft      = new KToggleAction( i18n("Align Left"),  "text_left",   "",       mainWindow->actionCollection(), "richtext_left"      );
	richTextCenter    = new KToggleAction( i18n("Centered"),    "text_center", "",       mainWindow->actionCollection(), "richtext_center"    );
	richTextRight     = new KToggleAction( i18n("Align Right"), "text_right",  "",       mainWindow->actionCollection(), "richtext_right"     );
	richTextJustified = new KToggleAction( i18n("Justified"),   "text_block",  "",       mainWindow->actionCollection(), "richtext_block"     );

	richTextLeft->setExclusiveGroup("rt_justify");
	richTextCenter->setExclusiveGroup("rt_justify");
	richTextRight->setExclusiveGroup("rt_justify");
	richTextJustified->setExclusiveGroup("rt_justify");

	disableRichTextToolBar();
}

KToolBar* InlineEditors::richTextToolBar()
{
	if (m_mainWindow) {
		m_mainWindow->toolBar(); // Make sure we create the main toolbar FIRST, so it will be on top of the edit toolbar!
		return m_mainWindow->toolBar("richTextEditToolBar");
	} else
		return 0;
}

void InlineEditors::enableRichTextToolBar()
{
	richTextFont->setEnabled(true);
	richTextColor->setEnabled(true);
	richTextBold->setEnabled(true);
	richTextItalic->setEnabled(true);
	richTextUnderline->setEnabled(true);
	richTextLeft->setEnabled(true);
	richTextCenter->setEnabled(true);
	richTextRight->setEnabled(true);
	richTextJustified->setEnabled(true);
}

void InlineEditors::disableRichTextToolBar()
{
	disconnect(richTextFont);
	disconnect(richTextColor);
	disconnect(richTextBold);
	disconnect(richTextItalic);
	disconnect(richTextUnderline);
	disconnect(richTextLeft);
	disconnect(richTextCenter);
	disconnect(richTextRight);
	disconnect(richTextJustified);

	richTextFont->setEnabled(false);
	richTextColor->setEnabled(false);
	richTextBold->setEnabled(false);
	richTextItalic->setEnabled(false);
	richTextUnderline->setEnabled(false);
	richTextLeft->setEnabled(false);
	richTextCenter->setEnabled(false);
	richTextRight->setEnabled(false);
	richTextJustified->setEnabled(false);
}

#include "noteedit.moc"

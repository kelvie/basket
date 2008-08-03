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

#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
//Added by qt3to4:
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QGridLayout>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kcolordialog.h>
#include <kservice.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <ktoolbar.h>
#include <kaction.h>
#include <kurifilter.h>
#include <kdebug.h>
#include <kstandardaction.h>

#include "kicondialog.h"
#include "noteedit.h"
#include "notecontent.h"
 // Use Tools::
#include "notefactory.h"
#include "note.h"
#include "basket.h"
#include "settings.h"
#include "tools.h"
#include "variouswidgets.h"
#include "focusedwidgets.h"

#include <KActionCollection>
#include <KToggleAction>
#include <KDesktopFile>

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

/** class TextEditor: */

TextEditor::TextEditor(TextContent *textContent, QWidget *parent)
 : NoteEditor(textContent), m_textContent(textContent)
{
	FocusedTextEdit *textEdit = new FocusedTextEdit(/*disableUpdatesOnKeyPress=*/true, parent);
	textEdit->setLineWidth(0);
	textEdit->setMidLineWidth(0);
	textEdit->setTextFormat(Qt::PlainText);
	textEdit->setPaletteBackgroundColor(note()->backgroundColor());
	textEdit->setPaletteForegroundColor(note()->textColor());
	textEdit->setFont(note()->font());
	textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	if (Settings::spellCheckTextNotes())
		textEdit->setCheckSpellingEnabled(true);
	textEdit->setText(m_textContent->text());
	textEdit->moveCursor(KTextEdit::MoveEnd, false); // FIXME: Sometimes, the cursor flicker at ends before being positionned where clicked (because kapp->processEvents() I think)
	textEdit->verticalScrollBar()->setCursor(Qt::ArrowCursor);
	setInlineEditor(textEdit);
	connect( textEdit, SIGNAL(escapePressed()), this, SIGNAL(askValidation())            );
	connect( textEdit, SIGNAL(mouseEntered()),  this, SIGNAL(mouseEnteredEditorWidget()) );

	connect( textEdit, SIGNAL(cursorPositionChanged(int, int)), textContent->note()->basket(), SLOT(editorCursorPositionChanged()) );
	// In case it is a very big note, the top is displayed and Enter is pressed: the cursor is on bottom, we should enure it visible:
	QTimer::singleShot( 0, textContent->note()->basket(), SLOT(editorCursorPositionChanged()) );
}

TextEditor::~TextEditor()
{
	delete widget(); // TODO: delete that in validate(), so we can remove one method
}

void TextEditor::autoSave(bool toFileToo)
{
	bool autoSpellCheck = true;
	if (toFileToo) {
		if (Settings::spellCheckTextNotes() != textEdit()->checkSpellingEnabled()) {
			Settings::setSpellCheckTextNotes(textEdit()->checkSpellingEnabled());
			Settings::saveConfig();
		}

		autoSpellCheck = textEdit()->checkSpellingEnabled();
		textEdit()->setCheckSpellingEnabled(false);
	}

	m_textContent->setText(textEdit()->text());

	if (toFileToo) {
		m_textContent->saveToFile();
		m_textContent->setEdited();
		textEdit()->setCheckSpellingEnabled(autoSpellCheck);
	}
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
	FocusedTextEdit *textEdit = new FocusedTextEdit(/*disableUpdatesOnKeyPress=*/true, parent);
	textEdit->setLineWidth(0);
	textEdit->setMidLineWidth(0);
	textEdit->setTextFormat(Qt::RichText);
	textEdit->setAutoFormatting(Settings::autoBullet() ? QTextEdit::AutoAll :
                                QTextEdit::AutoNone);
	textEdit->setPaletteBackgroundColor(note()->backgroundColor());
	textEdit->setPaletteForegroundColor(note()->textColor());
	textEdit->setFont(note()->font());
	textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	textEdit->setText(m_htmlContent->html());
	textEdit->moveCursor(KTextEdit::MoveEnd, false);
	textEdit->verticalScrollBar()->setCursor(Qt::ArrowCursor);
	setInlineEditor(textEdit);

	connect( textEdit,                                    SIGNAL(mouseEntered()),  this, SIGNAL(mouseEnteredEditorWidget()) );
	connect( textEdit,                                    SIGNAL(escapePressed()), this, SIGNAL(askValidation()) );

	connect( InlineEditors::instance()->richTextFont,     SIGNAL(textChanged(const QString&)), textEdit, SLOT(setFamily(const QString&)) );
	connect( InlineEditors::instance()->richTextFontSize, SIGNAL(sizeChanged(int)),            textEdit, SLOT(setPointSize(int))         );
	connect( InlineEditors::instance()->richTextColor,    SIGNAL(activated(const QColor&)),    textEdit, SLOT(setColor(const QColor&))   );

	connect( InlineEditors::instance()->focusWidgetFilter, SIGNAL(escapePressed()),  textEdit, SLOT(setFocus()) );
	connect( InlineEditors::instance()->focusWidgetFilter, SIGNAL(returnPressed()), textEdit, SLOT(setFocus()) );
	connect( InlineEditors::instance()->richTextFont,     SIGNAL(activated(int)),   textEdit, SLOT(setFocus()) );

	connect( InlineEditors::instance()->richTextFontSize, SIGNAL(activated(int)),   textEdit, SLOT(setFocus()) );

	connect( InlineEditors::instance()->richTextColor,    SIGNAL(escapePressed()),  textEdit, SLOT(setFocus()) );
	connect( InlineEditors::instance()->richTextColor,    SIGNAL(returnPressed()), textEdit, SLOT(setFocus()) );

	connect( textEdit,  SIGNAL(cursorPositionChanged(int, int)),  this, SLOT(cursorPositionChanged())   );
	connect( textEdit,  SIGNAL(clicked(int, int)),                this, SLOT(cursorPositionChanged())   );
	connect( textEdit,  SIGNAL(currentFontChanged(const QFont&)), this, SLOT(fontChanged(const QFont&)) );
//	connect( textEdit,  SIGNAL(currentVerticalAlignmentChanged(VerticalAlignment)), this, SLOT(slotVerticalAlignmentChanged()) );

	connect( InlineEditors::instance()->richTextBold,      SIGNAL(toggled(bool)),    textEdit, SLOT(setBold(bool)) );
	connect( InlineEditors::instance()->richTextItalic,    SIGNAL(toggled(bool)),    textEdit, SLOT(setItalic(bool)) );
	connect( InlineEditors::instance()->richTextUnderline, SIGNAL(toggled(bool)),    textEdit, SLOT(setUnderline(bool)) );
	//REMOVE:
	//connect( InlineEditors::instance()->richTextBold,      SIGNAL(activated()), this, SLOT(setBold())      );
	//connect( InlineEditors::instance()->richTextItalic,    SIGNAL(activated()), this, SLOT(setItalic())    );
	//connect( InlineEditors::instance()->richTextUnderline, SIGNAL(activated()), this, SLOT(setUnderline()) );
	connect( InlineEditors::instance()->richTextLeft,      SIGNAL(activated()), this, SLOT(setLeft())      );
	connect( InlineEditors::instance()->richTextCenter,    SIGNAL(activated()), this, SLOT(setCentered())  );
	connect( InlineEditors::instance()->richTextRight,     SIGNAL(activated()), this, SLOT(setRight())     );
	connect( InlineEditors::instance()->richTextJustified, SIGNAL(activated()), this, SLOT(setBlock())     );

//	InlineEditors::instance()->richTextToolBar()->show();
	cursorPositionChanged();
	fontChanged(textEdit->currentFont());
	//QTimer::singleShot( 0, this, SLOT(cursorPositionChanged()) );
	InlineEditors::instance()->enableRichTextToolBar();

	connect( InlineEditors::instance()->richTextUndo,      SIGNAL(activated()), textEdit, SLOT(undo())         );
	connect( InlineEditors::instance()->richTextRedo,      SIGNAL(activated()), textEdit, SLOT(redo())         );
	connect( textEdit, SIGNAL(undoAvailable(bool)), InlineEditors::instance()->richTextUndo, SLOT(setEnabled(bool)) );
	connect( textEdit, SIGNAL(redoAvailable(bool)), InlineEditors::instance()->richTextRedo, SLOT(setEnabled(bool)) );
	connect( textEdit, SIGNAL(textChanged()), this, SLOT(textChanged()));
	InlineEditors::instance()->richTextUndo->setEnabled(false);
	InlineEditors::instance()->richTextRedo->setEnabled(false);

	connect( textEdit, SIGNAL(cursorPositionChanged(int, int)), htmlContent->note()->basket(), SLOT(editorCursorPositionChanged()) );
	// In case it is a very big note, the top is displayed and Enter is pressed: the cursor is on bottom, we should enure it visible:
	QTimer::singleShot( 0, htmlContent->note()->basket(), SLOT(editorCursorPositionChanged()) );
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

void HtmlEditor::textChanged()
{
	// The following is a workaround for an apparent Qt bug.
	// When I start typing in a textEdit, the undo&redo actions are not enabled until I click
	// or move the cursor - probably, the signal undoAvailable() is not emitted.
	// So, I had to intervene and do that manually.
	InlineEditors::instance()->richTextUndo->setEnabled(textEdit()->isUndoAvailable());
	InlineEditors::instance()->richTextRedo->setEnabled(textEdit()->isRedoAvailable());
}

void HtmlEditor::fontChanged(const QFont &font)
{
	InlineEditors::instance()->richTextFontSize->setFontSize(font.pointSize());
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

 // REMOVE: These functions are unused - it's now done by direct connection to textEdit
 //void HtmlEditor::setBold()      { textEdit()->setBold(      InlineEditors::instance()->richTextBold->isChecked()      ); }
 //void HtmlEditor::setItalic()    { textEdit()->setItalic(    InlineEditors::instance()->richTextItalic->isChecked()    ); }
 //void HtmlEditor::setUnderline() { textEdit()->setUnderline( InlineEditors::instance()->richTextUnderline->isChecked() ); }
void HtmlEditor::setLeft()      { textEdit()->setAlignment(Qt::AlignLeft);    }
void HtmlEditor::setCentered()  { textEdit()->setAlignment(Qt::AlignCenter);  }
void HtmlEditor::setRight()     { textEdit()->setAlignment(Qt::AlignRight);   }
void HtmlEditor::setBlock()     { textEdit()->setAlignment(Qt::AlignJustify); }

HtmlEditor::~HtmlEditor()
{
	delete widget();
}

void HtmlEditor::autoSave(bool toFileToo)
{
	m_htmlContent->setHtml(textEdit()->text());
	if (toFileToo) {
		m_htmlContent->saveToFile();
	m_htmlContent->setEdited();
	}
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
	if (InlineEditors::instance())
	{
		InlineEditors::instance()->disableRichTextToolBar();
//		if (InlineEditors::instance()->richTextToolBar())
//			InlineEditors::instance()->richTextToolBar()->hide();
	}
	delete widget();
	setInlineEditor(0);
}

/** class ImageEditor: */

ImageEditor::ImageEditor(ImageContent *imageContent, QWidget *parent)
 : NoteEditor(imageContent)
{
	int choice = KMessageBox::questionYesNo(parent, i18n(
		"Images can not be edited here at the moment (the next version of BasKet Note Pads will include an image editor).\n"
		"Do you want to open it with an application that understand it?"),
		i18n("Edit Image Note"),
		KStandardGuiItem::open(),
		KStandardGuiItem::cancel());

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
		KStandardGuiItem::open(),
		KStandardGuiItem::cancel());

	if (choice == KMessageBox::Yes)
		note()->basket()->noteOpen(note());
}

/** class FileEditor: */

FileEditor::FileEditor(FileContent *fileContent, QWidget *parent)
 : NoteEditor(fileContent), m_fileContent(fileContent)
{
	KLineEdit *lineEdit = new KLineEdit(parent);
	FocusWidgetFilter *filter = new FocusWidgetFilter(lineEdit);
	lineEdit->setLineWidth(0);
	lineEdit->setMidLineWidth(0);
	lineEdit->setPaletteBackgroundColor(note()->backgroundColor());
	lineEdit->setPaletteForegroundColor(note()->textColor());
	lineEdit->setFont(note()->font());
	lineEdit->setText(m_fileContent->fileName());
	lineEdit->selectAll();
	setInlineEditor(lineEdit);
	connect(filter, SIGNAL(returnPressed()),
		this, SIGNAL(askValidation()));
	connect(filter, SIGNAL(escapePressed()),
		this, SIGNAL(askValidation()));
	connect(filter, SIGNAL(mouseEntered()),
		this, SIGNAL(mouseEnteredEditorWidget()));
}

FileEditor::~FileEditor()
{
	delete widget();
}

void FileEditor::autoSave(bool toFileToo)
{
	// FIXME: How to detect cancel?
	if (toFileToo && !lineEdit()->text().isEmpty() && m_fileContent->trySetFileName(lineEdit()->text())) {
		m_fileContent->setFileName(lineEdit()->text());
		m_fileContent->setEdited();
	}
}

void FileEditor::validate()
{
	autoSave(/*toFileToo=*/true);
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
	KColorDialog dialog(parent);
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
     : KDialog(parent)
     , m_noteContent(contentNote)
{
	// KDialog options
	setCaption(i18n("Edit Link Note"));
	setButtons(Ok | Cancel);
	setDefaultButton(Ok);
	setObjectName("EditLink");
	setModal(true);
	showButtonSeparator(true);
	connect(this, SIGNAL(okClicked()), SLOT(slotOk()));

	QWidget     *page   = new QWidget(this);
	setMainWidget(page);
	QGridLayout *layout = new QGridLayout(page, /*nRows=*/4, /*nCols=*/2, /*margin=*/0, spacingHint());

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

	m_url = new KUrlRequester(KUrl(""), wid);
	KUrl filteredURL = NoteFactory::filteredURL(KUrl(m_url->lineEdit()->text()));//KURIFilter::self()->filteredURI(KUrl(m_url->lineEdit()->text()));
	m_icon->setIconType(KIconLoader::NoGroup, KIconLoader::MimeType);
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
	stretchWidget->setSizePolicy(QSizePolicy(/*hor=*/QSizePolicy::Fixed, /*ver=*/QSizePolicy::Expanding, /*hStretch=*/1, /*vStretch=*/255)); // Make it fill ALL vertical space
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
	KDialog::polish();
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
	KUrl filteredURL = NoteFactory::filteredURL(KUrl(m_url->url()));//KURIFilter::self()->filteredURI(KUrl(m_url->url()));
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
		KUrl filteredURL = NoteFactory::filteredURL(KUrl(m_url->url()));//KURIFilter::self()->filteredURI(KUrl(m_url->url()));
		m_icon->setIcon(NoteFactory::iconForURL(filteredURL));
	}
}

void LinkEditDialog::guessTitle()
{
	if (m_autoTitle->isOn()) {
		KUrl filteredURL = NoteFactory::filteredURL(KUrl(m_url->url()));//KURIFilter::self()->filteredURI(KUrl(m_url->url()));
		m_title->setText(NoteFactory::titleForURL(filteredURL));
		m_autoTitle->setOn(true); // Because the setText() will disable it!
	}
}

void LinkEditDialog::slotOk()
{
	KUrl filteredURL = NoteFactory::filteredURL(KUrl(m_url->url()));//KURIFilter::self()->filteredURI(KUrl(m_url->url()));
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
}

/** class LauncherEditDialog: */

LauncherEditDialog::LauncherEditDialog(LauncherContent *contentNote, QWidget *parent)
     : KDialog(parent)
     , m_noteContent(contentNote)
{
	// KDialog options
	setCaption(i18n("Edit Launcher Note"));
	setButtons(Ok | Cancel);
	setDefaultButton(Ok);
	setObjectName("EditLauncher");
	setModal(true);
	showButtonSeparator(true);
	connect(this, SIGNAL(okClicked()), SLOT(slotOk()));

	QWidget     *page   = new QWidget(this);
	setMainWidget(page);

	QGridLayout *layout = new QGridLayout(page, /*nRows=*/4, /*nCols=*/2, /*margin=*/0, spacingHint());

	KService service(contentNote->fullPath());

	m_command = new RunCommandRequester(service.exec(), i18n("Choose a command to run:"), page);
	m_name    = new QLineEdit(service.name(), page);

	QWidget *wid = new QWidget(page);
	QHBoxLayout *hLay = new QHBoxLayout(wid, /*margin=*/0, spacingHint());
	m_icon = new KIconButton(wid);
	QLabel *label = new QLabel(m_icon, i18n("&Icon:"), page);
	m_icon->setIconType(KIconLoader::NoGroup, KIconLoader::Application);
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
	stretchWidget->setSizePolicy(QSizePolicy(/*hor=*/QSizePolicy::Fixed, /*ver=*/QSizePolicy::Expanding, /*hStretch=*/1, /*vStretch=*/255)); // Make it fill ALL vertical space
	layout->addWidget(stretchWidget, 3, 1, Qt::AlignVCenter);

	connect( guessButton, SIGNAL(clicked()), this, SLOT(guessIcon()) );
}

LauncherEditDialog::~LauncherEditDialog()
{
}

void LauncherEditDialog::polish()
{
	KDialog::polish();
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
    // TODO: Remember if a string has been modified AND IS DIFFERENT FROM THE
    // ORIGINAL!

    KDesktopFile dtFile(m_noteContent->fullPath());
    KConfigGroup grp = dtFile.desktopGroup();
    grp.writeEntry("Exec", m_command->runCommand());
    grp.writeEntry("Name", m_name->text());
    grp.writeEntry("Icon", m_icon->icon());

    // Just for faster feedback: conf object will save to disk (and then
    // m_note->loadContent() called)
    m_noteContent->setLauncher(m_name->text(), m_icon->icon(),
                               m_command->runCommand());
    m_noteContent->setEdited();
}

void LauncherEditDialog::guessIcon()
{
	m_icon->setIcon( NoteFactory::iconForCommand(m_command->runCommand()) );
}

/** class InlineEditors: */

InlineEditors::InlineEditors()
{
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

void InlineEditors::initToolBars(KActionCollection *ac)
{
	QFont defaultFont;
	QColor textColor = (Global::bnpView && Global::bnpView->currentBasket() ?
		Global::bnpView->currentBasket()->textColor() :
		palette().color(QPalette::Text));

	// Init the RichTextEditor Toolbar:
	richTextFont = new QFontComboBox(Global::mainWindow());
	focusWidgetFilter = new FocusWidgetFilter(richTextFont);
	richTextFont->setFixedWidth(richTextFont->sizeHint().width() * 2 / 3);
	richTextFont->setCurrentFont(defaultFont.family());

	KAction *action = ac->addAction("richtext_font");
	action->setDefaultWidget(richTextFont);
	action->setText(i18n("Font"));
	action->setShortcut(Qt::Key_F6);

	richTextFontSize = new FontSizeCombo(/*rw=*/true, Global::mainWindow());
	richTextFontSize->setFontSize(defaultFont.pointSize());
	action = ac->addAction("richtext_font_size");
	action->setDefaultWidget(richTextFontSize);
	action->setText(i18n("Font Size"));
	action->setShortcut(Qt::Key_F7);

	richTextColor = new KColorCombo(Global::mainWindow());
	richTextColor->installEventFilter(focusWidgetFilter);
	richTextColor->setFixedWidth(richTextColor->sizeHint().height() * 2);
	richTextColor->setColor(textColor);
	action = ac->addAction("richtext_color");
	action->setText(i18n("Color"));

	KToggleAction *ta = NULL;
	ta = new KToggleAction(ac);
	ac->addAction("richtext_bold", ta);
	ta->setText(i18n("Bold"));
	ta->setIcon(KIcon("format-text-bold"));
	ta->setShortcut(KShortcut("Ctrl+B"));
	richTextBold = ta;

	ta = new KToggleAction(ac);
	ac->addAction("richtext_italic", ta);
	ta->setText(i18n("Italic"));
	ta->setIcon(KIcon("format-text-italic"));
	ta->setShortcut(KShortcut("Ctrl+I"));
	richTextItalic = ta;

	ta = new KToggleAction(ac);
	ac->addAction("richtext_underline", ta);
	ta->setText(i18n("Underline"));
	ta->setIcon(KIcon("format-text-underline"));
	ta->setShortcut(KShortcut("Ctrl+U"));
	richTextUnderline = ta;

#if 0
	ta = new KToggleAction(ac);
	ac->addAction("richtext_super", ta);
	ta->setText(i18n("Superscript"));
	ta->setIcon(KIcon("text_super"));
	richTextSuper = ta;

	ta = new KToggleAction(ac);
	ac->addAction("richtext_sub", ta);
	ta->setText(i18n("Subscript"));
	ta->setIcon(KIcon("text_sub"));
	richTextSub = ta;
#endif

	ta = new KToggleAction(ac);
	ac->addAction("richtext_left", ta);
	ta->setText(i18n("Align Left"));
	ta->setIcon(KIcon("format-justify-left"));
	richTextLeft = ta;

	ta = new KToggleAction(ac);
	ac->addAction("richtext_center", ta);
	ta->setText(i18n("Centered"));
	ta->setIcon(KIcon("format-justify-center"));
	richTextCenter = ta;

	ta = new KToggleAction(ac);
	ac->addAction("richtext_right", ta);
	ta->setText(i18n("Align Right"));
	ta->setIcon(KIcon("format-justify-right"));
	richTextRight = ta;

	ta = new KToggleAction(ac);
	ac->addAction("richtext_block", ta);
	ta->setText(i18n("Justified"));
	ta->setIcon(KIcon("format-justify-fill"));
	richTextJustified = ta;

	QActionGroup *alignmentGroup = new QActionGroup(this);
	alignmentGroup->addAction(richTextLeft);
	alignmentGroup->addAction(richTextCenter);
	alignmentGroup->addAction(richTextRight);
	alignmentGroup->addAction(richTextJustified);

	ta = new KToggleAction(ac);
	ac->addAction("richtext_undo", ta);
	ta->setText(i18n("Undo"));
	ta->setIcon(KIcon("edit-undo"));
	richTextUndo = ta;

	ta = new KToggleAction(ac);
	ac->addAction("richtext_redo", ta);
	ta->setText(i18n("Redo"));
	ta->setIcon(KIcon("edit-redo"));
	richTextRedo = ta;

	disableRichTextToolBar();
}

KToolBar* InlineEditors::richTextToolBar()
{
	if (Global::mainWindow()) {
		Global::mainWindow()->toolBar(); // Make sure we create the main toolbar FIRST, so it will be on top of the edit toolbar!
		return Global::mainWindow()->toolBar("richTextEditToolBar");
	} else
		return 0;
}

void InlineEditors::enableRichTextToolBar()
{
	richTextFont->setEnabled(true);
	richTextFontSize->setEnabled(true);
	richTextColor->setEnabled(true);
	richTextBold->setEnabled(true);
	richTextItalic->setEnabled(true);
	richTextUnderline->setEnabled(true);
	richTextLeft->setEnabled(true);
	richTextCenter->setEnabled(true);
	richTextRight->setEnabled(true);
	richTextJustified->setEnabled(true);
	richTextUndo->setEnabled(true);
	richTextRedo->setEnabled(true);
}

void InlineEditors::disableRichTextToolBar()
{
	disconnect(richTextFont);
	disconnect(richTextFontSize);
	disconnect(richTextColor);
	disconnect(richTextBold);
	disconnect(richTextItalic);
	disconnect(richTextUnderline);
	disconnect(richTextLeft);
	disconnect(richTextCenter);
	disconnect(richTextRight);
	disconnect(richTextJustified);
	disconnect(richTextUndo);
	disconnect(richTextRedo);

	richTextFont->setEnabled(false);
	richTextFontSize->setEnabled(false);
	richTextColor->setEnabled(false);
	richTextBold->setEnabled(false);
	richTextItalic->setEnabled(false);
	richTextUnderline->setEnabled(false);
	richTextLeft->setEnabled(false);
	richTextCenter->setEnabled(false);
	richTextRight->setEnabled(false);
	richTextJustified->setEnabled(false);
	richTextUndo->setEnabled(false);
	richTextRedo->setEnabled(false);

	// Return to a "proper" state:
	QFont defaultFont;
	QColor textColor = (Global::bnpView && Global::bnpView->currentBasket() ?
		Global::bnpView->currentBasket()->textColor() :
		palette().color(QPalette::Text));
	richTextFont->setCurrentFont(defaultFont.family());
	richTextFontSize->setFontSize(defaultFont.pointSize());
	richTextColor->setColor(textColor);
	richTextBold->setChecked(false);
	richTextItalic->setChecked(false);
	richTextUnderline->setChecked(false);
	richTextLeft->setChecked(false);
	richTextCenter->setChecked(false);
	richTextRight->setChecked(false);
	richTextJustified->setChecked(false);
}

QPalette InlineEditors::palette() const
{
    return kapp->palette();
}


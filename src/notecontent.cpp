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

#include <qfile.h>
#include <qdir.h>
#include <qdom.h>
#include <qpainter.h>
#include <q3stylesheet.h>
#include <qfontmetrics.h>
#include <qwidget.h>
#include <qcursor.h>
#include <qstringlist.h>
#include <qbuffer.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <QPixmap>
#include <ktextedit.h>
#include <kservice.h>
#include <kcolordialog.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <qbitmap.h>
#include <kurifilter.h>
#include <qregexp.h>
//#include <kstringhandler.h>
#include <kfilemetainfo.h>
#include <qdatetime.h>
#include <k3multipledrag.h>

#include <qfileinfo.h>
//#include <kio/kfileitem.h>
#include <kfileitem.h>
#include <kio/previewjob.h>
#include <kio/global.h>

#include "notecontent.h"
#include "note.h"
#include "noteedit.h"
#include "tag.h"
#include "basket.h"
#include "filter.h"
#include "xmlwork.h"
#include "tools.h"
#include "notefactory.h"
#include "linklabel.h"
#include "global.h"
#include "settings.h"
#include "focusedwidgets.h"
#include "debugwindow.h"
#include "kcolorcombo2.h"
#include "htmlexporter.h"

#include "config.h"
#define WITHOUT_ARTS
#ifndef WITHOUT_ARTS
	#include <arts/kplayobject.h>
	#include <arts/kplayobjectfactory.h>
	#include <arts/kartsserver.h>
	#include <arts/kartsdispatcher.h>
#endif

#include <KDebug>

/** class NoteContent:
 */

const int NoteContent::FEEDBACK_DARKING = 105;

NoteContent::NoteContent(Note *parent, const QString &fileName)
 : m_note(parent)
{
	parent->setContent(this);
	setFileName(fileName);
}

void NoteContent::saveToNode(QDomDocument &doc, QDomElement &content)
{
	if (useFile()) {
		QDomText textNode = doc.createTextNode(fileName());
		content.appendChild(textNode);
	}
}

QRect NoteContent::zoneRect(int zone, const QPoint &/*pos*/)
{
	if (zone == Note::Content)
		return QRect(0, 0, note()->width(), note()->height()); // Too wide and height, but it will be clipped by Note::zoneRect()
	else
		return QRect();
}

KUrl NoteContent::urlToOpen(bool /*with*/)
{
	return (useFile() ? KUrl(fullPath()) : KUrl());
}

void NoteContent::setFileName(const QString &fileName)
{
	m_fileName = fileName;
}

bool NoteContent::trySetFileName(const QString &fileName)
{
	if (useFile() && fileName != m_fileName) {
		QString newFileName = Tools::fileNameForNewFile(fileName, basket()->fullPath());
		QDir dir;
		dir.rename(fullPath(), basket()->fullPathForFileName(newFileName));
		return true;
	}

	return false; // !useFile() or unsuccesful rename
}

QString NoteContent::fullPath()
{
	if (note() && useFile())
		return note()->fullPath();
	else
		return "";
}

void NoteContent::contentChanged(int newMinWidth)
{
	m_minWidth = newMinWidth;
	if (note()) {
//		note()->unbufferize();
		note()->requestRelayout(); // TODO: It should re-set the width!  m_width = 0 ?   contentChanged: setWidth, geteight, if size havent changed, only repaint and not relayout
	}
}

Basket* NoteContent::basket()
{
	if (note())
		return note()->basket();
	else
		return 0;
}

void NoteContent::setEdited()
{
	note()->setLastModificationDate(QDateTime::currentDateTime());
	basket()->save();
}

/** All the Content Classes:
 */

NoteType::Id TextContent::type()      { return NoteType::Text;      }
NoteType::Id HtmlContent::type()      { return NoteType::Html;      }
NoteType::Id ImageContent::type()     { return NoteType::Image;     }
NoteType::Id AnimationContent::type() { return NoteType::Animation; }
NoteType::Id SoundContent::type()     { return NoteType::Sound;     }
NoteType::Id FileContent::type()      { return NoteType::File;      }
NoteType::Id LinkContent::type()      { return NoteType::Link;      }
NoteType::Id LauncherContent::type()  { return NoteType::Launcher;  }
NoteType::Id ColorContent::type()     { return NoteType::Color;     }
NoteType::Id UnknownContent::type()   { return NoteType::Unknown;   }

QString TextContent::typeName()      { return i18n("Plain Text"); }
QString HtmlContent::typeName()      { return i18n("Text");       }
QString ImageContent::typeName()     { return i18n("Image");      }
QString AnimationContent::typeName() { return i18n("Animation");  }
QString SoundContent::typeName()     { return i18n("Sound");      }
QString FileContent::typeName()      { return i18n("File");       }
QString LinkContent::typeName()      { return i18n("Link");       }
QString LauncherContent::typeName()  { return i18n("Launcher");   }
QString ColorContent::typeName()     { return i18n("Color");      }
QString UnknownContent::typeName()   { return i18n("Unknown");    }

QString TextContent::lowerTypeName()      { return "text";      }
QString HtmlContent::lowerTypeName()      { return "html";      }
QString ImageContent::lowerTypeName()     { return "image";     }
QString AnimationContent::lowerTypeName() { return "animation"; }
QString SoundContent::lowerTypeName()     { return "sound";     }
QString FileContent::lowerTypeName()      { return "file";      }
QString LinkContent::lowerTypeName()      { return "link";      }
QString LauncherContent::lowerTypeName()  { return "launcher";  }
QString ColorContent::lowerTypeName()     { return "color";     }
QString UnknownContent::lowerTypeName()   { return "unknown";   }

QString NoteContent::toText(const QString &cuttedFullPath)
{
	return (cuttedFullPath.isEmpty() ? fullPath() : cuttedFullPath);
}

QString TextContent::toText(const QString &/*cuttedFullPath*/)      { return text();                    }
QString HtmlContent::toText(const QString &/*cuttedFullPath*/)      { return Tools::htmlToText(html()); }
QString LinkContent::toText(const QString &/*cuttedFullPath*/)
{
	if (autoTitle())
		return url().prettyUrl();
	else if (title().isEmpty() && url().isEmpty())
		return "";
	else if (url().isEmpty())
		return title();
	else if (title().isEmpty())
		return url().prettyUrl();
	else
		return QString("%1 <%2>").arg(title(), url().prettyUrl());
}
QString ColorContent::toText(const QString &/*cuttedFullPath*/)     { return color().name();                }
QString UnknownContent::toText(const QString &/*cuttedFullPath*/)   { return "";                            }

// TODO: If imageName.isEmpty() return fullPath() because it's for external use, else return fileName() because it's to display in a tooltip
QString TextContent::toHtml(const QString &/*imageName*/, const QString &/*cuttedFullPath*/)
{ return Tools::textToHTMLWithoutP(text());                                                       }

QString HtmlContent::toHtml(const QString &/*imageName*/, const QString &/*cuttedFullPath*/)
{ return Tools::htmlToParagraph(html());                                                          }

QString ImageContent::toHtml(const QString &/*imageName*/, const QString &cuttedFullPath)
{ return QString("<img src=\"%1\">").arg(cuttedFullPath.isEmpty() ? fullPath() : cuttedFullPath); }

QString AnimationContent::toHtml(const QString &/*imageName*/, const QString &cuttedFullPath)
{ return QString("<img src=\"%1\">").arg(cuttedFullPath.isEmpty() ? fullPath() : cuttedFullPath); }

QString SoundContent::toHtml(const QString &/*imageName*/, const QString &cuttedFullPath)
{ return QString("<a href=\"%1\">%2</a>").arg((cuttedFullPath.isEmpty() ? fullPath() : cuttedFullPath), fileName()); } // With the icon?

QString FileContent::toHtml(const QString &/*imageName*/, const QString &cuttedFullPath)
{ return QString("<a href=\"%1\">%2</a>").arg((cuttedFullPath.isEmpty() ? fullPath() : cuttedFullPath), fileName()); } // With the icon?

QString LinkContent::toHtml(const QString &/*imageName*/, const QString &/*cuttedFullPath*/)
{ return QString("<a href=\"%1\">%2</a>").arg(url().prettyUrl(), title());                        } // With the icon?

QString LauncherContent::toHtml(const QString &/*imageName*/, const QString &cuttedFullPath)
{ return QString("<a href=\"%1\">%2</a>").arg((cuttedFullPath.isEmpty() ? fullPath() : cuttedFullPath), name());     } // With the icon?

QString ColorContent::toHtml(const QString &/*imageName*/, const QString &/*cuttedFullPath*/)
{ return QString("<span style=\"color: %1\">%2</span>").arg(color().name(), color().name());  }

QString UnknownContent::toHtml(const QString &/*imageName*/, const QString &/*cuttedFullPath*/)
{ return "";                                                                                  }

QPixmap ImageContent::toPixmap()     { return pixmap();              }
QPixmap AnimationContent::toPixmap() { return m_movie->currentPixmap(); }

void NoteContent::toLink(KUrl *url, QString *title, const QString &cuttedFullPath)
{
	if (useFile()) {
		*url   = KUrl(cuttedFullPath.isEmpty() ? fullPath() : cuttedFullPath);
		*title =     (cuttedFullPath.isEmpty() ? fullPath() : cuttedFullPath);
	} else {
		*url   = KUrl();
		*title = QString();
	}
}
void LinkContent::toLink(KUrl *url, QString *title, const QString &/*cuttedFullPath*/)
{
	*url   = this->url();
	*title = this->title();
}

void LauncherContent::toLink(KUrl *url, QString *title, const QString &cuttedFullPath)
{
	*url   = KUrl(cuttedFullPath.isEmpty() ? fullPath() : cuttedFullPath);
	*title = name();
}
void UnknownContent::toLink(KUrl *url, QString *title, const QString &/*cuttedFullPath*/)
{
	*url   = KUrl();
	*title = QString();
}

bool TextContent::useFile()      { return true;  }
bool HtmlContent::useFile()      { return true;  }
bool ImageContent::useFile()     { return true;  }
bool AnimationContent::useFile() { return true;  }
bool SoundContent::useFile()     { return true;  }
bool FileContent::useFile()      { return true;  }
bool LinkContent::useFile()      { return false; }
bool LauncherContent::useFile()  { return true;  }
bool ColorContent::useFile()     { return false; }
bool UnknownContent::useFile()   { return true;  }

bool TextContent::canBeSavedAs()      { return true;  }
bool HtmlContent::canBeSavedAs()      { return true;  }
bool ImageContent::canBeSavedAs()     { return true;  }
bool AnimationContent::canBeSavedAs() { return true;  }
bool SoundContent::canBeSavedAs()     { return true;  }
bool FileContent::canBeSavedAs()      { return true;  }
bool LinkContent::canBeSavedAs()      { return true;  }
bool LauncherContent::canBeSavedAs()  { return true;  }
bool ColorContent::canBeSavedAs()     { return false; }
bool UnknownContent::canBeSavedAs()   { return false; }

QString TextContent::saveAsFilters()      { return "text/plain";            }
QString HtmlContent::saveAsFilters()      { return "text/html";             }
QString ImageContent::saveAsFilters()     { return "image/png";             } // TODO: Offer more types
QString AnimationContent::saveAsFilters() { return "image/gif";             } // TODO: MNG...
QString SoundContent::saveAsFilters()     { return "audio/mp3 audio/ogg";           } // TODO: OGG...
QString FileContent::saveAsFilters()      { return "*";                     } // TODO: Get MIME type of the url target
QString LinkContent::saveAsFilters()      { return "*";                     } // TODO: idem File + If isDir(): return
QString LauncherContent::saveAsFilters()  { return "application/x-desktop"; }
QString ColorContent::saveAsFilters()     { return "";                      }
QString UnknownContent::saveAsFilters()   { return "";                      }

bool TextContent::match(const FilterData &data)          { return (text().find(data.string, /*index=*/0, /*cs=*/false) != -1);         }
bool HtmlContent::match(const FilterData &data)          { return (m_textEquivalent/*toText("")*/.find(data.string, /*index=*/0, /*cs=*/false) != -1);     } //OPTIM_FILTER
bool ImageContent::match(const FilterData &/*data*/)     { return false;                                                               }
bool AnimationContent::match(const FilterData &/*data*/) { return false;                                                               }
bool SoundContent::match(const FilterData &data)         { return (fileName().find(data.string, /*index=*/0, /*cs=*/false) != -1);     }
bool FileContent::match(const FilterData &data)          { return (fileName().find(data.string, /*index=*/0, /*cs=*/false) != -1);     }
bool LinkContent::match(const FilterData &data)          { return (title().find(data.string, 0, false) != -1 || url().prettyUrl().find(data.string, 0, false) != -1); }
bool LauncherContent::match(const FilterData &data)      { return (exec().find(data.string, 0, false) != -1 || name().find(data.string, 0, false) != -1); }
bool ColorContent::match(const FilterData &data)         { return (color().name().find(data.string, /*index=*/0, /*cs=*/false) != -1); }
bool UnknownContent::match(const FilterData &data)       { return (mimeTypes().find(data.string, /*index=*/0, /*cs=*/false) != -1);    }

QString TextContent::editToolTipText()      { return i18n("Edit this plain text");             }
QString HtmlContent::editToolTipText()      { return i18n("Edit this text");                   }
QString ImageContent::editToolTipText()     { return i18n("Edit this image");                  }
QString AnimationContent::editToolTipText() { return i18n("Edit this animation");              }
QString SoundContent::editToolTipText()     { return i18n("Edit the file name of this sound"); }
QString FileContent::editToolTipText()      { return i18n("Edit the name of this file");       }
QString LinkContent::editToolTipText()      { return i18n("Edit this link");                   }
QString LauncherContent::editToolTipText()  { return i18n("Edit this launcher");               }
QString ColorContent::editToolTipText()     { return i18n("Edit this color");                  }
QString UnknownContent::editToolTipText()   { return i18n("Edit this unknown object");         }

QString TextContent::cssClass()      { return "";         }
QString HtmlContent::cssClass()      { return "";         }
QString ImageContent::cssClass()     { return "";         }
QString AnimationContent::cssClass() { return "";         }
QString SoundContent::cssClass()     { return "sound";    }
QString FileContent::cssClass()      { return "file";     }
QString LinkContent::cssClass()      { return (LinkLook::lookForURL(m_url) == LinkLook::localLinkLook ? "local" : "network"); }
QString LauncherContent::cssClass()  { return "launcher"; }
QString ColorContent::cssClass()     { return ""     ;    }
QString UnknownContent::cssClass()   { return "";         }

void TextContent::fontChanged()      { setText(text());                                          }
void HtmlContent::fontChanged()      { setHtml(html());                                          }
void ImageContent::fontChanged()     { setPixmap(pixmap());                                      }
void AnimationContent::fontChanged() { updateMovie();                                            }
void FileContent::fontChanged()      { setFileName(fileName());                                  }
void LinkContent::fontChanged()      { setLink(url(), title(), icon(), autoTitle(), autoIcon()); }
void LauncherContent::fontChanged()  { setLauncher(name(), icon(), exec());                      }
void ColorContent::fontChanged()     { setColor(color());                                        }
void UnknownContent::fontChanged()   { loadFromFile(/*lazyLoad=*/false);                         } // TODO: Optimize: setMimeTypes()

//QString TextContent::customOpenCommand()      { return (Settings::isTextUseProg()      && ! Settings::textProg().isEmpty()      ? Settings::textProg()      : QString()); }
QString HtmlContent::customOpenCommand()      { return (Settings::isHtmlUseProg()      && ! Settings::htmlProg().isEmpty()      ? Settings::htmlProg()      : QString()); }
QString ImageContent::customOpenCommand()     { return (Settings::isImageUseProg()     && ! Settings::imageProg().isEmpty()     ? Settings::imageProg()     : QString()); }
QString AnimationContent::customOpenCommand() { return (Settings::isAnimationUseProg() && ! Settings::animationProg().isEmpty() ? Settings::animationProg() : QString()); }
QString SoundContent::customOpenCommand()     { return (Settings::isSoundUseProg()     && ! Settings::soundProg().isEmpty()     ? Settings::soundProg()     : QString()); }

void LinkContent::serialize(QDataStream &stream)  { stream << url() << title() << icon() << (quint64)autoTitle() << (quint64)autoIcon(); }
void ColorContent::serialize(QDataStream &stream) { stream << color();  }

QPixmap TextContent::feedbackPixmap(int width, int height)
{
	QRect textRect = QFontMetrics(note()->font()).boundingRect(0, 0, width, height, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, text());
	QPixmap pixmap( qMin(width, textRect.width()), qMin(height, textRect.height()) );
	pixmap.fill(note()->backgroundColor().dark(FEEDBACK_DARKING));
	QPainter painter(&pixmap);
	painter.setPen(note()->textColor());
	painter.setFont(note()->font());
	painter.drawText(0, 0, pixmap.width(), pixmap.height(), Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, text());
	painter.end();
	return pixmap;
}

QPixmap HtmlContent::feedbackPixmap(int width, int height)
{
	Q3SimpleRichText richText(html(), note()->font());
	richText.setWidth(width);
	QColorGroup colorGroup(basket()->colorGroup());
	colorGroup.setColor(QColorGroup::Text,       note()->textColor());
	colorGroup.setColor(QColorGroup::Background, note()->backgroundColor().dark(FEEDBACK_DARKING));
	QPixmap pixmap( qMin(width, richText.widthUsed()), qMin(height, richText.height()) );
	pixmap.fill(note()->backgroundColor().dark(FEEDBACK_DARKING));
	QPainter painter(&pixmap);
	painter.setPen(note()->textColor());
	richText.draw(&painter, 0, 0, QRect(0, 0, pixmap.width(), pixmap.height()), colorGroup);
	painter.end();
	return pixmap;
}

QPixmap ImageContent::feedbackPixmap(int width, int height)
{
	if (width >= m_pixmap.width() && height >= m_pixmap.height()) { // Full size
		if (m_pixmap.hasAlpha()) {
			QPixmap opaque(m_pixmap.width(), m_pixmap.height());
			opaque.fill(note()->backgroundColor().dark(FEEDBACK_DARKING));
			QPainter painter(&opaque);
			painter.drawPixmap(0, 0, m_pixmap);
			painter.end();
			return opaque;
		} else
			return m_pixmap;
	} else { // Scalled down
		QImage imageToScale = m_pixmap.convertToImage();
		QPixmap pmScaled;
		pmScaled.convertFromImage(imageToScale.scaled(width, height,
                                                      Qt::ScaleMin));
		if (pmScaled.hasAlpha()) {
			QPixmap opaque(pmScaled.width(), pmScaled.height());
			opaque.fill(note()->backgroundColor().dark(FEEDBACK_DARKING));
			QPainter painter(&opaque);
			painter.drawPixmap(0, 0, pmScaled);
			painter.end();
			return opaque;
		} else
			return pmScaled;
	}
}

QPixmap AnimationContent::feedbackPixmap(int width, int height)
{
	QPixmap pixmap = m_movie->currentPixmap();
	if (width >= pixmap.width() && height >= pixmap.height()) // Full size
		return pixmap;
	else { // Scalled down
		QImage imageToScale = pixmap.convertToImage();
		QPixmap pmScaled;
		pmScaled.convertFromImage(imageToScale.scaled(width, height,
                                                      Qt::ScaleMin));
		return pmScaled;
	}
}

QPixmap LinkContent::feedbackPixmap(int width, int height)
{
	QColorGroup colorGroup(basket()->colorGroup());
	colorGroup.setColor(QColorGroup::Text,       note()->textColor());
	colorGroup.setColor(QColorGroup::Background, note()->backgroundColor().dark(FEEDBACK_DARKING));
	return m_linkDisplay.feedbackPixmap(width, height, colorGroup, /*isDefaultColor=*/note()->textColor() == basket()->textColor());
}

QPixmap ColorContent::feedbackPixmap(int width, int height)
{
	// TODO: Duplicate code: make a rect() method!
	QRect textRect = QFontMetrics(note()->font()).boundingRect(color().name());
	int rectHeight = (textRect.height() + 2)*3/2;
	int rectWidth  = rectHeight * 14 / 10; // 1.4 times the height, like A4 papers.

	QColorGroup colorGroup(basket()->colorGroup());
	colorGroup.setColor(QColorGroup::Text,       note()->textColor());
	colorGroup.setColor(QColorGroup::Background, note()->backgroundColor().dark(FEEDBACK_DARKING));

	QPixmap pixmap( qMin(width, rectWidth + RECT_MARGIN + textRect.width() + RECT_MARGIN), qMin(height, rectHeight) );
	pixmap.fill(note()->backgroundColor().dark(FEEDBACK_DARKING));
	QPainter painter(&pixmap);
	paint(&painter, pixmap.width(), pixmap.height(), colorGroup, false, false, false); // We don't care of the three last boolean parameters.
	painter.end();
	return pixmap;
}

QPixmap FileContent::feedbackPixmap(int width, int height)
{
	QColorGroup colorGroup(basket()->colorGroup());
	colorGroup.setColor(QColorGroup::Text,       note()->textColor());
	colorGroup.setColor(QColorGroup::Background, note()->backgroundColor().dark(FEEDBACK_DARKING));
	return m_linkDisplay.feedbackPixmap(width, height, colorGroup, /*isDefaultColor=*/note()->textColor() == basket()->textColor());
}

QPixmap LauncherContent::feedbackPixmap(int width, int height)
{
	QColorGroup colorGroup(basket()->colorGroup());
	colorGroup.setColor(QColorGroup::Text,       note()->textColor());
	colorGroup.setColor(QColorGroup::Background, note()->backgroundColor().dark(FEEDBACK_DARKING));
	return m_linkDisplay.feedbackPixmap(width, height, colorGroup, /*isDefaultColor=*/note()->textColor() == basket()->textColor());
}

QPixmap UnknownContent::feedbackPixmap(int width, int height)
{
	QRect textRect = QFontMetrics(note()->font()).boundingRect(0, 0, /*width=*/1, 500000, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, m_mimeTypes);

	QColorGroup colorGroup(basket()->colorGroup());
	colorGroup.setColor(QColorGroup::Text,       note()->textColor());
	colorGroup.setColor(QColorGroup::Background, note()->backgroundColor().dark(FEEDBACK_DARKING));

	QPixmap pixmap( qMin(width, DECORATION_MARGIN + textRect.width() + DECORATION_MARGIN), qMin(height, DECORATION_MARGIN + textRect.height() + DECORATION_MARGIN) );
	QPainter painter(&pixmap);
	paint(&painter, pixmap.width() + 1, pixmap.height(), colorGroup, false, false, false); // We don't care of the three last boolean parameters.
	painter.setPen(note()->backgroundColor().dark(FEEDBACK_DARKING));
	painter.drawPoint(0,                  0);
	painter.drawPoint(pixmap.width() - 1, 0);
	painter.drawPoint(0,                  pixmap.height() - 1);
	painter.drawPoint(pixmap.width() - 1, pixmap.height() - 1);
	painter.end();
	return pixmap;
}


/** class TextContent:
 */

TextContent::TextContent(Note *parent, const QString &fileName, bool lazyLoad)
 : NoteContent(parent, fileName), m_simpleRichText(0)
{
	basket()->addWatchedFile(fullPath());
	loadFromFile(lazyLoad);
}

TextContent::~TextContent()
{
	delete m_simpleRichText;
}

int TextContent::setWidthAndGetHeight(int width)
{
	if (m_simpleRichText) {
		width -= 1;
		m_simpleRichText->setWidth(width);
		return m_simpleRichText->height();
	} else
		return 10; // Lazy loaded
}

void TextContent::paint(QPainter *painter, int width, int height, const QColorGroup &colorGroup, bool /*isDefaultColor*/, bool /*isSelected*/, bool /*isHovered*/)
{
	if (m_simpleRichText) {
		width -= 1;
		m_simpleRichText->draw(painter, 0, 0, QRect(0, 0, width, height), colorGroup);
	}
}

bool TextContent::loadFromFile(bool lazyLoad)
{
	DEBUG_WIN << "Loading TextContent From " + basket()->folderName() + fileName();

	QString content;
	bool success = basket()->loadFromFile(fullPath(), &content, /*isLocalEncoding=*/true);

	if (success)
		setText(content, lazyLoad);
	else {
		kDebug() << "FAILED TO LOAD TextContent: " << fullPath();
		setText("", lazyLoad);
		if (!QFile::exists(fullPath()))
			saveToFile(); // Reserve the fileName so no new note will have the same name!
	}
	return success;
}

bool TextContent::finishLazyLoad()
{
	int width = (m_simpleRichText ? m_simpleRichText->width() : 1);
	delete m_simpleRichText;
	QString html = "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body>" + Tools::tagURLs(Tools::textToHTML(m_text)); // Don't collapse multiple spaces!
	m_simpleRichText = new Q3SimpleRichText(html, note()->font());
	m_simpleRichText->setWidth(1); // We put a width of 1 pixel, so usedWidth() is egual to the minimum width
	int minWidth = m_simpleRichText->widthUsed();
	m_simpleRichText->setWidth(width);
	contentChanged(minWidth + 1);

	return true;
}

bool TextContent::saveToFile()
{
	return basket()->saveToFile(fullPath(), text(), /*isLocalEncoding=*/true);
}

QString TextContent::linkAt(const QPoint &pos)
{
	if (m_simpleRichText)
		return m_simpleRichText->anchorAt(pos);
	else
		return ""; // Lazy loaded
}


QString TextContent::messageWhenOpening(OpenMessage where)
{
	switch (where) {
		case OpenOne:               return i18n("Opening plain text...");
		case OpenSeveral:           return i18n("Opening plain texts...");
		case OpenOneWith:           return i18n("Opening plain text with...");
		case OpenSeveralWith:       return i18n("Opening plain texts with...");
		case OpenOneWithDialog:     return i18n("Open plain text with:");
		case OpenSeveralWithDialog: return i18n("Open plain texts with:");
		default:                    return "";
	}
}

void TextContent::setText(const QString &text, bool lazyLoad)
{
	m_text = text;
	if (!lazyLoad)
		finishLazyLoad();
	else
		contentChanged(10);
}

void TextContent::exportToHTML(HTMLExporter *exporter, int indent)
{
	QString spaces;
	QString html = "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body>" +
	               Tools::tagURLs(Tools::textToHTMLWithoutP(text().replace("\t", "                "))); // Don't collapse multiple spaces!
	exporter->stream << html.replace("  ", " &nbsp;").replace("\n", "\n" + spaces.fill(' ', indent + 1));
}

/** class HtmlContent:
 */

HtmlContent::HtmlContent(Note *parent, const QString &fileName, bool lazyLoad)
 : NoteContent(parent, fileName), m_simpleRichText(0)
{
	basket()->addWatchedFile(fullPath());
	loadFromFile(lazyLoad);
}

HtmlContent::~HtmlContent()
{
	delete m_simpleRichText;
}

int HtmlContent::setWidthAndGetHeight(int width)
{
	if (m_simpleRichText) {
		width -= 1;
		m_simpleRichText->setWidth(width);
		return m_simpleRichText->height();
	} else
		return 10; // Lazy loaded
}

void HtmlContent::paint(QPainter *painter, int width, int height, const QColorGroup &colorGroup, bool /*isDefaultColor*/, bool /*isSelected*/, bool /*isHovered*/)
{
	if (m_simpleRichText) {
		width -= 1;
		m_simpleRichText->draw(painter, 0, 0, QRect(0, 0, width, height), colorGroup);
	}
}

bool HtmlContent::loadFromFile(bool lazyLoad)
{
	DEBUG_WIN << "Loading HtmlContent From " + basket()->folderName() + fileName();

	QString content;
	bool success = basket()->loadFromFile(fullPath(), &content, /*isLocalEncoding=*/true);

	if (success)
		setHtml(content, lazyLoad);
	else {
		kDebug() << "FAILED TO LOAD HtmlContent: " << fullPath();
		setHtml("", lazyLoad);
		if (!QFile::exists(fullPath()))
			saveToFile(); // Reserve the fileName so no new note will have the same name!
	}
	return success;
}

bool HtmlContent::finishLazyLoad()
{
	int width = (m_simpleRichText ? m_simpleRichText->width() : 1);
	delete m_simpleRichText;
	m_simpleRichText = new Q3SimpleRichText(Tools::tagURLs(m_html), note()->font());
	m_simpleRichText->setWidth(1); // We put a width of 1 pixel, so usedWidth() is egual to the minimum width
	int minWidth = m_simpleRichText->widthUsed();
	m_simpleRichText->setWidth(width);
	contentChanged(minWidth + 1);

	return true;
}

bool HtmlContent::saveToFile()
{
	return basket()->saveToFile(fullPath(), html(), /*isLocalEncoding=*/true);
}

QString HtmlContent::linkAt(const QPoint &pos)
{
	if (m_simpleRichText)
		return m_simpleRichText->anchorAt(pos);
	else
		return ""; // Lazy loaded
}


QString HtmlContent::messageWhenOpening(OpenMessage where)
{
	switch (where) {
		case OpenOne:               return i18n("Opening text...");
		case OpenSeveral:           return i18n("Opening texts...");
		case OpenOneWith:           return i18n("Opening text with...");
		case OpenSeveralWith:       return i18n("Opening texts with...");
		case OpenOneWithDialog:     return i18n("Open text with:");
		case OpenSeveralWithDialog: return i18n("Open texts with:");
		default:                    return "";
	}
}

void HtmlContent::setHtml(const QString &html, bool lazyLoad)
{
	m_html = html;
	m_textEquivalent = toText(""); //OPTIM_FILTER
	if (!lazyLoad)
		finishLazyLoad();
	else
		contentChanged(10);
}

void HtmlContent::exportToHTML(HTMLExporter *exporter, int indent)
{
	QString spaces;
	exporter->stream << Tools::htmlToParagraph(Tools::tagURLs(html().replace("\t", "                ")))
	                    .replace("  ", " &nbsp;")
	                    .replace("\n", "\n" + spaces.fill(' ', indent + 1));
}

/** class ImageContent:
 */

ImageContent::ImageContent(Note *parent, const QString &fileName, bool lazyLoad)
 : NoteContent(parent, fileName), m_format()
{
	basket()->addWatchedFile(fullPath());
	loadFromFile(lazyLoad);
}

int ImageContent::setWidthAndGetHeight(int width)
{
	width -= 1;
	// Don't store width: we will get it on paint!
	if (width >= m_pixmap.width()) // Full size
		return m_pixmap.height();
	else { // Scalled down
		double height = m_pixmap.height() * (double)width / m_pixmap.width();
		return int((double)(int)height <= (height - 0.5) ? height + 1 : height);
	}
}

void ImageContent::paint(QPainter *painter, int width, int /*height*/, const QColorGroup &/*colorGroup*/, bool /*isDefaultColor*/, bool /*isSelected*/, bool /*isHovered*/)
{
	width -= 1;
//	KPixmap pixmap = m_pixmap;
//	if (note()->isSelected())
//		pixmap = KPixmapEffect::selectedPixmap(m_pixmap, palette().color(QPalette::Highlight));

	if (width >= m_pixmap.width()) // Full size
		painter->drawPixmap(0, 0, m_pixmap);
	else { // Scalled down
		double scale = ((double)width) / m_pixmap.width();
		painter->scale(scale, scale);
		painter->drawPixmap(0, 0, m_pixmap);  // TODO: Smooth !!!
	}
}

bool ImageContent::loadFromFile(bool lazyLoad)
{
	if (lazyLoad)
		return true;
	else
		return finishLazyLoad();
}

bool ImageContent::finishLazyLoad()
{
	DEBUG_WIN << "Loading ImageContent From " + basket()->folderName() + fileName();

	QByteArray content;

	if (basket()->loadFromFile(fullPath(), &content))
	{
		QBuffer buffer(&content);

		buffer.open(QIODevice::ReadOnly);
		m_format = QImageReader::imageFormat(&buffer); // See QImageIO to know what formats can be supported.
		buffer.close();
		if (!m_format.isNull()) {
			m_pixmap.loadFromData(content);
			setPixmap(m_pixmap);
			return true;
		}
	}

	kDebug() << "FAILED TO LOAD ImageContent: " << fullPath();
	m_format = "PNG"; // If the image is set later, it should be saved without destruction, so we use PNG by default.
	m_pixmap.resize(1, 1); // Create a 1x1 pixels image instead of an undefined one.
	m_pixmap.fill();
	m_pixmap.setMask(m_pixmap.createHeuristicMask());
	setPixmap(m_pixmap);
	if (!QFile::exists(fullPath()))
		saveToFile(); // Reserve the fileName so no new note will have the same name!
	return false;
}

bool ImageContent::saveToFile()
{
	QByteArray ba;
	QBuffer buffer(&ba);

	buffer.open(QIODevice::WriteOnly);
	m_pixmap.save(&buffer, m_format);
	return basket()->saveToFile(fullPath(), ba);
}


void ImageContent::toolTipInfos(QStringList *keys, QStringList *values)
{
	keys->append(i18n("Size"));
	values->append(i18n("%1 by %2 pixels",QString::number(m_pixmap.width()), QString::number(m_pixmap.height())));
}

QString ImageContent::messageWhenOpening(OpenMessage where)
{
	switch (where) {
		case OpenOne:               return i18n("Opening image...");
		case OpenSeveral:           return i18n("Opening images...");
		case OpenOneWith:           return i18n("Opening image with...");
		case OpenSeveralWith:       return i18n("Opening images with...");
		case OpenOneWithDialog:     return i18n("Open image with:");
		case OpenSeveralWithDialog: return i18n("Open images with:");
		default:                    return "";
	}
}

void ImageContent::setPixmap(const QPixmap &pixmap)
{
	m_pixmap = pixmap;
	// Since it's scalled, the height is always greater or equal to the size of the tag emblems (16)
	contentChanged(16 + 1); // TODO: always good? I don't think...
}

void ImageContent::exportToHTML(HTMLExporter *exporter, int /*indent*/)
{
	int width  = m_pixmap.width();
	int height = m_pixmap.height();
	int contentWidth = note()->width() - note()->contentX() - 1 - Note::NOTE_MARGIN;

	QString imageName = exporter->copyFile(fullPath(), /*createIt=*/true);

	if (contentWidth <= m_pixmap.width()) { // Scalled down
		double scale = ((double)contentWidth) / m_pixmap.width();
		width  = (int)(m_pixmap.width()  * scale);
		height = (int)(m_pixmap.height() * scale);
		exporter->stream << "<a href=\"" << exporter->dataFolderName << imageName << "\" title=\"" << i18n("Click for full size view") << "\">";
	}

	exporter->stream << "<img src=\"" << exporter->dataFolderName << imageName
	                 << "\" width=\"" << width << "\" height=\"" << height << "\" alt=\"\">";

	if (contentWidth <= m_pixmap.width()) // Scalled down
		exporter->stream << "</a>";
}

/** class AnimationContent:
 */

AnimationContent::AnimationContent(Note *parent,
                                   const QString &fileName,
                                   bool lazyLoad)
 : NoteContent(parent, fileName)
 , m_buffer(new QBuffer(this))
 , m_movie(new QMovie(this))
{
    basket()->addWatchedFile(fullPath());
    loadFromFile(lazyLoad);
    connect(m_movie, SIGNAL(updated(QRect)), this, SLOT(movieUpdated()));
    connect(m_movie, SIGNAL(resized(QSize)), this, SLOT(movieResized()));
}

int AnimationContent::setWidthAndGetHeight(int /*width*/)
{
	/*width -= 1*/;
	return  m_movie->currentPixmap().height()  ; // TODO!!!
}

void AnimationContent::paint(QPainter *painter, int width, int /*height*/, const QColorGroup &/*colorGroup*/, bool /*isDefaultColor*/, bool /*isSelected*/, bool /*isHovered*/)
{
	QPixmap frame = m_movie->currentPixmap();
	if (width >= frame.width()) // Full size
		painter->drawPixmap(0, 0, frame);
	else // Scaled down
		painter->drawPixmap(0, 0, frame);  // TODO: Scale down
}

bool AnimationContent::loadFromFile(bool lazyLoad)
{
	if (lazyLoad)
		return true;
	else
		return finishLazyLoad();
}

bool AnimationContent::finishLazyLoad()
{
    QByteArray content;
    if (basket()->loadFromFile(fullPath(), &content)) {
        m_buffer->setData(content);
        updateMovie();
        return true;
    }
    m_buffer->setData(0);
    return false;
}

bool AnimationContent::saveToFile()
{
	// Impossible!
	return false;
}


QString AnimationContent::messageWhenOpening(OpenMessage where)
{
	switch (where) {
		case OpenOne:               return i18n("Opening animation...");
		case OpenSeveral:           return i18n("Opening animations...");
		case OpenOneWith:           return i18n("Opening animation with...");
		case OpenSeveralWith:       return i18n("Opening animations with...");
		case OpenOneWithDialog:     return i18n("Open animation with:");
		case OpenSeveralWithDialog: return i18n("Open animations with:");
		default:                    return "";
	}
}

bool AnimationContent::updateMovie()
{
    if (!m_buffer->data().isEmpty())
        return false;
    m_movie->setDevice(m_buffer);
    contentChanged(m_movie->currentPixmap().width() + 1); // TODO
    return true;
}

void AnimationContent::movieUpdated()
{
	note()->unbufferize();
	note()->update();
}

void AnimationContent::movieResized()
{
	note()->requestRelayout(); // ?
}

void AnimationContent::exportToHTML(HTMLExporter *exporter, int /*indent*/)
{
	exporter->stream << QString("<img src=\"%1\" width=\"%2\" height=\"%3\" alt=\"\">")
	                    .arg( exporter->dataFolderName + exporter->copyFile(fullPath(), /*createIt=*/true),
	                          QString::number(m_movie->currentPixmap().size().width()),
	                          QString::number(m_movie->currentPixmap().size().height()) );
}

/** class FileContent:
 */

FileContent::FileContent(Note *parent, const QString &fileName)
 : NoteContent(parent, fileName), m_previewJob(0)
{
	basket()->addWatchedFile(fullPath());
	setFileName(fileName); // FIXME: TO THAT HERE BECAUSE NoteContent() constructor seems to don't be able to call virtual methods???
}

int FileContent::setWidthAndGetHeight(int width)
{
	m_linkDisplay.setWidth(width);
	return m_linkDisplay.height();
}

void FileContent::paint(QPainter *painter, int width, int height, const QColorGroup &colorGroup, bool isDefaultColor, bool isSelected, bool isHovered)
{
	m_linkDisplay.paint(painter, 0, 0, width, height, colorGroup, isDefaultColor, isSelected, isHovered, isHovered && note()->hoveredZone() == Note::Custom0);
}

bool FileContent::loadFromFile(bool /*lazyLoad*/)
{
	setFileName(fileName()); // File changed: get new file preview!
	return true;
}

void FileContent::toolTipInfos(QStringList *keys, QStringList *values)
{
	// Get the size of the file:
	uint size = QFileInfo(fullPath()).size();
	QString humanFileSize = KIO::convertSize((KIO::filesize_t)size);

	keys->append(i18n("Size"));
	values->append(humanFileSize);

	KMimeType::Ptr mime = KMimeType::findByUrl(KUrl(fullPath()));
	if (mime) {
		keys->append(i18n("Type"));
		values->append(mime->comment());
	}

	KFileMetaInfo infos = KFileMetaInfo(KUrl(fullPath()));
	if (infos.isValid()) {
            QStringList groups = infos.preferredKeys();
            int i = 0;
            for (QStringList::Iterator it = groups.begin();
                 i < 6 && it != groups.end();
                 ++it) {
                KFileMetaInfoItem item = infos.item(*it);
                QString value = item.value().toString();
                if (!value.isEmpty()) {
                    keys->append(item.name());
                    value = QString("%1%2%3").arg(item.prefix(), value,
                                                  item.suffix());
                    values->append(value);
                    ++i;
                }
            }
	}
}

int FileContent::zoneAt(const QPoint &pos)
{
	return (m_linkDisplay.iconButtonAt(pos) ? 0 : Note::Custom0);
}

QRect FileContent::zoneRect(int zone, const QPoint &/*pos*/)
{
	QRect linkRect = m_linkDisplay.iconButtonRect();

	if (zone == Note::Custom0)
		return QRect(linkRect.width(), 0, note()->width(), note()->height()); // Too wide and height, but it will be clipped by Note::zoneRect()
	else if (zone == Note::Content)
		return linkRect;
	else
		return QRect();
}

QString FileContent::zoneTip(int zone)
{
	return (zone == Note::Custom0 ? i18n("Open this file") : QString());
}

void FileContent::setCursor(QWidget *widget, int zone)
{
	if (zone == Note::Custom0)
		widget->setCursor(Qt::PointingHandCursor);
}


int FileContent::xEditorIndent()
{
	return m_linkDisplay.iconButtonRect().width() + 2;
}


QString FileContent::messageWhenOpening(OpenMessage where)
{
	switch (where) {
		case OpenOne:               return i18n("Opening file...");
		case OpenSeveral:           return i18n("Opening files...");
		case OpenOneWith:           return i18n("Opening file with...");
		case OpenSeveralWith:       return i18n("Opening files with...");
		case OpenOneWithDialog:     return i18n("Open file with:");
		case OpenSeveralWithDialog: return i18n("Open files with:");
		default:                    return "";
	}
}

void FileContent::setFileName(const QString &fileName)
{
	NoteContent::setFileName(fileName);
	KUrl url = KUrl(fullPath());
	if (linkLook()->previewEnabled())
		m_linkDisplay.setLink(fileName, NoteFactory::iconForURL(url),            linkLook(), note()->font()); // FIXME: move iconForURL outside of NoteFactory !!!!!
	else
		m_linkDisplay.setLink(fileName, NoteFactory::iconForURL(url), QPixmap(), linkLook(), note()->font());
	startFetchingUrlPreview();
	contentChanged(m_linkDisplay.minWidth());
}

void FileContent::linkLookChanged()
{
	fontChanged();
	//setFileName(fileName());
	//startFetchingUrlPreview();
}

void FileContent::newPreview(const KFileItem*, const QPixmap &preview)
{
	LinkLook *linkLook = this->linkLook();
	m_linkDisplay.setLink(fileName(), NoteFactory::iconForURL(KUrl(fullPath())), (linkLook->previewEnabled() ? preview : QPixmap()), linkLook, note()->font());
	contentChanged(m_linkDisplay.minWidth());
}

void FileContent::removePreview(const KFileItem*)
{
	newPreview(0, QPixmap());
}

void FileContent::startFetchingUrlPreview()
{
	KUrl url(fullPath());
	LinkLook *linkLook = this->linkLook();

//	delete m_previewJob;
	if (!url.isEmpty() && linkLook->previewSize() > 0) {
		KUrl filteredUrl = NoteFactory::filteredURL(url);//KURIFilter::self()->filteredURI(url);
		KUrl::List urlList;
		urlList.append(filteredUrl);
		m_previewJob = KIO::filePreview(urlList, linkLook->previewSize(), linkLook->previewSize(), linkLook->iconSize());
		connect( m_previewJob, SIGNAL(gotPreview(const KFileItem*, const QPixmap&)), this, SLOT(newPreview(const KFileItem*, const QPixmap&)) );
		connect( m_previewJob, SIGNAL(failed(const KFileItem*)),                     this, SLOT(removePreview(const KFileItem*))              );
	}
}

void FileContent::exportToHTML(HTMLExporter *exporter, int indent)
{
	QString spaces;
	QString fileName = exporter->copyFile(fullPath(), true);
	exporter->stream << m_linkDisplay.toHtml(exporter, KUrl(exporter->dataFolderName + fileName), "").replace("\n", "\n" + spaces.fill(' ', indent + 1));
}

/** class SoundContent:
 */

SoundContent::SoundContent(Note *parent, const QString &fileName)
 : FileContent(parent, fileName)
{
	setFileName(fileName);
	music = new Phonon::MediaObject(this);
     	music->setCurrentSource(Phonon::MediaSource(fullPath()));
     	Phonon::AudioOutput *audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
     	Phonon::Path path = Phonon::createPath(music, audioOutput);
	connect(music, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(stateChanged(Phonon::State, Phonon::State)));
}

void SoundContent::stateChanged(Phonon::State newState, Phonon::State oldState){
kDebug()<<"stateChanged "<<oldState<<" to "<<newState;
}

QString SoundContent::zoneTip(int zone)
{
	return (zone == Note::Custom0 ? i18n("Open this sound") : QString());
}

void SoundContent::setHoveredZone(int oldZone, int newZone)
{
	if (newZone == Note::Custom0 || newZone == Note::Content) {
		// Start the sound preview:
		if (oldZone != Note::Custom0 && oldZone != Note::Content) { // Don't restart if it was already in one of those zones
		if(music->state()==1){
	     		music->play();
			
			}
		}
	} else {
//		 Stop the sound preview, if it was started:
		if (music->state()!=1) {
			music->stop();
//			delete music;//TODO implement this in slot connected with music alted signal 
//			music = 0;
		}
	}
}


QString SoundContent::messageWhenOpening(OpenMessage where)
{
	switch (where) {
		case OpenOne:               return i18n("Opening sound...");
		case OpenSeveral:           return i18n("Opening sounds...");
		case OpenOneWith:           return i18n("Opening sound with...");
		case OpenSeveralWith:       return i18n("Opening sounds with...");
		case OpenOneWithDialog:     return i18n("Open sound with:");
		case OpenSeveralWithDialog: return i18n("Open sounds with:");
		default:                    return "";
	}
}

/** class LinkContent:
 */

LinkContent::LinkContent(Note *parent, const KUrl &url, const QString &title, const QString &icon, bool autoTitle, bool autoIcon)
	: NoteContent(parent), m_http(0), m_httpBuff(0), m_previewJob(0)
{
	setLink(url, title, icon, autoTitle, autoIcon);
}
LinkContent::~LinkContent()
{
	delete m_http;
	delete m_httpBuff;
}

int LinkContent::setWidthAndGetHeight(int width)
{
	m_linkDisplay.setWidth(width);
	return m_linkDisplay.height();
}

void LinkContent::paint(QPainter *painter, int width, int height, const QColorGroup &colorGroup, bool isDefaultColor, bool isSelected, bool isHovered)
{
	m_linkDisplay.paint(painter, 0, 0, width, height, colorGroup, isDefaultColor, isSelected, isHovered, isHovered && note()->hoveredZone() == Note::Custom0);
}

void LinkContent::saveToNode(QDomDocument &doc, QDomElement &content)
{
	content.setAttribute("title",      title()                        );
	content.setAttribute("icon",       icon()                         );
	content.setAttribute("autoTitle", (autoTitle() ? "true" : "false"));
	content.setAttribute("autoIcon",  (autoIcon()  ? "true" : "false"));
	QDomText textNode = doc.createTextNode(url().prettyUrl());
	content.appendChild(textNode);
}


void LinkContent::toolTipInfos(QStringList *keys, QStringList *values)
{
	keys->append(i18n("Target"));
	values->append(m_url.prettyUrl());
}

int LinkContent::zoneAt(const QPoint &pos)
{
	return (m_linkDisplay.iconButtonAt(pos) ? 0 : Note::Custom0);
}

QRect LinkContent::zoneRect(int zone, const QPoint &/*pos*/)
{
	QRect linkRect = m_linkDisplay.iconButtonRect();

	if (zone == Note::Custom0)
		return QRect(linkRect.width(), 0, note()->width(), note()->height()); // Too wide and height, but it will be clipped by Note::zoneRect()
	else if (zone == Note::Content)
		return linkRect;
	else
		return QRect();
}

QString LinkContent::zoneTip(int zone)
{
	return (zone == Note::Custom0 ? i18n("Open this link") : QString());
}

void LinkContent::setCursor(QWidget *widget, int zone)
{
	if (zone == Note::Custom0)
		widget->setCursor(Qt::PointingHandCursor);
}

QString LinkContent::statusBarMessage(int zone)
{
	if (zone == Note::Custom0 || zone == Note::Content)
		return m_url.prettyUrl();
	else
		return "";
}


KUrl LinkContent::urlToOpen(bool /*with*/)
{
	return NoteFactory::filteredURL(url());//KURIFilter::self()->filteredURI(url());
}

QString LinkContent::messageWhenOpening(OpenMessage where)
{
	if (url().isEmpty())
		return i18n("Link have no URL to open.");

	switch (where) {
		case OpenOne:               return i18n("Opening link target...");
		case OpenSeveral:           return i18n("Opening link targets...");
		case OpenOneWith:           return i18n("Opening link target with...");
		case OpenSeveralWith:       return i18n("Opening link targets with...");
		case OpenOneWithDialog:     return i18n("Open link target with:");
		case OpenSeveralWithDialog: return i18n("Open link targets with:");
		default:                    return "";
	}
}

void LinkContent::setLink(const KUrl &url, const QString &title, const QString &icon, bool autoTitle, bool autoIcon)
{
	m_autoTitle = autoTitle;
	m_autoIcon  = autoIcon;
	m_url       = NoteFactory::filteredURL(KUrl(url));//KURIFilter::self()->filteredURI(url);
	m_title     = (autoTitle ? NoteFactory::titleForURL(m_url) : title);
	m_icon      = (autoIcon  ? NoteFactory::iconForURL(m_url)  : icon);

	LinkLook *look = LinkLook::lookForURL(m_url);
	if (look->previewEnabled())
		m_linkDisplay.setLink(m_title, m_icon,            look, note()->font());
	else
		m_linkDisplay.setLink(m_title, m_icon, QPixmap(), look, note()->font());
	startFetchingUrlPreview();
        if(autoTitle)
            startFetchingLinkTitle();
	contentChanged(m_linkDisplay.minWidth());
}

void LinkContent::linkLookChanged()
{
	fontChanged();
}

void LinkContent::newPreview(const KFileItem*, const QPixmap &preview)
{
	LinkLook *linkLook = LinkLook::lookForURL(url());
	m_linkDisplay.setLink(title(), icon(), (linkLook->previewEnabled() ? preview : QPixmap()), linkLook, note()->font());
	contentChanged(m_linkDisplay.minWidth());
}

void LinkContent::removePreview(const KFileItem*)
{
	newPreview(0, QPixmap());
}

// QHttp slots for getting link title
void LinkContent::httpReadyRead()
{
	Q_ULONG bytesAvailable = m_http->bytesAvailable();
	if(bytesAvailable <= 0)
		return;
	
	char* buf = new char[bytesAvailable+1];
	
	Q_LONG bytes_read = m_http->read(buf, bytesAvailable);
	if(bytes_read > 0) {
	
		// m_httpBuff will keep data if title is not found in initial read
		if(m_httpBuff == 0)	{
			m_httpBuff = new QString(buf);
		}
		else {
			(*m_httpBuff) += buf;
		}

		// todo: this should probably strip odd html tags like &nbsp; etc
		QRegExp reg("<title>[\\s]*(&nbsp;)?([^<]+)[\\s]*</title>", false);
		reg.setMinimal(TRUE);
		int offset = 0;
		//kDebug() << *m_httpBuff << " bytes: " << bytes_read;
		if((offset = reg.search(*m_httpBuff)) >= 0) {
			m_title = reg.cap(2);
			m_autoTitle = false;
			setEdited();
			
			// refresh the title
			setLink(url(), title(), icon(), autoTitle(), autoIcon());
			
			// stop the http connection
			m_http->abort();
			
			delete m_httpBuff;
			m_httpBuff = 0;
		}
		// Stop at 10k bytes
		else if(m_httpBuff->length() > 10000)	{
			m_http->abort();
			delete m_httpBuff;
			m_httpBuff = 0;
		}
	}
	delete buf;
}
void LinkContent::httpDone(bool)
{
	m_http->closeConnection();
}

void LinkContent::startFetchingLinkTitle()
{
	if(this->url().protocol() == "http")
	{
		// delete old m_http, for some reason it will not connect a second time...
		if(m_http != 0) {
			delete m_http;
			m_http = 0;
		}
		if(m_http == 0) {
			m_http = new QHttp(this);
			connect(m_http, SIGNAL(done(bool)), this, SLOT(httpDone(bool)));
			connect(m_http, SIGNAL(readyRead(QHttpResponseHeader)), this,
					SLOT(httpReadyRead()));
		}
		m_http->setHost(this->url().host(), this->url().port() == 0 ? 80: this->url().port());
		QString path = this->url().encodedPathAndQuery(KUrl::AddTrailingSlash);
		if(path == "")
			path = "/";
		//kDebug()  <<  "path: " << path;
		m_http->get(path);
	}
}

// Code dupicated from FileContent::startFetchingUrlPreview()
void LinkContent::startFetchingUrlPreview()
{
	KUrl url = this->url();
	LinkLook *linkLook = LinkLook::lookForURL(this->url());

//	delete m_previewJob;
	if (!url.isEmpty() && linkLook->previewSize() > 0) {
		KUrl filteredUrl = NoteFactory::filteredURL(url);//KURIFilter::self()->filteredURI(url);
		KUrl::List urlList;
		urlList.append(filteredUrl);
		m_previewJob = KIO::filePreview(urlList, linkLook->previewSize(), linkLook->previewSize(), linkLook->iconSize());
		connect( m_previewJob, SIGNAL(gotPreview(const KFileItem*, const QPixmap&)), this, SLOT(newPreview(const KFileItem*, const QPixmap&)) );
		connect( m_previewJob, SIGNAL(failed(const KFileItem*)),                     this, SLOT(removePreview(const KFileItem*))              );
	}
}

void LinkContent::exportToHTML(HTMLExporter *exporter, int indent)
{
	QString linkTitle = title();

// TODO:
//	// Append address (useful for print version of the page/basket):
//	if (exportData.formatForImpression && (!autoTitle() && title() != NoteFactory::titleForURL(url().prettyUrl()))) {
//		// The address is on a new line, unless title is empty (empty lines was replaced by &nbsp;):
//		if (linkTitle == " "/*"&nbsp;"*/)
//			linkTitle = url().prettyUrl()/*""*/;
//		else
//			linkTitle = linkTitle + " <" + url().prettyUrl() + ">"/*+ "<br>"*/;
//		//linkTitle += "<i>" + url().prettyUrl() + "</i>";
//	}

	KUrl linkURL;
/*
	QFileInfo fInfo(url().path());
//	DEBUG_WIN << url().path()
//	          << "IsFile:" + QString::number(fInfo.isFile())
//	          << "IsDir:"  + QString::number(fInfo.isDir());
	if (exportData.embedLinkedFiles && fInfo.isFile()) {
//		DEBUG_WIN << "Embed file";
		linkURL = exportData.dataFolderName + Basket::copyFile(url().path(), exportData.dataFolderPath, true);
	} else if (exportData.embedLinkedFolders && fInfo.isDir()) {
//		DEBUG_WIN << "Embed folder";
		linkURL = exportData.dataFolderName + Basket::copyFile(url().path(), exportData.dataFolderPath, true);
	} else {
//		DEBUG_WIN << "Embed LINK";
*/
		linkURL = url();
/*
	}
*/

	QString spaces;
	exporter->stream << m_linkDisplay.toHtml(exporter, linkURL, linkTitle).replace("\n", "\n" + spaces.fill(' ', indent + 1));
}

/** class LauncherContent:
 */

LauncherContent::LauncherContent(Note *parent, const QString &fileName)
 : NoteContent(parent, fileName)
{
	basket()->addWatchedFile(fullPath());
	loadFromFile(/*lazyLoad=*/false);
}

int LauncherContent::setWidthAndGetHeight(int width)
{
	m_linkDisplay.setWidth(width);
	return m_linkDisplay.height();
}

void LauncherContent::paint(QPainter *painter, int width, int height, const QColorGroup &colorGroup, bool isDefaultColor, bool isSelected, bool isHovered)
{
	m_linkDisplay.paint(painter, 0, 0, width, height, colorGroup, isDefaultColor, isSelected, isHovered, isHovered && note()->hoveredZone() == Note::Custom0);
}

bool LauncherContent::loadFromFile(bool /*lazyLoad*/) // TODO: saveToFile() ?? Is it possible?
{
	DEBUG_WIN << "Loading LauncherContent From " + basket()->folderName() + fileName();
	KService service(fullPath());
	setLauncher(service.name(), service.icon(), service.exec());
	return true;
}


void LauncherContent::toolTipInfos(QStringList *keys, QStringList *values)
{
	KService service(fullPath());

	QString exec = service.exec();
	if (service.terminal())
		exec = i18n("%1 <i>(run in terminal)</i>",exec);

	if (!service.comment().isEmpty() && service.comment() != service.name()) {
		keys->append(i18n("Comment"));
		values->append(service.comment());
	}

	keys->append(i18n("Command"));
	values->append(exec);
}

int LauncherContent::zoneAt(const QPoint &pos)
{
	return (m_linkDisplay.iconButtonAt(pos) ? 0 : Note::Custom0);
}

QRect LauncherContent::zoneRect(int zone, const QPoint &/*pos*/)
{
	QRect linkRect = m_linkDisplay.iconButtonRect();

	if (zone == Note::Custom0)
		return QRect(linkRect.width(), 0, note()->width(), note()->height()); // Too wide and height, but it will be clipped by Note::zoneRect()
	else if (zone == Note::Content)
		return linkRect;
	else
		return QRect();
}

QString LauncherContent::zoneTip(int zone)
{
	return (zone == Note::Custom0 ? i18n("Launch this application") : QString());
}

void LauncherContent::setCursor(QWidget *widget, int zone)
{
	if (zone == Note::Custom0)
		widget->setCursor(Qt::PointingHandCursor);
}


KUrl LauncherContent::urlToOpen(bool with)
{
	if (KService(fullPath()).exec().isEmpty())
		return KUrl();

	return (with ? KUrl() : KUrl(fullPath())); // Can open the appliation, but not with another application :-)
}

QString LauncherContent::messageWhenOpening(OpenMessage where)
{
	if (KService(fullPath()).exec().isEmpty())
		return i18n("The launcher have no command to run.");

	switch (where) {
		case OpenOne:               return i18n("Launching application...");
		case OpenSeveral:           return i18n("Launching applications...");
		case OpenOneWith:
		case OpenSeveralWith:
		case OpenOneWithDialog:
		case OpenSeveralWithDialog:            // TODO: "Open this application with this file as parameter"?
		default:                    return "";
	}
}

void LauncherContent::setLauncher(const QString &name, const QString &icon, const QString &exec)
{
	m_name = name;
	m_icon = icon;
	m_exec = exec;

	m_linkDisplay.setLink(name, icon, LinkLook::launcherLook, note()->font());
	contentChanged(m_linkDisplay.minWidth());
}

void LauncherContent::exportToHTML(HTMLExporter *exporter, int indent)
{
	QString spaces;
	QString fileName = exporter->copyFile(fullPath(), /*createIt=*/true);
	exporter->stream << m_linkDisplay.toHtml(exporter, KUrl(exporter->dataFolderName + fileName), "").replace("\n", "\n" + spaces.fill(' ', indent + 1));
}

/** class ColorContent:
 */

const int ColorContent::RECT_MARGIN = 2;

ColorContent::ColorContent(Note *parent, const QColor &color)
 : NoteContent(parent)
{
	setColor(color);
}

int ColorContent::setWidthAndGetHeight(int /*width*/) // We do not need width because we can't word-break, and width is always >= minWidth()
{
	// FIXME: Duplicate from setColor():
	QRect textRect = QFontMetrics(note()->font()).boundingRect(color().name());
	int rectHeight = (textRect.height() + 2)*3/2;
	return rectHeight;
}

void ColorContent::paint(QPainter *painter, int width, int height, const QColorGroup &colorGroup, bool /*isDefaultColor*/, bool /*isSelected*/, bool /*isHovered*/)
{
	// FIXME: Duplicate from setColor():
	QRect textRect = QFontMetrics(note()->font()).boundingRect(color().name());
	int rectHeight = (textRect.height() + 2)*3/2;
	int rectWidth  = rectHeight * 14 / 10; // 1.4 times the height, like A4 papers.

	// FIXME: Duplicate from CommonColorSelector::drawColorRect:
	// Fill:
	painter->fillRect(1, 1, rectWidth - 2, rectHeight - 2, color());
	// Stroke:
	QColor stroke = color().dark(125);
	painter->setPen(stroke);
	painter->drawLine(1,             0,              rectWidth - 2, 0);
	painter->drawLine(0,             1,              0,             rectHeight - 2);
	painter->drawLine(1,             rectHeight - 1, rectWidth - 2, rectHeight - 1);
	painter->drawLine(rectWidth - 1, 1,              rectWidth - 1, rectHeight - 2);
	// Round corners:
	painter->setPen(Tools::mixColor(color(), stroke));
	painter->drawPoint(1,             1);
	painter->drawPoint(1,             rectHeight - 2);
	painter->drawPoint(rectWidth - 2, rectHeight - 2);
	painter->drawPoint(rectWidth - 2, 1);

	// Draw the text:
	painter->setFont(note()->font());
	painter->setPen(colorGroup.text());
	painter->drawText(rectWidth + RECT_MARGIN, 0, width - rectWidth - RECT_MARGIN, height, Qt::AlignLeft | Qt::AlignVCenter, color().name());
}

void ColorContent::saveToNode(QDomDocument &doc, QDomElement &content)
{
	QDomText textNode = doc.createTextNode(color().name());
	content.appendChild(textNode);
}


void ColorContent::toolTipInfos(QStringList *keys, QStringList *values)
{
	int hue, saturation, value;
	m_color.getHsv(&hue, &saturation, &value);

	keys->append(i18nc("RGB Colorspace: Red/Green/Blue", "RGB"));
	values->append(i18n("<i>Red</i>: %1, <i>Green</i>: %2, <i>Blue</i>: %3,",QString::number(m_color.red()), QString::number(m_color.green()), QString::number(m_color.blue())));

	keys->append(i18nc("HSV Colorspace: Hue/Saturation/Value", "HSV"));
	values->append(i18n("<i>Hue</i>: %1, <i>Saturation</i>: %2, <i>Value</i>: %3,",QString::number(hue), QString::number(saturation), QString::number(value)));

	static QString cssColors[] = {
		"aqua",    "00ffff",
		"black",   "000000",
		"blue",    "0000ff",
		"fuchsia", "ff00ff",
		"gray",    "808080",
		"green",   "008000",
		"lime",    "00ff00",
		"maroon",  "800000",
		"navy",    "000080",
		"olive",   "808000",
		"purple",  "800080",
		"red",     "ff0000",
		"silver",  "c0c0c0",
		"teal",    "008080",
		"white",   "ffffff",
		"yellow",  "ffff00"
	};

	static QString cssExtendedColors[] = {
		"aliceblue",            "f0f8ff",
		"antiquewhite",         "faebd7",
		"aquamarine",           "7fffd4",
		"azure",                "f0ffff",
		"beige",                "f5f5dc",
		"bisque",               "ffe4c4",
		"blanchedalmond",       "ffebcd",
		"blueviolet",           "8a2be2",
		"brown",                "a52a2a",
		"burlywood",            "deb887",
		"cadetblue",            "5f9ea0",
		"chartreuse",           "7fff00",
		"chocolate",            "d2691e",
		"coral",                "ff7f50",
		"cornflowerblue",       "6495ed",
		"cornsilk",             "fff8dc",
		"crimson",              "dc1436",
		"cyan",                 "00ffff",
		"darkblue",             "00008b",
		"darkcyan",             "008b8b",
		"darkgoldenrod",        "b8860b",
		"darkgray",             "a9a9a9",
		"darkgreen",            "006400",
		"darkkhaki",            "bdb76b",
		"darkmagenta",          "8b008b",
		"darkolivegreen",       "556b2f",
		"darkorange",           "ff8c00",
		"darkorchid",           "9932cc",
		"darkred",              "8b0000",
		"darksalmon",           "e9967a",
		"darkseagreen",         "8fbc8f",
		"darkslateblue",        "483d8b",
		"darkslategray",        "2f4f4f",
		"darkturquoise",        "00ced1",
		"darkviolet",           "9400d3",
		"deeppink",             "ff1493",
		"deepskyblue",          "00bfff",
		"dimgray",              "696969",
		"dodgerblue",           "1e90ff",
		"firebrick",            "b22222",
		"floralwhite",          "fffaf0",
		"forestgreen",          "228b22",
		"gainsboro",            "dcdcdc",
		"ghostwhite",           "f8f8ff",
		"gold",                 "ffd700",
		"goldenrod",            "daa520",
		"greenyellow",          "adff2f",
		"honeydew",             "f0fff0",
		"hotpink",              "ff69b4",
		"indianred",            "cd5c5c",
		"indigo",               "4b0082",
		"ivory",                "fffff0",
		"khaki",                "f0e68c",
		"lavender",             "e6e6fa",
		"lavenderblush",        "fff0f5",
		"lawngreen",            "7cfc00",
		"lemonchiffon",         "fffacd",
		"lightblue",            "add8e6",
		"lightcoral",           "f08080",
		"lightcyan",            "e0ffff",
		"lightgoldenrodyellow", "fafad2",
		"lightgreen",           "90ee90",
		"lightgrey",            "d3d3d3",
		"lightpink",            "ffb6c1",
		"lightsalmon",          "ffa07a",
		"lightseagreen",        "20b2aa",
		"lightskyblue",         "87cefa",
		"lightslategray",       "778899",
		"lightsteelblue",       "b0c4de",
		"lightyellow",          "ffffe0",
		"limegreen",            "32cd32",
		"linen",                "faf0e6",
		"magenta",              "ff00ff",
		"mediumaquamarine",     "66cdaa",
		"mediumblue",           "0000cd",
		"mediumorchid",         "ba55d3",
		"mediumpurple",         "9370db",
		"mediumseagreen",       "3cb371",
		"mediumslateblue",      "7b68ee",
		"mediumspringgreen",    "00fa9a",
		"mediumturquoise",      "48d1cc",
		"mediumvioletred",      "c71585",
		"midnightblue",         "191970",
		"mintcream",            "f5fffa",
		"mistyrose",            "ffe4e1",
		"moccasin",             "ffe4b5",
		"navajowhite",          "ffdead",
		"oldlace",              "fdf5e6",
		"olivedrab",            "6b8e23",
		"orange",               "ffa500",
		"orangered",            "ff4500",
		"orchid",               "da70d6",
		"palegoldenrod",        "eee8aa",
		"palegreen",            "98fb98",
		"paleturquoise",        "afeeee",
		"palevioletred",        "db7093",
		"papayawhip",           "ffefd5",
		"peachpuff",            "ffdab9",
		"peru",                 "cd853f",
		"pink",                 "ffc0cb",
		"plum",                 "dda0dd",
		"powderblue",           "b0e0e6",
		"rosybrown",            "bc8f8f",
		"royalblue",            "4169e1",
		"saddlebrown",          "8b4513",
		"salmon",               "fa8072",
		"sandybrown",           "f4a460",
		"seagreen",             "2e8b57",
		"seashell",             "fff5ee",
		"sienna",               "a0522d",
		"skyblue",              "87ceeb",
		"slateblue",            "6a5acd",
		"slategray",            "708090",
		"snow",                 "fffafa",
		"springgreen",          "00ff7f",
		"steelblue",            "4682b4",
		"tan",                  "d2b48c",
		"thistle",              "d8bfd8",
		"tomato",               "ff6347",
		"turquoise",            "40e0d0",
		"violet",               "ee82ee",
		"wheat",                "f5deb3",
		"whitesmoke",           "f5f5f5",
		"yellowgreen",          "9acd32"
	};

	QString colorHex = color().name().mid(1); // Take the hexadecimal name of the color, without the '#'.

	bool cssColorFound = false;
	for (int i = 0; i < 2*16; i += 2) {
		if (colorHex == cssColors[i+1]) {
			keys->append(i18n("CSS Color Name"));
			values->append(cssColors[i]);
			cssColorFound = true;
			break;
		}
	}

	if (!cssColorFound)
		for (int i = 0; i < 2*124; i += 2) {
			if (colorHex == cssExtendedColors[i+1]) {
				keys->append(i18n("CSS Extended Color Name"));
				values->append(cssExtendedColors[i]);
				break;
			}
		}

	keys->append(i18n("Is Web Color"));
	values->append(Tools::isWebColor(color()) ? i18n("Yes") : i18n("No"));

}

void ColorContent::setColor(const QColor &color)
{
	m_color = color;

	QRect textRect = QFontMetrics(note()->font()).boundingRect(color.name());
	int rectHeight = (textRect.height() + 2)*3/2;
	int rectWidth  = rectHeight * 14 / 10; // 1.4 times the height, like A4 papers.
	contentChanged(rectWidth + RECT_MARGIN + textRect.width() + RECT_MARGIN); // The second RECT_MARGIN is because textRect.width() is too short. I done a bug? Can't figure out.
}

void ColorContent::addAlternateDragObjects(K3MultipleDrag *dragObject)
{
	dragObject->addDragObject( new Q3ColorDrag(color()) );

//	addDragObject(new K3ColorDrag( note->color(), 0 ));
//	addDragObject(new QTextDrag( note->color().name(), 0 ));

/*	// Creata and add the QDragObject:
	storedDrag = new QStoredDrag("application/x-color");
	storedDrag->setEncodedData(*array);
	dragObject->addDragObject(storedDrag);
	delete array;*/
}

void ColorContent::exportToHTML(HTMLExporter *exporter, int /*indent*/)
{
	// FIXME: Duplicate from setColor(): TODO: rectSize()
	QRect textRect = QFontMetrics(note()->font()).boundingRect(color().name());
	int rectHeight = (textRect.height() + 2)*3/2;
	int rectWidth  = rectHeight * 14 / 10; // 1.4 times the height, like A4 papers.

	QString fileName = /*Tools::fileNameForNewFile(*/QString("color_%1.png").arg(color().name().toLower().mid(1))/*, exportData.iconsFolderPath)*/;
	QString fullPath = exporter->iconsFolderPath + fileName;
	QPixmap colorIcon(rectWidth, rectHeight);
	QPainter painter(&colorIcon);
	painter.setBrush(color());
	painter.drawRoundedRect(0, 0, rectWidth, rectHeight, 2, 2);
	colorIcon.save(fullPath, "PNG");
	QString iconHtml = QString("<img src=\"%1\" width=\"%2\" height=\"%3\" alt=\"\">")
	                   .arg(exporter->iconsFolderName + fileName, QString::number(colorIcon.width()), QString::number(colorIcon.height()));

	exporter->stream << iconHtml + " " + color().name();
}



/** class UnknownContent:
 */

const int UnknownContent::DECORATION_MARGIN = 2;

UnknownContent::UnknownContent(Note *parent, const QString &fileName)
 : NoteContent(parent, fileName)
{
	basket()->addWatchedFile(fullPath());
	loadFromFile(/*lazyLoad=*/false);
}

int UnknownContent::setWidthAndGetHeight(int width)
{
	width -= 1;
	QRect textRect = QFontMetrics(note()->font()).boundingRect(0, 0, width, 500000, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, m_mimeTypes);
	return DECORATION_MARGIN + textRect.height() + DECORATION_MARGIN;
}

// TODO: Move this function from note.cpp to class Tools:
extern void drawGradient( QPainter *p, const QColor &colorTop, const QColor & colorBottom,
						  int x, int y, int w, int h,
						  bool sunken, bool horz, bool flat  ); /*const*/

void UnknownContent::paint(QPainter *painter, int width, int height, const QColorGroup &colorGroup, bool /*isDefaultColor*/, bool /*isSelected*/, bool /*isHovered*/)
{
	width -= 1;
	painter->setPen(colorGroup.text());

	// FIXME: Duplicate from ColorContent::paint() and CommonColorSelector::drawColorRect:
	// Fill with gradient:
	drawGradient(painter, colorGroup.background(), colorGroup.background().dark(110), 1, 1, width - 2, height - 2, /*sunken=*/false, /*horz=*/true, /*flat=*/false);
	// Stroke:
	QColor stroke = Tools::mixColor(colorGroup.background(), colorGroup.text());
	painter->setPen(stroke);
	painter->drawLine(1,         0,          width - 2, 0);
	painter->drawLine(0,         1,          0,         height - 2);
	painter->drawLine(1,         height - 1, width - 2, height - 1);
	painter->drawLine(width - 1, 1,          width - 1, height - 2);
	// Round corners:
	painter->setPen(Tools::mixColor(colorGroup.background(), stroke));
	painter->drawPoint(1,         1);
	painter->drawPoint(1,         height - 2);
	painter->drawPoint(width - 2, height - 2);
	painter->drawPoint(width - 2, 1);

	painter->setPen(colorGroup.text());
	painter->drawText(DECORATION_MARGIN, DECORATION_MARGIN, width - 2*DECORATION_MARGIN, height - 2*DECORATION_MARGIN,
	                  Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap, m_mimeTypes);
}

bool UnknownContent::loadFromFile(bool /*lazyLoad*/)
{
	DEBUG_WIN << "Loading UnknownContent From " + basket()->folderName() + fileName();
	QFile file(fullPath());
	if (file.open(QIODevice::ReadOnly)) {
		QDataStream stream(&file);
		QString line;
		m_mimeTypes = "";
		// Get the MIME-types names:
		do {
			if (!stream.atEnd()) {
				stream >> line;
				if (!line.isEmpty()) {
					if (m_mimeTypes.isEmpty())
						m_mimeTypes += line;
					else
						m_mimeTypes += QString("\n") + line;
				}
			}
		} while (!line.isEmpty() && !stream.atEnd());
		file.close();
	}

	QRect textRect = QFontMetrics(note()->font()).boundingRect(0, 0, /*width=*/1, 500000, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, m_mimeTypes);
	contentChanged(DECORATION_MARGIN + textRect.width() + DECORATION_MARGIN + 1);
	return true;
}

void UnknownContent::addAlternateDragObjects(K3MultipleDrag *dragObject)
{
	QFile file(fullPath());
	if (file.open(QIODevice::ReadOnly)) {
		QDataStream stream(&file);
		// Get the MIME types names:
		Q3ValueList<QString> mimes;
		QString line;
		do {
			if (!stream.atEnd()) {
				stream >> line;
				if (!line.isEmpty())
					mimes.append(line);
			}
		} while (!line.isEmpty() && !stream.atEnd());
		// Add the streams:
		quint64     size; // TODO: It was quint32 in version 0.5.0 !
		QByteArray  *array;
		Q3StoredDrag *storedDrag;
		for (int i = 0; i < mimes.count(); ++i) {
			// Get the size:
			stream >> size;
			// Allocate memory to retreive size bytes and store them:
			array = new QByteArray(size);
			stream.readRawBytes(array->data(), size);
			// Creata and add the QDragObject:
			storedDrag = new Q3StoredDrag(mimes.at(i)->toAscii());
			storedDrag->setEncodedData(*array);
			dragObject->addDragObject(storedDrag);
			delete array; // FIXME: Should we?
		}
		file.close();
	}
}

void UnknownContent::exportToHTML(HTMLExporter *exporter, int indent)
{
	QString spaces;
	exporter->stream << "<div class=\"unknown\">" << mimeTypes().replace("\n", "\n" + spaces.fill(' ', indent + 1 + 1)) << "</div>";
}




void NoteFactory__loadNode(const QDomElement &content, const QString &lowerTypeName, Note *parent, bool lazyLoad)
{
	if        (lowerTypeName == "text")      new TextContent(      parent, content.text(), lazyLoad );
	else if   (lowerTypeName == "html")      new HtmlContent(      parent, content.text(), lazyLoad );
	else if   (lowerTypeName == "image")     new ImageContent(     parent, content.text(), lazyLoad );
	else if   (lowerTypeName == "animation") new AnimationContent( parent, content.text(), lazyLoad );
	else if   (lowerTypeName == "sound")     new SoundContent(     parent, content.text()           );
	else if   (lowerTypeName == "file")      new FileContent(      parent, content.text()           );
	else if   (lowerTypeName == "link") {
		bool autoTitle = content.attribute("title") == content.text();
		bool autoIcon  = content.attribute("icon")  == NoteFactory::iconForURL(KUrl(content.text()));
		autoTitle = XMLWork::trueOrFalse( content.attribute("autoTitle"), autoTitle);
		autoIcon  = XMLWork::trueOrFalse( content.attribute("autoIcon"),  autoIcon );
		new LinkContent( parent, KUrl(content.text()), content.attribute("title"), content.attribute("icon"), autoTitle, autoIcon );
	} else if (lowerTypeName == "launcher")  new LauncherContent(  parent, content.text()         );
	else if   (lowerTypeName == "color")     new ColorContent(     parent, QColor(content.text()) );
	else if   (lowerTypeName == "unknown")   new UnknownContent(   parent, content.text()         );
}


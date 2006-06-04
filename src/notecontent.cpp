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

#include <qfile.h>
#include <qdir.h>
#include <qdom.h>
#include <qpainter.h>
#include <qstylesheet.h>
#include <qfontmetrics.h>
#include <qwidget.h>
#include <qcursor.h>
#include <qstringlist.h>
#include <ktextedit.h>
#include <kservice.h>
#include <kcolordialog.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kpixmapeffect.h>
#include <kurifilter.h>
#include <kstringhandler.h>
#include <kfilemetainfo.h>
#include <qdatetime.h>
#include <kmultipledrag.h>

#include <qfileinfo.h>
//#include <kio/kfileitem.h>
#include <kfileitem.h>
#include <kio/previewjob.h>
#include <kio/global.h>

#include <iostream>

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

#include "config.h"
#ifndef WITHOUT_ARTS
	#include <arts/kplayobject.h>
	#include <arts/kplayobjectfactory.h>
	#include <arts/kartsserver.h>
	#include <arts/kartsdispatcher.h>
#endif

/** Helping functions:
 */

QString loadFileToString(const QString &fileName) // TODO: Tools::loadTextFile()     With encoding...
{
	QFile file(fileName);
	if (file.open(IO_ReadOnly)) {
		QTextStream stream(&file);
		QString text;
		text = stream.read();
		file.close();
		return text;
	} else
		return "";
}

void saveStringAsFile(const QString &text, const QString &fileName) // TODO: Tools::saveTextFile()     + With encoding...  + KSaveFile    + With Disk Space checking
{
	QFile file(fileName);
	if (file.open(IO_WriteOnly)) {
		QTextStream stream(&file);
		stream << text;
		file.close();
	}
}

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

KURL NoteContent::urlToOpen(bool /*with*/)
{
	return (useFile() ? KURL(fullPath()) : KURL());
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

QString TextContent::typeName()      { return i18n("Text");      }
QString HtmlContent::typeName()      { return i18n("Rich Text"); }
QString ImageContent::typeName()     { return i18n("Image");     }
QString AnimationContent::typeName() { return i18n("Animation"); }
QString SoundContent::typeName()     { return i18n("Sound");     }
QString FileContent::typeName()      { return i18n("File");      }
QString LinkContent::typeName()      { return i18n("Link");      }
QString LauncherContent::typeName()  { return i18n("Launcher");  }
QString ColorContent::typeName()     { return i18n("Color");     }
QString UnknownContent::typeName()   { return i18n("Unknown");   }

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
		return url().prettyURL();
	else if (title().isEmpty() && url().isEmpty())
		return "";
	else if (url().isEmpty())
		return title();
	else if (title().isEmpty())
		return url().prettyURL();
	else
		return QString("%1 <%2>").arg(title(), url().prettyURL());
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
{ return QString("<a href=\"%1\">%2</a>").arg(url().prettyURL(), title());                        } // With the icon?

QString LauncherContent::toHtml(const QString &/*imageName*/, const QString &cuttedFullPath)
{ return QString("<a href=\"%1\">%2</a>").arg((cuttedFullPath.isEmpty() ? fullPath() : cuttedFullPath), name());     } // With the icon?

QString ColorContent::toHtml(const QString &/*imageName*/, const QString &/*cuttedFullPath*/)
{ return QString("<span style=\"color: %1\">%2</span>").arg(color().name(), color().name());  }

QString UnknownContent::toHtml(const QString &/*imageName*/, const QString &/*cuttedFullPath*/)
{ return "";                                                                                  }

QPixmap ImageContent::toPixmap()     { return pixmap();              }
QPixmap AnimationContent::toPixmap() { return movie().framePixmap(); }

void NoteContent::toLink(KURL *url, QString *title, const QString &cuttedFullPath)
{
	if (useFile()) {
		*url   = KURL(cuttedFullPath.isEmpty() ? fullPath() : cuttedFullPath);
		*title =     (cuttedFullPath.isEmpty() ? fullPath() : cuttedFullPath);
	} else {
		*url   = KURL();
		*title = QString();
	}
}
void LinkContent::toLink(KURL *url, QString *title, const QString &/*cuttedFullPath*/)
{
	*url   = this->url();
	*title = this->title();
}

void LauncherContent::toLink(KURL *url, QString *title, const QString &cuttedFullPath)
{
	*url   = KURL(cuttedFullPath.isEmpty() ? fullPath() : cuttedFullPath);
	*title = name();
}
void UnknownContent::toLink(KURL *url, QString *title, const QString &/*cuttedFullPath*/)
{
	*url   = KURL();
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
QString SoundContent::saveAsFilters()     { return "audio/x-mp3";           } // TODO: OGG...
QString FileContent::saveAsFilters()      { return "*";                     } // TODO: Get MIME type of the url target
QString LinkContent::saveAsFilters()      { return "*";                     } // TODO: idem File + If isDir(): return
QString LauncherContent::saveAsFilters()  { return "application/x-desktop"; }
QString ColorContent::saveAsFilters()     { return "";                      }
QString UnknownContent::saveAsFilters()   { return "";                      }

bool TextContent::match(const FilterData &data)          { return (text().find(data.string, /*index=*/0, /*cs=*/false) != -1);         }
bool HtmlContent::match(const FilterData &data)          { return (toText("").find(data.string, /*index=*/0, /*cs=*/false) != -1);     }
bool ImageContent::match(const FilterData &/*data*/)     { return false;                                                               }
bool AnimationContent::match(const FilterData &/*data*/) { return false;                                                               }
bool SoundContent::match(const FilterData &data)         { return (fileName().find(data.string, /*index=*/0, /*cs=*/false) != -1);     }
bool FileContent::match(const FilterData &data)          { return (fileName().find(data.string, /*index=*/0, /*cs=*/false) != -1);     }
bool LinkContent::match(const FilterData &data)          { return (title().find(data.string, 0, false) != -1 || url().prettyURL().find(data.string, 0, false) != -1); }
bool LauncherContent::match(const FilterData &data)      { return (exec().find(data.string, 0, false) != -1 || name().find(data.string, 0, false) != -1); }
bool ColorContent::match(const FilterData &data)         { return (color().name().find(data.string, /*index=*/0, /*cs=*/false) != -1); }
bool UnknownContent::match(const FilterData &data)       { return (mimeTypes().find(data.string, /*index=*/0, /*cs=*/false) != -1);    }

QString TextContent::editToolTipText()      { return i18n("Edit this text");                   }
QString HtmlContent::editToolTipText()      { return i18n("Edit this rich text");              }
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
void AnimationContent::fontChanged() { setMovie(movie());                                        }
void FileContent::fontChanged()      { setFileName(fileName());                                  }
void LinkContent::fontChanged()      { setLink(url(), title(), icon(), autoTitle(), autoIcon()); }
void LauncherContent::fontChanged()  { setLauncher(name(), icon(), exec());                      }
void ColorContent::fontChanged()     { setColor(color());                                        }
void UnknownContent::fontChanged()   { loadFromFile();                                           } // TODO: Optimize: setMimeTypes()

QString TextContent::customOpenCommand()      { return (Settings::isTextUseProg()      && ! Settings::textProg().isEmpty()      ? Settings::textProg()      : QString()); }
QString HtmlContent::customOpenCommand()      { return (Settings::isHtmlUseProg()      && ! Settings::htmlProg().isEmpty()      ? Settings::htmlProg()      : QString()); }
QString ImageContent::customOpenCommand()     { return (Settings::isImageUseProg()     && ! Settings::imageProg().isEmpty()     ? Settings::imageProg()     : QString()); }
QString AnimationContent::customOpenCommand() { return (Settings::isAnimationUseProg() && ! Settings::animationProg().isEmpty() ? Settings::animationProg() : QString()); }
QString SoundContent::customOpenCommand()     { return (Settings::isSoundUseProg()     && ! Settings::soundProg().isEmpty()     ? Settings::soundProg()     : QString()); }

void LinkContent::serialize(QDataStream &stream)  { stream << url() << title() << icon() << (Q_UINT64)autoTitle() << (Q_UINT64)autoIcon(); }
void ColorContent::serialize(QDataStream &stream) { stream << color();  }

QPixmap TextContent::feedbackPixmap(int width, int height)
{
	QRect textRect = QFontMetrics(note()->font()).boundingRect(0, 0, width, height, Qt::AlignAuto | Qt::AlignTop | Qt::WordBreak, text());
	QPixmap pixmap( QMIN(width, textRect.width()), QMIN(height, textRect.height()) );
	pixmap.fill(note()->backgroundColor().dark(FEEDBACK_DARKING));
	QPainter painter(&pixmap);
	painter.setPen(note()->textColor());
	painter.setFont(note()->font());
	painter.drawText(0, 0, pixmap.width(), pixmap.height(), Qt::AlignAuto | Qt::AlignTop | Qt::WordBreak, text());
	painter.end();
	return pixmap;
}

QPixmap HtmlContent::feedbackPixmap(int width, int height)
{
	QSimpleRichText richText(html(), note()->font());
	richText.setWidth(width);
	QColorGroup colorGroup(basket()->colorGroup());
	colorGroup.setColor(QColorGroup::Text,       note()->textColor());
	colorGroup.setColor(QColorGroup::Background, note()->backgroundColor().dark(FEEDBACK_DARKING));
	QPixmap pixmap( QMIN(width, richText.widthUsed()), QMIN(height, richText.height()) );
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
		pmScaled.convertFromImage(imageToScale./*smoothScale*/scale(width, height, QImage::ScaleMin));
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
	QPixmap pixmap = m_movie.framePixmap();
	if (width >= pixmap.width() && height >= pixmap.height()) // Full size
		return pixmap;
	else { // Scalled down
		QImage imageToScale = pixmap.convertToImage();
		QPixmap pmScaled;
		pmScaled.convertFromImage(imageToScale./*smoothScale*/scale(width, height, QImage::ScaleMin));
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

	QPixmap pixmap( QMIN(width, rectWidth + RECT_MARGIN + textRect.width() + RECT_MARGIN), QMIN(height, rectHeight) );
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
	QRect textRect = QFontMetrics(note()->font()).boundingRect(0, 0, /*width=*/1, 500000, Qt::AlignAuto | Qt::AlignTop | Qt::WordBreak, m_mimeTypes);

	QColorGroup colorGroup(basket()->colorGroup());
	colorGroup.setColor(QColorGroup::Text,       note()->textColor());
	colorGroup.setColor(QColorGroup::Background, note()->backgroundColor().dark(FEEDBACK_DARKING));

	QPixmap pixmap( QMIN(width, DECORATION_MARGIN + textRect.width() + DECORATION_MARGIN), QMIN(height, DECORATION_MARGIN + textRect.height() + DECORATION_MARGIN) );
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

TextContent::TextContent(Note *parent, const QString &fileName)
 : NoteContent(parent, fileName), m_simpleRichText(0)
{
	loadFromFile();
}

TextContent::~TextContent()
{
	delete m_simpleRichText;
}

int TextContent::setWidthAndGetHeight(int width)
{
	width -= 1;
	m_simpleRichText->setWidth(width);
	return m_simpleRichText->height();
}

void TextContent::paint(QPainter *painter, int width, int height, const QColorGroup &colorGroup, bool /*isDefaultColor*/, bool /*isSelected*/, bool /*isHovered*/)
{
	width -= 1;
	m_simpleRichText->draw(painter, 0, 0, QRect(0, 0, width, height), colorGroup);
}

void TextContent::loadFromFile()
{
	DEBUG_WIN << "Loading TextContent From " + basket()->folderName() + fileName();
	setText(loadFileToString(fullPath()));
}

void TextContent::saveToFile()
{
	saveStringAsFile(text(), fullPath());
}

QString TextContent::linkAt(const QPoint &pos)
{
	return m_simpleRichText->anchorAt(pos);
}


QString TextContent::messageWhenOpenning(OpenMessage where)
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

void TextContent::setText(const QString &text)
{
	m_text = text;
	int width = (m_simpleRichText ? m_simpleRichText->width() : 1);
	delete m_simpleRichText;
	QString html = "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body>" + KStringHandler::tagURLs(Tools::textToHTML(text)); // Don't collapse multiple spaces!
	m_simpleRichText = new QSimpleRichText(html, note()->font());
	m_simpleRichText->setWidth(1); // We put a width of 1 pixel, so usedWidth() is egual to the minimum width
	int minWidth = m_simpleRichText->widthUsed();
	m_simpleRichText->setWidth(width);
	contentChanged(minWidth + 1);
}

void TextContent::exportToHTML(QTextStream &stream, int indent, const HtmlExportData &/*exportData*/)
{
	QString spaces;
	QString html = "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body>" +
	               KStringHandler::tagURLs(Tools::textToHTMLWithoutP(text().replace("\t", "                "))); // Don't collapse multiple spaces!
	stream << html.replace("  ", " &nbsp;").replace("\n", "\n" + spaces.fill(' ', indent + 1));
}

/** class HtmlContent:
 */

HtmlContent::HtmlContent(Note *parent, const QString &fileName)
 : NoteContent(parent, fileName), m_simpleRichText(0)
{
	loadFromFile();
}

HtmlContent::~HtmlContent()
{
	delete m_simpleRichText;
}

int HtmlContent::setWidthAndGetHeight(int width)
{
	width -= 1;
	m_simpleRichText->setWidth(width);
	return m_simpleRichText->height();
}

void HtmlContent::paint(QPainter *painter, int width, int height, const QColorGroup &colorGroup, bool /*isDefaultColor*/, bool /*isSelected*/, bool /*isHovered*/)
{
	width -= 1;
	m_simpleRichText->draw(painter, 0, 0, QRect(0, 0, width, height), colorGroup);
}

void HtmlContent::loadFromFile()
{
	DEBUG_WIN << "Loading HtmlContent From " + basket()->folderName() + fileName();
	setHtml(loadFileToString(fullPath()));
}

void HtmlContent::saveToFile()
{
	saveStringAsFile(html(), fullPath());
}

QString HtmlContent::linkAt(const QPoint &pos)
{
	return m_simpleRichText->anchorAt(pos);
}


QString HtmlContent::messageWhenOpenning(OpenMessage where)
{
	switch (where) {
		case OpenOne:               return i18n("Opening rich text...");
		case OpenSeveral:           return i18n("Opening rich texts...");
		case OpenOneWith:           return i18n("Opening rich text with...");
		case OpenSeveralWith:       return i18n("Opening rich texts with...");
		case OpenOneWithDialog:     return i18n("Open rich text with:");
		case OpenSeveralWithDialog: return i18n("Open rich texts with:");
		default:                    return "";
	}
}

void HtmlContent::setHtml(const QString &html)
{
	m_html = html;
	int width = (m_simpleRichText ? m_simpleRichText->width() : 1);
	delete m_simpleRichText;
	m_simpleRichText = new QSimpleRichText(KStringHandler::tagURLs(html), note()->font());
	m_simpleRichText->setWidth(1); // We put a width of 1 pixel, so usedWidth() is egual to the minimum width
	int minWidth = m_simpleRichText->widthUsed();
	m_simpleRichText->setWidth(width);
	contentChanged(minWidth + 1);
}

void HtmlContent::exportToHTML(QTextStream &stream, int indent, const HtmlExportData &/*exportData*/)
{
	QString spaces;
	stream << Tools::htmlToParagraph(KStringHandler::tagURLs(html().replace("\t", "                ")))
	          .replace("  ", " &nbsp;")
	          .replace("\n", "\n" + spaces.fill(' ', indent + 1));
}

/** class ImageContent:
 */

ImageContent::ImageContent(Note *parent, const QString &fileName)
 : NoteContent(parent, fileName), m_format(0)
{
	loadFromFile();
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
//		pixmap = KPixmapEffect::selectedPixmap(m_pixmap, KGlobalSettings::highlightColor());

	if (width >= m_pixmap.width()) // Full size
		painter->drawPixmap(0, 0, m_pixmap);
	else { // Scalled down
		double scale = ((double)width) / m_pixmap.width();
		painter->scale(scale, scale);
		painter->drawPixmap(0, 0, m_pixmap);  // TODO: Smooth !!!
	}
}

void ImageContent::loadFromFile()
{
	DEBUG_WIN << "Loading ImageContent From " + basket()->folderName() + fileName();
	m_format = (char* /* from const char* */)QPixmap::imageFormat(fullPath()); // See QImageIO to know what formats can be supported.
	if (m_format)
		m_pixmap.load(fullPath());
	else
		m_format = (char*)"PNG"; // If the image is set later, it should be saved without destruction, so we use PNG by default.
	setPixmap(m_pixmap);
}


void ImageContent::saveToFile()
{
	m_pixmap.save(fullPath(), m_format);
}


void ImageContent::toolTipInfos(QStringList *keys, QStringList *values)
{
	keys->append(i18n("Size"));
	values->append(i18n("%1 by %2 pixels").arg(QString::number(m_pixmap.width()), QString::number(m_pixmap.height())));
}

QString ImageContent::messageWhenOpenning(OpenMessage where)
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

void ImageContent::exportToHTML(QTextStream &stream, int /*indent*/, const HtmlExportData &exportData)
{
	int width  = m_pixmap.width();
	int height = m_pixmap.height();
	int contentWidth = note()->width() - note()->contentX() - 1 - Note::NOTE_MARGIN;

	QString imageName = Basket::copyFile(fullPath(), exportData.dataFolderPath, /*createIt=*/true);

	if (contentWidth <= m_pixmap.width()) { // Scalled down
		double scale = ((double)contentWidth) / m_pixmap.width();
		width  = (int)(m_pixmap.width()  * scale);
		height = (int)(m_pixmap.height() * scale);
		stream << "<a href=\"" << exportData.dataFolderName << imageName << "\" title=\"" << i18n("Click for full size view") << "\">";
	}

	stream << "<img src=\"" << exportData.dataFolderName << imageName
	       << "\" width=\"" << width << "\" height=\"" << height << "\" alt=\"\">";

	if (contentWidth <= m_pixmap.width()) // Scalled down
		stream << "</a>";
}

/** class AnimationContent:
 */

int AnimationContent::INVALID_STATUS = -100;

AnimationContent::AnimationContent(Note *parent, const QString &fileName)
 : NoteContent(parent, fileName), m_oldStatus(INVALID_STATUS)
{
	loadFromFile();
}

int AnimationContent::setWidthAndGetHeight(int /*width*/)
{
	/*width -= 1*/;
	return  m_movie.framePixmap().height()  ; // TODO!!!
}

void AnimationContent::paint(QPainter *painter, int width, int /*height*/, const QColorGroup &/*colorGroup*/, bool /*isDefaultColor*/, bool /*isSelected*/, bool /*isHovered*/)
{
	/*width -= 1*/;
//	DEBUG_WIN << "AnimationContent::paint()";
	const QPixmap &frame = m_movie.framePixmap();
	if (width >= frame.width()) // Full size
		painter->drawPixmap(0, 0, frame);
	else // Scalled down
		painter->drawPixmap(0, 0, frame);  // TODO: Scall down
}

void AnimationContent::loadFromFile()
{
	DEBUG_WIN << "Loading MovieContent From " + basket()->folderName() + fileName();
	setMovie(QMovie(fullPath()));
}


void AnimationContent::saveToFile()
{
	// Impossible!
}


QString AnimationContent::messageWhenOpenning(OpenMessage where)
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

void AnimationContent::setMovie(const QMovie &movie)
{
	if (!m_movie.isNull()) {
		// Disconnect?
	}
	m_movie = movie;
	m_movie.connectUpdate( this, SLOT(movieUpdated(const QRect&)) );
	m_movie.connectResize( this, SLOT(movieResized(const QSize&)) );
	m_movie.connectStatus( this, SLOT(movieStatus(int))           );
	contentChanged(  m_movie.framePixmap().width() + 1  ); // TODO
}

void AnimationContent::movieUpdated(const QRect&)
{
	note()->unbufferize();
	note()->update();
}

void AnimationContent::movieResized(const QSize&)
{
	note()->requestRelayout(); // ?
}

/** When a user drop a .gif file, for instance, we don't know if it is an image
  * or an animtion (gif file contain multiple images).
  * To determin that, we assume this is an animation and count the number of images.
  * QMovie send, in this order:
  * - For a unique image: QMovie::EndOfFrame, QMovie::EndOfLoop, QMovie::EndOfMovie.
  * - For animation:      QMovie::EndOfFrame... (for each image), QMovie::EndOfLoop,
  *                       and it then restart that for each loop.
  */
void AnimationContent::movieStatus(int status)
{
	DEBUG_WIN << "movieStatus()";

	// At least two frames: it's an animation, everything is OK
	if (m_oldStatus == QMovie::EndOfFrame && status == QMovie::EndOfFrame) {
		movie().disconnectStatus(this);
		m_oldStatus = INVALID_STATUS;
//		if (note()->isFocused())                 // When inserting a new note we ensure it visble
//			basket()->ensureNoteVisible(note()); //  But after loading it has certainly grown and if it was
	}
	// Only one image: it's an image, change note's type
	else if (m_oldStatus == QMovie::EndOfFrame && status == QMovie::EndOfLoop) {
		movie().disconnectStatus(this);
		m_oldStatus = INVALID_STATUS;
		note()->setContent(new ImageContent(note(), fileName()));
		basket()->save();
		//delete this; // CRASH, as always !!!!!!!!!
		//QTimer::singleShot(0,   this, SLOT(loadContent()));    // Delayed to avoid crash!
		//QTimer::singleShot(100, this, SLOT(saveProperties())); // We should save it's an image and not an animation
//		if (note()->isFocused())
//			QTimer::singleShot(25, note(), SLOT(delayedEnsureVisible()));
	}
	else
		m_oldStatus = status;
}

void AnimationContent::exportToHTML(QTextStream &stream, int /*indent*/, const HtmlExportData &exportData)
{
	stream << QString("<img src=\"%1\" width=\"%2\" height=\"%3\" alt=\"\">")
	          .arg( exportData.dataFolderName + Basket::copyFile(fullPath(), exportData.dataFolderPath, /*createIt=*/true),
	                QString::number(movie().framePixmap().size().width()),
	                QString::number(movie().framePixmap().size().height()) );
}

/** class FileContent:
 */

FileContent::FileContent(Note *parent, const QString &fileName)
 : NoteContent(parent, fileName), m_previewJob(0)
{
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

void FileContent::loadFromFile()
{
	setFileName(fileName()); // File changed: get new file preview!
}

void FileContent::toolTipInfos(QStringList *keys, QStringList *values)
{
	// Get the size of the file:
	uint size = QFileInfo(fullPath()).size();
	QString humanFileSize = KIO::convertSize((KIO::filesize_t)size);

	keys->append(i18n("Size"));
	values->append(humanFileSize);

	KMimeType::Ptr mime = KMimeType::findByURL(KURL(fullPath()));
	if (mime) {
		keys->append(i18n("Type"));
		values->append(mime->comment());
	}

	KFileMetaInfo infos = KFileMetaInfo(KURL(fullPath()));
	if (infos.isValid() && !infos.isEmpty()) {
		QStringList groups = infos.preferredKeys();
		int i = 0;
		for (QStringList::Iterator it = groups.begin(); i < 6 && it != groups.end(); ++it) {
			KFileMetaInfoItem metaInfoItem = infos.item(*it);
			if (!metaInfoItem.string().isEmpty()) {
				keys->append(metaInfoItem.translatedKey());
				values->append(metaInfoItem.string());
				++i;
			}
		}
	}
}

int FileContent::zoneAt(const QPoint &pos)
{
	return (m_linkDisplay.iconButtonAt(pos) ? Note::Custom0 : 0);
}

QRect FileContent::zoneRect(int zone, const QPoint &/*pos*/)
{
	QRect linkRect = m_linkDisplay.iconButtonRect();

	if (zone == Note::Content)
		return QRect(linkRect.width(), 0, note()->width(), note()->height()); // Too wide and height, but it will be clipped by Note::zoneRect()
	else if (zone == Note::Custom0)
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


QString FileContent::messageWhenOpenning(OpenMessage where)
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
	KURL url = KURL(fullPath());
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
	m_linkDisplay.setLink(fileName(), NoteFactory::iconForURL(KURL(fullPath())), (linkLook->previewEnabled() ? preview : QPixmap()), linkLook, note()->font());
	contentChanged(m_linkDisplay.minWidth());
}

void FileContent::removePreview(const KFileItem*)
{
	newPreview(0, QPixmap());
}

void FileContent::startFetchingUrlPreview()
{
	KURL url(fullPath());
	LinkLook *linkLook = this->linkLook();

//	delete m_previewJob;
	if (!url.isEmpty() && linkLook->previewSize() > 0) {
		KURL filteredUrl = NoteFactory::filteredURL(url);//KURIFilter::self()->filteredURI(url);
		KURL::List urlList;
		urlList.append(filteredUrl);
		m_previewJob = KIO::filePreview(urlList, linkLook->previewSize(), linkLook->previewSize(), linkLook->iconSize());
		connect( m_previewJob, SIGNAL(gotPreview(const KFileItem*, const QPixmap&)), this, SLOT(newPreview(const KFileItem*, const QPixmap&)) );
		connect( m_previewJob, SIGNAL(failed(const KFileItem*)),                     this, SLOT(removePreview(const KFileItem*))              );
	}
}

void FileContent::exportToHTML(QTextStream &stream, int indent, const HtmlExportData &exportData)
{
	QString spaces;
	QString fileName = Basket::copyFile(fullPath(), exportData.dataFolderPath, true);
	stream << m_linkDisplay.toHtml(exportData, KURL(exportData.dataFolderName + fileName), "").replace("\n", "\n" + spaces.fill(' ', indent + 1));
}

/** class SoundContent:
 */

SoundContent::SoundContent(Note *parent, const QString &fileName)
 : FileContent(parent, fileName)
{
	setFileName(fileName); // FIXME: TO THAT HERE BECAUSE NoteContent() constructor seems to don't be able to call virtual methods???
}


QString SoundContent::zoneTip(int zone)
{
	return (zone == Note::Custom0 ? i18n("Open this sound") : QString());
}

void SoundContent::setHoveredZone(int oldZone, int newZone)
{
#ifdef WITHOUT_ARTS
	Q_UNUSED(oldZone);
	if (newZone == Note::Custom0 || newZone == Note::Content)
		std::cout << "Compiled without aRts: sound is not played." << std::endl;
#else
	static KArtsDispatcher        *s_dispatcher  = new KArtsDispatcher(); // Needed for s_playObj (we don't use it directly)
	static KArtsServer            *s_playServer  = new KArtsServer();
	static KDE::PlayObjectFactory *s_playFactory = new KDE::PlayObjectFactory(s_playServer);
	static KDE::PlayObject        *s_playObj     = 0;

	Q_UNUSED(s_dispatcher); // Avoid the compiler to tell us it is not used!
	if (newZone == Note::Custom0 || newZone == Note::Content) {
		// Start the sound preview:
		if (oldZone != Note::Custom0 && oldZone != Note::Content) { // Don't restart if it was already in one of those zones
			s_playObj = s_playFactory->createPlayObject(fullPath(), true);
			s_playObj->play();
		}
	} else {
		// Stop the sound preview, if it was started:
		if (s_playObj) {
			s_playObj->halt();
			delete s_playObj;
			s_playObj = 0;
		}
	}
#endif
}


QString SoundContent::messageWhenOpenning(OpenMessage where)
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

LinkContent::LinkContent(Note *parent, const KURL &url, const QString &title, const QString &icon, bool autoTitle, bool autoIcon)
 : NoteContent(parent), m_previewJob(0)
{
	setLink(url, title, icon, autoTitle, autoIcon);
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
	QDomText textNode = doc.createTextNode(url().prettyURL());
	content.appendChild(textNode);
}


void LinkContent::toolTipInfos(QStringList *keys, QStringList *values)
{
	keys->append(i18n("Target"));
	values->append(m_url.prettyURL());
}

int LinkContent::zoneAt(const QPoint &pos)
{
	return (m_linkDisplay.iconButtonAt(pos) ? Note::Custom0 : 0);
}

QRect LinkContent::zoneRect(int zone, const QPoint &/*pos*/)
{
	QRect linkRect = m_linkDisplay.iconButtonRect();

	if (zone == Note::Content)
		return QRect(linkRect.width(), 0, note()->width(), note()->height()); // Too wide and height, but it will be clipped by Note::zoneRect()
	else if (zone == Note::Custom0)
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
		return m_url.prettyURL();
	else
		return "";
}


KURL LinkContent::urlToOpen(bool /*with*/)
{
	return NoteFactory::filteredURL(url());//KURIFilter::self()->filteredURI(url());
}

QString LinkContent::messageWhenOpenning(OpenMessage where)
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

void LinkContent::setLink(const KURL &url, const QString &title, const QString &icon, bool autoTitle, bool autoIcon)
{
	m_autoTitle = autoTitle;
	m_autoIcon  = autoIcon;
	m_url       = NoteFactory::filteredURL(KURL(url));//KURIFilter::self()->filteredURI(url);
	m_title     = (autoTitle ? NoteFactory::titleForURL(m_url) : title);
	m_icon      = (autoIcon  ? NoteFactory::iconForURL(m_url)  : icon);

	LinkLook *look = LinkLook::lookForURL(m_url);
	if (look->previewEnabled())
		m_linkDisplay.setLink(m_title, m_icon,            look, note()->font());
	else
		m_linkDisplay.setLink(m_title, m_icon, QPixmap(), look, note()->font());
	startFetchingUrlPreview();
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

// Code dupicated from FileContent::startFetchingUrlPreview()
void LinkContent::startFetchingUrlPreview()
{
	KURL url = this->url();
	LinkLook *linkLook = LinkLook::lookForURL(this->url());

//	delete m_previewJob;
	if (!url.isEmpty() && linkLook->previewSize() > 0) {
		KURL filteredUrl = NoteFactory::filteredURL(url);//KURIFilter::self()->filteredURI(url);
		KURL::List urlList;
		urlList.append(filteredUrl);
		m_previewJob = KIO::filePreview(urlList, linkLook->previewSize(), linkLook->previewSize(), linkLook->iconSize());
		connect( m_previewJob, SIGNAL(gotPreview(const KFileItem*, const QPixmap&)), this, SLOT(newPreview(const KFileItem*, const QPixmap&)) );
		connect( m_previewJob, SIGNAL(failed(const KFileItem*)),                     this, SLOT(removePreview(const KFileItem*))              );
	}
}

void LinkContent::exportToHTML(QTextStream &stream, int indent, const HtmlExportData &exportData)
{
	QString linkTitle = title();

	// Append address (useful for print version of the page/basket):
	if (exportData.formatForImpression && (!autoTitle() && title() != NoteFactory::titleForURL(url().prettyURL()))) {
		// The address is on a new line, unless title is empty (empty lines was replaced by &nbsp;):
		if (linkTitle == " "/*"&nbsp;"*/)
			linkTitle = url().prettyURL()/*""*/;
		else
			linkTitle = linkTitle + " <" + url().prettyURL() + ">"/*+ "<br>"*/;
		//linkTitle += "<i>" + url().prettyURL() + "</i>";
	}

	KURL linkURL;
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
		linkURL = url();
	}

	QString spaces;
	stream << m_linkDisplay.toHtml(exportData, linkURL, linkTitle).replace("\n", "\n" + spaces.fill(' ', indent + 1));
}

/** class LauncherContent:
 */

LauncherContent::LauncherContent(Note *parent, const QString &fileName)
 : NoteContent(parent, fileName)
{
	loadFromFile();
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

void LauncherContent::loadFromFile() // TODO: saveToFile() ?? Is it possible?
{
	DEBUG_WIN << "Loading LauncherContent From " + basket()->folderName() + fileName();
	KService service(fullPath());
	setLauncher(service.name(), service.icon(), service.exec());
}


void LauncherContent::toolTipInfos(QStringList *keys, QStringList *values)
{
	KService service(fullPath());

	QString exec = service.exec();
	if (service.terminal())
		exec = i18n("%1 <i>(run in terminal)</i>").arg(exec);

	if (!service.comment().isEmpty() && service.comment() != service.name()) {
		keys->append(i18n("Comment"));
		values->append(service.comment());
	}

	keys->append(i18n("Command"));
	values->append(exec);
}

int LauncherContent::zoneAt(const QPoint &pos)
{
	return (m_linkDisplay.iconButtonAt(pos) ? Note::Custom0 : 0);
}

QRect LauncherContent::zoneRect(int zone, const QPoint &/*pos*/)
{
	QRect linkRect = m_linkDisplay.iconButtonRect();

	if (zone == Note::Content)
		return QRect(linkRect.width(), 0, note()->width(), note()->height()); // Too wide and height, but it will be clipped by Note::zoneRect()
	else if (zone == Note::Custom0)
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


KURL LauncherContent::urlToOpen(bool with)
{
	if (KService(fullPath()).exec().isEmpty())
		return KURL();

	return (with ? KURL() : KURL(fullPath())); // Can open the appliation, but not with another application :-)
}

QString LauncherContent::messageWhenOpenning(OpenMessage where)
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

void LauncherContent::exportToHTML(QTextStream &stream, int indent, const HtmlExportData &exportData)
{
	QString spaces;
	QString fileName = Basket::copyFile(fullPath(), exportData.dataFolderPath, true);
	stream << m_linkDisplay.toHtml(exportData, KURL(exportData.dataFolderName + fileName), "").replace("\n", "\n" + spaces.fill(' ', indent + 1));
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
	painter->drawText(rectWidth + RECT_MARGIN, 0, width - rectWidth - RECT_MARGIN, height, Qt::AlignAuto | Qt::AlignVCenter, color().name());
}

void ColorContent::saveToNode(QDomDocument &doc, QDomElement &content)
{
	QDomText textNode = doc.createTextNode(color().name());
	content.appendChild(textNode);
}


void ColorContent::toolTipInfos(QStringList *keys, QStringList *values)
{
	int hue, saturation, value;
	m_color.getHsv(hue, saturation, value);

	keys->append(i18n("RGB Colorspace: Red/Green/Blue", "RGB"));
	values->append(i18n("<i>Red</i>: %1, <i>Green</i>: %2, <i>Blue</i>: %3,").arg(QString::number(m_color.red()), QString::number(m_color.green()), QString::number(m_color.blue())));

	keys->append(i18n("HSV Colorspace: Hue/Saturation/Value", "HSV"));
	values->append(i18n("<i>Hue</i>: %1, <i>Saturation</i>: %2, <i>Value</i>: %3,").arg(QString::number(hue), QString::number(saturation), QString::number(value)));

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

void ColorContent::addAlternateDragObjects(KMultipleDrag *dragObject)
{
	dragObject->addDragObject( new QColorDrag(color()) );

//	addDragObject(new KColorDrag( note->color(), 0 ));
//	addDragObject(new QTextDrag( note->color().name(), 0 ));

/*	// Creata and add the QDragObject:
	storedDrag = new QStoredDrag("application/x-color");
	storedDrag->setEncodedData(*array);
	dragObject->addDragObject(storedDrag);
	delete array;*/
}

void ColorContent::exportToHTML(QTextStream &stream, int /*indent*/, const HtmlExportData &exportData)
{
	// FIXME: Duplicate from setColor(): TODO: rectSize()
	QRect textRect = QFontMetrics(note()->font()).boundingRect(color().name());
	int rectHeight = (textRect.height() + 2)*3/2;
	int rectWidth  = rectHeight * 14 / 10; // 1.4 times the height, like A4 papers.

	QString fileName = /*Tools::fileNameForNewFile(*/QString("color_%1.png").arg(color().name().lower().mid(1))/*, exportData.iconsFolderPath)*/;
	QString fullPath = exportData.iconsFolderPath + fileName;
	QPixmap colorIcon = KColorCombo2::colorRectPixmap(color(), /*isDefault=*/false, rectWidth, rectHeight);
	colorIcon.save(fullPath, "PNG");
	QString iconHtml = QString("<img src=\"%1\" width=\"%2\" height=\"%3\" alt=\"\">")
	                   .arg(exportData.iconsFolderName + fileName, QString::number(colorIcon.width()), QString::number(colorIcon.height()));

	stream << iconHtml + " " + color().name();
}



/** class UnknownContent:
 */

const int UnknownContent::DECORATION_MARGIN = 2;

UnknownContent::UnknownContent(Note *parent, const QString &fileName)
 : NoteContent(parent, fileName)
{
	loadFromFile();
}

int UnknownContent::setWidthAndGetHeight(int width)
{
	width -= 1;
	QRect textRect = QFontMetrics(note()->font()).boundingRect(0, 0, width, 500000, Qt::AlignAuto | Qt::AlignTop | Qt::WordBreak, m_mimeTypes);
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
	                  Qt::AlignAuto | Qt::AlignVCenter | Qt::WordBreak, m_mimeTypes);
}

void UnknownContent::loadFromFile()
{
	DEBUG_WIN << "Loading UnknownContent From " + basket()->folderName() + fileName();
	QFile file(fullPath());
	if (file.open(IO_ReadOnly)) {
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

	QRect textRect = QFontMetrics(note()->font()).boundingRect(0, 0, /*width=*/1, 500000, Qt::AlignAuto | Qt::AlignTop | Qt::WordBreak, m_mimeTypes);
	contentChanged(DECORATION_MARGIN + textRect.width() + DECORATION_MARGIN + 1);
}

void UnknownContent::addAlternateDragObjects(KMultipleDrag *dragObject)
{
	QFile file(fullPath());
	if (file.open(IO_ReadOnly)) {
		QDataStream stream(&file);
		// Get the MIME types names:
		QValueList<QString> mimes;
		QString line;
		do {
			if (!stream.atEnd()) {
				stream >> line;
				if (!line.isEmpty())
					mimes.append(line);
			}
		} while (!line.isEmpty() && !stream.atEnd());
		// Add the streams:
		Q_UINT64     size; // TODO: It was Q_UINT32 in version 0.5.0 !
		QByteArray  *array;
		QStoredDrag *storedDrag;
		for (uint i = 0; i < mimes.count(); ++i) {
			// Get the size:
			stream >> size;
			// Allocate memory to retreive size bytes and store them:
			array = new QByteArray(size);
			stream.readRawBytes(array->data(), size);
			// Creata and add the QDragObject:
			storedDrag = new QStoredDrag(*(mimes.at(i)));
			storedDrag->setEncodedData(*array);
			dragObject->addDragObject(storedDrag);
			delete array; // FIXME: Should we?
		}
		file.close();
	}
}

void UnknownContent::exportToHTML(QTextStream &stream, int indent, const HtmlExportData &/*exportData*/)
{
	QString spaces;
	stream << "<div class=\"unknown\">" << mimeTypes().replace("\n", "\n" + spaces.fill(' ', indent + 1 + 1)) << "</div>";
}




void NoteFactory__loadNode(const QDomElement &content, const QString &lowerTypeName, Note *parent)
{
	if        (lowerTypeName == "text")      new TextContent(      parent, content.text()         );
	else if   (lowerTypeName == "html")      new HtmlContent(      parent, content.text()         );
	else if   (lowerTypeName == "image")     new ImageContent(     parent, content.text()         );
	else if   (lowerTypeName == "animation") new AnimationContent( parent, content.text()         );
	else if   (lowerTypeName == "sound")     new SoundContent(     parent, content.text()         );
	else if   (lowerTypeName == "file")      new FileContent(      parent, content.text()         );
	else if   (lowerTypeName == "link") {
		bool autoTitle = content.attribute("title") == content.text();
		bool autoIcon  = content.attribute("icon")  == NoteFactory::iconForURL(KURL(content.text()));
		autoTitle = XMLWork::trueOrFalse( content.attribute("autoTitle"), autoTitle);
		autoIcon  = XMLWork::trueOrFalse( content.attribute("autoIcon"),  autoIcon );
		new LinkContent( parent, KURL(content.text()), content.attribute("title"), content.attribute("icon"), autoTitle, autoIcon );
	} else if (lowerTypeName == "launcher")  new LauncherContent(  parent, content.text()         );
	else if   (lowerTypeName == "color")     new ColorContent(     parent, QColor(content.text()) );
	else if   (lowerTypeName == "unknown")   new UnknownContent(   parent, content.text()         );
}

#include "notecontent.moc"

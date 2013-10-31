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

#include "notecontent.h"

#include <QtCore/QBuffer>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtCore/QStringList>
#include <QtGui/QAbstractTextDocumentLayout>    //For m_simpleRichText->documentLayout()
#include <QtGui/QBitmap>                        //For QPixmap::createHeuristicMask()
#include <QtGui/QFontMetrics>
#include <QtGui/QMovie>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QWidget>
#include <QtXml/QDomDocument>
#include <QtNetwork/QNetworkReply>
#include <KDE/KIO/AccessManager>

#include <KDE/KDebug>
#include <KDE/KService>
#include <KDE/KLocale>
#include <KDE/KFileMetaInfo>
#include <KDE/KFileItem>
#include <KDE/KIO/PreviewJob>                   //For KIO::file_preview(...)

#include <phonon/AudioOutput>
#include <phonon/MediaObject>

#include "note.h"
#include "basketscene.h"
#include "filter.h"
#include "xmlwork.h"
#include "tools.h"
#include "notefactory.h"
#include "global.h"
#include "settings.h"
#include "debugwindow.h"
#include "htmlexporter.h"
#include "config.h"

/**
 * LinkDisplayItem definition
 * 
 */

QRectF LinkDisplayItem::boundingRect() const
{
  if(m_note)
  {
    return QRect(0, 0, m_note->width() - m_note->contentX() - Note::NOTE_MARGIN, m_note->height() - 2*Note::NOTE_MARGIN);
  }
  return QRectF();  
}

void LinkDisplayItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
  if(!m_note)
    return;
  
  QRectF rect = boundingRect();
  m_linkDisplay.paint(painter, 0, 0, rect.width(), rect.height(), m_note->palette(), true, m_note->isSelected(), m_note->hovered(), m_note->hovered() && m_note->hoveredZone() == Note::Custom0);
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

QRectF NoteContent::zoneRect(int zone, const QPointF &/*pos*/)
{
    if (zone == Note::Content)
        return QRectF(0, 0, note()->width(), note()->height()); // Too wide and height, but it will be clipped by Note::zoneRect()
    else
        return QRectF();
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

    return false; // !useFile() or unsuccessful rename
}

QString NoteContent::fullPath()
{
    if (note() && useFile())
        return note()->fullPath();
    else
        return "";
}

void NoteContent::contentChanged(qreal newMinWidth)
{
    m_minWidth = newMinWidth;
    if (note()) {
//      note()->unbufferize();
        note()->requestRelayout(); // TODO: It should re-set the width!  m_width = 0 ?   contentChanged: setWidth, geteight, if size havent changed, only repaint and not relayout
    }
}

BasketScene* NoteContent::basket()
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

NoteType::Id TextContent::type() const
{
    return NoteType::Text;
}
NoteType::Id HtmlContent::type() const
{
    return NoteType::Html;
}
NoteType::Id ImageContent::type() const
{
    return NoteType::Image;
}
NoteType::Id AnimationContent::type() const
{
    return NoteType::Animation;
}
NoteType::Id SoundContent::type() const
{
    return NoteType::Sound;
}
NoteType::Id FileContent::type() const
{
    return NoteType::File;
}
NoteType::Id LinkContent::type() const
{
    return NoteType::Link;
}
NoteType::Id CrossReferenceContent::type() const
{
    return NoteType::CrossReference;
}
NoteType::Id LauncherContent::type() const
{
    return NoteType::Launcher;
}
NoteType::Id ColorContent::type() const
{
    return NoteType::Color;
}
NoteType::Id UnknownContent::type() const
{
    return NoteType::Unknown;
}

QString TextContent::typeName() const
{
    return i18n("Plain Text");
}
QString HtmlContent::typeName() const
{
    return i18n("Text");
}
QString ImageContent::typeName() const
{
    return i18n("Image");
}
QString AnimationContent::typeName() const
{
    return i18n("Animation");
}
QString SoundContent::typeName() const
{
    return i18n("Sound");
}
QString FileContent::typeName() const
{
    return i18n("File");
}
QString LinkContent::typeName() const
{
    return i18n("Link");
}
QString CrossReferenceContent::typeName() const
{
    return i18n("Cross Reference");
}
QString LauncherContent::typeName() const
{
    return i18n("Launcher");
}
QString ColorContent::typeName() const
{
    return i18n("Color");
}
QString UnknownContent::typeName() const
{
    return i18n("Unknown");
}

QString TextContent::lowerTypeName() const
{
    return "text";
}
QString HtmlContent::lowerTypeName() const
{
    return "html";
}
QString ImageContent::lowerTypeName() const
{
    return "image";
}
QString AnimationContent::lowerTypeName() const
{
    return "animation";
}
QString SoundContent::lowerTypeName() const
{
    return "sound";
}
QString FileContent::lowerTypeName() const
{
    return "file";
}
QString LinkContent::lowerTypeName() const
{
    return "link";
}
QString CrossReferenceContent::lowerTypeName() const
{
    return "cross_reference";
}
QString LauncherContent::lowerTypeName() const
{
    return "launcher";
}
QString ColorContent::lowerTypeName() const
{
    return "color";
}
QString UnknownContent::lowerTypeName() const
{
    return "unknown";
}

QString NoteContent::toText(const QString &cuttedFullPath)
{
    return (cuttedFullPath.isEmpty() ? fullPath() : cuttedFullPath);
}

QString TextContent::toText(const QString &/*cuttedFullPath*/)
{
    return text();
}
QString HtmlContent::toText(const QString &/*cuttedFullPath*/)
{
    return Tools::htmlToText(html());
}
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
QString CrossReferenceContent::toText(const QString &/*cuttedFullPath*/)
{
    if (title().isEmpty() && url().isEmpty())
        return "";
    else if (url().isEmpty())
        return title();
    else if (title().isEmpty())
        return url().prettyUrl();
    else
        return QString("%1 <%2>").arg(title(), url().prettyUrl());
}
QString ColorContent::toText(const QString &/*cuttedFullPath*/)
{
    return color().name();
}
QString UnknownContent::toText(const QString &/*cuttedFullPath*/)
{
    return "";
}

// TODO: If imageName.isEmpty() return fullPath() because it's for external use, else return fileName() because it's to display in a tooltip
QString TextContent::toHtml(const QString &/*imageName*/, const QString &/*cuttedFullPath*/)
{
    return Tools::textToHTMLWithoutP(text());
}

QString HtmlContent::toHtml(const QString &/*imageName*/, const QString &/*cuttedFullPath*/)
{
    return Tools::htmlToParagraph(html());
}

QString ImageContent::toHtml(const QString &/*imageName*/, const QString &cuttedFullPath)
{
    return QString("<img src=\"%1\">").arg(cuttedFullPath.isEmpty() ? fullPath() : cuttedFullPath);
}

QString AnimationContent::toHtml(const QString &/*imageName*/, const QString &cuttedFullPath)
{
    return QString("<img src=\"%1\">").arg(cuttedFullPath.isEmpty() ? fullPath() : cuttedFullPath);
}

QString SoundContent::toHtml(const QString &/*imageName*/, const QString &cuttedFullPath)
{
    return QString("<a href=\"%1\">%2</a>").arg((cuttedFullPath.isEmpty() ? fullPath() : cuttedFullPath), fileName());
} // With the icon?

QString FileContent::toHtml(const QString &/*imageName*/, const QString &cuttedFullPath)
{
    return QString("<a href=\"%1\">%2</a>").arg((cuttedFullPath.isEmpty() ? fullPath() : cuttedFullPath), fileName());
} // With the icon?

QString LinkContent::toHtml(const QString &/*imageName*/, const QString &/*cuttedFullPath*/)
{
    return QString("<a href=\"%1\">%2</a>").arg(url().prettyUrl(), title());
} // With the icon?

QString CrossReferenceContent::toHtml(const QString &/*imageName*/, const QString &/*cuttedFullPath*/)
{
    return QString("<a href=\"%1\">%2</a>").arg(url().prettyUrl(), title());
} // With the icon?

QString LauncherContent::toHtml(const QString &/*imageName*/, const QString &cuttedFullPath)
{
    return QString("<a href=\"%1\">%2</a>").arg((cuttedFullPath.isEmpty() ? fullPath() : cuttedFullPath), name());
} // With the icon?

QString ColorContent::toHtml(const QString &/*imageName*/, const QString &/*cuttedFullPath*/)
{
    return QString("<span style=\"color: %1\">%2</span>").arg(color().name(), color().name());
}

QString UnknownContent::toHtml(const QString &/*imageName*/, const QString &/*cuttedFullPath*/)
{
    return "";
}

QPixmap ImageContent::toPixmap()
{
    return pixmap();
}
QPixmap AnimationContent::toPixmap()
{
    return m_movie->currentPixmap();
}

void NoteContent::toLink(KUrl *url, QString *title, const QString &cuttedFullPath)
{
    if (useFile()) {
        *url   = KUrl(cuttedFullPath.isEmpty() ? fullPath() : cuttedFullPath);
        *title = (cuttedFullPath.isEmpty() ? fullPath() : cuttedFullPath);
    } else {
        *url   = KUrl();
        title->clear();
    }
}
void LinkContent::toLink(KUrl *url, QString *title, const QString &/*cuttedFullPath*/)
{
    *url   = this->url();
    *title = this->title();
}
void CrossReferenceContent::toLink(KUrl *url, QString *title, const QString &/*cuttedFullPath*/)
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

bool TextContent::useFile() const
{
    return true;
}
bool HtmlContent::useFile() const
{
    return true;
}
bool ImageContent::useFile() const
{
    return true;
}
bool AnimationContent::useFile() const
{
    return true;
}
bool SoundContent::useFile() const
{
    return true;
}
bool FileContent::useFile() const
{
    return true;
}
bool LinkContent::useFile() const
{
    return false;
}
bool CrossReferenceContent::useFile() const
{
    return false;
}
bool LauncherContent::useFile() const
{
    return true;
}
bool ColorContent::useFile() const
{
    return false;
}
bool UnknownContent::useFile() const
{
    return true;
}

bool TextContent::canBeSavedAs() const
{
    return true;
}
bool HtmlContent::canBeSavedAs() const
{
    return true;
}
bool ImageContent::canBeSavedAs() const
{
    return true;
}
bool AnimationContent::canBeSavedAs() const
{
    return true;
}
bool SoundContent::canBeSavedAs() const
{
    return true;
}
bool FileContent::canBeSavedAs() const
{
    return true;
}
bool LinkContent::canBeSavedAs() const
{
    return true;
}
bool CrossReferenceContent::canBeSavedAs() const
{
    return true;
}
bool LauncherContent::canBeSavedAs() const
{
    return true;
}
bool ColorContent::canBeSavedAs() const
{
    return false;
}
bool UnknownContent::canBeSavedAs() const
{
    return false;
}

QString TextContent::saveAsFilters() const
{
    return "text/plain";
}
QString HtmlContent::saveAsFilters() const
{
    return "text/html";
}
QString ImageContent::saveAsFilters() const
{
    return "image/png";
} // TODO: Offer more types
QString AnimationContent::saveAsFilters() const
{
    return "image/gif";
} // TODO: MNG...
QString SoundContent::saveAsFilters() const
{
    return "audio/mp3 audio/ogg";
} // TODO: OGG...
QString FileContent::saveAsFilters() const
{
    return "*";
} // TODO: Get MIME type of the url target
QString LinkContent::saveAsFilters() const
{
    return "*";
} // TODO: idem File + If isDir() const: return
QString CrossReferenceContent::saveAsFilters() const
{
    return "*";
} // TODO: idem File + If isDir() const: return
QString LauncherContent::saveAsFilters() const
{
    return "application/x-desktop";
}
QString ColorContent::saveAsFilters() const
{
    return "";
}
QString UnknownContent::saveAsFilters() const
{
    return "";
}

bool TextContent::match(const FilterData &data)
{
    return text().contains(data.string);
}
bool HtmlContent::match(const FilterData &data)
{
    return m_textEquivalent/*toText("")*/.contains(data.string);
} //OPTIM_FILTER
bool ImageContent::match(const FilterData &/*data*/)
{
    return false;
}
bool AnimationContent::match(const FilterData &/*data*/)
{
    return false;
}
bool SoundContent::match(const FilterData &data)
{
    return fileName().contains(data.string);
}
bool FileContent::match(const FilterData &data)
{
    return fileName().contains(data.string);
}
bool LinkContent::match(const FilterData &data)
{
    return title().contains(data.string) || url().prettyUrl().contains(data.string);
}
bool CrossReferenceContent::match(const FilterData &data)
{
    return title().contains(data.string) || url().prettyUrl().contains(data.string);
}
bool LauncherContent::match(const FilterData &data)
{
    return exec().contains(data.string) || name().contains(data.string);
}
bool ColorContent::match(const FilterData &data)
{
    return color().name().contains(data.string);
}
bool UnknownContent::match(const FilterData &data)
{
    return mimeTypes().contains(data.string);
}

QString TextContent::editToolTipText() const
{
    return i18n("Edit this plain text");
}
QString HtmlContent::editToolTipText() const
{
    return i18n("Edit this text");
}
QString ImageContent::editToolTipText() const
{
    return i18n("Edit this image");
}
QString AnimationContent::editToolTipText() const
{
    return i18n("Edit this animation");
}
QString SoundContent::editToolTipText() const
{
    return i18n("Edit the file name of this sound");
}
QString FileContent::editToolTipText() const
{
    return i18n("Edit the name of this file");
}
QString LinkContent::editToolTipText() const
{
    return i18n("Edit this link");
}
QString CrossReferenceContent::editToolTipText() const
{
    return i18n("Edit this cross reference");
}
QString LauncherContent::editToolTipText() const
{
    return i18n("Edit this launcher");
}
QString ColorContent::editToolTipText() const
{
    return i18n("Edit this color");
}
QString UnknownContent::editToolTipText() const
{
    return i18n("Edit this unknown object");
}

QString TextContent::cssClass() const
{
    return "";
}
QString HtmlContent::cssClass() const
{
    return "";
}
QString ImageContent::cssClass() const
{
    return "";
}
QString AnimationContent::cssClass() const
{
    return "";
}
QString SoundContent::cssClass() const
{
    return "sound";
}
QString FileContent::cssClass() const
{
    return "file";
}
QString LinkContent::cssClass() const
{
    return (LinkLook::lookForURL(m_url) == LinkLook::localLinkLook ? "local" : "network");
}
QString CrossReferenceContent::cssClass() const
{
    return "cross_reference";
}
QString LauncherContent::cssClass() const
{
    return "launcher";
}
QString ColorContent::cssClass() const
{
    return ""     ;
}
QString UnknownContent::cssClass() const
{
    return "";
}

void TextContent::fontChanged()
{
    setText(text());
}
void HtmlContent::fontChanged()
{
    setHtml(html());
}
void ImageContent::fontChanged()
{
    setPixmap(pixmap());
}
void AnimationContent::fontChanged()
{
    /*startMovie();*/
}
void FileContent::fontChanged()
{
    setFileName(fileName());
}
void LinkContent::fontChanged()
{
    setLink(url(), title(), icon(), autoTitle(), autoIcon());
}
void CrossReferenceContent::fontChanged()
{
    setCrossReference(url(), title(), icon());
}
void LauncherContent::fontChanged()
{
    setLauncher(name(), icon(), exec());
}
void ColorContent::fontChanged()
{
    setColor(color());
}
void UnknownContent::fontChanged()
{
    loadFromFile(/*lazyLoad=*/false);
} // TODO: Optimize: setMimeTypes()

//QString TextContent::customOpenCommand()      { return (Settings::isTextUseProg()      && ! Settings::textProg().isEmpty()      ? Settings::textProg()      : QString()); }
QString HtmlContent::customOpenCommand()
{
    return (Settings::isHtmlUseProg()      && ! Settings::htmlProg().isEmpty()      ? Settings::htmlProg()      : QString());
}
QString ImageContent::customOpenCommand()
{
    return (Settings::isImageUseProg()     && ! Settings::imageProg().isEmpty()     ? Settings::imageProg()     : QString());
}
QString AnimationContent::customOpenCommand()
{
    return (Settings::isAnimationUseProg() && ! Settings::animationProg().isEmpty() ? Settings::animationProg() : QString());
}
QString SoundContent::customOpenCommand()
{
    return (Settings::isSoundUseProg()     && ! Settings::soundProg().isEmpty()     ? Settings::soundProg()     : QString());
}

void LinkContent::serialize(QDataStream &stream)
{
    stream << url() << title() << icon() << (quint64)autoTitle() << (quint64)autoIcon();
}
void CrossReferenceContent::serialize(QDataStream &stream)
{
    stream << url() << title() << icon();
}
void ColorContent::serialize(QDataStream &stream)
{
    stream << color();
}

QPixmap TextContent::feedbackPixmap(qreal width, qreal height)
{
    QRectF textRect = QFontMetrics(note()->font()).boundingRect(0, 0, width, height, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, text());
    QPixmap pixmap(qMin(width, textRect.width()), qMin(height, textRect.height()));
    pixmap.fill(note()->backgroundColor().dark(FEEDBACK_DARKING));
    QPainter painter(&pixmap);
    painter.setPen(note()->textColor());
    painter.setFont(note()->font());
    painter.drawText(0, 0, pixmap.width(), pixmap.height(), Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, text());
    painter.end();

    return pixmap;
}

QPixmap HtmlContent::feedbackPixmap(qreal width, qreal height)
{
    QTextDocument richText;
    richText.setHtml(html());
    richText.setDefaultFont(note()->font());
    richText.setTextWidth(width);
    QPalette palette;
    palette = basket()->palette();
    palette.setColor(QPalette::Text,       note()->textColor());
    palette.setColor(QPalette::Background, note()->backgroundColor().dark(FEEDBACK_DARKING));
    QPixmap pixmap(qMin(width, richText.idealWidth()), qMin(height, richText.size().height()));
    pixmap.fill(note()->backgroundColor().dark(FEEDBACK_DARKING));
    QPainter painter(&pixmap);
    painter.setPen(note()->textColor());
    painter.translate(0, 0);
    richText.drawContents(&painter, QRectF(0, 0, pixmap.width(), pixmap.height()));
    painter.end();

    return pixmap;
}

QPixmap ImageContent::feedbackPixmap(qreal width, qreal height)
{
    if (width >= m_pixmapItem.pixmap().width() && height >= m_pixmapItem.pixmap().height()) { // Full size
        if (m_pixmapItem.pixmap().hasAlpha()) {
            QPixmap opaque(m_pixmapItem.pixmap().width(), m_pixmapItem.pixmap().height());
            opaque.fill(note()->backgroundColor().dark(FEEDBACK_DARKING));
            QPainter painter(&opaque);
            painter.drawPixmap(0, 0, m_pixmapItem.pixmap());
            painter.end();
            return opaque;
	} else{
	   return m_pixmapItem.pixmap();
	}
    } else { // Scalled down
        QImage imageToScale = m_pixmapItem.pixmap().toImage();
        QPixmap pmScaled;
        pmScaled = QPixmap::fromImage(imageToScale.scaled(width, height,
                                      Qt::KeepAspectRatio));
        if (pmScaled.hasAlpha()) {
            QPixmap opaque(pmScaled.width(), pmScaled.height());
            opaque.fill(note()->backgroundColor().dark(FEEDBACK_DARKING));
            QPainter painter(&opaque);
            painter.drawPixmap(0, 0, pmScaled);
            painter.end();
	    return opaque;
	} else {
	  return pmScaled;
	}
    }
}

QPixmap AnimationContent::feedbackPixmap(qreal width, qreal height)
{
    QPixmap pixmap = m_movie->currentPixmap();
    if (width >= pixmap.width() && height >= pixmap.height()) // Full size
	return pixmap;
    else { // Scalled down
        QImage imageToScale = pixmap.toImage();
        QPixmap pmScaled;
        pmScaled = QPixmap::fromImage(imageToScale.scaled(width, height,
                                      Qt::KeepAspectRatio));
        return pmScaled;
    }
}

QPixmap LinkContent::feedbackPixmap(qreal width, qreal height)
{
    QPalette palette;
    palette = basket()->palette();
    palette.setColor(QPalette::WindowText,       note()->textColor());
    palette.setColor(QPalette::Background, note()->backgroundColor().dark(FEEDBACK_DARKING));
    return m_linkDisplayItem.linkDisplay().feedbackPixmap(width, height, palette, /*isDefaultColor=*/note()->textColor() == basket()->textColor());
}

QPixmap CrossReferenceContent::feedbackPixmap(qreal width, qreal height)
{
    QPalette palette;
    palette = basket()->palette();
    palette.setColor(QPalette::WindowText, note()->textColor());
    palette.setColor(QPalette::Background, note()->backgroundColor().dark(FEEDBACK_DARKING));
    return m_linkDisplayItem.linkDisplay().feedbackPixmap(width, height, palette, /*isDefaultColor=*/note()->textColor() == basket()->textColor());
}

QPixmap ColorContent::feedbackPixmap(qreal width, qreal height)
{
    // TODO: Duplicate code: make a rect() method!
    QRectF boundingRect = m_colorItem.boundingRect();
    
    QPalette palette;
    palette = basket()->palette();
    palette.setColor(QPalette::WindowText,       note()->textColor());
    palette.setColor(QPalette::Background, note()->backgroundColor().dark(FEEDBACK_DARKING));

    QPixmap pixmap(qMin(width, boundingRect.width()), qMin(height, boundingRect.height()));
    pixmap.fill(note()->backgroundColor().dark(FEEDBACK_DARKING));
    QPainter painter(&pixmap);
    m_colorItem.paint(&painter,0,0);//, pixmap.width(), pixmap.height(), palette, false, false, false); // We don't care of the three last boolean parameters.
    painter.end();

    return pixmap;
}

QPixmap FileContent::feedbackPixmap(qreal width, qreal height)
{
    QPalette palette;
    palette = basket()->palette();
    palette.setColor(QPalette::WindowText,       note()->textColor());
    palette.setColor(QPalette::Background, note()->backgroundColor().dark(FEEDBACK_DARKING));
    return m_linkDisplayItem.linkDisplay().feedbackPixmap(width, height, palette, /*isDefaultColor=*/note()->textColor() == basket()->textColor());
}

QPixmap LauncherContent::feedbackPixmap(qreal width, qreal height)
{
    QPalette palette;
    palette = basket()->palette();
    palette.setColor(QPalette::WindowText,       note()->textColor());
    palette.setColor(QPalette::Background, note()->backgroundColor().dark(FEEDBACK_DARKING));
    return m_linkDisplayItem.linkDisplay().feedbackPixmap(width, height, palette, /*isDefaultColor=*/note()->textColor() == basket()->textColor());
}

QPixmap UnknownContent::feedbackPixmap(qreal width, qreal height)
{
    QRectF boundingRect = m_unknownItem.boundingRect();

    QPalette palette;
    palette = basket()->palette();
    palette.setColor(QPalette::WindowText,       note()->textColor());
    palette.setColor(QPalette::Background, note()->backgroundColor().dark(FEEDBACK_DARKING));

    QPixmap pixmap(qMin(width, boundingRect.width()), qMin(height, boundingRect.height()));
    QPainter painter(&pixmap);
    m_unknownItem.paint(&painter,0,0);//, pixmap.width() + 1, pixmap.height(), palette, false, false, false); // We don't care of the three last boolean parameters.
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
        : NoteContent(parent, fileName), m_graphicsTextItem(parent)
{
    if(parent)
    {
      parent->addToGroup(&m_graphicsTextItem);
      m_graphicsTextItem.setPos(parent->contentX(),Note::NOTE_MARGIN);
    }

    basket()->addWatchedFile(fullPath());
    loadFromFile(lazyLoad);
}

TextContent::~TextContent()
{
    if(note()) note()->removeFromGroup(&m_graphicsTextItem);
}

qreal TextContent::setWidthAndGetHeight(qreal /*width*/)
{
    return m_graphicsTextItem.boundingRect().height();
}

bool TextContent::loadFromFile(bool lazyLoad)
{
    DEBUG_WIN << "Loading TextContent From " + basket()->folderName() + fileName();

    QString content;
    bool success = basket()->loadFromFile(fullPath(), &content);

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
    m_graphicsTextItem.setFont(note()->font());
    contentChanged(m_graphicsTextItem.boundingRect().width() + 1);
    return true;
}

bool TextContent::saveToFile()
{
    return basket()->saveToFile(fullPath(), text());
}

QString TextContent::linkAt(const QPointF &/*pos*/)
{
  return "";
/*    if (m_simpleRichText)
        return m_simpleRichText->documentLayout()->anchorAt(pos);
    else
        return ""; // Lazy loaded*/
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
    m_graphicsTextItem.setText(text);
    if (!lazyLoad)
        finishLazyLoad();
    else
        contentChanged(m_graphicsTextItem.boundingRect().width());
}

void TextContent::exportToHTML(HTMLExporter *exporter, int indent)
{
    QString spaces;
    QString html = "<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"><meta name=\"qrichtext\" content=\"1\" /></head><body>" +
                   Tools::tagCrossReferences(Tools::tagURLs(Tools::textToHTMLWithoutP(text().replace("\t", "                "))), false, exporter); // Don't collapse multiple spaces!
    exporter->stream << html.replace("  ", " &nbsp;").replace("\n", "\n" + spaces.fill(' ', indent + 1));
}

/** class HtmlContent:
 */

HtmlContent::HtmlContent(Note *parent, const QString &fileName, bool lazyLoad)
        : NoteContent(parent, fileName), m_simpleRichText(0), m_graphicsTextItem(parent)
{
  if(parent)
  {
      parent->addToGroup(&m_graphicsTextItem);
      m_graphicsTextItem.setPos(parent->contentX(),Note::NOTE_MARGIN);
  }
  basket()->addWatchedFile(fullPath());
  loadFromFile(lazyLoad);
}

HtmlContent::~HtmlContent()
{
    if(note()) note()->removeFromGroup(&m_graphicsTextItem);

    delete m_simpleRichText;
}

qreal HtmlContent::setWidthAndGetHeight(qreal width)
{
    width -= 1;
    m_graphicsTextItem.setTextWidth(width);
    return m_graphicsTextItem.boundingRect().height();
}

bool HtmlContent::loadFromFile(bool lazyLoad)
{
    DEBUG_WIN << "Loading HtmlContent From " + basket()->folderName() + fileName();

    QString content;
    bool success = basket()->loadFromFile(fullPath(), &content);

    if (success)
        setHtml(content, lazyLoad);
    else {
        setHtml("", lazyLoad);
        if (!QFile::exists(fullPath()))
            saveToFile(); // Reserve the fileName so no new note will have the same name!
    }
    return success;
}

bool HtmlContent::finishLazyLoad()
{
    qreal width = m_graphicsTextItem.document()->idealWidth();
    
    m_graphicsTextItem.setFlags(QGraphicsItem::ItemIsSelectable|QGraphicsItem::ItemIsFocusable);
    m_graphicsTextItem.setTextInteractionFlags(Qt::TextEditorInteraction);
    
    QString css = ".cross_reference { display: block; width: 100%; text-decoration: none; color: #336600; }"
       "a:hover.cross_reference { text-decoration: underline; color: #ff8000; }";
    m_graphicsTextItem.document()->setDefaultStyleSheet(css);
    QString convert = Tools::tagURLs(m_html);
    if(note()->allowCrossReferences())
        convert = Tools::tagCrossReferences(convert);
    m_graphicsTextItem.setHtml(convert);
    m_graphicsTextItem.setDefaultTextColor(basket()->textColor());
    m_graphicsTextItem.setFont(note()->font());
    m_graphicsTextItem.setTextWidth(1); // We put a width of 1 pixel, so usedWidth() is egual to the minimum width
    int minWidth = m_graphicsTextItem.document()->idealWidth();
    m_graphicsTextItem.setTextWidth(width);
    contentChanged(minWidth + 1);

    return true;
}

bool HtmlContent::saveToFile()
{
    return basket()->saveToFile(fullPath(), html());
}

QString HtmlContent::linkAt(const QPointF &pos)
{
    return m_graphicsTextItem.document()->documentLayout()->anchorAt(pos);
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
    /* The code was commented, so now non-Latin text is stored directly in Unicode.
     * If testing doesn't show any bugs, this block should be deleted
    QRegExp rx("([^\\x00-\\x7f])");
    while (m_html.contains(rx)) {
        m_html.replace( rx.cap().unicode()[0], QString("&#%1;").arg(rx.cap().unicode()[0].unicode()) );
    }*/
    m_textEquivalent = toText(""); //OPTIM_FILTER
    if (!lazyLoad)
        finishLazyLoad();
    else
        contentChanged(10);
}

void HtmlContent::exportToHTML(HTMLExporter *exporter, int indent)
{
    QString spaces;
    QString convert = Tools::tagURLs(html().replace("\t", "                "));
    if(note()->allowCrossReferences())
        convert = Tools::tagCrossReferences(convert, false, exporter);

    exporter->stream << Tools::htmlToParagraph(convert)
    .replace("  ", " &nbsp;")
    .replace("\n", "\n" + spaces.fill(' ', indent + 1));
}

/** class ImageContent:
 */

ImageContent::ImageContent(Note *parent, const QString &fileName, bool lazyLoad)
        : NoteContent(parent, fileName), m_pixmapItem(parent), m_format()
{
    if(parent)
    {
      parent->addToGroup(&m_pixmapItem);
      m_pixmapItem.setPos(parent->contentX(),Note::NOTE_MARGIN);
    }

    basket()->addWatchedFile(fullPath());
    loadFromFile(lazyLoad);
}

ImageContent::~ImageContent()
{
    if(note()) note()->removeFromGroup(&m_pixmapItem);
}

qreal ImageContent::setWidthAndGetHeight(qreal width)
{
    width -= 1;
    // Don't store width: we will get it on paint!
    if (width >= m_pixmapItem.pixmap().width()) // Full size
    {
        m_pixmapItem.setScale(1.0);
        return m_pixmapItem.boundingRect().height();
    }
    else { // Scalled down
        qreal scaleFactor = width / m_pixmapItem.pixmap().width();
	m_pixmapItem.setScale( scaleFactor );
        return m_pixmapItem.boundingRect().height()*scaleFactor;
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
    QPixmap pixmap;
    
    if (basket()->loadFromFile(fullPath(), &content)) {
        QBuffer buffer(&content);

        buffer.open(QIODevice::ReadOnly);
        m_format = QImageReader::imageFormat(&buffer); // See QImageIO to know what formats can be supported.
        buffer.close();
        if (!m_format.isNull()) {
            pixmap.loadFromData(content);
            setPixmap(pixmap);
            return true;
        }
    }

    kDebug() << "FAILED TO LOAD ImageContent: " << fullPath();
    m_format = "PNG"; // If the image is set later, it should be saved without destruction, so we use PNG by default.
    pixmap = QPixmap(1, 1); // Create a 1x1 pixels image instead of an undefined one.
    pixmap.fill();
    pixmap.setMask(pixmap.createHeuristicMask());
    setPixmap(pixmap);
    if (!QFile::exists(fullPath()))
        saveToFile(); // Reserve the fileName so no new note will have the same name!
    return false;
}

bool ImageContent::saveToFile()
{
    QByteArray ba;
    QBuffer buffer(&ba);

    buffer.open(QIODevice::WriteOnly);
    m_pixmapItem.pixmap().save(&buffer, m_format);
    return basket()->saveToFile(fullPath(), ba);
}


void ImageContent::toolTipInfos(QStringList *keys, QStringList *values)
{
    keys->append(i18n("Size"));
    values->append(i18n("%1 by %2 pixels", QString::number(m_pixmapItem.pixmap().width()), QString::number(m_pixmapItem.pixmap().height())));
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
    m_pixmapItem.setPixmap(pixmap);
    // Since it's scalled, the height is always greater or equal to the size of the tag emblems (16)
    contentChanged(16 + 1); // TODO: always good? I don't think...
}

void ImageContent::exportToHTML(HTMLExporter *exporter, int /*indent*/)
{
    qreal width  = m_pixmapItem.pixmap().width();
    qreal height = m_pixmapItem.pixmap().height();
    qreal contentWidth = note()->width() - note()->contentX() - 1 - Note::NOTE_MARGIN;

    QString imageName = exporter->copyFile(fullPath(), /*createIt=*/true);

    if (contentWidth <= m_pixmapItem.pixmap().width()) { // Scalled down
        qreal scale = contentWidth / m_pixmapItem.pixmap().width();
        width  = m_pixmapItem.pixmap().width()  * scale;
        height = m_pixmapItem.pixmap().height() * scale;
        exporter->stream << "<a href=\"" << exporter->dataFolderName << imageName << "\" title=\"" << i18n("Click for full size view") << "\">";
    }

    exporter->stream << "<img src=\"" << exporter->dataFolderName << imageName
    << "\" width=\"" << width << "\" height=\"" << height << "\" alt=\"\">";

    if (contentWidth <= m_pixmapItem.pixmap().width()) // Scalled down
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
	, m_currentWidth(0)
	, m_graphicsPixmap(parent)
{
    if(parent)
    {
      parent->addToGroup(&m_graphicsPixmap);
      m_graphicsPixmap.setPos(parent->contentX(),Note::NOTE_MARGIN);
      connect(parent->basket(), SIGNAL(activated()), m_movie, SLOT(start()));
      connect(parent->basket(), SIGNAL(closed()), m_movie, SLOT(stop()));
    }
    
    basket()->addWatchedFile(fullPath());
    connect(m_movie, SIGNAL(resized(QSize)), this, SLOT(movieResized()));
    connect(m_movie, SIGNAL(frameChanged(int)), this, SLOT(movieFrameChanged()));

    loadFromFile(lazyLoad);
}

AnimationContent::~AnimationContent()
{
    note()->removeFromGroup(&m_graphicsPixmap);
}

qreal AnimationContent::setWidthAndGetHeight(qreal width)
{
    m_currentWidth = width;
    QPixmap pixmap = m_graphicsPixmap.pixmap();
    if(pixmap.width() > m_currentWidth)
    {
      qreal scaleFactor = m_currentWidth / pixmap.width();
      m_graphicsPixmap.setScale(scaleFactor);
      return pixmap.height() * scaleFactor;
    }
    else
    {
      m_graphicsPixmap.setScale(1.0);
      return pixmap.height();
    }
    
    return 0;
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
        startMovie();
	contentChanged(16);
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

bool AnimationContent::startMovie()
{
    if (m_buffer->data().isEmpty())
        return false;
    m_movie->setDevice(m_buffer);
    m_movie->start();
    return true;
}

void AnimationContent::movieUpdated()
{
    m_graphicsPixmap.setPixmap(m_movie->currentPixmap());
}

void AnimationContent::movieResized()
{
     m_graphicsPixmap.setPixmap(m_movie->currentPixmap());
}

void AnimationContent::movieFrameChanged()
{
    m_graphicsPixmap.setPixmap(m_movie->currentPixmap());
}

void AnimationContent::exportToHTML(HTMLExporter *exporter, int /*indent*/)
{
    exporter->stream << QString("<img src=\"%1\" width=\"%2\" height=\"%3\" alt=\"\">")
    .arg(exporter->dataFolderName + exporter->copyFile(fullPath(), /*createIt=*/true),
         QString::number(m_movie->currentPixmap().size().width()),
         QString::number(m_movie->currentPixmap().size().height()));
}

/** class FileContent:
 */

FileContent::FileContent(Note *parent, const QString &fileName)
        : NoteContent(parent, fileName), m_linkDisplayItem(parent), m_previewJob(0)
{
    basket()->addWatchedFile(fullPath());
    setFileName(fileName); // FIXME: TO THAT HERE BECAUSE NoteContent() constructor seems to don't be able to call virtual methods???
    if(parent)
    {
      parent->addToGroup(&m_linkDisplayItem);
      m_linkDisplayItem.setPos(parent->contentX(),Note::NOTE_MARGIN);
    }
}

FileContent::~FileContent()
{
    if(note()) note()->removeFromGroup(&m_linkDisplayItem);
}

qreal FileContent::setWidthAndGetHeight(qreal width)
{
    m_linkDisplayItem.linkDisplay().setWidth(width);
    return m_linkDisplayItem.linkDisplay().height();
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

    KFileMetaInfo info = KFileMetaInfo(KUrl(fullPath()));
    if (info.isValid()) {
        QStringList groups = info.preferredKeys();
        int i = 0;
        for (QStringList::Iterator it = groups.begin();
                i < 6 && it != groups.end();
                ++it) {
            KFileMetaInfoItem item = info.item(*it);
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

int FileContent::zoneAt(const QPointF &pos)
{
    return (m_linkDisplayItem.linkDisplay().iconButtonAt(pos) ? 0 : Note::Custom0);
}

QRectF FileContent::zoneRect(int zone, const QPointF &/*pos*/)
{
    QRectF linkRect = m_linkDisplayItem.linkDisplay().iconButtonRect();

    if (zone == Note::Custom0)
        return QRectF(linkRect.width(), 0, note()->width(), note()->height()); // Too wide and height, but it will be clipped by Note::zoneRect()
    else if (zone == Note::Content)
        return linkRect;
    else
        return QRectF();
}

QString FileContent::zoneTip(int zone)
{
    return (zone == Note::Custom0 ? i18n("Open this file") : QString());
}

Qt::CursorShape FileContent::cursorFromZone(int zone) const
{
    if (zone == Note::Custom0)
        return Qt::PointingHandCursor;
    return Qt::ArrowCursor;
}


int FileContent::xEditorIndent()
{
    return m_linkDisplayItem.linkDisplay().iconButtonRect().width() + 2;
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
        m_linkDisplayItem.linkDisplay().setLink(fileName, NoteFactory::iconForURL(url),            linkLook(), note()->font()); // FIXME: move iconForURL outside of NoteFactory !!!!!
    else
        m_linkDisplayItem.linkDisplay().setLink(fileName, NoteFactory::iconForURL(url), QPixmap(), linkLook(), note()->font());
    startFetchingUrlPreview();
    contentChanged(m_linkDisplayItem.linkDisplay().minWidth());
}

void FileContent::linkLookChanged()
{
    fontChanged();
    //setFileName(fileName());
    //startFetchingUrlPreview();
}

void FileContent::newPreview(const KFileItem&, const QPixmap &preview)
{
    LinkLook *linkLook = this->linkLook();
    m_linkDisplayItem.linkDisplay().setLink(fileName(), NoteFactory::iconForURL(KUrl(fullPath())), (linkLook->previewEnabled() ? preview : QPixmap()), linkLook, note()->font());
    contentChanged(m_linkDisplayItem.linkDisplay().minWidth());
}

void FileContent::removePreview(const KFileItem& ki)
{
    newPreview(ki, QPixmap());
}

void FileContent::startFetchingUrlPreview()
{
    /*
    KUrl url(fullPath());
    LinkLook *linkLook = this->linkLook();

//  delete m_previewJob;
    if (!url.isEmpty() && linkLook->previewSize() > 0) {
        KUrl filteredUrl = NoteFactory::filteredURL(url);//KURIFilter::self()->filteredURI(url);
        KUrl::List urlList;
        urlList.append(filteredUrl);
        m_previewJob = KIO::filePreview(urlList, linkLook->previewSize(), linkLook->previewSize(), linkLook->iconSize());
        connect(m_previewJob, SIGNAL(gotPreview(const KFileItem&, const QPixmap&)), this, SLOT(newPreview(const KFileItem&, const QPixmap&)));
        connect(m_previewJob, SIGNAL(failed(const KFileItem&)),                     this, SLOT(removePreview(const KFileItem&)));
    }
    */
}

void FileContent::exportToHTML(HTMLExporter *exporter, int indent)
{
    QString spaces;
    QString fileName = exporter->copyFile(fullPath(), true);
    exporter->stream << m_linkDisplayItem.linkDisplay().toHtml(exporter, KUrl(exporter->dataFolderName + fileName), "").replace("\n", "\n" + spaces.fill(' ', indent + 1));
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

void SoundContent::stateChanged(Phonon::State newState, Phonon::State oldState)
{
    kDebug() << "stateChanged " << oldState << " to " << newState;
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
            if (music->state() == 1) {
                music->play();

            }
        }
    } else {
//       Stop the sound preview, if it was started:
        if (music->state() != 1) {
            music->stop();
//          delete music;//TODO implement this in slot connected with music alted signal
//          music = 0;
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
        : NoteContent(parent), m_linkDisplayItem(parent), m_access_manager(0), m_httpBuff(0), m_previewJob(0)
{
    setLink(url, title, icon, autoTitle, autoIcon);
    if(parent)
    {
      parent->addToGroup(&m_linkDisplayItem);
      m_linkDisplayItem.setPos(parent->contentX(),Note::NOTE_MARGIN);
    }
}
LinkContent::~LinkContent()
{
    if(note()) note()->removeFromGroup(&m_linkDisplayItem);
    delete m_access_manager;
    delete m_httpBuff;
}

qreal LinkContent::setWidthAndGetHeight(qreal width)
{
    m_linkDisplayItem.linkDisplay().setWidth(width);
    return m_linkDisplayItem.linkDisplay().height();
}

void LinkContent::saveToNode(QDomDocument &doc, QDomElement &content)
{
    content.setAttribute("title",      title());
    content.setAttribute("icon",       icon());
    content.setAttribute("autoTitle", (autoTitle() ? "true" : "false"));
    content.setAttribute("autoIcon", (autoIcon()  ? "true" : "false"));
    QDomText textNode = doc.createTextNode(url().prettyUrl());
    content.appendChild(textNode);
}


void LinkContent::toolTipInfos(QStringList *keys, QStringList *values)
{
    keys->append(i18n("Target"));
    values->append(m_url.prettyUrl());
}

int LinkContent::zoneAt(const QPointF &pos)
{
    return (m_linkDisplayItem.linkDisplay().iconButtonAt(pos) ? 0 : Note::Custom0);
}

QRectF LinkContent::zoneRect(int zone, const QPointF &/*pos*/)
{
    QRectF linkRect = m_linkDisplayItem.linkDisplay().iconButtonRect();

    if (zone == Note::Custom0)
        return QRectF(linkRect.width(), 0, note()->width(), note()->height()); // Too wide and height, but it will be clipped by Note::zoneRect()
    else if (zone == Note::Content)
        return linkRect;
    else
        return QRectF();
}

QString LinkContent::zoneTip(int zone)
{
    return (zone == Note::Custom0 ? i18n("Open this link") : QString());
}

Qt::CursorShape LinkContent::cursorFromZone(int zone) const
{
    if (zone == Note::Custom0)
        return Qt::PointingHandCursor;
    return Qt::ArrowCursor;
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
        m_linkDisplayItem.linkDisplay().setLink(m_title, m_icon,            look, note()->font());
    else
        m_linkDisplayItem.linkDisplay().setLink(m_title, m_icon, QPixmap(), look, note()->font());
    startFetchingUrlPreview();
    if (autoTitle)
        startFetchingLinkTitle();
    contentChanged(m_linkDisplayItem.linkDisplay().minWidth());
}

void LinkContent::linkLookChanged()
{
    fontChanged();
}

void LinkContent::newPreview(const KFileItem&, const QPixmap &preview)
{
    LinkLook *linkLook = LinkLook::lookForURL(url());
    m_linkDisplayItem.linkDisplay().setLink(title(), icon(), (linkLook->previewEnabled() ? preview : QPixmap()), linkLook, note()->font());
    contentChanged(m_linkDisplayItem.linkDisplay().minWidth());
}

void LinkContent::removePreview(const KFileItem& ki)
{
    newPreview(ki, QPixmap());
}

// QHttp slots for getting link title
void LinkContent::httpReadyRead()
{
    //Check for availability
    unsigned long bytesAvailable = m_reply->bytesAvailable();
    if (bytesAvailable <= 0)
        return;

    char* buf = new char[bytesAvailable+1];

    long bytes_read = m_reply->read(buf, bytesAvailable);
    if (bytes_read > 0) {

        // m_httpBuff will keep data if title is not found in initial read
        if (m_httpBuff == 0) {
            m_httpBuff = new QString(buf);
        } else {
            (*m_httpBuff) += buf;
        }

        // todo: this should probably strip odd html tags like &nbsp; etc
        QRegExp reg("<title>[\\s]*(&nbsp;)?([^<]+)[\\s]*</title>", Qt::CaseInsensitive);
        reg.setMinimal(true);
        int offset = 0;
        //kDebug() << *m_httpBuff << " bytes: " << bytes_read;

        //FIXME: that check doesn't seem to make any sense
        if ((offset = reg.indexIn(*m_httpBuff)) >= 0) {
            m_title = reg.cap(2);
            m_autoTitle = false;
            setEdited();

            // refresh the title
            setLink(url(), title(), icon(), autoTitle(), autoIcon());

            // stop the http connection
            m_reply->abort();

            delete m_httpBuff;
            m_httpBuff = 0;
        }
        // Stop at 10k bytes
        else if (m_httpBuff->length() > 10000)   {
            m_reply->abort();
            delete m_httpBuff;
            m_httpBuff = 0;
        }
    }
    delete buf;
}

void LinkContent::httpDone(QNetworkReply* reply) {
    //If all done, close and delete the reply.
    reply->deleteLater();
}

void LinkContent::startFetchingLinkTitle()
{
    KUrl newUrl = this->url();

    //If this is not an HTTP request, just ignore it.
    if (newUrl.protocol() == "http") {

        //If we have no access_manager, create one.
        if (m_access_manager == 0) {
            m_access_manager = new KIO::Integration::AccessManager(this);
            connect(m_access_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(httpDone(QNetworkReply*)));
        }

        //If no explicit port, default to port 80.
        if (newUrl.port() == 0)
            newUrl.setPort(80);

        //If no path or query part, default to /
        if (newUrl.encodedPathAndQuery(KUrl::AddTrailingSlash).isEmpty())
            newUrl.setPath("/");

        //Issue request
        m_reply = m_access_manager->get(QNetworkRequest(newUrl));
        connect(m_reply, SIGNAL(readyRead()), this, SLOT(httpReadyRead()));
    }
}

// Code dupicated from FileContent::startFetchingUrlPreview()
void LinkContent::startFetchingUrlPreview()
{
    KUrl url = this->url();
    LinkLook *linkLook = LinkLook::lookForURL(this->url());

//  delete m_previewJob;
    if (!url.isEmpty() && linkLook->previewSize() > 0) {
        KUrl filteredUrl = NoteFactory::filteredURL(url);//KURIFilter::self()->filteredURI(url);
        KUrl::List urlList;
        urlList.append(filteredUrl);
        m_previewJob = KIO::filePreview(urlList, linkLook->previewSize(), linkLook->previewSize(), linkLook->iconSize());
        connect(m_previewJob, SIGNAL(gotPreview(const KFileItem&, const QPixmap&)), this, SLOT(newPreview(const KFileItem&, const QPixmap&)));
        connect(m_previewJob, SIGNAL(failed(const KFileItem&)),                     this, SLOT(removePreview(const KFileItem&)));
    }
}

void LinkContent::exportToHTML(HTMLExporter *exporter, int indent)
{
    QString linkTitle = title();

// TODO:
//  // Append address (useful for print version of the page/basket):
//  if (exportData.formatForImpression && (!autoTitle() && title() != NoteFactory::titleForURL(url().prettyUrl()))) {
//      // The address is on a new line, unless title is empty (empty lines was replaced by &nbsp;):
//      if (linkTitle == " "/*"&nbsp;"*/)
//          linkTitle = url().prettyUrl()/*""*/;
//      else
//          linkTitle = linkTitle + " <" + url().prettyUrl() + ">"/*+ "<br>"*/;
//      //linkTitle += "<i>" + url().prettyUrl() + "</i>";
//  }

    KUrl linkURL;
    /*
        QFileInfo fInfo(url().path());
    //  DEBUG_WIN << url().path()
    //            << "IsFile:" + QString::number(fInfo.isFile())
    //            << "IsDir:"  + QString::number(fInfo.isDir());
        if (exportData.embedLinkedFiles && fInfo.isFile()) {
    //      DEBUG_WIN << "Embed file";
            linkURL = exportData.dataFolderName + BasketScene::copyFile(url().path(), exportData.dataFolderPath, true);
        } else if (exportData.embedLinkedFolders && fInfo.isDir()) {
    //      DEBUG_WIN << "Embed folder";
            linkURL = exportData.dataFolderName + BasketScene::copyFile(url().path(), exportData.dataFolderPath, true);
        } else {
    //      DEBUG_WIN << "Embed LINK";
    */
    linkURL = url();
    /*
        }
    */

    QString spaces;
    exporter->stream << m_linkDisplayItem.linkDisplay().toHtml(exporter, linkURL, linkTitle).replace("\n", "\n" + spaces.fill(' ', indent + 1));
}

/** class CrossReferenceContent:
 */

CrossReferenceContent::CrossReferenceContent(Note *parent, const KUrl &url, const QString &title, const QString &icon)
        : NoteContent(parent), m_linkDisplayItem(parent)
{
    this->setCrossReference(url, title, icon);
    if(parent) parent->addToGroup(&m_linkDisplayItem);
}

CrossReferenceContent::~CrossReferenceContent()
{
    if(note()) note()->removeFromGroup(&m_linkDisplayItem);
}

qreal CrossReferenceContent::setWidthAndGetHeight(qreal width)
{
    m_linkDisplayItem.linkDisplay().setWidth(width);
    return m_linkDisplayItem.linkDisplay().height();
}

void CrossReferenceContent::saveToNode(QDomDocument &doc, QDomElement &content)
{
    content.setAttribute("title",      title());
    content.setAttribute("icon",       icon());
    QDomText textNode = doc.createTextNode(url().prettyUrl());
    content.appendChild(textNode);
}

void CrossReferenceContent::toolTipInfos(QStringList *keys, QStringList *values)
{
    keys->append(i18n("Target"));
    values->append(m_url.prettyUrl());
}

int CrossReferenceContent::zoneAt(const QPointF &pos)
{
    return (m_linkDisplayItem.linkDisplay().iconButtonAt(pos) ? 0 : Note::Custom0);
}

QRectF CrossReferenceContent::zoneRect(int zone, const QPointF &/*pos*/)
{
    QRectF linkRect = m_linkDisplayItem.linkDisplay().iconButtonRect();

    if (zone == Note::Custom0)
        return QRectF(linkRect.width(), 0, note()->width(), note()->height()); // Too wide and height, but it will be clipped by Note::zoneRect()
    else if (zone == Note::Content)
        return linkRect;
    else
        return QRectF();
}

QString CrossReferenceContent::zoneTip(int zone)
{
    return (zone == Note::Custom0 ? i18n("Open this link") : QString());
}

Qt::CursorShape CrossReferenceContent::cursorFromZone(int zone) const
{
    if (zone == Note::Custom0)
        return Qt::PointingHandCursor;
    return Qt::ArrowCursor;
}

QString CrossReferenceContent::statusBarMessage(int zone)
{
    if (zone == Note::Custom0 || zone == Note::Content)
        return i18n("Link to %1").arg(this->title());
    else
        return "";
}

KUrl CrossReferenceContent::urlToOpen(bool /*with*/)
{
    return m_url;
}

QString CrossReferenceContent::messageWhenOpening(OpenMessage where)
{
    if (url().isEmpty())
        return i18n("Link has no basket to open.");

    switch (where) {
    case OpenOne:               return i18n("Opening basket...");
    default:                    return "";
    }
}

void CrossReferenceContent::setLink(const KUrl &url, const QString &title, const QString &icon)
{
    this->setCrossReference(url, title, icon);
}

void CrossReferenceContent::setCrossReference(const KUrl &url, const QString &title, const QString &icon)
{
    m_url = url;
    m_title = (title.isEmpty() ? url.url() : title);
    m_icon = icon;

    LinkLook *look = LinkLook::crossReferenceLook;
    m_linkDisplayItem.linkDisplay().setLink(m_title, m_icon, look, note()->font());

    contentChanged(m_linkDisplayItem.linkDisplay().minWidth());

}

void CrossReferenceContent::linkLookChanged()
{
    fontChanged();
}

void CrossReferenceContent::exportToHTML(HTMLExporter *exporter, int /*indent*/)
{
    QString url = m_url.url();
    QString title;

    if(url.startsWith(QLatin1String("basket://")))
        url = url.mid(9, url.length() -9);
    if(url.endsWith('/'))
        url = url.left(url.length() - 1);

    BasketScene* basket = Global::bnpView->basketForFolderName(url);

    if(!basket)
        title = "unknown basket";
    else
        title = basket->basketName();

    //if the basket we're trying to link to is the basket that was exported then
    //we have to use a special way to refer to it for the links.
    if(basket == exporter->exportedBasket)
        url = "../../" + exporter->fileName;
    else {
        //if we're in the exported basket then the links have to include
        // the sub directories.
        if(exporter->currentBasket == exporter->exportedBasket)
            url.prepend(exporter->basketsFolderName);
        url.append(".html");
    }

    QString linkIcon = exporter->iconsFolderName + exporter->copyIcon(m_icon,
                                                 LinkLook::crossReferenceLook->iconSize());
    linkIcon = QString("<img src=\"%1\" alt=\"\">")
               .arg(linkIcon);

    exporter->stream << QString("<a href=\"%1\">%2 %3</a>").arg(url, linkIcon, title);
}

/** class LauncherContent:
 */

LauncherContent::LauncherContent(Note *parent, const QString &fileName)
        : NoteContent(parent, fileName), m_linkDisplayItem(parent)
{
    basket()->addWatchedFile(fullPath());
    loadFromFile(/*lazyLoad=*/false);
    if(parent)
    {
      parent->addToGroup(&m_linkDisplayItem);
      m_linkDisplayItem.setPos(parent->contentX(),Note::NOTE_MARGIN);
    }
}

LauncherContent::~LauncherContent()
{
    if(note()) note()->removeFromGroup(&m_linkDisplayItem);
}

qreal LauncherContent::setWidthAndGetHeight(qreal width)
{
    m_linkDisplayItem.linkDisplay().setWidth(width);
    return m_linkDisplayItem.linkDisplay().height();
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
        exec = i18n("%1 <i>(run in terminal)</i>", exec);

    if (!service.comment().isEmpty() && service.comment() != service.name()) {
        keys->append(i18n("Comment"));
        values->append(service.comment());
    }

    keys->append(i18n("Command"));
    values->append(exec);
}

int LauncherContent::zoneAt(const QPointF &pos)
{
    return (m_linkDisplayItem.linkDisplay().iconButtonAt(pos) ? 0 : Note::Custom0);
}

QRectF LauncherContent::zoneRect(int zone, const QPointF &/*pos*/)
{
    QRectF linkRect = m_linkDisplayItem.linkDisplay().iconButtonRect();

    if (zone == Note::Custom0)
        return QRectF(linkRect.width(), 0, note()->width(), note()->height()); // Too wide and height, but it will be clipped by Note::zoneRect()
    else if (zone == Note::Content)
        return linkRect;
    else
        return QRectF();
}

QString LauncherContent::zoneTip(int zone)
{
    return (zone == Note::Custom0 ? i18n("Launch this application") : QString());
}

Qt::CursorShape LauncherContent::cursorFromZone(int zone) const
{
    if (zone == Note::Custom0)
        return Qt::PointingHandCursor;
    return Qt::ArrowCursor;
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

    m_linkDisplayItem.linkDisplay().setLink(name, icon, LinkLook::launcherLook, note()->font());
    contentChanged(m_linkDisplayItem.linkDisplay().minWidth());
}

void LauncherContent::exportToHTML(HTMLExporter *exporter, int indent)
{
    QString spaces;
    QString fileName = exporter->copyFile(fullPath(), /*createIt=*/true);
    exporter->stream << m_linkDisplayItem.linkDisplay().toHtml(exporter, KUrl(exporter->dataFolderName + fileName), "").replace("\n", "\n" + spaces.fill(' ', indent + 1));
}

/** class ColorItem:
 */
const int ColorItem::RECT_MARGIN = 2;

ColorItem::ColorItem(Note *parent, const QColor &color)
  :QGraphicsItem(parent)
  , m_note(parent)
{
  setColor(color);
}

void ColorItem::setColor(const QColor &color)
{
    m_color = color;
    m_textRect = QFontMetrics(m_note->font()).boundingRect(m_color.name());
}

QRectF ColorItem::boundingRect() const
{
    qreal rectHeight = (m_textRect.height() + 2) * 3 / 2;
    qreal rectWidth  = rectHeight * 14 / 10; // 1.4 times the height, like A4 papers.
    return QRectF(0, 0, rectWidth + RECT_MARGIN + m_textRect.width() + RECT_MARGIN, rectHeight);
}

void ColorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    QRectF boundingRect = this->boundingRect();
    qreal rectHeight = (m_textRect.height() + 2) * 3 / 2;
    qreal rectWidth  = rectHeight * 14 / 10; // 1.4 times the height, like A4 papers.

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
    painter->setFont(m_note->font());
    painter->setPen(m_note->palette().color(QPalette::Active, QPalette::WindowText));
    painter->drawText(rectWidth + RECT_MARGIN, 0, m_textRect.width(), boundingRect.height(), Qt::AlignLeft | Qt::AlignVCenter, color().name());
}

/** class ColorContent:
 */

ColorContent::ColorContent(Note *parent, const QColor &color)
        : NoteContent(parent), m_colorItem(parent, color)
{
    if(parent)
    {
      parent->addToGroup(&m_colorItem);
      m_colorItem.setPos(parent->contentX(),Note::NOTE_MARGIN);
    }
}

ColorContent::~ColorContent()
{
  if(note()) note()->removeFromGroup(&m_colorItem);
}

qreal ColorContent::setWidthAndGetHeight(qreal /*width*/) // We do not need width because we can't word-break, and width is always >= minWidth()
{
    return m_colorItem.boundingRect().height();
}

void ColorContent::saveToNode(QDomDocument &doc, QDomElement &content)
{
    QDomText textNode = doc.createTextNode(color().name());
    content.appendChild(textNode);
}

void ColorContent::toolTipInfos(QStringList *keys, QStringList *values)
{
    int hue, saturation, value;
    color().getHsv(&hue, &saturation, &value);

    keys->append(i18nc("RGB Colorspace: Red/Green/Blue", "RGB"));
    values->append(i18n("<i>Red</i>: %1, <i>Green</i>: %2, <i>Blue</i>: %3,", QString::number(color().red()), QString::number(color().green()), QString::number(color().blue())));

    keys->append(i18nc("HSV Colorspace: Hue/Saturation/Value", "HSV"));
    values->append(i18n("<i>Hue</i>: %1, <i>Saturation</i>: %2, <i>Value</i>: %3,", QString::number(hue), QString::number(saturation), QString::number(value)));

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
    m_colorItem.setColor(color);
    contentChanged(m_colorItem.boundingRect().width());
}

void ColorContent::addAlternateDragObjects(QMimeData *dragObject)
{
    dragObject->setColorData(color());
}

void ColorContent::exportToHTML(HTMLExporter *exporter, int /*indent*/)
{
    // FIXME: Duplicate from setColor(): TODO: rectSize()
    QRectF textRect = QFontMetrics(note()->font()).boundingRect(color().name());
    int rectHeight = (textRect.height() + 2) * 3 / 2;
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

/** class UnknownItem:
 */
// TODO: Move this function from note.cpp to class Tools:
extern void drawGradient(QPainter *p, const QColor &colorTop, const QColor & colorBottom,
                         qreal x, qreal y, qreal w, qreal h,
                         bool sunken, bool horz, bool flat);   /*const*/

const qreal UnknownItem::DECORATION_MARGIN = 2;

UnknownItem::UnknownItem(Note *parent)
  :QGraphicsItem(parent)
  , m_note(parent)
{
}

QRectF UnknownItem::boundingRect() const
{
    return QRectF(0, 0, m_textRect.width()+2*DECORATION_MARGIN, m_textRect.height()+2*DECORATION_MARGIN);
}

void UnknownItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    QPalette palette = m_note->basket()->palette();
    qreal width = boundingRect().width();
    qreal height = boundingRect().height();
    painter->setPen(palette.color(QPalette::Active, QPalette::WindowText));

    // FIXME: Duplicate from ColorContent::paint() and CommonColorSelector::drawColorRect:
    // Fill with gradient:
    //drawGradient(painter, palette.color(QPalette::Active, QPalette::WindowText), palette.color(QPalette::Active, QPalette::WindowText).dark(110), 1, 1, width - 2, height - 2, /*sunken=*/false, /*horz=*/true, /*flat=*/false);
    // Stroke:
    QColor stroke = Tools::mixColor(palette.color(QPalette::Active, QPalette::Background), palette.color(QPalette::Active, QPalette::WindowText));
    painter->setPen(stroke);
    painter->drawLine(1,         0,          width - 2, 0);
    painter->drawLine(0,         1,          0,         height - 2);
    painter->drawLine(1,         height - 1, width - 2, height - 1);
    painter->drawLine(width - 1, 1,          width - 1, height - 2);
    // Round corners:
    painter->setPen(Tools::mixColor(palette.color(QPalette::Active, QPalette::Background), stroke));
    painter->drawPoint(1,         1);
    painter->drawPoint(1,         height - 2);
    painter->drawPoint(width - 2, height - 2);
    painter->drawPoint(width - 2, 1);

    painter->setPen(palette.color(QPalette::Active, QPalette::WindowText));
    painter->drawText(DECORATION_MARGIN, DECORATION_MARGIN, width - 2*DECORATION_MARGIN, height - 2*DECORATION_MARGIN,
                      Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap, m_mimeTypes);
}

void UnknownItem::setMimeTypes(QString mimeTypes)
{
    m_mimeTypes = mimeTypes;
    m_textRect = QFontMetrics(m_note->font()).boundingRect(0, 0, 1, 500000, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, m_mimeTypes);
}

void UnknownItem::setWidth(qreal width)
{
    prepareGeometryChange();
    m_textRect = QFontMetrics(m_note->font()).boundingRect(0, 0, width, 500000, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, m_mimeTypes);  
}

/** class UnknownContent:
 */

UnknownContent::UnknownContent(Note *parent, const QString &fileName)
        : NoteContent(parent, fileName)
	, m_unknownItem(parent)
{
    if(parent)
    {
      parent->addToGroup(&m_unknownItem);
      m_unknownItem.setPos(parent->contentX(),Note::NOTE_MARGIN);
    }

    basket()->addWatchedFile(fullPath());
    loadFromFile(/*lazyLoad=*/false);
}

UnknownContent::~UnknownContent()
{
    if(note()) note()->removeFromGroup(&m_unknownItem);
}

qreal UnknownContent::setWidthAndGetHeight(qreal width)
{
    m_unknownItem.setWidth(width);
    return m_unknownItem.boundingRect().height();
}

bool UnknownContent::loadFromFile(bool /*lazyLoad*/)
{
    DEBUG_WIN << "Loading UnknownContent From " + basket()->folderName() + fileName();
    QString mimeTypes;
    QFile file(fullPath());
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        QString line;
        // Get the MIME-types names:
        do {
            if (!stream.atEnd()) {
                line = stream.readLine();
                if (!line.isEmpty()) {
                    if (mimeTypes.isEmpty())
                        mimeTypes += line;
                    else
                        mimeTypes += QString("\n") + line;
                }
            }
        } while (!line.isEmpty() && !stream.atEnd());
        file.close();
    }

    m_unknownItem.setMimeTypes(mimeTypes);
    contentChanged( m_unknownItem.boundingRect().width()+1 );

    return true;
}

void UnknownContent::addAlternateDragObjects(QMimeData *dragObject)
{
    QFile file(fullPath());
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream stream(&file);
        // Get the MIME types names:
        QStringList mimes;
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
        for (int i = 0; i < mimes.count(); ++i) {
            // Get the size:
            stream >> size;
            // Allocate memory to retrieve size bytes and store them:
            array = new QByteArray;
            array->resize(size);
            stream.readRawData(array->data(), size);
            // Creata and add the QDragObject:
            dragObject->setData(mimes.at(i).toAscii(), *array);
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
    if (lowerTypeName == "text")      new TextContent(parent, content.text(), lazyLoad);
    else if (lowerTypeName == "html")      new HtmlContent(parent, content.text(), lazyLoad);
    else if (lowerTypeName == "image")     new ImageContent(parent, content.text(), lazyLoad);
    else if (lowerTypeName == "animation") new AnimationContent(parent, content.text(), lazyLoad);
    else if (lowerTypeName == "sound")     new SoundContent(parent, content.text());
    else if (lowerTypeName == "file")      new FileContent(parent, content.text());
    else if (lowerTypeName == "link") {
        bool autoTitle = content.attribute("title") == content.text();
        bool autoIcon  = content.attribute("icon")  == NoteFactory::iconForURL(KUrl(content.text()));
        autoTitle = XMLWork::trueOrFalse(content.attribute("autoTitle"), autoTitle);
        autoIcon  = XMLWork::trueOrFalse(content.attribute("autoIcon"),  autoIcon);
        new LinkContent(parent, KUrl(content.text()), content.attribute("title"), content.attribute("icon"), autoTitle, autoIcon);
    } else if (lowerTypeName == "cross_reference") {
        new CrossReferenceContent(parent, KUrl(content.text()), content.attribute("title"), content.attribute("icon"));
    } else if (lowerTypeName == "launcher")  new LauncherContent(parent, content.text());
    else if (lowerTypeName == "color")     new ColorContent(parent, QColor(content.text()));
    else if (lowerTypeName == "unknown")   new UnknownContent(parent, content.text());
}


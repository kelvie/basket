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

#include "linklabel.h"

#include <QtCore/QEvent>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QPixmap>
#include <QtGui/QFrame>
#include <QtGui/QCursor>
#include <QtGui/QCheckBox>
#include <QtGui/QPainter>
#include <QtGui/QStyle>
#include <QtGui/QGroupBox>

#include <KDE/KApplication>
#include <KDE/KAboutData>
#include <KDE/KComboBox>
#include <KDE/KLocale>
#include <KDE/KIconLoader>
#include <KDE/KUrl>
#include <KDE/KCModule>

#include "variouswidgets.h"
#include "tools.h"
#include "global.h"
#include "kcolorcombo2.h"
#include "htmlexporter.h"

/** LinkLook */

LinkLook *LinkLook::soundLook       = new LinkLook(/*useLinkColor=*/false, /*canPreview=*/false);
LinkLook *LinkLook::fileLook        = new LinkLook(/*useLinkColor=*/false, /*canPreview=*/true);
LinkLook *LinkLook::localLinkLook   = new LinkLook(/*useLinkColor=*/true,  /*canPreview=*/true);
LinkLook *LinkLook::networkLinkLook = new LinkLook(/*useLinkColor=*/true,  /*canPreview=*/false);
LinkLook *LinkLook::launcherLook    = new LinkLook(/*useLinkColor=*/true,  /*canPreview=*/false);
LinkLook *LinkLook::crossReferenceLook=new LinkLook(/*useLinkColor=*/true,  /*canPreview=*/false);

LinkLook::LinkLook(bool useLinkColor, bool canPreview)
{
    m_useLinkColor = useLinkColor;
    m_canPreview   = canPreview;
    m_iconSize     = 0;
}

LinkLook::LinkLook(const LinkLook &other)
{
    m_useLinkColor = other.useLinkColor();
    m_canPreview   = other.canPreview();
    setLook(other.italic(), other.bold(), other.underlining(),
            other.color(), other.hoverColor(),
            other.iconSize(), other.preview());
}

void LinkLook::setLook(bool italic, bool bold, int underlining,
                       QColor color, QColor hoverColor,
                       int iconSize, int preview)
{
    m_italic      = italic;
    m_bold        = bold;
    m_underlining = underlining;
    m_color       = color;
    m_hoverColor  = hoverColor;
    m_iconSize    = iconSize;
    m_preview     = (canPreview() ? preview : None);
}

int LinkLook::previewSize() const
{
    if (previewEnabled()) {
        switch (preview()) {
        default:
        case None:          return 0;
        case IconSize:      return iconSize();
        case TwiceIconSize: return iconSize() * 2;
        case ThreeIconSize: return iconSize() * 3;
        }
    } else
        return 0;
}

QColor LinkLook::effectiveColor() const
{
    if (m_color.isValid())
        return m_color;
    else
        return defaultColor();
}

QColor LinkLook::effectiveHoverColor() const
{
    if (m_hoverColor.isValid())
        return m_hoverColor;
    else
        return defaultHoverColor();
}

QColor LinkLook::defaultColor() const
{
    if (m_useLinkColor)
        return kapp->palette().color(QPalette::Link);
    else
        return kapp->palette().color(QPalette::Text);
}

QColor LinkLook::defaultHoverColor() const
{
    return Qt::red;
}

LinkLook* LinkLook::lookForURL(const KUrl &url)
{
    return url.isLocalFile() ? localLinkLook : networkLinkLook;
}

QString LinkLook::toCSS(const QString &cssClass, const QColor &defaultTextColor) const
{
    // Set the link class:
    QString css = QString("{ display: block; width: 100%;");
    if (underlineOutside())
        css += " text-decoration: underline;";
    else
        css += " text-decoration: none;";
    if (m_italic == true)
        css += " font-style: italic;";
    if (m_bold == true)
        css += " font-weight: bold;";
    QColor textColor = (color().isValid() || m_useLinkColor ? effectiveColor() : defaultTextColor);
    css += QString(" color: %1; }\n").arg(textColor.name());

   QString css2 = css;
   css.prepend(QString("   .%1 a").arg(cssClass));
   css2.prepend(QString("   a.%1").arg(cssClass));

    // Set the hover state class:
    QString hover;
    if (m_underlining == OnMouseHover)
        hover = "text-decoration: underline;";
    else if (m_underlining == OnMouseOutside)
        hover = "text-decoration: none;";
    if (effectiveHoverColor() != effectiveColor()) {
        if (!hover.isEmpty())
            hover += " ";
        hover += QString("color: %4;").arg(effectiveHoverColor().name());
    }

    // But include it only if it contain a different style than non-hover state:
    if (!hover.isEmpty()) {
        css += QString("   .%1 a:hover { %2 }\n").arg(cssClass, hover);
        css2 += QString("    a:hover.%1 { %2 }\n").arg(cssClass, hover);
    }
    return css + css2;
}

/** LinkLabel */

LinkLabel::LinkLabel(int hAlign, int vAlign, QWidget *parent, Qt::WFlags f)
        : QFrame(parent, f), m_isSelected(false), m_isHovered(false), m_look(0)
{
    initLabel(hAlign, vAlign);
}

LinkLabel::LinkLabel(const QString &title, const QString &icon, LinkLook *look, int hAlign, int vAlign,
                     QWidget *parent, Qt::WFlags f)
        : QFrame(parent, f), m_isSelected(false), m_isHovered(false), m_look(0)
{
    initLabel(hAlign, vAlign);
    setLink(title, icon, look);
}

void LinkLabel::initLabel(int hAlign, int vAlign)
{
    m_layout  = new QBoxLayout(QBoxLayout::LeftToRight, this);
    m_icon    = new QLabel(this);
    m_title   = new QLabel(this);
    m_spacer1 = new QSpacerItem(0, 0, QSizePolicy::Preferred/*Expanding*/, QSizePolicy::Preferred/*Expanding*/);
    m_spacer2 = new QSpacerItem(0, 0, QSizePolicy::Preferred/*Expanding*/, QSizePolicy::Preferred/*Expanding*/);

    m_hAlign = hAlign;
    m_vAlign = vAlign;

    m_title->setTextFormat(Qt::PlainText);

    // DEGUB:
    //m_icon->setPaletteBackgroundColor("lightblue");
    //m_title->setPaletteBackgroundColor("lightyellow");
}

LinkLabel::~LinkLabel()
{
}

void LinkLabel::setLink(const QString &title, const QString &icon, LinkLook *look)
{
    if (look)
        m_look = look; // Needed for icon size

    m_title->setText(title);
    m_title->setShown(! title.isEmpty());

    if (icon.isEmpty())
        m_icon->clear();
    else {
        QPixmap pixmap = DesktopIcon(icon, m_look->iconSize(), m_look->iconSize());
        if (!pixmap.isNull())
            m_icon->setPixmap(pixmap);
    }
    m_icon->setShown(! icon.isEmpty());

    if (look)
        setLook(look);
}

void LinkLabel::setLook(LinkLook *look) // FIXME: called externaly (so, without setLink()) it's buggy (icon not
{
    m_look = look;

    QFont font;
    font.setBold(look->bold());
    font.setUnderline(look->underlineOutside());
    font.setItalic(look->italic());
    m_title->setFont(font);
    QPalette palette;
    if (m_isSelected)
        palette.setColor(m_title->foregroundRole(), KApplication::palette().color(QPalette::Text));
    else
        palette.setColor(m_title->foregroundRole(), look->effectiveColor());

    m_title->setPalette(palette);


    m_icon->setVisible(m_icon->pixmap() && ! m_icon->pixmap()->isNull());

    setAlign(m_hAlign, m_vAlign);
}

void LinkLabel::setAlign(int hAlign, int vAlign)
{
    m_hAlign = hAlign;
    m_vAlign = vAlign;

    if (!m_look)
        return;

    // Define alignment flags :
    Qt::Alignment hFlag, vFlag;
    switch (hAlign) {
    default:
    case 0: hFlag = Qt::AlignLeft;    break;
    case 1: hFlag = Qt::AlignHCenter; break;
    case 2: hFlag = Qt::AlignRight;   break;
    }
    switch (vAlign) {
    case 0: vFlag = Qt::AlignTop;     break;
    default:
    case 1: vFlag = Qt::AlignVCenter; break;
    case 2: vFlag = Qt::AlignBottom;  break;
    }

    // Clear the widget :
    m_layout->removeItem(m_spacer1);
    m_layout->removeWidget(m_icon);
    m_layout->removeWidget(m_title);
    m_layout->removeItem(m_spacer2);

    // Otherwise, minimumSize will be incoherent (last size ? )
    m_layout->setSizeConstraint(QLayout::SetMinimumSize);

    // And re-populate the widget with the appropriates things and order
    bool addSpacers = (hAlign == 1);
    m_layout->setDirection(QBoxLayout::LeftToRight);
    //m_title->setSizePolicy( QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum/*Expanding*/, 0, 0, false) );
    m_icon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    m_spacer1->changeSize(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_spacer2->changeSize(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_icon->setAlignment(hFlag | vFlag);
    m_title->setAlignment(hFlag | vFlag);
    if (hAlign)
        m_title->setWordWrap(true);

    if ((addSpacers && (vAlign != 0)) ||
            (m_title->text().isEmpty() && hAlign == 2))
        m_layout->addItem(m_spacer1);
    if (hAlign == 2) { // If align at right, icon is at right
        m_layout->addWidget(m_title);
        m_layout->addWidget(m_icon);
    } else {
        m_layout->addWidget(m_icon);
        m_layout->addWidget(m_title);
    }
    if ((addSpacers && (vAlign != 2)) ||
            (m_title->text().isEmpty() && hAlign == 0))
        m_layout->addItem(m_spacer2);
}

void LinkLabel::enterEvent(QEvent*)
{
    m_isHovered = true;

    if (!m_isSelected) {
        QPalette palette;
        palette.setColor(m_title->foregroundRole(), m_look->effectiveHoverColor());
        m_title->setPalette(palette);
    }

    QFont font = m_title->font();
    font.setUnderline(m_look->underlineInside());
    m_title->setFont(font);
}

void LinkLabel::leaveEvent(QEvent*)
{
    m_isHovered = false;

    if (!m_isSelected) {
        QPalette palette;
        palette.setColor(m_title->foregroundRole(), m_look->effectiveColor());
        m_title->setPalette(palette);
    }

    QFont font = m_title->font();
    font.setUnderline(m_look->underlineOutside());
    m_title->setFont(font);
}

void LinkLabel::setSelected(bool selected)
{
    m_isSelected = selected;
    QPalette palette;

    if (selected)
        palette.setColor(m_title->foregroundRole(), KApplication::palette().color(QPalette::HighlightedText));
    else if (m_isHovered)
        palette.setColor(m_title->foregroundRole(), m_look->effectiveHoverColor());
    else
        palette.setColor(m_title->foregroundRole(), m_look->effectiveColor());

    m_title->setPalette(palette);
}

void LinkLabel::setPaletteBackgroundColor(const QColor &color)
{
    QPalette framePalette;
    framePalette.setColor(QFrame::foregroundRole(), color);
    QFrame::setPalette(framePalette);

    QPalette titlePalette;
    titlePalette.setColor(m_title->foregroundRole(), color);
    m_title->setPalette(titlePalette);
}

int LinkLabel::heightForWidth(int w) const
{
    int iconS  = (m_icon->isVisible()) ? m_look->iconSize()                 : 0; // Icon size
    int iconW  = iconS;                                                          // Icon width to remove to w
    int titleH = (m_title->isVisible()) ? m_title->heightForWidth(w - iconW) : 0; // Title height

    return (titleH >= iconS) ? titleH : iconS; // No margin for the moment !
}

/** class LinkDisplay
 */

LinkDisplay::LinkDisplay()
        : m_title(), m_icon(), m_preview(), m_look(0), m_font(), m_minWidth(0), m_width(0), m_height(0)
{
}

void LinkDisplay::setLink(const QString &title, const QString &icon, LinkLook *look, const QFont &font)
{
    setLink(title, icon, m_preview, look, font);
}

void LinkDisplay::setLink(const QString &title, const QString &icon, const QPixmap &preview, LinkLook *look, const QFont &font)
{
    m_title   = title;
    m_icon    = icon;
    m_preview = preview;
    m_look    = look;
    m_font    = font;

    // "Constants":
    int BUTTON_MARGIN = kapp->style()->pixelMetric(QStyle::PM_ButtonMargin);
    int LINK_MARGIN   = BUTTON_MARGIN + 2;

    // Recompute m_minWidth:
    QRect textRect = QFontMetrics(labelFont(font, false)).boundingRect(0, 0, /*width=*/1, 500000, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, m_title);
    int iconPreviewWidth = qMax(m_look->iconSize(), (m_look->previewEnabled() ? m_preview.width() : 0));
    m_minWidth = BUTTON_MARGIN - 1 + iconPreviewWidth + LINK_MARGIN + textRect.width();
    // Recompute m_maxWidth:
    textRect = QFontMetrics(labelFont(font, false)).boundingRect(0, 0, /*width=*/50000000, 500000, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, m_title);
    m_maxWidth = BUTTON_MARGIN - 1 + iconPreviewWidth + LINK_MARGIN + textRect.width();
    // Adjust m_width:
    if (m_width < m_minWidth)
        setWidth(m_minWidth);
    // Recompute m_height:
    m_height = heightForWidth(m_width);
}

void LinkDisplay::setWidth(int width)
{
    if (width < m_minWidth)
        width = m_minWidth;

    if (width != m_width) {
        m_width  = width;
        m_height = heightForWidth(m_width);
    }
}

/** Paint on @p painter
  *       in (@p x, @p y, @p width, @p height)
  *       using @p palette for the button drawing (if @p isHovered)
  *       and the LinkLook color() for the text,
  *       unless [the LinkLook !color.isValid() and it does not useLinkColor()] or [@p isDefaultColor is false]: in this case it will use @p palette's active text color.
  *       It will draw the button if @p isIconButtonHovered.
  */
void LinkDisplay::paint(QPainter *painter, int x, int y, int width, int height, const QPalette &palette,
                        bool isDefaultColor, bool isSelected, bool isHovered, bool isIconButtonHovered) const
{
    int BUTTON_MARGIN = kapp->style()->pixelMetric(QStyle::PM_ButtonMargin);
    int LINK_MARGIN   = BUTTON_MARGIN + 2;

    QPixmap pixmap;
    // Load the preview...:
    if (!isHovered && m_look->previewEnabled() && !m_preview.isNull())
        pixmap  = m_preview;
    // ... Or the icon (if no preview or if the "Open" icon should be shown):
    else {
        int           iconSize   = m_look->iconSize();
        QString       iconName   = (isHovered ? Global::openNoteIcon() : m_icon);
        KIconLoader::States iconState  = (isIconButtonHovered ? KIconLoader::ActiveState : KIconLoader::DefaultState);
        pixmap = KIconLoader::global()->loadIcon(
                     iconName, KIconLoader::Desktop, iconSize, iconState, QStringList(),
                     0L, /*canReturnNull=*/false
                 );
    }
    int iconPreviewWidth  = qMax(m_look->iconSize(), (m_look->previewEnabled() ? m_preview.width()  : 0));
    int pixmapX = (iconPreviewWidth - pixmap.width()) / 2;
    int pixmapY = (height - pixmap.height()) / 2;
    // Draw the button (if any) and the icon:
    if (isHovered) {
        QStyleOption opt;
        opt.rect = QRect(-1, -1, iconPreviewWidth + 2 * BUTTON_MARGIN, height + 2);
        opt.state = isIconButtonHovered ? (QStyle::State_MouseOver | QStyle::State_Enabled)  : QStyle::State_Enabled;
        kapp->style()->drawPrimitive(QStyle::PE_PanelButtonCommand, &opt, painter);
    }
    painter->drawPixmap(x + BUTTON_MARGIN - 1 + pixmapX, y + pixmapY, pixmap);

    // Figure out the text color:
    if (isSelected) {
        painter->setPen(kapp->palette().color(QPalette::HighlightedText));
    } else if (isIconButtonHovered)
        painter->setPen(m_look->effectiveHoverColor());
    else if (!isDefaultColor || (!m_look->color().isValid() && !m_look->useLinkColor())) // If the color is FORCED or if the link color default to the text color:
        painter->setPen(palette.color(QPalette::Active, QPalette::WindowText));
    else
        painter->setPen(m_look->effectiveColor());
    // Draw the text:
    painter->setFont(labelFont(m_font, isIconButtonHovered));
    painter->drawText(x + BUTTON_MARGIN - 1 + iconPreviewWidth + LINK_MARGIN, y, width - BUTTON_MARGIN + 1 - iconPreviewWidth - LINK_MARGIN, height,
                      Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap, m_title);
}

QPixmap LinkDisplay::feedbackPixmap(int width, int height, const QPalette &palette, bool isDefaultColor)
{
    int theWidth  = qMin(width, maxWidth());
    int theHeight = qMin(height, heightForWidth(theWidth));
    QPixmap pixmap(theWidth, theHeight);
    pixmap.fill(palette.color(QPalette::Active, QPalette::Background));
    QPainter painter(&pixmap);
    paint(&painter, 0, 0, theWidth, theHeight, palette, isDefaultColor,
          /*isSelected=*/false, /*isHovered=*/false, /*isIconButtonHovered=*/false);
    painter.end();
    return pixmap;
}

bool LinkDisplay::iconButtonAt(const QPoint &pos) const
{
    int BUTTON_MARGIN    = kapp->style()->pixelMetric(QStyle::PM_ButtonMargin);
//  int LINK_MARGIN      = BUTTON_MARGIN + 2;
    int iconPreviewWidth = qMax(m_look->iconSize(), (m_look->previewEnabled() ? m_preview.width()  : 0));

    return pos.x() <= BUTTON_MARGIN - 1 + iconPreviewWidth + BUTTON_MARGIN;
}

QRect LinkDisplay::iconButtonRect() const
{
    int BUTTON_MARGIN    = kapp->style()->pixelMetric(QStyle::PM_ButtonMargin);
//  int LINK_MARGIN      = BUTTON_MARGIN + 2;
    int iconPreviewWidth = qMax(m_look->iconSize(), (m_look->previewEnabled() ? m_preview.width()  : 0));

    return QRect(0, 0, BUTTON_MARGIN - 1 + iconPreviewWidth + BUTTON_MARGIN, m_height);
}

QFont LinkDisplay::labelFont(QFont font, bool isIconButtonHovered) const
{
    if (m_look->italic())
        font.setItalic(true);
    if (m_look->bold())
        font.setBold(true);
    if (isIconButtonHovered) {
        if (m_look->underlineInside())
            font.setUnderline(true);
    } else {
        if (m_look->underlineOutside())
            font.setUnderline(true);
    }
    return font;
}

int LinkDisplay::heightForWidth(int width) const
{
    int BUTTON_MARGIN     = kapp->style()->pixelMetric(QStyle::PM_ButtonMargin);
    int LINK_MARGIN       = BUTTON_MARGIN + 2;
    int iconPreviewWidth  = qMax(m_look->iconSize(), (m_look->previewEnabled() ? m_preview.width()  : 0));
    int iconPreviewHeight = qMax(m_look->iconSize(), (m_look->previewEnabled() ? m_preview.height() : 0));

    QRect textRect = QFontMetrics(labelFont(m_font, false)).boundingRect(0, 0, width - BUTTON_MARGIN + 1 - iconPreviewWidth - LINK_MARGIN, 500000, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, m_title);
    return qMax(textRect.height(), iconPreviewHeight + 2*BUTTON_MARGIN - 2);
}

QString LinkDisplay::toHtml(const QString &/*imageName*/) const
{
    // TODO
    return "";
}

QString LinkDisplay::toHtml(HTMLExporter *exporter, const KUrl &url, const QString &title)
{
    QString linkIcon;
    if (m_look->previewEnabled() && !m_preview.isNull()) {
        QString fileName = Tools::fileNameForNewFile("preview_" + url.fileName() + ".png", exporter->iconsFolderPath);
        QString fullPath = exporter->iconsFolderPath + fileName;
        m_preview.save(fullPath, "PNG");
        linkIcon = QString("<img src=\"%1\" width=\"%2\" height=\"%3\" alt=\"\">")
                   .arg(exporter->iconsFolderName + fileName, QString::number(m_preview.width()), QString::number(m_preview.height()));
    } else {
        linkIcon = exporter->iconsFolderName + exporter->copyIcon(m_icon, m_look->iconSize());
        linkIcon = QString("<img src=\"%1\" width=\"%2\" height=\"%3\" alt=\"\">")
                   .arg(linkIcon, QString::number(m_look->iconSize()), QString::number(m_look->iconSize()));
    }

    QString linkTitle = Tools::textToHTMLWithoutP(title.isEmpty() ? m_title : title);

    return QString("<a href=\"%1\">%2 %3</a>").arg(url.prettyUrl(), linkIcon, linkTitle);
}

/** LinkLookEditWidget **/

LinkLookEditWidget::LinkLookEditWidget(KCModule *module, const QString exTitle, const QString exIcon,
                                       QWidget *parent, Qt::WFlags fl)
        : QWidget(parent, fl)
{
    QLabel      *label;
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_italic = new QCheckBox(i18n("I&talic"), this);
    layout->addWidget(m_italic);

    m_bold = new QCheckBox(i18n("&Bold"), this);
    layout->addWidget(m_bold);

    QGridLayout *gl = new QGridLayout;
    layout->addLayout(gl);
    gl->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding), 1, /*2*/3);

    m_underlining = new KComboBox(this);
    m_underlining->addItem(i18n("Always"));
    m_underlining->addItem(i18n("Never"));
    m_underlining->addItem(i18n("On mouse hovering"));
    m_underlining->addItem(i18n("When mouse is outside"));
    label = new QLabel(this);
    label->setText(i18n("&Underline:"));
    label->setBuddy(m_underlining);
    gl->addWidget(label, 0, 0);
    gl->addWidget(m_underlining, 0, 1);

    m_color = new KColorCombo2(QRgb(), this);
    label = new QLabel(this);
    label->setText(i18n("Colo&r:"));
    label->setBuddy(m_color);
    gl->addWidget(label,   1, 0);
    gl->addWidget(m_color, 1, 1);

    m_hoverColor = new KColorCombo2(QRgb(), this);
    label = new QLabel(this);
    label->setText(i18n("&Mouse hover color:"));
    label->setBuddy(m_hoverColor);
    gl->addWidget(label,        2, 0);
    gl->addWidget(m_hoverColor, 2, 1);

    QHBoxLayout *icoLay = new QHBoxLayout(0);
    m_iconSize = new IconSizeCombo(this);
    icoLay->addWidget(m_iconSize);
    label = new QLabel(this);
    label->setText(i18n("&Icon size:"));
    label->setBuddy(m_iconSize);
    gl->addWidget(label,  3, 0);
    gl->addItem(icoLay, 3, 1);

    m_preview = new KComboBox(this);
    m_preview->addItem(i18n("None"));
    m_preview->addItem(i18n("Icon size"));
    m_preview->addItem(i18n("Twice the icon size"));
    m_preview->addItem(i18n("Three times the icon size"));
    m_label = new QLabel(this);
    m_label->setText(i18n("&Preview:"));
    m_label->setBuddy(m_preview);
    m_hLabel = new HelpLabel(
        i18n("You disabled preview but still see images?"),
        i18n("<p>This is normal because there are several type of notes.<br>"
             "This setting only applies to file and local link notes.<br>"
             "The images you see are image notes, not file notes.<br>"
             "File notes are generic documents, whereas image notes are pictures you can draw in.</p>"
             "<p>When dropping files to baskets, %1 detects their type and shows you the content of the files.<br>"
             "For instance, when dropping image or text files, image and text notes are created for them.<br>"
             "For type of files %2 does not understand, they are shown as generic file notes with just an icon or file preview and a filename.</p>"
             "<p>If you do not want the application to create notes depending on the content of the files you drop, "
             "go to the \"General\" page and uncheck \"Image or animation\" in the \"View Content of Added Files for the Following Types\" group.</p>",
             // TODO: Note: you can resize down maximum size of images...
             KGlobal::mainComponent().aboutData()->programName(), KGlobal::mainComponent().aboutData()->programName()),
        this);
    gl->addWidget(m_label,   4, 0);
    gl->addWidget(m_preview, 4, 1);
    gl->addWidget(m_hLabel, 5, 1, 1, 2);

    QGroupBox *gb = new QGroupBox(i18n("Example"), this);
    QHBoxLayout* gbLayout = new QHBoxLayout;
    gb->setLayout(gbLayout);

    m_exLook = new LinkLook;
    m_example = new LinkLabel(exTitle, exIcon, m_exLook, 1, 1);
    gbLayout->addWidget(m_example);
    m_example->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_example->setCursor(QCursor(Qt::PointingHandCursor));
    layout->addWidget(gb);
    m_exTitle = exTitle;
    m_exIcon  = exIcon;

    connect(m_italic,      SIGNAL(stateChanged(int)),      this,   SLOT(slotChangeLook()));
    connect(m_bold,        SIGNAL(stateChanged(int)),      this,   SLOT(slotChangeLook()));
    connect(m_underlining, SIGNAL(activated(int)),         this,   SLOT(slotChangeLook()));
    connect(m_color,       SIGNAL(activated(const QColor&)), this,   SLOT(slotChangeLook()));
    connect(m_hoverColor,  SIGNAL(activated(const QColor&)), this,   SLOT(slotChangeLook()));
    connect(m_iconSize,    SIGNAL(activated(int)),         this,   SLOT(slotChangeLook()));
    connect(m_preview,     SIGNAL(activated(int)),         this,   SLOT(slotChangeLook()));

    connect(m_italic,      SIGNAL(stateChanged(int)),      module, SLOT(changed()));
    connect(m_bold,        SIGNAL(stateChanged(int)),      module, SLOT(changed()));
    connect(m_underlining, SIGNAL(activated(int)),         module, SLOT(changed()));
    connect(m_color,       SIGNAL(activated(const QColor&)), module, SLOT(changed()));
    connect(m_hoverColor,  SIGNAL(activated(const QColor&)), module, SLOT(changed()));
    connect(m_iconSize,    SIGNAL(activated(int)),         module, SLOT(changed()));
    connect(m_preview,     SIGNAL(activated(int)),         module, SLOT(changed()));
}

void LinkLookEditWidget::set(LinkLook *look)
{
    m_look = look;

    m_italic->setChecked(look->italic());
    m_bold->setChecked(look->bold());
    m_underlining->setCurrentIndex(look->underlining());
    m_preview->setCurrentIndex(look->preview());
    m_color->setDefaultColor(m_look->defaultColor());
    m_color->setColor(m_look->color());
    m_hoverColor->setDefaultColor(m_look->defaultHoverColor());
    m_hoverColor->setColor(m_look->hoverColor());
    m_iconSize->setSize(look->iconSize());
    m_exLook = new LinkLook(*look);
    m_example->setLook(m_exLook);

    if (!look->canPreview()) {
        m_label->setEnabled(false);
        m_hLabel->setEnabled(false);
        m_preview->setEnabled(false);
    }
    slotChangeLook();
}

void LinkLookEditWidget::slotChangeLook()
{
    saveToLook(m_exLook);
    m_example->setLink(m_exTitle, m_exIcon, m_exLook); // and can't reload it at another size
}

LinkLookEditWidget::~LinkLookEditWidget()
{
}

void LinkLookEditWidget::saveChanges()
{
    saveToLook(m_look);
}

void LinkLookEditWidget::saveToLook(LinkLook *look)
{
    look->setLook(m_italic->isChecked(), m_bold->isChecked(), m_underlining->currentIndex(),
                  m_color->color(), m_hoverColor->color(),
                  m_iconSize->iconSize(), (look->canPreview() ? m_preview->currentIndex() : LinkLook::None));
}


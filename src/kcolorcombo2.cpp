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

#include "kcolorcombo2.h"

#ifndef USE_OLD_KCOLORCOMBO

#include <qapplication.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <QListWidget>
//Added by qt3to4:
#include <QDropEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QDragEnterEvent>
#include <QDebug>
#include <klocale.h>
#include <kcolordialog.h>
#include <qclipboard.h>
#include <kstdaccel.h>
#include <kglobalsettings.h>

//#define DEBUG_COLOR_ARRAY
//#define OUTPUT_GIMP_PALETTE

#ifdef DEBUG_COLOR_ARRAY
#include <iomanip>
#endif
#ifdef OUTPUT_GIMP_PALETTE
#include <iomanip>
#endif


/** class KColorPopup: */

const int KColorPopup::MARGIN      = 1;
const int KColorPopup::FRAME_WIDTH = 1;


KColorPopup::KColorPopup(KColorCombo2 *parent)
        : QWidget(/*parent=*/0, Qt::Popup),
        m_selector(parent), m_pixmap(0)
{
    hide();
    setMouseTracking(true);
    //resize(20, 20);
}

KColorPopup::~KColorPopup()
{
    delete m_pixmap;
}

#include <qcursor.h>

void KColorPopup::relayout() // FIXME: relayout should NOT redraw the pixmap!
{
    int  columnCount = m_selector->columnCount();
    int  rowCount    = m_selector->rowCount();
    int  colorHeight = m_selector->colorRectHeight();
    int  colorWidth  = m_selector->colorRectWidthForHeight(colorHeight);
    bool haveDefault = m_selector->defaultColor().isValid();

    int width  = 2 + MARGIN + (colorWidth  + MARGIN) * columnCount;
    int height = 2 + MARGIN + (colorHeight + MARGIN) * rowCount + (colorHeight + MARGIN);

    resize(width, height);

    // Initialize the pixmap:
    delete m_pixmap;
    m_pixmap = new QPixmap(width, height);
    QPainter painter(m_pixmap);
    painter.fillRect(0, 0, width, height, palette().color(QPalette::Base));
    painter.setPen(palette().color(QPalette::Text));
    painter.drawRect(0, 0, width, height);

    // Needed to draw:
    int x, y;
    QRect selectionRect;

    // Draw the color array:
    for (int i = 0; i < columnCount; ++i) {
        for (int j = 0; j < rowCount; ++j) {
            x = 1 + MARGIN + (colorWidth  + MARGIN) * i;
            y = 1 + MARGIN + (colorHeight + MARGIN) * j;
            if (i == m_selectedColumn && j == m_selectedRow) {
                selectionRect = QRect(x - 2, y - 2, colorWidth + 4, colorHeight + 4);
                painter.fillRect(selectionRect, palette().color(QPalette::Highlight));
            }
            m_selector->drawColorRect(painter, x, y, m_selector->colorAt(i, j), /*isDefault=*/false, colorWidth, colorHeight);
        }
    }

    m_columnOther = (haveDefault ? columnCount / 2 : 0); // "(Default)" is allowed, paint "Other..." on the right
    int defaultCellWidth = (colorWidth  + MARGIN) * m_columnOther;
    int otherCellWidth   = (colorWidth  + MARGIN) * (columnCount - m_columnOther);

    // Draw the "(Default)" and "Other..." colors:
    y = height - (colorHeight + MARGIN) - 1;
    QColor textColor;
    if (m_selector->defaultColor().isValid()) {
        x = 1 + MARGIN;
        if (m_selectedColumn < m_columnOther && rowCount == m_selectedRow) {
            selectionRect = QRect(x - 2, y - 2, defaultCellWidth, colorHeight + 4);
            painter.fillRect(selectionRect, palette().color(QPalette::Highlight));
            textColor = palette().color(QPalette::HighlightedText);
        } else
            textColor = palette().color(QPalette::Text);
        m_selector->drawColorRect(painter, x, y, m_selector->defaultColor(), /*isDefault=*/true, colorWidth, colorHeight);
        painter.setFont(m_selector->font());
        painter.setPen(textColor);
        painter.drawText(x + 2 + colorWidth, y, /*width=*/5000, colorHeight, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextDontClip, i18n("(Default)"));
    }
    x = 1 + MARGIN + m_columnOther * (colorWidth + MARGIN);
    if (m_selectedColumn >= m_columnOther && rowCount == m_selectedRow) {
        selectionRect = QRect(x - 2, y - 2, otherCellWidth, colorHeight + 4);
        painter.fillRect(selectionRect, palette().color(QPalette::Highlight));
        textColor = palette().color(QPalette::HighlightedText);
    } else
        textColor = palette().color(QPalette::Text);
    m_selector->drawColorRect(painter, x, y, m_otherColor, /*isDefault=*/false, colorWidth, colorHeight);
    painter.setFont(m_selector->font());
    painter.setPen(textColor);
    painter.drawText(x + 2 + colorWidth, y, /*width=*/5000, colorHeight, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextDontClip, i18n("Other..."));

//  QPoint pos = mapFromGlobal(QCursor::pos());
//  painter.drawRect(pos.x(), pos.y(), 5000, 5000);
}

void KColorPopup::updateCell(int column, int row)
{
    int  colorHeight = m_selector->colorRectHeight();
    int  colorWidth  = m_selector->colorRectWidthForHeight(colorHeight);

    int x      = 1 + MARGIN + - 2 + column * (colorWidth  + MARGIN);
    int y      = 1 + MARGIN + - 2 + row    * (colorHeight + MARGIN);
    int width  = colorWidth  + MARGIN;
    int height = colorHeight + MARGIN;

    if (row == m_selector->rowCount()) {
        if (m_selectedColumn < m_columnOther) // The "(Default)" cell:
            width = (colorWidth  + MARGIN) * m_columnOther;
        else // The "Other..." cell:
            width = (colorWidth  + MARGIN) * (m_selector->columnCount() - m_columnOther);
    }

    update(x, y, width, height);
}

void KColorPopup::doSelection()
{
    m_otherColor = QColor();

    // If the selected color is not the default one, try to find it in the array:
    if (m_selector->color().isValid()) {
        bool isInArray = false;
        for (int column = 0; column < m_selector->columnCount(); ++column)
            for (int row = 0; row < m_selector->rowCount(); ++row)
                if (m_selector->color() == m_selector->colorAt(column, row)) {
                    m_selectedColumn = column;
                    m_selectedRow    = row;
                    isInArray        = true;
                }
        // If not found in array, it's another one:
        if (!isInArray) {
            m_selectedColumn = m_columnOther;
            m_selectedRow    = m_selector->rowCount();
            m_otherColor     = m_selector->color();
        }
        // If it's the default one:
    } else {
        m_selectedColumn = 0;
        m_selectedRow    = m_selector->rowCount();
    }
}

void KColorPopup::validate()
{
    hide();
    close();
    emit closed();

    if (m_selectedRow != m_selector->rowCount()) // A normal row:
        m_selector->setColor(m_selector->colorAt(m_selectedColumn, m_selectedRow));
    else if (m_selectedColumn < m_columnOther) // The default color:
        m_selector->setColor(QColor());
    else { // The user want to choose one:
        QColor color = m_selector->effectiveColor();
        if (KColorDialog::getColor(color, this) == QDialog::Accepted)
            m_selector->setColor(color);
    }
}

void KColorPopup::mousePressEvent(QMouseEvent *event)
{
    int x = event->pos().x();
    int y = event->pos().y();
    if (x < 0 || y < 0 || x >= width() || y >= height()) {
        hide();
        close();
        emit closed();
    } else
        validate();

    event->accept();
}

void KColorPopup::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    if (m_pixmap)
        painter.drawPixmap(0, 0, *m_pixmap);
    painter.setPen(Qt::black);
    painter.drawRect(event->rect());
}

void KColorPopup::mouseMoveEvent(QMouseEvent *event)
{
    int x = event->pos().x();
    int y = event->pos().y();
    if (x < FRAME_WIDTH + 2 || y < FRAME_WIDTH + 2 || x > width() - 2 - 2*FRAME_WIDTH || y > height() - 2 - 2*FRAME_WIDTH)
        return;

    int colorHeight = m_selector->colorRectHeight();
    int colorWidth  = m_selector->colorRectWidthForHeight(colorHeight);

//  int oldSelectedColumn = m_selectedColumn;
//  int oldSelectedRow    = m_selectedRow;
    m_selectedColumn = (x - FRAME_WIDTH - MARGIN + 2) / (colorWidth  + MARGIN);
    m_selectedRow    = (y - FRAME_WIDTH - MARGIN + 2) / (colorHeight + MARGIN);

    relayout();
    update();
}

void KColorPopup::keyPressEvent(QKeyEvent *event)
{
    int column      = m_selectedColumn;
    int row         = m_selectedRow;
    int columnCount = m_selector->columnCount();
    int rowCount    = m_selector->rowCount();

    switch (event->key()) {
    case Qt::Key_Right:
        if (m_selectedRow != rowCount) // A normal row:
            column = (column + 1) % columnCount;
        else {
            // The last row, if there are two choices, switch. Else, do nothing:
            if (m_selector->defaultColor().isValid())
                column = (m_selectedColumn < m_columnOther ? m_columnOther : 0);
        }
        break;
    case Qt::Key_Left:
        if (m_selectedRow != rowCount) { // A normal row:
            column = (column - 1);
            if (column < 0)
                column = columnCount - 1;
        } else {
            // The last row, if there are two choices, switch. Else, do nothing:
            if (m_selector->defaultColor().isValid())
                column = (m_selectedColumn < m_columnOther ? m_columnOther : 0);
        }
        break;
    case Qt::Key_Up:       row    = (row    - 1); if (row < 0)    row    = rowCount;        break;
    case Qt::Key_Down:     row    = (row    + 1) % (rowCount+1);          break;
    case Qt::Key_PageDown: row += 10; if (row > rowCount) row = rowCount; break;
    case Qt::Key_PageUp:   row -= 10; if (row < 0)        row = 0;        break;
    case Qt::Key_Home:     row = 0;        column = 0;                    break;
    case Qt::Key_End:      row = rowCount; column = columnCount - 1;      break;
    case Qt::Key_Return:
        validate();
        break;
    default:
        QWidget::keyPressEvent(event);
    }

    if (row != m_selectedRow || column != m_selectedColumn) {
        m_selectedRow    = row;
        m_selectedColumn = column;
        relayout();
        update();
    }
}

/** Helper function: */

QColor Tool_mixColors(const QColor &color1, const QColor &color2)
{
    QColor mixedColor;
    mixedColor.setRgb((color1.red()   + color2.red())   / 2,
                      (color1.green() + color2.green()) / 2,
                      (color1.blue()  + color2.blue())  / 2);
    return mixedColor;
}

/** class KColorCombo2Private */

class KColorCombo2::KColorCombo2Private
{
};

/** class KColorCombo2: */

/* All code for the popup management (including the constructor, popup() and eventFilter())
 * has been copied from the KDateEdit widget (in libkdepim).
 *
 * Some other piece of code comes from KColorButton (in libkdeui) to enable color drag, drop, copy and paste.
 */

KColorCombo2::KColorCombo2(const QColor &color, const QColor &defaultColor, QWidget *parent)
        : QComboBox(parent),
        m_color(color), m_defaultColor(defaultColor)
{
    setEditable(false);
    init();
}

KColorCombo2::KColorCombo2(const QColor &color, QWidget *parent)
        : QComboBox(parent),
        m_color(color), m_defaultColor()
{
    setEditable(false);
    init();
}

void KColorCombo2::init()
{
    m_colorArray            = 0;
    d                       = new KColorCombo2Private();

    setDefaultColor(m_defaultColor);
    insertItem(/*index=*/0, "");
    updateComboBox(); // It need an item of index 0 to exists, so we created it.
    setAcceptDrops(true);

    m_popup = new KColorPopup(this);
    m_popup->installEventFilter(this);
    connect(m_popup, SIGNAL(closed()), SLOT(popupClosed()));

    // By default, the array is filled with setRainbowPreset().
    // But we allocate it on demand (the later as possible) to avoid performances issues if the developer set another array.
    // However, to keep columnCount() rowCount() const, we define theme here:
    m_columnCount = 13;
    m_rowCount    = 9;
}

KColorCombo2::~KColorCombo2()
{
    deleteColorArray();
}

void KColorCombo2::setColor(const QColor &color)
{
    // Do nothing if the color should be set to the default one and there is no such default color allowed:
    if (!color.isValid() && !m_defaultColor.isValid()) {
        // kdebug << this::FUNCTION << "Trying to assign the default color (an invalid one) whereas no such default color is allowed";
        return;
    }

    if (m_color != color) {
        m_color = color;
        updateComboBox();
        emit changed(color);
    }
}

QColor KColorCombo2::color() const
{
    return m_color;
}

QColor KColorCombo2::effectiveColor() const
{
    if (m_color.isValid())
        return m_color;
    else
        return m_defaultColor;
}

void KColorCombo2::setRainbowPreset(int colorColumnCount, int lightRowCount, int darkRowCount, bool withGray)
{
    // At least one row and one column:
    if (colorColumnCount < 1 - (withGray ? 1 : 0))
        colorColumnCount = 1 - (withGray ? 1 : 0);
    if (lightRowCount < 0)
        lightRowCount = 0;
    if (darkRowCount < 0)
        darkRowCount = 0;

    // Create the array:
    int  columnCount = colorColumnCount + (withGray ? 1 : 0);
    int  rowCount    = lightRowCount + 1 + darkRowCount;
    newColorArray(columnCount, rowCount);

    // Fill the array:
    for (int i = 0; i < colorColumnCount; ++i) {
        int hue = i * 360 / colorColumnCount;
        // With light colors:
        for (int j = 1; j <= lightRowCount; ++j) { // Start to 1 because we don't want a row full of white!
            int saturation = j * 255 / (lightRowCount + 1);
            setColorAt(i, j - 1, QColor::fromHsv(hue, saturation, 255));
        }
        // With pure colors:
        setColorAt(i, lightRowCount, QColor::fromHsv(hue, 255, 255));
        // With dark colors:
        for (int j = 1; j <= darkRowCount; ++j) {
            int value = 255 - j * 255 / (darkRowCount + 1);
            setColorAt(i, lightRowCount + j, QColor::fromHsv(hue, 255, value));
        }
    }

    // Fill the gray column:
    if (withGray) {
        for (int i = 0; i < rowCount; ++i) {
            int gray = (rowCount == 1  ?  128  :  255 - (i * 255 / (rowCount - 1)));
            setColorAt(columnCount - 1, i, QColor(gray, gray, gray));
        }
    }

#ifdef DEBUG_COLOR_ARRAY
    kDebug() << "KColorCombo2::setColorPreset";
    for (int j = 0; j < rowCount; ++j) {
        for (int i = 0; i < columnCount; ++i) {
            int h, s, v;
            m_colorArray[i][j].getHsv(h, s, v);
            kDebug() << QString("(%1,%2,%3)").arg(h, 3).arg(s, 3).arg(v, 3);
            //kDebug() << colorArray[i][j].name() << " ";
        }
        kDebug();
    }
#endif
#ifdef OUTPUT_GIMP_PALETTE
    kDebug() << "GIMP Palette";
    for (int j = 0; j < rowCount; ++j) {
        for (int i = 0; i < columnCount; ++i) {
            kDebug() << QString("(%1,%2,%3)")
            .arg(m_colorArray[i][j].red(), 3)
            .arg(m_colorArray[i][j].green(), 3)
            .arg(m_colorArray[i][j].blue(), 3);
        }
    }
#endif
}

int KColorCombo2::columnCount() const
{
    return m_columnCount;
}

int KColorCombo2::rowCount() const
{
    return m_rowCount;
}

QColor KColorCombo2::colorAt(int column, int row)/* const*/
{
    if (!m_colorArray)
        setRainbowPreset();

    if (column < 0 || row < 0 || column >= m_columnCount || row >= m_rowCount)
        return QColor();

    return m_colorArray[column][row];
}

QColor KColorCombo2::defaultColor() const
{
    return m_defaultColor;
}

void KColorCombo2::newColorArray(int columnCount, int rowCount)
{
    if (columnCount <= 0 || rowCount <= 0) {
        // kdebug << this::FUNCTION << "Trying to create an empty new color array (with %d columns and %d rows)";
        return;
    }

    // Delete any previous array (if any):
    deleteColorArray();

    // Create a new array of the wanted dimentions:
    m_columnCount = columnCount;
    m_rowCount    = rowCount;
    m_colorArray  = new QColor* [columnCount];
    for (int i = 0; i < columnCount; ++i)
        m_colorArray[i] = new QColor[rowCount];
}

void KColorCombo2::setColorAt(int column, int row, const QColor &color)
{
    if (!m_colorArray)
        setRainbowPreset();

    if (column < 0 || row < 0 || column >= m_columnCount || row >= m_rowCount) {
        // kdebug << this::FUNCTION << "Trying to set a color at an invalid index (at column %d and row %d, whereas the array have %d columns and %d rows)";
        return;
    }

    m_colorArray[column][row] = color;
}

void KColorCombo2::setDefaultColor(const QColor &color)
{
    m_defaultColor = color;
    if (!m_defaultColor.isValid() && !m_color.isValid())
        m_color = Qt::white; // FIXME: Use the first one.
}

QPixmap KColorCombo2::colorRectPixmap(const QColor &color, bool isDefault, int width, int height)
{
    // Prepare to draw:
    QPixmap  pixmap(width, height);
    QBitmap  mask(width, height);
    QPainter painter(&pixmap);
    QPainter maskPainter(&mask);

    // Draw pixmap:
    drawColorRect(painter, 0, 0, color, isDefault, width, height);

    // Draw mask (make the four corners transparent):
    maskPainter.fillRect(0, 0, width, height, Qt::color1); // opaque
    maskPainter.setPen(Qt::color0); // transparent
    maskPainter.drawPoint(0,         0);
    maskPainter.drawPoint(0,         height - 1);
    maskPainter.drawPoint(width - 1, height - 1);
    maskPainter.drawPoint(width - 1, 0);

    // Finish:
    painter.end();
    maskPainter.end();
    pixmap.setMask(mask);
    return pixmap;
}

void KColorCombo2::drawColorRect(QPainter &painter, int x, int y, const QColor &color, bool isDefault, int width, int height)
{
    // Fill:
    if (color.isValid())
        painter.fillRect(x /*+ 1*/, y /*+ 1*/, width /*- 2*/, height /*- 2*/, color);
    else {
        // If it's an invalid color, it's for the "Other..." entry: draw a rainbow.
        // If it wasn't for the "Other..." entry, the programmer made a fault, so (s)he will be informed about that visually.
        for (int i = 0; i < width - 2; ++i) {
            int hue = i * 360 / (width - 2);
            for (int j = 0; j < height - 2; ++j) {
                int saturation = 255 - (j * 255 / (height - 2));
                painter.setPen(QColor::fromHsv(hue, saturation, /*value=*/255));
                painter.drawPoint(x + i + 1, y + j + 1);
            }
        }
    }

    // Stroke:
    int dontCare, value;
    color.getHsv(/*hue:*/&dontCare, /*saturation:*/&dontCare, &value);
    QColor stroke = (color.isValid() ? color.dark(125) : palette().color(QPalette::Text));
    painter.setPen(/*color);//*/stroke);
    painter.drawLine(x + 1,         y,              x + width - 2, y);
    painter.drawLine(x,             y + 1,          x,             y + height - 2);
    painter.drawLine(x + 1,         y + height - 1, x + width - 2, y + height - 1);
    painter.drawLine(x + width - 1, y + 1,          x + width - 1, y + height - 2);

    // Round corners:
    QColor antialiasing;
    if (color.isValid()) {
        antialiasing = Tool_mixColors(color, stroke);
        painter.setPen(antialiasing);
        painter.drawPoint(x + 1,         y + 1);
        painter.drawPoint(x + 1,         y + height - 2);
        painter.drawPoint(x + width - 2, y + height - 2);
        painter.drawPoint(x + width - 2, y + 1);
    } else {
        // The two top corners:
        antialiasing = Tool_mixColors(Qt::red, stroke);
        painter.setPen(antialiasing);
        painter.drawPoint(x + 1,         y + 1);
        painter.drawPoint(x + width - 2, y + 1);
        // The two bottom ones:
        antialiasing = Tool_mixColors(Qt::white, stroke);
        painter.setPen(antialiasing);
        painter.drawPoint(x + 1,         y + height - 2);
        painter.drawPoint(x + width - 2, y + height - 2);
    }

    // Mark default color:
    if (isDefault) {
        painter.setPen(stroke);
        painter.drawLine(x + 1, y + height - 2, x + width - 2, y + 1);
    }
}

int KColorCombo2::colorRectHeight() const
{
    return (fontMetrics().boundingRect(i18n("(Default)")).height() + 2)*3 / 2;
}

int KColorCombo2::colorRectWidthForHeight(int height) const
{
    return height * 14 / 10; // 1.4 times the height, like A4 papers.
}

void KColorCombo2::deleteColorArray()
{
    if (m_colorArray) {
        for (int i = 0; i < m_columnCount; ++i)
            delete[] m_colorArray[i];
        delete[] m_colorArray;
        m_colorArray = 0;
    }
}

void KColorCombo2::updateComboBox()
{
    int height = colorRectHeight() * 2 / 3; // fontMetrics().boundingRect(i18n("(Default)")).height() + 2
    QPixmap pixmap = colorRectPixmap(effectiveColor(), !m_color.isValid(), height, height); // TODO: isDefaultColorSelected()
    setItemIcon(/*index=*/0, pixmap);
    setItemText(/*index=*/0,
                (m_color.isValid()
                 ? QString(i18n("R:%1, G:%2, B:%3")).arg(m_color.red()).arg(m_color.green()).arg(m_color.blue())
                 : i18nc("color", "(Default)")));
}

void KColorCombo2::showPopup()
{
    if (!m_colorArray)
        setRainbowPreset();

    // Compute where to show the popup:
    QRect desk = KGlobalSettings::desktopGeometry(this);

    QPoint popupPoint = mapToGlobal(QPoint(0, 0));

    int popupHeight = m_popup->size().height();
    if (popupPoint.y() + height() + popupHeight > desk.bottom())
        popupPoint.setY(popupPoint.y() - popupHeight);
    else
        popupPoint.setY(popupPoint.y() + height());

    int popupWidth = m_popup->size().width();
    if (popupPoint.x() + popupWidth > desk.right())
        popupPoint.setX(desk.right() - popupWidth);

    if (popupPoint.x() < desk.left())
        popupPoint.setX(desk.left());

    if (popupPoint.y() < desk.top())
        popupPoint.setY(desk.top());

    // Configure the popup:
    m_popup->move(popupPoint);
    //m_popup->setColor(m_color);
    m_popup->doSelection();
    m_popup->relayout(); // FIXME: In aboutToShow() ?
#if 0
//#ifndef QT_NO_EFFECTS
    if (QApplication::isEffectEnabled(UI_AnimateCombo)) {
        if (m_popup->y() < mapToGlobal(QPoint(0, 0)).y())
            qScrollEffect(m_popup, QEffects::UpScroll);
        else
            qScrollEffect(m_popup);
    } else
#endif
        m_popup->show();

    // The combo box is now shown pressed. Make it show not pressed again
    // by causing its (invisible) list box to emit a 'selected' signal.
    // Simulate an Enter to unpress it:
    /*QListWidget *lb = listBox();
    if (lb) {
        lb->setCurrentItem(0);
        QKeyEvent* keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Enter, 0, 0);
        QApplication::postEvent(lb, keyEvent);
    }*/
}

void KColorCombo2::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) &&
            (event->pos() - m_dragStartPos).manhattanLength() > KGlobalSettings::dndEventDelay()) {
        // Drag color object:
        QMimeData* mimeData = new QMimeData;
        QDrag* colorDrag = new QDrag(this);
        mimeData->setColorData(effectiveColor());
        // Replace the drag pixmap with our own rounded one, at the same position and dimetions:
        QPixmap pixmap = colorDrag->pixmap();
        pixmap = colorRectPixmap(effectiveColor(), /*isDefault=*/false, pixmap.width(), pixmap.height());
        colorDrag->setPixmap(pixmap);
        colorDrag->setHotSpot(colorDrag->hotSpot());
        colorDrag->exec(Qt::CopyAction, Qt::CopyAction);
        //setDown(false);
    }
}

void KColorCombo2::dragEnterEvent(QDragEnterEvent *event)
{
    if (isEnabled() && event->mimeData()->hasColor())
        event->accept();
}

void KColorCombo2::dropEvent(QDropEvent *event)
{
    QColor color;
    color = qvariant_cast<QColor>(event->mimeData()->colorData());
    if (color.isValid())
        setColor(color);
}

void KColorCombo2::keyPressEvent(QKeyEvent *event)
{
    QKeySequence key(event->key());

    if (KStandardShortcut::copy().contains(key)) {
        QMimeData *mime = new QMimeData;
        mime->setColorData(effectiveColor());
        QApplication::clipboard()->setMimeData(mime, QClipboard::Clipboard);
    } else if (KStandardShortcut::paste().contains(key)) {
        QColor color;
        color = qvariant_cast<QColor>(QApplication::clipboard()->mimeData(QClipboard::Clipboard)->colorData());
        setColor(color);
    } else
        QComboBox::keyPressEvent(event);
}

void KColorCombo2::fontChange(const QFont &oldFont)
{
    // Since the color-rectangle is the same height of the text, we should resize it if the font change:
    updateComboBox();
    QComboBox::fontChange(oldFont); // To update geometry.
}

void KColorCombo2::virtual_hook(int /*id*/, void */*data*/)
{
    /* KBASE::virtual_hook(id, data); */
}

void KColorCombo2::popupClosed()
{
    hidePopup();
}


#endif // USE_OLD_KCOLORCOMBO

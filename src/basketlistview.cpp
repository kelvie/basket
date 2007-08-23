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

#include "basketlistview.h"
#include <qregexp.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstringhandler.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qpixmapcache.h>
#include <qtooltip.h>
#include <iostream>
#include <kdebug.h>
#include "global.h"
#include "bnpview.h"
#include "basket.h"
#include "tools.h"
#include "settings.h"
#include "notedrag.h"
#include <kcolorscheme.h>
#include <QDropEvent>

/** class BasketListViewItem: */

BasketListViewItem::BasketListViewItem(QListWidget *parent, Basket *basket)
	: QListWidgetItem(parent), m_basket(basket)
	, m_isUnderDrag(false)
	, m_isAbbreviated(false)
{
// TO REMOVE	setAcceptDrops(true);
}

/* TO REMOVE
BasketListViewItem::BasketListViewItem(QListWidgetItem *parent, Basket *basket)
	: QListView(parent), m_basket(basket)
	, m_isUnderDrag(false)
	, m_isAbbreviated(false)
{
	setDropEnabled(true);
}
*/

BasketListViewItem::BasketListViewItem(QListWidget *parent, QListWidgetItem *after, Basket *basket)
	: QListWidgetItem(parent, parent->row(after)), m_basket(basket)
	, m_isUnderDrag(false)
	, m_isAbbreviated(false)
{
//	setDropEnabled(true);
}

/* TO REMOVE 
BasketListViewItem::BasketListViewItem(QListWidgetItem *parent, QListWidget *after, Basket *basket)
	: QListView(parent, after), m_basket(basket)
	, m_isUnderDrag(false)
	, m_isAbbreviated(false)
{
	setDropEnabled(true);
}
*/

BasketListViewItem::~BasketListViewItem()
{
}

bool BasketListViewItem::acceptDrop(const QMimeData *) const
{
	std::cout << "accept" << std::endl;
	return true;
}

void BasketListViewItem::dropped(QDropEvent *event)
{
	qDebug()<< "Dropping into basket " << m_basket->objectName();
	m_basket->contentsDropEvent(event);
	//Global::bnpView->currentBasket()->contentsDropEvent(event); // FIXME
}

int BasketListViewItem::width(const QFontMetrics &/* fontMetrics */, const QListWidgetItem */*listView*/, int /* column */) const
{
	return listWidget ()->width() + 100;
/*
	int BASKET_ICON_SIZE = 16;
	int MARGIN = 1;

	QRect textRect = fontMetrics.boundingRect(0, 0, / *width=* /1, 500000, Qt::AlignLeft | Qt::AlignTop | Qt::ShowPrefix, text(column));

	return MARGIN + BASKET_ICON_SIZE + MARGIN + textRect.width() +   BASKET_ICON_SIZE/2   + MARGIN;
*/
}

QString BasketListViewItem::escapedName(const QString &string)
{
	// Underlining the Alt+Letter shortcut (and escape all other '&' characters), if any:
	QString basketName = string;
	basketName.replace('&', "&&"); // First escape all the amperstamp
	QString letter; // Find the letter
	QString altKey   = /*i18n(*/"Alt"/*)*/;   //i18n("The [Alt] key, as shown in shortcuts like Alt+C...", "Alt");
	QString shiftKey = /*i18n(*/"Shift"/*)*/; //i18n("The [Shift] key, as shown in shortcuts like Alt+Shift+1...", "Shift");
	QRegExp altLetterExp(      QString("^%1\\+(.)$").arg(altKey)                );
	QRegExp altShiftLetterExp( QString("^%1\\+%2\\+(.)$").arg(altKey, shiftKey) );
	if (altLetterExp.indexIn(m_basket->shortcut().toString()) != -1)
		letter = altLetterExp.cap(1);
	if (letter.isEmpty() && altShiftLetterExp.indexIn(m_basket->shortcut().toString()) != -1)
		letter = altShiftLetterExp.cap(1);
	if (!letter.isEmpty()) {
		int index = basketName.count(letter, Qt::CaseInsensitive);
		if (index != -1)
			basketName.insert(index, '&');
	}
	return basketName;
}

void BasketListViewItem::setup()
{
	int BASKET_ICON_SIZE = 16;
	int MARGIN = 1;

	setText( escapedName(m_basket->basketName()));

	widthChanged();
	QRect textRect = listWidget()->fontMetrics().boundingRect(0, 0, /*width=*/1, 500000, Qt::AlignLeft | Qt::AlignTop | Qt::TextShowMnemonic, text());

	int height = MARGIN + qMax(BASKET_ICON_SIZE, textRect.height()) + MARGIN;
	setHeight(height);

	setIcon(KIcon(m_basket->icon()));
}

BasketListViewItem* BasketListViewItem::lastChild()
{
	QListWidgetItem *child = firstChild();
	while (child) {
		if (child->nextSibling())
			child = child->nextSibling();
		else
			return (BasketListViewItem*)child;
	}
	return 0;
}

BasketListViewItem* BasketListViewItem::prevSibling()
{
	BasketListViewItem *item = this;
	while (item) {
		if (item->nextSibling() == this)
			return item;
		item = (BasketListViewItem*)(item->itemAbove());
	}
	return 0;
}

BasketListViewItem* BasketListViewItem::shownItemAbove()
{
	BasketListViewItem *item = (BasketListViewItem*)itemAbove();
	while (item) {
		if (item->isShown())
			return item;
		item = (BasketListViewItem*)(item->itemAbove());
	}
	return 0;
}

BasketListViewItem* BasketListViewItem::shownItemBelow()
{
	BasketListViewItem *item = (BasketListViewItem*)itemBelow();
	while (item) {
		if (item->isShown())
			return item;
		item = (BasketListViewItem*)(item->itemBelow());
	}
	return 0;
}

QStringList BasketListViewItem::childNamesTree(int deep)
{
	QStringList result;
	for (QListWidgetItem *child = firstChild(); child; child = child->nextSibling()) {
		BasketListViewItem *item = (BasketListViewItem*)child;
		// Compute indentation spaces:
		QString spaces;
		for (int i = 0; i < deep; ++i)
			spaces += "  ";
		// Append the name:
		result.append(spaces + item->basket()->basketName());
		// Append the childs:
		if (child->firstChild()) {
			QStringList childs = item->childNamesTree(deep + 1);
			for (QStringList::iterator it = childs.begin(); it != childs.end(); ++it)
				result.append(*it);
		}
	}
	return result;
}

void BasketListViewItem::moveChildsBaskets()
{
	QListWidgetItem *insertAfterThis = this;
	QListWidgetItem *nextOne;
	for (QListWidgetItem *child = firstChild(); child; child = nextOne) {
		nextOne = child->nextSibling();
		// Re-insert the item with the good parent:
		takeItem(child);
		if (parent())
			parent()->insertItem(child);
		else
			listView()->insertItem(child);
		// And move it at the good place:
		child->moveItem(insertAfterThis);
		insertAfterThis = child;
	}
}

void BasketListViewItem::ensureVisible()
{
	BasketListViewItem *item = this;
	while (item->parent()) {
		item = (BasketListViewItem*)(item->parent());
		item->setOpen(true);
	}
}

bool BasketListViewItem::isShown()
{
	QListWidgetItem *item = parent();
	while (item) {
		if (!item->isOpen())
			return false;
		item = item->parent();
	}
	return true;
}

bool BasketListViewItem::isCurrentBasket()
{
	return basket() == Global::bnpView->currentBasket();
}

// TODO: Move this function from item.cpp to class Tools:
extern void drawGradient( QPainter *p, const QColor &colorTop, const QColor & colorBottom,
						  int x, int y, int w, int h,
						  bool sunken, bool horz, bool flat  ); /*const*/

QPixmap BasketListViewItem::circledTextPixmap(const QString &text, int height, const QFont &font, const QColor &color)
{
	QString key = QString("BLI-%1.%2.%3.%4")
		.arg(text).arg(height).arg(font.toString()).arg(color.rgb());
	if (QPixmap* cached=QPixmapCache::find(key)) {
		return *cached;
	}

	// Compute the sizes of the image components:
	QRect textRect = QFontMetrics(font).boundingRect(0, 0, /*width=*/1, height, Qt::AlignLeft | Qt::AlignTop, text);
	int xMargin = height / 6;
	int width   = xMargin + textRect.width() + xMargin;

	// Create the gradient image:
	QPixmap gradient(3 * width, 3 * height); // We double the size to be able to smooth scale down it (== antialiased curves)
	QPainter gradientPainter(&gradient);
#if 1 // Enable the new look of the gradient:
	QColor topColor       = KColorScheme(KColorScheme::Selection).background().color().lighter(130); //120
	QColor topMidColor    = KColorScheme(KColorScheme::Selection).background().color().lighter(105); //105
	QColor bottomMidColor = KColorScheme(KColorScheme::Selection).background().color().darker(130);  //120
	QColor bottomColor    = KColorScheme(KColorScheme::Selection).background().color();
	drawGradient(&gradientPainter, topColor, topMidColor,
				  0, 0, gradient.width(), gradient.height() / 2, /*sunken=*/false, /*horz=*/true, /*flat=*/false);
	drawGradient(&gradientPainter, bottomMidColor, bottomColor,
				  0, gradient.height() / 2, gradient.width(), gradient.height() - gradient.height() / 2, /*sunken=*/false, /*horz=*/true, /*flat=*/false);
	gradientPainter.fillRect(0, 0, gradient.width(), 3, KColorScheme(KColorScheme::Selection).background().color());
#else
	drawGradient(&gradientPainter, KColorScheme(KColorScheme::Selection).background().color(), KColorScheme(KColorScheme::Selection).background().color().darker(),
				  0, 0, gradient.width(), gradient.height(), /*sunken=*/false, /*horz=*/true, /*flat=*/false);
#endif
	gradientPainter.end();

	// Draw the curved rectangle:
	QBitmap curvedRectangle(3 * width, 3 * height);
	curvedRectangle.fill(Qt::color0);
	QPainter curvePainter(&curvedRectangle);
	curvePainter.setPen(Qt::color1);
	curvePainter.setBrush(Qt::color1);
	curvePainter.setClipRect(0, 0, 3*(height / 5), 3*(height)); // If the width is small, don't fill the right part of the pixmap
	curvePainter.drawEllipse(0, 3*(-height / 4), 3*(height), 3*(height * 3 / 2)); // Don't forget we double the sizes
	curvePainter.setClipRect(3*(width - height / 5), 0, 3*(height / 5), 3*(height));
	curvePainter.drawEllipse(3*(width - height), 3*(-height / 4), 3*(height), 3*(height * 3 / 2));
	curvePainter.setClipping(false);
	curvePainter.fillRect(3*(height / 6), 0, 3*(width - 2 * height / 6), 3*(height), curvePainter.brush());
	curvePainter.end();

	// Apply the curved rectangle as the mask of the gradient:
	gradient.setMask(curvedRectangle);
	QImage resultImage = gradient.toImage();
	resultImage.setAlphaBuffer(true);

	// Scale down the image smoothly to get anti-aliasing:
	QPixmap pmScaled;
	pmScaled.fromImage(resultImage.smoothScale(width, height));

	// Draw the text, and return the result:
	QPainter painter(&pmScaled);
	painter.setPen(color);
	painter.setFont(font);
	painter.drawText(0+1, 0, width, height, Qt::AlignHCenter | Qt::AlignVCenter, text);
	painter.end();

	QPixmapCache::insert(key, pmScaled);

	return pmScaled;
}

QPixmap BasketListViewItem::foundCountPixmap(bool isLoading, int countFound, bool childsAreLoading, int countChildsFound, const QFont &font, int height)
{
	if (isLoading)
		return QPixmap();

	QFont boldFont(font);
	boldFont.setBold(true);

	QString text;
	if (childsAreLoading) {
		if (countChildsFound > 0)
			text = i18n("%1+%2+").arg(QString::number(countFound), QString::number(countChildsFound));
		else
			text = i18n("%1+").arg(QString::number(countFound));
	} else {
		if (countChildsFound > 0)
			text = i18n("%1+%2").arg(QString::number(countFound), QString::number(countChildsFound));
		else if (countFound > 0)
			text = QString::number(countFound);
		else
			return QPixmap();
	}

	return circledTextPixmap(text, height, boldFont, KColorScheme(KColorScheme::Selection).foreground().color());
}

bool BasketListViewItem::haveChildsLoading()
{
	QListWidgetItem *child = firstChild();
	while (child) {
		BasketListViewItem *childItem = (BasketListViewItem*)child;
		if (!childItem->basket()->isLoaded() && !childItem->basket()->isLocked())
			return true;
		if (childItem->haveChildsLoading())
			return true;
		child = child->nextSibling();
	}
	return false;
}

bool BasketListViewItem::haveHiddenChildsLoading()
{
	if (isOpen())
		return false;
	return haveChildsLoading();
}

bool BasketListViewItem::haveChildsLocked()
{
	QListWidgetItem *child = firstChild();
	while (child) {
		BasketListViewItem *childItem = (BasketListViewItem*)child;
		if (/*!*/childItem->basket()->isLocked())
			return true;
		if (childItem->haveChildsLocked())
			return true;
		child = child->nextSibling();
	}
	return false;
}

bool BasketListViewItem::haveHiddenChildsLocked()
{
	if (isOpen())
		return false;
	return haveChildsLocked();
}

int BasketListViewItem::countChildsFound()
{
	int count = 0;
	QListWidgetItem *child = firstChild();
	while (child) {
		BasketListViewItem *childItem = (BasketListViewItem*)child;
		count += childItem->basket()->countFounds();
		count += childItem->countChildsFound();
		child = child->nextSibling();
	}
	return count;
}

int BasketListViewItem::countHiddenChildsFound()
{
	if (isOpen())
		return 0;
	return countChildsFound();
}

void BasketListViewItem::paintCell(QPainter *painter, const QPalette &/*colorGroup*/, int /*column*/, int width, int /*align*/)
{
	// Workaround a Qt bug:
	// When the splitter is moved to hide the tree view and then the application is restarted,
	// Qt try to draw items with a negative size!
	if (width <= 0) {
		std::cout << "width <= 0" << std::endl;
		return;
	}

	int BASKET_ICON_SIZE = 16;
	int MARGIN = 1;


	// If we are filtering all baskets, and are effectively filtering on something:
	bool showLoadingIcon = false;
	bool showEncryptedIcon = false;
	QPixmap countPixmap;
	bool showCountPixmap = Global::bnpView->isFilteringAllBaskets() &&
		Global::bnpView->currentBasket()->decoration()->filterBar()->filterData().isFiltering;
	if (showCountPixmap) {
		showLoadingIcon = (!m_basket->isLoaded() && !m_basket->isLocked()) || haveHiddenChildsLoading();
		showEncryptedIcon = m_basket->isLocked() || haveHiddenChildsLocked();
		countPixmap = foundCountPixmap(!m_basket->isLoaded(), m_basket->countFounds(), haveHiddenChildsLoading() || haveHiddenChildsLocked(),
										countHiddenChildsFound(), listView()->font(), height() - 2 * MARGIN);
	}
	int effectiveWidth = width - (countPixmap.isNull() ? 0 : countPixmap.width() + MARGIN)
			- (showLoadingIcon || showEncryptedIcon ? BASKET_ICON_SIZE + MARGIN : 0)/*
			- (showEncryptedIcon ? BASKET_ICON_SIZE + MARGIN : 0)*/;


	bool drawRoundRect = m_basket->backgroundColorSetting().isValid() || m_basket->textColorSetting().isValid();
	QColor textColor = (drawRoundRect ? m_basket->textColor() : (isCurrentBasket() ? KColorScheme(KColorScheme::Selection).foreground().color() : KColorScheme(KColorScheme::View).foreground().color()));

	BasketListViewItem *shownAbove = shownItemAbove();
	BasketListViewItem *shownBelow = shownItemBelow();

	// Don't forget to update the key computation if parameters
	// affecting the rendering logic change
	QString key = QString("BLVI::pC-%1.%2.%3.%4.%5.%6.%7.%8.%9.%10.%11.%12.%13.%14.%15")
		.arg(effectiveWidth)
		.arg(drawRoundRect)
		.arg(textColor.rgb())
		.arg(m_basket->backgroundColor().rgb())
		.arg(isCurrentBasket())
		.arg(shownBelow && shownBelow->isCurrentBasket())
		.arg(shownAbove && shownAbove->isCurrentBasket())
		.arg(showLoadingIcon)
		.arg(showEncryptedIcon)
		.arg(showCountPixmap)
		.arg(m_basket->countFounds())
		.arg(countHiddenChildsFound())
		.arg(m_isUnderDrag)
		.arg(m_basket->basketName())
		.arg(m_basket->icon());
	if (QPixmap* cached = QPixmapCache::find(key)) {
		// Qt's documentation recommends copying the pointer
		// into a QPixmap immediately
		QPixmap cachedBuffer = *cached;
		painter->drawPixmap(0, 0, cachedBuffer);
		return;
	}

	// Bufferize the drawing of items (otherwize, resizing the splitter make the tree act like a Christmas Tree ;-D ):
	QPixmap theBuffer(width, height());
	QPainter thePainter(&theBuffer);

	// Fill with the basket background color:
	QColor background = (isCurrentBasket() ? KColorScheme(KColorScheme::Selection).background().color() : listView()->paletteBackgroundColor());
	thePainter.fillRect(0, 0, width, height(), background);

	int textWidth = effectiveWidth - MARGIN - BASKET_ICON_SIZE - MARGIN - MARGIN;

	// Draw the rounded rectangle:
	if (drawRoundRect) {
		QRect textRect = listView()->fontMetrics().boundingRect(0, 0, /*width=*/1, 500000, Qt::AlignLeft | Qt::AlignTop | Qt::ShowPrefix, text(/*column=*/0));
		int xRound = MARGIN;
		int yRound = MARGIN;
		int hRound = height() - 2 * MARGIN;
		int wRound = qMin(BASKET_ICON_SIZE + MARGIN + textRect.width() + hRound/2,  effectiveWidth - MARGIN - MARGIN);
		if (wRound > 0) { // Do not crash if there is no space anymore to draw the rounded rectangle:
			QPixmap buffer(wRound * 2, hRound * 2);
			buffer.fill(background);
			QPainter pBuffer(&buffer);
			QColor colorRound = m_basket->backgroundColor();
			pBuffer.setPen(colorRound);
			pBuffer.setBrush(colorRound);
			if (wRound > hRound) { // If the rectangle is smaller in width than in height, don't overlap ellipses...
				pBuffer.drawEllipse(0,                                 0, hRound * 2, hRound * 2);
				pBuffer.drawEllipse(wRound * 2 - hRound * 2, 0, hRound * 2, hRound * 2);
				pBuffer.fillRect(hRound*2/2, 0, wRound * 2 - hRound * 2, hRound * 2, colorRound);
			} else
				pBuffer.drawEllipse(0, 0, wRound * 2, hRound * 2);
				pBuffer.end();
				QImage imageToScale = buffer.toImage();
				QPixmap pmScaled;
				pmScaled.fromImage(imageToScale.smoothScale(wRound, hRound));
				thePainter.drawPixmap(xRound, yRound, pmScaled);
				textWidth -= hRound/2;
		}
	}

	QColor bgColor  = listView()->paletteBackgroundColor();
	QColor selColor = KColorScheme(KColorScheme::Selection).background().color();
	QColor midColor = Tools::mixColor(bgColor, selColor);
	// Draw the left selection roundings:
	if (isCurrentBasket()) {
		thePainter.setPen(bgColor);
		thePainter.drawPoint(0, 0);
		thePainter.drawPoint(1, 0);
		thePainter.drawPoint(0, 1);
		thePainter.drawPoint(0, height() - 1);
		thePainter.drawPoint(1, height() - 1);
		thePainter.drawPoint(0, height() - 2);
		thePainter.setPen(midColor);
		thePainter.drawPoint(2, 0);
		thePainter.drawPoint(0, 2);
		thePainter.drawPoint(2, height() - 1);
		thePainter.drawPoint(0, height() - 3);
	}
	// Draw the bottom-right selection roundings:
	//BasketListViewItem *shownBelow = shownItemBelow();
	if (shownBelow && shownBelow->isCurrentBasket()) {
		thePainter.setPen(selColor);
		thePainter.drawPoint(width - 1, height() - 1);
		thePainter.drawPoint(width - 2, height() - 1);
		thePainter.drawPoint(width - 1, height() - 2);
		thePainter.setPen(midColor);
		thePainter.drawPoint(width - 3, height() - 1);
		thePainter.drawPoint(width - 1, height() - 3);
	}
	// Draw the top-right selection roundings:
	//	BasketListViewItem *shownAbove = shownItemAbove();
	if (shownAbove && shownAbove->isCurrentBasket()) {
		thePainter.setPen(selColor);
		thePainter.drawPoint(width - 1, 0);
		thePainter.drawPoint(width - 2, 0);
		thePainter.drawPoint(width - 1, 1);
		thePainter.setPen(midColor);
		thePainter.drawPoint(width - 3, 0);
		thePainter.drawPoint(width - 1, 2);
	}

	// Draw the icon and text:
	int yPixmap = (height() - BASKET_ICON_SIZE) / 2;
	thePainter.drawPixmap(MARGIN, yPixmap, *pixmap(/*column=*/0));
	thePainter.setPen(textColor);
	if (textWidth > 0) { // IF there is space left to draw the text:
		int xText = MARGIN + BASKET_ICON_SIZE + MARGIN;
		QString theText = m_basket->basketName();
		if (painter->fontMetrics().width(theText) > textWidth) {
			theText = KStringHandler::rPixelSqueeze(theText, painter->fontMetrics(), textWidth);
			m_isAbbreviated = true;
		}
		else {
			m_isAbbreviated = false;
		}
		theText = escapedName(theText);
		thePainter.drawText(xText, 0, textWidth, height(), Qt::AlignLeft | Qt::AlignVCenter | Qt::ShowPrefix, theText);
	}

	// If we are filtering all baskets, and are effectively filtering on something:
	if (!countPixmap.isNull())
	{
		thePainter.drawPixmap(effectiveWidth, 1, countPixmap);
		effectiveWidth += countPixmap.width() + MARGIN;
	}
	if (showLoadingIcon) {
		QPixmap icon = KIconLoader::global()->loadIcon("edit-find", K3Icon::NoGroup, 16, KIcon::DefaultState, 0L, /*canReturnNull=*/false);
		thePainter.drawPixmap(effectiveWidth, 0, icon);
		effectiveWidth += BASKET_ICON_SIZE + MARGIN;
	}
	if (showEncryptedIcon && !showLoadingIcon) {
		QPixmap icon = KIconLoader::global()->loadIcon("encrypted", K3Icon::NoGroup, 16, KIcon::DefaultState, 0L, /*canReturnNull=*/false);
		thePainter.drawPixmap(effectiveWidth, 0, icon);
	}

	if (m_isUnderDrag) {
		thePainter.drawWinFocusRect(0, 0, width, height());
	}
	thePainter.end();

	QPixmapCache::insert(key, theBuffer);
	// Apply the buffer:
	painter->drawPixmap(0, 0, theBuffer);
}

void BasketListViewItem::setUnderDrag(bool underDrag)
{
	m_isUnderDrag = underDrag;
}

bool BasketListViewItem::isAbbreviated()
{
	return m_isAbbreviated;
}

/** class BasketListViewToolTip: */

class BasketTreeListView_ToolTip : public QToolTip {
public:
	BasketTreeListView_ToolTip(BasketTreeListView* basketView)
		: QToolTip(basketView->viewport())
		, m_basketView(basketView)
	{}
public:
	void maybeTip(const QPoint& pos)
	{
		QListWidgetItem *item = m_basketView->itemAt(m_basketView->contentsToViewport(pos));
		BasketListViewItem* bitem = dynamic_cast<BasketListViewItem*>(item);
		if (bitem && bitem->isAbbreviated()) {
			tip(m_basketView->itemRect(bitem), bitem->basket()->basketName());
		}
	}
private:
	BasketTreeListView* m_basketView;
};

/** class BasketTreeListView: */

BasketTreeListView::BasketTreeListView(QWidget *parent, const char *name)
	:  KListWidget(parent), m_autoOpenItem(0)
	, m_itemUnderDrag(0)
{
	setObjectName(name);
	setWFlags(Qt::WStaticContents | WNoAutoErase);
	clearWFlags(Qt::WStaticContents | WNoAutoErase);
	//viewport()->clearWFlags(Qt::WStaticContents);
	connect( &m_autoOpenTimer, SIGNAL(timeout()), this, SLOT(autoOpen()) );

	new BasketTreeListView_ToolTip(this);
}

void BasketTreeListView::resizeEvent(QResizeEvent *event)
{
	KListWidget::resizeEvent(event);
	triggerUpdate();
}

void BasketTreeListView::contentsDragEnterEvent(QDragEnterEvent *event)
{
	if (event->provides("application/x-qlistviewitem")) {
		QListWidgetIterator it(this); // TODO: Don't show expanders if it's not a basket drag...
		while (it.current()) {
			QListWidgetItem *item = it.current();
			if (!item->firstChild()) {
				item->setExpandable(true);
				item->setOpen(true);
			}
			++it;
		}
		update();
	}

	KListWidget::dragEnterEvent(event);
}

void BasketTreeListView::removeExpands()
{
// TODO
/*	QListViewIterator it(this);
	while (it.current()) {
		QListWidgetItem *item = it.current();
		if (!item->firstChild())
			item->setExpandable(false);
		++it;
	}
	viewport()->update();
*/
}

void BasketTreeListView::contentsDragLeaveEvent(QDragLeaveEvent *event)
{
	std::cout << "BasketTreeListView::contentsDragLeaveEvent" << std::endl;
	m_autoOpenItem = 0;
	m_autoOpenTimer.stop();
	setItemUnderDrag(0);
	removeExpands();
	KListWidget::dragLeaveEvent(event);
}

void BasketTreeListView::contentsDropEvent(QDropEvent *event)
{
	std::cout << "BasketTreeListView::contentsDropEvent()" << std::endl;
	if (event->provides("application/x-qlistviewitem"))
	{
		KListWidget::dropEvent(event);
	}
	else {
		std::cout << "Forwarding dropped data to the basket" << std::endl;
		QListWidgetItem *item = itemAt(contentsToViewport(event->pos()));
		BasketListViewItem* bitem = dynamic_cast<BasketListViewItem*>(item);
		if (bitem) {
			bitem->basket()->blindDrop(event);
		}
		else {
			std::cout << "Forwarding failed: no bitem found" << std::endl;
		}
	}

	m_autoOpenItem = 0;
	m_autoOpenTimer.stop();
	setItemUnderDrag(0);
	removeExpands();

	Global::bnpView->save(); // TODO: Don't save if it was not a basket drop...
}

void BasketTreeListView::contentsDragMoveEvent(QDragMoveEvent *event)
{
	std::cout << "BasketTreeListView::contentsDragMoveEvent" << std::endl;
	if (event->provides("application/x-qlistviewitem"))
		KListWidget::dragMoveEvent(event);
	else {
		QListWidgetItem *item = itemAt(contentsToViewport(event->pos()));
		BasketListViewItem* bitem = dynamic_cast<BasketListViewItem*>(item);
		if (m_autoOpenItem != item) {
			m_autoOpenItem = item;
			m_autoOpenTimer.setSingleShot(true);
			m_autoOpenTimer.start(1700);
		}
		if (item) {
			event->acceptAction(true);
			event->accept(true);
		}
		setItemUnderDrag(bitem);

		KListWidget::dragMoveEvent(event); // FIXME: ADDED
	}
}

void BasketTreeListView::setItemUnderDrag(BasketListViewItem* item)
{
	if (m_itemUnderDrag != item) {
		if (m_itemUnderDrag) {
			// Remove drag status from the old item
			m_itemUnderDrag->setUnderDrag(false);
// TO REMOVE			repaintItem(m_itemUnderDrag);
		}

		m_itemUnderDrag = item;

		if (m_itemUnderDrag) {
			// add drag status to the new item
			m_itemUnderDrag->setUnderDrag(true);
// TO REMOVE			repaintItem(m_itemUnderDrag);
		}
	}
}

void BasketTreeListView::autoOpen()
{
	BasketListViewItem *item = (BasketListViewItem*)m_autoOpenItem;
	if (item)
		Global::bnpView->setCurrentBasket(item->basket());
}

void BasketTreeListView::resizeEvent(QResizeEvent *event)
{
	KListWidget::resizeEvent(event);
}

void BasketTreeListView::paintEmptyArea(QPainter *painter, const QRect &rect)
{
	QListWidgetItem::paintEmptyArea(painter, rect);

	BasketListViewItem *last = Global::bnpView->lastListViewItem();
	if (last && !last->isShown())
		last = last->shownItemAbove();
	if (last && last->isCurrentBasket()) {
		int y = last->itemPos() + last->height();
		QColor bgColor  = KColorScheme(KColorScheme::View).background().color();
		QColor selColor = KColorScheme(KColorScheme::Selection).background().color();
		QColor midColor = Tools::mixColor(bgColor, selColor);
		painter->setPen(selColor);
		painter->drawPoint(width() - 1, y);
		painter->drawPoint(width() - 2, y);
		painter->drawPoint(width() - 1, y + 1);
		painter->setPen(midColor);
		painter->drawPoint(width() - 3, y);
		painter->drawPoint(width() - 1, y + 2);
	}
}

/** We should NEVER get focus (because of QWidget::NoFocus focusPolicy())
 * but K3ListView can programatically give us the focus.
 * So we give it to the basket.
 */
void BasketTreeListView::focusInEvent(QFocusEvent*)
{
	//K3ListView::focusInEvent(event);
	Basket *basket = Global::bnpView->currentBasket();
	if (basket)
		basket->setFocus();
}

#include "basketlistview.moc"

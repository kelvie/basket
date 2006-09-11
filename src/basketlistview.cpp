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
#include <iostream>
#include <kdebug.h>
#include "global.h"
#include "bnpview.h"
#include "basket.h"
#include "tools.h"
#include "settings.h"

/** class BasketListViewItem: */

BasketListViewItem::BasketListViewItem(QListView *parent, Basket *basket)
	: QListViewItem(parent), m_basket(basket)
{
	setDropEnabled(true);
}

BasketListViewItem::BasketListViewItem(QListViewItem *parent, Basket *basket)
	: QListViewItem(parent), m_basket(basket)
{
	setDropEnabled(true);
}

BasketListViewItem::BasketListViewItem(QListView *parent, QListViewItem *after, Basket *basket)
	: QListViewItem(parent, after), m_basket(basket)
{
	setDropEnabled(true);
}

BasketListViewItem::BasketListViewItem(QListViewItem *parent, QListViewItem *after, Basket *basket)
	: QListViewItem(parent, after), m_basket(basket)
{
	setDropEnabled(true);
}

BasketListViewItem::~BasketListViewItem()
{
}

bool BasketListViewItem::acceptDrop(const QMimeSource *) const
{
	std::cout << "accept" << std::endl;
	return true;
}

void BasketListViewItem::dropped(QDropEvent *event)
{
	std::cout << "drop" << std::endl;
	Global::bnpView->currentBasket()->contentsDropEvent(event); // FIXME
}

int BasketListViewItem::width(const QFontMetrics &/* fontMetrics */, const QListView */*listView*/, int /* column */) const
{
	return listView()->visibleWidth() + 100;
/*
	int BASKET_ICON_SIZE = 16;
	int MARGIN = 1;

	QRect textRect = fontMetrics.boundingRect(0, 0, / *width=* /1, 500000, Qt::AlignAuto | Qt::AlignTop | Qt::ShowPrefix, text(column));

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
	if (altLetterExp.search(m_basket->shortcut().toStringInternal()) != -1)
		letter = altLetterExp.cap(1);
	if (letter.isEmpty() && altShiftLetterExp.search(m_basket->shortcut().toStringInternal()) != -1)
		letter = altShiftLetterExp.cap(1);
	if (!letter.isEmpty()) {
		int index = basketName.find(letter, /*index=*/0, /*caseSensitive=*/false);
		if (index != -1)
			basketName.insert(index, '&');
	}
	return basketName;
}

void BasketListViewItem::setup()
{
	int BASKET_ICON_SIZE = 16;
	int MARGIN = 1;

	setText(/*column=*/0, escapedName(m_basket->basketName()));

	widthChanged();
	QRect textRect = listView()->fontMetrics().boundingRect(0, 0, /*width=*/1, 500000, Qt::AlignAuto | Qt::AlignTop | Qt::ShowPrefix, text(/*column=*/0));

	int height = MARGIN + QMAX(BASKET_ICON_SIZE, textRect.height()) + MARGIN;
	setHeight(height);

	QPixmap icon = kapp->iconLoader()->loadIcon(m_basket->icon(), KIcon::NoGroup, 16, KIcon::DefaultState, 0L, /*canReturnNull=*/false);

	setPixmap(/*column=*/0, icon);

	repaint();
}

BasketListViewItem* BasketListViewItem::lastChild()
{
	QListViewItem *child = firstChild();
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
	for (QListViewItem *child = firstChild(); child; child = child->nextSibling()) {
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
	QListViewItem *insertAfterThis = this;
	QListViewItem *nextOne;
	for (QListViewItem *child = firstChild(); child; child = nextOne) {
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
	QListViewItem *item = parent();
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
	QString key = QString("BLI-%1.%2.%3")
		.arg(text).arg(font.toString()).arg(color.rgb());
	if (QPixmap* cached=QPixmapCache::find(key)) {
		return *cached;
	}

	// Compute the sizes of the image components:
	QRect textRect = QFontMetrics(font).boundingRect(0, 0, /*width=*/1, height, Qt::AlignAuto | Qt::AlignTop, text);
	int xMargin = height / 6;
	int width   = xMargin + textRect.width() + xMargin;

	// Create the gradient image:
	QPixmap gradient(3 * width, 3 * height); // We double the size to be able to smooth scale down it (== antialiased curves)
	QPainter gradientPainter(&gradient);
#if 1 // Enable the new look of the gradient:
	QColor topColor       = KGlobalSettings::highlightColor().light(130); //120
	QColor topMidColor    = KGlobalSettings::highlightColor().light(105); //105
	QColor bottomMidColor = KGlobalSettings::highlightColor().dark(130);  //120
	QColor bottomColor    = KGlobalSettings::highlightColor();
	drawGradient(&gradientPainter, topColor, topMidColor,
				  0, 0, gradient.width(), gradient.height() / 2, /*sunken=*/false, /*horz=*/true, /*flat=*/false);
	drawGradient(&gradientPainter, bottomMidColor, bottomColor,
				  0, gradient.height() / 2, gradient.width(), gradient.height() - gradient.height() / 2, /*sunken=*/false, /*horz=*/true, /*flat=*/false);
	gradientPainter.fillRect(0, 0, gradient.width(), 3, KGlobalSettings::highlightColor());
#else
	drawGradient(&gradientPainter, KGlobalSettings::highlightColor(), KGlobalSettings::highlightColor().dark(),
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
	QImage resultImage = gradient.convertToImage();
	resultImage.setAlphaBuffer(true);

	// Scale down the image smoothly to get anti-aliasing:
	QPixmap pmScaled;
	pmScaled.convertFromImage(resultImage.smoothScale(width, height));

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

	return circledTextPixmap(text, height, boldFont, KGlobalSettings::highlightedTextColor());
}

bool BasketListViewItem::haveChildsLoading()
{
	QListViewItem *child = firstChild();
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
	QListViewItem *child = firstChild();
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
	QListViewItem *child = firstChild();
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

void BasketListViewItem::paintCell(QPainter *painter, const QColorGroup &/*colorGroup*/, int /*column*/, int width, int /*align*/)
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
	QColor textColor = (drawRoundRect ? m_basket->textColor() : (isCurrentBasket() ? KGlobalSettings::highlightedTextColor() : KGlobalSettings::textColor()));

	BasketListViewItem *shownAbove = shownItemAbove();
	BasketListViewItem *shownBelow = shownItemBelow();

	// Don't forget to update the key computation if parameters
	// affecting the rendering logic change
	QString key = QString("BLVI::pC-%1.%2.%3.%4.%5.%6.%7.%8.%9.%10")
		.arg(effectiveWidth)
		.arg(drawRoundRect)
		.arg(textColor.rgb())
		.arg(isCurrentBasket())
		.arg(shownBelow && shownBelow->isCurrentBasket())
		.arg(shownAbove && shownAbove->isCurrentBasket())
		.arg(showLoadingIcon)
		.arg(showEncryptedIcon)
		.arg(showCountPixmap)
		.arg(m_basket->basketName());
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
	QColor background = (isCurrentBasket() ? KGlobalSettings::highlightColor() : listView()->paletteBackgroundColor());
	thePainter.fillRect(0, 0, width, height(), background);

	int textWidth = effectiveWidth - MARGIN - BASKET_ICON_SIZE - MARGIN - MARGIN;

	// Draw the rounded rectangle:
	if (drawRoundRect) {
		QRect textRect = listView()->fontMetrics().boundingRect(0, 0, /*width=*/1, 500000, Qt::AlignAuto | Qt::AlignTop | Qt::ShowPrefix, text(/*column=*/0));
		int xRound = MARGIN;
		int yRound = MARGIN;
		int hRound = height() - 2 * MARGIN;
		int wRound = QMIN(BASKET_ICON_SIZE + MARGIN + textRect.width() + hRound/2,  effectiveWidth - MARGIN - MARGIN);
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
				QImage imageToScale = buffer.convertToImage();
				QPixmap pmScaled;
				pmScaled.convertFromImage(imageToScale.smoothScale(wRound, hRound));
				thePainter.drawPixmap(xRound, yRound, pmScaled);
				textWidth -= hRound/2;
		}
	}

	QColor bgColor  = listView()->paletteBackgroundColor();
	QColor selColor = KGlobalSettings::highlightColor();
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
		if (painter->fontMetrics().width(theText) > textWidth)
			theText = KStringHandler::rPixelSqueeze(theText, painter->fontMetrics(), textWidth);
		theText = escapedName(theText);
		thePainter.drawText(xText, 0, textWidth, height(), Qt::AlignAuto | Qt::AlignVCenter | Qt::ShowPrefix, theText);
	}

	// If we are filtering all baskets, and are effectively filtering on something:
	if (!countPixmap.isNull())
	{
		thePainter.drawPixmap(effectiveWidth, 1, countPixmap);
		effectiveWidth += countPixmap.width() + MARGIN;
	}
	if (showLoadingIcon) {
		QPixmap icon = kapp->iconLoader()->loadIcon("find", KIcon::NoGroup, 16, KIcon::DefaultState, 0L, /*canReturnNull=*/false);
		thePainter.drawPixmap(effectiveWidth, 0, icon);
		effectiveWidth += BASKET_ICON_SIZE + MARGIN;
	}
	if (showEncryptedIcon && !showLoadingIcon) {
		QPixmap icon = kapp->iconLoader()->loadIcon("encrypted", KIcon::NoGroup, 16, KIcon::DefaultState, 0L, /*canReturnNull=*/false);
		thePainter.drawPixmap(effectiveWidth, 0, icon);
	}

	thePainter.end();

	QPixmapCache::insert(key, theBuffer);
	// Apply the buffer:
	painter->drawPixmap(0, 0, theBuffer);
}

/** class BasketTreeListView: */

BasketTreeListView::BasketTreeListView(QWidget *parent, const char *name)
	: KListView(parent, name), m_autoOpenItem(0)
{
	setWFlags(Qt::WStaticContents | WNoAutoErase);
	clearWFlags(Qt::WStaticContents | WNoAutoErase);
	//viewport()->clearWFlags(Qt::WStaticContents);
	connect( &m_autoOpenTimer, SIGNAL(timeout()), this, SLOT(autoOpen()) );
}

void BasketTreeListView::viewportResizeEvent(QResizeEvent *event)
{
	KListView::viewportResizeEvent(event);
	triggerUpdate();
}

void BasketTreeListView::contentsDragEnterEvent(QDragEnterEvent *event)
{
	std::cout << "BasketTreeListView::contentsDragEnterEvent" << std::endl;
	if (event->provides("application/x-qlistviewitem")) {
		QListViewItemIterator it(this); // TODO: Don't show expanders if it's not a basket drag...
		while (it.current()) {
			QListViewItem *item = it.current();
			if (!item->firstChild()) {
				item->setExpandable(true);
				item->setOpen(true);
			}
			++it;
		}
		update();
	}

	KListView::contentsDragEnterEvent(event);
}

void BasketTreeListView::removeExpands()
{
	QListViewItemIterator it(this);
	while (it.current()) {
		QListViewItem *item = it.current();
		if (!item->firstChild())
			item->setExpandable(false);
		++it;
	}
	viewport()->update();
}

void BasketTreeListView::contentsDragLeaveEvent(QDragLeaveEvent *event)
{
	std::cout << "BasketTreeListView::contentsDragLeaveEvent" << std::endl;
	m_autoOpenItem = 0;
	m_autoOpenTimer.stop();
	removeExpands();
	KListView::contentsDragLeaveEvent(event);
}

void BasketTreeListView::contentsDropEvent(QDropEvent *event)
{
	std::cout << "BasketTreeListView::contentsDropEvent" << std::endl;
	KListView::contentsDropEvent(event);
	m_autoOpenItem = 0;
	m_autoOpenTimer.stop();
	removeExpands();
	Global::bnpView->save(); // TODO: Don't save if it was not a basket drop...
}

void BasketTreeListView::contentsDragMoveEvent(QDragMoveEvent *event)
{
	std::cout << "BasketTreeListView::contentsDragMoveEvent" << std::endl;
	if (event->provides("application/x-qlistviewitem"))
		KListView::contentsDragMoveEvent(event);
	else {
		QListViewItem *item = itemAt(contentsToViewport(event->pos()));
		if (m_autoOpenItem != item) {
			m_autoOpenItem = item;
			m_autoOpenTimer.start(700, /*singleShot=*/true);
		}
		if (item) {
			event->acceptAction(true);
			event->accept(true);
		}
		KListView::contentsDragMoveEvent(event); // FIXME: ADDED
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
	KListView::resizeEvent(event);
}

void BasketTreeListView::paintEmptyArea(QPainter *painter, const QRect &rect)
{
	QListView::paintEmptyArea(painter, rect);

	BasketListViewItem *last = Global::bnpView->lastListViewItem();
	if (last && !last->isShown())
		last = last->shownItemAbove();
	if (last && last->isCurrentBasket()) {
		int y = last->itemPos() + last->height();
		QColor bgColor  = paletteBackgroundColor();
		QColor selColor = KGlobalSettings::highlightColor();
		QColor midColor = Tools::mixColor(bgColor, selColor);
		painter->setPen(selColor);
		painter->drawPoint(visibleWidth() - 1, y);
		painter->drawPoint(visibleWidth() - 2, y);
		painter->drawPoint(visibleWidth() - 1, y + 1);
		painter->setPen(midColor);
		painter->drawPoint(visibleWidth() - 3, y);
		painter->drawPoint(visibleWidth() - 1, y + 2);
	}
}

/** We should NEVER get focus (because of QWidget::NoFocus focusPolicy())
 * but KListView can programatically give us the focus.
 * So we give it to the basket.
 */
void BasketTreeListView::focusInEvent(QFocusEvent*)
{
	//KListView::focusInEvent(event);
	Basket *basket = Global::bnpView->currentBasket();
	if (basket)
		basket->setFocus();
}

#include "basketlistview.moc"

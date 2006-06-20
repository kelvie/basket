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

#include <qtabwidget.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qcursor.h>
#include <qwhatsthis.h>
#include <qvaluelist.h>
#include <qregexp.h>
#include <qbuttongroup.h>
#include <kstringhandler.h>

#include <ksqueezedtextlabel.h>
#include <qpoint.h>
#include <qpixmap.h>
#include <qinputdialog.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <qiconset.h>
#include <kaction.h>
#include <kapp.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kedittoolbar.h>
#include <qsignalmapper.h>
#include <qstringlist.h>

#include <qpainter.h>
#include <qstyle.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <qdir.h>
#include <qstringlist.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <qtimer.h>
#include <qaction.h>
#include <kstdaccel.h>
#include <kglobalaccel.h>
#include <kkeydialog.h>
#include <kpassivepopup.h>
#include <kconfig.h>
#include <kcolordialog.h>

#include <kdeversion.h>
#include <qdesktopwidget.h>
#include <kwin.h>

#include "container.h"
#include "basket.h"
#include "basketproperties.h"
#include "note.h"
#include "noteedit.h" // To launch InlineEditors::initToolBars()
#include "settings.h"
#include "global.h"
//#include "addbasketwizard.h"
#include "newbasketdialog.h"
#include "basketfactory.h"
#include "popupmenu.h"
#include "xmlwork.h"
#include "debugwindow.h"
#include "notefactory.h"
#include "notedrag.h"
#include "tools.h"
#include "tag.h"
#include "formatimporter.h"
#include "softwareimporters.h"
#include "regiongrabber.h"
#include "password.h"


#include <iostream>


/// NEW:

#include <qwidgetstack.h>

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
	Global::mainContainer->currentBasket()->contentsDropEvent(event); // FIXME
}

int BasketListViewItem::width(const QFontMetrics &fontMetrics, const QListView */*listView*/, int column) const
{
	int BASKET_ICON_SIZE = 16;
	int MARGIN = 1;

	QRect textRect = fontMetrics.boundingRect(0, 0, /*width=*/1, 500000, Qt::AlignAuto | Qt::AlignTop | Qt::ShowPrefix, text(column));

	return MARGIN + BASKET_ICON_SIZE + MARGIN + textRect.width() +   BASKET_ICON_SIZE/2   + MARGIN;
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
	return basket() == Global::basketTree->currentBasket();
}

// TODO: Move this function from item.cpp to class Tools:
extern void drawGradient( QPainter *p, const QColor &colorTop, const QColor & colorBottom,
				   int x, int y, int w, int h,
				   bool sunken, bool horz, bool flat  ); /*const*/

QPixmap BasketListViewItem::circledTextPixmap(const QString &text, int height, const QFont &font, const QColor &color)
{
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
		if (!childItem->basket()->isLoaded())
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
	if (width <= 0)
		return;

	int BASKET_ICON_SIZE = 16;
	int MARGIN = 1;


	// If we are filtering all baskets, and are effectively filtering on something:
	bool showLoadingIcon = false;
	QPixmap countPixmap;
	if (Global::basketTree->isFilteringAllBaskets() && Global::basketTree->currentBasket()->decoration()->filterBar()->filterData().isFiltering) {
		showLoadingIcon = !m_basket->isLoaded() || haveHiddenChildsLoading();
		countPixmap = foundCountPixmap(!m_basket->isLoaded(), m_basket->countFounds(), haveHiddenChildsLoading(),
		                               countHiddenChildsFound(), listView()->font(), height() - 2 * MARGIN);
	}
	int effectiveWidth = width - (countPixmap.isNull() ? 0 : countPixmap.width() + MARGIN) - (showLoadingIcon ? BASKET_ICON_SIZE + MARGIN : 0);


	bool drawRoundRect = m_basket->backgroundColorSetting().isValid() || m_basket->textColorSetting().isValid();
	QColor textColor = (drawRoundRect ? m_basket->textColor() : (isCurrentBasket() ? KGlobalSettings::highlightedTextColor() : KGlobalSettings::textColor()));

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
	BasketListViewItem *shownBelow = shownItemBelow();
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
	BasketListViewItem *shownAbove = shownItemAbove();
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
		thePainter.drawPixmap(effectiveWidth, 1, countPixmap);
	if (showLoadingIcon) {
		QPixmap filterIcon = kapp->iconLoader()->loadIcon("find", KIcon::NoGroup, 16, KIcon::DefaultState, 0L, /*canReturnNull=*/false);
		thePainter.drawPixmap(width - 16 - 1, 0, filterIcon);
	}

	thePainter.end();

	// Apply the buffer:
	painter->drawPixmap(0, 0, theBuffer);
}

/** class BasketTreeListView: */

BasketTreeListView::BasketTreeListView(QWidget *parent, const char *name)
 : KListView(parent, name), m_autoOpenItem(0)
{
	connect( &m_autoOpenTimer, SIGNAL(timeout()), this, SLOT(autoOpen()) );
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
	Global::basketTree->save(); // TODO: Don't save if it was not a basket drop...
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
		Global::basketTree->setCurrentBasket(item->basket());
}

void BasketTreeListView::resizeEvent(QResizeEvent *event)
{
	KListView::resizeEvent(event);

	int treeWidth = Global::basketTree->sizes()[Settings::treeOnLeft() ? 0 : 1];
	Settings::setBasketTreeWidth(treeWidth);
	Settings::saveConfig();
}

void BasketTreeListView::paintEmptyArea(QPainter *painter, const QRect &rect)
{
	QListView::paintEmptyArea(painter, rect);

	BasketListViewItem *last = Global::basketTree->lastListViewItem();
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
	Basket *basket = Global::basketTree->currentBasket();
	if (basket)
		basket->setFocus();
}

/** class BasketTree: */

BasketTree::BasketTree(QWidget *parent)
 : QSplitter(Qt::Horizontal, parent), m_loading(true)
{
	Global::basketTree = this;

	/// Configure the Splitter:
	m_tree  = new BasketTreeListView(this);
	m_stack = new QWidgetStack(this);
	setCollapsible(m_tree,  true);
	setCollapsible(m_stack, false);
	setResizeMode(m_tree,  QSplitter::KeepSize);
	setResizeMode(m_stack, QSplitter::Stretch);
	int treeWidth = Settings::basketTreeWidth();
	if (treeWidth < 0)
		treeWidth = m_tree->fontMetrics().maxWidth() * 11;
	QValueList<int> sizes;
	sizes.append(treeWidth);
	setSizes(sizes);
	setOpaqueResize(true);

	/// Configure the List View Columns:
	m_tree->addColumn(i18n("Baskets"));
	m_tree->setColumnWidthMode(0, QListView::Maximum);
	m_tree->setFullWidth(true);
	m_tree->setSorting(-1/*Disabled*/);
	m_tree->setRootIsDecorated(true);
	m_tree->setTreeStepSize(16);
	m_tree->setLineWidth(1);
	m_tree->setMidLineWidth(0);
	m_tree->setFocusPolicy(QWidget::NoFocus);

	/// Configure the List View Drag and Drop:
	m_tree->setDragEnabled(true);
	m_tree->setAcceptDrops(true);
	m_tree->setItemsMovable(true);
	m_tree->setDragAutoScroll(true);
	m_tree->setDropVisualizer(true);
	m_tree->setDropHighlighter(true);

	/// Configure the List View Signals:
	connect( m_tree, SIGNAL(returnPressed(QListViewItem*)),    this, SLOT(slotPressed(QListViewItem*)) );
	connect( m_tree, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotPressed(QListViewItem*)) );
	connect( m_tree, SIGNAL(pressed(QListViewItem*)),          this, SLOT(slotPressed(QListViewItem*)) );
	connect( m_tree, SIGNAL(expanded(QListViewItem*)),         this, SLOT(needSave(QListViewItem*))    );
	connect( m_tree, SIGNAL(collapsed(QListViewItem*)),        this, SLOT(needSave(QListViewItem*))    );
	connect( m_tree, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),      this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&))      );
	connect( m_tree, SIGNAL(mouseButtonPressed(int, QListViewItem*, const QPoint&, int)), this, SLOT(slotMouseButtonPressed(int, QListViewItem*, const QPoint&, int)) );
	connect( m_tree, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)), this, SLOT(slotShowProperties(QListViewItem*, const QPoint&, int)) );

	connect( m_tree, SIGNAL(expanded(QListViewItem*)),  this, SIGNAL(basketChanged()) );
	connect( m_tree, SIGNAL(collapsed(QListViewItem*)), this, SIGNAL(basketChanged()) );
	connect( this,   SIGNAL(basketNumberChanged(int)),  this, SIGNAL(basketChanged()) );

	/// What's This Help for the tree:
	QWhatsThis::add(m_tree, i18n(
		"<h2>Basket Tree</h2>"
		"Here is the list of your baskets. "
		"You can organize your data by putting them in different baskets. "
		"You can group baskets by subject by creating new baskets inside others. "
		"You can browse between them by clicking a basket to open it, or reorganize them using drag and drop."));

	setTreePlacement(Settings::treeOnLeft());
}

BasketTree::~BasketTree()
{
}

QListViewItem* BasketTree::firstListViewItem()
{
	return m_tree->firstChild();
}

void BasketTree::slotShowProperties(QListViewItem *item, const QPoint&, int)
{
	if (item)
		Global::mainContainer->propBasket();
}

void BasketTree::slotMouseButtonPressed(int button, QListViewItem *item, const QPoint &/*pos*/, int /*column*/)
{
	if (item && (button & Qt::MidButton)) {
		// TODO: Paste into ((BasketListViewItem*)listViewItem)->basket()
	}
}

void BasketTree::slotContextMenu(KListView */*listView*/, QListViewItem *item, const QPoint &pos)
{
	QString menuName;
	if (item) {
		Basket* basket = ((BasketListViewItem*)item)->basket();

		setCurrentBasket(basket);
		menuName = "basket_popup";
	} else {
		menuName = "tab_bar_popup";
		/*
		 * "File -> New" create a new basket with the same parent basket as the the current one.
		 * But when invoked when right-clicking the empty area at the bottom of the basket tree,
		 * it is obvious the user want to create a new basket at the bottom of the tree (with no parent).
		 * So we set a temporary variable during the time the popup menu is shown,
		 * so the slot askNewBasket() will do the right thing:
		 */
		Global::mainContainer->setNewBasketPopup();
	}

	QPopupMenu *menu = Global::mainContainer->popupMenu(menuName);
	connect( menu, SIGNAL(aboutToHide()),  Global::mainContainer, SLOT(aboutToHideNewBasketPopup()) );
	menu->exec(pos);
}

void Container::aboutToHideNewBasketPopup()
{
	QTimer::singleShot(0, this, SLOT(cancelNewBasketPopup()));
}

void Container::cancelNewBasketPopup()
{
	m_newBasketPopup = false;
}

void Container::setNewBasketPopup()
{
	m_newBasketPopup = true;
}

void BasketTree::save()
{
	DEBUG_WIN << "Basket Tree: Saving...";

	// Create Document:
	QDomDocument document("basketTree");
	QDomElement root = document.createElement("basketTree");
	document.appendChild(root);

	// Save Basket Tree:
	save(m_tree->firstChild(), document, root);

	// Write to Disk:
	QFile file(Global::basketsFolder() + "baskets.xml");
	if (file.open(IO_WriteOnly)) {
		QTextStream stream(&file);
		stream.setEncoding(QTextStream::UnicodeUTF8);
		QString xml = document.toString();
		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
		stream << xml;
		file.close();
	}
}

void BasketTree::save(QListViewItem *firstItem, QDomDocument &document, QDomElement &parentElement)
{
	QListViewItem *item = firstItem;
	while (item) {
		Basket *basket = ((BasketListViewItem*)item)->basket();
		QDomElement basketElement = document.createElement("basket");
		parentElement.appendChild(basketElement);
		// Save Attributes:
		basketElement.setAttribute("folderName", basket->folderName());
		if (item->firstChild()) // If it can be expanded/folded:
			basketElement.setAttribute("folded", XMLWork::trueOrFalse(!item->isOpen()));
		if (((BasketListViewItem*)item)->isCurrentBasket())
			basketElement.setAttribute("lastOpened", "true");
		// Save Properties:
		QDomElement properties = document.createElement("properties");
		basketElement.appendChild(properties);
		basket->saveProperties(document, properties);
		// Save Child Basket:
		if (item->firstChild())
			save(item->firstChild(), document, basketElement);
		// Next Basket:
		item = item->nextSibling();
	}
}

void BasketTree::load()
{
	QDomDocument *doc = XMLWork::openFile("basketTree", Global::basketsFolder() + "baskets.xml");
	//BEGIN Compatibility with 0.6.0 Pre-Alpha versions:
	if (!doc)
		doc = XMLWork::openFile("basketsTree", Global::basketsFolder() + "baskets.xml");
	//END
	if (doc != 0) {
		QDomElement docElem = doc->documentElement();
		load(m_tree, 0L, docElem);
	}
	m_loading = false;
}

void BasketTree::load(KListView */*listView*/, QListViewItem *item, const QDomElement &baskets)
{
	QDomNode n = baskets.firstChild();
	while ( ! n.isNull() ) {
		QDomElement element = n.toElement();
		if ( (!element.isNull()) && element.tagName() == "basket" ) {
			QString folderName = element.attribute("folderName");
			if (!folderName.isEmpty()) {
				Basket *basket = loadBasket(folderName);
				BasketListViewItem *basketItem = appendBasket(basket, item);
				basketItem->setOpen(!XMLWork::trueOrFalse(element.attribute("folded", "false"), false));
				basket->loadProperties(XMLWork::getElement(element, "properties"));
				if (XMLWork::trueOrFalse(element.attribute("lastOpened", element.attribute("lastOpenned", "false")), false)) // Compat with 0.6.0-Alphas
					setCurrentBasket(basket);
				// Load Sub-baskets:
				load(/*(QListView*)*/0L, basketItem, element);
			}
		}
		n = n.nextSibling();
	}
}

Basket* BasketTree::loadBasket(const QString &folderName)
{
	if (folderName.isEmpty())
		return 0;

	DecoratedBasket *decoBasket = new DecoratedBasket(m_stack, folderName);
	Basket          *basket     = decoBasket->basket();
	m_stack->addWidget(decoBasket);
	connect( basket, SIGNAL(countsChanged(Basket*)), Global::mainContainer, SLOT(countsChanged(Basket*)) );
	// Important: Create listViewItem and connect signal BEFORE loadProperties(), so we get the listViewItem updated without extra work:
	connect( basket, SIGNAL(propertiesChanged(Basket*)), this, SLOT(updateBasketListViewItem(Basket*)) );

	connect( basket->decoration()->filterBar(), SIGNAL(newFilter(const FilterData&)), this, SLOT(newFilterFromFilterBar()) );

	return basket;
}

int BasketTree::basketCount(QListViewItem *parent)
{
	int count = 0;

	QListViewItem *item = (parent ? parent->firstChild() : m_tree->firstChild());
	while (item) {
		count += 1 + basketCount(item);
		item = item->nextSibling();
	}

	return count;
}

bool BasketTree::canFold()
{
	BasketListViewItem *item = listViewItemForBasket(currentBasket());
	if (!item)
		return false;
	return item->parent() || (item->firstChild() && item->isOpen());
}

bool BasketTree::canExpand()
{
	BasketListViewItem *item = listViewItemForBasket(currentBasket());
	if (!item)
		return false;
	return item->firstChild();
}

BasketListViewItem* BasketTree::appendBasket(Basket *basket, QListViewItem *parentItem)
{
	BasketListViewItem *newBasketItem;
	if (parentItem)
		newBasketItem = new BasketListViewItem(parentItem, ((BasketListViewItem*)parentItem)->lastChild(), basket);
	else {
		QListViewItem *child     = m_tree->firstChild();
		QListViewItem *lastChild = 0;
		while (child) {
			lastChild = child;
			child = child->nextSibling();
		}
		newBasketItem = new BasketListViewItem(m_tree, lastChild, basket);
	}

	emit basketNumberChanged(basketCount());

	return newBasketItem;
}

void BasketTree::loadNewBasket(const QString &folderName, const QDomElement &properties, Basket *parent)
{
	Basket *basket = loadBasket(folderName);
	appendBasket(basket, (basket ? listViewItemForBasket(parent) : 0));
	basket->loadProperties(properties);
	setCurrentBasket(basket);
//	save();
}

BasketListViewItem* BasketTree::lastListViewItem()
{
	QListViewItem *child     = m_tree->firstChild();
	QListViewItem *lastChild = 0;
	// Set lastChild to the last primary child of the list view:
	while (child) {
		lastChild = child;
		child = child->nextSibling();
	}
	// If this child have child(s), recursivly browse through them to find the real last one:
	while (lastChild && lastChild->firstChild()) {
		child = lastChild->firstChild();
		while (child) {
			lastChild = child;
			child = child->nextSibling();
		}
	}
	return (BasketListViewItem*)lastChild;
}

void BasketTree::goToPreviousBasket()
{
	if (!m_tree->firstChild())
		return;

	BasketListViewItem *item     = listViewItemForBasket(currentBasket());
	BasketListViewItem *toSwitch = item->shownItemAbove();
	if (!toSwitch) {
		toSwitch = lastListViewItem();
		if (toSwitch && !toSwitch->isShown())
			toSwitch = toSwitch->shownItemAbove();
	}

	if (toSwitch)
		setCurrentBasket(toSwitch->basket());

	if (Settings::usePassivePopup())
		Global::mainContainer->showPassiveContent();
}

void BasketTree::goToNextBasket()
{
	if (!m_tree->firstChild())
		return;

	BasketListViewItem *item     = listViewItemForBasket(currentBasket());
	BasketListViewItem *toSwitch = item->shownItemBelow();
	if (!toSwitch)
		toSwitch = ((BasketListViewItem*)m_tree->firstChild());

	if (toSwitch)
		setCurrentBasket(toSwitch->basket());

	if (Settings::usePassivePopup())
		Global::mainContainer->showPassiveContent();
}

void BasketTree::foldBasket()
{
	BasketListViewItem *item = listViewItemForBasket(currentBasket());
	if (item && !item->firstChild())
		item->setOpen(false); // If Alt+Left is hitted and there is nothing to close, make sure the focus will go to the parent basket

	QKeyEvent* keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, 0, 0);
	QApplication::postEvent(m_tree, keyEvent);
}

void BasketTree::expandBasket()
{
	QKeyEvent* keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, 0, 0);
	QApplication::postEvent(m_tree, keyEvent);
}

void BasketTree::closeAllEditors()
{
	QListViewItemIterator it(m_tree);
	while (it.current()) {
		BasketListViewItem *item = (BasketListViewItem*)(it.current());
		item->basket()->closeEditor();
		++it;
	}
}

/** isRunning is to avoid recursive calls because this method can be called
  * when clicking the menu action or when using the filter-bar icon... either of those calls
  * call the other to be checked... and it can cause recursive calls.
  * PS: Uggly hack? Yes, I think so :-)
  */
void BasketTree::toggleFilterAllBaskets(bool doFilter)
{
	static bool isRunning = false;
	if (isRunning)
		return;
	isRunning = true;

	// Set the state:
	Global::mainContainer->m_actFilterAllBaskets->setChecked(doFilter);
	//currentBasket()->decoration()->filterBar()->setFilterAll(doFilter);

//	Basket *current = currentBasket();
	QListViewItemIterator it(m_tree);
	while (it.current()) {
		BasketListViewItem *item = ((BasketListViewItem*)it.current());
		item->basket()->decoration()->filterBar()->setFilterAll(doFilter);
		++it;
	}

	// Protection is not necessary anymore:
	isRunning = false;

	if (doFilter)
		currentBasket()->decoration()->filterBar()->setEditFocus();

	// Filter every baskets:
	newFilter();
}

/** This function can be called recursively because we call kapp->processEvents().
  * If this function is called whereas another "instance" is running,
  * this new "instance" leave and set up a flag that is read by the first "instance"
  * to know it should re-begin the work.
  * PS: Yes, that's a very lame pseudo-threading but that works, and it's programmer-efforts cheap :-)
  */
void BasketTree::newFilter()
{
	static bool alreadyEntered = false;
	static bool shouldRestart  = false;

	if (alreadyEntered) {
		shouldRestart = true;
		return;
	}
	alreadyEntered = true;
	shouldRestart  = false;

	Basket *current = currentBasket();
	const FilterData &filterData = current->decoration()->filterBar()->filterData();

	// Set the filter data for every other baskets, or reset the filter for every other baskets if we just disabled the filterInAllBaskets:
	QListViewItemIterator it(m_tree);
	while (it.current()) {
		BasketListViewItem *item = ((BasketListViewItem*)it.current());
		if (item->basket() != current)
			if (isFilteringAllBaskets())
				item->basket()->decoration()->filterBar()->setFilterData(filterData); // Set the new FilterData for every other baskets
			else
				item->basket()->decoration()->filterBar()->setFilterData(FilterData()); // We just disabled the global filtering: remove the FilterData
		++it;
	}

	// Show/hide the "little filter icons" (during basket load)
	// or the "little numbers" (to show number of found notes in the baskets) is the tree:
	m_tree->triggerUpdate();
	kapp->processEvents();

	// Load every baskets for filtering, if they are not already loaded, and if necessary:
	if (filterData.isFiltering) {
		Basket *current = currentBasket();
		QListViewItemIterator it(m_tree);
		while (it.current()) {
			BasketListViewItem *item = ((BasketListViewItem*)it.current());
			if (item->basket() != current) {
				Basket *basket = item->basket();
				if (!basket->loadingLaunched())
					basket->load();
				m_tree->triggerUpdate();
				kapp->processEvents();
				if (shouldRestart) {
					alreadyEntered = false;
					shouldRestart  = false;
					newFilter();
					return;
				}
			}
			++it;
		}
	}

	m_tree->triggerUpdate();
//	kapp->processEvents();

	alreadyEntered = false;
	shouldRestart  = false;
}

void BasketTree::newFilterFromFilterBar()
{
	if (isFilteringAllBaskets())
		QTimer::singleShot(0, this, SLOT(newFilter())); // Keep time for the QLineEdit to display the filtered character and refresh correctly!
}

bool BasketTree::isFilteringAllBaskets()
{
	return Global::mainContainer->m_actFilterAllBaskets->isChecked();
}


BasketListViewItem* BasketTree::listViewItemForBasket(Basket *basket)
{
	QListViewItemIterator it(m_tree);
	while (it.current()) {
		BasketListViewItem *item = ((BasketListViewItem*)it.current());
		if (item->basket() == basket)
			return item;
		++it;
	}
	return 0L;
}

Basket* BasketTree::currentBasket()
{
	DecoratedBasket *decoBasket = (DecoratedBasket*)m_stack->visibleWidget();
	if (decoBasket)
		return decoBasket->basket();
	else
		return 0;
}

Basket* BasketTree::parentBasketOf(Basket *basket)
{
	BasketListViewItem *item = (BasketListViewItem*)(listViewItemForBasket(basket)->parent());
	if (item)
		return item->basket();
	else
		return 0;
}

void BasketTree::setCurrentBasket(Basket *basket)
{
	if (currentBasket() == basket)
		return;

	if (currentBasket())
		currentBasket()->closeEditor();

	BasketListViewItem *item = listViewItemForBasket(basket);
	if (item) {
		m_tree->setSelected(item, true);
		item->ensureVisible();
		m_stack->raiseWidget(basket->decoration());
		// If the window has changed size, only the current basket receive the event,
		// the others will receive ony one just before they are shown.
		// But this triggers unwanted animations, so we eliminate it:
		basket->relayoutNotes(/*animate=*/false);
		Global::mainContainer->setCaption(item->basket()->basketName());
		Global::mainContainer->countsChanged(basket);
		Global::mainContainer->updateStatusBarHint();
		if (Global::tray)
			Global::tray->updateToolTip();
	}
	m_tree->viewport()->update();
	emit basketChanged();
}

void BasketTree::removeBasket(Basket *basket)
{
	if (basket->isDuringEdit())
		basket->closeEditor();

	// Find a new basket to switch to and select it.
	// Strategy: get the next sibling, or the previous one if not found.
	// If there is no such one, get the parent basket:
	BasketListViewItem *basketItem = listViewItemForBasket(basket);
	BasketListViewItem *nextBasketItem = (BasketListViewItem*)(basketItem->nextSibling());
	if (!nextBasketItem)
		nextBasketItem = basketItem->prevSibling();
	if (!nextBasketItem)
		nextBasketItem = (BasketListViewItem*)(basketItem->parent());

	if (nextBasketItem)
		setCurrentBasket(nextBasketItem->basket());

	// Remove from the view:
	basket->unsubscribeBackgroundImages();
	m_stack->removeWidget(basket->decoration());
//	delete basket->decoration();
	delete basketItem;
//	delete basket;

	// If there is no basket anymore, add a new one:
	if (!nextBasketItem)
		BasketFactory::newBasket(/*icon=*/"", /*name=*/i18n("General"), /*backgroundImage=*/"", /*backgroundColor=*/QColor(), /*textColor=*/QColor(), /*templateName=*/"1column", /*createIn=*/0);
	else // No need to save two times if we add a basket
		save();

	emit basketNumberChanged(basketCount());
}

void BasketTree::setTreePlacement(bool onLeft)
{
	if (onLeft)
		moveToFirst(m_tree);
	else
		moveToLast(m_tree);
	//updateGeometry();
	kapp->postEvent( this, new QResizeEvent(size(), size()) );
}

void BasketTree::relayoutAllBaskets()
{
	QListViewItemIterator it(m_tree);
	while (it.current()) {
		BasketListViewItem *item = ((BasketListViewItem*)it.current());
		//item->basket()->unbufferizeAll();
		item->basket()->unsetNotesWidth();
		item->basket()->relayoutNotes(true);
		++it;
	}
}

void BasketTree::linkLookChanged()
{
	QListViewItemIterator it(m_tree);
	while (it.current()) {
		BasketListViewItem *item = ((BasketListViewItem*)it.current());
		item->basket()->linkLookChanged();
		++it;
	}
}

void BasketTree::filterPlacementChanged(bool onTop)
{
	QListViewItemIterator it(m_tree);
	while (it.current()) {
		BasketListViewItem *item        = static_cast<BasketListViewItem*>(it.current());
		DecoratedBasket    *decoration  = static_cast<DecoratedBasket*>(item->basket()->parent());
		decoration->setFilterBarPosition(onTop);
		++it;
	}
}

void BasketTree::updateBasketListViewItem(Basket *basket)
{
	BasketListViewItem *item = listViewItemForBasket(basket);
	if (item)
		item->setup();

	if (basket == currentBasket()) {
		Global::mainContainer->setCaption(basket->basketName());
		if (Global::tray)
			Global::tray->updateToolTip();
	}

	// Don't save if we are loading!
	if (!m_loading)
		save();
}

void BasketTree::needSave(QListViewItem*)
{
	if (!m_loading)
		// A basket has been collapsed/expanded or a new one is select: this is not urgent:
		QTimer::singleShot(500/*ms*/, this, SLOT(save()));
}

void BasketTree::slotPressed(QListViewItem *item, const QPoint &/*pos*/, int /*column*/)
{
	// Impossible to Select no Basket:
	if (!item)
		m_tree->setSelected(listViewItemForBasket(currentBasket()), true);
	else if (currentBasket() != ((BasketListViewItem*)item)->basket()) {
		setCurrentBasket( ((BasketListViewItem*)item)->basket() );
		needSave(0);
	}
	currentBasket()->setFocus();
}

/// ///

/** DektopColorPicker */

/* From Qt documentation:
 * " Note that only visible widgets can grab mouse input.
 *   If isVisible() returns FALSE for a widget, that widget cannot call grabMouse(). "
 * So, we should use an always visible widget to be able to pick a color from screen,
 * even by first hidding the main window (user seldomly want to grab a color from BasKet!)
 * or use a global shortcut (main window can be hidden when hitting that shortcut).
 */

DesktopColorPicker::DesktopColorPicker()
 : QDesktopWidget()
{
	setName("DesktopColorPicker");
	m_gettingColorFromScreen = false;
}

DesktopColorPicker::~DesktopColorPicker()
{
}

void DesktopColorPicker::pickColor()
{
	m_gettingColorFromScreen = true;
//	Global::mainContainer->setActive(false);
	QTimer::singleShot( 50, this, SLOT(slotDelayedPick()) );
}

/* When firered from basket context menu, and not from menu, grabMouse doesn't work!
 * It's perhapse because context menu call slotColorFromScreen() and then
 * ungrab the mouse (since menus grab the mouse).
 * But why isn't there such bug with normal menus?...
 * By calling this method with a QTimer::singleShot, we are sure context menu code is
 * finished and we can grab the mouse without loosing the grab:
 */
void DesktopColorPicker::slotDelayedPick()
{
	grabKeyboard();
	grabMouse(crossCursor);
}

/* Validate the color
 */
void DesktopColorPicker::mouseReleaseEvent(QMouseEvent *event)
{
	if (m_gettingColorFromScreen) {
		m_gettingColorFromScreen = false;
		releaseMouse();
		releaseKeyboard();
		QColor color = KColorDialog::grabColor(event->globalPos());
		emit pickedColor(color);
	} else
		QDesktopWidget::mouseReleaseEvent(event);
}

/* Cancel the mode
 */
void DesktopColorPicker::keyPressEvent(QKeyEvent *event)
{
	if (m_gettingColorFromScreen)
		if (event->key() == Qt::Key_Escape) {
			m_gettingColorFromScreen = false;
			releaseMouse();
			releaseKeyboard();
			emit canceledPick();
		}
	QDesktopWidget::keyPressEvent(event);
}

/** KSystemTray2 */

// To draw the systray screenshot image:
#include <qdesktopwidget.h>
#include <qmime.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qpixmap.h>
// To know the program name:
#include <kglobal.h>
#include <kinstance.h>
#include <kaboutdata.h>
// Others:
#include <kmessagebox.h>
#include <kmanagerselection.h>

KSystemTray2::KSystemTray2(QWidget *parent, const char *name)
 : KSystemTray(parent, name)
{
}

KSystemTray2::~KSystemTray2()
{
}

void KSystemTray2::displayCloseMessage(QString fileMenu)
{
	/* IDEAS OF IMPROVEMENTS:
	*  - Use queuedMessageBox() but it need a dontAskAgainName parameter
	*    and image in QMimeSourceFactory shouldn't be removed.
	*  - Sometimes the systray icon is covered (a passive popup...)
	*    Use XComposite extension, if available, to get the kicker pixmap.
	*  - Perhapse desaturate the area around the proper SysTray icon,
	*    helping bring it into sharper focus. Or draw the cicle with XOR
	*    brush.
	*  - Perhapse add the icon in the text (eg. "... in the
	*    system tray ([icon])."). Add some clutter to the dialog.
	*/
#if KDE_IS_VERSION( 3, 1, 90 )
	// Don't do all the computations if they are unneeded:
	if ( ! KMessageBox::shouldBeShownContinue("hideOnCloseInfo") )
		return;
#endif
	// "Default parameter". Here, to avoid a i18n() call and dependancy in the .h
	if (fileMenu.isEmpty())
		fileMenu = i18n("File");

	// Some values we need:
	QPoint g = mapToGlobal(pos());
	int desktopWidth  = kapp->desktop()->width();
	int desktopHeight = kapp->desktop()->height();
	int tw = width();
	int th = height();

	// We are triying to make a live screenshot of the systray icon to circle it
	//  and show it to the user. If no systray is used or if the icon is not visible,
	//  we should not show that screenshot but only a text!

	// 1. Determine if the user use a system tray area or not:
	QCString screenstr;
	screenstr.setNum(qt_xscreen());
	QCString trayatom = "_NET_SYSTEM_TRAY_S" + screenstr;
	bool useSystray = (KSelectionWatcher(trayatom).owner() != None);

	// 2. And then if the icon is visible too (eg. this->show() has been called):
	useSystray = useSystray && isVisible();

	// 3. Kicker (or another systray manager) can be visible but masked out of
	//    the screen (ie. on right or on left of it). We check if the icon isn't
	//    out of screen.
	if (useSystray) {
		QRect deskRect(0, 0, desktopWidth, desktopHeight);
		if ( !deskRect.contains(g.x(), g.y()) ||
		     !deskRect.contains(g.x() + tw, g.y() + th) )
			useSystray = false;
	}

	// 4. We raise the window containing the systray icon (typically the kicker) to
	//    have the most chances it is visible during the capture:
/*	if (useSystray) {
		// We are testing if one of the corners is hidden, and if yes, we would enter
		// a time consuming process (raise kicker and wait some time):
//		if (kapp->widgetAt(g) != this ||
//		    kapp->widgetAt(g + QPoint(tw-1, 0)) != this ||
//		    kapp->widgetAt(g + QPoint(0, th-1)) != this ||
//		    kapp->widgetAt(g + QPoint(tw-1, th-1)) != this) {
			int systrayManagerWinId = topLevelWidget()->winId();
			KWin::forceActiveWindow(systrayManagerWinId);
			kapp->processEvents(); // Because without it the systrayManager is raised only after the messageBox is displayed
//			KWin::activateWindow(systrayManagerWinId);
//			kapp->processEvents(); // Because without it the systrayManager is raised only after the messageBox is displayed
//				KWin::raiseWindow(systrayManagerWinId);
//			kapp->processEvents(); // Because without it the systrayManager is raised only after the messageBox is displayed
			sleep(1);
			// TODO: Re-verify that at least one corner is now visible
//		}
	}*/

//	KMessageBox::information(this, QString::number(g.x()) + ":" + QString::number(g.y()) + ":" +
//	                         QString::number((int)(kapp->widgetAt(g+QPoint(1,1)))));

	QString message = i18n(
		"<p>Closing the main window will keep %1 running in the system tray. "
		"Use <b>Quit</b> from the <b>Basket</b> menu to quit the application.</p>"
			).arg(KGlobal::instance()->aboutData()->programName());
	// We are sure the systray icon is visible: ouf!
	if (useSystray) {
		// Compute size and position of the pixmap to be grabbed:
		int w = desktopWidth / 4;
		int h = desktopHeight / 9;
		int x = g.x() + tw/2 - w/2; // Center the rectange in the systray icon
		int y = g.y() + th/2 - h/2;
		if (x < 0)                 x = 0; // Move the rectangle to stay in the desktop limits
		if (y < 0)                 y = 0;
		if (x + w > desktopWidth)  x = desktopWidth - w;
		if (y + h > desktopHeight) y = desktopHeight - h;

		// Grab the desktop and draw a circle arround the icon:
		QPixmap shot = QPixmap::grabWindow(qt_xrootwin(), x, y, w, h);
		QPainter painter(&shot);
		const int CIRCLE_MARGINS = 6;
		const int CIRCLE_WIDTH   = 3;
		const int SHADOW_OFFSET  = 1;
		const int IMAGE_BORDER   = 1;
		int ax = g.x() - x - CIRCLE_MARGINS - 1;
		int ay = g.y() - y - CIRCLE_MARGINS - 1;
		painter.setPen( QPen(KApplication::palette().active().dark(), CIRCLE_WIDTH) );
		painter.drawArc(ax + SHADOW_OFFSET, ay + SHADOW_OFFSET,
		                tw + 2*CIRCLE_MARGINS, th + 2*CIRCLE_MARGINS, 0, 16*360);
		painter.setPen( QPen(Qt::red/*KApplication::palette().active().highlight()*/, CIRCLE_WIDTH) );
		painter.drawArc(ax, ay, tw + 2*CIRCLE_MARGINS, th + 2*CIRCLE_MARGINS, 0, 16*360);
#if 1
		// Draw the pixmap over the screenshot in case a window hide the icon:
		painter.drawPixmap(g.x() - x, g.y() - y + 1, *pixmap());
#endif
		painter.end();

		// Then, we add a border arround the image to make it more visible:
		QPixmap finalShot(w + 2*IMAGE_BORDER, h + 2*IMAGE_BORDER);
		finalShot.fill(KApplication::palette().active().foreground());
		painter.begin(&finalShot);
		painter.drawPixmap(IMAGE_BORDER, IMAGE_BORDER, shot);
		painter.end();

		// Associate source to image and show the dialog:
		QMimeSourceFactory::defaultFactory()->setPixmap("systray_shot", finalShot);
		KMessageBox::information(this,
			message + "<p><center><img source=\"systray_shot\"></center></p>",
			i18n("Docking in System Tray"), "hideOnCloseInfo");
		QMimeSourceFactory::defaultFactory()->setData("systray_shot", 0L);
	} else {
		KMessageBox::information(this,
			message,
			i18n("Docking in System Tray"), "hideOnCloseInfo");
	}
}

/** ContainerSystemTray */

const int Container::c_delayTooltipTime = 275;

ContainerSystemTray::ContainerSystemTray(QWidget *parent, const char *name)
 : KSystemTray2(parent, name != 0 ? name : "ContainerSystemTray"), m_parentContainer((Container*)parent)
{
	setAcceptDrops(true);

	m_showTimer = new QTimer(this);
	connect( m_showTimer, SIGNAL(timeout()), m_parentContainer, SLOT(setActive()) );

	m_autoShowTimer = new QTimer(this);
	connect( m_autoShowTimer, SIGNAL(timeout()), m_parentContainer, SLOT(setActive()) );

	// Create pixmaps for the icon:
	m_iconPixmap              = loadIcon("basket");
//	FIXME: When main window is shown at start, the icon is loaded 1 pixel too high
//	       and then reloaded instantly after at the right position.
//	setPixmap(m_iconPixmap); // Load it the sooner as possible to avoid flicker
	QImage  lockedIconImage   = m_iconPixmap.convertToImage();
	QPixmap lockOverlayPixmap = loadIcon("lockoverlay");
	QImage  lockOverlayImage  = lockOverlayPixmap.convertToImage();
	KIconEffect::overlay(lockedIconImage, lockOverlayImage);
	m_lockedIconPixmap.convertFromImage(lockedIconImage);

	updateToolTip(); // Set toolTip AND icon
}

ContainerSystemTray::~ContainerSystemTray()
{
}

void ContainerSystemTray::mousePressEvent(QMouseEvent *event)
{
	if (event->button() & Qt::LeftButton) {          // Prepare drag
		m_pressPos = event->globalPos();
		m_canDrag  = true;
		event->accept();
	} else if (event->button() & Qt::MidButton) {    // Paste
		m_parentContainer->currentBasket()->setInsertPopupMenu();
		m_parentContainer->currentBasket()->pasteNote(QClipboard::Selection);
		m_parentContainer->currentBasket()->cancelInsertPopupMenu();
		if (Settings::usePassivePopup())
			Global::mainContainer->showPassiveDropped(i18n("Pasted selection to basket <i>%1</i>"));
		event->accept();
	} else if (event->button() & Qt::RightButton) { // Popup menu
		KPopupMenu menu(this);
		menu.insertTitle( SmallIcon("basket"), kapp->aboutData()->programName() );

		m_parentContainer->actNewBasket->plug(&menu);
		m_parentContainer->actNewSubBasket->plug(&menu);
		m_parentContainer->actNewSiblingBasket->plug(&menu);
		menu.insertSeparator();
		m_parentContainer->m_actPaste->plug(&menu);
		m_parentContainer->m_actGrabScreenshot->plug(&menu);
		m_parentContainer->m_actColorPicker->plug(&menu);
		menu.insertSeparator();
		m_parentContainer->actConfigGlobalShortcuts->plug(&menu);
		m_parentContainer->actAppConfig->plug(&menu);
		menu.insertSeparator();

		// Minimize / restore : since we manage the popup menu by ourself, we should do that work :
		KAction* action = actionCollection()->action("minimizeRestore");
		if (m_parentContainer->isVisible())
			action->setText(i18n("&Minimize"));
		else
			action->setText(i18n("&Restore"));
		action->plug(&menu);

		m_parentContainer->actQuit->plug(&menu);

		Global::basketTree->currentBasket()->setInsertPopupMenu();
		connect( &menu, SIGNAL(aboutToHide()), Global::basketTree->currentBasket(), SLOT(delayedCancelInsertPopupMenu()) );
		menu.exec(event->globalPos());
		event->accept();
	} else
		event->ignore();
}

void ContainerSystemTray::mouseMoveEvent(QMouseEvent *event)
{
	event->ignore();
}

void ContainerSystemTray::mouseReleaseEvent(QMouseEvent *event)
{
	m_canDrag = false;

	if (event->button() == Qt::LeftButton)         // Show / hide main window
		if ( rect().contains(event->pos()) ) {     // Accept only if released in systemTray
			m_parentContainer->changeActive();
			event->accept();
		} else
			event->ignore();
}

void ContainerSystemTray::dragEnterEvent(QDragEnterEvent *event)
{
	m_showTimer->start( Settings::dropTimeToShow() * 100, true );
	m_parentContainer->currentBasket()->showFrameInsertTo();
///	m_parentContainer->setStatusBarDrag(); // FIXME: move this line in Basket::showFrameInsertTo() ?
	Basket::acceptDropEvent(event);
}

void ContainerSystemTray::dragMoveEvent(QDragMoveEvent *event)
{
	Basket::acceptDropEvent(event);
}

void ContainerSystemTray::dragLeaveEvent(QDragLeaveEvent*)
{
	m_showTimer->stop();
	m_canDrag = false;
	m_parentContainer->currentBasket()->resetInsertTo();
	m_parentContainer->updateStatusBarHint();
}

void ContainerSystemTray::dropEvent(QDropEvent *event)
{
	m_showTimer->stop();
	m_parentContainer->currentBasket()->dropEvent(event);

	if (Settings::usePassivePopup())
		Global::mainContainer->showPassiveDropped(i18n("Dropped to basket <i>%1</i>"));
}

/* This function comes directly from JuK: */

/*
 * This function copies the entirety of src into dest, starting in
 * dest at x and y.  This function exists because I was unable to find
 * a function like it in either QImage or kdefx
 */
static bool copyImage(QImage &dest, QImage &src, int x, int y)
{
	if(dest.depth() != src.depth())
		return false;
	if((x + src.width()) >= dest.width())
		return false;
	if((y + src.height()) >= dest.height())
		return false;

	// We want to use KIconEffect::overlay to do this, since it handles
	// alpha, but the images need to be the same size.  We can handle that.

	QImage large_src(dest);

	// It would perhaps be better to create large_src based on a size, but
	// this is the easiest way to make a new image with the same depth, size,
	// etc.

	large_src.detach();

	// However, we do have to specifically ensure that setAlphaBuffer is set
	// to false

	large_src.setAlphaBuffer(false);
	large_src.fill(0); // All transparent pixels
	large_src.setAlphaBuffer(true);

	int w = src.width();
	int h = src.height();
	for(int dx = 0; dx < w; dx++)
		for(int dy = 0; dy < h; dy++)
			large_src.setPixel(dx + x, dy + y, src.pixel(dx, dy));

	// Apply effect to image

	KIconEffect::overlay(dest, large_src);

	return true;
}

void ContainerSystemTray::updateToolTip()
{
//	return; /////////////////////////////////////////////////////

	Basket *basket = m_parentContainer->currentBasket();
	if (!basket)
		return;

	if (basket->icon().isEmpty() || basket->icon() == "basket" || ! Settings::showIconInSystray())
		setPixmap(basket->isLocked() ? m_lockedIconPixmap : m_iconPixmap);
	else {
		// Code that comes from JuK:
		QPixmap bgPix = loadIcon("basket");
		QPixmap fgPix = SmallIcon(basket->icon());

		QImage bgImage = bgPix.convertToImage(); // Probably 22x22
		QImage fgImage = fgPix.convertToImage(); // Should be 16x16
		QImage lockOverlayImage = loadIcon("lockoverlay").convertToImage();

		KIconEffect::semiTransparent(bgImage);
		copyImage(bgImage, fgImage, (bgImage.width() - fgImage.width()) / 2,
				(bgImage.height() - fgImage.height()) / 2);
		if (basket->isLocked())
			KIconEffect::overlay(bgImage, lockOverlayImage);

		bgPix.convertFromImage(bgImage);
		setPixmap(bgPix);
	}

	//QTimer::singleShot( Container::c_delayTooltipTime, this, SLOT(updateToolTipDelayed()) );
	// No need to delay: it's be called when notes are changed:
	updateToolTipDelayed();
}

void ContainerSystemTray::updateToolTipDelayed()
{
	Basket *basket = m_parentContainer->currentBasket();

	QString tip = "<p><nobr>" + ( basket->isLocked() ? kapp->makeStdCaption(i18n("%1 (Locked)"))
	                                                 : kapp->makeStdCaption(     "%1")          )
	                            .arg(Tools::textToHTMLWithoutP(basket->basketName()));

	QToolTip::add(this, tip);
}

void ContainerSystemTray::wheelEvent(QWheelEvent *event)
{
	if (event->delta() > 0)
		Global::basketTree->goToPreviousBasket();
	else
		Global::basketTree->goToNextBasket();

	if (Settings::usePassivePopup())
		Global::mainContainer->showPassiveContent();
}

void ContainerSystemTray::enterEvent(QEvent*)
{
	if (Settings::showOnMouseIn())
		m_autoShowTimer->start(Settings::timeToShowOnMouseIn() * 100, true );
}

void ContainerSystemTray::leaveEvent(QEvent*)
{
	m_autoShowTimer->stop();
}

/** Container */

Container::Container(QWidget *parent, const char *name)
 : KMainWindow(parent, name != 0 ? name : "MainWindow"), m_passivePopup(0L), m_newBasketPopup(false), m_regionGrabber(0)
{
	Global::mainContainer = this; // FIXME: Needed for the uggly hack in Basket:: setupActions() :-/ And elsewhere too :-D

	DEBUG_WIN << "Baskets are loaded from " + Global::basketsFolder();

	m_baskets = new BasketTree(this);
	setCentralWidget(m_baskets);

	connect( m_baskets, SIGNAL(basketNumberChanged(int)), this, SLOT(basketNumberChanged(int)) );
	connect( m_baskets, SIGNAL(basketChanged()),          this, SLOT(basketChanged())          );

	setupActions();
	setupStatusBar();
	createGUI();
	setAutoSaveSettings(/*groupName=*/QString::fromLatin1("MainWindow"), /*saveWindowSize=*/false);
	InlineEditors::instance()->richTextToolBar()->hide();

	m_actShowToolbar->setChecked(   toolBar()->isShown()   );
	m_actShowStatusbar->setChecked( statusBar()->isShown() );

	Tag::loadTags(); // Tags should be ready before loading baskets, but tags need the mainContainer to be ready to create KActions!
	m_baskets->load(); // FIXME: Don't load, and let main.cpp do everything in the good order!

	// If no basket has been found, try to import from an older version,
	if (!m_baskets->firstListViewItem()) {
		QDir dir;
		dir.mkdir(Global::basketsFolder());
		if (FormatImporter::shouldImportBaskets()) {
			FormatImporter::importBaskets();
			m_baskets->load();
		}
		if (!m_baskets->firstListViewItem()) {
			// Create first basket:
			BasketFactory::newBasket(/*icon=*/"", /*name=*/i18n("General"), /*backgroundImage=*/"", /*backgroundColor=*/QColor(), /*textColor=*/QColor(), /*templateName=*/"1column", /*createIn=*/0);
		}
		// TODO: Create Welcome Baskets:
	}

	m_tryHideTimer = new QTimer(this);
	m_hideTimer    = new QTimer(this);
	connect( m_tryHideTimer, SIGNAL(timeout()), this, SLOT(timeoutTryHide()) );
	connect( m_hideTimer,    SIGNAL(timeout()), this, SLOT(timeoutHide())    );
}

Container::~Container()
{
	if (currentBasket() && currentBasket()->isDuringEdit())
		currentBasket()->closeEditor();
	Global::mainContainer = 0;
	delete m_colorPicker;
}

void Container::countsChanged(Basket *basket)
{
	if (basket == currentBasket())
		countSelectedsChanged();
}

void Container::activatedTagShortcut()
{
	Tag *tag = Tag::tagForKAction((KAction*)sender());
	currentBasket()->activatedTagShortcut(tag);
}

void Container::setupStatusBar()
{
	statusBar()->show();
	statusBar()->setSizeGripEnabled(true);

	m_basketStatus = new QLabel(this);
	m_basketStatus->setSizePolicy( QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored, 0, 0, false) );

	statusBar()->addWidget( m_basketStatus, 1, false ); // Fit all extra space and is hiddable

	m_selectionStatus = new QLabel(this);
	statusBar()->addWidget( m_selectionStatus, 0, true );

	m_lockStatus = new ClickableLabel(0/*this*/);
	m_lockStatus->setMinimumSize(18, 18);
	m_lockStatus->setAlignment(Qt::AlignCenter);
//	statusBar()->addWidget( m_lockStatus, 0, true );
	connect( m_lockStatus, SIGNAL(clicked()), this, SLOT(lockBasket()) );
}

void Container::setupActions()
{
	/** Basket : **************************************************************/

	actNewBasket        = new KAction( i18n("&New Basket..."), "filenew", KStdAccel::shortcut(KStdAccel::New),
	                                   this, SLOT(askNewBasket()), actionCollection(), "basket_new" );
	actNewSubBasket     = new KAction( i18n("New &Sub-Basket..."), "", "Ctrl+Shift+N",
	                                   this, SLOT(askNewSubBasket()), actionCollection(), "basket_new_sub" );
	actNewSiblingBasket = new KAction( i18n("New Si&bling Basket..."), "", "",
	                                   this, SLOT(askNewSiblingBasket()), actionCollection(), "basket_new_sibling" );

	KActionMenu *newBasketMenu = new KActionMenu(i18n("&New"), "filenew", actionCollection(), "basket_new_menu");
	newBasketMenu->insert(actNewBasket);
	newBasketMenu->insert(actNewSubBasket);
	newBasketMenu->insert(actNewSiblingBasket);
	connect( newBasketMenu, SIGNAL(activated()), this, SLOT(askNewBasket()) );

	m_actPropBasket = new KAction( i18n("&Properties..."), "misc", "F2",
	                               this, SLOT(propBasket()), actionCollection(), "basket_properties" );
	m_actDelBasket  = new KAction( i18n("Remove Basket", "&Remove"), "", 0,
	                               this, SLOT(delBasket()), actionCollection(), "basket_remove" );
#ifdef HAVE_LIBGPGME
	m_actPassBasket = new KAction( i18n("Password protection", "&Password..."), "", 0,
								   this, SLOT(password()), actionCollection(), "basket_password" );
	m_actLockBasket = new KAction( i18n("Lock Basket", "&Lock"), "", 0,
								   this, SLOT(lockBasket()), actionCollection(), "basket_lock" );
#endif

	new KAction( i18n("&Export to HTML..."), "fileexport", 0,
	             this, SLOT(exportToHTML()),      actionCollection(), "basket_export_html" );
	new KAction( i18n("K&Notes"), "knotes", 0,
	             this, SLOT(importKNotes()),      actionCollection(), "basket_import_knotes" );
	new KAction( i18n("K&Jots"), "kjots", 0,
	             this, SLOT(importKJots()),       actionCollection(), "basket_import_kjots" );
	new KAction( i18n("&KnowIt..."), "knowit", 0,
	             this, SLOT(importKnowIt()),      actionCollection(), "basket_import_knowit" );
	new KAction( i18n("Tux&Cards..."), "tuxcards", 0,
	             this, SLOT(importTuxCards()),    actionCollection(), "basket_import_tuxcards" );
	new KAction( i18n("&Sticky Notes"), "gnome", 0,
	             this, SLOT(importStickyNotes()), actionCollection(), "basket_import_sticky_notes" );
	new KAction( i18n("&Tomboy"), "tintin", 0,
	             this, SLOT(importTomboy()),      actionCollection(), "basket_import_tomboy" );

	m_actHideWindow = new KAction( i18n("&Hide Window"), "", KStdAccel::shortcut(KStdAccel::Close),
	                               this, SLOT(hideOnEscape()), actionCollection(), "window_hide" );
	m_actHideWindow->setEnabled(Settings::useSystray()); // Init here !
	actQuit         = KStdAction::quit( this, SLOT(askForQuit()), actionCollection() );

	/** Edit : ****************************************************************/

	//m_actUndo     = KStdAction::undo(  this, SLOT(undo()),                 actionCollection() );
	//m_actUndo->setEnabled(false); // Not yet implemented !
	//m_actRedo     = KStdAction::redo(  this, SLOT(redo()),                 actionCollection() );
	//m_actRedo->setEnabled(false); // Not yet implemented !

	m_actCutNote  = KStdAction::cut(   this, SLOT(cutNote()),               actionCollection() );
	m_actCopyNote = KStdAction::copy(  this, SLOT(copyNote()),              actionCollection() );
#if KDE_IS_VERSION( 3, 1, 90 ) // KDE 3.2.x
	m_actPaste = KStdAction::pasteText( this, SLOT(pasteInCurrentBasket()), actionCollection() );
#else
	m_actPaste = KStdAction::paste(     this, SLOT(pasteInCurrentBasket()), actionCollection() );
#endif
	m_actDelNote  = new KAction( i18n("D&elete"), "editdelete", "Delete",
	                            this, SLOT(delNote()), actionCollection(), "edit_delete" );

	m_actSelectAll = KStdAction::selectAll( this, SLOT( slotSelectAll() ), actionCollection() );
	m_actSelectAll->setStatusText( i18n( "Selects all notes" ) );
	m_actUnselectAll = new KAction( i18n( "U&nselect All" ), "", this, SLOT( slotUnselectAll() ),
	                                actionCollection(), "edit_unselect_all" );
	m_actUnselectAll->setStatusText( i18n( "Unselects all selected notes" ) );
	m_actInvertSelection = new KAction( i18n( "&Invert Selection" ), CTRL+Key_Asterisk,
	                                    this, SLOT( slotInvertSelection() ),
	                                    actionCollection(), "edit_invert_selection" );
	m_actInvertSelection->setStatusText( i18n( "Inverts the current selection of notes" ) );

	m_actShowFilter  = new KToggleAction( i18n("&Filter"), "filter", KStdAccel::shortcut(KStdAccel::Find),
	                                      actionCollection(), "edit_filter" );
	connect( m_actShowFilter, SIGNAL(toggled(bool)), this, SLOT(showHideFilterBar(bool)) );

	m_actFilterAllBaskets = new KToggleAction( i18n("Filter all &Baskets"), "find", "Ctrl+Shift+F",
	                                           actionCollection(), "edit_filter_all_baskets" );
	connect( m_actFilterAllBaskets, SIGNAL(toggled(bool)), Global::basketTree, SLOT(toggleFilterAllBaskets(bool)) );

	m_actResetFilter = new KAction( i18n( "&Reset Filter" ), "locationbar_erase", "Ctrl+R",
	                                this, SLOT( slotResetFilter() ), actionCollection(), "edit_filter_reset" );

	/** Go : ******************************************************************/

	m_actPreviousBasket = new KAction( i18n( "&Previous Basket" ), "up",      "Alt+Up",
	                                   m_baskets, SLOT(goToPreviousBasket()), actionCollection(), "go_basket_previous" );
	m_actNextBasket     = new KAction( i18n( "&Next Basket" ),     "down",    "Alt+Down",
	                                   m_baskets, SLOT(goToNextBasket()),     actionCollection(), "go_basket_next"     );
	m_actFoldBasket     = new KAction( i18n( "&Fold Basket" ),     "back",    "Alt+Left",
	                                   m_baskets, SLOT(foldBasket()),         actionCollection(), "go_basket_fold"     );
	m_actExpandBasket   = new KAction( i18n( "&Expand Basket" ),   "forward", "Alt+Right",
	                                   m_baskets, SLOT(expandBasket()),       actionCollection(), "go_basket_expand"   );

	/** Note : ****************************************************************/

	m_actEditNote         = new KAction( i18n("Verb; not Menu", "&Edit..."), "edit",   "Return",
	                                     this, SLOT(editNote()), actionCollection(), "note_edit" );

	m_actOpenNote         = KStdAction::open( this, SLOT(openNote()), actionCollection(), "note_open" );
	m_actOpenNote->setIcon("window_new");
	m_actOpenNote->setText(i18n("&Open"));
	m_actOpenNote->setShortcut("F9");

	m_actOpenNoteWith     = new KAction( i18n("Open &With..."), "", "Shift+F9",
	                                 this, SLOT(openNoteWith()), actionCollection(), "note_open_with" );
	m_actSaveNoteAs       = KStdAction::saveAs( this, SLOT(saveNoteAs()), actionCollection(), "note_save_to_file" );
	m_actSaveNoteAs->setIcon("");
	m_actSaveNoteAs->setText(i18n("&Save to File..."));
	m_actSaveNoteAs->setShortcut("F10");

	m_actGroup        = new KAction( i18n("&Group"),          "attach",     "Ctrl+G",
	                                 this, SLOT(noteGroup()),    actionCollection(), "note_group" );
	m_actUngroup      = new KAction( i18n("U&ngroup"),        "",           "Ctrl+Shift+G",
	                                 this, SLOT(noteUngroup()),  actionCollection(), "note_ungroup" );

	m_actMoveOnTop    = new KAction( i18n("Move on &Top"),    "2uparrow",   "Ctrl+Shift+Home",
	                                 this, SLOT(moveOnTop()),    actionCollection(), "note_move_top" );
	m_actMoveNoteUp   = new KAction( i18n("Move &Up"),        "1uparrow",   "Ctrl+Shift+Up",
	                                 this, SLOT(moveNoteUp()),   actionCollection(), "note_move_up" );
	m_actMoveNoteDown = new KAction( i18n("Move &Down"),      "1downarrow", "Ctrl+Shift+Down",
	                                 this, SLOT(moveNoteDown()), actionCollection(), "note_move_down" );
	m_actMoveOnBottom = new KAction( i18n("Move on &Bottom"), "2downarrow", "Ctrl+Shift+End",
	                                 this, SLOT(moveOnBottom()), actionCollection(), "note_move_bottom" );

	/** Insert : **************************************************************/

	QSignalMapper *insertEmptyMapper  = new QSignalMapper(this);
	QSignalMapper *insertWizardMapper = new QSignalMapper(this);
	connect( insertEmptyMapper,  SIGNAL(mapped(int)), this, SLOT(insertEmpty(int))  );
	connect( insertWizardMapper, SIGNAL(mapped(int)), this, SLOT(insertWizard(int)) );

	m_actInsertText   = new KAction( i18n("&Text"),      "text",     "Insert", actionCollection(), "insert_text"     );
	m_actInsertHtml   = new KAction( i18n("&Rich Text"), "html",     "Ctrl+H", actionCollection(), "insert_html"     );
	m_actInsertLink   = new KAction( i18n("&Link"),      "link",     "Ctrl+Y", actionCollection(), "insert_link"     );
	m_actInsertImage  = new KAction( i18n("&Image"),     "image",    "",       actionCollection(), "insert_image"    );
	m_actInsertColor  = new KAction( i18n("&Color"),     "colorset", "",       actionCollection(), "insert_color"    );
	m_actInsertLauncher=new KAction( i18n("L&auncher"),  "launch",   "",       actionCollection(), "insert_launcher" );

	m_actImportKMenu  = new KAction( i18n("Import Launcher from &KDE Menu..."), "kmenu",      "", actionCollection(), "insert_kmenu"     );
	m_actImportIcon   = new KAction( i18n("Im&port Icon..."),                   "icons",      "", actionCollection(), "insert_icon"      );
	m_actLoadFile     = new KAction( i18n("Load From &File..."),                "fileimport", "", actionCollection(), "insert_from_file" );

	connect( m_actInsertText,     SIGNAL(activated()), insertEmptyMapper, SLOT(map()) );
	connect( m_actInsertHtml,     SIGNAL(activated()), insertEmptyMapper, SLOT(map()) );
	connect( m_actInsertImage,    SIGNAL(activated()), insertEmptyMapper, SLOT(map()) );
	connect( m_actInsertLink,     SIGNAL(activated()), insertEmptyMapper, SLOT(map()) );
	connect( m_actInsertColor,    SIGNAL(activated()), insertEmptyMapper, SLOT(map()) );
	connect( m_actInsertLauncher, SIGNAL(activated()), insertEmptyMapper, SLOT(map()) );
	insertEmptyMapper->setMapping(m_actInsertText,     NoteType::Text    );
	insertEmptyMapper->setMapping(m_actInsertHtml,     NoteType::Html    );
	insertEmptyMapper->setMapping(m_actInsertImage,    NoteType::Image   );
	insertEmptyMapper->setMapping(m_actInsertLink,     NoteType::Link    );
	insertEmptyMapper->setMapping(m_actInsertColor,    NoteType::Color   );
	insertEmptyMapper->setMapping(m_actInsertLauncher, NoteType::Launcher);

	connect( m_actImportKMenu, SIGNAL(activated()), insertWizardMapper, SLOT(map()) );
	connect( m_actImportIcon,  SIGNAL(activated()), insertWizardMapper, SLOT(map()) );
	connect( m_actLoadFile,    SIGNAL(activated()), insertWizardMapper, SLOT(map()) );
	insertWizardMapper->setMapping(m_actImportKMenu,  1 );
	insertWizardMapper->setMapping(m_actImportIcon,   2 );
	insertWizardMapper->setMapping(m_actLoadFile,     3 );

	m_colorPicker = new DesktopColorPicker();
	m_actColorPicker = new KAction( i18n("C&olor from Screen"), "kcolorchooser", "",
	                                 this, SLOT(slotColorFromScreen()), actionCollection(), "insert_screen_color" );
	connect( m_colorPicker, SIGNAL(pickedColor(const QColor&)), this, SLOT(colorPicked(const QColor&)) );
	connect( m_colorPicker, SIGNAL(canceledPick()),             this, SLOT(colorPickingCanceled())     );

	m_actGrabScreenshot = new KAction( i18n("Grab Screen &Zone"), "ksnapshot", "",
	                                   this, SLOT(grabScreenshot()), actionCollection(), "insert_screen_capture" );
	//connect( m_actGrabScreenshot, SIGNAL(regionGrabbed(const QPixmap&)), this, SLOT(screenshotGrabbed(const QPixmap&)) );
	//connect( m_colorPicker, SIGNAL(canceledPick()),             this, SLOT(colorPickingCanceled())     );

	m_insertActions.append( m_actInsertText     );
	m_insertActions.append( m_actInsertHtml     );
	m_insertActions.append( m_actInsertLink     );
	m_insertActions.append( m_actInsertImage    );
	m_insertActions.append( m_actInsertColor    );
	m_insertActions.append( m_actImportKMenu    );
	m_insertActions.append( m_actInsertLauncher );
	m_insertActions.append( m_actImportIcon     );
	m_insertActions.append( m_actLoadFile       );
	m_insertActions.append( m_actColorPicker    );
	m_insertActions.append( m_actGrabScreenshot );

	/** Settings : ************************************************************/
	m_actShowToolbar   = KStdAction::showToolbar(   this, SLOT(toggleToolBar()),   actionCollection());
    m_actShowStatusbar = KStdAction::showStatusbar( this, SLOT(toggleStatusBar()), actionCollection());

	m_actShowToolbar->setCheckedState( KGuiItem(i18n("Hide &Toolbar")) );

	(void) KStdAction::keyBindings( this, SLOT(showShortcutsSettingsDialog()), actionCollection() );

	actConfigGlobalShortcuts = KStdAction::keyBindings(this, SLOT(showGlobalShortcutsSettingsDialog()),
	                                                   actionCollection(), "options_configure_global_keybinding");
	actConfigGlobalShortcuts->setText(i18n("Configure &Global Shortcuts..."));

	(void) KStdAction::configureToolbars(this, SLOT(configureToolbars()), actionCollection() );

	//KAction *actCfgNotifs = KStdAction::configureNotifications(this, SLOT(configureNotifications()), actionCollection() );
	//actCfgNotifs->setEnabled(false); // Not yet implemented !

	actAppConfig = KStdAction::preferences( this, SLOT(showSettingsDialog()), actionCollection() );

	InlineEditors::instance()->initToolBars(this);
}

QPopupMenu* Container::popupMenu(const QString &menuName)
{
	QPopupMenu *menu = (QPopupMenu *)factory()->container(menuName, this);
	if (menu == 0) {
		KStandardDirs stdDirs;
		KMessageBox::error( this, i18n(
			"<p><b>The file basketui.rc seems to not exist or is too old.<br>"
			"%1 cannot run without it and will stop.</b></p>"
			"<p>Please check your installation of %2.</p>"
			"<p>If you haven't administrator access to install the application "
			"system wide, you can copy the file basketui.rc from the installation "
			"archive to the folder <a href='file://%3'>%4</a>.</p>"
			"<p>In last ressort, if you are sure the application is well installed "
			"but you had a preview version of it, try to remove the "
			"file %5basketui.rc</p>")
				.arg(kapp->aboutData()->programName(), kapp->aboutData()->programName(),
				     stdDirs.saveLocation("data", "basket/")).arg(stdDirs.saveLocation("data", "basket/"), stdDirs.saveLocation("data", "basket/")),
			i18n("Ressource not Found"), KMessageBox::AllowLink );
		exit(1); // We SHOULD exit right now and abord everything because the caller except menu != 0 to not crash.
	}
	return menu;
}

// Redirected actions :

void Container::exportToHTML()              { currentBasket()->exportToHTML();         }
void Container::editNote()                  { currentBasket()->noteEdit();             }
void Container::cutNote()                   { currentBasket()->noteCut();              }
void Container::copyNote()                  { currentBasket()->noteCopy();             }
void Container::delNote()                   { currentBasket()->noteDelete();           }
void Container::openNote()                  { currentBasket()->noteOpen();             }
void Container::openNoteWith()              { currentBasket()->noteOpenWith();         }
void Container::saveNoteAs()                { currentBasket()->noteSaveAs();           }
void Container::noteGroup()                 { currentBasket()->noteGroup();            }
void Container::noteUngroup()               { currentBasket()->noteUngroup();          }
void Container::moveOnTop()                 { currentBasket()->noteMoveOnTop();        }
void Container::moveOnBottom()              { currentBasket()->noteMoveOnBottom();     }
void Container::moveNoteUp()                { currentBasket()->noteMoveNoteUp();       }
void Container::moveNoteDown()              { currentBasket()->noteMoveNoteDown();     }
void Container::slotSelectAll()             { currentBasket()->selectAll();            }
void Container::slotUnselectAll()           { currentBasket()->unselectAll();          }
void Container::slotInvertSelection()       { currentBasket()->invertSelection();      }
void Container::slotResetFilter()           { currentDecoratedBasket()->resetFilter(); }

void Container::importKJots()       { SoftwareImporters::importKJots();       }
void Container::importKNotes()      { SoftwareImporters::importKNotes();      }
void Container::importKnowIt()      { SoftwareImporters::importKnowIt();      }
void Container::importTuxCards()    { SoftwareImporters::importTuxCards();    }
void Container::importStickyNotes() { SoftwareImporters::importStickyNotes(); }
void Container::importTomboy()      { SoftwareImporters::importTomboy();      }

void Container::showHideFilterBar(bool show, bool switchFocus)
{
//	if (show != m_actShowFilter->isChecked())
//		m_actShowFilter->setChecked(show);
	m_actShowFilter->setChecked(currentDecoratedBasket()->filterData().isFiltering);

	currentDecoratedBasket()->setFilterBarShown(show, switchFocus);
	currentDecoratedBasket()->resetFilter();
}

void Container::insertEmpty(int type)
{
	if (currentBasket()->isLocked()) {
		showPassiveImpossible(i18n("Cannot add note."));
		return;
	}
	currentBasket()->insertEmptyNote(type);
}

void Container::insertWizard(int type)
{
	if (currentBasket()->isLocked()) {
		showPassiveImpossible(i18n("Cannot add note."));
		return;
	}
	currentBasket()->insertWizard(type);
}

// BEGIN Screen Grabbing: // FIXME

void Container::grabScreenshot(bool global)
{
	if (m_regionGrabber) {
		KWin::activateWindow(m_regionGrabber->winId());
		return;
	}

	// Delay before to take a screenshot because if we hide the main window OR the systray popup menu,
	// we should wait the windows below to be repainted!!!
	// A special case is where the action is triggered with the global keyboard shortcut.
	// In this case, global is true, and we don't wait.
	// In the future, if global is also defined for other cases, check for
	// enum KAction::ActivationReason { UnknownActivation, EmulatedActivation, AccelActivation, PopupMenuActivation, ToolBarActivation };
	int delay = (isActiveWindow() ? 500 : (global/*kapp->activePopupWidget()*/ ? 0 : 200));

	m_colorPickWasGlobal = global;
	if (isActiveWindow()) {
		hide();
		m_colorPickWasShown = true;
	} else
		m_colorPickWasShown = false;

	currentBasket()->saveInsertionData();
	m_regionGrabber = new RegionGrabber(delay);
	connect( m_regionGrabber, SIGNAL(regionGrabbed(const QPixmap&)), this, SLOT(screenshotGrabbed(const QPixmap&)) );
}

void Container::grabScreenshotGlobal()
{
	grabScreenshot(true);
}

void Container::screenshotGrabbed(const QPixmap &pixmap)
{
	delete m_regionGrabber;
	m_regionGrabber = 0;

	// Cancelled (pressed Escape):
	if (pixmap.isNull()) {
		if (m_colorPickWasShown)
			show();
		return;
	}

	if (!currentBasket()->isLoaded()) {
		showPassiveLoading(currentBasket());
		currentBasket()->load();
	}
	currentBasket()->insertImage(pixmap);

	if (m_colorPickWasShown)
		show();

	if (Settings::usePassivePopup())
		showPassiveDropped(i18n("Grabbed screen zone to basket <i>%1</i>"));
}

// BEGIN Color picker (code from KColorEdit):

/* Activate the mode
 */
void Container::slotColorFromScreen(bool global)
{
	m_colorPickWasGlobal = global;
	if (isActiveWindow()) {
		hide();
		m_colorPickWasShown = true;
	} else
		m_colorPickWasShown = false;

	currentBasket()->saveInsertionData();
	m_colorPicker->pickColor();

/*	m_gettingColorFromScreen = true;
	kapp->processEvents();
	QTimer::singleShot( 100, this, SLOT(grabColorFromScreen()) );*/
}

void Container::slotColorFromScreenGlobal()
{
	slotColorFromScreen(true);
}

void Container::colorPicked(const QColor &color)
{
	if (!currentBasket()->isLoaded()) {
		showPassiveLoading(currentBasket());
		currentBasket()->load();
	}
	currentBasket()->insertColor(color);

	if (m_colorPickWasShown)
		show();

	if (Settings::usePassivePopup())
		showPassiveDropped(i18n("Picked color to basket <i>%1</i>"));
}

void Container::colorPickingCanceled()
{
	if (m_colorPickWasShown)
		show();
}

void Container::toggleToolBar()
{
	if (toolBar()->isVisible())
		toolBar()->hide();
	else
		toolBar()->show();

	saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
}

void Container::toggleStatusBar()
{
	if (statusBar()->isVisible())
		statusBar()->hide();
	else
		statusBar()->show();

	saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
}

void Container::configureToolbars()
{
	saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );

	KEditToolbar dlg(actionCollection());
	connect( &dlg, SIGNAL(newToolbarConfig()), this, SLOT(slotNewToolbarConfig()) );
	dlg.exec();
}

void Container::configureNotifications()
{
	// TODO
	// KNotifyDialog *dialog = new KNotifyDialog(this, "KNotifyDialog", false);
	// dialog->show();
}

void Container::slotNewToolbarConfig() // This is called when OK or Apply is clicked
{
	// ...if you use any action list, use plugActionList on each here...
	createGUI();
	plugActionList( QString::fromLatin1("go_baskets_list"), actBasketsList);
	applyMainWindowSettings( KGlobal::config(), autoSaveGroup() );
}

void Container::postStatusbarMessage(const QString& text)
{
    statusBar()->message(text, 2000);
}

void Container::setStatusBarHint(const QString &hint)
{
	if (hint.isEmpty())
		updateStatusBarHint();
	else if (m_basketStatus->text() != hint) // Avoid flicker
		m_basketStatus->setText(hint);
}

void Container::updateStatusBarHint()
{
	QString message = "";

	if (currentBasket()->isDuringDrag())
		message = i18n("Ctrl+drop: copy, Shift+drop: move, Shift+Ctrl+drop: link.");
// Too much noise information:
//	else if (currentBasket()->inserterShown() && currentBasket()->inserterSplit() && !currentBasket()->inserterGroup())
//		message = i18n("Click to insert a note, right click for more options. Click on the right of the line to group instead of insert.");
//	else if (currentBasket()->inserterShown() && currentBasket()->inserterSplit() && currentBasket()->inserterGroup())
//		message = i18n("Click to group a note, right click for more options. Click on the left of the line to group instead of insert.");
	else if (Global::debugWindow)
		message = "DEBUG: " + currentBasket()->folderName();

	if (message != m_basketStatus->text()) // Avoid flicker
		m_basketStatus->setText(message);
}

Basket* Container::basketForFolderName(const QString &/*folderName*/)
{
/*	QPtrList<Basket> basketsList = listBaskets();
	Basket *basket;
	for (basket = basketsList.first(); basket; basket = basketsList.next())
		if (basket->folderName() == folderName)
			return basket;
*/
	return 0;
}

DecoratedBasket* Container::currentDecoratedBasket()
{
	if (m_baskets->currentBasket())
		return m_baskets->currentBasket()->decoration();
	else
		return 0;
}

void Container::setFiltering(bool filtering)
{
	m_actShowFilter->setChecked(filtering);
	m_actResetFilter->setEnabled(filtering);
}

void Container::showAppPurpose()
{
	QWhatsThis::display(i18n(
		"<p>%1 let you to collect a wide variety of objects and keep them all in one place.</p>"
		"<p>You can group things for different purposes in tabs by creating as many baskets as you want.<br>"
		"Notes can be re-arranged, tags can be associated to, and you can drag them back to other "
		"applications when needed.</p>"
		"<p>You can use baskets to take notes, clean up your desktop, replace your bookmarks, store links to "
		"applications you often use, create shop-lists...</p>").arg(KGlobal::instance()->aboutData()->programName()));
}

void Container::undo()
{
	// TODO
}

void Container::redo()
{
	// TODO
}

void Container::pasteToBasket(int /*index*/, QClipboard::Mode /*mode*/)
{
	//TODO: REMOVE!
	//basketAt(index)->pasteNote(mode);
}

void Container::propBasket()
{
	BasketPropertiesDialog dialog(currentBasket(), this);
	dialog.exec();
}

void Container::delBasket()
{
//	DecoratedBasket *decoBasket    = currentDecoratedBasket();
	Basket          *basket        = currentBasket();

#if 0
	KDialogBase *dialog = new KDialogBase(this, /*name=*/0, /*modal=*/true, /*caption=*/i18n("Delete Basket"),
										  KDialogBase::User1 | KDialogBase::User2 | KDialogBase::No, KDialogBase::User1,
										 /*separator=*/false,
										 /*user1=*/KGuiItem(i18n("Delete Only that Basket")/*, icon=""*/),
										 /*user2=*/KGuiItem(i18n("Delete With its Childs")/*, icon=""*/) );
	QStringList basketsList;
	basketsList.append("Basket 1");
	basketsList.append("  Basket 2");
	basketsList.append("    Basket 3");
	basketsList.append("  Basket 4");
	KMessageBox::createKMessageBox(
			dialog, QMessageBox::Information,
			i18n("<qt>Do you really want to remove <b>%1</b> and its contents?</qt>")
				.arg(Tools::textToHTMLWithoutP(basket->basketName())),
			basketsList, /*ask=*/"", /*checkboxReturn=*/0, /*options=*/KMessageBox::Notify/*, const QString &details=QString::null*/);
#endif

	int really = KMessageBox::questionYesNo( this,
		i18n("<qt>Do you really want to remove <b>%1</b> and its contents?</qt>")
			.arg(Tools::textToHTMLWithoutP(basket->basketName())),
		i18n("Remove Basket")
#if KDE_IS_VERSION( 3, 2, 90 ) // KDE 3.3.x
		, KGuiItem(i18n("&Remove Basket"), "editdelete"), KStdGuiItem::cancel());
#else
		                    );
#endif

	if (really == KMessageBox::No)
		return;

	QStringList basketsList = Global::basketTree->listViewItemForBasket(basket)->childNamesTree();
	if (basketsList.count() > 0) {
		int deleteChilds = KMessageBox::questionYesNoList( this,
			i18n("<qt><b>%1</b> have the following child baskets.<br>Do you want to remove them too?</qt>")
				.arg(Tools::textToHTMLWithoutP(basket->basketName())),
			basketsList,
			i18n("Remove Child Baskets")
#if KDE_IS_VERSION( 3, 2, 90 ) // KDE 3.3.x
			, KGuiItem(i18n("&Remove Child Baskets"), "editdelete"));
#else
		);
#endif

		if (deleteChilds == KMessageBox::No)
			Global::basketTree->listViewItemForBasket(basket)->moveChildsBaskets();
	}

	doBasketDeletion(basket);

//	basketNumberChanged();
//	rebuildBasketsMenu();
}

void Container::doBasketDeletion(Basket *basket)
{
	QListViewItem *basketItem = Global::basketTree->listViewItemForBasket(basket);
	QListViewItem *nextOne;
	for (QListViewItem *child = basketItem->firstChild(); child; child = nextOne) {
		nextOne = child->nextSibling();
		// First delete the child baskets:
		doBasketDeletion(((BasketListViewItem*)child)->basket());
	}
	// Then, basket have no child anymore, delete it:
	DecoratedBasket *decoBasket = basket->decoration();
	basket->deleteFiles();
	Global::basketTree->removeBasket(basket);
	delete decoBasket;
//	delete basket;
}

void Container::password()
{
#ifdef HAVE_LIBGPGME
	int result = KMessageBox::warningContinueCancel(0, "THIS FEATURE IS STILL YOUNG AND TERRIBLY BUGGY. USE IT AT YOUR OWN RISK. AT THE MOMENT, IT MAY RESULT IN DATA LOSS!", "Buggy Feature");
	if (result == KMessageBox::Cancel)
		return;

	PasswordDlg dlg(this, "Password");
	Basket *cur = currentBasket();

	dlg.setType(cur->encryptionType());
	dlg.setKey(cur->encryptionKey());
	if(dlg.exec())
		cur->setProtection(dlg.type(), dlg.key());
#endif
}

void Container::lockBasket()
{
#ifdef HAVE_LIBGPGME
	Basket *cur = currentBasket();

	cur->lock();
#endif
}

void Container::showSettingsDialog()
{
	SettingsDialog *appS = new SettingsDialog(this);
	appS->exec();
	delete appS;
}

void Container::showShortcutsSettingsDialog()
{
	KKeyDialog::configure(actionCollection(), "basketui.rc");
	//.setCaption(..)
	//actionCollection()->writeSettings();
}

void Container::showGlobalShortcutsSettingsDialog()
{
	KKeyDialog::configure(Global::globalAccel);
	//.setCaption(..)
	Global::globalAccel->writeSettings();
}

void Container::changedSelectedNotes()
{
//	tabChanged(0); // FIXME: NOT OPTIMIZED
}

/*void Container::areSelectedNotesCheckedChanged(bool checked)
{
	m_actCheckNotes->setChecked(checked && currentBasket()->showCheckBoxes());
}*/

void Container::basketNumberChanged(int number)
{
	m_actPreviousBasket->setEnabled(number > 1);
	m_actNextBasket    ->setEnabled(number > 1);
}

void Container::basketChanged()
{
	m_actFoldBasket->setEnabled(Global::basketTree->canFold());
	m_actExpandBasket->setEnabled(Global::basketTree->canExpand());
	setFiltering(currentBasket() && currentBasket()->decoration()->filterData().isFiltering);
}

void Container::currentBasketChanged()
{
}

void Container::isLockedChanged()
{
	bool isLocked = currentBasket()->isLocked();

	if (isLocked) {
		m_lockStatus->setPixmap(SmallIcon("encrypted.png"));
		QToolTip::add(m_lockStatus, i18n(
			"<p>This basket is <b>locked</b>.<br>Click to unlock it.</p>").replace(" ", "&nbsp;") );
//		QToolTip::add(m_lockStatus, i18n("This basket is locked.\nClick to unlock it."));
	} else {
		m_lockStatus->clear();
		QToolTip::add(m_lockStatus, i18n(
			"<p>This basket is <b>unlocked</b>.<br>Click to lock it.</p>").replace(" ", "&nbsp;") );
//		QToolTip::add(m_lockStatus, i18n("This basket is unlocked.\nClick to lock it."));
	}

//	m_actLockBasket->setChecked(isLocked);
	m_actPropBasket->setEnabled(!isLocked);
	m_actDelBasket ->setEnabled(!isLocked);
	updateNotesActions();
}

void Container::countSelectedsChanged() // TODO: rename to countChanged() or notesStateChanged()..;
{
	Basket *basket = currentBasket();

	// Update statusbar message :
	if (!basket->isLoaded())
		m_selectionStatus->setText(i18n("Loading..."));
	else if (basket->count() == 0)
		m_selectionStatus->setText(i18n("No notes"));
	else {
		QString count     = i18n("%n note",     "%n notes",    basket->count()         );
		QString selecteds = i18n("%n selected", "%n selected", basket->countSelecteds());
		QString showns    = (currentDecoratedBasket()->filterData().isFiltering ? i18n("all matches") : i18n("no filter"));
		if (basket->countFounds() != basket->count())
			showns = i18n("%n match", "%n matches", basket->countFounds());
		m_selectionStatus->setText(
			i18n("e.g. '18 notes, 10 matches, 5 selected'", "%1, %2, %3").arg(count, showns, selecteds) );
	}

	// If we added a note that match the global filter, update the count number in the tree:
	if (Global::basketTree->isFilteringAllBaskets())
		Global::basketTree->listViewItemForBasket(basket)->listView()->triggerUpdate();

	if (currentBasket()->redirectEditActions()) {
		m_actSelectAll         ->setEnabled( !currentBasket()->selectedAllTextInEditor() );
		m_actUnselectAll       ->setEnabled( currentBasket()->hasSelectedTextInEditor()  );
	} else {
		m_actSelectAll         ->setEnabled( basket->countSelecteds() < basket->countFounds() );
		m_actUnselectAll       ->setEnabled( basket->countSelecteds() > 0                     );
	}
	m_actInvertSelection   ->setEnabled( basket->countFounds() > 0 );

	updateNotesActions();
}

void Container::updateNotesActions()
{
	bool isLocked             = currentBasket()->isLocked();
	bool oneSelected          = currentBasket()->countSelecteds() == 1;
	bool oneOrSeveralSelected = currentBasket()->countSelecteds() >= 1;
	bool severalSelected      = currentBasket()->countSelecteds() >= 2;

	// FIXME: m_actCheckNotes is also modified in void Container::areSelectedNotesCheckedChanged(bool checked)
	//        bool Basket::areSelectedNotesChecked() should return false if bool Basket::showCheckBoxes() is false
//	m_actCheckNotes->setChecked( oneOrSeveralSelected &&
//	                             currentBasket()->areSelectedNotesChecked() &&
//	                             currentBasket()->showCheckBoxes()             );

	m_actEditNote            ->setEnabled( !isLocked && oneSelected && !currentBasket()->isDuringEdit() );
	if (currentBasket()->redirectEditActions()) {
		m_actCutNote         ->setEnabled( currentBasket()->hasSelectedTextInEditor() );
		m_actCopyNote        ->setEnabled( currentBasket()->hasSelectedTextInEditor() );
		m_actPaste           ->setEnabled( true                                       );
		m_actDelNote         ->setEnabled( currentBasket()->hasSelectedTextInEditor() );
	} else {
		m_actCutNote         ->setEnabled( !isLocked && oneOrSeveralSelected );
		m_actCopyNote        ->setEnabled(              oneOrSeveralSelected );
		m_actPaste           ->setEnabled( !isLocked                         );
		m_actDelNote         ->setEnabled( !isLocked && oneOrSeveralSelected );
	}
	m_actOpenNote        ->setEnabled(              oneOrSeveralSelected );
	m_actOpenNoteWith    ->setEnabled(              oneSelected          ); // TODO: oneOrSeveralSelected IF SAME TYPE
	m_actSaveNoteAs      ->setEnabled(              oneSelected          ); // IDEM?
	m_actGroup           ->setEnabled( !isLocked && severalSelected && !currentBasket()->selectionIsOneGroup() );
	m_actUngroup         ->setEnabled( !isLocked && oneSelected          );
	m_actMoveOnTop       ->setEnabled( !isLocked && oneOrSeveralSelected );
	m_actMoveNoteUp      ->setEnabled( !isLocked && oneOrSeveralSelected );
	m_actMoveNoteDown    ->setEnabled( !isLocked && oneOrSeveralSelected );
	m_actMoveOnBottom    ->setEnabled( !isLocked && oneOrSeveralSelected );

	for (KAction *action = m_insertActions.first(); action; action = m_insertActions.next())
		action->setEnabled( !isLocked );

	// From the old Note::contextMenuEvent(...) :
/*	if (useFile() || m_type == Link) {
		m_type == Link ? i18n("&Open target")         : i18n("&Open")
		m_type == Link ? i18n("Open target &with...") : i18n("Open &with...")
		m_type == Link ? i18n("&Save target as...")   : i18n("&Save a copy as...")
		// If useFile() theire is always a file to open / open with / save, but :
		if (m_type == Link) {
			if (url().prettyURL().isEmpty() && runCommand().isEmpty())     // no URL nor runCommand :
				popupMenu->setItemEnabled(7, false);                       //  no possible Open !
			if (url().prettyURL().isEmpty())                               // no URL :
				popupMenu->setItemEnabled(8, false);                       //  no possible Open with !
			if (url().prettyURL().isEmpty() || url().path().endsWith("/")) // no URL or target a folder :
				popupMenu->setItemEnabled(9, false);                       //  not possible to save target file
		}
	} else if (m_type != Color) {
		popupMenu->insertSeparator();
		popupMenu->insertItem( SmallIconSet("filesaveas"), i18n("&Save a copy as..."), this, SLOT(slotSaveAs()), 0, 10 );
	}*/
}

void Container::askNewBasket()
{
	askNewBasket(0, 0);
}

void Container::askNewBasket(Basket *parent, Basket *pickProperties)
{
	NewBasketDefaultProperties properties;
	if (pickProperties) {
		properties.icon            = pickProperties->icon();
		properties.backgroundImage = pickProperties->backgroundImageName();
		properties.backgroundColor = pickProperties->backgroundColorSetting();
		properties.textColor       = pickProperties->textColorSetting();
		properties.freeLayout      = pickProperties->isFreeLayout();
		properties.columnCount     = pickProperties->columnsCount();
	}

	NewBasketDialog(parent, properties, this).exec();
}

void Container::askNewSubBasket()
{
	askNewBasket( /*parent=*/Global::basketTree->currentBasket(), /*pickPropertiesOf=*/Global::basketTree->currentBasket() );
}

void Container::askNewSiblingBasket()
{
	askNewBasket( /*parent=*/Global::basketTree->parentBasketOf(currentBasket()), /*pickPropertiesOf=*/currentBasket() );
}

Basket* Container::currentBasket()
{
	DecoratedBasket *decoration = currentDecoratedBasket();
	if (decoration)
		return decoration->basket();
	else
		return 0;
}

void Container::setCurrentBasket(Basket *basket)
{
	m_baskets->setCurrentBasket(basket);
}

void Container::setActive(bool active)
{
//	std::cout << "Main Window Position: setActive(" << (active ? "true" : "false") << ")" << std::endl;
#if KDE_IS_VERSION( 3, 2, 90 )   // KDE 3.3.x
	if (active) {
		kapp->updateUserTimestamp(); // If "activate on mouse hovering systray", or "on drag throught systray"
		Global::tray->setActive();   //  FIXME: add this in the places it need
	} else
		Global::tray->setInactive();
#elif KDE_IS_VERSION( 3, 1, 90 ) // KDE 3.2.x
	// Code from Kopete (that seem to work, in waiting KSystemTray make puplic the toggleSHown) :
	if (active) {
		show();
		//raise() and show() should normaly deIconify the window. but it doesn't do here due
		// to a bug in Qt or in KDE  (qt3.1.x or KDE 3.1.x) then, i have to call KWin's method
		if (isMinimized())
			KWin::deIconifyWindow(winId());

		if ( ! KWin::windowInfo(winId(), NET::WMDesktop).onAllDesktops() )
			KWin::setOnDesktop(winId(), KWin::currentDesktop());
		raise();
		// Code from me: expected and correct behavviour:
		kapp->updateUserTimestamp(); // If "activate on mouse hovering systray", or "on drag throught systray"
		KWin::activateWindow(winId());
	} else
		hide();
#else                            // KDE 3.1.x and lower
	if (active) {
		if (isMinimized())
			hide();        // If minimized, show() doesn't work !
		show();            // Show it
//		showNormal();      // If it was minimized
		raise();           // Raise it on top
		setActiveWindow(); // And set it the active window
	} else
		hide();
#endif
}

void Container::changeActive()
{
#if KDE_IS_VERSION( 3, 2, 90 ) // KDE 3.3.x
	kapp->updateUserTimestamp(); // If "activate on mouse hovering systray", or "on drag throught systray"
	Global::tray->toggleActive();
#else
	setActive( ! isActiveWindow() );
#endif
}

void Container::polish()
{
	bool shouldSave = false;

	// If position and size has never been set, set nice ones:
	//  - Set size to sizeHint()
	//  - Keep the window manager placing the window where it want and save this
	if (Settings::mainWindowSize().isEmpty()) {
//		std::cout << "Main Window Position: Initial Set in show()" << std::endl;
		int defaultWidth  = kapp->desktop()->width()  * 5 / 6;
		int defaultHeight = kapp->desktop()->height() * 5 / 6;
		resize(defaultWidth, defaultHeight); // sizeHint() is bad (too small) and we want the user to have a good default area size
		shouldSave = true;
	} else {
//		std::cout << "Main Window Position: Recall in show(x="
//		          << Settings::mainWindowPosition().x() << ", y=" << Settings::mainWindowPosition().y()
//		          << ", width=" << Settings::mainWindowSize().width() << ", height=" << Settings::mainWindowSize().height()
//		          << ")" << std::endl;
		move(Settings::mainWindowPosition());
		resize(Settings::mainWindowSize());
	}

	KMainWindow::polish();

	if (shouldSave) {
//		std::cout << "Main Window Position: Save size and position in show(x="
//		          << pos().x() << ", y=" << pos().y()
//		          << ", width=" << size().width() << ", height=" << size().height()
//		          << ")" << std::endl;
		Settings::setMainWindowPosition(pos());
		Settings::setMainWindowSize(size());
		Settings::saveConfig();
	}
}

void Container::resizeEvent(QResizeEvent*)
{
//	std::cout << "Main Window Position: Save size in resizeEvent(width=" << size().width() << ", height=" << size().height() << ") ; isMaximized="
//	          << (isMaximized() ? "true" : "false") << std::endl;
	Settings::setMainWindowSize(size());
	Settings::saveConfig();
}

void Container::moveEvent(QMoveEvent*)
{
//	std::cout << "Main Window Position: Save position in moveEvent(x=" << pos().x() << ", y=" << pos().y() << ")" << std::endl;
	Settings::setMainWindowPosition(pos());
	Settings::saveConfig();
}

bool Container::askForQuit()
{
	QString message = i18n("<p>Do you really want to quit %1?</p>").arg(kapp->aboutData()->programName());
	if (Settings::useSystray())
		message += i18n("<p>Notice that you do not have to quit the application before ending your KDE session: "
		                "it will be reloaded the next time you log in.</p>");

	int really = KMessageBox::warningContinueCancel( this, message, i18n("Quit Confirm"),
		KStdGuiItem::quit(), "confirmQuitAsking" );

	if (really == KMessageBox::Cancel)
		return false;

	kapp->quit();
	return true;
}

bool Container::queryExit()
{
	hide();
	return true;
}

#include <qdesktopwidget.h>
#include <qmime.h>
#include <qpainter.h>
// To know the program name:
#include <kglobal.h>
#include <kinstance.h>
#include <kaboutdata.h>

bool Container::queryClose()
{
/*	if (m_shuttingDown) // Set in askForQuit(): we don't have to ask again
		return true;*/

	if (kapp->sessionSaving()) {
		Settings::setStartDocked(false); // If queryClose() is called it's because the window is shown
		Settings::saveConfig();
		return true;
	}

	if (Settings::useSystray()) {
		Global::tray->displayCloseMessage(i18n("Basket"));
		hide();
		return false;
	} else
		return askForQuit();
}

void Container::hideOnEscape()
{
	if (Settings::useSystray())
		setActive(false);
}

/** Scenario of "Hide main window to system tray icon when mouse move out of the window" :
  * - At enterEvent() we stop m_tryHideTimer
  * - After that and before next, we are SURE cursor is hovering window
  * - At leaveEvent() we restart m_tryHideTimer
  * - Every 'x' ms, timeoutTryHide() seek if cursor hover a widget of the application or not
  * - If yes, we musn't hide the window
  * - But if not, we start m_hideTimer to hide main window after a configured elapsed time
  * - timeoutTryHide() continue to be called and if cursor move again to one widget of the app, m_hideTimer is stopped
  * - If after the configured time cursor hasn't go back to a widget of the application, timeoutHide() is called
  * - It then hide the main window to systray icon
  * - When the user will show it, enterEvent() will be called the first time he enter mouse to it
  * - ...
  */

/** Why do as this ? Problems with the use of only enterEvent() and leaveEvent() :
  * - Resize window or hover titlebar isn't possible : leave/enterEvent
  *   are
  *   > Use the grip or Alt+rightDND to resize window
  *   > Use Alt+DND to move window
  * - Each menu trigger the leavEvent
  */

void Container::enterEvent(QEvent*)
{
	m_tryHideTimer->stop();
	m_hideTimer->stop();
}

void Container::leaveEvent(QEvent*)
{
	if (Settings::useSystray() && Settings::hideOnMouseOut())
		m_tryHideTimer->start(50);
}

void Container::timeoutTryHide()
{
	// If a menu is displayed, do nothing for the moment
	if (kapp->activePopupWidget() != 0L)
		return;

	if (kapp->widgetAt(QCursor::pos()) != 0L)
		m_hideTimer->stop();
	else if ( ! m_hideTimer->isActive() ) // Start only one time
		m_hideTimer->start(Settings::timeToHideOnMouseOut() * 100, true);

	// If a sub-dialog is oppened, we musn't hide the main window:
	if (kapp->activeWindow() != 0L && kapp->activeWindow() != Global::mainContainer)
		m_hideTimer->stop();
}

void Container::timeoutHide()
{
	// We check that because the setting can have been set to off
	if (Settings::useSystray() && Settings::hideOnMouseOut())
		setActive(false);
	m_tryHideTimer->stop();
}

void Container::globalPasteInCurrentBasket()
{
	currentBasket()->setInsertPopupMenu();
	pasteInCurrentBasket();
	currentBasket()->cancelInsertPopupMenu();
}

void Container::pasteInCurrentBasket()
{
	currentBasket()->pasteNote();

	if (Settings::usePassivePopup())
		showPassiveDropped(i18n("Clipboard content pasted to basket <i>%1</i>"));
}

void Container::pasteSelInCurrentBasket()
{
	currentBasket()->pasteNote(QClipboard::Selection);

	if (Settings::usePassivePopup())
		showPassiveDropped(i18n("Selection pasted to basket <i>%1</i>"));
}

void Container::showPassiveDropped(const QString &title)
{
	if ( ! currentBasket()->isLocked() ) {
		// TODO: Keep basket, so that we show the message only if something was added to a NOT visible basket
		m_passiveDroppedTitle     = title;
		m_passiveDroppedSelection = currentBasket()->selectedNotes();
		QTimer::singleShot( c_delayTooltipTime, this, SLOT(showPassiveDroppedDelayed()) );
		// DELAY IT BELOW:
	} else
		showPassiveImpossible(i18n("No note was added."));
}

void Container::showPassiveDroppedDelayed()
{
	if (isActiveWindow())
		return;

	QString title = m_passiveDroppedTitle;

	delete m_passivePopup; // Delete previous one (if exists): it will then hide it (only one at a time)
	m_passivePopup = new KPassivePopup(Settings::useSystray() ? (QWidget*)Global::tray : (QWidget*)Global::mainContainer);
	QPixmap contentsPixmap = NoteDrag::feedbackPixmap(m_passiveDroppedSelection);
	QMimeSourceFactory::defaultFactory()->setPixmap("_passivepopup_image_", contentsPixmap);
	m_passivePopup->setView(
		title.arg(Tools::textToHTMLWithoutP(currentBasket()->basketName())),
		(contentsPixmap.isNull() ? "" : "<img src=\"_passivepopup_image_\">"),
		kapp->iconLoader()->loadIcon(currentBasket()->icon(), KIcon::NoGroup, 16, KIcon::DefaultState, 0L, true));
	m_passivePopup->show();
}

void Container::showPassiveImpossible(const QString &message)
{
	delete m_passivePopup; // Delete previous one (if exists): it will then hide it (only one at a time)
	m_passivePopup = new KPassivePopup(Settings::useSystray() ? (QWidget*)Global::tray : (QWidget*)Global::mainContainer);
	m_passivePopup->setView(
		QString("<font color=red>%1</font>")
			.arg(i18n("Basket <i>%1</i> is locked"))
			.arg(Tools::textToHTMLWithoutP(currentBasket()->basketName())),
		message,
		kapp->iconLoader()->loadIcon(currentBasket()->icon(), KIcon::NoGroup, 16, KIcon::DefaultState, 0L, true));
	m_passivePopup->show();
}

void Container::showPassiveContentForced()
{
	showPassiveContent(/*forceShow=*/true);
}

void Container::showPassiveContent(bool forceShow/* = false*/)
{
	if (!forceShow && isActiveWindow())
		return;

	// FIXME: Duplicate code (2 times)
	QString message;

	delete m_passivePopup; // Delete previous one (if exists): it will then hide it (only one at a time)
	m_passivePopup = new KPassivePopup(Settings::useSystray() ? (QWidget*)Global::tray : (QWidget*)Global::mainContainer);
	m_passivePopup->setView(
		"<qt>" + kapp->makeStdCaption( currentBasket()->isLocked()
			? QString("%1 <font color=gray30>%2</font>")
				.arg(Tools::textToHTMLWithoutP(currentBasket()->basketName()), i18n("(Locked)"))
			: Tools::textToHTMLWithoutP(currentBasket()->basketName()) ),
		message,
		kapp->iconLoader()->loadIcon(currentBasket()->icon(), KIcon::NoGroup, 16, KIcon::DefaultState, 0L, true));
	m_passivePopup->show();
}

void Container::showPassiveLoading(Basket *basket)
{
	if (isActiveWindow())
		return;

	delete m_passivePopup; // Delete previous one (if exists): it will then hide it (only one at a time)
	m_passivePopup = new KPassivePopup(Settings::useSystray() ? (QWidget*)Global::tray : (QWidget*)Global::mainContainer);
	m_passivePopup->setView(
		Tools::textToHTMLWithoutP(basket->basketName()),
		i18n("Loading..."),
		kapp->iconLoader()->loadIcon(basket->icon(), KIcon::NoGroup, 16, KIcon::DefaultState, 0L, true));
	m_passivePopup->show();
}

void Container::addNoteText()  { show(); currentBasket()->insertEmptyNote(NoteType::Text);  }
void Container::addNoteHtml()  { show(); currentBasket()->insertEmptyNote(NoteType::Html);  }
void Container::addNoteImage() { show(); currentBasket()->insertEmptyNote(NoteType::Image); }
void Container::addNoteLink()  { show(); currentBasket()->insertEmptyNote(NoteType::Link);  }
void Container::addNoteColor() { show(); currentBasket()->insertEmptyNote(NoteType::Color); }

#include "container.moc"

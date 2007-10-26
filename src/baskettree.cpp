/***************************************************************************
 *   Copyright (C) 2003 by S�astien Laot                                   *
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

#include "baskettree.h"

/** class BasketTreeItem: */

BasketTreeItem::BasketTreeItem ( QTreeWidgetItem *parent, Basket *basket )
		: QTreeWidgetItem ( parent ), m_basket ( basket )
		, m_isUnderDrag ( false )
		, m_isAbbreviated ( false )
{
// TO REMOVE	setAcceptDrops(true);
}

/* TO REMOVE
BasketTreeItem::BasketTreeItem(QTreeWidgetItem *parent, Basket *basket)
	: QListView(parent), m_basket(basket)
	, m_isUnderDrag(false)
	, m_isAbbreviated(false)
{
	setDropEnabled(true);
}
*/

BasketTreeItem::BasketTreeItem ( QTreeWidgetItem *parent, QTreeWidgetItem *after, Basket *basket )
		: QTreeWidgetItem ( parent, after ), m_basket ( basket )
		, m_isUnderDrag ( false )
		, m_isAbbreviated ( false )
{
//	setDropEnabled(true);
}

/* TO REMOVE
BasketTreeItem::BasketTreeItem(QTreeWidgetItem *parent, QTreeWidget *after, Basket *basket)
	: QListView(parent, after), m_basket(basket)
	, m_isUnderDrag(false)
	, m_isAbbreviated(false)
{
	setDropEnabled(true);
}
*/

BasketTreeItem::~BasketTreeItem()
{}

bool BasketTreeItem::acceptDrop ( const QMimeData * ) const
{
	std::cout << "accept" << std::endl;
	return true;
}

void BasketTreeItem::dropped ( QDropEvent *event )
{
	qDebug() << "Dropping into basket " << m_basket->objectName();
	m_basket->contentsDropEvent ( event );
	//Global::bnpView->currentBasket()->contentsDropEvent(event); // FIXME
}

int BasketTreeItem::width ( const QFontMetrics &/* fontMetrics */, const QTreeWidgetItem */*listView*/, int /* column */ ) const
{
	return treeWidget ()->visualItemRect ( this ).width() + 100;
	/*
		int BASKET_ICON_SIZE = 16;
		int MARGIN = 1;

		QRect textRect = fontMetrics.boundingRect(0, 0, / *width=* /1, 500000, Qt::AlignLeft | Qt::AlignTop | Qt::ShowPrefix, text(column));

		return MARGIN + BASKET_ICON_SIZE + MARGIN + textRect.width() +   BASKET_ICON_SIZE/2   + MARGIN;
	*/
}

QString BasketTreeItem::escapedName ( const QString &string )
{
	// Underlining the Alt+Letter shortcut (and escape all other '&' characters), if any:
	QString basketName = string;
	basketName.replace ( '&', "&&" ); // First escape all the amperstamp
	QString letter; // Find the letter
	QString altKey   = /*i18n(*/"Alt"/*)*/;   //i18n("The [Alt] key, as shown in shortcuts like Alt+C...", "Alt");
	QString shiftKey = /*i18n(*/"Shift"/*)*/; //i18n("The [Shift] key, as shown in shortcuts like Alt+Shift+1...", "Shift");
	QRegExp altLetterExp ( QString ( "^%1\\+(.)$" ).arg ( altKey ) );
	QRegExp altShiftLetterExp ( QString ( "^%1\\+%2\\+(.)$" ).arg ( altKey, shiftKey ) );
	if ( altLetterExp.indexIn ( m_basket->shortcut().toString() ) != -1 )
		letter = altLetterExp.cap ( 1 );
	if ( letter.isEmpty() && altShiftLetterExp.indexIn ( m_basket->shortcut().toString() ) != -1 )
		letter = altShiftLetterExp.cap ( 1 );
	if ( !letter.isEmpty() )
	{
		int index = basketName.count ( letter, Qt::CaseInsensitive );
		if ( index != -1 )
			basketName.insert ( index, '&' );
	}
	return basketName;
}

void BasketTreeItem::setup()
{
	int BASKET_ICON_SIZE = 16;
	int MARGIN = 1;

	setText ( 0, escapedName ( m_basket->basketName() ) );

//	widthChanged();
	QRect textRect = treeWidget()->fontMetrics().boundingRect ( 0, 0, /*width=*/1, 500000, Qt::AlignLeft | Qt::AlignTop | Qt::TextShowMnemonic, text ( 0 ) );

//FIXME 1.5	int height = MARGIN + qMax ( BASKET_ICON_SIZE, textRect.height() ) + MARGIN;
//FIXME 1.5	setHeight(height);

	setIcon ( 0,KIcon ( m_basket->icon() ) );
}

BasketTreeItem* BasketTreeItem::lastChild()
{
	if ( childCount() !=0 )
		return ( BasketTreeItem* ) child ( childCount ()-1 ) ;
	else
		return 0;
}

BasketTreeItem* BasketTreeItem::prevSibling()
{
	BasketTreeItem *item = this;
	int itemIndex=0;
	/* check if it's a topLevelItem
	   if not, get item index from parent
	   else get item index from widget
           if index is not null return item with index-1
	*/
	if(item->parent()!=0 )
	{
		 itemIndex=item->parent()->indexOfChild ( this );
		 if(itemIndex)
		 return (BasketTreeItem*) ( item->parent()->child(itemIndex-1));
	}
        else{
		 itemIndex=item->treeWidget()->indexOfTopLevelItem ( this );
		 if(itemIndex)
		 return ( BasketTreeItem* ) ( item->treeWidget()->topLevelItem(itemIndex-1));
	}
	return 0;
}

BasketTreeItem* BasketTreeItem::shownItemAbove()
{
	BasketTreeItem *item = ( BasketTreeItem* ) this->treeWidget()->itemAbove(this);
	while ( item )
	{
		if ( item->isVisible() )
			return item;
		item = ( BasketTreeItem* ) ( item->treeWidget()->itemAbove(item) );
	}
	return 0;
}

BasketTreeItem* BasketTreeItem::shownItemBelow()
{
	BasketTreeItem *item = ( BasketTreeItem* ) this->treeWidget()->itemBelow(this);
	while ( item )
	{
		if ( item->isVisible() )
			return item;
		item = ( BasketTreeItem* ) ( item->treeWidget()->itemBelow(item) );
	}
	return 0;
}

QStringList BasketTreeItem::childNamesTree ( int deep )
{
	QStringList result;
	for ( QTreeWidgetItem *child = this->child(0); child; child = this->child(this->indexOfChild(child)+1) )
	{
		BasketTreeItem *item = ( BasketTreeItem* ) child;
		// Compute indentation spaces:
		QString spaces;
		for ( int i = 0; i < deep; ++i )
			spaces += "  ";
		// Append the name:
		result.append ( spaces + item->basket()->basketName() );
		// Append the childs:
		if ( child->child(0) )
		{
			QStringList childs = item->childNamesTree ( deep + 1 );
			for ( QStringList::iterator it = childs.begin(); it != childs.end(); ++it )
				result.append ( *it );
		}
	}
	return result;
}

void BasketTreeItem::moveChildsBaskets()
{
	QTreeWidgetItem *insertAfterThis = this;
	QTreeWidgetItem *nextOne;
	for ( QTreeWidgetItem *child = this->child(0); child; child = nextOne )
	{
		nextOne = this->child(this->indexOfChild(child)+1);
		// Re-insert the item with the good parent:
		removeChild (child );
		if ( parent() )
			parent()->insertChild ( parent()->childCount(), child );
		else
			treeWidget()->addTopLevelItem ( child );
		// And move it at the good place:
//FIXME 1.5		child->moveItem ( insertAfterThis );
		insertAfterThis = child;
	}
}

void BasketTreeItem::ensureVisible()
{
	BasketTreeItem *item = this;
	while ( item->parent() )
	{
		item = ( BasketTreeItem* ) ( item->parent() );
		item->setExpanded ( true );
	}
}

bool BasketTreeItem::isVisible()
{
	QTreeWidgetItem *item = parent();
	while ( item )
	{
		if ( !item->isExpanded() )
			return false;
		item = item->parent();
	}
	return true;
}

bool BasketTreeItem::isCurrentBasket()
{
	return basket() == Global::bnpView->currentBasket();
}

// TODO: Move this function from item.cpp to class Tools:
extern void drawGradient ( QPainter *p, const QColor &colorTop, const QColor & colorBottom,
	                           int x, int y, int w, int h,
	                           bool sunken, bool horz, bool flat ); /*const*/

QPixmap BasketTreeItem::circledTextPixmap ( const QString &text, int height, const QFont &font, const QColor &color )
{
	QString key = QString ( "BLI-%1.%2.%3.%4" )
	              .arg ( text ).arg ( height ).arg ( font.toString() ).arg ( color.rgb() );
	if ( QPixmap* cached=QPixmapCache::find ( key ) )
	{
		return *cached;
	}

	// Compute the sizes of the image components:
	QRect textRect = QFontMetrics ( font ).boundingRect ( 0, 0, /*width=*/1, height, Qt::AlignLeft | Qt::AlignTop, text );
	int xMargin = height / 6;
	int width   = xMargin + textRect.width() + xMargin;

	// Create the gradient image:
	QPixmap gradient ( 3 * width, 3 * height ); // We double the size to be able to smooth scale down it (== antialiased curves)
	QPainter gradientPainter ( &gradient );
#if 1 // Enable the new look of the gradient:
	QColor topColor       = KColorScheme ( QPalette::Active, KColorScheme::Selection ).background().color().lighter ( 130 ); //120
	QColor topMidColor    = KColorScheme ( QPalette::Active, KColorScheme::Selection ).background().color().lighter ( 105 ); //105
	QColor bottomMidColor = KColorScheme ( QPalette::Active, KColorScheme::Selection ).background().color().darker ( 130 );  //120
	QColor bottomColor    = KColorScheme ( QPalette::Active, KColorScheme::Selection ).background().color();
	drawGradient ( &gradientPainter, topColor, topMidColor,
	               0, 0, gradient.width(), gradient.height() / 2, /*sunken=*/false, /*horz=*/true, /*flat=*/false );
	drawGradient ( &gradientPainter, bottomMidColor, bottomColor,
	               0, gradient.height() / 2, gradient.width(), gradient.height() - gradient.height() / 2, /*sunken=*/false, /*horz=*/true, /*flat=*/false );
	gradientPainter.fillRect ( 0, 0, gradient.width(), 3, KColorScheme ( QPalette::Active, KColorScheme::Selection ).background().color() );
#else
	drawGradient ( &gradientPainter, KColorScheme ( QPalette::Active, KColorScheme::Selection ).background().color(), KColorScheme ( QPalette::Active, KColorScheme::Selection ).background().color().darker(),
	               0, 0, gradient.width(), gradient.height(), /*sunken=*/false, /*horz=*/true, /*flat=*/false );
#endif
	gradientPainter.end();

	// Draw the curved rectangle:
	QBitmap curvedRectangle ( 3 * width, 3 * height );
	curvedRectangle.fill ( Qt::color0 );
	QPainter curvePainter ( &curvedRectangle );
	curvePainter.setPen ( Qt::color1 );
	curvePainter.setBrush ( Qt::color1 );
	curvePainter.setClipRect ( 0, 0, 3* ( height / 5 ), 3* ( height ) ); // If the width is small, don't fill the right part of the pixmap
	curvePainter.drawEllipse ( 0, 3* ( -height / 4 ), 3* ( height ), 3* ( height * 3 / 2 ) ); // Don't forget we double the sizes
	curvePainter.setClipRect ( 3* ( width - height / 5 ), 0, 3* ( height / 5 ), 3* ( height ) );
	curvePainter.drawEllipse ( 3* ( width - height ), 3* ( -height / 4 ), 3* ( height ), 3* ( height * 3 / 2 ) );
	curvePainter.setClipping ( false );
	curvePainter.fillRect ( 3* ( height / 6 ), 0, 3* ( width - 2 * height / 6 ), 3* ( height ), curvePainter.brush() );
	curvePainter.end();

	// Apply the curved rectangle as the mask of the gradient:
	gradient.setMask ( curvedRectangle );
	QImage resultImage = gradient.toImage();
//FIXME 1.5	resultImage.setAlphaBuffer ( true );

	// Scale down the image smoothly to get anti-aliasing:
	QPixmap pmScaled;
	pmScaled.fromImage ( resultImage.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

	// Draw the text, and return the result:
	QPainter painter ( &pmScaled );
	painter.setPen ( color );
	painter.setFont ( font );
	painter.drawText ( 0+1, 0, width, height, Qt::AlignHCenter | Qt::AlignVCenter, text );
	painter.end();

	QPixmapCache::insert ( key, pmScaled );

	return pmScaled;
}

QPixmap BasketTreeItem::foundCountPixmap ( bool isLoading, int countFound, bool childsAreLoading, int countChildsFound, const QFont &font, int height )
{
	if ( isLoading )
		return QPixmap();

	QFont boldFont ( font );
	boldFont.setBold ( true );

	QString text;
	if ( childsAreLoading )
	{
		if ( countChildsFound > 0 )
			text = i18n ( "%1+%2+" ).arg ( QString::number ( countFound ), QString::number ( countChildsFound ) );
		else
			text = i18n ( "%1+" ).arg ( QString::number ( countFound ) );
	}
	else
	{
		if ( countChildsFound > 0 )
			text = i18n ( "%1+%2" ).arg ( QString::number ( countFound ), QString::number ( countChildsFound ) );
		else if ( countFound > 0 )
			text = QString::number ( countFound );
		else
			return QPixmap();
	}

	return circledTextPixmap ( text, height, boldFont, KColorScheme ( QPalette::Active, KColorScheme::Selection ).foreground().color() );
}

bool BasketTreeItem::haveChildsLoading()
{
	QTreeWidgetItem *child = this->child(0);
	while ( child )
	{
		BasketTreeItem *childItem = ( BasketTreeItem* ) child;
		if ( !childItem->basket()->isLoaded() && !childItem->basket()->isLocked() )
			return true;
		if ( childItem->haveChildsLoading() )
			return true;
		child = this->child(this->indexOfChild(child)+1);
	}
	return false;
}

bool BasketTreeItem::haveHiddenChildsLoading()
{
	if ( isExpanded() )
		return false;
	return haveChildsLoading();
}

bool BasketTreeItem::haveChildsLocked()
{
	QTreeWidgetItem *child = this->child(0);
	while ( child )
	{
		BasketTreeItem *childItem = ( BasketTreeItem* ) child;
		if ( /*!*/childItem->basket()->isLocked() )
			return true;
		if ( childItem->haveChildsLocked() )
			return true;
		child = this->child(this->indexOfChild(child)+1);
	}
	return false;
}

bool BasketTreeItem::haveHiddenChildsLocked()
{
	if ( isExpanded() )
		return false;
	return haveChildsLocked();
}

int BasketTreeItem::countChildsFound()
{
	int count = 0;
	QTreeWidgetItem *child = this->child(0);
	while ( child )
	{
		BasketTreeItem *childItem = ( BasketTreeItem* ) child;
		count += childItem->basket()->countFounds();
		count += childItem->countChildsFound();
		child = this->child(this->indexOfChild(child)+1);
	}
	return count;
}

int BasketTreeItem::countHiddenChildsFound()
{
	if ( isExpanded() )
		return 0;
	return countChildsFound();
}

void BasketTreeItem::paintCell ( QPainter *painter, const QPalette &/*colorGroup*/, int /*column*/, int width, int /*align*/ )
{
	// Workaround a Qt bug:
	// When the splitter is moved to hide the tree view and then the application is restarted,
	// Qt try to draw items with a negative size!
	if ( width <= 0 )
	{
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
	if ( showCountPixmap )
	{
		showLoadingIcon = ( !m_basket->isLoaded() && !m_basket->isLocked() ) || haveHiddenChildsLoading();
		showEncryptedIcon = m_basket->isLocked() || haveHiddenChildsLocked();
		countPixmap = foundCountPixmap ( !m_basket->isLoaded(), m_basket->countFounds(), haveHiddenChildsLoading() || haveHiddenChildsLocked(),
		                                 countHiddenChildsFound(), treeWidget()->font(),  treeWidget()->visualItemRect (this).height() - 2 * MARGIN );
	}
	int effectiveWidth = width - ( countPixmap.isNull() ? 0 : countPixmap.width() + MARGIN )
	                     - ( showLoadingIcon || showEncryptedIcon ? BASKET_ICON_SIZE + MARGIN : 0 ) /*
	                     			- (showEncryptedIcon ? BASKET_ICON_SIZE + MARGIN : 0)*/;


	bool drawRoundRect = m_basket->backgroundColorSetting().isValid() || m_basket->textColorSetting().isValid();
	QColor textColor = ( drawRoundRect ? m_basket->textColor() : ( isCurrentBasket() ? KColorScheme ( QPalette::Active, KColorScheme::Selection ).foreground().color() : KColorScheme ( QPalette::Active, KColorScheme::View ).foreground().color() ) );

	BasketTreeItem *shownAbove = shownItemAbove();
	BasketTreeItem *shownBelow = shownItemBelow();

	// Don't forget to update the key computation if parameters
	// affecting the rendering logic change
	QString key = QString ( "BLVI::pC-%1.%2.%3.%4.%5.%6.%7.%8.%9.%10.%11.%12.%13.%14.%15" )
	              .arg ( effectiveWidth )
	              .arg ( drawRoundRect )
	              .arg ( textColor.rgb() )
	              .arg ( m_basket->backgroundColor().rgb() )
	              .arg ( isCurrentBasket() )
	              .arg ( shownBelow && shownBelow->isCurrentBasket() )
	              .arg ( shownAbove && shownAbove->isCurrentBasket() )
	              .arg ( showLoadingIcon )
	              .arg ( showEncryptedIcon )
	              .arg ( showCountPixmap )
	              .arg ( m_basket->countFounds() )
	              .arg ( countHiddenChildsFound() )
	              .arg ( m_isUnderDrag )
	              .arg ( m_basket->basketName() )
	              .arg ( m_basket->icon() );
	if ( QPixmap* cached = QPixmapCache::find ( key ) )
	{
		// Qt's documentation recommends copying the pointer
		// into a QPixmap immediately
		QPixmap cachedBuffer = *cached;
		painter->drawPixmap ( 0, 0, cachedBuffer );
		return;
	}

	// Bufferize the drawing of items (otherwize, resizing the splitter make the tree act like a Christmas Tree ;-D ):
	QPixmap theBuffer ( width,  treeWidget()->visualItemRect ( this).height() );
	QPainter thePainter ( &theBuffer );

	// Fill with the basket background color:
	QColor background = ( isCurrentBasket() ? KColorScheme ( QPalette::Active, KColorScheme::Selection ).background().color() : treeWidget()->palette().color(QPalette::Window) );
	thePainter.fillRect ( 0, 0, width, treeWidget()->visualItemRect ( this).height(), background );

	int textWidth = effectiveWidth - MARGIN - BASKET_ICON_SIZE - MARGIN - MARGIN;

	// Draw the rounded rectangle:
	if ( drawRoundRect )
	{
		QRect textRect = treeWidget()->fontMetrics().boundingRect ( 0, 0, /*width=*/1, 500000, Qt::AlignLeft | Qt::AlignTop | Qt::TextShowMnemonic, text ( /*column=*/0 ) );
		int xRound = MARGIN;
		int yRound = MARGIN;
		int hRound = treeWidget()->visualItemRect ( this).height() - 2 * MARGIN;
		int wRound = qMin ( BASKET_ICON_SIZE + MARGIN + textRect.width() + hRound/2,  effectiveWidth - MARGIN - MARGIN );
		if ( wRound > 0 )
		{ // Do not crash if there is no space anymore to draw the rounded rectangle:
			QPixmap buffer ( wRound * 2, hRound * 2 );
			buffer.fill ( background );
			QPainter pBuffer ( &buffer );
			QColor colorRound = m_basket->backgroundColor();
			pBuffer.setPen ( colorRound );
			pBuffer.setBrush ( colorRound );
			if ( wRound > hRound )
			{ // If the rectangle is smaller in width than in height, don't overlap ellipses...
				pBuffer.drawEllipse ( 0,                                 0, hRound * 2, hRound * 2 );
				pBuffer.drawEllipse ( wRound * 2 - hRound * 2, 0, hRound * 2, hRound * 2 );
				pBuffer.fillRect ( hRound*2/2, 0, wRound * 2 - hRound * 2, hRound * 2, colorRound );
			}
			else
				pBuffer.drawEllipse ( 0, 0, wRound * 2, hRound * 2 );
			pBuffer.end();
			QImage imageToScale = buffer.toImage();
			QPixmap pmScaled;
			pmScaled.fromImage ( imageToScale.scaled(xRound, yRound, Qt::IgnoreAspectRatio, Qt::SmoothTransformation) );
			thePainter.drawPixmap ( xRound, yRound, pmScaled );
			textWidth -= hRound/2;
		}
	}

	QColor bgColor  = treeWidget()->palette().color(QPalette::Window);
	QColor selColor = KColorScheme ( QPalette::Active, KColorScheme::Selection ).background().color();
	QColor midColor = Tools::mixColor ( bgColor, selColor );
	// Draw the left selection roundings:
	if ( isCurrentBasket() )
	{
		thePainter.setPen ( bgColor );
		thePainter.drawPoint ( 0, 0 );
		thePainter.drawPoint ( 1, 0 );
		thePainter.drawPoint ( 0, 1 );
		thePainter.drawPoint ( 0, treeWidget()->visualItemRect ( this).height() - 1 );
		thePainter.drawPoint ( 1, treeWidget()->visualItemRect ( this).height() - 1 );
		thePainter.drawPoint ( 0, treeWidget()->visualItemRect ( this).height() - 2 );
		thePainter.setPen ( midColor );
		thePainter.drawPoint ( 2, 0 );
		thePainter.drawPoint ( 0, 2 );
		thePainter.drawPoint ( 2, treeWidget()->visualItemRect ( this).height() - 1 );
		thePainter.drawPoint ( 0, treeWidget()->visualItemRect ( this).height() - 3 );
	}
	// Draw the bottom-right selection roundings:
	//BasketTreeItem *shownBelow = shownItemBelow();
	if ( shownBelow && shownBelow->isCurrentBasket() )
	{
		thePainter.setPen ( selColor );
		thePainter.drawPoint ( width - 1, treeWidget()->visualItemRect ( this).height() - 1 );
		thePainter.drawPoint ( width - 2, treeWidget()->visualItemRect ( this).height() - 1 );
		thePainter.drawPoint ( width - 1, treeWidget()->visualItemRect ( this).height() - 2 );
		thePainter.setPen ( midColor );
		thePainter.drawPoint ( width - 3, treeWidget()->visualItemRect ( this).height() - 1 );
		thePainter.drawPoint ( width - 1, treeWidget()->visualItemRect ( this).height() - 3 );
	}
	// Draw the top-right selection roundings:
	//	BasketTreeItem *shownAbove = shownItemAbove();
	if ( shownAbove && shownAbove->isCurrentBasket() )
	{
		thePainter.setPen ( selColor );
		thePainter.drawPoint ( width - 1, 0 );
		thePainter.drawPoint ( width - 2, 0 );
		thePainter.drawPoint ( width - 1, 1 );
		thePainter.setPen ( midColor );
		thePainter.drawPoint ( width - 3, 0 );
		thePainter.drawPoint ( width - 1, 2 );
	}

	// Draw the icon and text:
	int yPixmap = ( treeWidget()->visualItemRect ( this).height() - BASKET_ICON_SIZE ) / 2;
	thePainter.drawPixmap ( MARGIN, yPixmap, icon( /*column=*/0 ).pixmap(32,32) );
	thePainter.setPen ( textColor );
	if ( textWidth > 0 )
	{ // IF there is space left to draw the text:
		int xText = MARGIN + BASKET_ICON_SIZE + MARGIN;
		QString theText = m_basket->basketName();
		if ( painter->fontMetrics().width ( theText ) > textWidth )
		{
//FIXME 1.5			theText = KStringHandler::rPixelSqueeze ( theText, painter->fontMetrics(), textWidth );
			m_isAbbreviated = true;
		}
		else
		{
			m_isAbbreviated = false;
		}
		theText = escapedName ( theText );
		thePainter.drawText ( xText, 0, textWidth, treeWidget()->visualItemRect ( this).height(), Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic, theText );
	}

	// If we are filtering all baskets, and are effectively filtering on something:
	if ( !countPixmap.isNull() )
	{
		thePainter.drawPixmap ( effectiveWidth, 1, countPixmap );
		effectiveWidth += countPixmap.width() + MARGIN;
	}
	if ( showLoadingIcon )
	{
		QPixmap icon = KIcon( "edit-find").pixmap(32,32);
		thePainter.drawPixmap ( effectiveWidth, 0, icon );
		effectiveWidth += BASKET_ICON_SIZE + MARGIN;
	}
	if ( showEncryptedIcon && !showLoadingIcon )
	{
		QPixmap icon = KIcon( "encrypted").pixmap(32,32);
		thePainter.drawPixmap ( effectiveWidth, 0, icon );
	}

	if ( m_isUnderDrag )
	{
	//FIXME 1.5	thePainter.drawWinFocusRect ( 0, 0, width, treeWidget()->visualItemRect ( this).height() );
	}
	thePainter.end();

	QPixmapCache::insert ( key, theBuffer );
	// Apply the buffer:
	painter->drawPixmap ( 0, 0, theBuffer );
}

void BasketTreeItem::setUnderDrag ( bool underDrag )
{
	m_isUnderDrag = underDrag;
}

bool BasketTreeItem::isAbbreviated()
{
	return m_isAbbreviated;
}

/** class BasketListViewToolTip: */

// class BasketTreeListView_ToolTip : public QToolTip
// {
// 	public:
// 		BasketTreeListView_ToolTip ( BasketTreeListView* basketView )
// 				: QToolTip ( basketView->toolTip() )
// 				, m_basketView ( basketView )
// 		{}
// 	public:
// 		void maybeTip ( const QPoint& pos )
// 		{
// 			QTreeWidgetItem *item = m_basketView->itemAt (  pos  );
// 			BasketTreeItem* bitem = dynamic_cast<BasketTreeItem*> ( item );
// 			if ( bitem && bitem->isAbbreviated() )
// 			{
// 				tip ( m_basketView->visualItemRect ( bitem ), bitem->basket()->basketName() );
// 			}
// 		}
// 	private:
// 		BasketTreeListView* m_basketView;
// };

/** class BasketTreeListView: */

BasketTree::BasketTree ( QWidget *parent, const char *name )
		:  QTreeWidget ( parent )
		, m_autoOpenItem ( 0 )
		, m_itemUnderDrag ( 0 )
{
	setObjectName ( name );
/* FIXME 1.5	setWFlags ( Qt::WStaticContents | WNoAutoErase );
	clearWFlags ( Qt::WStaticContents | WNoAutoErase ); */
	//viewport()->clearWFlags(Qt::WStaticContents);
	connect ( &m_autoOpenTimer, SIGNAL ( timeout() ), this, SLOT ( autoOpen() ) );

//	new BasketTreeListView_ToolTip ( this );
}

void BasketTree::contentsDragEnterEvent ( QDragEnterEvent *event )
{
/* FIXME 1.5 
*/
	if ( event->provides ( "application/x-qlistviewitem" ) )
	{
		QTreeWidgetItem* it= this->topLevelItem(0) ; // TODO: Don't show expanders if it's not a basket drag...
		while ( it )
		{
			QTreeWidgetItem *item = it->child(0);
			if ( !item->child(0) )
			{
				item->setExpanded( true );
			}
			it=it->child(it->indexOfChild(item)+1);
		}
		update();
	}

	QTreeWidget::dragEnterEvent ( event );
}

void BasketTree::removeExpands()
{
// TODO
	/*	QListViewIterator it(this);
		while (it.current()) {
			QTreeWidgetItem *item = it.current();
			if (!item->firstChild())
				item->setExpandable(false);
			++it;
		}
		viewport()->update();
	*/
}

void BasketTree::contentsDragLeaveEvent ( QDragLeaveEvent *event )
{
	std::cout << "BasketTreeListView::contentsDragLeaveEvent" << std::endl;
	m_autoOpenItem = 0;
	m_autoOpenTimer.stop();
	setItemUnderDrag ( 0 );
	removeExpands();
	QTreeWidget::dragLeaveEvent ( event );
}

void BasketTree::contentsDropEvent ( QDropEvent *event )
{
	std::cout << "BasketTreeListView::contentsDropEvent()" << std::endl;
	if ( event->provides ( "application/x-qlistviewitem" ) )
	{
		QTreeWidget::dropEvent ( event );
	}
	else
	{
		std::cout << "Forwarding dropped data to the basket" << std::endl;
		QTreeWidgetItem *item = itemAt ( event->pos() );
		BasketTreeItem* bitem = dynamic_cast<BasketTreeItem*> ( item );
		if ( bitem )
		{
			bitem->basket()->blindDrop ( event );
		}
		else
		{
			std::cout << "Forwarding failed: no bitem found" << std::endl;
		}
	}

	m_autoOpenItem = 0;
	m_autoOpenTimer.stop();
	setItemUnderDrag ( 0 );
	removeExpands();

	Global::bnpView->save(); // TODO: Don't save if it was not a basket drop...
}

void BasketTree::contentsDragMoveEvent ( QDragMoveEvent *event )
{
	kDebug() << "Enter : " << (int)event << endl;

	if ( event->provides ( "application/x-qlistviewitem" ) )
		QTreeWidget::dragMoveEvent ( event );
	else
	{
		QTreeWidgetItem *item = itemAt ( event->pos() );
		BasketTreeItem* bitem = dynamic_cast<BasketTreeItem*> ( item );
		if ( m_autoOpenItem != item )
		{
			m_autoOpenItem = item;
			m_autoOpenTimer.setSingleShot ( true );
			m_autoOpenTimer.start ( 1700 );
		}
		if ( item )
		{
//FIXME 1.5			event->acceptAction ( true );
			event->setAccepted( true );
		}
		setItemUnderDrag ( bitem );

		QTreeWidget::dragMoveEvent ( event ); // FIXME: ADDED
	}
}

void BasketTree::setItemUnderDrag ( BasketTreeItem* item )
{
	if ( m_itemUnderDrag != item )
	{
		if ( m_itemUnderDrag )
		{
			// Remove drag status from the old item
			m_itemUnderDrag->setUnderDrag ( false );
// TO REMOVE			repaintItem(m_itemUnderDrag);
		}

		m_itemUnderDrag = item;

		if ( m_itemUnderDrag )
		{
			// add drag status to the new item
			m_itemUnderDrag->setUnderDrag ( true );
// TO REMOVE			repaintItem(m_itemUnderDrag);
		}
	}
}

void BasketTree::autoOpen()
{
	BasketTreeItem *item = ( BasketTreeItem* ) m_autoOpenItem;
	if ( item )
		Global::bnpView->setCurrentBasket ( item->basket() );
}

void BasketTree::resizeEvent ( QResizeEvent *event )
{
	QTreeWidget::resizeEvent ( event );
}

void BasketTree::paintEmptyArea ( QPainter *painter, const QRect &rect )
{
	//FIXME 1.5 QTreeWidgetItem::paintEmptyArea ( painter, rect );

	BasketTreeItem *last = Global::bnpView->lastListViewItem();
	if ( last && !last->isVisible() )
		last = last->shownItemAbove();
	if ( last && last->isCurrentBasket() )
	{
		int y = last->treeWidget()->visualItemRect ( last).height();
		QColor bgColor  = KColorScheme ( QPalette::Active, KColorScheme::View ).background().color();
		QColor selColor = KColorScheme ( QPalette::Active, KColorScheme::Selection ).background().color();
		QColor midColor = Tools::mixColor ( bgColor, selColor );
		painter->setPen ( selColor );
		painter->drawPoint ( width() - 1, y );
		painter->drawPoint ( width() - 2, y );
		painter->drawPoint ( width() - 1, y + 1 );
		painter->setPen ( midColor );
		painter->drawPoint ( width() - 3, y );
		painter->drawPoint ( width() - 1, y + 2 );
	}
}

/** We should NEVER get focus (because of QWidget::NoFocus focusPolicy())
 * but QTreeWidget can programatically give us the focus.
 * So we give it to the basket.
 */
void BasketTree::focusInEvent ( QFocusEvent* )
{
	//QTreeWidget::focusInEvent(event);
	Basket *basket = Global::bnpView->currentBasket();
	if ( basket )
		basket->setFocus();
}

#include "baskettree.moc"

#if 0

// Note: this file has been copied from the Qt source.
// Those classes are normally used internally in Qt
// but we need them for immitate the roll-over effect of QComboBox.
//
// A portion of code has been added. It's underlined by "THIS CODE WAS ADDED:".
//
// And some class definitions have been moved from this file to qeffects.h
// Theire old position is indicated by "REMOVED CLASS DEFINITION HERE (MOVED TO qeffects.h)"

/****************************************************************************
** $Id: qt/qeffects.cpp   3.3.4   edited Dec 10 10:13 $
**
** Implementation of QEffects functions
**
** Created : 000621
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qapplication.h"
//Added by qt3to4:
#include <QCloseEvent>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QEvent>
#ifndef QT_NO_EFFECTS
#include "qwidget.h"
#include "qeffects.h"
#include "qpixmap.h"
#include "qimage.h"
#include "qtimer.h"
#include "qdatetime.h"
#include "qpointer.h"
#include "q3scrollview.h"

/* REMOVED CLASS DEFINITION HERE (MOVED TO qeffects.h) */

static QAlphaWidget* q_blend = 0;

/*
  Constructs a QAlphaWidget.
*/
QAlphaWidget::QAlphaWidget( QWidget* w, Qt::WFlags f )
	: QWidget( QApplication::desktop()->screen(QApplication::desktop()->screenNumber(w)),
			   "qt internal alpha effect widget", f )
{
#if 1 //ndef Q_WS_WIN
    setEnabled( FALSE );
#endif

    pm.setOptimization( QPixmap::BestOptim );
    setBackgroundMode( NoBackground );
	widget = (QAccessWidget*)w;
	alpha = 0;
}

/*
  \reimp
*/
void QAlphaWidget::paintEvent( QPaintEvent* )
{
	bitBlt( this, QPoint(0,0), &pm );
}

/*
  Starts the alphablending animation.
  The animation will take about \a time ms
*/
void QAlphaWidget::run( int time )
{
	duration = time;

	if ( duration < 0 )
		duration = 150;

	if ( !widget )
		return;

	elapsed = 0;
	checkTime.start();

	showWidget = TRUE;
	qApp->installEventFilter( this );

	widget->setWState( WState_Visible );

	move( widget->geometry().x(),widget->geometry().y() );
	resize( widget->size().width(), widget->size().height() );

	front = QImage( widget->size(), 32 );
	front = QPixmap::grabWidget( widget );

	back = QImage( widget->size(), 32 );
	back = QPixmap::grabWindow( QApplication::desktop()->winId(),
								widget->geometry().x(), widget->geometry().y(),
								widget->geometry().width(), widget->geometry().height() );

	if ( !back.isNull() && checkTime.elapsed() < duration / 2 ) {
		mixed = back.copy();
		pm = mixed;
		show();
		setEnabled(FALSE);

		connect( &anim, SIGNAL(timeout()), this, SLOT(render()));
		anim.start( 1 );
	} else {
		duration = 0;
		render();
	}
}

/*
  \reimp
*/
bool QAlphaWidget::eventFilter( QObject* o, QEvent* e )
{
	switch ( e->type() ) {
		case QEvent::Move:
			if ( o != widget )
				break;
			move( widget->geometry().x(),widget->geometry().y() );
			update();
			break;
		case QEvent::Hide:
		case QEvent::Close:
			if ( o != widget )
				break;
		case QEvent::MouseButtonPress:
#ifndef QT_NO_SCROLLVIEW
	if ( ::qt_cast<Q3ScrollView*>(o) )
	    break;
#endif
		case QEvent::MouseButtonDblClick:
			setEnabled(TRUE);
			showWidget = FALSE;
			render();
			break;
		case QEvent::KeyPress:
		{
			QKeyEvent *ke = (QKeyEvent*)e;
			if ( ke->key() == Qt::Key_Escape )
				showWidget = FALSE;
			else
				duration = 0;
			render();
			break;
		}
		default:
			break;
	}
	return QWidget::eventFilter( o, e );
}

/*
  \reimp
*/
void QAlphaWidget::closeEvent( QCloseEvent *e )
{
	e->accept();
	if ( !q_blend )
		return;

	showWidget = FALSE;
	render();

	QWidget::closeEvent( e );
}

/*
  Render alphablending for the time elapsed.

  Show the blended widget and free all allocated source
  if the blending is finished.
*/
void QAlphaWidget::render()
{
	int tempel = checkTime.elapsed();
	if ( elapsed >= tempel )
		elapsed++;
	else
		elapsed = tempel;

	if ( duration != 0 )
		alpha = tempel / double(duration);
	else
		alpha = 1;
	if ( alpha >= 1 || !showWidget) {
		anim.stop();
		qApp->removeEventFilter( this );

		if ( widget ) {
			if ( !showWidget ) {
#ifdef Q_WS_WIN
                setEnabled(TRUE);
                setFocus();
#endif
		widget->hide();
		widget->setWState( WState_ForceHide );
		widget->clearWState( WState_Visible );
			} else if ( duration ) {
				BackgroundMode bgm = widget->backgroundMode();
				QColor erc = widget->eraseColor();
				const QPixmap *erp = widget->erasePixmap();

				widget->clearWState( WState_Visible );
				widget->setBackgroundMode( NoBackground );
				widget->show();
				if ( bgm != FixedColor && bgm != FixedPixmap ) {
					widget->clearWState( WState_Visible ); // prevent update in setBackgroundMode
					widget->setBackgroundMode( bgm );
					widget->setWState( WState_Visible );
				}
				if ( erc.isValid() ) {
					widget->setEraseColor( erc );
				} else if ( erp ) {
					widget->setErasePixmap( *erp );
				}
			} else {
				widget->clearWState( WState_Visible );
				widget->show();
			}
		}
		q_blend = 0;
		deleteLater();
	} else {
		if (widget)
			widget->clearWState( WState_ForceHide );
		alphaBlend();
		pm = mixed;
		repaint( FALSE );
	}
}

/*
  Calculate an alphablended image.
*/
void QAlphaWidget::alphaBlend()
{
	const double ia = 1-alpha;
	const int sw = front.width();
	const int sh = front.height();
	switch( front.depth() ) {
		case 32:
		{
			quint32** md = (quint32**)mixed.jumpTable();
			quint32** bd = (quint32**)back.jumpTable();
			quint32** fd = (quint32**)front.jumpTable();

			for (int sy = 0; sy < sh; sy++ ) {
				quint32* bl = ((quint32*)bd[sy]);
				quint32* fl = ((quint32*)fd[sy]);
				for (int sx = 0; sx < sw; sx++ ) {
					quint32 bp = bl[sx];
					quint32 fp = fl[sx];

					((quint32*)(md[sy]))[sx] =  qRgb(int (qRed(bp)*ia + qRed(fp)*alpha),
					int (qGreen(bp)*ia + qGreen(fp)*alpha),
					int (qBlue(bp)*ia + qBlue(fp)*alpha) );
				}
			}
		}
		default:
			break;
	}
}

/* REMOVED CLASS DEFINITION HERE (MOVED TO qeffects.h) */

static QRollEffect* q_roll = 0;

/*
  Construct a QRollEffect widget.
*/
QRollEffect::QRollEffect( QWidget* w, Qt::WFlags f, DirFlags orient )
	: QWidget( QApplication::desktop()->screen(QApplication::desktop()->screenNumber(w)),
			   "qt internal roll effect widget", f ), orientation(orient)
{
#if 1 //ndef Q_WS_WIN
    setEnabled( FALSE );
#endif
    widget = (QAccessWidget*) w;
    Q_ASSERT( widget );

	setBackgroundMode( NoBackground );

	if ( widget->testWState( WState_Resized ) ) {
		totalWidth = widget->width();
		totalHeight = widget->height();
	} else {
		totalWidth = widget->sizeHint().width();
		totalHeight = widget->sizeHint().height();
	}

	currentHeight = totalHeight;
	currentWidth = totalWidth;

	if ( orientation & (RightScroll|LeftScroll) )
		currentWidth = 0;
	if ( orientation & (DownScroll|UpScroll) )
		currentHeight = 0;

	pm.setOptimization( QPixmap::BestOptim );
	pm = QPixmap::grabWidget( widget );
}

/*
  \reimp
*/
void QRollEffect::paintEvent( QPaintEvent* )
{
	int x = orientation & RightScroll ? qMin(0, currentWidth - totalWidth) : 0;
	int y = orientation & DownScroll ? qMin(0, currentHeight - totalHeight) : 0;

	bitBlt( this, x, y, &pm,
			0, 0, pm.width(), pm.height(), CopyROP, TRUE );
}

/*
  \reimp
*/
bool QRollEffect::eventFilter( QObject* o, QEvent* e )
{
	switch ( e->type() ) {
		case QEvent::Move:
			if ( o != widget )
				break;
			move( widget->geometry().x(),widget->geometry().y() );
			update();
			break;
		case QEvent::Hide:
		case QEvent::Close:
			if ( o != widget || done )
				break;
			setEnabled(TRUE);
			showWidget = FALSE;
			done = TRUE;
			scroll();
			break;
		case QEvent::MouseButtonPress:
#ifndef QT_NO_SCROLLVIEW
	if ( ::qt_cast<Q3ScrollView*>(o) )
	    break;
#endif
		case QEvent::MouseButtonDblClick:
			if ( done )
				break;
			setEnabled(TRUE);
			showWidget = FALSE;
			done = TRUE;
			scroll();
			break;
		case QEvent::KeyPress:
		{
			QKeyEvent *ke = (QKeyEvent*)e;


			/* THIS CODE WAS ADDED: **************                                                           */
			if (ke->key() == Qt::Key_Enter)   /*** Because we are simulating an Enter key press.             */
				break;                        /*** So we should not car about it and continue the animation. */
			/*************************************                                                           */


			if ( ke->key() == Qt::Key_Escape )
				showWidget = FALSE;
			done = TRUE;
			scroll();
			break;
		}
		default:
			break;
	}
	return QWidget::eventFilter( o, e );
}

/*
  \reimp
*/
void QRollEffect::closeEvent( QCloseEvent *e )
{
	e->accept();
	if ( done )
		return;
	showWidget = FALSE;
	done = TRUE;
	scroll();

	QWidget::closeEvent( e );
}

/*
  Start the animation.

  The animation will take about \a time ms, or is
  calculated if \a time is negative
*/
void QRollEffect::run( int time )
{
	if ( !widget )
		return;

	duration  = time;
	elapsed = 0;

	if ( duration < 0 ) {
		int dist = 0;
		if ( orientation & (RightScroll|LeftScroll) )
			dist += totalWidth - currentWidth;
		if ( orientation & (DownScroll|UpScroll) )
			dist += totalHeight - currentHeight;
		duration = qMin( qMax( dist/3, 50 ), 120 );
	}

	connect( &anim, SIGNAL(timeout()), this, SLOT(scroll()));

	widget->setWState( WState_Visible );

	move( widget->geometry().x(),widget->geometry().y() );
	resize( qMin( currentWidth, totalWidth ), qMin( currentHeight, totalHeight ) );

	show();
	setEnabled(FALSE);

	qApp->installEventFilter( this );

	showWidget = TRUE;
	done = FALSE;
	anim.start( 1 );
	checkTime.start();
}

/*
  Roll according to the time elapsed.
*/
void QRollEffect::scroll()
{
	if ( !done && widget) {
		widget->clearWState( WState_ForceHide );
		int tempel = checkTime.elapsed();
		if ( elapsed >= tempel )
			elapsed++;
		else
			elapsed = tempel;

		if ( currentWidth != totalWidth ) {
			currentWidth = totalWidth * (elapsed/duration)
					+ ( 2 * totalWidth * (elapsed%duration) + duration )
		/ ( 2 * duration );
	    // equiv. to int( (totalWidth*elapsed) / duration + 0.5 )
			done = (currentWidth >= totalWidth);
		}
		if ( currentHeight != totalHeight ) {
			currentHeight = totalHeight * (elapsed/duration)
					+ ( 2 * totalHeight * (elapsed%duration) + duration )
		/ ( 2 * duration );
	    // equiv. to int( (totalHeight*elapsed) / duration + 0.5 )
			done = (currentHeight >= totalHeight);
		}
		done = ( currentHeight >= totalHeight ) &&
					( currentWidth >= totalWidth );

		int w = totalWidth;
		int h = totalHeight;
		int x = widget->geometry().x();
		int y = widget->geometry().y();

		if ( orientation & RightScroll || orientation & LeftScroll )
			w = qMin( currentWidth, totalWidth );
		if ( orientation & DownScroll || orientation & UpScroll )
			h = qMin( currentHeight, totalHeight );

		setUpdatesEnabled( FALSE );
		if ( orientation & UpScroll )
			y = widget->geometry().y() + qMax( 0, totalHeight - currentHeight );
		if ( orientation & LeftScroll )
			x = widget->geometry().x() + qMax( 0, totalWidth - currentWidth );
		if ( orientation & UpScroll || orientation & LeftScroll )
			move( x, y );

		resize( w, h );
		setUpdatesEnabled( TRUE );
		repaint( FALSE );
	}
	if ( done ) {
		anim.stop();
		qApp->removeEventFilter( this );
		if ( widget ) {
			if ( !showWidget ) {
#ifdef Q_WS_WIN
                setEnabled(TRUE);
                setFocus();
#endif
		widget->hide();
		widget->setWState( WState_ForceHide );
		widget->clearWState( WState_Visible );
			} else {
				BackgroundMode bgm = widget->backgroundMode();
				QColor erc = widget->eraseColor();
				const QPixmap *erp = widget->erasePixmap();

				widget->clearWState( WState_Visible );
				widget->setBackgroundMode( NoBackground );
				widget->show();
				if ( bgm != FixedColor && bgm != FixedPixmap ) {
					widget->clearWState( WState_Visible ); // prevent update in setBackgroundMode
					widget->setBackgroundMode( bgm );
					widget->setWState( WState_Visible );
				}
				if ( erc.isValid() ) {
					widget->setEraseColor( erc );
				} else if ( erp ) {
					widget->setErasePixmap( *erp );
				}
			}
		}
		q_roll = 0;
		deleteLater();
	}
}

/*
  Delete this after timeout
*/

#include "qeffects.moc"

/*!
    Scroll widget \a w in \a time ms. \a orient may be 1 (vertical), 2
    (horizontal) or 3 (diagonal).
 */
void qScrollEffect( QWidget* w, QEffects::DirFlags orient, int time )
{
	if ( q_roll ) {
		delete q_roll;
		q_roll = 0;
	}

	qApp->sendPostedEvents( w, QEvent::Move );
	qApp->sendPostedEvents( w, QEvent::Resize );
#ifdef Q_WS_X11
    uint flags = Qt::WStyle_Customize | Qt::WNoAutoErase | Qt::WStyle_StaysOnTop
	| (w->isPopup() ? Qt::WType_Popup : (Qt::WX11BypassWM | Qt::WStyle_Tool));
#else
    uint flags = Qt::WStyle_Customize | Qt::WType_Popup | Qt::WX11BypassWM | Qt::WNoAutoErase | Qt::WStyle_StaysOnTop;
#endif

    // those can popups - they would steal the focus, but are disabled
    q_roll = new QRollEffect( w, flags, orient );
    q_roll->run( time );
}

/*!
    Fade in widget \a w in \a time ms.
 */
void qFadeEffect( QWidget* w, int time )
{
	if ( q_blend ) {
		delete q_blend;
		q_blend = 0;
	}

	qApp->sendPostedEvents( w, QEvent::Move );
	qApp->sendPostedEvents( w, QEvent::Resize );

#ifdef Q_WS_X11
    uint flags = Qt::WStyle_Customize | Qt::WNoAutoErase | Qt::WStyle_StaysOnTop
	| (w->isPopup() ? Qt::WType_Popup : (Qt::WX11BypassWM | Qt::WStyle_Tool));
#else
    uint flags = Qt::WStyle_Customize | Qt::WType_Popup | Qt::WX11BypassWM | Qt::WNoAutoErase | Qt::WStyle_StaysOnTop;
#endif

    // those can popups - they would steal the focus, but are disabled
    q_blend = new QAlphaWidget( w, flags );

    q_blend->run( time );
}
#endif //QT_NO_EFFECTS

#endif // #if 0

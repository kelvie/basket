#if 0

// Note: this file has been copied from the Qt source.
// Those classes are normally used internally in Qt
// but we need them for immitate the roll-over effect of QComboBox.
//
// Some class definitions have been moved from qeffects.cpp to this file.
// They are framed with the comment "MOVED FROM qeffect.cpp"

/****************************************************************************
** $Id: qt/qeffects_p.h   3.3.4   edited May 27 2003 $
**
** Definition of QEffects functions
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

#ifndef QEFFECTS_P_H
#define QEFFECTS_P_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qeffects.cpp, qcombobox.cpp, qpopupmenu.cpp and qtooltip.cpp.
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#include "qnamespace.h"
#endif // QT_H

#ifndef QT_NO_EFFECTS
class QWidget;

struct QEffects
{
	enum Direction {
		LeftScroll	= 0x0001,
		RightScroll	= 0x0002,
		UpScroll	= 0x0004,
		DownScroll	= 0x0008
	};

	typedef uint DirFlags;
};

extern void Q_EXPORT qScrollEffect( QWidget*, QEffects::DirFlags dir = QEffects::DownScroll, int time = -1 );
extern void Q_EXPORT qFadeEffect( QWidget*, int time = -1 );


/******************* MOVED FROM qeffect.cpp: */

#include "qguardedptr.h"
#include "qdatetime.h"
#include "qtimer.h"
#include "qpixmap.h"
#include "qimage.h"

/*
  Internal class to get access to protected QWidget-members
*/

class QAccessWidget : public QWidget
{
	friend class QAlphaWidget;
	friend class QRollEffect;
	public:
		QAccessWidget( QWidget* parent=0, const char* name=0, Qt::WFlags f = 0 )
	: QWidget( parent, name, f ) {}
};

/*
  Internal class QAlphaWidget.

  The QAlphaWidget is shown while the animation lasts
  and displays the pixmap resulting from the alpha blending.
*/

class QAlphaWidget: public QWidget, private QEffects
{
	Q_OBJECT
	public:
		QAlphaWidget( QWidget* w, Qt::WFlags f = 0 );

		void run( int time );

	protected:
		void paintEvent( QPaintEvent* e );
		void closeEvent( QCloseEvent* );
		bool eventFilter( QObject* o, QEvent* e );
		void alphaBlend();

	protected slots:
		void render();

	private:
		QPixmap pm;
		double alpha;
		QImage back;
		QImage front;
		QImage mixed;
		QGuardedPtr<QAccessWidget> widget;
		int duration;
		int elapsed;
		bool showWidget;
		QTimer anim;
		QTime checkTime;
};

/*
  Internal class QRollEffect

  The QRollEffect widget is shown while the animation lasts
  and displays a scrolling pixmap.
*/

class QRollEffect : public QWidget, private QEffects
{
	Q_OBJECT
	public:
		QRollEffect( QWidget* w, WFlags f, DirFlags orient );

		void run( int time );

	protected:
		void paintEvent( QPaintEvent* );
		bool eventFilter( QObject*, QEvent* );
		void closeEvent( QCloseEvent* );

	private slots:
		void scroll();

	private:
		QGuardedPtr<QAccessWidget> widget;

		int currentHeight;
		int currentWidth;
		int totalHeight;
		int totalWidth;

		int duration;
		int elapsed;
		bool done;
		bool showWidget;
		int orientation;

		QTimer anim;
		QTime checkTime;

		QPixmap pm;
};

/******************************/

#endif // QT_NO_EFFECTS

#endif // QEFFECTS_P_H

#endif // #if 0

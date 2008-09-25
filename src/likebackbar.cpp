/***************************************************************************
                          (filename).cpp -  description
                             -------------------
    begin                : (Weekday) (Month) (day) 2008
    copyright            : (C) 2008 by (yourname)
    email                : (e-mail address)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <KApplication>
#include <KDebug>

#include "likebackbar.h"



LikeBackBar::LikeBackBar( LikeBack *likeBack )
: QWidget( 0, Qt::Tool | Qt::X11BypassWindowManagerHint )
           /* Qt::WX11BypassWM | Qt::WStyle_NoBorder | Qt::WNoAutoErase | Qt::WStyle_StaysOnTop | Qt::WStyle_NoBorder | Qt::WGroupLeader */
, Ui::LikeBackBar()
, m_likeBack( likeBack )
{
  // Set up the user interface
  setupUi( this );

  // Set the button icons
  m_likeButton   ->setIcon( KIcon( "likeback_like"    ) );
  m_dislikeButton->setIcon( KIcon( "likeback_dislike" ) );
  m_bugButton    ->setIcon( KIcon( "likeback_bug"     ) );
  m_featureButton->setIcon( KIcon( "likeback_feature" ) );

  connect( &m_timer, SIGNAL(timeout()), this, SLOT(autoMove()) );

  LikeBack::Button buttons = likeBack->buttons();
  m_likeButton   ->setShown( buttons & LikeBack::Like    );
  m_dislikeButton->setShown( buttons & LikeBack::Dislike );
  m_bugButton    ->setShown( buttons & LikeBack::Bug     );
  m_featureButton->setShown( buttons & LikeBack::Feature );
}

LikeBackBar::~LikeBackBar()
{
}

void LikeBackBar::startTimer()
{
  m_timer.start(10);
}

void LikeBackBar::stopTimer()
{
  m_timer.stop();
}

void LikeBackBar::autoMove()
{
  static QWidget *lastWindow = 0;

  QWidget *window = kapp->activeWindow();
  // When a Kicker applet has the focus, like the Commandline QLineEdit,
  // the systemtray icon indicates to be the current window and the LikeBack is shown next to the system tray icon.
  // It's obviously bad ;-) :
  bool shouldShow = (m_likeBack->userWantsToShowBar() && m_likeBack->enabledBar() && window && !window->inherits("KSystemTray"));
  if (shouldShow) {
    //move(window->x() + window->width() - 100 - width(), window->y());
    //move(window->x() + window->width() - 100 - width(), window->mapToGlobal(QPoint(0, 0)).y() - height());
    move(window->mapToGlobal(QPoint(0, 0)).x() + window->width() - width(), window->mapToGlobal(QPoint(0, 0)).y() + 1);

    if (window != lastWindow && m_likeBack->windowNamesListing() != LikeBack::NoListing) {
      if( window->objectName() == "unnamed" || window->objectName() == "unnamed" )
      {
        kDebug() << "===== LikeBack ===== UNNAMED ACTIVE WINDOW OF TYPE " << window->metaObject()->className() << " ======" << LikeBack::activeWindowPath();
      } else if (m_likeBack->windowNamesListing() == LikeBack::AllWindows) {
        kDebug() << "LikeBack: Active Window: " << LikeBack::activeWindowPath();
      }
    }
    lastWindow = window;
  }

  // Show or hide the bar accordingly:
  if (shouldShow && !isVisible()) {
    show();
  } else if (!shouldShow && isVisible()) {
    hide();
  }
}

void LikeBackBar::likeClicked()
{
  kDebug() << "LIKE";
  m_likeBack->execCommentDialog( LikeBack::Like );
}

void LikeBackBar::dislikeClicked()
{
  kDebug() << "DISLIKE";
  m_likeBack->execCommentDialog( LikeBack::Dislike );
}

void LikeBackBar::bugClicked()
{
  kDebug() << "BUG";
  m_likeBack->execCommentDialog( LikeBack::Bug );
}

void LikeBackBar::featureClicked()
{
  kDebug() << "FEATURE";
  m_likeBack->execCommentDialog( LikeBack::Feature );
}

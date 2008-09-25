/***************************************************************************
 *   Copyright (C) 2006 by Sebastien Laout                                 *
 *   slaout@linux62.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.         *
 ***************************************************************************/

#ifndef LIKEBACK_PRIVATE_H
#define LIKEBACK_PRIVATE_H

#include <QTimer>


class QButtonGroup;

class Kaction;



class LikeBackPrivate
{
  public:
  LikeBackPrivate();
  ~LikeBackPrivate();
  LikeBackBar             *bar;
  KConfig                 *config;
  const KAboutData        *aboutData;
  LikeBack::Button         buttons;
  QString                  hostName;
  QString                  remotePath;
  quint16                 hostPort;
  QStringList              acceptedLocales;
  QString                  acceptedLanguagesMessage;
  LikeBack::WindowListing  windowListing;
  bool                     showBarByDefault;
  bool                     showBar;
  int                      disabledCount;
  QString                  fetchedEmail;
  KAction                 *action;
};


LikeBackPrivate::LikeBackPrivate()
 : bar(0)
 , config(0)
 , aboutData(0)
 , buttons(LikeBack::DefaultButtons)
 , hostName()
 , remotePath()
 , hostPort(80)
 , acceptedLocales()
 , acceptedLanguagesMessage()
 , windowListing(LikeBack::NoListing)
 , showBar(false)
 , disabledCount(0)
 , fetchedEmail()
 , action(0)
{
}

LikeBackPrivate::~LikeBackPrivate()
{
  delete bar;
  delete action;

  config = 0;
  aboutData = 0;
}


#endif // LIKEBACK_PRIVATE_H

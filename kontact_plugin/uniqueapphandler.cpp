/*
   This file is part of KDE Kontact.

   Copyright (c) 2003 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "uniqueapphandler.h"
#include <kstartupinfo.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kontact/core.h>
#include <kwin.h>
#include <dcopclient.h>
#include <kdebug.h>

/*
 Test plan for the various cases of interaction between standalone apps and kontact:

 1) start kontact, select "Mail".
 1a) type "korganizer" -> it switches to korganizer
 1b) type "kmail" -> it switches to kmail
 1c) type "kaddressbook" -> it switches to kaddressbook
 1d) type "kmail foo@kde.org" -> it opens a kmail composer, without switching
 1e) type "knode" -> it switches to knode
 1f) type "kaddressbook --new-contact" -> it opens a kaddressbook contact window
 1g) type "knode news://foobar/group" -> it pops up "can't resolve hostname"

 2) close kontact. Launch kmail. Launch kontact again.
 2a) click "Mail" icon -> kontact doesn't load a part, but activates the kmail window
 2b) type "kmail foo@kde.org" -> standalone kmail opens composer.
 2c) close kmail, click "Mail" icon -> kontact loads the kmail part.
 2d) type "kmail" -> kontact is brought to front

 3) close kontact. Launch korganizer, then kontact.
 3a) both Todo and Calendar activate the running korganizer.
 3b) type "korganizer" -> standalone korganizer is brought to front
 3c) close korganizer, click Calendar or Todo -> kontact loads part.
 3d) type "korganizer" -> kontact is brought to front

 4) close kontact. Launch kaddressbook, then kontact.
 4a) "Contacts" icon activate the running kaddressbook.
 4b) type "kaddressbook" -> standalone kaddressbook is brought to front
 4c) close kaddressbook, type "kaddressbook -a foo@kde.org" -> kontact loads part and opens editor
 4d) type "kaddressbook" -> kontact is brought to front

 5) close kontact. Launch knode, then kontact.
 5a) "News" icon activate the running knode.
 5b) type "knode" -> standalone knode is brought to front
 5c) close knode, type "knode news://foobar/group" -> kontact loads knode and pops up msgbox
 5d) type "knode" -> kontact is brought to front

*/

using namespace Kontact;

int UniqueAppHandler::newInstance()
{
  // This bit is duplicated from KUniqueApplication::newInstance()
  if ( kapp->mainWidget() ) {
    kapp->mainWidget()->show();
    KWin::forceActiveWindow( kapp->mainWidget()->winId() );
    KStartupInfo::appStarted();
  }

  // Then ensure the part appears in kontact
  mPlugin->core()->selectPlugin( mPlugin );
  return 0;
}

bool UniqueAppHandler::process( const QCString &fun, const QByteArray &data,
                                QCString& replyType, QByteArray &replyData )
{
  if ( fun == "newInstance()" ) {
    replyType = "int";

    KCmdLineArgs::reset(); // forget options defined by other "applications"
    loadCommandLineOptions();

    // This bit is duplicated from KUniqueApplication::processDelayed()
    QDataStream ds( data, QIODevice::ReadOnly );
    KCmdLineArgs::loadAppArgs( ds );
    if ( !ds.atEnd() ) { // backwards compatibility
      QCString asn_id;
      ds >> asn_id;
      kapp->setStartupId( asn_id );
    }

    QDataStream _replyStream( replyData, QIODevice::WriteOnly );
    _replyStream << newInstance( );
  } else if ( fun == "load()" ) {
    replyType = "bool";
    (void)mPlugin->part(); // load the part without bringing it to front

    QDataStream _replyStream( replyData, QIODevice::WriteOnly );
    _replyStream << true;
  } else {
    return DCOPObject::process( fun, data, replyType, replyData );
  }
  return true;
}

QCStringList UniqueAppHandler::interfaces()
{
  QCStringList ifaces = DCOPObject::interfaces();
  ifaces += "Kontact::UniqueAppHandler";
  return ifaces;
}

QCStringList UniqueAppHandler::functions()
{
  QCStringList funcs = DCOPObject::functions();
  funcs << "int newInstance()";
  funcs << "bool load()";
  return funcs;
}

UniqueAppWatcher::UniqueAppWatcher( UniqueAppHandlerFactoryBase* factory, Plugin* plugin )
    : QObject( plugin ), mFactory( factory ), mPlugin( plugin )
{
  // The app is running standalone if 1) that name is known to DCOP
  mRunningStandalone = kapp->dcopClient()->isApplicationRegistered( plugin->name() );

  // and 2) it's not registered by kontact (e.g. in another plugin)
  if ( mRunningStandalone && kapp->dcopClient()->findLocalClient( plugin->name() ) )
      mRunningStandalone = false;

  if ( mRunningStandalone ) {
    kapp->dcopClient()->setNotifications( true );
    connect( kapp->dcopClient(), SIGNAL( applicationRemoved( const QCString& ) ),
             this, SLOT( unregisteredFromDCOP( const QCString& ) ) );
  } else {
    mFactory->createHandler( mPlugin );
  }
}

UniqueAppWatcher::~UniqueAppWatcher()
{
  if ( mRunningStandalone )
    kapp->dcopClient()->setNotifications( false );

  delete mFactory;
}

void UniqueAppWatcher::unregisteredFromDCOP( const QCString& appId )
{
  if ( appId == mPlugin->name() && mRunningStandalone ) {
    disconnect( kapp->dcopClient(), SIGNAL( applicationRemoved( const QCString& ) ),
                this, SLOT( unregisteredFromDCOP( const QCString& ) ) );
    kdDebug(5601) << k_funcinfo << appId << endl;
    mFactory->createHandler( mPlugin );
    kapp->dcopClient()->setNotifications( false );
    mRunningStandalone = false;
  }
}

#include "uniqueapphandler.moc"

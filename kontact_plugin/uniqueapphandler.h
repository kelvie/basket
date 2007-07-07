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

#ifndef KONTACT_UNIQUEAPPHANDLER_H
#define KONTACT_UNIQUEAPPHANDLER_H

#include <dcopobject.h>
#include <kontact/plugin.h>
#include <kdepimmacros.h>

namespace Kontact
{

/**
 * DCOP Object that has the name of the standalone application (e.g. "kmail")
 * and implements newInstance() so that running the separate application does
 * the right thing when kontact is running.
 * By default this means simply bringing the main window to the front,
 * but newInstance can be reimplemented.
 */
class KDE_EXPORT UniqueAppHandler : public DCOPObject
{
  K_DCOP

  public:
    UniqueAppHandler( Plugin* plugin ) : DCOPObject( plugin->name() ), mPlugin( plugin ) {}

    /// This must be reimplemented so that app-specific command line options can be parsed
    virtual void loadCommandLineOptions() = 0;

    /// We can't use k_dcop and dcopidl here, because the data passed
    /// to newInstance can't be expressed in terms of normal data types.
    virtual int newInstance();

    Plugin* plugin() const { return mPlugin; }

  private:
    Plugin* mPlugin;
};

/// Base class for UniqueAppHandler
class UniqueAppHandlerFactoryBase
{
  public:
    virtual UniqueAppHandler* createHandler( Plugin* ) = 0;
};

/**
 * Used by UniqueAppWatcher below, to create the above UniqueAppHandler object
 * when necessary.
 * The template argument is the UniqueAppHandler-derived class.
 * This allows to remove the need to subclass UniqueAppWatcher.
 */
template <class T> class UniqueAppHandlerFactory : public UniqueAppHandlerFactoryBase
{
  public:
    virtual UniqueAppHandler* createHandler( Plugin* plugin ) {
        (void)plugin->dcopClient(); // ensure that we take over the DCOP name
        return new T( plugin );
    }
};


/**
 * If the standalone application is running by itself, we need to watch
 * for when the user closes it, and activate the uniqueapphandler then.
 * This prevents, on purpose, that the standalone app can be restarted.
 * Kontact takes over from there.
 *
 */
class KDE_EXPORT UniqueAppWatcher : public QObject
{
  Q_OBJECT

  public:
    /**
     * Create an instance of UniqueAppWatcher, which does everything necessary
     * for the "unique application" behavior: create the UniqueAppHandler as soon
     * as possible, i.e. either right now or when the standalone app is closed.
     *
     * @param factory templatized factory to create the handler. Example:
     * ...   Note that the watcher takes ownership of the factory.
     * @param plugin is the plugin application
     */
    UniqueAppWatcher( UniqueAppHandlerFactoryBase* factory, Plugin* plugin );

    virtual ~UniqueAppWatcher();

    bool isRunningStandalone() const { return mRunningStandalone; }

  protected slots:
    void unregisteredFromDCOP( const QByteArray& appId );

  private:
    bool mRunningStandalone;
    UniqueAppHandlerFactoryBase* mFactory;
    Plugin* mPlugin;
};

} // namespace

#endif /* KONTACT_UNIQUEAPPHANDLER_H */

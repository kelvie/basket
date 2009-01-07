/*
  This file is part of the KDE Kontact Plugin Interface Library.

  Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
  Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>

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
#ifndef KONTACTINTERFACES_CORE_H
#define KONTACTINTERFACES_CORE_H

#include "kontactinterfaces_export.h"
#include <kparts/mainwindow.h>
#include <kparts/part.h>

namespace Kontact
{

class Plugin;

/**
  This class provides the interface to the Kontact core for the plugins.
*/
class KONTACTINTERFACES_EXPORT Core : public KParts::MainWindow
{
  Q_OBJECT
  public:
    virtual ~Core();

    /**
      Selects the given plugin and raises the associated part.
      @see selectPlugin(const QString &)

      @param plugin is a pointer to the Kontact Plugin to select.
     */
    virtual void selectPlugin( Kontact::Plugin *plugin ) = 0;

    /**
      This is an overloaded member function
      @see selectPlugin(Kontact::Plugin *)

      @param plugin is the name of the Kontact Plugin select.
     */
    virtual void selectPlugin( const QString &plugin ) = 0;

    /**
      Returns the pointer list of available plugins.
     */
    virtual QList<Kontact::Plugin*> pluginList() const = 0;

    /**
      @internal (for Plugin)
     */
    KParts::ReadOnlyPart *createPart( const char *libname );

    /**
      @internal (for Plugin)
      Tell kontact that a part was loaded
     */
    virtual void partLoaded( Plugin *plugin, KParts::ReadOnlyPart *part ) = 0;

  Q_SIGNALS:
    /**
      Emitted when a new day starts
      */
    void dayChanged( const QDate & );

  protected:
    explicit Core( QWidget *parentWidget = 0, Qt::WindowFlags f = KDE_DEFAULT_WINDOWFLAGS );

    QString lastErrorMessage() const;

  private:
    class Private;
    Private *const d;
    Q_PRIVATE_SLOT( d, void slotPartDestroyed( QObject * ) )
    Q_PRIVATE_SLOT( d, void checkNewDay() )
};

}

#endif

// vim: sw=2 sts=2 et tw=80

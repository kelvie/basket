/*
  This file is part of the KDE Kontact Plugin Interface Library.

  Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
  Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>
  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef KONTACTINTERFACES_PLUGIN_H
#define KONTACTINTERFACES_PLUGIN_H

#include "kontactinterfaces_export.h"

#include <kxmlguiclient.h>
#include <kpluginfactory.h>

#include <QtCore/QList>
#include <QtCore/QObject>

class KAboutData;
class KAction;
class KConfig;
class KConfigGroup;
class QDropEvent;
class QMimeData;
class QStringList;
class QWidget;
namespace KParts {
  class ReadOnlyPart;
}

/**
  Exports Kontact plugin.
 */
#define EXPORT_KONTACT_PLUGIN( pluginclass, pluginname ) \
class Instance                                           \
{                                                        \
  public:                                                \
    static QObject *createInstance( QWidget *, QObject *parent, const QVariantList &list ) \
    { return new pluginclass( static_cast<Kontact::Core*>( parent ), list ); } \
};                                                                    \
K_PLUGIN_FACTORY( KontactPluginFactory, registerPlugin< pluginclass >   \
                  ( QString(), Instance::createInstance ); )            \
K_EXPORT_PLUGIN( KontactPluginFactory( "kontact_" #pluginname "plugin" ) )

/**
  Increase this version number whenever you make a change in the API.
 */
#define KONTACT_PLUGIN_VERSION 7

namespace Kontact
{

class Core;
class Summary;

/**
  Base class for all Plugins in Kontact. Inherit from it
  to get a plugin. It can insert an icon into the sidepane,
  add widgets to the widgetstack and add menu items via XMLGUI.
 */
class KONTACTINTERFACES_EXPORT Plugin : public QObject, virtual public KXMLGUIClient
{
  Q_OBJECT

  public:
    /**
      Creates a new Plugin, note that name parameter name is required if
      you want your plugin to do dcop via it's own instance of
      DCOPClient by calling dcopClient.
      @note name MUST be the name of the application that
      provides the part! This is the name used for DCOP registration.
      It's ok to have several plugins using the same application name.
    */
    Plugin( Core *core, QObject *parent, const char *name );

    virtual ~Plugin();

    /**
      Sets the identifier.
    */
    void setIdentifier( const QString &identifier );

    /**
      Returns the identifier. It is used as argument for several
      methods of Kontacts core.
    */
    QString identifier() const;

    /**
      Sets the localized title.
     */
    void setTitle( const QString &title );

    /**
      Returns the localized title.
    */
    QString title() const;

    /**
      Sets the icon name.
    */
    void setIcon( const QString &icon );

    /**
      Returns the icon name.
    */
    QString icon() const;

    /**
      Sets the name of executable (if existent).
    */
    void setExecutableName( const QString &bin );

    /**
      Returns the name of the binary (if existent).
    */
    QString executableName() const;

    /**
      Set name of library which contains the KPart used by this plugin.
    */
    void setPartLibraryName( const QByteArray & );

    /**
      Create the D-Bus interface for the given @p serviceType, if this
      plugin provides it. Return false otherwise.
    */
    virtual bool createDBUSInterface( const QString &serviceType );

    /**
      Reimplement this method and return whether a standalone application
      is still running. This is only required if your part is also available
      as standalone application.
    */
    virtual bool isRunningStandalone();

    /**
      Reimplement this method if your application needs a different approach to be brought
      in the foreground. The default behaviour is calling the binary.
      This is only required if your part is also available as standalone application.
    */
    virtual void bringToForeground();

    /**
      Reimplement this method if you want to add your credits to the Kontact
      about dialog.
    */
    virtual const KAboutData *aboutData();

    /**
      You can use this method if you need to access the current part. You can be
      sure that you always get the same pointer as long as the part has not been
      deleted.
    */
    KParts::ReadOnlyPart *part();

     /**
       Reimplement this method and return the a path relative to "data" to the tips file.
       The tips file contains hints/tips that are displayed at the beginning of the program
       as "tip of the day". It has nothing to do with tooltips.
     */
    virtual QString tipFile() const;

    /**
      This function is called when the plugin is selected by the user before the
      widget of the KPart belonging to the plugin is raised.
    */
    virtual void select();

    /**
      This function is called whenever the config dialog has been closed
      successfully.
     */
    virtual void configUpdated();

    /**
      Reimplement this method if you want to add a widget for your application
      to Kontact's summary page.
    */
    virtual Summary *createSummaryWidget( QWidget *parent );

    /**
      Returns whether the plugin provides a part that should be shown in the sidebar.
    */
    virtual bool showInSideBar() const;

    /**
      Set if the plugin provides a part that should be shown in the sidebar.
    */
    void setShowInSideBar( bool hasPart );

    /**
      Reimplement this method if you want to add checks before closing the
      main kontact window. Return true if it's OK to close the window.
      If any loaded plugin returns false from this method, then the
      main kontact window will not close.
    */
    virtual bool queryClose() const;

    QString registerClient();

    /**
      Return the weight of the plugin. The higher the weight the lower it will
      be displayed in the sidebar. The default implementation returns 0.
    */
    virtual int weight() const;

    /**
      Insert "New" action.
    */
    void insertNewAction( KAction *action );

    /**
      Insert "Sync" action.
    */
    void insertSyncAction( KAction *action );

    /**
      FIXME: write API doc for Plugin::newActions().
    */
    QList<KAction*>* newActions() const;

    /**
      FIXME: write API doc for Plugin::syncActions().
    */
    QList<KAction*>* syncActions() const;

    /**
      Returns a list of action name which shall be hidden in the main toolbar.
     */
    virtual QStringList invisibleToolbarActions() const;

    /**
      Return, if the plugin can handle the drag object of the given mime type.
    */
    virtual bool canDecodeMimeData( const QMimeData *data );

    /**
      Process drop event.
    */
    virtual void processDropEvent( QDropEvent * ) {}

    /**
     * Session management: read properties
     */
    virtual void readProperties( const KConfigGroup & ) {}

    /**
     * Session management: save properties
     */
    virtual void saveProperties( KConfigGroup & ) {}

    Core *core() const;

    bool disabled() const;
    void setDisabled( bool v );

  public Q_SLOTS:
    /**
      internal usage
     */
    void slotConfigUpdated();

  protected:
    /**
      Reimplement and return the part here. Reimplementing createPart() is
      mandatory!
    */
    virtual KParts::ReadOnlyPart *createPart() = 0;

    KParts::ReadOnlyPart *loadPart();

    virtual void virtual_hook( int id, void *data );

  private:
    class Private;
    Private *const d;
    Q_PRIVATE_SLOT( d, void partDestroyed() )
};

}

#endif

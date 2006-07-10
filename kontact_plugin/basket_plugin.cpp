/***************************************************************************
 *   Copyright (C) 2006 by Petri Damsten                                   *
 *   damu@iki.fi                                                           *
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

#include <kgenericfactory.h>
#include <kparts/componentfactory.h>
#include <kontact/core.h>

#include "basket_plugin.h"

typedef KGenericFactory<BasketPlugin, Kontact::Core> BasketPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_basket,
                            BasketPluginFactory( "kontact_basketplugin" ) )

BasketPlugin::BasketPlugin( Kontact::Core *core, const char *, const QStringList& )
  : Kontact::Plugin( core, core, "Basket" )
{
  setInstance( BasketPluginFactory::instance() );
}

BasketPlugin::~BasketPlugin()
{
}

KParts::ReadOnlyPart* BasketPlugin::createPart()
{
  return loadPart();
}

#include "basket_plugin.moc"

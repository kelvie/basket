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
#include <klocale.h>
#include "basketdcopiface_stub.h"
#include "basket_plugin.h"

typedef KGenericFactory<BasketPlugin, Kontact::Core> BasketPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_basket,
                            BasketPluginFactory( "kontact_basketplugin" ) )

BasketPlugin::BasketPlugin( Kontact::Core *core, const char *, const QStringList& )
  : Kontact::Plugin( core, core, "basket" )
{
  setInstance( BasketPluginFactory::instance() );
  insertNewAction(new KAction( i18n("&New Basket..."), "basket", CTRL+SHIFT+Key_B,
				  this, SLOT(newBasket()), actionCollection(), "basket_new" ));

	//mUniqueAppWatcher = new Kontact::UniqueAppWatcher(
	//	  new Kontact::UniqueAppHandlerFactory<KMailUniqueAppHandler>(), this );
}

BasketPlugin::~BasketPlugin()
{
}

KParts::ReadOnlyPart* BasketPlugin::createPart()
{
	KParts::ReadOnlyPart *part = loadPart();
	if(!part)
		return 0;

	m_stub = new BasketDcopInterface_stub(dcopClient(), "basket", "BasketIface");

	return part;
}

void BasketPlugin::newBasket()
{
	(void) part(); // ensure part is loaded
	Q_ASSERT( m_stub );
	if ( m_stub ) {
		kdDebug() << k_funcinfo << endl;
		m_stub->newBasket();
	}
}

#include "basket_plugin.moc"

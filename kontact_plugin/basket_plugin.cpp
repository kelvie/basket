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
#include <kcmdlineargs.h>
#include <dcopref.h>
#include "basketdcopiface_stub.h"
#include "basket_plugin.h"
#include "basket_options.h"
#include "basket_part.h"

typedef KGenericFactory<BasketPlugin, Kontact::Core> BasketPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_basket,
                            BasketPluginFactory( "kontact_basketplugin" ) )

BasketPlugin::BasketPlugin( Kontact::Core *core, const char *, const QStringList& )
  : Kontact::Plugin( core, core, "basket" )
{
	setInstance( BasketPluginFactory::instance() );
	insertNewAction(new KAction( i18n("&New Basket..."), "basket", Qt::CTRL+Qt::SHIFT+Qt::Key_B,
					this, SLOT(newBasket()), actionCollection(), "basket_new" ));

	m_uniqueAppWatcher = new Kontact::UniqueAppWatcher(
		new Kontact::UniqueAppHandlerFactory<BasketUniqueAppHandler>(), this);
}

BasketPlugin::~BasketPlugin()
{
}

KParts::ReadOnlyPart* BasketPlugin::createPart()
{
	BasketPart* part = static_cast<BasketPart*>(loadPart());
	if(!part)
		return 0;

	m_stub = new BasketDcopInterface_stub(dcopClient(), "basket", "BasketIface");
	connect(part, SIGNAL(showPart()), this, SLOT(showPart()));
	return part;
}

void BasketPlugin::newBasket()
{
	(void) part(); // ensure part is loaded
	Q_ASSERT(m_stub);
	if (m_stub) {
		kdDebug() << k_funcinfo << endl;
		m_stub->newBasket();
	}
}

void BasketPlugin::showPart()
{
	core()->selectPlugin(this);
}

#if 0
bool BasketPlugin::createDCOPInterface( const QString& serviceType )
{
	kdDebug() << k_funcinfo << serviceType << endl;
	return false;
}
#endif
bool BasketPlugin::isRunningStandalone()
{
	return m_uniqueAppWatcher->isRunningStandalone();
}

void BasketUniqueAppHandler::loadCommandLineOptions()
{
	KCmdLineArgs::addCmdLineOptions(basket_options);
}

int BasketUniqueAppHandler::newInstance()
{
	(void)plugin()->part();
	DCOPRef kmail("basket", "BasketIface");
	DCOPReply reply = kmail.call("handleCommandLine", false);
	if (reply.isValid()) {
		bool handled = reply;
		if ( !handled )
			return Kontact::UniqueAppHandler::newInstance();
	}
	return 0;
}

#include "basket_plugin.moc"

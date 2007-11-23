#include "BasketContent.h"

#include <KDebug>

#include "plasma/layouts/layoutanimator.h"
#include "plasma/phase.h"

BasketContent::BasketContent( QObject *parent, const QVariantList &args )
    : Plasma::Containment( parent, args )
{
    m_columns = new Plasma::FlowLayout( this );
    m_columns->setColumnWidth( 512 );
}

Plasma::Containment::Type BasketContent::containmentType() const {
	return CustomContainment;
}

void BasketContent::launchAppletBrowser( ) {
}

#define K_EXPORT_AMAROK_APPLET(libname, classname) \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("basket_content_applet_" #libname))

K_EXPORT_PLASMA_APPLET( content, BasketContent )

//K_EXPORT_BASKET_APPLET( content, BasketContent )
//K_PLUGIN_FACTORY(factory, registerPlugin<BasketContent>();)
//K_EXPORT_PLUGIN(factory("basket_content_applet", "content"))



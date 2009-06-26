#include "basket_plugin.h"
#include "global.h"

#include "basket_part.h"

#include <kontactinterfaces/core.h>
#include <kontactinterfaces/plugin.h>

#include <kactioncollection.h>
#include <kgenericfactory.h>
#include <kicon.h>
#include <kparts/componentfactory.h>

EXPORT_KONTACT_PLUGIN(BasketPlugin, basket)

BasketPlugin::BasketPlugin(Kontact::Core *core, const QVariantList &)
        : Kontact::Plugin(core, core, "Basket")
{
    setComponentData(KontactPluginFactory::componentData());
    Global::basketConfig = KSharedConfig::openConfig("basketrc");
}

BasketPlugin::~BasketPlugin()
{
}

KParts::ReadOnlyPart *BasketPlugin::createPart()
{
    KParts::ReadOnlyPart *part = loadPart();

    connect(part, SIGNAL(showPart()), this, SLOT(showPart()));

    return part;
}

void BasketPlugin::readProperties(const KConfigGroup &config)
{
    if (part()) {
        BasketPart *myPart = static_cast<BasketPart*>(part());
    }
}

void BasketPlugin::saveProperties(KConfigGroup &config)
{
    if (part()) {
        BasketPart *myPart = static_cast<BasketPart*>(part());
    }
}

void BasketPlugin::showPart()
{
    core()->selectPlugin(this);
}

#include "basket_plugin.moc"

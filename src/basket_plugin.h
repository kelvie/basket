#ifndef BASKET_PLUGIN_H
#define BASKET_PLUGIN_H

#include <kontactinterfaces/plugin.h>
#include <KDE/KParts/Part>

class BasketPlugin : public Kontact::Plugin
{
    Q_OBJECT

public:
    BasketPlugin(Kontact::Core *core, const QVariantList &);
    ~BasketPlugin();

    virtual void readProperties(const KConfigGroup &config);
    virtual void saveProperties(KConfigGroup &config);

private slots:
    void showPart();

protected:
    KParts::ReadOnlyPart *createPart();
};

#endif

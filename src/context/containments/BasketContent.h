#ifndef BASKETCONTENT_H
#define BASKETCONTENT_H

#include "plasma/layouts/flowlayout.h"
#include "plasma/containment.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QList>

#include <KService>
#include <KServiceTypeTrader>
#include <kdemacros.h>
#include <kpluginfactory.h>
#include <kexportplugin.h>
#include <KPluginLoader>

class KDE_EXPORT BasketContent : public Plasma::Containment {
    Q_OBJECT
public:
    BasketContent( QObject *parent, const QVariantList &args );
    BasketContent() {}

	virtual Type containmentType() const;

protected slots:
	void launchAppletBrowser( );
    
private:
    Plasma::FlowLayout* m_columns;
};


#endif

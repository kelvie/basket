#include "nepomukintegration.h"

#include <KDE/KDebug>


#include "global.h"
#include "debugwindow.h"

#include <QUrl>
#include <Nepomuk/ResourceManager>
#include <Nepomuk/Variant>
#include <Nepomuk/Types/Class>
#include <Nepomuk/Tag>
#include <Soprano/Vocabulary/NAO>
#include "nie.h"
#include "nfo.h"
#include "pimo.h"

bool nepomukintegration::updateMetadata(const QString &fullPath, const QDomDocument &document){

    if ( Nepomuk::ResourceManager::instance()->initialized() || Nepomuk::ResourceManager::instance()->init() )
    {
        DEBUG_WIN << "\tinitialized";
        QUrl basketUri( "file://" + fullPath );
        Nepomuk::Resource basketRes(basketUri);
        basketRes.setProperty( Soprano::Vocabulary::NIE::mimeType() /*"http://www.semanticdesktop.org/ontologies/2007/01/19/nie#mimeType"*/
                               , "application/x-basket-item" );
        basketRes.setProperty( Soprano::Vocabulary::NIE::url() /*"http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url"*/
                               , basketUri );
        /* basketRes.setProperty( Soprano::Vocabulary::NAO::prefLabel(), "myFirstLabel");
           */

        QHash<QUrl, Nepomuk::Variant> basketProperties = basketRes.properties();
        DEBUG_WIN << "\tproperties : ";
        for( QHash<QUrl, Nepomuk::Variant>::const_iterator it = basketProperties.constBegin();
        it != basketProperties.constEnd(); ++it ) {
            QUrl propertyUri = it.key();
            Nepomuk::Variant value = it.value();
            Nepomuk::Types::Class propertyType( propertyUri );
            DEBUG_WIN << "\t" + propertyType.uri().toString() + "/"
                    + propertyType.label() + "/" + propertyType.name() + ": " + value.toString();
        }

        //basketRes.remove();
    } else {
        DEBUG_WIN << "\tinitialization failed!";
        return false;
    }
    return true;

}

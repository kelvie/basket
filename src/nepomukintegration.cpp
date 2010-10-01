#include "nepomukintegration.h"

#include <QDomNode>
#include <QDomDocument>
#include <QDomElement>
#include <KUrl>
#include <KDE/KDebug>


#include "global.h"
#include "tag.h"
#include "debugwindow.h"

#include <Nepomuk/ResourceManager>
#include <Nepomuk/Variant>
#include <Nepomuk/Types/Class>
/* #include <Nepomuk/File> */
#include <Nepomuk/Tag>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/RDFS>
#include "nie.h"
#include "nfo.h"
#include "pimo.h"


bool nepomukintegration::updateMetadata(const QString &fullPath, const QDomDocument &document) {

    if ( Nepomuk::ResourceManager::instance()->initialized() || Nepomuk::ResourceManager::instance()->init() ) {
        DEBUG_WIN << "\tinitialized";
        KUrl basketUri = KUrl( fullPath );
        /* Nepomuk::File basketRes(basketUri); */
        Nepomuk::Resource basketRes(basketUri);
        basketRes.addType( Soprano::Vocabulary::PIMO::Note() );
        /* Added by Strigi; no need: basketRes.addType( Soprano::Vocabulary::NFO::FileDataObject() ); */
        basketRes.setProperty( Soprano::Vocabulary::RDF::type(), Soprano::Vocabulary::PIMO::Note() );
        basketRes.setProperty( Soprano::Vocabulary::NIE::mimeType(), "application/x-basket-item" );
        basketRes.setProperty( Soprano::Vocabulary::NIE::url(), basketUri );
        /* nfo:fileUrl is deprecated in favor of nie:url: basketRes.setProperty( Soprano::Vocabulary::NFO::fileUrl(), basketUri ); */
        basketRes.setProperty( Soprano::Vocabulary::NFO::fileName(), ".basket" );
        QString basketName = document.elementsByTagName("name").item(0).firstChild().nodeValue();
        basketRes.setProperty( Soprano::Vocabulary::NIE::title(), basketName);
        basketRes.setLabel( basketName );
        QDomNode n;
        Nepomuk::Tag basketTag;
        QList<Nepomuk::Tag> tagList;
        
        QDomNodeList tagElements = document.elementsByTagName("tags");
        for (int i = 0; i < tagElements.count(); i++) {
            foreach(QString t, tagElements.item(i).firstChild().nodeValue().split(';') ) {
                QString tagName = Tag::stateForId(t)->name();
                basketTag = Nepomuk::Tag( tagName );
                basketTag.setProperty( Soprano::Vocabulary::NAO::prefLabel(), tagName );
                tagList.append( basketTag );
            }
        }
        basketRes.setTags( tagList );

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
    } else {
        DEBUG_WIN << "\tinitialization failed!";
        return false;
    }
    return true;
}

bool nepomukintegration::deleteMetadata(const QString &fullPath) {

    if ( Nepomuk::ResourceManager::instance()->initialized() || Nepomuk::ResourceManager::instance()->init() ) {
        DEBUG_WIN << "\tinitialized";
        KUrl basketUri = KUrl( fullPath );
        Nepomuk::Resource basketRes(basketUri);
        basketRes.remove();
    } else {
        DEBUG_WIN << "\tinitialization failed!";
        return false;
    }
    return true;
}

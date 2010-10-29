#include "nepomukintegration.h"

#include <QCoreApplication>
#include <QThread>
#include <QtConcurrentRun>
#include <QPair>
#include <QMutex>
#include <QMutexLocker>
#include <QDomNode>
#include <QDomDocument>
#include <QDomElement>
#include <QDBusMessage>
#include <QDBusConnection>
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

nepomukUpdater * nepomukIntegration::updaterInstance = NULL;
QMutex nepomukIntegration::updaterInstanceMutex;

nepomukUpdater::nepomukUpdater() : QThread()
{
  idleTime = 1;
  connect( this, SIGNAL(updateCompleted(QString, bool)),
	   this, SLOT(checkQueue()),
	   Qt::QueuedConnection
	);
  /*QDBusConnection::sessionBus().connect( "org.kde.nepomuk.services.nepomukstrigiservice", "/nepomukstrigiservice", "", "indexingStopped",
                                         this, SLOT(cleanupRequestedIndexes()) );*/
  connect( this, SIGNAL(indexCleanupCompleted(KUrl)),
	   this, SLOT(cleanupRequestedIndexes()),
	   Qt::QueuedConnection
	);
  connect( this, SIGNAL(finished()),
	   this, SLOT(cleanupFinishedThread()),
	   Qt::QueuedConnection
	);
  connect( this, SIGNAL(terminated()),
	   this, SLOT(cleanupFinishedThread()),
	   Qt::QueuedConnection
	);
  moveToThread(this);
  //DEBUG
  DEBUG_WIN << "nepomukUpdater object constructed";
}

//DEBUG
nepomukUpdater::~nepomukUpdater() {
  DEBUG_WIN << "nepomukUpdater object destructed";
}

void nepomukUpdater::run()
{
  QTimer::singleShot(500, this, SLOT(doUpdate()));
  exec();
}

void nepomukUpdater::cleanupFinishedThread() {
  QtConcurrent::run(nepomukIntegration::releaseUpdaterInstance);
}

void nepomukIntegration::releaseUpdaterInstance()
{
  QMutexLocker lockerParent(&updaterInstanceMutex);
  //QMutexLocker lockerChild(&(updaterInstance->mutex));
  if ( updaterInstance->isEmpty() ) {
    if ( updaterInstance->isFinished() ) {
      //delete updaterInstance;
      updaterInstance->deleteLater();
      updaterInstance = NULL;
      DEBUG_WIN << "nepomukUpdater is set to NULL";
    } else {
      DEBUG_WIN << "nepomukUpdater queue is empty but it is still running!";
      DEBUG_WIN << "\t<font color=red>This should NOT happen!</font>";
      DEBUG_WIN << "\tquitting updaterInstance!";
      updaterInstance->quit();
    }
  } else {
    if ( updaterInstance->isFinished() ) {
      DEBUG_WIN << "nepomukUpdater queue is NOT empty but it is NOT running!";
      DEBUG_WIN << "\t<font color=red>This should NOT happen!</font>";
    }
  }
}




void nepomukUpdater::checkQueue() {
  //mutex.lock();
  QMutexLocker locker(&mutex);
  if ( basketList.isEmpty() ) {
    if ( idleTime >= nepomukUpdaterIdleTime && requestedIndexList.isEmpty() ) {
      quit();
      moveToThread(QCoreApplication::instance()->thread());
      //mutex.unlock();
      return;
    }
    idleTime *= 2;
    if ( idleTime > nepomukUpdaterIdleTime ) { idleTime = nepomukUpdaterIdleTime; }
    //mutex.unlock();
    QTimer::singleShot(idleTime*500, this, SLOT(checkQueue()));
  } else {
    //mutex.unlock();
    //doUpdate();
    QTimer::singleShot(500, this, SLOT(doUpdate()));
  }
  //I hope it is not needed :)
  //mutex.unlock();
}




void nepomukUpdater::queueBasket(QString fullPath, QDomDocument document) {
  basketPathAndDocumentPair element;
  QMutexLocker locker(&mutex);
  element.first  = fullPath;
  element.second = document;
  basketList << element;
}

void nepomukUpdater::queueIndexRequest(KUrl file) {
  QMutexLocker locker(&mutex);
  if ( requestedIndexList.isEmpty() ) {
    QDBusConnection::sessionBus().connect( "org.kde.nepomuk.services.nepomukstrigiservice", "/nepomukstrigiservice", "", "indexingStopped",
                                           this, SLOT(cleanupRequestedIndexes()) );
  }
  //If there is another one of this file in the queue, do not add it again
  KUrl indexedFile;
  foreach (indexedFile, requestedIndexList) {
    if ( indexedFile == file ) {
      return;
    }
  }
  requestedIndexList << file;
}

void nepomukIntegration::updateMetadata(const QString &fullPath, const QDomDocument &document) {
  QMutexLocker locker(&updaterInstanceMutex);
  if ( updaterInstance == NULL ) {
    updaterInstance = new nepomukUpdater();
  }
  updaterInstance->queueBasket(fullPath, document);
  if ( ! updaterInstance->isRunning() ) {
     updaterInstance->start(QThread::IdlePriority);
  }
}




void nepomukUpdater::cleanupRequestedIndexes() {
  //DEBUG
  DEBUG_WIN << "cleanupRequestedIndexes";
  mutex.lock();
  if ( requestedIndexList.isEmpty() ) {
    QDBusConnection::sessionBus().disconnect( "org.kde.nepomuk.services.nepomukstrigiservice", "/nepomukstrigiservice", "", "indexingStopped",
                                              this, SLOT(cleanupRequestedIndexes()) );
    mutex.unlock();
    DEBUG_WIN << "\tqueue is empty, desconnected from indexingStopped signal.";
    return;
  }
  idleTime = 1;
  KUrl indexedFile = requestedIndexList.takeFirst();
  mutex.unlock();
  
  DEBUG_WIN << "\tnote : " + indexedFile.pathOrUrl();
  Nepomuk::Resource noteRes(indexedFile);
  noteRes.setProperty( Soprano::Vocabulary::RDF::type(), Soprano::Vocabulary::PIMO::Note() );
  noteRes.setProperty( Soprano::Vocabulary::NIE::mimeType(), "application/x-basket-item" );
  
  //I would rather keep doUpdate and cleanupRequestedIndexes seprate
  emit indexCleanupCompleted(indexedFile);
}

void nepomukUpdater::doUpdate() {
  mutex.lock();
  if ( basketList.isEmpty() ) { return; }
  idleTime = 1;
  basketPathAndDocumentPair element = basketList.takeFirst();
  QString fullPath = element.first;
  QDomDocument document = element.second;
  //If there is another one of this basket item in the queue, that one will be processed later (instead of the current one)
  foreach (element, basketList) {
    if ( element.first == fullPath ) {
      mutex.unlock();
      emit updateCompleted(fullPath, false);
      return;
    }
  }
  mutex.unlock();
  
  if ( Nepomuk::ResourceManager::instance()->initialized() || Nepomuk::ResourceManager::instance()->init() ) {
    DEBUG_WIN << "\tNepomuk::ResourceManager::instance initialized";
    KUrl basketUri = KUrl( fullPath + ".basket" );
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

    QDBusMessage nepomukstrigiDBusMessage = QDBusMessage::createMethodCall("org.kde.nepomuk.services.nepomukstrigiservice", 
                                                                           "/nepomukstrigiservice", "", "indexFolder" );
    QList<QVariant> indexingArgs;
    KUrl noteUrl;
    indexingArgs.append(fullPath);
    indexingArgs.append(false);
    nepomukstrigiDBusMessage.setArguments(indexingArgs);
    if ( QDBusConnection::sessionBus().send(nepomukstrigiDBusMessage) ) {
      DEBUG_WIN << "\t\tindexing requested via DBus.";
    } else {
      DEBUG_WIN << "\t\tindexing request FAILED!";
    }

    QDomNodeList noteElements = document.elementsByTagName("note");
    for (int i = 0; i < noteElements.count(); i++) {
      QString noteContentFile = noteElements.item(i).firstChildElement().firstChild().nodeValue();
      noteUrl = KUrl( noteContentFile );
      if (noteUrl.isRelative() ) {
        noteUrl = KUrl( fullPath + noteContentFile );
      }
      DEBUG_WIN << "\tnote : " + noteContentFile /*+ " (" + noteUrl.pathOrUrl() + ")"*/ ;
      Nepomuk::Resource noteRes(noteUrl);
      noteRes.setProperty( Soprano::Vocabulary::RDF::type(), Soprano::Vocabulary::PIMO::Note() );
      noteRes.setProperty( Soprano::Vocabulary::NIE::mimeType(), "application/x-basket-item" );
      noteRes.setProperty( Soprano::Vocabulary::NIE::isPartOf(), basketRes );
      //TODO
      //Is this name OK?
      noteRes.setProperty( Soprano::Vocabulary::NIE::title(), basketName + " / " + noteContentFile);
      noteRes.setLabel( basketName + " / " + noteContentFile );
      
      queueIndexRequest( noteUrl );
    }
    
    //DEBUG
    /*
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
    */
  } else {
    DEBUG_WIN << "\tNepomuk::ResourceManager::instance initialization failed!";
    emit updateCompleted(fullPath, false);
    return;
  }
  emit updateCompleted(fullPath, true);
  return;
}

bool nepomukIntegration::deleteMetadata(const QString &fullPath) {

    if ( Nepomuk::ResourceManager::instance()->initialized() || Nepomuk::ResourceManager::instance()->init() ) {
        DEBUG_WIN << "\tNepomuk::ResourceManager::instance initialized";
        KUrl basketUri = KUrl( fullPath );
        Nepomuk::Resource basketRes(basketUri);
        basketRes.remove();
    } else {
        DEBUG_WIN << "\tNepomuk::ResourceManager::instance initialization failed!";
        return false;
    }
    return true;
}

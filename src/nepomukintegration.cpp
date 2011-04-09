#include "nepomukintegration.h"

#include <QCoreApplication>
#include <QThread>
#include <QtConcurrentRun>
#include <QMutex>
#include <QMutexLocker>
#include <QDomNode>
#include <QDomDocument>
#include <QDomElement>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QFileInfo>
#include <KUrl>
#include <KDE/KDebug>
#include <QEventLoop>
#include <KDE/KApplication>


#include "global.h"
#include "tag.h"
#include "debugwindow.h"
#include "basketview.h"

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

nepomukIntegration * nepomukIntegration::instance = NULL;
QMutex nepomukIntegration::instanceMutex;


/*
 *                    Constructor / Cleanup
 */

nepomukIntegration::nepomukIntegration(BasketView * basket, int idleTime = 15000) : QObject()
        , idleTime(idleTime), workerThread(this), cleanupTimer(this), mutex()
        , basketList(), isDoingUpdate(true), requestedIndexList(), isCleaningupRequestedIndexes(false)
{
    connect( this, SIGNAL(updateCompleted(QString, bool)),
             this, SLOT(checkQueue()),
             Qt::QueuedConnection
             );
    connect( &cleanupTimer, SIGNAL(timeout()),
             this, SLOT(cleanup()),
             Qt::QueuedConnection
             );

    connect( &workerThread, SIGNAL(finished()),
             this, SLOT(deleteLater()),
             Qt::QueuedConnection
             );
    connect( &workerThread, SIGNAL(terminated()),
             this, SLOT(deleteLater()),
             Qt::QueuedConnection
             );

    QMutexLocker locker(&mutex);
    basketList << basket;

    moveToThread(&workerThread);
    QTimer::singleShot(500, this, SLOT(doUpdate()));
    workerThread.start(QThread::IdlePriority);

    DEBUG_WIN << "nepomukIntegration object constructed";
}


void nepomukIntegration::cleanup() {
    DEBUG_WIN << "cleanup: starting";
    QMutexLocker locker(&instanceMutex);
    //Keep the timer active for a long time
    cleanupTimer.start(idleTime*100);

    if ( ! basketList.isEmpty() ) {
        if ( ! isDoingUpdate ) {
            DEBUG_WIN << "<font color='red'>basketList is NOT empty but isDoingUpdate is false!</font>";
            QTimer::singleShot(500, this, SLOT(checkQueue()));
        }
        DEBUG_WIN << "cleanup: deactivating cleanupTimer and returning.";
        cleanupTimer.stop();
        return;
    }
    if ( ! requestedIndexList.isEmpty() ) {
        if ( ! isCleaningupRequestedIndexes ) {
            DEBUG_WIN << "cleanup: requestedIndexList is NOT empty";
            QDBusInterface * nepomukstrigiDBusInterface = new QDBusInterface( "org.kde.nepomuk.services.nepomukstrigiservice",
                                                                              "/nepomukstrigiservice", "", QDBusConnection::sessionBus() , this
                                                                              );
            QDBusReply<bool> isIndexing = nepomukstrigiDBusInterface->call("isIndexing");
            if ( isIndexing.isValid() && ! isIndexing ) {
                DEBUG_WIN << "cleanup: nepomukstrigiservice is NOT indexing, going to run cleanupRequestedIndexes.";
                QDBusConnection::sessionBus().disconnect(
                        "org.kde.nepomuk.services.nepomukstrigiservice", "/nepomukstrigiservice", "", "indexingStopped",
                        this, SLOT(cleanupRequestedIndexes())
                        );
                cleanupRequestedIndexes();
            } else if ( ! isDoingUpdate ) {
                DEBUG_WIN << "requestedIndexList is NOT empty, nepomukstrigiservice isIndexing is true and isDoingUpdate is false.";
                DEBUG_WIN << "Scheduling checkQueue.";
                QTimer::singleShot(500, this, SLOT(checkQueue()));
            }
        }
        DEBUG_WIN << "cleanup: deactivating cleanupTimer and returning.";
        cleanupTimer.stop();
        return;
    }
    instance = NULL;
    DEBUG_WIN << "nepomukIntegration instance is set to NULL";
    workerThread.quit();
    moveToThread(QCoreApplication::instance()->thread());
}


/*
 *                    Queue baskets and indexRequests / checkQueue
 */

void nepomukIntegration::updateMetadata(BasketView * basket) {
    DEBUG_WIN << "updateMetadata: Going to lock updaterInstanceMutex";
    QMutexLocker locker(&instanceMutex);
    if ( instance == NULL ) {
        instance = new nepomukIntegration(basket);
    } else {
        instance->queueBasket(basket);
    }
    DEBUG_WIN << "updateMetadata: Done";
}

void nepomukIntegration::checkQueue() {
    DEBUG_WIN << "checkQueue: Going to lock";
    QMutexLocker locker(&mutex);
    if ( basketList.isEmpty() ) {
        if ( ! cleanupTimer.isActive() ) {
            if ( requestedIndexList.isEmpty() ) {
                cleanupTimer.start(idleTime);
                DEBUG_WIN << "checkQueue: cleanupTimer started";
            } else if ( ! isCleaningupRequestedIndexes ) {
                cleanupTimer.start(idleTime*10);
                DEBUG_WIN << "checkQueue: requestedIndexList is NOT empty, cleanupTimer started with an interval of idleTime x 10";
            }
        }
        isDoingUpdate = false;
    } else {
        cleanupTimer.stop();
        isDoingUpdate = true;
        QTimer::singleShot(500, this, SLOT(doUpdate()));
    }
    DEBUG_WIN << "checkQueue: Done";
}

void nepomukIntegration::queueBasket(BasketView * basket) {
    DEBUG_WIN << "queueBasket: Going to lock";
    QMutexLocker locker(&mutex);
    basketList << basket;
    if ( ! isDoingUpdate ) {
        cleanupTimer.stop();
        isDoingUpdate = true;
        QTimer::singleShot(500, this, SLOT(doUpdate()));
        DEBUG_WIN << "queueBasket : doUpdate scheduled.";
    }
    DEBUG_WIN << "queueBasket: Done";
}

void nepomukIntegration::queueIndexRequest(KUrl file) {
    DEBUG_WIN << "queueIndexRequest: Going to lock";
    QMutexLocker locker(&mutex);
    if ( requestedIndexList.isEmpty() ) {
        QDBusConnection::sessionBus().connect(
                "org.kde.nepomuk.services.nepomukstrigiservice", "/nepomukstrigiservice", "", "indexingStopped",
                this, SLOT(cleanupRequestedIndexes())
                );
        isCleaningupRequestedIndexes = false;
    }
    //If there is another one of this file in the queue, do not add it again
    KUrl indexedFile;
    foreach (indexedFile, requestedIndexList) {
        if ( indexedFile == file ) {
            DEBUG_WIN << "queueIndexRequest: (duplicate) Done";
            return;
        }
    }
    requestedIndexList << file;
    DEBUG_WIN << "queueIndexRequest: Done";
}


/*
 *                    Dequeue / Process baskets and indexRequests
 */

void nepomukIntegration::cleanupRequestedIndexes() {
    DEBUG_WIN << "cleanupRequestedIndexes: Going to lock";
    if ( ! mutex.tryLock() ) {
        DEBUG_WIN << "<font color='red'>cleanupRequestedIndexes could NOT get the lock!</font>";
        DEBUG_WIN << "<font color='red'>Scheduling for a later execution and Returning!</font>";
        QTimer::singleShot(1000, this, SLOT(cleanupRequestedIndexes()));
        return;
    }
    if ( requestedIndexList.isEmpty() ) {
        QDBusConnection::sessionBus().disconnect(
                "org.kde.nepomuk.services.nepomukstrigiservice", "/nepomukstrigiservice", "", "indexingStopped",
                this, SLOT(cleanupRequestedIndexes())
                );
        mutex.unlock();
        isCleaningupRequestedIndexes = false;
        DEBUG_WIN << "\tqueue is empty, desconnected from indexingStopped signal and isCleaningupRequestedIndexes set to false.";
        if ( ! isDoingUpdate ) {
            //Make sure the timer is not active so that the queues are rechecked
            //             and timeout of the timer is reevaluated and restarted
            cleanupTimer.stop();
            QTimer::singleShot(500, this, SLOT(checkQueue()));
        }
        DEBUG_WIN << "cleanupRequestedIndexes: Done";
        return;
    }
    isCleaningupRequestedIndexes = true;
    KUrl indexedFile = requestedIndexList.takeFirst();
    mutex.unlock();
    DEBUG_WIN << "cleanupRequestedIndexes: unlocked";

    DEBUG_WIN << "cleanupRequestedIndexes: \tnote : " << indexedFile.pathOrUrl();
    Nepomuk::Resource noteRes(indexedFile);
    noteRes.setProperty( Soprano::Vocabulary::RDF::type(), Soprano::Vocabulary::PIMO::Note() );
    noteRes.setProperty( Soprano::Vocabulary::NIE::mimeType(), "application/x-basket-item" );

    QTimer::singleShot(500, this, SLOT(cleanupRequestedIndexes()));
    DEBUG_WIN << "cleanupRequestedIndexes: Done. Returning";
}

void listAllNotes(Note * note, QString basketFolderAbsolutePath, QList<QString> & noteFileList) {
    while (note) {
        if ( note->isGroup() ) {
            listAllNotes(note->firstChild(), basketFolderAbsolutePath, noteFileList);
        } else if ( note->content()->useFile() ) {
            noteFileList << basketFolderAbsolutePath + note->content()->fileName();
        }
        note = note->next();
    }
}


void nepomukIntegration::doUpdate() {
    DEBUG_WIN << "doUpdate: Going to lock";
    mutex.lock();
    if ( basketList.isEmpty() ) {
        DEBUG_WIN << "<font color='red'>doUpdate should not be run with an empty basketList! Returning!</font>";
        return;
    }
    BasketView * basket = basketList.takeFirst();
    QString basketFolderName = basket->folderName();
    QString basketFolderAbsolutePath = Global::basketsFolder() + basketFolderName;
    QString basketName = basket->basketName();
    //If there is another one of this basket item in the queue, that one will be processed later (instead of the current one)
    BasketView * tmpBasket;
    foreach (tmpBasket, basketList) {
        if ( basketFolderName == tmpBasket->folderName() ) {
            mutex.unlock();
            printf("doUpdate: \tDuplicate, unlocked\n"); fflush(stdout);fflush(stderr);
            emit updateCompleted(basketFolderName, false);
            return;
        }
    }
    mutex.unlock();
    DEBUG_WIN << "doUpdate: \tUnlocked";
    QFileInfo basketDirInfo(basketFolderAbsolutePath);
    if ( ! basketDirInfo.isDir() ) {
        DEBUG_WIN << "<font color='red'>Global::basketsFolder() + basket->folderName() does not yield in a valid dir! Returning!</font>";
        emit updateCompleted(basketFolderName, false);
        return;
    }
    DEBUG_WIN << "Indexing (" << basketFolderName << "): " << basketName ;
    if ( Nepomuk::ResourceManager::instance()->initialized() || Nepomuk::ResourceManager::instance()->init() ) {
        DEBUG_WIN << "\tNepomuk::ResourceManager::instance initialized";
        KUrl basketUri = KUrl( basketFolderAbsolutePath + ".basket" );
        /* Nepomuk::File basketRes(basketUri); */
        Nepomuk::Resource basketRes(basketUri);
        /*addType works better: basketRes.setProperty( Soprano::Vocabulary::RDF::type(), Soprano::Vocabulary::PIMO::Note() ); */
        basketRes.addType( Soprano::Vocabulary::PIMO::Note() );
        /* Added by Strigi, no need: basketRes.addType( Soprano::Vocabulary::NFO::FileDataObject() ); */
        basketRes.setProperty( Soprano::Vocabulary::NIE::mimeType(), "application/x-basket-item" );
        /* nfo:fileUrl is deprecated in favor of nie:url: basketRes.setProperty( Soprano::Vocabulary::NFO::fileUrl(), basketUri ); */
        /* This is done internally anyway: basketRes.setProperty( Soprano::Vocabulary::NIE::url(), basketUri ); */
        basketRes.setProperty( Soprano::Vocabulary::NFO::fileName(), ".basket" );
        basketRes.setProperty( Soprano::Vocabulary::NIE::title(), basketName);
        basketRes.setLabel( basketName );

        QList<Tag*> usedTagsList;
        basket->listUsedTags(usedTagsList);
        Tag * tmpTag;
        Nepomuk::Tag basketTag;
        QList<Nepomuk::Tag> tagList;
        foreach (tmpTag, usedTagsList) {
            basketTag = Nepomuk::Tag( tmpTag->name() );
            basketTag.setLabel( tmpTag->name() );
            tagList.append( basketTag );
        }
        basketRes.setTags( tagList );
        DEBUG_WIN << "doUpdate: \tTags are set";
        DEBUG_WIN << "doUpdate: \tQueuing note names:";

        QList<QString> basketFileList;
        Note *note = basket->firstNote();
        if (! note ) {
            DEBUG_WIN << "doUpdate: \tHas NO notes!";
        } else {
            listAllNotes(note, basketFolderAbsolutePath, basketFileList);
        }
        basketFileList << basketFolderAbsolutePath + ".basket";
        QString noteContentFile;
        KUrl noteUrl;
        foreach (noteContentFile, basketFileList) {
            DEBUG_WIN << "doUpdate: \t  noteContentFile: " << noteContentFile;
            noteUrl = KUrl( noteContentFile );
            if ( noteUrl.isRelative() ) {
                noteUrl = KUrl( Global::basketsFolder() + "/" + noteContentFile );
            }
            if ( ! noteUrl.isValid() || ! noteUrl.isLocalFile()
                || ! QFile::exists(noteUrl.path()) || ! QFileInfo(noteUrl.path()).isFile() ) {
                DEBUG_WIN << "\tnote : <font color='red'>" + noteContentFile + " NOT indexed!</font>" ;
                continue;
            }
            Nepomuk::Resource noteRes(noteUrl);
            noteRes.addType( Soprano::Vocabulary::PIMO::Note() );
            noteRes.setProperty( Soprano::Vocabulary::NIE::mimeType(), "application/x-basket-item" );
            noteRes.setProperty( Soprano::Vocabulary::PIMO::partOf(), basketRes );
            //TODO
            //Is this name OK?
            noteRes.setProperty( Soprano::Vocabulary::NIE::title(), basketName + " (" + noteContentFile + ")" );
            noteRes.setLabel( basketName + " (" + noteContentFile + ")" );

            queueIndexRequest( noteUrl );
        }
        DEBUG_WIN << "doUpdate: Note queuing completed, now going to request indexing from nepomukstrigiservice:";
        QDBusMessage nepomukstrigiDBusMessage =
                QDBusMessage::createMethodCall( "org.kde.nepomuk.services.nepomukstrigiservice",
                                                "/nepomukstrigiservice", "", "indexFolder"
                                                );
        DEBUG_WIN << "doUpdate: \tnepomukstrigiDBusMessage created";
        QList<QVariant> indexingArgs;
        indexingArgs.append(basketFolderAbsolutePath);
        indexingArgs.append(false);
        nepomukstrigiDBusMessage.setArguments(indexingArgs);
        DEBUG_WIN << "doUpdate: \tnepomukstrigiDBusMessage prepared";
        if ( QDBusConnection::sessionBus().send(nepomukstrigiDBusMessage) ) {
            DEBUG_WIN << "doUpdate: \t\tindexing requested via DBus.";
        } else {
            DEBUG_WIN << "doUpdate: \t\tindexing request FAILED!";
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
        emit updateCompleted(basketFolderName, false);
        return;
    }
    DEBUG_WIN << "doUpdate: \tDone";
    emit updateCompleted(basketFolderName, true);
    return;
}

bool nepomukIntegration::doDelete(const QString &fullPath) {
    DEBUG_WIN << "doDelete: going to lock";
    instanceMutex.lock();
    if ( instance != NULL ) {
        DEBUG_WIN << "doDelete: cleanup any instance of " << fullPath;
        QString basketsFolder = Global::basketsFolder();
        int i;
        for ( i=0; i<instance->basketList.count(); i++ ) {
            if( basketsFolder + instance->basketList.at(i)->folderName() == fullPath ) {
                instance->basketList.removeAt(i);
            }
        }
        instance->requestedIndexList.removeAll(KUrl(fullPath));
    }
    instanceMutex.unlock();

    if ( Nepomuk::ResourceManager::instance()->initialized() || Nepomuk::ResourceManager::instance()->init() ) {
        DEBUG_WIN << "\tNepomuk::ResourceManager::instance initialized";
        KUrl basketUri = KUrl( fullPath );
        Nepomuk::Resource basketRes(basketUri);
        basketRes.remove();
    } else {
        DEBUG_WIN << "\tNepomuk::ResourceManager::instance initialization failed!";
        return false;
    }
    DEBUG_WIN << "doDelete: Done";
    return true;
}

void nepomukIntegration::deleteMetadata(const QString &fullPath) {
    //Only process files in Global::basketsFolder()
    if ( fullPath.contains(Global::basketsFolder()) )
        QtConcurrent::run(doDelete, fullPath);
}

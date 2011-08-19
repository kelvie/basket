#ifndef NEPOMUKINTEGRATION_H
#define NEPOMUKINTEGRATION_H

#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QTimer>

//For DEBUG_WIN:
#include "global.h"
#include "debugwindow.h"

class QString;
class KUrl;
class BasketScene;

class nepomukIntegration : public QObject
{
    Q_OBJECT
public:
    static void updateMetadata(BasketScene * basket);
    static void deleteMetadata(const QString &fullPath);
    static bool doDelete(const QString &fullPath);
private:
    static nepomukIntegration *instance;
    static QMutex instanceMutex;

    nepomukIntegration(BasketScene * basket, int idleTime);
    ~nepomukIntegration() {
        //I hope deletion is handled automatically
        //delete workerThread;
        //delete cleanupIdle;
        DEBUG_WIN << "nepomukUpdater object destructed";
    }

    int idleTime;
    QThread workerThread;
    QTimer cleanupTimer;
    QMutex mutex;
    QList<BasketScene *> basketList;
    bool isDoingUpdate;
    QList<KUrl> requestedIndexList;
    bool isCleaningupRequestedIndexes;
    void queueBasket(BasketScene * basket);
    void queueIndexRequest(KUrl file);
signals:
    void updateCompleted(QString basketFolderName, bool successful);
    //void indexCleanupCompleted(KUrl file);
private slots:
    void doUpdate();
    void checkQueue();
    void cleanupRequestedIndexes();
    void cleanup();

};

#endif // NEPOMUKINTEGRATION_H

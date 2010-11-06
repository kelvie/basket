#ifndef NEPOMUKINTEGRATION_H
#define NEPOMUKINTEGRATION_H

#include <QThread>
#include <QPair>
#include <QMutex>
#include <QTimer>
#include <QString>
#include <QDomDocument>
#include <KUrl>

#include "basketview.h"
#include "debugwindow.h"

class nepomukIntegration : public QObject
{
    Q_OBJECT
public:
    static void updateMetadata(BasketView * basket);
    static void deleteMetadata(const QString &fullPath);
    static bool doDelete(const QString &fullPath);
private:
    static nepomukIntegration *instance;
    static QMutex instanceMutex;

    nepomukIntegration(BasketView * basket, int idleTime);
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
    QList<BasketView *> basketList;
    bool isDoingUpdate;
    QList<KUrl> requestedIndexList;
    bool isCleaningupRequestedIndexes;
    void queueBasket(BasketView * basket);
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

#ifndef NEPOMUKINTEGRATION_H
#define NEPOMUKINTEGRATION_H

#include <QThread>
#include <QPair>
#include <QMutex>
#include <QTimer>
#include <QString>
#include <QDomDocument>
#include <KUrl>

#define nepomukUpdaterIdleTime  30

class nepomukUpdater : public QThread
{
  Q_OBJECT
public:
  void queueBasket(QString fullPath, QDomDocument document);
  bool isEmpty() { return ( basketList.isEmpty() && requestedIndexList.isEmpty() ); }
  nepomukUpdater();
  ~nepomukUpdater();
protected:
  void run();
private:
  QMutex mutex;
  typedef QPair<QString, QDomDocument> basketPathAndDocumentPair;
  QList<basketPathAndDocumentPair> basketList;
  unsigned int idleTime;
  QList<KUrl> requestedIndexList;
  void queueIndexRequest(KUrl file);
signals:
  void updateCompleted(QString fullPath, bool successful);
  void indexCleanupCompleted(KUrl file);
private slots:
  void doUpdate();
  void checkQueue();
  void cleanupRequestedIndexes();
  void cleanupFinishedThread();
};

class nepomukIntegration
{
private:
  //nepomukIntegration(){};
  static nepomukUpdater *updaterInstance;
public:
  static QMutex updaterInstanceMutex;
  static void releaseUpdaterInstance();
  static void updateMetadata(const QString &fullPath, const QDomDocument &document);
  static bool deleteMetadata(const QString &fullPath);
};

#endif // NEPOMUKINTEGRATION_H

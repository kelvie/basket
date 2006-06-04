/*
 * This file has been took from KDE 3.2.x because it was working
 * and KDE 3.3.x break it.
 * Hopefully, KDE 4.x will provide a working KDirWatch and this "fork"
 * could be dropped.
 * See http://bugs.kde.org/show_bug.cgi?id=85989 for more informations.
 * Files copy concern kdirwatch.h, kdirwatch_p.h and kdirwatch.cpp
 */

/* Private Header for class of KDirWatchPrivate
 *
 * this separate header file is needed for MOC processing
 * because KDirWatchPrivate has signals and slots
 */

#ifndef _KDIRWATCH_P_H
#define _KDIRWATCH_P_H

/********MODIFIED*/
#include "have_fam.h"
#define HAVE_DNOTIFY 1
/*****************/

#ifdef HAVE_FAM
#include <fam.h>
#endif

#include <ctime>

#define invalid_ctime ((time_t)-1)

/* KDirWatchPrivate is a singleton and does the watching
 * for every KDirWatch instance in the application.
 */
class KDirWatchPrivate : public QObject
{
  Q_OBJECT
public:

  enum entryStatus { Normal = 0, NonExistent };
  enum entryMode { UnknownMode = 0, StatMode, DNotifyMode, FAMMode };
  enum { NoChange=0, Changed=1, Created=2, Deleted=4 };

  struct Client {
    KDirWatch* instance;
    int count;
    // did the instance stop watching
    bool watchingStopped;
    // events blocked when stopped
    int pending;
  };

  class Entry
  {
  public:
    // the last observed modification time
    time_t m_ctime;
    // the last observed link count
    int m_nlink;
    entryStatus m_status;
    entryMode m_mode;
    bool isDir;
    // instances interested in events
    QPtrList<Client> m_clients;
    // nonexistent entries of this directory
    QPtrList<Entry> m_entries;
    QString path;

    int msecLeft, freq;

    void addClient(KDirWatch*);
    void removeClient(KDirWatch*);
    int clients();
    bool isValid() { return m_clients.count() || m_entries.count(); }

#ifdef HAVE_FAM
    FAMRequest fr;
#endif

#ifdef HAVE_DNOTIFY
    int dn_fd;
    bool dn_dirty;
    void propagate_dirty();
#endif
  };

  typedef QMap<QString,Entry> EntryMap;

  KDirWatchPrivate();
  ~KDirWatchPrivate();

  void resetList (KDirWatch*,bool);
  void useFreq(Entry* e, int newFreq);
  void addEntry(KDirWatch*,const QString&, Entry*, bool);
  void removeEntry(KDirWatch*,const QString&, Entry*);
  bool stopEntryScan(KDirWatch*, Entry*);
  bool restartEntryScan(KDirWatch*, Entry*, bool );
  void stopScan(KDirWatch*);
  void startScan(KDirWatch*, bool, bool);

  void removeEntries(KDirWatch*);
  void statistics();

  Entry* entry(const QString&);
  int scanEntry(Entry* e);
  void emitEvent(Entry* e, int event, const QString &fileName = QString::null);

  // Memory management - delete when last KDirWatch gets deleted
  void ref() { m_ref++; }
  bool deref() { return ( --m_ref == 0 ); }

public slots:
  void slotRescan();
  void famEventReceived(); // for FAM
  void slotActivated(); // for DNOTIFY
  void slotRemoveDelayed();

public:
  QTimer *timer;
  EntryMap m_mapEntries;

  int freq;
  int statEntries;
  int m_nfsPollInterval, m_PollInterval;
  int m_ref;
  bool useStat(Entry*);

  bool delayRemove;
  QPtrList<Entry> removeList;

#ifdef HAVE_FAM
  QSocketNotifier *sn;
  FAMConnection fc;
  bool use_fam;

  void checkFAMEvent(FAMEvent*);
  bool useFAM(Entry*);
#endif

#ifdef HAVE_DNOTIFY
  bool supports_dnotify;
  bool rescan_all;
  int mPipe[2];
  QTimer mTimer;
  QSocketNotifier *mSn;
  QIntDict<Entry> fd_Entry;

  static void dnotify_handler(int, siginfo_t *si, void *);
  static void dnotify_sigio_handler(int, siginfo_t *si, void *);
  bool useDNotify(Entry*);
#endif
};

#endif // KDIRWATCH_P_H


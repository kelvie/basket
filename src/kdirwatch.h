/*
 * This file has been took from KDE 3.2.x because it was working
 * and KDE 3.3.x break it.
 * Hopefully, KDE 4.x will provide a working KDirWatch and this "fork"
 * could be dropped.
 * See http://bugs.kde.org/show_bug.cgi?id=85989 for more informations.
 * Files copy concern kdirwatch.h, kdirwatch_p.h and kdirwatch.cpp
 */

/* This file is part of the KDE libraries
   Copyright (C) 1998 Sven Radej <sven@lisa.exp.univie.ac.at>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef _KDIRWATCH_H
#define _KDIRWATCH_H

#include <qtimer.h>
#include <qdatetime.h>
#include <qmap.h>

#define kdirwatch KDirWatch::self()

class KDirWatchPrivate;

 /**
  * Watch directories and files for changes.
  * The watched directories or files don't have to exist yet.
  *
  * When a watched directory is changed, i.e. when files therein are
  * created or deleted, KDirWatch will emit the signal dirty().
  *
  * When a watched, but previously not existing directory gets created,
  * KDirWatch will emit the signal created().
  *
  * When a watched directory gets deleted, KDirWatch will emit the
  * signal deleted(). The directory is still watched for new
  * creation.
  *
  * When a watched file is changed, i.e. attributes changed or written
  * to, KDirWatch will emit the signal dirty().
  *
  * Scanning of particular directories or files can be stopped temporarily
  * and restarted. The whole class can be stopped and restarted.
  * Directories and files can be added/removed from the list in any state.
  *
  * The implementation uses the FAM service when available;
  * if FAM is not available, the DNOTIFY functionality is used on LINUX.
  * As a last resort, a regular polling for change of modification times
  * is done; the polling interval is a global config option:
  * DirWatch/PollInterval and DirWatch/NFSPollInterval for NFS mounted
  * directories.
  *
  * @see self()
  * @short Class for watching directory and file changes.
  * @author Sven Radej <sven@lisa.exp.univie.ac.at>
  */
class KDirWatch : public QObject
{
  Q_OBJECT

  public:
   /**
    * Constructor.
    *
    * Scanning begins immediately when a dir/file watch
    * is added.
    * @param parent the parent of the QObject (or 0 for parent-less KDataTools)
    * @param name the name of the QObject, can be 0
    */
   KDirWatch (QObject* parent = 0, const char* name = 0);

   /**
    * Destructor.
    *
    * Stops scanning and cleans up.
    */
   ~KDirWatch();

   /**
    * Adds a directory to be watched.
    *
    * The directory does not have to exist. When @p watchFiles is
    * false (the default), the signals dirty(), created(), deleted()
    * can be emitted, all for the watched directory.
    * When @p watchFiles is true, all files in the watched directory
    * are watched for changes, too. Thus, the signals dirty(),
    * created(), deleted() can be emitted.
    *
    * @param path the path to watch
    * @param watchFiles if true, the KDirWatch will also watch files
    * @param recursive if true, all sub directories are also watched
    */
   void addDir(const QString& path,
	       bool watchFiles = false, bool recursive = false);

   /**
    * Adds a file to be watched.
    * @param file the file to watch
    */
   void addFile(const QString& file);

   /**
    * Returns the time the directory/file was last changed.
    * @param path the file to check
    * @return the date of the last modification
    */
   QDateTime ctime(const QString& path);

   /**
    * Removes a directory from the list of scanned directories.
    *
    * If specified path is not in the list this does nothing.
    * @param path the path of the dir to be removed from the list
    */
   void removeDir(const QString& path);

   /**
    * Removes a file from the list of watched files.
    *
    * If specified path is not in the list this does nothing.
    * @param file the file to be removed from the list
    */
   void removeFile(const QString& file);

   /**
    * Stops scanning the specified path.
    *
    * The @p path is not deleted from the interal just, it is just skipped.
    * Call this function when you perform an huge operation
    * on this directory (copy/move big files or many files). When finished,
    * call restartDirScan(path).
    *
    * @param path the path to skip
    * @return true if the @p path is being watched, otherwise false
    * @see restartDirScanning()
    */
   bool stopDirScan(const QString& path);

   /**
    * Restarts scanning for specified path.
    *
    * Resets ctime. It doesn't notify
    * the change (by emitted a signal), since the ctime value is reset.
    *
    * Call it when you are finished with big operations on that path,
    * @em and when @em you have refreshed that path.
    *
    * @param path the path to restart scanning
    * @return true if the @p path is being watched, otherwise false
    * @see stopDirScanning()
    */
   bool restartDirScan(const QString& path);

   /**
    * Starts scanning of all dirs in list.
    *
    * @param notify If true, all changed directories (since
    * stopScan() call) will be notified for refresh. If notify is
    * false, all ctimes will be reset (except those who are stopped,
    * but only if @p skippedToo is false) and changed dirs won't be
    * notified. You can start scanning even if the list is
    * empty. First call should be called with @p false or else all
    * directories
    * in list will be notified.
    * @param skippedToo if true, the skipped directoris (scanning of which was
    * stopped with stopDirScan() ) will be reset and notified
    * for change. Otherwise, stopped directories will continue to be
    * unnotified.
    */
   void startScan( bool notify=false, bool skippedToo=false );

   /**
    * Stops scanning of all directories in internal list.
    *
    * The timer is stopped, but the list is not cleared.
    */
   void stopScan();

   /**
    * Is scanning stopped?
    * After creation of a KDirWatch instance, this is false.
    * @return true when scanning stopped
    */
   bool isStopped() { return _isStopped; }

   /**
    * Check if a directory is being watched by this KDirWatch instance
    * @param path the directory to check
    * @return true if the directory is being watched
    */
   bool contains( const QString& path ) const;

   /**
    * Dump statistic information about all KDirWatch instances.
    * This checks for consistency, too.
    */
   static void statistics();

   /**
    * Emits created().
    * @param path the path of the file or directory
    */
   void setCreated( const QString &path );
   /**
    * Emits dirty().
    * @param path the path of the file or directory
    */
   void setDirty( const QString &path );
   /**
    * Emits deleted().
    * @param path the path of the file or directory
    */
   void setDeleted( const QString &path );

   enum Method { FAM, DNotify, Stat };
   /**
    * Returns the preferred internal method to
    * watch for changes.
    * @since 3.2
    */
   Method internalMethod();

   /**
    * The KDirWatch instance usually globally used in an application.
    * It is automatically deleted when the application exits.
    *
    * However, you can create an arbitrary number of KDirWatch instances
    * aside from this one - for those you have to take care of memory management.
    *
    * This function returns an instance of KDirWatch. If there is none, it
    * will be created.
    *
    * @return a KDirWatch instance
    */
   static KDirWatch* self();
   /**
    * Returns true if there is an instance of KDirWatch.
    * @return true if there is an instance of KDirWatch.
    * @see KDirWatch::self()
    * @since 3.1
    */
   static bool exists();

 signals:

   /**
    * Emitted when a watched object is changed.
    * For a directory this signal is emitted when files
    * therein are created or deleted.
    * For a file this signal is emitted when its size or attributes change.
    *
    * When you watch a directory, changes in the size or attributes of
    * contained files may or may not trigger this signal to be emitted
    * depending on which backend is used by KDirWatch.
    *
    * The new ctime is set before the signal is emitted.
    * @param path the path of the file or directory
    */
   void dirty (const QString &path);

   /**
    * Emitted when a file or directory is created.
    * @param path the path of the file or directory
    */
   void created (const QString &path );

   /**
    * Emitted when a file or directory is deleted.
    *
    * The object is still watched for new creation.
    * @param path the path of the file or directory
    */
   void deleted (const QString &path );

 private:
   bool _isStopped;

   KDirWatchPrivate *d;
   static KDirWatch* s_pSelf;
};

#endif

// vim: sw=3 et

/***************************************************************************
 *   Copyright (C) 2003 by Sébastien Laoût                                 *
 *   slaout@linux62.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef BACKUP_H
#define BACKUP_H

#include <KDE/KDialog>

#include <QtCore/QThread>

class QApplication;
class QLabel;

#include "basket_export.h"

/**
 * @author Sébastien Laoût
 */
class BackupDialog : public KDialog
{
    Q_OBJECT
public:
    BackupDialog(QWidget *parent = 0, const char *name = 0);
    ~BackupDialog();
private slots:
    void moveToAnotherFolder();
    void useAnotherExistingFolder();
    void backup();
    void restore();
    void populateLastBackup();
private:
    QLabel *m_lastBackup;
};

/**
 * @author Sébastien Laoût <slaout@linux62.org>
 */
class BASKET_EXPORT Backup
{
public:
    static void figureOutBinaryPath(const char *argv0, QApplication &app);
    static void setFolderAndRestart(const QString &folder, const QString &message);
    static QString newSafetyFolder();

private:
    static QString binaryPath;
};

class BackupThread : public QThread
{
public:
    BackupThread(const QString &tarFile, const QString &folderToBackup);
protected:
    virtual void run();
private:
    QString m_tarFile;
    QString m_folderToBackup;
};

class RestoreThread : public QThread
{
public:
    RestoreThread(const QString &tarFile, const QString &destFolder);
    inline bool success() {
        return m_success;
    }
protected:
    virtual void run();
private:
    QString m_tarFile;
    QString m_destFolder;
    bool m_success;
};

#endif // BACKUP_H

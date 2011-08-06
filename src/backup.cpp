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

#include "backup.h"

#include "global.h"
#include "variouswidgets.h"
#include "settings.h"
#include "tools.h"
#include "formatimporter.h" // To move a folder

#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QProgressBar>

#include <KDE/KLocale>
#include <KDE/KApplication>
#include <KDE/KAboutData>
#include <KDE/KDirSelectDialog>
#include <KDE/KRun>
#include <KDE/KConfig>
#include <KDE/KTar>
#include <KDE/KFileDialog>
#include <KDE/KProgressDialog>
#include <KDE/KMessageBox>
#include <KDE/KVBox>

#include <unistd.h> // usleep()


/**
 * Backups are wrapped in a .tar.gz, inside that folder name.
 * An archive is not a backup or is corrupted if data are not in that folder!
 */
const QString backupMagicFolder = "BasKet-Note-Pads_Backup";

/** class BackupDialog: */

BackupDialog::BackupDialog(QWidget *parent, const char *name)
        : KDialog(parent)
{
    setObjectName(name);
    setModal(true);
    setCaption(i18n("Backup & Restore"));
    setButtons(KDialog::Close);
    setDefaultButton(KDialog::Close);
    showButtonSeparator(false);

    KVBox *page  = new KVBox(this);
    setMainWidget(page);

//  page->setSpacing(spacingHint());

    QString savesFolder = Global::savesFolder();
    savesFolder = savesFolder.left(savesFolder.length() - 1); // savesFolder ends with "/"

    QGroupBox *folderGroup = new QGroupBox(i18n("Save Folder"), page);
    QVBoxLayout* folderGroupLayout = new QVBoxLayout;
    folderGroup->setLayout(folderGroupLayout);
    folderGroupLayout->addWidget(new QLabel("<qt><nobr>" + i18n("Your baskets are currently stored in that folder:<br><b>%1</b>", savesFolder), folderGroup));
    QWidget *folderWidget = new QWidget;
    folderGroupLayout->addWidget(folderWidget);

    QHBoxLayout *folderLayout = new QHBoxLayout(folderWidget);
    folderLayout->setContentsMargins(0, 0, 0, 0);

    QPushButton *moveFolder = new QPushButton(i18n("&Move to Another Folder..."),      folderWidget);
    QPushButton *useFolder  = new QPushButton(i18n("&Use Another Existing Folder..."), folderWidget);
    HelpLabel *helpLabel = new HelpLabel(i18n("Why to do that?"), i18n(
                                             "<p>You can move the folder where %1 store your baskets to:</p><ul>"
                                             "<li>Store your baskets in a visible place in your home folder, like ~/Notes or ~/Baskets, so you can manually backup them when you want.</li>"
                                             "<li>Store your baskets on a server to share them between two computers.<br>"
                                             "In this case, mount the shared-folder to the local file system and ask %1 to use that mount point.<br>"
                                             "Warning: you should not run %1 at the same time on both computers, or you risk to loss data while the two applications are desynced.</li>"
                                             "</ul><p>Please remember that you should not change the content of that folder manually (eg. adding a file in a basket folder will not add that file to the basket).</p>",
                                             KGlobal::mainComponent().aboutData()->programName()),
                                         folderWidget);
    folderLayout->addWidget(moveFolder);
    folderLayout->addWidget(useFolder);
    folderLayout->addWidget(helpLabel);
    folderLayout->addStretch();
    connect(moveFolder, SIGNAL(clicked()), this, SLOT(moveToAnotherFolder()));
    connect(useFolder,  SIGNAL(clicked()), this, SLOT(useAnotherExistingFolder()));

    QGroupBox *backupGroup = new QGroupBox(i18n("Backups"), page);
    QVBoxLayout* backupGroupLayout = new QVBoxLayout;
    backupGroup->setLayout(backupGroupLayout);
    QWidget *backupWidget = new QWidget;
    backupGroupLayout->addWidget(backupWidget);

    QHBoxLayout *backupLayout = new QHBoxLayout(backupWidget);
    backupLayout->setContentsMargins(0, 0, 0, 0);

    QPushButton *backupButton  = new QPushButton(i18n("&Backup..."),           backupWidget);
    QPushButton *restoreButton = new QPushButton(i18n("&Restore a Backup..."), backupWidget);
    m_lastBackup = new QLabel("", backupWidget);
    backupLayout->addWidget(backupButton);
    backupLayout->addWidget(restoreButton);
    backupLayout->addWidget(m_lastBackup);
    backupLayout->addStretch();
    connect(backupButton,  SIGNAL(clicked()), this, SLOT(backup()));
    connect(restoreButton, SIGNAL(clicked()), this, SLOT(restore()));

    populateLastBackup();

    (new QWidget(page))->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

BackupDialog::~BackupDialog()
{
}

void BackupDialog::populateLastBackup()
{
    QString lastBackupText = i18n("Last backup: never");
    if (Settings::lastBackup().isValid())
        lastBackupText = i18n("Last backup: %1", Settings::lastBackup().toString(Qt::LocalDate));

    m_lastBackup->setText(lastBackupText);
}

void BackupDialog::moveToAnotherFolder()
{
    KUrl selectedURL = KDirSelectDialog::selectDirectory(
                           /*startDir=*/Global::savesFolder(), /*localOnly=*/true, /*parent=*/0,
                           /*caption=*/i18n("Choose a Folder Where to Move Baskets"));

    if (!selectedURL.isEmpty()) {
        QString folder = selectedURL.path();
        QDir dir(folder);
        // The folder should not exists, or be empty (because KDirSelectDialog will likely create it anyway):
        if (dir.exists()) {
            // Get the content of the folder:
            QStringList content = dir.entryList();
            if (content.count() > 2) { // "." and ".."
                int result = KMessageBox::questionYesNo(
                                 0,
                                 "<qt>" + i18n("The folder <b>%1</b> is not empty. Do you want to override it?", folder),
                                 i18n("Override Folder?"),
                                 KGuiItem(i18n("&Override"), "document-save")
                             );
                if (result == KMessageBox::No)
                    return;
            }
            Tools::deleteRecursively(folder);
        }
        FormatImporter copier;
        copier.moveFolder(Global::savesFolder(), folder);
        Backup::setFolderAndRestart(folder, i18n("Your baskets have been successfully moved to <b>%1</b>. %2 is going to be restarted to take this change into account."));
    }
}

void BackupDialog::useAnotherExistingFolder()
{
    KUrl selectedURL = KDirSelectDialog::selectDirectory(
                           /*startDir=*/Global::savesFolder(), /*localOnly=*/true, /*parent=*/0,
                           /*caption=*/i18n("Choose an Existing Folder to Store Baskets"));

    if (!selectedURL.isEmpty()) {
        Backup::setFolderAndRestart(selectedURL.path(), i18n("Your basket save folder has been successfuly changed to <b>%1</b>. %2 is going to be restarted to take this change into account."));
    }
}

void BackupDialog::backup()
{
    QDir dir;

    // Compute a default file name & path (eg. "Baskets_2007-01-31.tar.gz"):
    KConfig *config = KGlobal::config().data();
    KConfigGroup configGroup(config, "Backups");
    QString folder = configGroup.readEntry("lastFolder", QDir::homePath()) + "/";
    QString fileName = i18nc("Backup filename (without extension), %1 is the date", "Baskets_%1", QDate::currentDate().toString(Qt::ISODate));
    QString url = folder + fileName;

    // Ask a file name & path to the user:
    QString filter = "*.tar.gz|" + i18n("Tar Archives Compressed by Gzip") + "\n*|" + i18n("All Files");
    QString destination = url;
    for (bool askAgain = true; askAgain;) {
        // Ask:
        destination = KFileDialog::getSaveFileName(destination, filter, 0, i18n("Backup Baskets"));
        // User canceled?
        if (destination.isEmpty())
            return;
        // File already existing? Ask for overriding:
        if (dir.exists(destination)) {
            int result = KMessageBox::questionYesNoCancel(
                             0,
                             "<qt>" + i18n("The file <b>%1</b> already exists. Do you really want to override it?",
                                           KUrl(destination).fileName()),
                             i18n("Override File?"),
                             KGuiItem(i18n("&Override"), "document-save")
                         );
            if (result == KMessageBox::Cancel)
                return;
            else if (result == KMessageBox::Yes)
                askAgain = false;
        } else
            askAgain = false;
    }

    KProgressDialog dialog(0, i18n("Backup Baskets"), i18n("Backing up baskets. Please wait..."));
    dialog.setModal(true);
    dialog.setAllowCancel(false);
    dialog.setAutoClose(true);
    dialog.show();
    QProgressBar *progress = dialog.progressBar();
    progress->setRange(0, 0/*Busy/Undefined*/);
    progress->setValue(0);
    progress->setTextVisible(false);

    BackupThread thread(destination, Global::savesFolder());
    thread.start();
    while (thread.isRunning()) {
        progress->setValue(progress->value() + 1); // Or else, the animation is not played!
        kapp->processEvents();
        usleep(300); // Not too long because if the backup process is finished, we wait for nothing
    }

    Settings::setLastBackup(QDate::currentDate());
    Settings::saveConfig();
    populateLastBackup();
}

void BackupDialog::restore()
{
    // Get last backup folder:
    KConfig *config = KGlobal::config().data();
    KConfigGroup configGroup(config, "Backups");
    QString folder = configGroup.readEntry("lastFolder", QDir::homePath()) + "/";

    // Ask a file name to the user:
    QString filter = "*.tar.gz|" + i18n("Tar Archives Compressed by Gzip") + "\n*|" + i18n("All Files");
    QString path = KFileDialog::getOpenFileName(folder, filter, this, i18n("Open Basket Archive"));
    if (path.isEmpty()) // User has canceled
        return;

    // Before replacing the basket data folder with the backup content, we safely backup the current baskets to the home folder.
    // So if the backup is corrupted or something goes wrong while restoring (power cut...) the user will be able to restore the old working data:
    QString safetyPath = Backup::newSafetyFolder();
    FormatImporter copier;
    copier.moveFolder(Global::savesFolder(), safetyPath);

    // Add the README file for user to cancel a bad restoration:
    QString readmePath = safetyPath + i18n("README.txt");
    QFile file(readmePath);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << i18n("This is a safety copy of your baskets like they were before you started to restore the backup %1.", KUrl(path).fileName()) + "\n\n"
        << i18n("If the restoration was a success and you restored what you wanted to restore, you can remove this folder.") + "\n\n"
        << i18n("If something went wrong during the restoration process, you can re-use this folder to store your baskets and nothing will be lost.") + "\n\n"
        << i18n("Choose \"Basket\" -> \"Backup & Restore...\" -> \"Use Another Existing Folder...\" and select that folder.") + "\n";
        file.close();
    }

    QString message =
        "<p><nobr>" + i18n("Restoring <b>%1</b>. Please wait...", KUrl(path).fileName()) + "</nobr></p><p>" +
        i18n("If something goes wrong during the restoration process, read the file <b>%1</b>.", readmePath);

    KProgressDialog *dialog = new KProgressDialog(0, i18n("Restore Baskets"), message);
    dialog->setModal(/*modal=*/true);
    dialog->setAllowCancel(false);
    dialog->setAutoClose(true);
    dialog->show();
    QProgressBar *progress = dialog->progressBar();
    progress->setRange(0, 0/*Busy/Undefined*/);
    progress->setValue(0);
    progress->setTextVisible(false);

    // Uncompress:
    RestoreThread thread(path, Global::savesFolder());
    thread.start();
    while (thread.isRunning()) {
        progress->setValue(progress->value() + 1); // Or else, the animation is not played!
        kapp->processEvents();
        usleep(300); // Not too long because if the restore process is finished, we wait for nothing
    }

    dialog->hide(); // The restore is finished, do not continue to show it while telling the user the application is going to be restarted
    delete dialog; // If we only hidden it, it reappeared just after having restored a small backup... Very strange.
    dialog = 0;    // This was annoying since it is modal and the "BasKet Note Pads is going to be restarted" message was not reachable.
    //kapp->processEvents();

    // Check for errors:
    if (!thread.success()) {
        // Restore the old baskets:
        QDir dir;
        dir.remove(readmePath);
        copier.moveFolder(safetyPath, Global::savesFolder());
        // Tell the user:
        KMessageBox::error(0, i18n("This archive is either not a backup of baskets or is corrupted. It cannot be imported. Your old baskets have been preserved instead."), i18n("Restore Error"));
        return;
    }

    // Note: The safety backup is not removed now because the code can has been wrong, somehow, or the user perhapse restored an older backup by error...
    //       The restore process will not be called very often (it is possible it will only be called once or twice arround the world during the next years).
    //       So it is rare enough to force the user to remove the safety folder, but keep him in control and let him safely recover from restoration errors.

    Backup::setFolderAndRestart(Global::savesFolder()/*No change*/, i18n("Your backup has been successfuly restored to <b>%1</b>. %2 is going to be restarted to take this change into account."));
}

/** class Backup: */

QString Backup::binaryPath = "";

#include <kiconloader.h>

void Backup::figureOutBinaryPath(const char *argv0, QApplication &app)
{
    /*
       The application can be launched by two ways:
       - Globaly (app.applicationFilePath() is good)
       - In KDevelop or with an absolute path (app.applicationFilePath() is wrong)
       This function is called at the very start of main() so that the current directory has not been changed yet.

       Command line (argv[0])   QDir(argv[0]).canonicalPath()                   app.applicationFilePath()
       ======================   =============================================   =========================
       "basket"                 ""                                              "/opt/kde3/bin/basket"
       "./src/.libs/basket"     "/home/seb/prog/basket/debug/src/.lib/basket"   "/opt/kde3/bin/basket"
    */

    binaryPath = QDir(argv0).canonicalPath();
    if (binaryPath.isEmpty())
        binaryPath = app.applicationFilePath();
}

void Backup::setFolderAndRestart(const QString &folder, const QString &message)
{
    // Set the folder:
    Settings::setDataFolder(folder);
    Settings::saveConfig();

    // Rassure the user that the application main window disapearition is not a crash, but a normal restart.
    // This is important for users to trust the application in such a critical phase and understands what's happening:
    KMessageBox::information(
        0,
        "<qt>" + message.arg(
            (folder.endsWith('/') ? folder.left(folder.length() - 1) : folder),
            KGlobal::mainComponent().aboutData()->programName()),
        i18n("Restart")
    );

    // Restart the application:
    KRun::runCommand(binaryPath, KGlobal::mainComponent().aboutData()->programName(), KGlobal::mainComponent().aboutData()->programName(), 0);
    exit(0);
}

QString Backup::newSafetyFolder()
{
    QDir dir;
    QString fullPath;

    fullPath = QDir::homePath() + "/" + i18nc("Safety folder name before restoring a basket data archive", "Baskets Before Restoration") + "/";
    if (!dir.exists(fullPath))
        return fullPath;

    for (int i = 2; ; ++i) {
        fullPath = QDir::homePath() + "/" + i18nc("Safety folder name before restoring a basket data archive", "Baskets Before Restoration (%1)", i) + "/";
        if (!dir.exists(fullPath))
            return fullPath;
    }

    return "";
}

/** class BackupThread: */

BackupThread::BackupThread(const QString &tarFile, const QString &folderToBackup)
        : m_tarFile(tarFile), m_folderToBackup(folderToBackup)
{
}

void BackupThread::run()
{
    KTar tar(m_tarFile, "application/x-gzip");
    tar.open(QIODevice::WriteOnly);
    tar.addLocalDirectory(m_folderToBackup, backupMagicFolder);
    // KArchive does not add hidden files. Basket description files (".basket") are hidden, we add them manually:
    QDir dir(m_folderToBackup + "baskets/");
    QStringList baskets = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (QStringList::Iterator it = baskets.begin(); it != baskets.end(); ++it) {
        tar.addLocalFile(
            m_folderToBackup + "baskets/" + *it + "/.basket",
            backupMagicFolder + "/baskets/" + *it + "/.basket"
        );
    }
    // We finished:
    tar.close();
}

/** class RestoreThread: */

RestoreThread::RestoreThread(const QString &tarFile, const QString &destFolder)
        : m_tarFile(tarFile), m_destFolder(destFolder)
{
}

void RestoreThread::run()
{
    m_success = false;
    KTar tar(m_tarFile, "application/x-gzip");
    tar.open(QIODevice::ReadOnly);
    if (tar.isOpen()) {
        const KArchiveDirectory *directory = tar.directory();
        if (directory->entries().contains(backupMagicFolder)) {
            const KArchiveEntry *entry = directory->entry(backupMagicFolder);
            if (entry->isDirectory()) {
                ((const KArchiveDirectory*) entry)->copyTo(m_destFolder);
                m_success = true;
            }
        }
        tar.close();
    }
}


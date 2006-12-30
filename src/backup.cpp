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

#include <qhbox.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <klocale.h>
#include <qdir.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <qgroupbox.h>
#include <kdirselectdialog.h>
#include <krun.h>
#include <kconfig.h>
#include <ktar.h>
#include <kfiledialog.h>
#include <kprogress.h>
#include <kmessagebox.h>

/** class BackupDialog: */

BackupDialog::BackupDialog(QWidget *parent, const char *name)
 : KDialogBase(parent, name, /*modal=*/true, i18n("Backup & Restore"),
               KDialogBase::Close, KDialogBase::Close, /*separator=*/false)
{
	QVBox *page  = makeVBoxMainWidget();
//	page->setSpacing(spacingHint());

	QString savesFolder = Global::savesFolder();
	savesFolder = savesFolder.left(savesFolder.length() - 1); // savesFolder ends with "/"

	QGroupBox *folderGroup = new QGroupBox(1, Qt::Horizontal, i18n("Save Folder"), page);
	new QLabel("<qt><nobr>" + i18n("Your baskets are currently stored in that folder:<br><b>%1</b>").arg(savesFolder), folderGroup);
	QWidget *folderWidget = new QWidget(folderGroup);
	QHBoxLayout *folderLayout = new QHBoxLayout(folderWidget, 0, spacingHint());
	QPushButton *moveFolder = new QPushButton(i18n("&Move to Another Folder..."),      folderWidget);
	QPushButton *useFolder  = new QPushButton(i18n("&Use Another Existing Folder..."), folderWidget);
	HelpLabel *helpLabel = new HelpLabel(i18n("Why to do that?"), i18n(
		"<p>You can move the folder where %1 store your baskets to:</p><ul>"
		"<li>Store your baskets in a visible place in your home folder, like ~/Notes or ~/Baskets, so you can manually backup them when you want.</li>"
		"<li>Store your baskets on a server to share them between two computers.<br>"
		"In this case, mount the shared-folder to the local file system and ask %2 to use that mount point.<br>"
		"Warning: you should not run %3 at the same time on both computers, or you risk to loss data while the two applications are desynced.</li>"
		"</ul><p>Please remember that you should not change the content of that folder manually (eg. adding a file in a basket folder will not add that file to the basket).</p>")
			.arg(kapp->aboutData()->programName())
			.arg(kapp->aboutData()->programName())
			.arg(kapp->aboutData()->programName()),
		folderWidget);
	folderLayout->addWidget(moveFolder);
	folderLayout->addWidget(useFolder);
	folderLayout->addWidget(helpLabel);
	folderLayout->addStretch();
	connect( moveFolder, SIGNAL(clicked()), this, SLOT(moveToAnotherFolder())      );
	connect( useFolder,  SIGNAL(clicked()), this, SLOT(useAnotherExistingFolder()) );

	QString lastBackupText = i18n("Last backup: never.");

	QGroupBox *backupGroup = new QGroupBox(1, Qt::Horizontal, i18n("Backups"), page);
	QWidget *backupWidget = new QWidget(backupGroup);
	QHBoxLayout *backupLayout = new QHBoxLayout(backupWidget, 0, spacingHint());
	QPushButton *backupButton  = new QPushButton(i18n("&Backup..."),           backupWidget);
	QPushButton *restoreButton = new QPushButton(i18n("&Restore a Backup..."), backupWidget);
	QLabel *lastBackup = new QLabel(lastBackupText, backupWidget);
	backupLayout->addWidget(backupButton);
	backupLayout->addWidget(restoreButton);
	backupLayout->addWidget(lastBackup);
	backupLayout->addStretch();
	connect( backupButton,  SIGNAL(clicked()), this, SLOT(backup())  );
	connect( restoreButton, SIGNAL(clicked()), this, SLOT(restore()) );

	(new QWidget(page))->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

BackupDialog::~BackupDialog()
{
}

void BackupDialog::moveToAnotherFolder()
{
	KURL selectedURL = KDirSelectDialog::selectDirectory(
		/*startDir=*/Global::savesFolder(), /*localOnly=*/true, /*parent=*/0,
		/*caption=*/i18n("Choose a Folder Where to Move Baskets"));

	if (!selectedURL.isEmpty()) {
		//TODO
		//Backup::setFolderAndRestart(selectedURL.path());
	}
}

void BackupDialog::useAnotherExistingFolder()
{
	KURL selectedURL = KDirSelectDialog::selectDirectory(
		/*startDir=*/Global::savesFolder(), /*localOnly=*/true, /*parent=*/0,
		/*caption=*/i18n("Choose an Existing Folder to Store Baskets"));

	if (!selectedURL.isEmpty()) {
		Backup::setFolderAndRestart(selectedURL.path());
	}
}

void BackupDialog::backup()
{
	QDir dir;

	// Compute a default file name & path (eg. "Baskets_2007-01-31.tar.gz"):
	KConfig *config = KGlobal::config();
	config->setGroup("Backups");
	QString folder = config->readEntry("lastFolder", QDir::homeDirPath()) + "/";
	QString fileName = i18n("Backup filename (without extension), %1 is the date", "Baskets_%1")
		.arg(QDate::currentDate().toString(Qt::ISODate));
	QString url = folder + fileName;

	// Ask a file name & path to the user:
	QString filter = "*.tar.gz|" + i18n("Tar Archives Compressed by Gzip") + "\n*|" + i18n("All Files");
	QString destination = url;
	for (bool askAgain = true; askAgain; ) {
		// Ask:
		destination = KFileDialog::getSaveFileName(destination, filter, 0, i18n("Backup Baskets"));
		// User canceled?
		if (destination.isEmpty())
			return;
		// File already existing? Ask for overriding:
		if (dir.exists(destination)) {
			int result = KMessageBox::questionYesNoCancel(
				0,
				"<qt>" + i18n("The file <b>%1</b> already exists. Do you really want to override it?")
					.arg(KURL(destination).fileName()),
				i18n("Override File?"),
				KGuiItem(i18n("&Override"), "filesave")
			);
			if (result == KMessageBox::Cancel)
				return;
			else if (result == KMessageBox::Yes)
				askAgain = false;
		} else
			askAgain = false;
	}

	KProgressDialog dialog(0, 0, i18n("Backup Baskets"), i18n("Backing up baskets. Please wait..."), /*modal=*/true);
	dialog.setAllowCancel(false);
	dialog.setAutoClose(true);
	dialog.show();
	KProgress *progress = dialog.progressBar();
	progress->setTotalSteps(0/*Busy/Undefined*/);
	progress->setProgress(0);
	progress->setPercentageVisible(false);

	BackupThread thread(destination, Global::savesFolder());
	thread.start();
	while (thread.running()) {
		progress->advance(1); // Or else, the animation is not played!
		kapp->processEvents();
		usleep(2000);
	}
}

void BackupDialog::restore()
{
}

/** class Backup: */

QString Backup::binaryPath = "";

void Backup::setFolderAndRestart(const QString &folder)
{
	// Set the folder:
	Settings::setDataFolder(folder);
	Settings::saveConfig();

	// Restart the application:
	KRun::runCommand(binaryPath, kapp->aboutData()->programName(), kapp->iconName());
	exit(0);
}

#include <iostream>

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

	std::cout << "Binary path is " << binaryPath << std::endl;
}

/** class BackupThread: */

BackupThread::BackupThread(const QString &tarFile, const QString &folderToBackup)
 : m_tarFile(tarFile), m_folderToBackup(folderToBackup)
{
}

void BackupThread::run()
{
	KTar tar(m_tarFile, "application/x-gzip");
	tar.open(IO_WriteOnly);
	tar.addLocalDirectory(m_folderToBackup, "BasKet-Note-Pads_Backup");
	tar.close();
}

#include "backup.moc"

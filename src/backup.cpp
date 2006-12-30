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

/** class BackupDialog: */

BackupDialog::BackupDialog(QWidget *parent, const char *name)
 : KDialogBase(parent, name, /*modal=*/true, i18n("Backup & Restore"),
               KDialogBase::Close, KDialogBase::Close, /*separator=*/false)
{
	QVBox *page  = makeVBoxMainWidget();
//	page->setSpacing(spacingHint());

	QString savesFolder = Global::savesFolder();
	savesFolder = savesFolder.left(savesFolder.length() - 1);

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
}

void BackupDialog::useAnotherExistingFolder()
{
}

void BackupDialog::backup()
{
}

void BackupDialog::restore()
{
}

/** class Backup: */

QString Backup::binaryPath = "";

Backup::Backup()
{
}

Backup::~Backup()
{
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

#include "backup.moc"

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

#ifndef FORMATIMPORTER_H
#define FORMATIMPORTER_H

#include <qobject.h>
//#include <qwidget.h>
#include <qdom.h>

namespace KIO {
	class Job;
}

/**
  * @author Sébastien Laoût
  */
class FormatImporter : QObject
{
  Q_OBJECT
  public:
	static bool shouldImportBaskets();
	static void importBaskets();
	static QDomElement importBasket(const QString &folderName);

	void copyFolder(const QString &folder, const QString &newFolder);
  private slots:
	void slotCopyingDone(KIO::Job*);
  private:
	bool copyFinished;
};

#endif // FORMATIMPORTER_H

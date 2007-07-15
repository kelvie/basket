/***************************************************************************
 *   Copyright (C) 2003 by S�astien Lao�t                                 *
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

#ifndef EXPORTERDIALOG_H
#define EXPORTERDIALOG_H

#include <kdialog.h>

class KUrlRequester;
class QCheckBox;
class QString;

class Basket;

/**
  * @author S�astien Lao�t
  */
class ExporterDialog : public KDialog
{
  Q_OBJECT
  public:
	ExporterDialog(Basket *basket, QWidget *parent = 0, const char *name = 0);
	~ExporterDialog();
	QString filePath();
	bool    embedLinkedFiles();
	bool    embedLinkedFolders();
	bool    erasePreviousFiles();
	bool    formatForImpression();
	void show();
  protected slots:
	void slotOk();
	void load();
	void save();
  private:
	Basket        *m_basket;
	KUrlRequester *m_url;
	QCheckBox     *m_embedLinkedFiles;
	QCheckBox     *m_embedLinkedFolders;
	QCheckBox     *m_erasePreviousFiles;
	QCheckBox     *m_formatForImpression;
};

#endif // EXPORTERDIALOG_H

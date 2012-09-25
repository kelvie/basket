/***************************************************************************
 *   Copyright (C) 2006 by Sébastien Laoût                                 *
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

#include <qstringlist.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <QTextStream>
#include <QTextCodec>
#include <ktempdir.h>

#include "basketthumbcreator.h"

bool BasketThumbCreator::create(const QString &path, int /*width*/, int /*height*/, QImage &image)
{
	// Create the temporar folder:
	KTempDir tempDir;
	tempDir.setAutoRemove(true);
	QString tempFolder = tempDir.name();
	QDir dir;
	dir.mkdir(tempFolder);
	const unsigned long int BUFFER_SIZE = 1024;

	QFile file(path);
	if (file.open(QIODevice::ReadOnly)) {
		QTextStream stream(&file);
		stream.setCodec(QTextCodec::codecForName("UTF-8"));
		QString line = stream.readLine();
		if (line != "BasKetNP:archive" && line != "BasKetNP:template") {
			file.close();
			return false;
		}
		while (!stream.atEnd()) {
			// Get Key/Value Pair From the Line to Read:
			line = stream.readLine();
			int index = line.indexOf(':');
			QString key;
			QString value;
			if (index >= 0) {
				key = line.left(index);
				value = line.right(line.length() - index - 1);
			} else {
				key = line;
				value = "";
			}
			if (key == "preview*") {
				bool ok;
				ulong size = value.toULong(&ok);
				if (!ok) {
					file.close();
					return false;
				}
				// Get the preview file:
				QFile previewFile(tempFolder + "preview.png");
				if (previewFile.open(QIODevice::WriteOnly)) {
					char *buffer = new char[BUFFER_SIZE];
					long int sizeRead;
					while ((sizeRead = file.read(buffer, qMin(BUFFER_SIZE, size))) > 0) {
						previewFile.write(buffer, sizeRead);
						size -= sizeRead;
					}
					previewFile.close();
					delete buffer;
					image = QImage(tempFolder + "preview.png");
					file.close();
					return true;
				}
			} else if (key.endsWith('*')) {
				// We do not know what it is, but we should read the embedded-file in order to discard it:
				bool ok;
				ulong size = value.toULong(&ok);
				if (!ok) {
					file.close();
					return false;
				}
				// Get the archive file:
				char *buffer = new char[BUFFER_SIZE];
				long int sizeRead;
				while ((sizeRead = file.read(buffer, qMin(BUFFER_SIZE, size))) > 0) {
					size -= sizeRead;
				}
				delete buffer;
			}
		}
		file.close();
	}
	return false;
}

ThumbCreator::Flags BasketThumbCreator::flags() const
{
	return (Flags) (DrawFrame | BlendIcon);
}

extern "C"
{
	KDE_EXPORT ThumbCreator *new_creator()
	{
		return new BasketThumbCreator();
	}
};

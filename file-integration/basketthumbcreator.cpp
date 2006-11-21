#include <qstringlist.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <ktempdir.h>

#include "basketthumbcreator.h"

bool BasketThumbCreator::create(const QString &path, int /*width*/, int /*height*/, QImage &image)
{
	// Create the temporar folder:
	KTempDir tempDir;
	tempDir.setAutoDelete(true);
	QString tempFolder = tempDir.name();//"/tmp/kde-seb/temp-archive/";
	QDir dir;
	dir.mkdir(tempFolder);
	const Q_ULONG BUFFER_SIZE = 1024;

	QFile file(path);
	if (file.open(IO_ReadOnly)) {
		QTextStream stream(&file);
		stream.setEncoding(QTextStream::Latin1);
		QString line = stream.readLine();
		if (line != "BasKetNP:archive" && line != "BasKetNP:template") {
			file.close();
//			deleteRecursively(tempFolder);
			return false;
		}
		while (!stream.atEnd()) {
			// Get Key/Value Pair From the Line to Read:
			line = stream.readLine();
			int index = line.find(':');
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
//					deleteRecursively(tempFolder);
					return false;
				}
				// Get the preview file:
				QFile previewFile(tempFolder + "preview.png");
				if (previewFile.open(IO_WriteOnly)) {
					char *buffer = new char[BUFFER_SIZE];
					Q_LONG sizeRead;
					while ((sizeRead = file.readBlock(buffer, QMIN(BUFFER_SIZE, size))) > 0) {
						previewFile.writeBlock(buffer, sizeRead);
						size -= sizeRead;
					}
					previewFile.close();
					delete buffer;
					image = QImage(tempFolder + "preview.png");
					file.close();
//					deleteRecursively(tempFolder);
					return true;
				}
			} else if (key.endsWith("*")) {
				// We do not know what it is, but we should read the embedded-file in order to discard it:
				bool ok;
				ulong size = value.toULong(&ok);
				if (!ok) {
					file.close();
//					deleteRecursively(tempFolder);
					return false;
				}
				// Get the archive file:
				char *buffer = new char[BUFFER_SIZE];
				Q_LONG sizeRead;
				while ((sizeRead = file.readBlock(buffer, QMIN(BUFFER_SIZE, size))) > 0) {
					size -= sizeRead;
				}
				delete buffer;
			} else {
				// We do not know what it is, and we do not care.
			}
			// Analyse the Value, if Understood:
		}
		file.close();
	}
//	deleteRecursively(tempFolder);
	return false;
}

ThumbCreator::Flags BasketThumbCreator::flags() const
{
	return (Flags) (DrawFrame | BlendIcon);
}

// Copied from src/Tools::deleteRecursively()
void BasketThumbCreator::deleteRecursively(const QString &/*folderOrFile*/)
{
// No need anymore, since we're using KTempDir
/*	if (folderOrFile.isEmpty())
		return;

	QFileInfo fileInfo(folderOrFile);
	if (fileInfo.isDir()) {
		// Delete the child files:
		QDir dir(folderOrFile, QString::null, QDir::Name | QDir::IgnoreCase, QDir::All | QDir::Hidden);
		QStringList list = dir.entryList();
		for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
			if ( *it != "." && *it != ".." )
				deleteRecursively(folderOrFile + "/" + *it);
		// And then delete the folder:
		dir.rmdir(folderOrFile);
	} else
		// Delete the file:
		QFile::remove(folderOrFile);
*/
}

extern "C"
{
	ThumbCreator *new_creator()
	{
		return new BasketThumbCreator();
	}
};

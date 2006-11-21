#include <qstring.h>
#include <qimage.h>
#include <kio/thumbcreator.h>

class BasketThumbCreator : public ThumbCreator
{
	bool create(const QString &path, int width, int height, QImage &image);
	Flags flags() const;

	void deleteRecursively(const QString &folderOrFile);
};

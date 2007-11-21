#include <KUniqueApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KDebug>

#include "basketapplication.h"

int main(int argc, char *argv[]) {
	KAboutData aboutData( "basket", 0,
			ki18n("BasKet Note Pads"),
			"1.90",
			ki18n(""),
			KAboutData::License_GPL,
			ki18n("(c) 2003-2007 Sébastien Laoût") );

	aboutData.addAuthor( ki18n("Sébastien Laoût"), ki18n("Author, Maintainer"), "slaout@linux62.org" );
	aboutData.setHomepage( "http://basket.kde.org" );

	KCmdLineArgs::init( argc, argv, &aboutData );

	KUniqueApplication::addCmdLineOptions();

	if (!KUniqueApplication::start()) {
		fprintf( stderr, "Basket is already running!\n" );
		return 0;
	}

	BasketApplication basket;
	return basket.exec();
}


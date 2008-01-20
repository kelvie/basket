#include <QWidget>

#include <KLocale>
#include <KDebug>

#include "basketsettingsdialog.h"

BasketSettingsDialog::BasketSettingsDialog( QWidget* parent ) : KPageDialog( parent ) {
	const QSize minSize = minimumSize();
	setMinimumSize( QSize( 512, minSize.height() ) );

	setFaceType( List );
	setCaption( i18n( "Basket Preferences" ) );
	setButtons( Ok | Apply | Cancel | Default );
	setDefaultButton( Ok );

	KPageWidgetItem *general = addPage( new QWidget( this ), i18n( "General" ) );
	general->setIcon( KIcon( "system-run" ) );

	KPageWidgetItem *appearance = addPage( new QWidget( this ), i18n( "Appearance" ) );
	appearance->setIcon( KIcon( "plasma" ) );
}

BasketSettingsDialog::~BasketSettingsDialog() {
}


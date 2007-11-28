#include <QString>

#include <KLocalizedString>
#include <KLineEdit>
#include <KVBox>

#include "newbasketdialog.h"

NewBasketDialog::NewBasketDialog() : KDialog(), m_isOk( false )  {
	this->setCaption( i18n( "New Basket" ) );
	this->setModal( true );

	this->enableButtonOk( false );

	m_name = new KLineEdit( this );

	connect( this, SIGNAL( okClicked() ), this, SLOT( setOk() ) );
	connect( m_name, SIGNAL( textChanged( const QString& ) ), this, SLOT( validate( const QString& ) ) );
}

NewBasketDialog::~NewBasketDialog() {
}

void NewBasketDialog::setOk() {
	m_isOk = true;
}

void NewBasketDialog::validate( const QString& newName ) {
	enableButtonOk( !newName.isEmpty() );
}

QString NewBasketDialog::name() {
	return m_name->text();
}


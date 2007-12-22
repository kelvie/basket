#include "note.h"

#include <QList>

#include <KDebug>

Note::Note( QPointF pos, const QString& text) : mPos( pos ), mText( text ) {
}

Note::Note( qreal x, qreal y, const QString& text ) : mPos(x, y), mText( text ) {
}

Note::~Note() {
}

bool Note::parseString( const QString& data ) {
	int i = data.indexOf( ';' );
	if ( i < 0 ) return false;
	int j = data.indexOf( ';', i + 1 );
	if ( j < 0 ) return false;
	qreal mX = data.left( i ).toDouble();
	qreal mY = data.mid( i + 1, j - i - 1 ).toDouble();
	mPos = QPointF( mX, mY );
	mText = data.right( data.length() - j - 1 );
	kDebug() << mText << endl;
	return true;
}

QString Note::toString() {
	kDebug() << QString::number( mPos.x() ) + ";" + QString::number( mPos.y() ) + ";" + mText << endl;
	return QString::number( mPos.x() ) + ";" + QString::number( mPos.y() ) + ";" + mText;
}

void Note::setPos( QPointF pos ) {
	mPos = pos;
}

void Note::setPos( qreal x, qreal y ) {
	mPos = QPointF( x, y );
}

void Note::setText( const QString& text ) {
	mText = text;
}

QPointF Note::pos() const {
	return mPos;
}

qreal Note::x() const {
	return mPos.x();
}

qreal Note::y() const {
	return mPos.y();
}

const QString& Note::text() const {
	return mText;
}



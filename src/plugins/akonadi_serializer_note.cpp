#include "akonadi_serializer_note.h"
#include "../note.h"

#include <QMimeData>

#include <KBookmark>
#include <KDebug>

#include <boost/shared_ptr.hpp>

#include <libakonadi/item.h>

using namespace Akonadi;

typedef boost::shared_ptr<Note> NotePtr;

bool SerializerPluginNote::deserialize( Item& item, const QString& label, QIODevice& data ) {
	if ( label != Item::PartBody )
		return false;

	NotePtr note;
	if ( !item.hasPayload() ) {
		Note* n = new Note();
		note = NotePtr( n );
		item.setPayload( note );
	} else {
		note = item.payload<NotePtr>();
	}

	QByteArray buffer = data.readAll();
	if ( !note->parseString( QString::fromUtf8( buffer ) ) ) {
		kDebug() << "can't parse data" << endl;
		kDebug() << buffer << endl;
		return false;
	}

	return true;
}

void SerializerPluginNote::serialize( const Item& item, const QString& label, QIODevice& data ) {
	if ( label != Item::PartBody )
		return;
	
	if ( item.mimeType() != QString::fromLatin1( "basket/note" ) )
		return;

	NotePtr note = item.payload<NotePtr>();
	data.write( note->toString().toUtf8() );
}

extern "C"
KDE_EXPORT Akonadi::ItemSerializerPlugin *
akonadi_serializer_note_create_item_serializer_plugin() {
	return new SerializerPluginNote();
}


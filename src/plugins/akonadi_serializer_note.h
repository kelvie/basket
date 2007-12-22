#ifndef AKONADI_SERIALIZER_NOTE_H
#define AKONADI_SERIALIZER_NOTE_H

#include <libakonadi/itemserializerplugin.h>

class QIODevice;
class QString;

class Akonadi::Item;

using namespace Akonadi;

/*
 *	Plugin for Akonadi to store Note items
 */
class SerializerPluginNote : public ItemSerializerPlugin {
public:
	bool deserialize( Item& item, const QString& label, QIODevice& data );
	void serialize( const Item& item, const QString& label, QIODevice& data );
};

#endif

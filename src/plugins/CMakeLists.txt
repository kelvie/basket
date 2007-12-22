
#include_directories( ${Boost_INCLUDE_DIR} )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

# TODO remove ugly hack to compile plugin
set( akonadi_serializer_note_PART_SRCS akonadi_serializer_note.cpp ../note.cpp )
kde4_add_plugin( akonadi_serializer_note ${akonadi_serializer_note_PART_SRCS} )
target_link_libraries( akonadi_serializer_note ${KDE4_KDECORE_LIBS} akonadi )
install( TARGETS akonadi_serializer_note DESTINATION ${PLUGIN_INSTALL_DIR} )


install( FILES akonadi_serializer_note.desktop DESTINATION ${DATA_INSTALL_DIR}/akonadi/plugins/serializer )


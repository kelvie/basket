project(content-containment)

include( KDE4Defaults )
include( MacroLibrary )

add_definitions( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS} )

include_directories( ../../..
                     ../plasma
                     ../..
                     ..
					 ${KDE4_INCLUDE_DIR}/basket
					 ${CMAKE_SOURCE_DIR}
					 ${CMAKE_BINARY_DIR}
					 ${KDE4_INCLUDES}
)
                     
set( content_SRCS BasketContent.cpp )

find_package( Plasma REQUIRED )

kde4_add_plugin( basket_containment_content ${content_SRCS} )
target_link_libraries( basket_containment_content 
	basketplasma 
	${KDE4_KIO_LIBS}
	${KDE4_KUTILS_LIBS}
	${KDE4_KDEUI_LIBS}
	${KDE4_KNEWSTUFF_LIBS}
	${KDE4_CORE_LIBS}
)

install( TARGETS basket_containment_content DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES basket-containment-content.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


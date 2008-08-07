
set(basketthumbcreator_PART_SRCS basketthumbcreator.cpp)

kde4_add_plugin(basketthumbcreator ${basketthumbcreator_PART_SRCS})

target_link_libraries(basketthumbcreator ${KDE4_KIO_LIBS})

install(TARGETS basketthumbcreator DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES  basketthumbcreator.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES  basket.magic DESTINATION ${CONFIG_INSTALL_DIR}/magic)
install(FILES  x-basket-archive.desktop x-basket-template.desktop DESTINATION ${MIME_INSTALL_DIR}/application)

kde4_install_icons(${ICON_INSTALL_DIR})

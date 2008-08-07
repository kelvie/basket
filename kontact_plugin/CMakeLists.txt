set(kontact_basket_PART_SRCS basket_plugin.cpp uniqueapphandler.cpp)

message(STATUS "DCOP has been removed in KDE 4; port code to D-Bus. kde4_add_dcop_skels macro does not exist anymore. We keep it just to remember to port to D-Bus.")
#kde4_add_dcop_stubs(kontact_basket_PART_SRCS
 basketdcopiface.h)

kde4_add_plugin(kontact_basket WITH_PREFIX ${kontact_basket_PART_SRCS})

target_link_libraries(kontact_basket ${KDE4_KDEUI_LIBS} kpinterfaces )

install(TARGETS kontact_basket DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES  kontact_basketplugin.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)
install(FILES  basket.desktop basket_v4.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)


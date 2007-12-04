# 
kde4_add_plugin(bnp_persistence bnp_persistence_interface.cpp)

#
target_link_libraries(bnp_persistence ${KDE4_KDECORE_LIBS})

#
install (TARGETS  bnp_persistence DESTINATION ${PLUGIN_INSTALL_DIR})

#
install (FILES  bnp_persistence.desktop DESTINATION ${SERVICES_INSTALL_DIR})

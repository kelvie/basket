if(QT_QTSQL_FOUND)
  message("-- Qt4 SQL module found........... YES")
else(SQLITE_FOUND)
  message("-- Qt4 SQL module found........... NO")
  message("")
  message("BasketNotePads needs the SQL module of Qt4")
  message("You need to install a package containing the module with support for sqlite3.")
  message("")
endif(QT_QTSQL_FOUND)
kde4_add_plugin(bnp_persistence bnp_persistence_interface.cpp)

#
target_link_libraries(bnp_persistence ${QT_QTSQL_LIBRARY} ${KDE4_KDECORE_LIBS} )

#
install (TARGETS  bnp_persistence DESTINATION ${PLUGIN_INSTALL_DIR})

#
install (FILES  bnp_persistence.desktop DESTINATION ${SERVICES_INSTALL_DIR})

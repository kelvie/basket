
include_directories(${CMAKE_SOURCE_DIR}/src ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############

set(kontact_basket_PART_SRCS basket_plugin.cpp uniqueapphandler.cpp)

message(STATUS "DCOP has been removed in KDE 4; port code to D-Bus. kde4_add_dcop_skels macro does not exist anymore. We keep it just to remember to port to D-Bus.")
#kde4_add_dcop_stubs(kontact_basket_PART_SRCS
 basketdcopiface.h)

kde4_add_plugin(kontact_basket WITH_PREFIX ${kontact_basket_PART_SRCS})

target_link_libraries(kontact_basket ${KDE4_KDECORE_LIBS} kpinterfaces DCOP qt-mt kdecore kdeui)

install(TARGETS kontact_basket DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES  kontact_basketplugin.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)
install(FILES  basket.desktop basket_v4.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/src -I$(top_builddir)/src -I$(top_srcdir) $(all_includes)
#
#kde_module_LTLIBRARIES = libkontact_basket.la
#libkontact_basket_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkontact_basket_la_LIBADD = -lkpinterfaces -lDCOP -lqt-mt -lkdecore -lkdeui
#
#noinst_HEADERS = basket_plugin.h uniqueapphandler.h
#libkontact_basket_la_SOURCES = basket_plugin.cpp uniqueapphandler.cpp basketdcopiface.stub
#
#METASOURCES = AUTO
#
#basketdcopiface_DIR = $(top_srcdir)/src
#
#servicedir = $(kde_servicesdir)/kontact
#service_DATA = basket.desktop basket_v4.desktop
#
#basketsetdlgdir = $(kde_datadir)/kontact/ksettingsdialog
#basketsetdlg_DATA = kontact_basketplugin.setdlg

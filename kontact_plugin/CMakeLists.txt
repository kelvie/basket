INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(kontact_basket_PART_SRCS
   basket_plugin.cpp
   uniqueapphandler.cpp
   )

#KDE4_AUTOMOC(${kontact_basket_PART_SRCS})

SET( kontact_basket_DCOP_STUB_SRCS
   basketdcopiface.h
   )

KDE4_ADD_DCOP_STUBS(kontact_basket_PART_SRCS ${kontact_basket_DCOP_STUB_SRCS})

KDE4_ADD_PLUGIN(kontact_basket WITH_PREFIX ${kontact_basket_PART_SRCS})

TARGET_LINK_LIBRARIES(kontact_basket  ${QT_AND_KDECORE_LIBS} kpinterfaces DCOP qt-mt kdecore kdeui )

INSTALL(TARGETS kontact_basket DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

INSTALL(FILES /share/apps/kontact/ksettingsdialog kontact_basketplugin.setdlg )
INSTALL(FILES /share/services/kontact basket.desktop basket_v4.desktop )



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

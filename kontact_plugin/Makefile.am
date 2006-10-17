INCLUDES = -I$(top_srcdir)/src -I$(top_builddir)/src -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libkontact_basket.la
libkontact_basket_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_basket_la_LIBADD = -lkpinterfaces -lDCOP -lqt-mt -lkdecore -lkdeui

noinst_HEADERS = basket_plugin.h uniqueapphandler.h
libkontact_basket_la_SOURCES = basket_plugin.cpp uniqueapphandler.cpp basketdcopiface.stub

METASOURCES = AUTO

basketdcopiface_DIR = $(top_srcdir)/src

servicedir = $(kde_servicesdir)/kontact
service_DATA = basket.desktop

basketsetdlgdir = $(kde_datadir)/kontact/ksettingsdialog
basketsetdlg_DATA = kontact_basketplugin.setdlg

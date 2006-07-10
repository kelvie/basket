INCLUDES = -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libkontact_basket.la
libkontact_basket_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_basket_la_LIBADD = -lkpinterfaces

libkontact_basket_la_SOURCES = basket_plugin.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kontact
service_DATA = basket.desktop

basketsetdlgdir = $(kde_datadir)/kontact/ksettingsdialog
basketsetdlg_DATA = kontact_basketplugin.setdlg

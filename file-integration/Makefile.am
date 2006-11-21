# MIME Types Information:
applicationmimedir = $(kde_mimedir)/application
applicationmime_DATA = x-basket-archive.desktop x-basket-template.desktop

# Magic MIME Types Discovery:
magicdir = $(kde_confdir)/magic
magic_DATA = basket.magic

# Thumbnail Creator:
INCLUDES = $(all_includes)
kde_module_LTLIBRARIES = basketthumbcreator.la
basketthumbcreator_la_SOURCES = basketthumbcreator.cpp
basketthumbcreator_la_LIBADD = $(LIB_KIO)
basketthumbcreator_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kde_services_DATA = basketthumbcreator.desktop

# Icons:
KDE_ICON = AUTO
#kde_icon_KDEICON = \
#	cr128-mimetype-baskets.png cr16-mimetype-baskets.png cr22-mimetype-baskets.png \
#	cr32-mimetype-baskets.png  cr48-mimetype-baskets.png cr64-mimetype-baskets.png \
#	cr128-mimetype-baskett.png cr16-mimetype-baskett.png cr22-mimetype-baskett.png \
#	cr32-mimetype-baskett.png  cr48-mimetype-baskett.png cr64-mimetype-baskett.png

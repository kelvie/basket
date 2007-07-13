INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(basketthumbcreator_PART_SRCS
   basketthumbcreator.cpp
   )

#KDE4_AUTOMOC(${basketthumbcreator_PART_SRCS})

KDE4_ADD_PLUGIN(basketthumbcreator ${basketthumbcreator_PART_SRCS})

TARGET_LINK_LIBRARIES(basketthumbcreator  ${QT_AND_KDECORE_LIBS} kio )

INSTALL(TARGETS basketthumbcreator DESTINATION ${LIB_INSTALL_DIR}/kde3)


########### install files ###############

INSTALL(FILES basketthumbcreator.desktop DESTINATION ${SERVICES_INSTALL_DIR})
INSTALL(FILES basket.magic DESTINATION /share/config/magic)
INSTALL(FILES x-basket-archive.desktop x-basket-template.desktop DESTINATION /share/mimelink/application)

KDE4_INSTALL_ICONS( hicolor )



#original Makefile.am contents follow:

## MIME Types Information:
#applicationmimedir = $(kde_mimedir)/application
#applicationmime_DATA = x-basket-archive.desktop x-basket-template.desktop
#
## Magic MIME Types Discovery:
#magicdir = $(kde_confdir)/magic
#magic_DATA = basket.magic
#
## Thumbnail Creator:
#INCLUDES = $(all_includes)
#kde_module_LTLIBRARIES = basketthumbcreator.la
#basketthumbcreator_la_SOURCES = basketthumbcreator.cpp
#basketthumbcreator_la_LIBADD = $(LIB_KIO)
#basketthumbcreator_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kde_services_DATA = basketthumbcreator.desktop
#
## Icons:
#KDE_ICON = AUTO
##kde_icon_KDEICON = \
##	cr128-mimetype-baskets.png cr16-mimetype-baskets.png cr22-mimetype-baskets.png \
##	cr32-mimetype-baskets.png  cr48-mimetype-baskets.png cr64-mimetype-baskets.png \
##	cr128-mimetype-baskett.png cr16-mimetype-baskett.png cr22-mimetype-baskett.png \
##	cr32-mimetype-baskett.png  cr48-mimetype-baskett.png cr64-mimetype-baskett.png

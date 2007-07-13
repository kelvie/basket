########### next target ###############

SET(basketcommon_LIB_SRCS
   main.cpp
   archive.cpp
   bnpview.cpp
   settings.cpp
   basket.cpp
   basketproperties.cpp
   linklabel.cpp
   variouswidgets.cpp
   xmlwork.cpp
   keyboard.cpp
   global.cpp
   exporterdialog.cpp
   htmlexporter.cpp
   notefactory.cpp
   softwareimporters.cpp
   focusedwidgets.cpp
   popupmenu.cpp
   basketfactory.cpp
   tag.cpp
   qeffects.cpp
   kcolorcombo2.cpp
   newbasketdialog.cpp
   tagsedit.cpp
   formatimporter.cpp
   note.cpp
   notecontent.cpp
   notedrag.cpp
   noteedit.cpp
   filter.cpp
   tools.cpp
   backgroundmanager.cpp
   regiongrabber.cpp
   kicondialog.cpp
   kiconcanvas.cpp
   kgpgme.cpp
   likeback.cpp
   crashhandler.cpp
   password.cpp
   colorpicker.cpp
   basketlistview.cpp
   debugwindow.cpp
   systemtray.cpp
   aboutdata.cpp
   basketstatusbar.cpp
   clickablelabel.cpp
   backup.cpp
   )

#kde4_automoc(${basketcommon_LIB_SRCS})

SET(basketcommon_UI
   kicondialogui.ui
   passwordlayout.ui
   )

KDE4_ADD_UI_FILES(basketcommon_LIB_SRCS ${basketcommon_UI})

SET( basketcommon_DCOP_SKEL_SRCS
   basketdcopiface.h
   )

#KDE4_ADD_DCOP_SKELS(basketcommon_LIB_SRCS ${basketcommon_DCOP_SKEL_SRCS})

KDE4_ADD_LIBRARY(basketcommon SHARED ${basketcommon_LIB_SRCS})

TARGET_LINK_LIBRARIES(basketcommon  ${QT_AND_KDECORE_LIBS} ${KDE4_KIO_LIBS} kdeui kparts DCOP kdefx  )

SET_TARGET_PROPERTIES(basketcommon PROPERTIES VERSION 4.2.0 SOVERSION 4 )
INSTALL(TARGETS basketcommon DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

SET(basket_SRCS
   main.cpp
   mainwindow.cpp
   application.cpp
   )

#KDE4_AUTOMOC(${basket_SRCS})

KDE4_ADD_EXECUTABLE(basket ${basket_SRCS})

TARGET_LINK_LIBRARIES(basket  ${QT_AND_KDECORE_LIBS} kutils basketcommon )

INSTALL(TARGETS basket DESTINATION ${BIN_INSTALL_DIR})


########### next target ###############

SET(kcm_basket_PART_SRCS
   kcm_basket.cpp
   )

#KDE4_AUTOMOC(${kcm_basket_PART_SRCS})

KDE4_ADD_PLUGIN(kcm_basket ${kcm_basket_PART_SRCS})

TARGET_LINK_LIBRARIES(kcm_basket  ${QT_AND_KDECORE_LIBS} basketcommon kdeui )

INSTALL(TARGETS kcm_basket DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

SET(basketpart_PART_SRCS
   basket_part.cpp
   )

#KDE_AUTOMOC(${basketpart_PART_SRCS})

KDE4_ADD_PLUGIN(basketpart WITH_PREFIX ${basketpart_PART_SRCS})

TARGET_LINK_LIBRARIES(basketpart  ${QT_AND_KDECORE_LIBS} basketcommon kparts kdeui DCOP )

INSTALL(TARGETS basketpart DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

INSTALL(FILES  basket_part.rc DESTINATION ${DATA_INSTALL_DIR}/basket)
INSTALL(FILES  basket_part.desktop DESTINATION ${SERVICES_INSTALL_DIR})
INSTALL(FILES  basket.desktop DESTINATION /share/applnk/Utilities)
INSTALL(FILES  basket_config_general.desktop  	basket_config_baskets.desktop  	basket_config_new_notes.desktop  	basket_config_notes_appearance.desktop  	basket_config_apps.desktop  	basket_config_features.desktop  	basket_config_notes.desktop DESTINATION ${SERVICES_INSTALL_DIR})
INSTALL(FILES basketui.rc DESTINATION ${DATA_INSTALL_DIR}/basket)

KDE4_INSTALL_ICONS( hicolor )



#original Makefile.am contents follow:

## set the include path for X, qt and KDE
#INCLUDES = $(all_includes)
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/basket.pot
#
#KDE_ICON = AUTO
#
## these are the headers for your project
#
#noinst_HEADERS = basket.h basketproperties.h mainwindow.h basket_part.h \
#							debugwindow.h 	linklabel.h variouswidgets.h xmlwork.h keyboard.h global.h settings.h \
#							focusedwidgets.h popupmenu.h basketfactory.h exporterdialog.h tag.h kcolorcombo2.h \
#							newbasketdialog.h tagsedit.h note.h notecontent.h 	notedrag.h noteedit.h notefactory.h \
#							filter.h tools.h backgroundmanager.h regiongrabber.h softwareimporters.h \
#							kicondialog.h kiconcanvas.h kgpgme.h crashhandler.h password.h bnpview.h \
#							systemtray.h clickablelabel.h colorpicker.h basketlistview.h qeffects.h likeback.h \
#							formatimporter.h aboutdata.h basketstatusbar.h basketdcopiface.h basket_options.h \
#					likeback_private.h application.h archive.h htmlexporter.h backup.h
#
##########################################################################
## COMMON FILES SECTION
##########################################################################
#
#lib_LTLIBRARIES = libbasketcommon.la
#
#AM_CPPFLAGS = $(GPGME_CFLAGS)
#
#libbasketcommon_la_LDFLAGS = -avoid-version -no-undefined $(all_libraries)
#
#libbasketcommon_la_LIBADD = $(LIB_ARTSKDE) -lkio $(LIB_KDEUI) $(LIB_KDECORE) \
#	$(LIB_QT) $(LIBSOCKET) $(GPGME_LIBS) $(LIB_KPARTS) -lDCOP -lkdefx -lkdeui
#
#libbasketcommon_la_SOURCES = \
#	archive.cpp bnpview.cpp settings.cpp basket.cpp basketproperties.cpp \
#	linklabel.cpp variouswidgets.cpp xmlwork.cpp keyboard.cpp \
#	global.cpp exporterdialog.cpp htmlexporter.cpp notefactory.cpp softwareimporters.cpp \
#	focusedwidgets.cpp popupmenu.cpp basketfactory.cpp \
#	tag.cpp qeffects.cpp kcolorcombo2.cpp newbasketdialog.cpp tagsedit.cpp \
#	formatimporter.cpp note.cpp notecontent.cpp notedrag.cpp noteedit.cpp \
#	filter.cpp tools.cpp backgroundmanager.cpp regiongrabber.cpp \
#	kicondialogui.ui kicondialog.cpp kiconcanvas.cpp kgpgme.cpp likeback.cpp \
#	crashhandler.cpp passwordlayout.ui password.cpp colorpicker.cpp \
#	basketlistview.cpp debugwindow.cpp systemtray.cpp aboutdata.cpp \
#	basketstatusbar.cpp clickablelabel.cpp basketdcopiface.skel backup.cpp
#
##########################################################################
## APPLICATION SECTION
##########################################################################
## this is the program that gets installed.  it's name is used for all
## of the other Makefile.am variables
#bin_PROGRAMS = basket
#
## the application source, library search path, and link libraries
#basket_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_KUTILS)
#basket_LDADD = libbasketcommon.la
#
## this is where the desktop file will go
#shelldesktopdir = $(kde_appsdir)/Utilities
#shelldesktop_DATA = basket.desktop
#
## this is where the shell's XML-GUI resource file goes
#rcdir = $(kde_datadir)/basket
#rc_DATA = basketui.rc
#
#basket_SOURCES = main.cpp mainwindow.cpp application.cpp
#
#kde_icon_KDEICON = cr128-app-basket.png cr16-app-basket.png cr22-app-basket.png \
#	cr32-app-basket.png cr48-app-basket.png cr64-app-basket.png hi16-app-basket_old.png \
#	hi32-app-basket_old.png crsc-app-basket.svg \
#	cr16-action-likeback_like.png cr16-action-likeback_dislike.png \
#	cr16-action-likeback_bug.png cr16-action-likeback_feature.png
#
###################
## KPART SECTION
###################
#
#kde_module_LTLIBRARIES = kcm_basket.la libbasketpart.la
#
#libbasketpart_la_SOURCES = basket_part.cpp
#libbasketpart_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module -avoid-version -no-undefined
#libbasketpart_la_LIBADD  = libbasketcommon.la -lkparts -lkdeui $(LIB_KDECORE) $(LIB_QT) -lDCOP
#
#kcm_basket_la_SOURCES = kcm_basket.cpp
#kcm_basket_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kcm_basket_la_LIBADD = libbasketcommon.la $(LIB_KDECORE) $(LIB_QT) -lkdeui
#
## this is where the desktop file will go
#partdesktopdir   = $(kde_servicesdir)
#partdesktop_DATA = basket_part.desktop
#
## this is where the part's XML-GUI resource file goes
#partdir   = $(kde_datadir)/basket
#part_DATA = basket_part.rc
#
#kde_services_DATA = \
#	basket_config_general.desktop \
#	basket_config_baskets.desktop \
#	basket_config_new_notes.desktop \
#	basket_config_notes_appearance.desktop \
#	basket_config_apps.desktop \
#	basket_config_features.desktop \
#	basket_config_notes.desktop

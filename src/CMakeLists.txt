
include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${QIMAGEBLITZ_INCLUDES} ${GPGME_INCLUDES})


########### next target ###############

set(basketcommon_LIB_SRCS
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
    basketfactory.cpp
    tag.cpp
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
    backup.cpp)

kde4_add_ui_files(basketcommon_LIB_SRCS passwordlayout.ui)

QT4_ADD_DBUS_ADAPTOR(basketcommon_LIB_SRCS org.basket.BNPView.xml bnpview.h BNPView)

kde4_add_library(basketcommon SHARED ${basketcommon_LIB_SRCS})

target_link_libraries(basketcommon
  ${KDE4_KDE3SUPPORT_LIBS} # Remove me
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KIO_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KPARTS_LIBS}
  ${QT_PHONON_LIBRARY}
  ${QIMAGEBLITZ_LIBRARIES}
  ${GPGME_VANILLA_LIBRARIES})

set_target_properties(basketcommon PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS basketcommon DESTINATION ${LIB_INSTALL_DIR})

########### next target ###############

set(basket_SRCS main.cpp mainwindow.cpp application.cpp)

kde4_add_executable(basket ${basket_SRCS})
target_link_libraries(basket
  ${KDE4_KDE3SUPPORT_LIBS} # Remove me as well
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KUTILS_LIBS}
  ${QT_PHONON_LIBRARY}
  basketcommon)

install(TARGETS basket DESTINATION ${BIN_INSTALL_DIR})

if(BUILD_KPARTS)
########### next target ###############

set(kcm_basket_PART_SRCS kcm_basket.cpp)

kde4_add_plugin(kcm_basket ${kcm_basket_PART_SRCS})

target_link_libraries(kcm_basket ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} basketcommon)

install(TARGETS kcm_basket DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(basketpart_PART_SRCS basket_part.cpp)

kde4_add_plugin(basketpart WITH_PREFIX ${basketpart_PART_SRCS})

target_link_libraries(basketpart ${KDE4_KDECORE_LIBS} basketcommon kparts kdeui DCOP)

install(TARGETS basketpart DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES  basket_part.rc DESTINATION ${DATA_INSTALL_DIR}/basket)
install(FILES  basket_part.desktop DESTINATION ${SERVICES_INSTALL_DIR})

endif(BUILD_KPARTS)

install(FILES  basket.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES  basket_config_general.desktop  	basket_config_baskets.desktop  	basket_config_new_notes.desktop  	basket_config_notes_appearance.desktop  	basket_config_apps.desktop  	basket_config_features.desktop  	basket_config_notes.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES  basketui.rc DESTINATION ${DATA_INSTALL_DIR}/basket)

kde4_install_icons(${ICON_INSTALL_DIR})



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

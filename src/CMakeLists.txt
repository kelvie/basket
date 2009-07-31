include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${QIMAGEBLITZ_INCLUDES} ${GPGME_INCLUDES})
add_subdirectory(tests)

########### next target ###############

set(basketcommon_LIB_SRCS
    aboutdata.cpp
    archive.cpp
    backgroundmanager.cpp
    backup.cpp
    basket.cpp
    basketfactory.cpp
    basketlistview.cpp
    basketproperties.cpp
    basketstatusbar.cpp
    bnpview.cpp
    colorpicker.cpp
    crashhandler.cpp
    debugwindow.cpp
    exporterdialog.cpp
    filter.cpp
    focusedwidgets.cpp
    formatimporter.cpp
    global.cpp
    htmlexporter.cpp
    kcolorcombo2.cpp
    keyboard.cpp
    kgpgme.cpp
    likeback.cpp
    linklabel.cpp
    newbasketdialog.cpp
    note.cpp
    notecontent.cpp
    notedrag.cpp
    noteedit.cpp
    notefactory.cpp
    noteselection.cpp
    password.cpp
    regiongrabber.cpp
    settings.cpp
    softwareimporters.cpp
    systemtray.cpp
    tag.cpp
    tagsedit.cpp
    tools.cpp
    variouswidgets.cpp
    xmlwork.cpp
)

kde4_add_ui_files(basketcommon_LIB_SRCS passwordlayout.ui)

QT4_ADD_DBUS_ADAPTOR(basketcommon_LIB_SRCS org.basket.BNPView.xml bnpview.h BNPView)

QT4_ADD_RESOURCES(basketcommon_LIB_SRCS ../basket.qrc)

kde4_add_library(basketcommon SHARED ${basketcommon_LIB_SRCS})

target_link_libraries(basketcommon
  ${KDE4_KDE3SUPPORT_LIBS} # Remove me
  ${KDE4_KIO_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KPARTS_LIBS}
  ${KDE4_KFILE_LIBS}
  ${KDE4_PHONON_LIBRARY}
  ${QIMAGEBLITZ_LIBRARIES}
  ${GPGME_VANILLA_LIBRARIES})

if(Q_WS_X11)
    target_link_libraries(basketcommon ${X11_X11_LIB})
endif(Q_WS_X11)

set_target_properties(basketcommon PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS basketcommon DESTINATION ${LIB_INSTALL_DIR})

########### next target ###############

set(basket_SRCS main.cpp mainwindow.cpp application.cpp)

kde4_add_executable(basket ${basket_SRCS})
target_link_libraries(basket
  ${KDE4_KDE3SUPPORT_LIBS} # Remove me as well
  ${KDE4_KUTILS_LIBS}
  ${KDE4_KFILE_LIBS}
  ${KDE4_PHONON_LIBRARY}
  basketcommon)

install(TARGETS basket DESTINATION ${BIN_INSTALL_DIR})

########### next target ###############

set(kcm_basket_PART_SRCS kcm_basket.cpp)

kde4_add_plugin(kcm_basket ${kcm_basket_PART_SRCS})

target_link_libraries(kcm_basket ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} basketcommon)

install(TARGETS kcm_basket DESTINATION ${PLUGIN_INSTALL_DIR})

if(BUILD_KPARTS)

########### next target ###############

set(basketpart_PART_SRCS basket_part.cpp)

kde4_add_plugin(basketpart WITH_PREFIX ${basketpart_PART_SRCS})

target_link_libraries(basketpart ${KDE4_KPARTS_LIBS} basketcommon kparts kdeui ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS basketpart DESTINATION ${PLUGIN_INSTALL_DIR})

set(kontact_basket_PLUGIN_SRCS basket_plugin.cpp)

kde4_add_plugin(kontact_basketplugin ${kontact_basket_PLUGIN_SRCS})

qt4_add_dbus_interfaces(kontact_basket_PLUGIN_SRCS org.kde.basket.basket.xml)

target_link_libraries(kontact_basketplugin ${KDE4_CORE_LIBS} kontactinterfaces basketcommon kparts kdeui ${KDE4_KPARTS_LIBS})

install(TARGETS kontact_basketplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES basket_plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

########### install files ###############

install(FILES  basket_part.rc DESTINATION ${DATA_INSTALL_DIR}/basket)
install(FILES  basket_part.desktop DESTINATION ${SERVICES_INSTALL_DIR})

endif(BUILD_KPARTS)

install(FILES  basket.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES  basket_config_general.desktop  	basket_config_baskets.desktop  	basket_config_new_notes.desktop  	basket_config_notes_appearance.desktop  	basket_config_apps.desktop  	DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES  basketui.rc DESTINATION ${DATA_INSTALL_DIR}/basket)

kde4_install_icons(${ICON_INSTALL_DIR})



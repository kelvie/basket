find_package(OpenGL)

add_subdirectory( containments )

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR} 
	#${KDEBASE_WORKSPACE_SOURCE_DIR}/libs $
	#{OPENGL_INCLUDE_DIR}
	widgets
	plasma
)

#add_subdirectory(widgets)
#add_subdirectory(tests)

########### next target ###############

set(plasmagik_SRCS
    plasma/packagemetadata.cpp
    plasma/packagestructure.cpp
    plasma/package.cpp
    )

set(plasma_LIB_SRCS
    ${plasmagik_SRCS}
    plasma/abstractrunner.cpp
    plasma/animator.cpp
    plasma/applet.cpp
    plasma/appletbrowser.cpp
    plasma/appletbrowser/kcategorizeditemsview.cpp
    plasma/appletbrowser/kcategorizeditemsviewdelegate.cpp
    plasma/appletbrowser/kcategorizeditemsviewmodels.cpp
    plasma/appletbrowser/plasmaappletitemmodel.cpp
    plasma/appletbrowser/customdragtreeview.cpp
    plasma/applethandle.cpp
    plasma/configxml.cpp
    plasma/containment.cpp
    plasma/corona.cpp
    plasma/datacontainer.cpp
    plasma/dataengine.cpp
    plasma/dataenginemanager.cpp
    plasma/layouts/boxlayout.cpp
    plasma/layouts/borderlayout.cpp
    plasma/layouts/freelayout.cpp
    plasma/layouts/nodelayout.cpp
    plasma/layouts/flowlayout.cpp
    plasma/layouts/layout.cpp
    plasma/layouts/layoutanimator.cpp
    plasma/layouts/layoutitem.cpp
    plasma/packages.cpp
    plasma/phase.cpp
    plasma/plasma.cpp
    plasma/plasma_export.h
    plasma/scriptengine.cpp
    plasma/searchaction.cpp
    plasma/searchcontext.cpp
    plasma/shadowitem.cpp
    plasma/svg.cpp
    plasma/theme.cpp
    plasma/toolbox.cpp
    plasma/uiloader.cpp
    plasma/view.cpp
    plasma/widgets/checkbox.cpp
    plasma/widgets/flash.cpp
    plasma/widgets/icon.cpp
    plasma/widgets/label.cpp
    plasma/widgets/lineedit.cpp
    plasma/widgets/progressbar.cpp
    plasma/widgets/pushbutton.cpp
    plasma/widgets/radiobutton.cpp
#    widgets/rectangle.cpp
    plasma/widgets/widget.cpp
    plasma/widgets/signalplotter.cpp
    plasma/widgets/meter.cpp
)

kde4_add_ui_files (
    plasma_LIB_SRCS
    plasma/appletbrowser/kcategorizeditemsviewbase.ui
)

if(QT_QTOPENGL_FOUND AND OPENGL_FOUND)
MESSAGE(STATUS "Adding support for OpenGL applets to libplasma")
set(plasma_LIB_SRCS
    ${plasma_LIB_SRCS}
    plasma/glapplet.cpp)
endif(QT_QTOPENGL_FOUND AND OPENGL_FOUND)

kde4_add_library(basketplasma SHARED ${plasma_LIB_SRCS})

target_link_libraries(basketplasma ${KDE4_KIO_LIBS} ${QT_QTUITOOLS_LIBRARY})

if(QT_QTOPENGL_FOUND AND OPENGL_FOUND)
target_link_libraries(basketplasma ${QT_QTOPENGL_LIBRARY} ${OPENGL_gl_LIBRARY})
endif(QT_QTOPENGL_FOUND AND OPENGL_FOUND)

set_target_properties(basketplasma PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS basketplasma DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

#set(plasmagik_HEADERS
#    packagemetadata.h
#    packagestructure.h
#    package.h
#    )
#
#install(FILES ${plasmagik_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/plasma/)
#
#set(plasma_LIB_INCLUDES
#    abstractrunner.h
#    animator.h
#    applet.h
#    appletbrowser.h
#    configxml.h
#    containment.h
#    corona.h
#    datacontainer.h
#    dataengine.h
#    dataenginemanager.h
#    phase.h
#    plasma.h
#    plasma_export.h
#    scriptengine.h
#    searchaction.h
#    searchcontext.h
#    shadowitem_p.h
#    svg.h
#    theme.h
#    uiloader.h
#    view.h)
#
#if(QT_QTOPENGL_FOUND AND OPENGL_FOUND)
#set(plasma_LIB_INCLUDES
#    ${plasma_LIB_INCLUDES}
#    glapplet.h)
#endif(QT_QTOPENGL_FOUND AND OPENGL_FOUND)
#
#install(FILES
#        ${plasma_LIB_INCLUDES}
#        DESTINATION ${INCLUDE_INSTALL_DIR}/plasma)
#
#install(FILES
#    widgets/flash.h
#    widgets/icon.h
#    widgets/label.h
#    widgets/lineedit.h
#    widgets/progressbar.h
#    widgets/pushbutton.h
#    widgets/checkbox.h
#    widgets/radiobutton.h
##    widgets/rectangle.h
#    widgets/widget.h
#    widgets/signalplotter.h
#    widgets/meter.h
#    DESTINATION ${INCLUDE_INSTALL_DIR}/plasma/widgets)
#
#install(FILES
#    layouts/boxlayout.h
#    layouts/vboxlayout.h
#    layouts/hboxlayout.h
#    layouts/borderlayout.h
#    layouts/freelayout.h
#    layouts/nodelayout.h
#    layouts/flowlayout.h
#    layouts/layout.h
#    layouts/layoutanimator.h
#    layouts/layoutitem.h
#    DESTINATION ${INCLUDE_INSTALL_DIR}/plasma/layouts)
#
#install(FILES
#    includes/AbstractRunner
#    includes/Applet
#    includes/AppletBrowser
#    includes/ConfigXml
#    includes/Containment
#    includes/Phase
#    includes/Plasma
#    includes/Package
#    includes/PackageStructure
#    includes/Theme
#    includes/DataContainer
#    includes/DataEngine
#    includes/DataEngineManager
#    includes/ScriptEngine
#    includes/Svg
#    includes/UiLoader
#    includes/PackageMetadata
#    includes/View
#    DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Plasma)
#
#if(QT_QTOPENGL_FOUND AND OPENGL_FOUND)
#install(FILES
#    includes/GLApplet
#    DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Plasma)
#endif(QT_QTOPENGL_FOUND AND OPENGL_FOUND)
#
#
#install(FILES
#    servicetypes/plasma-animator.desktop
#    servicetypes/plasma-applet.desktop
#    servicetypes/plasma-dataengine.desktop
#    servicetypes/plasma-runner.desktop
#    servicetypes/plasma-scriptengine.desktop
#    DESTINATION ${SERVICETYPES_INSTALL_DIR})


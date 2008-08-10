cmake_minimum_required(VERSION 2.4)

find_package(KDE4 REQUIRED)
find_package(QImageBlitz REQUIRED)
find_package(Gpgme)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

add_definitions(-DQT3_SUPPORT)

# fix these warnings after we get it to compile
#add_definitions(-DQT3_SUPPORT_WARNINGS)

include(KDE4Defaults)

include(MacroLibrary)

include(ConvenienceLibs.cmake)

#include(ManualStuff.cmake)

IF(GPGME_FOUND)
  SET(HAVE_LIBGPGME 1)
  ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64)
ELSE(GPGME_FOUND)
  MESSAGE("GPG not found, configuring without")
ENDIF(GPGME_FOUND)

#TODO: find meinproc

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})


#include(ConfigureChecks.cmake)

include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )

#add_subdirectory(doc)
add_subdirectory(src)
add_subdirectory(tags)
add_subdirectory(welcome)
add_subdirectory(backgrounds)
add_subdirectory(images)

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(TOPSUBDIRS)")


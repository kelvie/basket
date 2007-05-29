project(basket)

find_package(KDE3 REQUIRED)

# tell cmake where to search for Qt/KDE headers:
include_directories(${KDE3_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )
add_definitions(${QT_DEFINITIONS} ${KDE3_DEFINITIONS})

# tell cmake where to search for libraries:
link_directories(${QT_LIB_DIR} ${KDE3_LIB_DIR})

add_subdirectory(src)


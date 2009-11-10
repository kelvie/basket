set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src 
                    ${CMAKE_BINARY_DIR}/src)

link_directories(${CMAKE_BINARY_DIR}/lib)

macro(basket_standalone_unit_test _testname)
    kde4_add_unit_test(${_testname} ${_testname}.cpp ${_extra_sources} ${ARGN})
    target_link_libraries(${_testname} basketcommon ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})
endmacro(basket_standalone_unit_test)

basket_standalone_unit_test(notetest)
basket_standalone_unit_test(basketviewtest)

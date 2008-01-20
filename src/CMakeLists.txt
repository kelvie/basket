
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_subdirectory( plugins )

set( basket_SRCS 
	basketapplication.cpp
	basketmainwindow.cpp
	basketsystemtray.cpp
	baskettreemodel.cpp
	#baskettreeview.cpp
	baskettreewidget.cpp
	basketviewcontainer.cpp
	basketcontent.cpp
	newbasketdialog.cpp
	notewidget.cpp
	basketsettingsdialog.cpp
	note.cpp
	main.cpp 
)

kde4_add_executable( basket ${basket_SRCS} )
target_link_libraries( basket akonadicomponents kdepim ${KDE_CORE_LIBS} ${KDE4_KDEUI_LIBS} )
install( TARGETS basket DESTINATION ${BIN_INSTALL_DIR} )

install( FILES basketui.rc DESTINATION ${DATA_INSTALL_DIR}/basket )


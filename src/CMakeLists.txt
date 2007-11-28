
set( basket_SRCS 
	basketapplication.cpp
	basketmainwindow.cpp
	basketsystemtray.cpp
	baskettreeview.cpp
	basketviewcontainer.cpp
	newbasketdialog.cpp
	main.cpp 
)

kde4_add_executable( basket ${basket_SRCS} )
target_link_libraries( basket ${KDE_CORE_LIBS} ${KDE4_KDEUI_LIBS} )
install( TARGETS basket DESTINATION ${BIN_INSTALL_DIR} )

install( FILES basketui.rc DESTINATION ${DATA_INSTALL_DIR}/basket )


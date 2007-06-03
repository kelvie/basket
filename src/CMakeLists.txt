set(basketSources
	aboutdata.cpp
	application.cpp
	archive.cpp
	backgroundmanager.cpp
	backup.cpp
	basket.cpp
	basketdcopiface_skel.cpp
	basketfactory.cpp
	basketlistview.cpp
	basket_part.cpp
	basketproperties.cpp
	basketstatusbar.cpp
	bnpview.cpp
	clickablelabel.cpp
	colorpicker.cpp
	crashhandler.cpp
	debugwindow.cpp
	exporterdialog.cpp
	filter.cpp
	focusedwidgets.cpp
	formatimporter.cpp
	global.cpp
	htmlexporter.cpp
	kcm_basket.cpp
	kcolorcombo2.cpp
	keyboard.cpp
	kgpgme.cpp
	kiconcanvas.cpp
	kicondialog.cpp
	kicondialogui.cpp
	likeback.cpp
	linklabel.cpp
	main.cpp
	mainwindow.cpp
	newbasketdialog.cpp
	notecontent.cpp
	note.cpp
	notedrag.cpp
	noteedit.cpp
	notefactory.cpp
	password.cpp
	passwordlayout.cpp
	popupmenu.cpp
	qeffects.cpp
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

kde3_automoc( ${basketSources} )

kde3_add_executable(basket ${basketSources})

target_link_libraries(basket ${QT_AND_KDECORE_LIBS} kdeui kio kutils kparts artskde ${GPGME_LIB})

install(TARGETS basket RUNTIME DESTINATION bin)
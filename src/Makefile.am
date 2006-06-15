# set the include path for X, qt and KDE
INCLUDES = $(all_includes)

# these are the headers for your project


# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/basket.pot

KDE_ICON = basket

#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = basket

# the application source, library search path, and link libraries
AM_CPPFLAGS = $(GPGME_CFLAGS)
basket_LDFLAGS = $(KDE_RPATH) $(all_libraries)
basket_LDADD = $(LIB_ARTSKDE) -lkio $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) \
	$(LIBSOCKET) $(GPGME_LIBS)

# this is where the desktop file will go
shelldesktopdir = $(kde_appsdir)/Utilities
shelldesktop_DATA = basket.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir = $(kde_datadir)/basket
shellrc_DATA = basketui.rc




noinst_HEADERS = basket.h basketproperties.h container.h debugwindow.h \
				linklabel.h variouswidgets.h xmlwork.h keyboard.h global.h settings.h \
				focusedwidgets.h popupmenu.h basketfactory.h exporterdialog.h tag.h qeffects.h \
				kcolorcombo2.h newbasketdialog.h tagsedit.h formatimporter.h note.h notecontent.h \
				notedrag.h noteedit.h notefactory.h filter.h tools.h backgroundmanager.h \
				regiongrabber.h softwareimporters.h kicondialog.h kiconcanvas.h kgpgme.h likeback.h \
		crashhandler.h password.h
basket_SOURCES = basket.cpp basketproperties.cpp container.cpp debugwindow.cpp \
				linklabel.cpp variouswidgets.cpp main.cpp xmlwork.cpp keyboard.cpp global.cpp \
				settings.cpp focusedwidgets.cpp popupmenu.cpp basketfactory.cpp exporterdialog.cpp \
				tag.cpp qeffects.cpp kcolorcombo2.cpp newbasketdialog.cpp tagsedit.cpp \
				formatimporter.cpp note.cpp notecontent.cpp notedrag.cpp noteedit.cpp notefactory.cpp \
				filter.cpp tools.cpp backgroundmanager.cpp regiongrabber.cpp softwareimporters.cpp \
				kicondialogui.ui kicondialog.cpp kiconcanvas.cpp kgpgme.cpp likeback.cpp \
		crashhandler.cpp passwordlayout.ui password.cpp
kde_icon_KDEICON = cr128-app-basket.png cr16-app-basket.png cr22-app-basket.png \
	cr32-app-basket.png cr48-app-basket.png cr64-app-basket.png hi16-app-basket_old.png \
	hi32-app-basket_old.png crsc-app-basket.svg cr16-action-likeback_configure.png \
	cr16-action-likeback_dislike.png cr16-action-likeback_like.png

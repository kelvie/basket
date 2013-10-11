#-------------------------------------------------
#
# Project created by QtCreator 2012-06-13T11:04:20
#
#-------------------------------------------------

QT       += gui xml network dbus

TARGET = basket
TEMPLATE = app

SOURCES += \
    src/xmlwork.cpp \
    src/variouswidgets.cpp \
    src/transparentwidget.cpp \
    src/tools.cpp \
    src/tagsedit.cpp \
    src/tag.cpp \
    src/systemtray.cpp \
    src/softwareimporters.cpp \
    src/settings.cpp \
    src/regiongrabber.cpp \
    src/password.cpp \
    src/noteselection.cpp \
    src/notefactory.cpp \
    src/noteedit.cpp \
    src/notedrag.cpp \
    src/notecontent.cpp \
    src/note.cpp \
    src/newbasketdialog.cpp \
    src/mainwindow.cpp \
    src/main.cpp \
    src/linklabel.cpp \
    src/likeback.cpp \
    src/ksystemtrayicon2.cpp \
    src/kgpgme.cpp \
    src/kcolorcombo2.cpp \
    src/kcm_basket.cpp \
    src/htmlexporter.cpp \
    src/global.cpp \
    src/formatimporter.cpp \
    src/focusedwidgets.cpp \
    src/filter.cpp \
    src/exporterdialog.cpp \
    src/diskerrordialog.cpp \
    src/decoratedbasket.cpp \
    src/debugwindow.cpp \
    src/crashhandler.cpp \
    src/colorpicker.cpp \
    src/bnpview.cpp \
    src/basketview.cpp \
    src/basketstatusbar.cpp \
    src/basketproperties.cpp \
    #src/basket_plugin.cpp \
    src/basket_part.cpp \
    src/basketlistview.cpp \
    src/basketfactory.cpp \
    src/backup.cpp \
    src/backgroundmanager.cpp \
    src/archive.cpp \
    src/application.cpp \
    src/aboutdata.cpp \
    src/basketscene.cpp \
    src/history.cpp

OTHER_FILES +=

HEADERS += \
    src/xmlwork.h \
    src/variouswidgets.h \
    src/transparentwidget.h \
    src/tools.h \
    src/tagsedit.h \
    src/tag.h \
    src/systemtray.h \
    src/softwareimporters.h \
    src/settings.h \
    src/regiongrabber.h \
    src/password.h \
    src/noteselection.h \
    src/notefactory.h \
    src/noteedit.h \
    src/notedrag.h \
    src/notecontent.h \
    src/note.h \
    src/newbasketdialog.h \
    src/mainwindow.h \
    src/linklabel.h \
    src/likeback_p.h \
    src/likeback.h \
    src/ksystemtrayicon2.h \
    src/kgpgme.h \
    src/kcolorcombo2.h \
    src/htmlexporter.h \
    src/global.h \
    src/formatimporter.h \
    src/focusedwidgets.h \
    src/filter.h \
    src/exporterdialog.h \
    src/diskerrordialog.h \
    src/decoratedbasket.h \
    src/debugwindow.h \
    src/crashhandler.h \
    src/colorpicker.h \
    src/bnpview.h \
    src/basketview.h \
    src/basketstatusbar.h \
    src/basketproperties.h \
    #src/basket_plugin.h \
    src/basket_part.h \
    src/basket_options.h \
    src/basketlistview.h \
    src/basketfactory.h \
    src/basket_export.h \
    src/backup.h \
    src/backgroundmanager.h \
    src/archive.h \
    src/application.h \
    src/aboutdata.h \
    config.h \
    src/basketscene.h \
    src/history.h \
    src/icon_names.h

FORMS += \
    src/passwordlayout.ui \
    src/basketproperties.ui

INCLUDEPATH += /usr/include/KDE /usr/include/qt4/Qt /usr/include/phonon \
/home/user/basket/basket/src

LIBS += -lkdecore -lX11 -lkparts -lkdeui -lkio -lQtDBus -lqimageblitz -lkontactinterface -lphonon -lkfile -lkcmutils

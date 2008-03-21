
include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############

install(FILES  DESTINATION ${DATA_INSTALL_DIR}/basket/icons)

kde4_install_icons(${ICON_INSTALL_DIR})



#original Makefile.am contents follow:

#tagsdir = $(kde_datadir)/basket/icons
#tags_ICON = AUTO
#KDE_ICON = AUTO

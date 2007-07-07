INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

INSTALL(FILES /share/apps/basket/icons )

KDE4_INSTALL_ICONS( hicolor )



#original Makefile.am contents follow:

#tagsdir = $(kde_datadir)/basket/icons
#tags_ICON = AUTO
#KDE_ICON = AUTO

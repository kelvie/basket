INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

INSTALL(FILES abstract.png  	alien-artifact.png  	basket-title.png  	green-curves.png  	light.png  	painting.png  	pens.png  	pins.png  	rainbow-balls.png  	rounds-line.png  	strings.png  	todo.png  	working.png DESTINATION ${DATA_INSTALL_DIR}/basket/backgrounds/previews)



#original Makefile.am contents follow:

#backgroundspreviewsdir = $(kde_datadir)/basket/backgrounds/previews
#backgroundspreviews_DATA = \
#	abstract.png \
#	alien-artifact.png \
#	basket-title.png \
#	green-curves.png \
#	light.png \
#	painting.png \
#	pens.png \
#	pins.png \
#	rainbow-balls.png \
#	rounds-line.png \
#	strings.png \
#	todo.png \
#	working.png

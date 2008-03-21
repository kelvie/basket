
include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############

install(FILES  abstract.png  	alien-artifact.png alien-artifact.png.config  	basket-title.png  	green-curves.png  	light.png  	painting.png  	pens.png  	pins.png  	rainbow-balls.png  	rounds-line.png  	strings.png  	todo.png  	working.png DESTINATION ${DATA_INSTALL_DIR}/basket/backgrounds)



#original Makefile.am contents follow:

#backgroundsdir = $(kde_datadir)/basket/backgrounds
#backgrounds_DATA = \
#	abstract.png \
#	alien-artifact.png alien-artifact.png.config \
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

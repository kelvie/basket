
include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############

install(FILES  insertion_help.png                tag_export_help.png                tag_export_on_every_lines_help.png DESTINATION ${DATA_INSTALL_DIR}/basket/images)



#original Makefile.am contents follow:

#imagesdir = $(kde_datadir)/basket/images
#images_DATA = insertion_help.png \
#              tag_export_help.png \
#              tag_export_on_every_lines_help.png

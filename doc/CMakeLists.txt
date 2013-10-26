
include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )

########### install files ###############


kde4_create_handbook(index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en SUBDIR basket)



#original Makefile.am contents follow:

## the SUBDIRS is filled automatically by am_edit. If files are 
## in this directory they are installed into the english dir
#
#KDE_LANG = en
#KDE_DOCS = basket


include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### install files ###############

install(FILES  Welcome_de.baskets  	Welcome_en_US.baskets  	Welcome_fr.baskets  	Welcome_it.baskets  	Welcome_ja.baskets  	Welcome_nn.baskets  	Welcome_pt.baskets  	Welcome_ru.baskets DESTINATION ${DATA_INSTALL_DIR}/basket/welcome)



#original Makefile.am contents follow:

#welcomedir = $(kde_datadir)/basket/welcome
#welcome_DATA = \
#	Welcome_de.baskets \
#	Welcome_en_US.baskets \
#	Welcome_fr.baskets \
#	Welcome_it.baskets \
#	Welcome_ja.baskets \
#	Welcome_nn.baskets \
#	Welcome_pt.baskets \
#	Welcome_ru.baskets

FIND_PACKAGE(Qt4 REQUIRED)
FIND_PACKAGE(KDE4 REQUIRED)

SET(CMAKE_VERBOSE_MAKEFILE ON)

ADD_DEFINITIONS(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

LINK_DIRECTORIES(${KDE4_LIB_DIR})

MESSAGE(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(TOPSUBDIRS)")
INCLUDE_DIRECTORIES( ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR}  )


ADD_SUBDIRECTORY(doc)
ADD_SUBDIRECTORY(src)
########### install files ###############




#original Makefile.am contents follow:

#SUBDIRS = $(TOPSUBDIRS)
#
#$(top_srcdir)/configure.in: configure.in.in $(top_srcdir)/subdirs
#	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common configure.in ;
#
#$(top_srcdir)/subdirs:
#	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common subdirs
#
#$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in
#	@cd $(top_srcdir) && cat admin/acinclude.m4.in admin/libtool.m4.in > acinclude.m4
#
#MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 
#
#package-messages:
#	$(MAKE) -f admin/Makefile.common package-messages
#	$(MAKE) -C po merge
#
#EXTRA_DIST = admin COPYING configure.in.in
#
#dist-hook:
#	cd $(top_distdir) && perl admin/am_edit -padmin
#	cd $(top_distdir) && $(MAKE) -f admin/Makefile.common subdirs

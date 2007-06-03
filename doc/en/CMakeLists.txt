macro (KDE4_INSTALL_HANDBOOK)
   get_filename_component(_tmp_FILE ${CMAKE_CURRENT_SOURCE_DIR} ABSOLUTE)
   get_filename_component(_basename ${_tmp_FILE} NAME_WE)
   file(GLOB _books *.docbook)
   file(GLOB _images *.png)
   set(relative ${ARGV0})
   set( dirname ${relative}/${_basename})
   install(FILES ${CMAKE_CURRENT_BINARY_DIR}/index.cache.bz2 ${_books} ${_images} DESTINATION ${HTML_INSTALL_DIR}/en/${dirname})
   # TODO symlinks on non-unix platforms
   if (UNIX)
       # execute some cmake code on make install which creates the symlink
       install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${HTML_INSTALL_DIR}/en/common\"  \"${HTML_INSTALL_DIR}/en/${dirname}/common\" )" )
   endif (UNIX)
endmacro (KDE4_INSTALL_HANDBOOK )


macro (KDE4_CREATE_HANDBOOK _docbook)
   get_filename_component(_input ${_docbook} ABSOLUTE)
   set(_doc ${CMAKE_CURRENT_BINARY_DIR}/index.cache.bz2)

   #Boostrap
#    if (_kdeBootStrapping)
#       set(_ssheet ${CMAKE_SOURCE_DIR}/kdoctools/customization/kde-chunk.xsl)
#       set(_bootstrapOption "--srcdir=${CMAKE_SOURCE_DIR}/kdoctools/")
#    else (_kdeBootStrapping)
#       set(_ssheet ${KDE4_DATA_INSTALL_DIR}/ksgmltools2/customization/kde-chunk.xsl)
#       set(_bootstrapOption)
#    endif (_kdeBootStrapping)

   add_custom_command(OUTPUT ${_doc}
      COMMAND ${KDE4_MEINPROC_EXECUTABLE} --check ${_bootstrapOption} --cache ${_doc} ${_input}
      DEPENDS ${_input} ${_KDE4_MEINPROC_EXECUTABLE_DEP} ${_ssheet}
   )
   add_custom_target(handbook ALL DEPENDS ${_doc})
endmacro (KDE4_CREATE_HANDBOOK)

KDE4_CREATE_HANDBOOK(index.docbook)

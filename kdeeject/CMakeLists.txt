kde4_header()
include_directories(  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} )


########### install files ###############
install_files(/bin FILES kdeeject)

kde4_footer()



#original Makefile.am contents follow:

#bin_SCRIPTS = kdeeject


include_directories( ${KDE4_KPARTS_INCLUDES} )

########### next target ###############

set(kcmshell_KDEINIT_SRCS main.cpp )


kde4_add_kdeinit_executable( kcmshell ${kcmshell_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcmshell ${KDE4_KUTILS_LIBS} )

install(TARGETS kdeinit_kcmshell  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kcmshell kdeinit_kcmshell )
install(TARGETS kcmshell DESTINATION ${BIN_INSTALL_DIR} )

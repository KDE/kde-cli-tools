
set(kcmshell_KDEINIT_SRCS main.cpp )

kde4_add_kdeinit_executable( kcmshell4 ${kcmshell_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcmshell4 ${KDE4_KUTILS_LIBS} )

install(TARGETS kdeinit_kcmshell4 
        RUNTIME DESTINATION ${BIN_INSTALL_DIR}
        LIBRARY DESTINATION ${LIB_INSTALL_DIR}
        ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
)

target_link_libraries( kcmshell4 kdeinit_kcmshell4 )
install(TARGETS kcmshell4 DESTINATION ${BIN_INSTALL_DIR} )

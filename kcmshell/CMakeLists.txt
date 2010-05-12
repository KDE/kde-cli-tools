
set(kcmshell_KDEINIT_SRCS main.cpp )

kde4_add_kdeinit_executable( kcmshell4 ${kcmshell_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcmshell4 ${KDE4_KCMUTILS_LIBS} ${KDE4_KDEUI_LIBS} )

install(TARGETS kdeinit_kcmshell4  ${INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS kcmshell4          ${INSTALL_TARGETS_DEFAULT_ARGS})

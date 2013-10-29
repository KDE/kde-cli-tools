
set(kcmshell_KDEINIT_SRCS main.cpp )

kf5_add_kdeinit_executable( kcmshell5 ${kcmshell_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcmshell5 Qt5::DBus KF5::KCMUtils KF5::KI18n KF5::KDE4Support )

install(TARGETS kdeinit_kcmshell5  ${INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS kcmshell5          ${INSTALL_TARGETS_DEFAULT_ARGS})

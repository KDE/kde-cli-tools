add_executable(kstart kstart)

target_link_libraries(kstart KF5::KDE4Support ${X11_X11_LIB})

install(TARGETS kstart ${INSTALL_TARGETS_DEFAULT_ARGS})


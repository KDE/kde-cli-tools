add_executable(kstart kstart)

target_link_libraries(kstart ${KDE4_KDEUI_LIBS} ${X11_X11_LIB})

install(TARGETS kstart ${INSTALL_TARGETS_DEFAULT_ARGS})


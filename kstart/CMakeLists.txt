
set(kstart_SRCS kstart.cpp)


kde4_add_executable(kstart ${kstart_SRCS})

target_link_libraries(kstart ${KDE4_KDEUI_LIBS} ${X11_X11_LIB})

install(TARGETS kstart ${INSTALL_TARGETS_DEFAULT_ARGS})


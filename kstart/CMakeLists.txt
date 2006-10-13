project(kstart)

set(kstart_SRCS kstart.cpp )

kde4_automoc(kstart ${kstart_SRCS})

kde4_add_executable(kstart ${kstart_SRCS})

target_link_libraries(kstart  ${KDE4_KDECORE_LIBS} )

install(TARGETS kstart DESTINATION ${BIN_INSTALL_DIR})


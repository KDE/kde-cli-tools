



########### next target ###############

set(kreadconfig_SRCS kreadconfig.cpp )


kde4_add_executable(kreadconfig NOGUI ${kreadconfig_SRCS})

target_link_libraries(kreadconfig  ${KDE4_KDECORE_LIBS} )

install(TARGETS kreadconfig ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(kwriteconfig_SRCS kwriteconfig.cpp )


kde4_add_executable(kwriteconfig NOGUI ${kwriteconfig_SRCS})

target_link_libraries(kwriteconfig ${KDE4_KDECORE_LIBS} )

install(TARGETS kwriteconfig ${INSTALL_TARGETS_DEFAULT_ARGS})


project(kreadconfig)



########### next target ###############

set(kreadconfig_SRCS kreadconfig.cpp )

kde4_automoc(${kreadconfig_SRCS})

kde4_add_executable(kreadconfig ${kreadconfig_SRCS})

target_link_libraries(kreadconfig  ${KDE4_KDECORE_LIBS} )

install(TARGETS kreadconfig DESTINATION ${BIN_INSTALL_DIR})


########### next target ###############

set(kwriteconfig_SRCS kwriteconfig.cpp )

kde4_automoc(${kwriteconfig_SRCS})

kde4_add_executable(kwriteconfig ${kwriteconfig_SRCS})

target_link_libraries(kwriteconfig  ${KDE4_KDECORE_LIBS} )

install(TARGETS kwriteconfig DESTINATION ${BIN_INSTALL_DIR})

kde4_add_executable(kmimetypefinder NOGUI kmimetypefinder.cpp)

target_link_libraries(kmimetypefinder ${KDE4_KDECORE_LIBS})

install(TARGETS kmimetypefinder ${INSTALL_TARGETS_DEFAULT_ARGS} )

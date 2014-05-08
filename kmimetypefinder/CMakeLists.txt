add_executable(kmimetypefinder5 kmimetypefinder.cpp)
ecm_mark_nongui_executable(kmimetypefinder5)

target_link_libraries(kmimetypefinder5 KF5::KDE4Support)

install(TARGETS kmimetypefinder5 ${INSTALL_TARGETS_DEFAULT_ARGS} )

add_executable(kmimetypefinder kmimetypefinder.cpp)
ecm_mark_nongui_executable(kmimetypefinder)

target_link_libraries(kmimetypefinder KF5::KDE4Support)

install(TARGETS kmimetypefinder ${INSTALL_TARGETS_DEFAULT_ARGS} )

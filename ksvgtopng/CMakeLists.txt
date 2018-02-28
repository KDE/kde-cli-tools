add_executable(ksvgtopng5 ksvgtopng.cpp)
ecm_mark_nongui_executable(ksvgtopng5)
target_link_libraries(ksvgtopng5 Qt5::Svg)
install(TARGETS ksvgtopng5 ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

add_executable(ksvgtopng ksvgtopng.cpp)
ecm_mark_nongui_executable(ksvgtopng)
target_link_libraries(ksvgtopng Qt5::Svg)
install(TARGETS ksvgtopng ${INSTALL_TARGETS_DEFAULT_ARGS})

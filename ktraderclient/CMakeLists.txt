add_executable(ktraderclient5 ktraderclient.cpp)
target_compile_definitions(ktraderclient5 PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
ecm_mark_nongui_executable(ktraderclient5)

target_link_libraries(ktraderclient5 KF5::KIOWidgets KF5::KDE4Support)

install(TARGETS ktraderclient5 ${INSTALL_TARGETS_DEFAULT_ARGS} )

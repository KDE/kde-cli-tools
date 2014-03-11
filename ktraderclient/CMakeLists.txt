add_executable(ktraderclient ktraderclient.cpp)
ecm_mark_nongui_executable(ktraderclient)

target_link_libraries(ktraderclient KF5::KIOWidgets KF5::KDE4Support)

install(TARGETS ktraderclient ${INSTALL_TARGETS_DEFAULT_ARGS} )

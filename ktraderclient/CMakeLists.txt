add_executable(ktraderclient ktraderclient.cpp)
ecm_mark_nongui_executable(ktraderclient)

target_link_libraries(ktraderclient ${KDE4_KIO_LIBS})

install(TARGETS ktraderclient ${INSTALL_TARGETS_DEFAULT_ARGS} )

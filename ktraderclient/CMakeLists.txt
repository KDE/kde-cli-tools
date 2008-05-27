kde4_add_executable(ktraderclient NOGUI ktraderclient.cpp)

target_link_libraries(ktraderclient ${KDE4_KIO_LIBS})

install(TARGETS ktraderclient ${INSTALL_TARGETS_DEFAULT_ARGS} )

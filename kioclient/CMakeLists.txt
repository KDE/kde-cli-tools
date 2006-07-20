set(kioclient_SRCS kioclient.cpp )

kde4_automoc(${kioclient_SRCS})

kde4_add_executable(kioclient ${kioclient_SRCS})

target_link_libraries(kioclient ${KDE4_KIO_LIBS})

install(TARGETS kioclient DESTINATION ${BIN_INSTALL_DIR})

################

set(kde-open_SRCS kioclient.cpp )

kde4_automoc(${kde-open_SRCS})

kde4_add_executable(kde-open ${kde-open_SRCS})

target_link_libraries(kde-open ${KDE4_KIO_LIBS})

install(TARGETS kde-open DESTINATION ${BIN_INSTALL_DIR})

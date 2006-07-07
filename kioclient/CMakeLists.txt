set(kioclient_SRCS kioclient.cpp )

kde4_automoc(${kioclient_SRCS})

kde4_add_executable(kioclient ${kioclient_SRCS})

target_link_libraries(kioclient ${KDE4_KIO_LIBS})

install(TARGETS kioclient DESTINATION bin)

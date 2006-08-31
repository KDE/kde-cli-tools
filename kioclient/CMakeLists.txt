set(kioclient_SRCS kioclient.cpp )
kde4_automoc(${kioclient_SRCS})
kde4_add_executable(kioclient ${kioclient_SRCS})
macro_add_compile_flags(kioclient -DKIOCLIENT_AS_KIOCLIENT)
target_link_libraries(kioclient ${KDE4_KIO_LIBS})
install(TARGETS kioclient DESTINATION ${BIN_INSTALL_DIR})

################

set(kde-open_SRCS kioclient.cpp )
kde4_automoc(${kde-open_SRCS})
kde4_add_executable(kde-open ${kde-open_SRCS})
macro_add_compile_flags(kde-open -DKIOCLIENT_AS_KDEOPEN)
target_link_libraries(kde-open ${KDE4_KIO_LIBS})
install(TARGETS kde-open DESTINATION ${BIN_INSTALL_DIR})

################

set(kde-cp_SRCS kioclient.cpp )
kde4_automoc(${kde-cp_SRCS})
kde4_add_executable(kde-cp ${kde-cp_SRCS})
macro_add_compile_flags(kde-cp -DKIOCLIENT_AS_KDECP)
target_link_libraries(kde-cp ${KDE4_KIO_LIBS})
install(TARGETS kde-cp DESTINATION ${BIN_INSTALL_DIR})

################

set(kde-mv_SRCS kioclient.cpp )
kde4_automoc(${kde-mv_SRCS})
kde4_add_executable(kde-mv ${kde-mv_SRCS})
macro_add_compile_flags(kde-mv -DKIOCLIENT_AS_KDEMV)
target_link_libraries(kde-mv ${KDE4_KIO_LIBS})
install(TARGETS kde-mv DESTINATION ${BIN_INSTALL_DIR})


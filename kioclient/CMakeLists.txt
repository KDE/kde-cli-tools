set(kioclient_SRCS kioclient.cpp )
kde4_add_executable(kioclient NOGUI ${kioclient_SRCS})
macro_add_compile_flags(kioclient -DKIOCLIENT_AS_KIOCLIENT)
target_link_libraries(kioclient ${KDE4_KIO_LIBS})
install(TARGETS kioclient ${INSTALL_TARGETS_DEFAULT_ARGS})

################

set(kde-open_SRCS kioclient.cpp )
kde4_add_executable(kde-open NOGUI ${kde-open_SRCS})
macro_add_compile_flags(kde-open -DKIOCLIENT_AS_KDEOPEN)
target_link_libraries(kde-open ${KDE4_KIO_LIBS})
install(TARGETS kde-open ${INSTALL_TARGETS_DEFAULT_ARGS})

################

set(kde-cp_SRCS kioclient.cpp )
kde4_add_executable(kde-cp NOGUI ${kde-cp_SRCS})
macro_add_compile_flags(kde-cp -DKIOCLIENT_AS_KDECP)
target_link_libraries(kde-cp ${KDE4_KIO_LIBS})
install(TARGETS kde-cp ${INSTALL_TARGETS_DEFAULT_ARGS})

################

set(kde-mv_SRCS kioclient.cpp )
kde4_add_executable(kde-mv NOGUI ${kde-mv_SRCS})
macro_add_compile_flags(kde-mv -DKIOCLIENT_AS_KDEMV)
target_link_libraries(kde-mv ${KDE4_KIO_LIBS})
install(TARGETS kde-mv ${INSTALL_TARGETS_DEFAULT_ARGS})


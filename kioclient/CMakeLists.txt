include_directories(${KDE4_INCLUDE_DIR}/kio ${KDE4_INCLUDE_DIR}/KDE) 
set(kioclient_SRCS kioclient.cpp )
kde4_add_executable(kioclient NOGUI ${kioclient_SRCS})
set_property(TARGET kioclient APPEND_STRING PROPERTY COMPILE_FLAGS "-DKIOCLIENT_AS_KIOCLIENT")
target_link_libraries(kioclient 
        ${KDE4_KIO_LIBS} 
        ${KDE4_SOLID_LIBS}   
        ${KDE4Support_LIBRARIES}
        ${KDECLARATIVE_LIBRARIES} 
        KF5::KI18n KF5::KConfigCore KF5::KConfigGui
        ${KDE4_KDEUI_LIBS} 
        ${KDE4_SOLID_LIBS}
        ${KCore_LIBRARIES}
        kdeqt5staging
        ${Qt5Script_LIBRARIES}
        )
install(TARGETS kioclient ${INSTALL_TARGETS_DEFAULT_ARGS})

################

set(kdeopen_SRCS kioclient.cpp )
kde4_add_executable(kdeopen NOGUI ${kdeopen_SRCS})
set_property(TARGET kdeopen APPEND_STRING PROPERTY COMPILE_FLAGS "-DKIOCLIENT_AS_KDEOPEN")
#macro_add_compile_flags(kde-open -DKIOCLIENT_AS_KDEOPEN)
target_link_libraries(kdeopen ${KDE4_KIO_LIBS}
        ${KDE4_SOLID_LIBS}   
        ${KDE4Support_LIBRARIES}
        ${KDECLARATIVE_LIBRARIES} 
        KF5::KI18n KF5::KConfigCore KF5::KConfigGui
        ${KDE4_KDEUI_LIBS} 
        ${KDE4_SOLID_LIBS}
        ${KCore_LIBRARIES}
        kdeqt5staging
        ${Qt5Script_LIBRARIES})
install(TARGETS kdeopen ${INSTALL_TARGETS_DEFAULT_ARGS})

################
set(kdecp_SRCS kioclient.cpp )
kde4_add_executable(kdecp NOGUI ${kdecp_SRCS})
set_property(TARGET kdecp APPEND_STRING PROPERTY COMPILE_FLAGS "-DKIOCLIENT_AS_KDECP")
#macro_add_compile_flags(kde-open -DKIOCLIENT_AS_KDEOPEN)
target_link_libraries(kdecp ${KDE4_KIO_LIBS}
        ${KDE4_SOLID_LIBS}   
        ${KDE4Support_LIBRARIES}
        ${KDECLARATIVE_LIBRARIES} 
        KF5::KI18n KF5::KConfigCore KF5::KConfigGui
        ${KDE4_KDEUI_LIBS} 
        ${KDE4_SOLID_LIBS}
        ${KCore_LIBRARIES}
        kdeqt5staging
        ${Qt5Script_LIBRARIES})
install(TARGETS kdecp ${INSTALL_TARGETS_DEFAULT_ARGS})


################
set(kdemv_SRCS kioclient.cpp )
kde4_add_executable(kdemv NOGUI ${kdemv_SRCS})
set_property(TARGET kdemv APPEND_STRING PROPERTY COMPILE_FLAGS "-DKIOCLIENT_AS_KDEMV")
#macro_add_compile_flags(kde-open -DKIOCLIENT_AS_KDEOPEN)
target_link_libraries(kdemv ${KDE4_KIO_LIBS}
        ${KDE4_SOLID_LIBS}   
        ${KDE4Support_LIBRARIES}
        ${KDECLARATIVE_LIBRARIES} 
        KF5::KI18n KF5::KConfigCore KF5::KConfigGui
        ${KDE4_KDEUI_LIBS} 
        ${KDE4_SOLID_LIBS}
        ${KCore_LIBRARIES}
        kdeqt5staging
        ${Qt5Script_LIBRARIES})
install(TARGETS kdemv ${INSTALL_TARGETS_DEFAULT_ARGS})
#set(kdemv_SRCS kioclient.cpp )
#kde4_add_executable(kde-mv NOGUI ${kdemv_SRCS})
#set_property(TARGET kio-mv APPEND_STRING PROPERTY COMPILE_FLAGS "-DKIOCLIENT_AS_KDEMV")
#macro_add_compile_flags(kde-mv -DKIOCLIENT_AS_KDEMV)
#target_link_libraries(kde-mv ${KDE4_KIO_LIBS})
#install(TARGETS kde-mv ${INSTALL_TARGETS_DEFAULT_ARGS})


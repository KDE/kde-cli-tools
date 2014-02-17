set(kioclient_SRCS kioclient.cpp )
kde4_add_executable(kioclient ${kioclient_SRCS})
set_property(TARGET kioclient APPEND_STRING PROPERTY COMPILE_FLAGS "-DKIOCLIENT_AS_KIOCLIENT")
target_link_libraries(kioclient 
	Qt5::Core
	Qt5::Script
	Qt5::DBus
	KF5::CoreAddons
	KF5::KIOCore KF5::KIOWidgets 
        KF5::Solid  
        KF5::KIOFileWidgets	
	KF5::Declarative
        KF5::I18n 
	KF5::ConfigCore
	KF5::ConfigGui
        KF5::Solid
        KF5::KDE4Support
        )
install(TARGETS kioclient ${INSTALL_TARGETS_DEFAULT_ARGS})

################

set(kdeopen_SRCS kioclient.cpp )
kde4_add_executable(kdeopen NOGUI ${kdeopen_SRCS})
set_property(TARGET kdeopen APPEND_STRING PROPERTY COMPILE_FLAGS "-DKIOCLIENT_AS_KDEOPEN")
#macro_add_compile_flags(kde-open -DKIOCLIENT_AS_KDEOPEN)
target_link_libraries(kdeopen KF5::KIOCore KF5::KIOWidgets
        KF5::Solid   
        KF5::KDE4Support
        ${KDECLARATIVE_LIBRARIES} 
        KF5::I18n KF5::ConfigCore KF5::ConfigGui Qt5::DBus
        KF5::Solid
        ${KCore_LIBRARIES}
        ${Qt5Script_LIBRARIES})
install(TARGETS kdeopen ${INSTALL_TARGETS_DEFAULT_ARGS})

################
set(kdecp_SRCS kioclient.cpp )
kde4_add_executable(kdecp NOGUI ${kdecp_SRCS})
set_property(TARGET kdecp APPEND_STRING PROPERTY COMPILE_FLAGS "-DKIOCLIENT_AS_KDECP")
#macro_add_compile_flags(kde-open -DKIOCLIENT_AS_KDEOPEN)
target_link_libraries(kdecp KF5::KIOCore KF5::KIOWidgets
        KF5::Solid   
        KF5::KDE4Support
        ${KDECLARATIVE_LIBRARIES} 
        KF5::I18n KF5::ConfigCore KF5::ConfigGui Qt5::DBus
        KF5::Solid
        ${KCore_LIBRARIES}
        ${Qt5Script_LIBRARIES})
install(TARGETS kdecp ${INSTALL_TARGETS_DEFAULT_ARGS})


################
set(kdemv_SRCS kioclient.cpp )
kde4_add_executable(kdemv NOGUI ${kdemv_SRCS})
set_property(TARGET kdemv APPEND_STRING PROPERTY COMPILE_FLAGS "-DKIOCLIENT_AS_KDEMV")
#macro_add_compile_flags(kde-open -DKIOCLIENT_AS_KDEOPEN)
target_link_libraries(kdemv KF5::KIOCore KF5::KIOWidgets
        KF5::Solid   
        KF5::KDE4Support
        ${KDECLARATIVE_LIBRARIES} 
        KF5::I18n KF5::ConfigCore KF5::ConfigGui Qt5::DBus
        KF5::Solid
        ${KCore_LIBRARIES}
        ${Qt5Script_LIBRARIES})
install(TARGETS kdemv ${INSTALL_TARGETS_DEFAULT_ARGS})
#set(kdemv_SRCS kioclient.cpp )
#kde4_add_executable(kde-mv NOGUI ${kdemv_SRCS})
#set_property(TARGET kio-mv APPEND_STRING PROPERTY COMPILE_FLAGS "-DKIOCLIENT_AS_KDEMV")
#macro_add_compile_flags(kde-mv -DKIOCLIENT_AS_KDEMV)
#target_link_libraries(kde-mv KF5::KIOCore KF5::KIOWidgets)
#install(TARGETS kde-mv ${INSTALL_TARGETS_DEFAULT_ARGS})


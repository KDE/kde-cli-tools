add_executable(kreadconfig kreadconfig.cpp)
ecm_mark_nongui_executable(kreadconfig)

target_link_libraries(kreadconfig KF5::CoreAddons KF5::I18n KF5::ConfigCore)

install(TARGETS kreadconfig ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

kde4_add_executable(kwriteconfig kwriteconfig.cpp)
ecm_mark_nongui_executable(kwriteconfig)

target_link_libraries(kwriteconfig KF5::CoreAddons KF5::I18n KF5::ConfigCore)

install(TARGETS kwriteconfig ${INSTALL_TARGETS_DEFAULT_ARGS})

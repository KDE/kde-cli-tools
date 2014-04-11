function(add_kioclient_interface TARGET_NAME)
    add_executable(${TARGET_NAME} kioclient.cpp)
    string(TOUPPER "${TARGET_NAME}" UPPER_TARGET_NAME)
    target_compile_definitions(${TARGET_NAME} PRIVATE "-DKIOCLIENT_AS_${UPPER_TARGET_NAME}")
    target_link_libraries(${TARGET_NAME} Qt5::DBus KF5::CoreAddons KF5::KIOWidgets KF5::I18n)
    install(TARGETS ${TARGET_NAME} ${INSTALL_TARGETS_DEFAULT_ARGS})
endfunction()

#we compile every file with different definitions, so it will behave slightly different
add_kioclient_interface(kioclient)
add_kioclient_interface(kdecp)
add_kioclient_interface(kdemv)

#kioclient is not marked as nongui since download and openProperties do graphical things
ecm_mark_nongui_executable(kdemv)
ecm_mark_nongui_executable(kdecp)

add_executable(kde-open kioclient.cpp)
target_compile_definitions(kde-open PRIVATE "-DKIOCLIENT_AS_KDEOPEN")
target_link_libraries(kde-open Qt5::DBus KF5::CoreAddons KF5::KIOWidgets KF5::I18n)
install(TARGETS kde-open ${INSTALL_TARGETS_DEFAULT_ARGS})
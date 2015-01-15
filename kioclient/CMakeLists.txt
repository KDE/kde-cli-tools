
if (NOT TARGET KF5::KIOWidgets)
   set(KIOCORE_ONLY ON)
   add_definitions(-DKIOCORE_ONLY=1)
   set(kio_libs KF5::KIOCore)
else ()
   set(kio_libs KF5::KIOWidgets)
endif ()

function(add_kioclient_interface TARGET_NAME)
    add_executable(${TARGET_NAME} kioclient.cpp)
    string(TOUPPER "${TARGET_NAME}" UPPER_TARGET_NAME)
    target_compile_definitions(${TARGET_NAME} PRIVATE "-DKIOCLIENT_AS_${UPPER_TARGET_NAME}")
    target_compile_definitions(${TARGET_NAME} PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
    target_link_libraries(${TARGET_NAME} Qt5::DBus Qt5::Widgets KF5::CoreAddons ${kio_libs} KF5::I18n)
    install(TARGETS ${TARGET_NAME} ${INSTALL_TARGETS_DEFAULT_ARGS})
endfunction()

#we compile every file with different definitions, so it will behave slightly different
add_kioclient_interface(kioclient5)
add_kioclient_interface(kdecp5)
add_kioclient_interface(kdemv5)

#kioclient is not marked as nongui since download and openProperties do graphical things
ecm_mark_nongui_executable(kdemv5)
ecm_mark_nongui_executable(kdecp5)

if (NOT KIOCORE_ONLY)
add_executable(kde-open5 kioclient.cpp)
target_compile_definitions(kde-open5 PRIVATE "-DKIOCLIENT_AS_KDEOPEN")
target_compile_definitions(kde-open5 PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(kde-open5 Qt5::DBus KF5::CoreAddons KF5::KIOWidgets KF5::I18n)
install(TARGETS kde-open5 ${INSTALL_TARGETS_DEFAULT_ARGS})
endif()

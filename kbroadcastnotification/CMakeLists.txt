add_executable(kbroadcastnotification main.cpp)
target_compile_definitions(kbroadcastnotification PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(kbroadcastnotification Qt::DBus KF5::CoreAddons KF5::I18n)

install(TARGETS kbroadcastnotification ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

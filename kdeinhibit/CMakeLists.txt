add_executable(kde-inhibit main.cpp)
target_link_libraries(kde-inhibit Qt5::Core Qt5::DBus KF5::I18n KF5::CoreAddons)
install(TARGETS kde-inhibit ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

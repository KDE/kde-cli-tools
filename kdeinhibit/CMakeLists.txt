add_executable(kde-inhibit main.cpp)
target_link_libraries(kde-inhibit Qt::Core Qt::DBus KF6::I18n KF6::CoreAddons)
install(TARGETS kde-inhibit ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

if (ECM_VERSION VERSION_GREATER_EQUAL 5.101.0)
  install(FILES completions/kdeinhibit.zsh RENAME _kde-inhibit DESTINATION ${KDE_INSTALL_ZSHAUTOCOMPLETEDIR})
endif()

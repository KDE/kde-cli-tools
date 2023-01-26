add_executable(kstart kstart.cpp)
target_compile_definitions(kstart PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(kstart
    Qt::Gui
    KF5::I18n
    KF5::KIOGui
    KF5::Service
    KF5::WindowSystem)

install_compat_symlink(kstart)
install(TARGETS kstart DESTINATION ${KDE_INSTALL_FULL_BINDIR})

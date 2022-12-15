add_executable(kstart kstart.cpp)
target_compile_definitions(kstart PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(kstart
    Qt::Gui
    Qt::GuiPrivate
    KF6::I18n
    KF6::KIOGui
    KF6::Service
    KF6::WindowSystem)

install_compat_symlink(kstart)
install(TARGETS kstart DESTINATION ${KDE_INSTALL_FULL_BINDIR})

find_package(X11)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                        URL "http://www.x.org"
                        TYPE REQUIRED
                    )

add_executable(kstart kstart.cpp)
target_compile_definitions(kstart PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(kstart
    Qt::Widgets
    Qt::X11Extras
    KF5::I18n
    KF5::KIOGui
    KF5::Service
    KF5::WindowSystem
    ${X11_X11_LIB})

install_compat_symlink(kstart)
install(TARGETS kstart DESTINATION ${KDE_INSTALL_FULL_BINDIR})

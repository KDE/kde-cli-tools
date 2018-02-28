find_package(X11)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                        URL "http://www.x.org"
                        TYPE REQUIRED
                    )

add_executable(kstart5 kstart.cpp)
target_compile_definitions(kstart5 PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(kstart5
    Qt5::Widgets
    Qt5::X11Extras
    KF5::I18n
    KF5::Service
    KF5::WindowSystem
    ${X11_X11_LIB})

install(TARGETS kstart5 ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

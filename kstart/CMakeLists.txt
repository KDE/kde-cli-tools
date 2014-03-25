find_package(X11)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                        URL "http://www.x.org"
                        TYPE REQUIRED
                    )

add_executable(kstart kstart.cpp)
target_link_libraries(kstart Qt5::X11Extras KF5::KDE4Support ${X11_X11_LIB})

install(TARGETS kstart ${INSTALL_TARGETS_DEFAULT_ARGS})


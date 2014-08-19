add_executable(kdesu_executable kdesu.cpp sudlg.cpp)
target_compile_definitions(kdesu_executable PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

# in KDELibsDependencies.cmake installed by kdelibs there is a dependency to "kdesu"
# which is then recognized here as the target name for this executable
# so give the target here a different name and use the OUTPUT_NAME property to 
# give it the name kdesu
set_target_properties(kdesu_executable PROPERTIES OUTPUT_NAME kdesu)

target_link_libraries(kdesu_executable KF5::WidgetsAddons KF5::I18n KF5::ConfigCore KF5::WindowSystem KF5::Su)

install(TARGETS kdesu_executable DESTINATION ${LIBEXEC_INSTALL_DIR})






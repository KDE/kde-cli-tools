add_executable(kmimetypefinder5 kmimetypefinder.cpp)
target_compile_definitions(kmimetypefinder5 PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
ecm_mark_nongui_executable(kmimetypefinder5)

target_link_libraries(kmimetypefinder5 KF5::KDELibs4Support)

install(TARGETS kmimetypefinder5 ${INSTALL_TARGETS_DEFAULT_ARGS} )

add_executable(kmimetypefinder5 kmimetypefinder.cpp)
target_compile_definitions(kmimetypefinder5 PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
ecm_mark_nongui_executable(kmimetypefinder5)

target_link_libraries(kmimetypefinder5
   KF5::CoreAddons # KAboutData
   KF5::I18n       # i18n
   Qt5::Core
)

install(TARGETS kmimetypefinder5 ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

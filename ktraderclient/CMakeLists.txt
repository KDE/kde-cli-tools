add_executable(ktraderclient5 ktraderclient.cpp)
target_compile_definitions(ktraderclient5 PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
ecm_mark_nongui_executable(ktraderclient5)

target_link_libraries(ktraderclient5
   KF6::Service    # KService, traders...
   KF6::CoreAddons # KAboutData
   KF6::I18n       # i18n
   Qt::Core
)

install(TARGETS ktraderclient5 ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

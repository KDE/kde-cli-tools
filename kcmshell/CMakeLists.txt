
set(kcmshell_KDEINIT_SRCS main.cpp )

kf5_add_kdeinit_executable( kcmshell5 ${kcmshell_KDEINIT_SRCS})
target_compile_definitions(kdeinit_kcmshell5 PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
ecm_mark_nongui_executable(kcmshell5)

target_link_libraries(kdeinit_kcmshell5
   Qt5::DBus
   KF5::KCMUtils
   KF5::I18n
   KF5::WindowSystem
   KF5::Activities
)

install(TARGETS kdeinit_kcmshell5  ${INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS kcmshell5          ${INSTALL_TARGETS_DEFAULT_ARGS})

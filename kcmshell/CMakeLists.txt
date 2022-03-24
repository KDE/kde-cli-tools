
set(kcmshell_KDEINIT_SRCS main.cpp )

add_executable(kcmshell5 ${kcmshell_KDEINIT_SRCS})
target_compile_definitions(kcmshell5 PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
ecm_mark_nongui_executable(kcmshell5)

target_link_libraries(kcmshell5
   Qt::DBus
   KF5::KCMUtils
   KF5::I18n
   KF5::WindowSystem
   KF5::Activities
   PW::KWorkspace
)

install(TARGETS kcmshell5          ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

cmake_minimum_required(VERSION 3.16)

project(KDE-CLI-Tools)
set(PROJECT_VERSION "6.3.80")

set(QT_MIN_VERSION "6.7.0")
set(KF6_MIN_VERSION "6.14.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_DOC "Whether to build the documentation" ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)
include(FeatureSummary)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(ECMOptionalAddSubdirectory)
include(KDEClangFormat)
include(ECMDeprecationSettings)

include(KDEGitCommitHooks)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Widgets
    Svg
    DBus
)

find_package(Qt6Test ${QT_MIN_VERSION} CONFIG QUIET)
set_package_properties(Qt6Test PROPERTIES
         PURPOSE "Required for tests"
         TYPE OPTIONAL
         )
add_feature_info("Qt6Test" Qt6Test_FOUND "Required for building tests")
if (NOT Qt6Test_FOUND)
    set(BUILD_TESTING OFF CACHE BOOL "Build the testing tree.")
endif()

if (Qt6Gui_VERSION VERSION_GREATER_EQUAL "6.10.0")
    find_package(Qt6GuiPrivate ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
endif()

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    Config
    IconThemes
    I18n
    KCMUtils
    KIO
    Service
    WindowSystem
    Parts
    CoreAddons
    OPTIONAL_COMPONENTS
    Su
    WidgetsAddons
    QUIET
)

# Disables automatic conversions from QString (or char *) to QUrl.
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

option(WITH_X11 "Build kdesu and kstart with X11 support." ON)

ecm_set_disabled_deprecation_versions(QT 6.7.0 KF 6.4.0)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(BUILD_DOC)
    find_package(KF6DocTools ${KF6_MIN_VERSION})
    set_package_properties(KF6DocTools PROPERTIES
        DESCRIPTION "Tools to generate documentation"
        TYPE REQUIRED
    )
    add_subdirectory(doc)
    kdoctools_install(po)
endif()

function(install_compat_symlink executable_target)
    add_custom_command(TARGET ${executable_target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E create_symlink ${executable_target} ${executable_target}5)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${executable_target}5 DESTINATION ${KDE_INSTALL_FULL_BINDIR})
endfunction()

add_subdirectory(keditfiletype)
add_subdirectory(kmimetypefinder)
add_subdirectory(kioclient)
add_subdirectory(ksvgtopng)
add_subdirectory(kdeinhibit)
add_subdirectory(plasma-open-settings)
add_subdirectory(kinfo)
add_subdirectory(kstart)

if(KF6Su_FOUND AND KF6WidgetsAddons_FOUND)
    add_subdirectory(kdesu)
endif()

if(UNIX)
    add_subdirectory(kdeeject)
    add_subdirectory(kbroadcastnotification)
endif()

check_include_files(sys/wait.h HAVE_SYS_WAIT_H)

check_include_file("sys/prctl.h" HAVE_SYS_PRCTL_H)
check_symbol_exists(PR_SET_DUMPABLE "sys/prctl.h" HAVE_PR_SET_DUMPABLE)
check_include_file("sys/procctl.h" HAVE_SYS_PROCCTL_H)
check_symbol_exists(PROC_TRACE_CTL "sys/procctl.h" HAVE_PROC_TRACE_CTL)
if (HAVE_PR_SET_DUMPABLE OR HAVE_PROC_TRACE_CTL)
  set(CAN_DISABLE_PTRACE TRUE)
endif()
add_feature_info("prctl-dumpable"
                 CAN_DISABLE_PTRACE
                "Required for disallowing ptrace on kdesu process")

configure_file (config-kde-cli-tools.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kde-cli-tools.h )

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

ki18n_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

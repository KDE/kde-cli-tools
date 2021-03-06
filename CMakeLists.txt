cmake_minimum_required(VERSION 3.16)

project(KDE-CLI-Tools)
set(PROJECT_VERSION "5.22.80")

set(QT_MIN_VERSION "5.15.0")
set(KF5_MIN_VERSION "5.82")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
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

include(KDEGitCommitHooks)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Widgets
    Svg
    DBus
)

find_package(Qt5Test ${QT_MIN_VERSION} CONFIG QUIET)
set_package_properties(Qt5Test PROPERTIES
         PURPOSE "Required for tests"
         TYPE OPTIONAL
         )
add_feature_info("Qt5Test" Qt5Test_FOUND "Required for building tests")
if (NOT Qt5Test_FOUND)
    set(BUILD_TESTING OFF CACHE BOOL "Build the testing tree.")
endif()

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Config
    DocTools
    IconThemes
    I18n
    KCMUtils
    KIO
    Service
    WindowSystem
    Activities
    Declarative
    OPTIONAL_COMPONENTS
    Su
    QUIET
)
find_package(LibKWorkspace ${PROJECT_VERSION} REQUIRED)

# Disables automatic conversions from QString (or char *) to QUrl.
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

find_package(X11)
find_package(XCB COMPONENTS XCB)
if(X11_FOUND AND XCB_XCB_FOUND)
  set(HAVE_X11 1)
  find_package(Qt5X11Extras ${QT_MIN_VERSION} CONFIG)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(doc)

add_subdirectory(kcmshell)
add_subdirectory(keditfiletype)
add_subdirectory(kmimetypefinder)
add_subdirectory(ktraderclient)
add_subdirectory(kioclient)
add_subdirectory(ksvgtopng)
add_subdirectory(kdeinhibit)
add_subdirectory(plasma-open-settings)

if(Qt5X11Extras_FOUND)
    add_subdirectory(kstart)
endif()


if(KF5Su_FOUND)
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

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

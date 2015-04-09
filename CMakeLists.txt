project(KDE-CLI-Tools)
set(PROJECT_VERSION "5.2.95")

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.4.0")

find_package(ECM 1.3.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(ECMInstallIcons)
include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)
include(FeatureSummary)
include(CheckIncludeFiles)
include(ECMOptionalAddSubdirectory)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Widgets
    Svg
    DBus
)

find_package(Qt5X11Extras ${QT_MIN_VERSION} CONFIG)

find_package(Qt5Test ${QT_MIN_VERSION} CONFIG QUIET)
set_package_properties(Qt5Test PROPERTIES
         PURPOSE "Required for tests"
         TYPE OPTIONAL
         )
add_feature_info("Qt5Test" Qt5Test_FOUND "Required for building tests")
if (NOT Qt5Test_FOUND)
    set(BUILD_TESTING OFF CACHE BOOL "Build the testing tree.")
endif()

find_package(KF5 REQUIRED COMPONENTS
    Config
    IconThemes
    I18n
    KCMUtils
    Su
    KDELibs4Support
    WindowSystem
)

find_package(X11)
if(X11_FOUND AND XCB_XCB_FOUND)
  set(HAVE_X11 1)
endif()

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(doc)

add_subdirectory(kcmshell)
add_subdirectory(keditfiletype)
add_subdirectory(kmimetypefinder)
add_subdirectory(ktraderclient)
add_subdirectory(kioclient)
add_subdirectory(ksvgtopng)

if(Qt5X11Extras_FOUND)
    add_subdirectory(kstart)
endif()

if(UNIX)
    add_subdirectory(kdesu)
    add_subdirectory(kdeeject)
endif()

check_include_files(sys/wait.h HAVE_SYS_WAIT_H)
configure_file (config-kde-cli-tools.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kde-cli-tools.h )

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

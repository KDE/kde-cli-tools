project(KDE-CLI-Tools)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.2.0")

find_package(ECM 0.0.11 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMInstallIcons)
include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)
include(FeatureSummary)
include(CheckIncludeFiles)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Widgets
    Svg
    Test
)

find_package(KF5 CONFIG REQUIRED COMPONENTS
    Config
    Init
    KCMUtils
    Su
    KDE4Support
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

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

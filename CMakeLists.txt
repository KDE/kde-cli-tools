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

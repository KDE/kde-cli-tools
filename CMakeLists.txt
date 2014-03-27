add_subdirectory(kcmshell)
add_subdirectory(keditfiletype)
add_subdirectory(kmimetypefinder)
add_subdirectory(ktraderclient)
add_subdirectory(kioclient)

if(Qt5X11Extras_FOUND)
    add_subdirectory(kstart)
endif()

if(UNIX)
    add_subdirectory(kdeeject)
endif()

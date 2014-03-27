check_include_files(sys/wait.h HAVE_SYS_WAIT_H)

add_subdirectory( kdesu ) 
if (UNIX AND NOT APPLE)
    add_subdirectory( kdesud )
endif()

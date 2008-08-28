

add_subdirectory( kdesu ) 
if (UNIX AND NOT APPLE)
add_subdirectory( kdesud )
endif(UNIX AND NOT APPLE)

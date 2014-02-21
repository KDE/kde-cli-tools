set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KDE4_INCLUDES} )

########### filetypestest ###############

add_executable(filetypestest
 filetypestest.cpp
 ../mimetypedata.cpp
 ../mimetypewriter.cpp
 ../sharedmimeinfoversion.cpp
)
ecm_mark_as_test(filetypestest)
add_test(filetypestest filetypestest)
target_link_libraries(filetypestest ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBRARIES}
                      ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY})

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

########### filetypestest ###############

add_executable(filetypestest
 filetypestest.cpp
 ../mimetypedata.cpp
 ../mimetypewriter.cpp
 ../sharedmimeinfoversion.cpp
)
ecm_mark_as_test(filetypestest)
add_test(filetypestest filetypestest)
target_link_libraries(filetypestest KF5::KDE4Support KF5::KIOCore
                      Qt5::Core Qt5::Gui Qt5::Test)

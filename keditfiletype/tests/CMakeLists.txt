include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

########### filetypestest ###############

add_executable(filetypestest
 filetypestest.cpp
 ../mimetypedata.cpp
 ../mimetypewriter.cpp
)
ecm_mark_as_test(filetypestest)
add_test(NAME filetypestest COMMAND filetypestest)
target_link_libraries(filetypestest
   KF6::KIOCore
   KF6::Service
   KF6::Parts
   Qt::Core
   Qt::Gui
   Qt::Test
)

add_test(NAME kcm_smoketest COMMAND kcmshell6 --smoke-test kcm_filetypes)
set_tests_properties(kcm_smoketest PROPERTIES
    ENVIRONMENT_MODIFICATION QT_PLUGIN_PATH=path_list_prepend:${CMAKE_BINARY_DIR}/bin
)

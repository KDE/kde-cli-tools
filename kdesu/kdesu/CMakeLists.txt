



########### next target ###############

set(kdesu_SRCS kdesu.cpp sudlg.cpp )


kde4_add_executable(kdesu_executable ${kdesu_SRCS})
# in KDELibsDependencies.cmake installed by kdelibs there is a dependency to "kdesu"
# which is then recognized here as the target name for this executable
# so give the target here a different name and use the OUTPUT_NAME property to 
# give it the name kdesu
set_target_properties(kdesu_executable PROPERTIES OUTPUT_NAME kdesu)

target_link_libraries(kdesu_executable  ${KDE4_KDEUI_LIBS} ${KDE4_KDESU_LIBS} )

install(TARGETS kdesu_executable DESTINATION ${LIBEXEC_INSTALL_DIR})






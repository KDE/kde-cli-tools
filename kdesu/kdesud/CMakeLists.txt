check_function_exists(getpeereid HAVE_GETPEEREID) # openbsd style
check_function_exists(getpeereucred HAVE_GETPEERUCRED) # solaris style

configure_file (config-kdesud.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdesud.h )


set(kdesud_SRCS
   kdesud.cpp
   repo.cpp
   lexer.cpp
   handler.cpp
   secure.cpp )

add_executable(kdesud ${kdesud_SRCS})
ecm_mark_nongui_executable(kdesud)

target_link_libraries(kdesud KF5::Su KF5::KDE4Support ${X11_LIBRARIES})

if(KDE4_ENABLE_FPIE)
    macro_add_compile_flags(kdesud ${KDE4_CXX_FPIE_FLAGS})
    macro_add_link_flags(kdesud ${KDE4_PIE_LDFLAGS})
endif(KDE4_ENABLE_FPIE)

########### install files ###############

install(TARGETS kdesud DESTINATION ${LIBEXEC_INSTALL_DIR})
install(CODE "
    set(KDESUD_PATH \"\$ENV{DESTDIR}${LIBEXEC_INSTALL_DIR}/kdesud\")
    EXECUTE_PROCESS(COMMAND sh -c \"chgrp nogroup '\${KDESUD_PATH}' && chmod g+s '\${KDESUD_PATH}'\")
")


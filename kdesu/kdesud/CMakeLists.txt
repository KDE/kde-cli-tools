
set(kdesud_SRCS
   kdesud.cpp
   repo.cpp
   lexer.cpp
   handler.cpp
   secure.cpp )




kde4_add_executable(kdesud NOGUI ${kdesud_SRCS})

target_link_libraries(kdesud ${KDE4_KDESU_LIBS} ${X11_LIBRARIES})

if(KDE4_ENABLE_FPIE)
    macro_add_compile_flags(kdesud ${KDE4_CXX_FPIE_FLAGS})
    macro_add_link_flags(kdesud ${KDE4_PIE_LDFLAGS})
endif(KDE4_ENABLE_FPIE)

MESSAGE(STATUS "Create macro to display info at the end of install")
install(TARGETS kdesud PERMISSIONS SETGID OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############






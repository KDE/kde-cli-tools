
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





#original Makefile.am contents follow:

### Makefile.am for kdesud
#
#INCLUDES = $(all_includes)
#
#KDE_CXXFLAGS = $(KDE_USE_FPIE)
#
#bin_PROGRAMS = kdesud
#kdesud_SOURCES = kdesud.cpp repo.cpp lexer.cpp handler.cpp secure.cpp
#kdesud_LDFLAGS = $(KDE_USE_PIE) $(all_libraries) $(KDE_RPATH)
#kdesud_LDADD = $(LIB_KDECORE) -lkdesu $(LIBSOCKET)
#noinst_HEADERS = repo.h handler.h lexer.h secure.h
#
### kdesud needs to be suid or sgid something
#install-data-local:
#	@echo "********************************************************"
#	@echo ""
#	@echo "For security reasons, kdesud is installed setgid nogroup."
#	@echo "Kdesud is the daemon that implements the password caching."
#	@echo ""
#	@echo "You should NOT use the password caching feature if kdesud is"
#	@echo "not installed setgid nogroup."
#	@echo ""
#	@echo "********************************************************"
#
#install-exec-hook:
#	@(chown root:@nogroup@ $(DESTDIR)$(bindir)/kdesud && chmod 2755  $(DESTDIR)$(bindir)/kdesud) \
#	|| echo "Error: Could not install kdesud as setgid nogroup!!\n" \
#	"The password caching feature is disabled."
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/kdesud.pot

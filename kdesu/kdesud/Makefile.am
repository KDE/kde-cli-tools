## Makefile.am for kdesud

INCLUDES = -I$(top_srcdir)/kdesu/common

bin_PROGRAMS = kdesud
kdesud_SOURCES = kdesud.cpp repo.cpp lexer.cpp handler.cpp secure.cpp 
kdesud_LDADD = $(top_srcdir)/kdesu/common/libkdesu.a
noinst_HEADERS = repo.h kdesud.h handler.h lexer.h secure.h

## kdesud needs to be suid or sgid something
install-data-local:
	@echo ""
	@echo "For security reasons, kdesud is intalled setgid nogroup."
	@echo "Kdesud is the daemon that implements the password caching."
	@echo ""
	@echo "You should NOT use the password caching feature if kdesud is"
	@echo "not installed setgid nogroup."
	@echo ""

install-exec-local:
	@(chown root.@nogroup@ $(bindir)/kdesud && chmod 2755  $(bindir)/kdesud) \
	|| echo "Error: Could not install kdesud as setgid nogroup!!\n" \
	"The password caching feature is disabled."

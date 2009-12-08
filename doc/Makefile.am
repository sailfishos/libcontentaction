all-local: doxygen

DOXYCFG = $(srcdir)/doxy.cfg

doxygen: html/index.html

html/index.html: ../src/*.cpp ../src/*.h
	@if test x$(srcdir) = x. ; then \
		echo srcdir=$(srcdir) $(DOXYGEN) $(DOXYCFG); \
		srcdir=$(srcdir) $(DOXYGEN) $(DOXYCFG); \
	else \
		echo This is a VPATH build, so not building doxygen; \
	fi

EXTRA_DIST = $(DOXYCFG) html

distclean-local:
	rm -f doxygen.log

maintainer-clean-local:
	rm -rf html

.PHONY: doxygen


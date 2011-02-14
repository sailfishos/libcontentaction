if BUILD_DOCS
all-local: doxygen
endif

DOXYCFG = $(srcdir)/doxy.cfg

doxygen: html/index.html

html/index.html: $(srcdir)/../src/*.cpp $(srcdir)/../src/*.h $(DOXYCFG)
	srcdir=$(srcdir) $(DOXYGEN) $(DOXYCFG);

EXTRA_DIST = $(DOXYCFG)

html_docsdir = $(datadir)/doc/libcontentaction-doc/html
html_docs_DATA = $(wildcard $(builddir)/html/*)

clean-local:
	rm -f doxygen.log
	rm -rf html

.PHONY: doxygen


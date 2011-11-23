ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src t doc data

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = \
	contentaction-0.1.pc
noinst_DATA = \
	contentaction-0.1-uninstalled.pc

coverage:covoptioncheck
	$(MAKE) -C src coverage

.PHONY: coverage

include $(top_srcdir)/am/covoptioncheck.am

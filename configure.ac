AC_PREREQ([2.61])
AC_INIT([libcontentaction], [0.0.4~unreleased], [marius.vollmer@nokia.com], libcontentaction)

AC_CONFIG_SRCDIR([Makefile.am])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_MACRO_DIR([m4])

AC_LANG([C++])
AC_PROG_CXX
CXXFLAGS="$CXXFLAGS -Wall"
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

# check for libraries
PKG_CHECK_MODULES([QtTest], [QtTest])
PKG_CHECK_MODULES([QtCore], [QtCore])
PKG_CHECK_MODULES([QtXml], [QtXml])
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.12.0])
PKG_CHECK_MODULES([TRACKER], [tracker-client-0.7])
PKG_CHECK_MODULES([DUI], [Dui])
PKG_CHECK_MODULES([GCONF], [gconf-2.0])
# a temporary fix: since maemo-services-dev 0.3.4, generated
# interfaces include DuiApplication, it includes QApplication, and
# Dui.pc doesn't state Requires: QtGui
PKG_CHECK_MODULES([QtGui], [QtGui])

AC_CHECK_FILE([/usr/include/gallery/service/galleryinterface.h], [],
              AC_MSG_ERROR([please install maemo-services-dev]))

# tools for documentation
AX_FEATURE_DISABLEABLE([doc], [BUILD_DOCS], [disable building of documentation])
AX_DISABLE_FEATURE_ON_PROG([doxygen], [BUILD_DOCS], [DOXYGEN], [doxygen])
AM_CONDITIONAL([BUILD_DOCS], [test "$missing_deps_BUILD_DOCS" = ""])

# coverage tools
AX_FEATURE_DISABLEABLE([coverage], [COVERAGE], [disable unittests' coverage support])
AX_DISABLE_FEATURE_ON_PROG([GCov coverage recording tool], [COVERAGE], [GCOV], [gcov])
AX_DISABLE_FEATURE_ON_PROG([LCov coverage visualization tool], [COVERAGE], [LCOV], [lcov])
AM_CONDITIONAL([COVERAGE], [test "$missing_deps_COVERAGE" = ""])

AC_CONFIG_FILES([
        Makefile
        contentaction-0.1.pc
        contentaction-0.1-uninstalled.pc
        src/Makefile
        t/Makefile
        t/env.sh
        t/env.py
        doc/Makefile
        data/Makefile
])

AC_OUTPUT

# $Id: Makefile.am,v 1.3 2006/11/27 09:59:31 mloskot Exp $
#
# Author: Mateusz Loskot <mateusz@loskot.net>
#
INCLUDES = -I$(top_srcdir)/src/core
AM_CXXFLAGS = -fPIC
AM_CPPFLAGS = @MYSQL_CFLAGS@

if MYSQL
lib_LTLIBRARIES	= \
	lib@SOCI_MYSQL_LIBRARY_NAME@.la

lib@SOCI_MYSQL_LIBRARY_NAME@_la_LDFLAGS = \
	-release @LIBTOOL_RELEASE@ -no-undefined @MYSQL_LDFLAGS@

lib@SOCI_MYSQL_LIBRARY_NAME@_la_LIBADD = \
	@top_builddir@/src/core/lib@SOCI_CORE_LIBRARY_NAME@.la

lib@SOCI_MYSQL_LIBRARY_NAME@_la_SOURCES = \
	blob.cpp \
	common.cpp \
	factory.cpp \
	row-id.cpp \
	session.cpp \
	standard-into-type.cpp \
	standard-use-type.cpp \
	statement.cpp \
	vector-into-type.cpp \
	vector-use-type.cpp

soci_mysqldir = $(includedir)/soci/mysql

soci_mysql_HEADERS = \
	common.h \
	soci-mysql.h
endif

EXTRA_DIST = Makefile.basic

MAINTAINERCLEANLOCAL = Makefile.in

maintainer-clean-local:
	-test -z "$(MAINTAINERCLEANLOCAL)" || rm -rf $(MAINTAINERCLEANLOCAL)

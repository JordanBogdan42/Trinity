# $Id: Makefile.am 1779 2018-11-29 08:10:47Z psizun $
AM_CPPFLAGS = -I@top_srcdir@/src -I@top_builddir@/src \
 $(BOOST_CPPFLAGS) $(MFM_CFLAGS) $(QT_CPPFLAGS)
AM_CXXFLAGS = $(QT_CXXFLAGS)

bin_PROGRAMS = cobo-frame-merge@DEBUG_SUFFIX@
cobo_frame_merge@DEBUG_SUFFIX@_SOURCES        = cobo-frame-merge.cpp CoBoFrameMerger.h CoBoFrameMerger.cpp
cobo_frame_merge@DEBUG_SUFFIX@_LDFLAGS        = $(BOOST_LDFLAGS) $(MFM_LDFLAGS) $(QT_LDFLAGS)
cobo_frame_merge@DEBUG_SUFFIX@_LDADD          = $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_SYSTEM_LIB) $(MFM_LIBS) $(QT_LIBS)

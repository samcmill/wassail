SUBDIRS = . checks data python

AM_CPPFLAGS = $(CODE_COVERAGE_CPPFLAGS)
AM_CXXFLAGS = $(CODE_COVERAGE_CXXFLAGS)

AM_CPPFLAGS += -I$(top_srcdir)/src/3rdparty
AM_CPPFLAGS += -I$(top_srcdir)/test
LDADD = $(top_builddir)/src/libwassail.la

TEST_LOG_DRIVER = $(SHELL) $(top_srcdir)/catch-driver

EXTRA_DIST =
check_PROGRAMS =

noinst_HEADERS  = $(top_srcdir)/test/3rdparty/catch/catch.hpp
noinst_HEADERS += $(top_srcdir)/test/3rdparty/catch/catch_reporter_automake.hpp
noinst_HEADERS += $(top_srcdir)/include/wassail/json/json.hpp
noinst_HEADERS += $(top_srcdir)/include/wassail/wassail.hpp

# Test cases

check_PROGRAMS += format.test
format_test_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/include/wassail -I$(top_srcdir)/src -I$(top_srcdir)/src/3rdparty
format_test_SOURCES = tostring.h test_format.cpp

check_PROGRAMS += internal.test
internal_test_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/include/wassail -I$(top_srcdir)/src -I$(top_srcdir)/src/3rdparty
internal_test_SOURCES = tostring.h test_internal.cpp

check_PROGRAMS += result.test
result_test_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src/3rdparty
result_test_SOURCES = tostring.h test_result.cpp

check_PROGRAMS += version.test
version_test_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src -I$(top_srcdir)/src/3rdparty
version_test_SOURCES = tostring.h test_version.cpp

# Declare tests
TESTS = $(check_PROGRAMS)

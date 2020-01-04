AM_CPPFLAGS = $(CODE_COVERAGE_CPPFLAGS)
AM_CXXFLAGS = $(CODE_COVERAGE_CXXFLAGS)

EXTRA_DIST =
noinst_LTLIBRARIES = libwassail_common.la

noinst_HEADERS = $(top_srcdir)/include/wassail/json/json.hpp
noinst_HEADERS += $(top_srcdir)/src/3rdparty/spdlog/spdlog.h
noinst_HEADERS += $(top_srcdir)/include/wassail/result.hpp

libwassail_common_la_CPPFLAGS = -I$(top_srcdir)/include \
                                -I$(top_srcdir)/include/wassail \
                                -I$(top_srcdir)/src \
                                -I$(top_srcdir)/src/3rdparty
libwassail_common_la_SOURCES = $(top_srcdir)/include/wassail/common.hpp

libwassail_common_la_SOURCES += initialize.cpp
libwassail_common_la_SOURCES += logger.cpp
libwassail_common_la_SOURCES += parallel.cpp
libwassail_common_la_SOURCES += result.cpp
libwassail_common_la_SOURCES += version.cpp

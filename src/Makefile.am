AM_CPPFLAGS = $(CODE_COVERAGE_CPPFLAGS)
AM_CXXFLAGS = $(CODE_COVERAGE_CXXFLAGS)

# Order is significant
SUBDIRS = 3rdparty checks common data . python samples

lib_LTLIBRARIES = libwassail.la

libwassail_la_LDFLAGS = -version-info @version_info@
libwassail_la_SOURCES =
# Dummy C++ source to cause C++ linking.
nodist_EXTRA_libwassail_la_SOURCES = dummy.cpp

libwassail_la_LIBADD = checks/libwassail_checks.la \
                       common/libwassail_common.la \
                       data/libwassail_data.la \
                       3rdparty/libwassail_3rdparty.la

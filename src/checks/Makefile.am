AM_CPPFLAGS = $(CODE_COVERAGE_CPPFLAGS)
AM_CXXFLAGS = $(CODE_COVERAGE_CXXFLAGS)

EXTRA_DIST =
AUTOMAKE_OPTIONS = subdir-objects

noinst_LTLIBRARIES = libwassail_checks.la

libwassail_checks_la_CPPFLAGS = -I$(top_srcdir)/include \
                                -I$(top_srcdir)/include/wassail \
                                -I$(top_srcdir)/src \
                                -I$(top_srcdir)/src/3rdparty

libwassail_checks_la_SOURCES = $(top_srcdir)/include/wassail/checks/check.hpp \
    $(top_srcdir)/include/wassail/checks/compare.hpp \
    $(top_srcdir)/include/wassail/common.hpp \
    $(top_srcdir)/include/wassail/result.hpp \
    $(top_srcdir)/src/internal.hpp

noinst_HEADERS = $(top_srcdir)/include/wassail/checks/check.hpp \
                 $(top_srcdir)/include/wassail/checks/compare.hpp \
                 $(top_srcdir)/include/wassail/result.hpp
nobase_dist_pkgdata_DATA = skeleton_check/skeleton.hpp \
                           skeleton_check/skeleton.cpp

libwassail_checks_la_SOURCES += cpu/core_count.cpp \
    $(top_srcdir)/include/wassail/checks/cpu/core_count.hpp \
    $(top_srcdir)/include/wassail/data/sysconf.hpp \
    $(top_srcdir)/include/wassail/data/sysctl.hpp \
    $(top_srcdir)/include/wassail/data/sysinfo.hpp

libwassail_checks_la_SOURCES += disk/amount_free.cpp disk/percent_free.cpp \
    $(top_srcdir)/include/wassail/checks/disk/amount_free.hpp \
    $(top_srcdir)/include/wassail/checks/disk/percent_free.hpp \
    $(top_srcdir)/include/wassail/data/getfsstat.hpp \
    $(top_srcdir)/include/wassail/data/getmntent.hpp

libwassail_checks_la_SOURCES += memory/physical_size.cpp \
    $(top_srcdir)/include/wassail/checks/memory/physical_size.hpp \
    $(top_srcdir)/include/wassail/data/sysconf.hpp \
    $(top_srcdir)/include/wassail/data/sysctl.hpp \
    $(top_srcdir)/include/wassail/data/sysinfo.hpp

libwassail_checks_la_SOURCES += misc/environment.cpp \
    $(top_srcdir)/include/wassail/checks/misc/environment.hpp \
    $(top_srcdir)/include/wassail/data/environment.hpp

libwassail_checks_la_SOURCES += misc/load_average.cpp \
    $(top_srcdir)/include/wassail/checks/misc/load_average.hpp \
    $(top_srcdir)/include/wassail/data/getloadavg.hpp

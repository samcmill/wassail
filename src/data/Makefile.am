AM_CPPFLAGS = $(CODE_COVERAGE_CPPFLAGS)
AM_CXXFLAGS = $(CODE_COVERAGE_CXXFLAGS) -DLIBEXECDIR='"$(libexecdir)/$(PACKAGE)"'

schemadir = $(pkgdatadir)/schemas
dist_schema_DATA =

nobase_dist_pkgdata_DATA = skeleton_data/skeleton.hpp \
                           skeleton_data/skeleton.cpp \
                           skeleton_data/skeleton.json
noinst_LTLIBRARIES = libwassail_data.la

noinst_HEADERS = $(top_srcdir)/include/wassail/json/json.hpp
noinst_HEADERS += $(top_srcdir)/src/internal.hpp

libwassail_data_la_CPPFLAGS = -I$(top_srcdir)/include \
                              -I$(top_srcdir)/include/wassail \
                              -I$(top_srcdir)/src \
                              -I$(top_srcdir)/src/3rdparty
libwassail_data_la_SOURCES = data.cpp \
    $(top_srcdir)/include/wassail/data/data.hpp

libwassail_data_la_SOURCES += environment.cpp \
    $(top_srcdir)/include/wassail/data/environment.hpp
dist_schema_DATA += environment.json

libwassail_data_la_SOURCES += getcpuid.cpp \
    $(top_srcdir)/include/wassail/data/getcpuid.hpp
dist_schema_DATA += getcpuid.json

libwassail_data_la_SOURCES += getfsstat.cpp \
    $(top_srcdir)/include/wassail/data/getfsstat.hpp
dist_schema_DATA += getfsstat.json

libwassail_data_la_SOURCES += getloadavg.cpp \
    $(top_srcdir)/include/wassail/data/getloadavg.hpp
dist_schema_DATA += getloadavg.json

libwassail_data_la_SOURCES += getmntent.cpp \
    $(top_srcdir)/include/wassail/data/getmntent.hpp
dist_schema_DATA += getmntent.json

libwassail_data_la_SOURCES += getrlimit.cpp \
    $(top_srcdir)/include/wassail/data/getrlimit.hpp
dist_schema_DATA += getrlimit.json

libwassail_data_la_SOURCES += nvml.cpp \
    $(top_srcdir)/include/wassail/data/nvml.hpp
dist_schema_DATA += nvml.json

libwassail_data_la_SOURCES += pciaccess.cpp \
    $(top_srcdir)/include/wassail/data/pciaccess.hpp
dist_schema_DATA += pciaccess.json

libwassail_data_la_SOURCES += pciutils.cpp \
    $(top_srcdir)/include/wassail/data/pciutils.hpp
dist_schema_DATA += pciutils.json

libwassail_data_la_SOURCES += ps.cpp \
    $(top_srcdir)/include/wassail/data/ps.hpp
dist_schema_DATA += ps.json

libwassail_data_la_SOURCES += remote_shell_command.cpp \
    $(top_srcdir)/include/wassail/data/remote_shell_command.hpp
dist_schema_DATA += remote_shell_command.json

libwassail_data_la_SOURCES += shell_command.cpp \
    $(top_srcdir)/include/wassail/data/shell_command.hpp
dist_schema_DATA += shell_command.json

libwassail_data_la_SOURCES += stat.cpp \
    $(top_srcdir)/include/wassail/data/stat.hpp
dist_schema_DATA += stat.json

libwassail_data_la_SOURCES += stream.cpp \
    $(top_srcdir)/include/wassail/data/stream.hpp
dist_schema_DATA += stream.json

libwassail_data_la_SOURCES += sysconf.cpp \
    $(top_srcdir)/include/wassail/data/sysconf.hpp
dist_schema_DATA += sysconf.json

libwassail_data_la_SOURCES += sysctl.cpp \
    $(top_srcdir)/include/wassail/data/sysctl.hpp
dist_schema_DATA += sysctl.json

libwassail_data_la_SOURCES += sysinfo.cpp \
    $(top_srcdir)/include/wassail/data/sysinfo.hpp
dist_schema_DATA += sysinfo.json

libwassail_data_la_SOURCES += umad.cpp \
    $(top_srcdir)/include/wassail/data/umad.hpp
dist_schema_DATA += umad.json

libwassail_data_la_SOURCES += uname.cpp \
    $(top_srcdir)/include/wassail/data/uname.hpp
dist_schema_DATA += uname.json

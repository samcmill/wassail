AM_CPPFLAGS = $(CODE_COVERAGE_CPPFLAGS)
AM_CXXFLAGS = $(CODE_COVERAGE_CXXFLAGS) -DLIBEXECDIR='"$(abs_top_srcdir)/test/data"'

AM_CPPFLAGS += -I$(top_srcdir)/include
AM_CPPFLAGS += -I$(top_srcdir)/src
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

### Test cases

check_PROGRAMS += environment.test
environment_test_SOURCES = test_environment.cpp

check_PROGRAMS += getcpuid.test
getcpuid_test_SOURCES = test_getcpuid.cpp

check_PROGRAMS += getloadavg.test
getloadavg_test_SOURCES = test_getloadavg.cpp

check_PROGRAMS += getfsstat.test
getfsstat_test_SOURCES = test_getfsstat.cpp

check_PROGRAMS += getmntent.test
getmntent_test_SOURCES = test_getmntent.cpp

check_PROGRAMS += getrlimit.test
getrlimit_test_SOURCES = test_getrlimit.cpp

check_PROGRAMS += pciaccess.test
pciaccess_test_SOURCES = test_pciaccess.cpp

check_PROGRAMS += pciutils.test
pciutils_test_SOURCES = test_pciutils.cpp

check_PROGRAMS += ps.test
ps_test_SOURCES = test_ps.cpp

check_PROGRAMS += remote_shell_command.test
remote_shell_command_test_SOURCES = test_remote_shell_command.cpp

check_PROGRAMS += shell_command.test
shell_command_test_SOURCES = test_shell_command.cpp

check_PROGRAMS += stat.test
stat_test_SOURCES = test_stat.cpp

check_PROGRAMS += stream.test
stream_test_SOURCES = test_stream.cpp
EXTRA_DIST += stream

check_PROGRAMS += sysconf.test
sysconf_test_SOURCES = test_sysconf.cpp

check_PROGRAMS += sysctl.test
sysctl_test_SOURCES = test_sysctl.cpp

check_PROGRAMS += sysinfo.test
sysinfo_test_SOURCES = test_sysinfo.cpp

check_PROGRAMS += uname.test
uname_test_SOURCES = test_uname.cpp

check_PROGRAMS += umad.test
umad_test_SOURCES = test_umad.cpp

# Declare tests
TESTS = $(check_PROGRAMS)

AM_CPPFLAGS = $(CODE_COVERAGE_CPPFLAGS)
AM_CXXFLAGS = $(CODE_COVERAGE_CXXFLAGS)

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

# Test cases

check_PROGRAMS += compare.test
compare_test_SOURCES = $(top_srcdir)/test/tostring.h test_compare.cpp

check_PROGRAMS += rules_engine.test
rules_engine_test_SOURCES = $(top_srcdir)/test/tostring.h test_rules_engine.cpp

check_PROGRAMS += cpu_core_count.test
cpu_core_count_test_SOURCES = $(top_srcdir)/test/tostring.h \
                              test_cpu_core_count.cpp

check_PROGRAMS += disk_amount_free.test
disk_amount_free_test_SOURCES = $(top_srcdir)/test/tostring.h \
                                 test_disk_amount_free.cpp

check_PROGRAMS += disk_percent_free.test
disk_percent_free_test_SOURCES = $(top_srcdir)/test/tostring.h \
                                 test_disk_percent_free.cpp

check_PROGRAMS += file_permissions.test
file_permissions_test_SOURCES = $(top_srcdir)/test/tostring.h \
                                test_file_permissions.cpp

check_PROGRAMS += memory_physical_size.test
memory_physical_size_test_SOURCES = $(top_srcdir)/test/tostring.h \
                                    test_memory_physical_size.cpp

check_PROGRAMS += misc_environment.test
misc_environment_test_SOURCES = $(top_srcdir)/test/tostring.h \
                                test_misc_environment.cpp

check_PROGRAMS += misc_load_average.test
misc_load_average_test_SOURCES = $(top_srcdir)/test/tostring.h \
                                    test_misc_load_average.cpp

# Declare tests
TESTS = $(check_PROGRAMS)

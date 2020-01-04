dnl Package version
m4_define([wassail_version_major], [0])
m4_define([wassail_version_minor], [1])
m4_define([wassail_version_micro], [0])

dnl Library interface version
m4_define([lt_cur], [0])
m4_define([lt_rev], [0])
m4_define([lt_age], [0])

m4_define([wassail_version],
          [wassail_version_major.wassail_version_minor.wassail_version_micro])

AC_INIT([wassail], [wassail_version], [scott.andrew.mcmillan@gmail.com], [],
        [https://github.com/samcmill/wassail])

AC_DEFINE_UNQUOTED([VERSION_MAJOR], [wassail_version_major],
                   [Major version number])
AC_DEFINE_UNQUOTED([VERSION_MINOR], [wassail_version_minor],
                   [Minor version number])
AC_DEFINE_UNQUOTED([VERSION_MICRO], [wassail_version_micro],
                   [Micro version number])
AC_SUBST([VERSION_MAJOR], [wassail_version_major])
AC_SUBST([VERSION_MINOR], [wassail_version_minor])
AC_SUBST([VERSION_MICRO], [wassail_version_micro])
AC_SUBST([version_info], [lt_cur:lt_rev:lt_age])

dnl Internal logger
AC_DEFINE(LOGGER, "wassail_logger", [Logger name])
AC_DEFINE(SPDLOG_FMT_EXTERNAL, [], [Use external fmtlib with spdlog])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_MACRO_DIRS([m4])
m4_include([m4/ax_check_compile_flag.m4])
m4_include([m4/ax_cxx_compile_stdcxx.m4])
m4_include([m4/ax_cxx_compile_stdcxx_14.m4])
m4_include([m4/ax_cxx_compile_stdcxx_17.m4])
m4_include([m4/ax_prog_doxygen.m4])
m4_include([m4/python.m4])

AM_SILENT_RULES([yes])

AX_CODE_COVERAGE

AC_C_INLINE
AC_PROG_CXX
AM_PROG_AR

dnl Libtool
LT_INIT
AC_REQUIRE_AUX_FILE([catch-driver])

dnl Doxygen
DX_HTML_FEATURE(ON)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN([$PACKAGE_NAME], [Doxyfile], [doxygen])

dnl Python
AC_ARG_ENABLE([python],
    [AS_HELP_STRING([--disable-python], [disable Python binding])],
    [],
    [enable_python=yes])
if test "x${enable_python}" = "xyes"; then
    PC_INIT([2.7])
    PC_PYTHON_CHECK_HEADERS(,[AC_MSG_WARN(could not find Python headers)])
    PC_PYTHON_CHECK_EXTENSION_SUFFIX
    PC_PYTHON_CHECK_SITE_DIR
    AM_CONDITIONAL(WITH_PYTHON, true)
    AX_CHECK_COMPILE_FLAG([-fvisibility=hidden],
                          [PYBIND11_EXTRA_CXXFLAGS="-fvisibility=hidden"])
    AC_SUBST(pythondir)
    AS_IF([test "x$PYTHON_EXTENSION_SUFFIX" = "x"],
          AC_SUBST(PYTHON_EXTENSION_SUFFIX, [.so])
          AC_SUBST(PYTHON_EXTENSION_SUFFIX))
    AC_SUBST(PYTHON_INCLUDES)
    AC_SUBST(PYBIND11_EXTRA_CXXFLAGS)
else
    AM_CONDITIONAL(WITH_PYTHON, false)
fi

dnl clang-format
AC_ARG_WITH([clang-format],
    [AS_HELP_STRING([--with-clang-format@<:@=ARG@:>@],
        [Enable re-formatting source code with clang-format. @<:@ARG=clang-format@:>@ ])],
        [case "${withval}" in
          yes ) prog_clang_format="clang-format" ;;
          no )  prog_clang_format="no" ;;
          * )   prog_clang_format="${withval}" ;;
          esac],
        [prog_clang_format="clang-format"])
AS_IF([test "x$prog_clang_format" != "xno"],
    [AC_PATH_PROG([CLANG_FORMAT], [$prog_clang_format], [])])
AM_CONDITIONAL([HAVE_CLANG_FORMAT], [test "x$CLANG_FORMAT" != "x"])

AC_LANG([C++])

dnl Check necessary types
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UID_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_CHECK_TYPES([fixpt_t, ptrdiff_t])
AC_CHECK_TYPES([struct xsw_usage, struct loadavg],,,
               [[#include <sys/sysctl.h>]])

dnl Prefer C++ 2017, but fallback to C++ 2014
AX_CXX_COMPILE_STDCXX([17], [noext], [optional])
if test "$HAVE_CXX17" -ne 1; then
  AX_CXX_COMPILE_STDCXX([14], [noext], [mandatory])
fi

AX_CXXFLAGS_WARN_ALL

dnl pthreads
AX_PTHREAD([LIBS="$PTHREAD_LIBS $LIBS"
            CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
            CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"])

dnl Check required features
AC_HEADER_STDC
AC_CHECK_HEADERS([cassert],,AC_MSG_ERROR(cassert is required))
AC_CHECK_HEADERS([fcntl.h],,AC_MSG_ERROR(fcntl.h is required))
AC_CHECK_HEADERS([limits.h],,AC_MSG_ERROR(limits.h is required))
AC_CHECK_HEADERS([locale.h],,AC_MSG_ERROR(locale.h is required))
AC_CHECK_HEADERS([syslog.h],,AC_MSG_ERROR(syslog.h is required))
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_STRERROR_R
AC_FUNC_STRTOD
AC_CHECK_FUNCS([clock_gettime getenv gethostname getpagesize localeconv],,
               AC_MSG_ERROR(Missing necessary function))
AC_CHECK_FUNCS([localtime_r memmove memset strerror strstr strtoul strtoull],,
               AC_MSG_ERROR(Missing necessary function))

dnl Require a working std::regex (gcc prior to 4.9 does not work)
AC_CHECK_HEADERS(regex,,AC_MSG_ERROR(regex header missing))
AC_MSG_CHECKING(for working std::regex)
AC_RUN_IFELSE([AC_LANG_PROGRAM([[#include <regex>]],
                               [[std::regex re("[0-9]"); std::regex_search("test123",re);
               ]])],
              [
                AC_MSG_RESULT([yes])
              ],
              [
                AC_MSG_ERROR([no functioning std::regex found])
              ]
)

dnl Check optional features (if missing, some functionality will be disabled)
AC_CHECK_HEADERS([infiniband/umad.h])
AC_CHECK_HEADERS([pci/pci.h])
AC_CHECK_HEADERS([pciaccess.h])
AC_CHECK_HEADERS([libssh/libsshpp.hpp])
AC_CHECK_HEADERS([execution])
AC_CHECK_HEADERS([dlfcn.h mntent.h poll.h signal.h sys/mount.h sys/param.h])
AC_CHECK_HEADERS([sys/resource.h sys/stat.h sys/statvfs.h sys/sysctl.h])
AC_CHECK_HEADERS([sys/sysinfo.h sys/ucred.h sys/utsname.h sys/wait.h unistd.h])
AC_FUNC_FORK
AC_CHECK_FUNCS([dup2 execvp getfsstat getloadavg getmntent getrlimit killpg])
AC_CHECK_FUNCS([pipe poll setpgid stat statvfs sysconf sysctlbyname])
AC_CHECK_FUNCS([sysinfo uname waitpid])
AC_CHECK_MEMBERS([struct stat.st_atime, struct stat.st_ctime,
                  struct stat.st_mtime, struct stat.st_atimespec,
                  struct stat.st_ctimespec, struct stat.st_mtimespec],
                  [], [], [[#include <sys/stat.h>]])
AC_SEARCH_LIBS([dlopen], [dl dld],
               AC_DEFINE([HAVE_DLOPEN], 1,
                         [Define to 1 if you have the `dlopen' function.])
               wassail_cv_func_dlopen="yes")
AC_SEARCH_LIBS([ssh_init], [ssh],
               AC_DEFINE([HAVE_SSH_INIT], 1,
                         [Define to 1 if you have the `ssh_init' function.])
               wassail_cv_func_ssh_init="yes")

dnl cpuid instruction
AC_CHECK_HEADERS(cpuid.h)
AC_MSG_CHECKING([for _get_cpuid intrinsic])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <cpuid.h>]],
                  [[unsigned int eax, ebx, ecx, edx;
                    __get_cpuid(0, &eax, &ebx, &ecx, &edx);
		  ]])],
		 [
		   AC_MSG_RESULT([yes])
		   AC_DEFINE_UNQUOTED([HAVE_GETCPUID], 1,
                     [Define to 1 if you have the `__get_cpuid' intrinsic.])
                   wassail_cv__get_cpuid="yes"
		 ],
		 [
		   AC_MSG_RESULT([no])
                   wassail_cv__get_cpuid="no"
		 ]
)

dnl environ extern
AC_MSG_CHECKING([for extern environ])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <unistd.h>]],
                  [[extern char ** environ; char **x = environ;]])],
                  [
                    AC_MSG_RESULT([yes])
                    AC_DEFINE_UNQUOTED([HAVE_EXTERN_ENVIRON], 1,
                      [Define to 1 if `extern environ' is available.])
                    wassail_cv_extern_environ="yes"
                  ],
                  [
                    AC_MSG_RESULT([no])
                    wassail_cv_extern_environ="no"
                  ]
)

dnl check for libdispatch - optional and can be disabled
AC_ARG_ENABLE([libdispatch],
    [AS_HELP_STRING([--disable-libdispatch], [disable libdispatch])],
    [],
    [enable_libdispatch=yes])
if test "x${enable_libdispatch}" = "xyes"; then
    AC_CHECK_HEADERS([dispatch/dispatch.h])
    AC_SEARCH_LIBS([dispatch_apply_f], [dispatch],
                   AC_DEFINE([HAVE_DISPATCH_APPLY_F], 1,
                             [Define to 1 if you have the `dispatch_apply_f' function.])
                   wassail_cv_func_dispatch_apply_f="yes")
    AS_IF([test "x$ac_cv_header_dispatch_dispatch_h" = "xyes" &&
           test "x$ac_cv_search_dispatch_apply_f" != "xno"],
          [AM_CONDITIONAL(WITH_LIBDISPATCH, true)
           AC_DEFINE(HAVE_LIBDISPATCH, 1,[Define to 1 if you have the `libdispatch' library.])],
          [AM_CONDITIONAL(WITH_LIBDISPATCH, false)])

else
    AM_CONDITIONAL(WITH_LIBDISPATCH, false)
fi

dnl check for TBB - optional and can be disabled
AC_ARG_ENABLE([tbb],
    [AS_HELP_STRING([--disable-tbb], [disable threading building blocks])],
    [],
    [enable_tbb=yes])
if test "x${enable_tbb}" = "xyes"; then
    AC_CHECK_HEADERS([tbb/parallel_for_each.h])
    AC_CHECK_LIB([tbb], [main],
                 AC_DEFINE([HAVE_TBB], 1,
                           [Define to 1 if you have the `tbb' library.])
                 wassail_cv_lib_tbb="yes")
    AS_IF([test "x$ac_cv_header_tbb_parallel_for_each_h" = "xyes" &&
           test "x$wassail_cv_lib_tbb" = "xyes"],
          [AM_CONDITIONAL(WITH_TBB, true)
           AC_DEFINE(HAVE_TBB, 1,[Define to 1 if you have the `tbb' library.])
           LIBS="$LIBS -ltbb"],
          [AM_CONDITIONAL(WITH_TBB, false)])
else
    AM_CONDITIONAL(WITH_TBB, false)
fi

######

dnl environment data source
AS_IF([test "x$ac_cv_header_unistd_h" = "xyes" &&
       test "x$wassail_cv_extern_environ" = "xyes"],
      [AM_CONDITIONAL(WITH_DATA_ENVIRONMENT, true)
       AC_DEFINE(WITH_DATA_ENVIRONMENT,,[environment data source])],
      [AM_CONDITIONAL(WITH_DATA_ENVIRONMENT, false)
       AC_MSG_WARN([disabling environment data source])])

dnl getcpuid data source
AS_IF([test "x$ac_cv_header_cpuid_h" = "xyes" &&
       test "x$wassail_cv__get_cpuid" = "xyes"],
      [AM_CONDITIONAL(WITH_DATA_GETCPUID, true)
       AC_DEFINE(WITH_DATA_GETCPUID,,[getcpuid data source])],
      [AM_CONDITIONAL(WITH_DATA_GETCPUID, false)
       AC_MSG_WARN([disabling getcpuid data source])])

dnl getfsstat data source
AS_IF([test "x$ac_cv_header_sys_mount_h" = "xyes" &&
       test "x$ac_cv_header_sys_param_h" = "xyes" &&
       test "x$ac_cv_header_sys_ucred_h" = "xyes" &&
       test "x$ac_cv_func_getfsstat" = "xyes"],
      [AM_CONDITIONAL(WITH_DATA_GETFSSTAT, true)
       AC_DEFINE(WITH_DATA_GETFSSTAT,,[getfsstat data source])],
      [AM_CONDITIONAL(WITH_DATA_GETFSSTAT, false)
       AC_MSG_WARN([disabling getfsstat data source])])

dnl getloadavg data source
AS_IF([test "x$ac_cv_func_getloadavg" = "xyes"],
      [AM_CONDITIONAL(WITH_DATA_GETLOADAVG, true)
       AC_DEFINE(WITH_DATA_GETLOADAVG,,[getloadavg data source])],
      [AM_CONDITIONAL(WITH_DATA_GETLOADAVG, false)
       AC_MSG_WARN([disabling getloadavg data source])])

dnl getmntent data source
AS_IF([test "x$ac_cv_header_mntent_h" = "xyes" &&
       test "x$ac_cv_header_sys_statvfs_h" = "xyes" &&
       test "x$ac_cv_func_getmntent" = "xyes" &&
       test "x$ac_cv_func_statvfs" = "xyes"],
      [AM_CONDITIONAL(WITH_DATA_GETMNTENT, true)
       AC_DEFINE(WITH_DATA_GETMNTENT,,[getmntent data source])],
      [AM_CONDITIONAL(WITH_DATA_GETMNTENT, false)
       AC_MSG_WARN([disabling getmntent data source])])

dnl getrlimit data source
AS_IF([test "x$ac_cv_header_sys_resource_h" = "xyes" &&
       test "x$ac_cv_func_getrlimit" = "xyes"],
      [AM_CONDITIONAL(WITH_DATA_GETRLIMIT, true)
       AC_DEFINE(WITH_DATA_GETRLIMIT,,[getrlimit data source])],
      [AM_CONDITIONAL(WITH_DATA_GETRLIMIT, false)
       AC_MSG_WARN([disabling getrlimit data source])])

dnl pciaccess data source
AS_IF([test "x$ac_cv_header_pciaccess_h" = "xyes" &&
       test "x$wassail_cv_func_dlopen" = "xyes"],
      [AM_CONDITIONAL(WITH_DATA_PCIACCESS, true)
       AC_DEFINE(WITH_DATA_PCIACCESS,,[pciaccess data source])],
      [AM_CONDITIONAL(WITH_DATA_PCIACCESS, false)
       AC_MSG_WARN([disabling pciaccess data source])])

dnl pciutils data source
AS_IF([test "x$ac_cv_header_pci_pci_h" = "xyes" &&
       test "x$wassail_cv_func_dlopen" = "xyes"],
      [AM_CONDITIONAL(WITH_DATA_PCIUTILS, true)
       AC_DEFINE(WITH_DATA_PCIUTILS,,[pciutils data source])],
      [AM_CONDITIONAL(WITH_DATA_PCIUTILS, false)
       AC_MSG_WARN([disabling pciutils data source])])

dnl remote_shell_command data source
AS_IF([test "x$ac_cv_header_libssh_libsshpp_hpp" = "xyes" &&
       test "x$wassail_cv_func_ssh_init" = "xyes"],
      [AM_CONDITIONAL(WITH_DATA_REMOTE_SHELL_COMMAND, true)
       AC_DEFINE(WITH_DATA_REMOTE_SHELL_COMMAND,,
                 [remote_shell_command data source])],
      [AM_CONDITIONAL(WITH_DATA_REMOTE_SHELL_COMMAND, false)
       AC_MSG_WARN([disabling remote_shell_command data source])])

dnl shell_command data source
AS_IF([test "x$ac_cv_header_poll_h" = "xyes" &&
       test "x$ac_cv_header_signal_h" = "xyes" &&
       test "x$ac_cv_header_sys_wait_h" = "xyes" &&
       test "x$ac_cv_header_unistd_h" = "xyes" &&
       test "x$ac_cv_func_dup2" = "xyes" &&
       test "x$ac_cv_func_execvp" = "xyes" &&
       test "x$ac_cv_func_fork" = "xyes" &&
       test "x$ac_cv_func_killpg" = "xyes" &&
       test "x$ac_cv_func_pipe" = "xyes" &&
       test "x$ac_cv_func_poll" = "xyes" &&
       test "x$ac_cv_func_setpgid" = "xyes" &&
       test "x$ac_cv_func_waitpid" = "xyes"],
      [AM_CONDITIONAL(WITH_DATA_SHELL_COMMAND, true)
       AC_DEFINE(WITH_DATA_SHELL_COMMAND,,[shell_command data source])],
      [AM_CONDITIONAL(WITH_DATA_SHELL_COMMAND, false)
       AC_MSG_WARN([disabling shell_command data source])])

dnl stat data source
AS_IF([test "x$ac_cv_header_sys_stat_h" = "xyes" &&
       test "x$ac_cv_func_stat" = "xyes"],
      [AM_CONDITIONAL(WITH_DATA_STAT, true)
       AC_DEFINE(WITH_DATA_STAT,,[stat data source])],
      [AM_CONDITIONAL(WITH_DATA_STAT, false)
       AC_MSG_WARN([disabling stat data source])])

dnl sysconf data source
AS_IF([test "x$ac_cv_header_unistd_h" = "xyes" &&
       test "x$ac_cv_func_sysconf" = "xyes"],
      [AM_CONDITIONAL(WITH_DATA_SYSCONF, true)
       AC_DEFINE(WITH_DATA_SYSCONF,,[sysconf data source])],
      [AM_CONDITIONAL(WITH_DATA_SYSCONF, false)
       AC_MSG_WARN([disabling sysconf data source])])

dnl sysctl data source
AS_IF([test "x$ac_cv_header_sys_sysctl_h" = "xyes" &&
       test "x$ac_cv_func_sysctlbyname" = "xyes"],
      [AM_CONDITIONAL(WITH_DATA_SYSCTL, true)
       AC_DEFINE(WITH_DATA_SYSCTL,,[sysctl data source])],
      [AM_CONDITIONAL(WITH_DATA_SYSCTL, false)
       AC_MSG_WARN([disabling sysctl data source])])

dnl sysinfo data source
AS_IF([test "x$ac_cv_header_sys_sysinfo_h" = "xyes" &&
       test "x$ac_cv_func_sysinfo" = "xyes"],
      [AM_CONDITIONAL(WITH_DATA_SYSINFO, true)
       AC_DEFINE(WITH_DATA_SYSINFO,,[sysinfo data source])],
      [AM_CONDITIONAL(WITH_DATA_SYSINFO, false)
       AC_MSG_WARN([disabling sysinfo data source])])

dnl umad data source
AS_IF([test "x$ac_cv_header_infiniband_umad_h" = "xyes" &&
       test "x$wassail_cv_func_dlopen" = "xyes"],
      [AM_CONDITIONAL(WITH_DATA_UMAD, true)
       AC_DEFINE(WITH_DATA_UMAD,,[umad data source])],
      [AM_CONDITIONAL(WITH_DATA_UMAD, false)
       AC_MSG_WARN([disabling umad data source])])

dnl uname data source
AS_IF([test "x$ac_cv_header_sys_utsname_h" = "xyes" &&
       test "x$ac_cv_func_uname" = "xyes"],
      [AM_CONDITIONAL(WITH_DATA_UNAME, true)
       AC_DEFINE(WITH_DATA_UNAME,,[uname data source])],
      [AM_CONDITIONAL(WITH_DATA_UNAME, false)
       AC_MSG_WARN([disabling uname data source])])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_SUBDIRS([src/3rdparty/stream])

AC_CONFIG_FILES([
 Makefile
 Doxyfile
 wassail.spec
 include/wassail/Makefile
 src/Makefile
 src/3rdparty/Makefile
 src/3rdparty/fmt/Makefile
 src/checks/Makefile
 src/common/Makefile
 src/data/Makefile
 src/python/Makefile
 src/samples/Makefile
 src/samples/c++/Makefile
 test/Makefile
 test/checks/Makefile
 test/data/Makefile
 test/python/Makefile
])

AC_OUTPUT

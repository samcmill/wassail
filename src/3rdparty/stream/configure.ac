AC_INIT([stream], [5.10])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([foreign])
AM_SILENT_RULES([yes])

dnl Force -O3 instead of -O2
CFLAGS="-g -O3"
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB

# Intel compiler specific
AX_CHECK_COMPILE_FLAG([-qopt-streaming-stores always],
                      [AC_SUBST(AM_CFLAGS, "-qopt-streaming-stores always")])
AX_CHECK_COMPILE_FLAG([-opt-streaming-stores always],
                      [AC_SUBST(AM_CFLAGS, "-opt-streaming-stores always")])

AC_OPENMP
AC_SUBST(AM_CFLAGS, "$OPENMP_CFLAGS")

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

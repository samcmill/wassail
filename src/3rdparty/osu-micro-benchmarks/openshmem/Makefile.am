AUTOMAKE_OPTIONS = subdir-objects

openshmemdir = $(pkglibexecdir)/openshmem

openshmem_PROGRAMS = osu_oshm_get osu_oshm_put osu_oshm_put_mr \
					 osu_oshm_atomics osu_oshm_barrier osu_oshm_broadcast \
					 osu_oshm_collect osu_oshm_fcollect osu_oshm_reduce \
					 osu_oshm_get_nb osu_oshm_put_nb osu_oshm_put_overlap \
					 osu_oshm_get_mr_nb osu_oshm_put_mr_nb 

AM_CFLAGS = -I${top_srcdir}/util

if EMBEDDED_BUILD
     AM_CPPFLAGS = -I$(top_builddir)/../src/include \
                  -I${top_srcdir}/util \
                  -I${top_srcdir}/../src/include
endif

osu_oshm_get_SOURCES = osu_oshm_get.c
osu_oshm_get_LDADD = $(top_builddir)/util/libutil.la
osu_oshm_put_SOURCES = osu_oshm_put.c
osu_oshm_put_LDADD = $(top_builddir)/util/libutil.la
osu_oshm_put_mr_SOURCES = osu_oshm_put_mr.c
osu_oshm_put_mr_LDADD = $(top_builddir)/util/libutil.la
osu_oshm_atomics_SOURCES = osu_oshm_atomics.c
osu_oshm_atomics_LDADD = $(top_builddir)/util/libutil.la
osu_oshm_barrier_SOURCES = osu_oshm_barrier.c
osu_oshm_barrier_LDADD = $(top_builddir)/util/libutil.la
osu_oshm_broadcast_SOURCES = osu_oshm_broadcast.c
osu_oshm_broadcast_LDADD = $(top_builddir)/util/libutil.la
osu_oshm_collect_SOURCES = osu_oshm_collect.c
osu_oshm_collect_LDADD = $(top_builddir)/util/libutil.la
osu_oshm_fcollect_SOURCES = osu_oshm_fcollect.c
osu_oshm_fcollect_LDADD = $(top_builddir)/util/libutil.la
osu_oshm_reduce_SOURCES = osu_oshm_reduce.c
osu_oshm_reduce_LDADD = $(top_builddir)/util/libutil.la
osu_oshm_get_nb_SOURCES = osu_oshm_get_nb.c
osu_oshm_get_nb_LDADD = $(top_builddir)/util/libutil.la
osu_oshm_put_nb_SOURCES = osu_oshm_put_nb.c
osu_oshm_put_nb_LDADD = $(top_builddir)/util/libutil.la
osu_oshm_put_overlap_SOURCES = osu_oshm_put_overlap.c
osu_oshm_put_overlap_LDADD = $(top_builddir)/util/libutil.la
osu_oshm_get_mr_nb_SOURCES = osu_oshm_get_mr_nb.c
osu_oshm_get_mr_nb_LDADD = $(top_builddir)/util/libutil.la
osu_oshm_put_mr_nb_SOURCES = osu_oshm_put_mr_nb.c
osu_oshm_put_mr_nb_LDADD = $(top_builddir)/util/libutil.la

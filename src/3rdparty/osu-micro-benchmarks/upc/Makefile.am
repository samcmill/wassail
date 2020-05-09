AUTOMAKE_OPTIONS = subdir-objects

upcdir = $(pkglibexecdir)/upc
upc_PROGRAMS = osu_upc_memget osu_upc_memput osu_upc_all_barrier \
	       osu_upc_all_broadcast osu_upc_all_exchange \
	       osu_upc_all_gather_all osu_upc_all_gather osu_upc_all_reduce \
	       osu_upc_all_scatter

AM_CFLAGS = -I${top_srcdir}/util

if EMBEDDED_BUILD
     AM_CPPFLAGS = -I$(top_builddir)/../src/include \
                  -I${top_srcdir}/util \
                  -I${top_srcdir}/../src/include
endif

osu_upc_all_barrier_SOURCES = osu_upc_all_barrier.c
osu_upc_all_barrier_LDADD = $(top_builddir)/util/libutil.la
osu_upc_all_broadcast_SOURCES = osu_upc_all_broadcast.c
osu_upc_all_broadcast_LDADD = $(top_builddir)/util/libutil.la
osu_upc_all_exchange_SOURCES = osu_upc_all_exchange.c
osu_upc_all_exchange_LDADD = $(top_builddir)/util/libutil.la
osu_upc_all_gather_SOURCES = osu_upc_all_gather.c
osu_upc_all_gather_LDADD = $(top_builddir)/util/libutil.la
osu_upc_all_gather_all_SOURCES = osu_upc_all_gather_all.c
osu_upc_all_gather_all_LDADD = $(top_builddir)/util/libutil.la
osu_upc_all_reduce_SOURCES = osu_upc_all_reduce.c
osu_upc_all_reduce_LDADD = $(top_builddir)/util/libutil.la
osu_upc_all_scatter_SOURCES = osu_upc_all_scatter.c
osu_upc_all_scatter_LDADD = $(top_builddir)/util/libutil.la

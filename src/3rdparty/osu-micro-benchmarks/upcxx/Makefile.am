AUTOMAKE_OPTIONS = subdir-objects

upcxxdir = $(pkglibexecdir)/upcxx
upcxx_PROGRAMS = osu_upcxx_allgather osu_upcxx_alltoall osu_upcxx_bcast \
	       osu_upcxx_gather osu_upcxx_reduce osu_upcxx_scatter \
	       osu_upcxx_async_copy_get osu_upcxx_async_copy_put

AM_CPPFLAGS = -I${top_srcdir}/util

if EMBEDDED_BUILD
     AM_CPPFLAGS += -I$(top_builddir)/../src/include \
                  -I${top_srcdir}/util \
                  -I${top_srcdir}/../src/include
endif

osu_upcxx_allgather_SOURCES = osu_upcxx_allgather.cpp
osu_upcxx_allgather_LDADD = $(top_builddir)/util/libutil.la
osu_upcxx_alltoall_SOURCES = osu_upcxx_alltoall.cpp
osu_upcxx_alltoall_LDADD = $(top_builddir)/util/libutil.la
osu_upcxx_bcast_SOURCES = osu_upcxx_bcast.cpp
osu_upcxx_bcast_LDADD = $(top_builddir)/util/libutil.la
osu_upcxx_gather_SOURCES = osu_upcxx_gather.cpp
osu_upcxx_gather_LDADD = $(top_builddir)/util/libutil.la
osu_upcxx_reduce_SOURCES = osu_upcxx_reduce.cpp
osu_upcxx_reduce_LDADD = $(top_builddir)/util/libutil.la
osu_upcxx_scatter_SOURCES = osu_upcxx_scatter.cpp
osu_upcxx_scatter_LDADD = $(top_builddir)/util/libutil.la
osu_upcxx_async_copy_get_SOURCES = osu_upcxx_async_copy_get.cpp
osu_upcxx_async_copy_get_LDADD = $(top_builddir)/util/libutil.la
osu_upcxx_async_copy_put_SOURCES = osu_upcxx_async_copy_put.cpp
osu_upcxx_async_copy_put_LDADD = $(top_builddir)/util/libutil.la

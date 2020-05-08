AUTOMAKE_OPTIONS = subdir-objects

NVCC = nvcc
NVCFLAGS = -cuda -maxrregcount 32 -ccbin $(CXX)
SUFFIXES = .cu .cpp
.cu.cpp:
	$(NVCC) $(NVCFLAGS) $(INCLUDES) $(CPPFLAGS) --output-file $@ $<

collectivedir = $(pkglibexecdir)/mpi/collective
collective_PROGRAMS = osu_alltoallv osu_allgatherv osu_scatterv osu_gatherv osu_reduce_scatter osu_barrier osu_reduce osu_allreduce osu_alltoall osu_bcast osu_gather osu_allgather osu_scatter osu_iallgather osu_ibcast  osu_ialltoall osu_ibarrier osu_igather osu_iscatter osu_iscatterv osu_igatherv osu_iallgatherv osu_ialltoallv osu_ialltoallw osu_ireduce osu_iallreduce

AM_CFLAGS = -I${top_srcdir}/util

if CUDA_KERNELS
if BUILD_USE_PGI
AM_CXXFLAGS = --nvcchost --no_preincludes
endif
endif

osu_allgatherv_SOURCES = osu_allgatherv.c
osu_allgatherv_LDADD = $(top_builddir)/util/libutil.la
osu_scatterv_SOURCES = osu_scatterv.c
osu_scatterv_LDADD = $(top_builddir)/util/libutil.la
osu_gather_SOURCES = osu_gather.c
osu_gather_LDADD = $(top_builddir)/util/libutil.la
osu_gatherv_SOURCES = osu_gatherv.c
osu_gatherv_LDADD = $(top_builddir)/util/libutil.la
osu_reduce_scatter_SOURCES = osu_reduce_scatter.c
osu_reduce_scatter_LDADD = $(top_builddir)/util/libutil.la
osu_barrier_SOURCES = osu_barrier.c
osu_barrier_LDADD = $(top_builddir)/util/libutil.la
osu_reduce_SOURCES = osu_reduce.c
osu_reduce_LDADD = $(top_builddir)/util/libutil.la
osu_allreduce_SOURCES = osu_allreduce.c
osu_allreduce_LDADD = $(top_builddir)/util/libutil.la
osu_bcast_SOURCES = osu_bcast.c
osu_bcast_LDADD = $(top_builddir)/util/libutil.la
osu_alltoall_SOURCES = osu_alltoall.c
osu_alltoall_LDADD = $(top_builddir)/util/libutil.la
osu_alltoallv_SOURCES = osu_alltoallv.c
osu_alltoallv_LDADD = $(top_builddir)/util/libutil.la
osu_ialltoall_SOURCES = osu_ialltoall.c
osu_ialltoall_LDADD = $(top_builddir)/util/libutil.la
osu_ialltoallv_SOURCES = osu_ialltoallv.c
osu_ialltoallv_LDADD = $(top_builddir)/util/libutil.la
osu_ialltoallw_SOURCES = osu_ialltoallw.c
osu_ialltoallw_LDADD = $(top_builddir)/util/libutil.la
osu_ibarrier_SOURCES = osu_ibarrier.c
osu_ibarrier_LDADD = $(top_builddir)/util/libutil.la
osu_ibcast_SOURCES = osu_ibcast.c
osu_ibcast_LDADD = $(top_builddir)/util/libutil.la
osu_igather_SOURCES = osu_igather.c
osu_igather_LDADD = $(top_builddir)/util/libutil.la
osu_igatherv_SOURCES = osu_igatherv.c
osu_igatherv_LDADD = $(top_builddir)/util/libutil.la
osu_allgather_SOURCES = osu_allgather.c
osu_allgather_LDADD = $(top_builddir)/util/libutil.la
osu_iallgather_SOURCES = osu_iallgather.c
osu_iallgather_LDADD = $(top_builddir)/util/libutil.la
osu_iallgatherv_SOURCES = osu_iallgatherv.c
osu_iallgatherv_LDADD = $(top_builddir)/util/libutil.la
osu_scatter_SOURCES = osu_scatter.c
osu_scatter_LDADD = $(top_builddir)/util/libutil.la
osu_iscatter_SOURCES = osu_iscatter.c
osu_iscatter_LDADD = $(top_builddir)/util/libutil.la
osu_iscatterv_SOURCES = osu_iscatterv.c
osu_iscatterv_LDADD = $(top_builddir)/util/libutil.la
osu_ireduce_SOURCES = osu_ireduce.c
osu_ireduce_LDADD = $(top_builddir)/util/libutil.la
osu_iallreduce_SOURCES = osu_iallreduce.c
osu_iallreduce_LDADD = $(top_builddir)/util/libutil.la

if EMBEDDED_BUILD
    AM_LDFLAGS =
    AM_CPPFLAGS = -I$(top_builddir)/../src/include \
          -I${top_srcdir}/util \
		  -I${top_srcdir}/../src/include
if BUILD_PROFILING_LIB
    AM_LDFLAGS += $(top_builddir)/../lib/lib@PMPILIBNAME@.la
endif
    AM_LDFLAGS += $(top_builddir)/../lib/lib@MPILIBNAME@.la
endif

if OPENACC
    AM_CFLAGS += -acc -ta=tesla:nordc
    AM_CXXFLAGS = -acc -ta=tesla:nordc
endif

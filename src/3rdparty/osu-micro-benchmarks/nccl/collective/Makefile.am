AUTOMAKE_OPTIONS = subdir-objects

NVCC = nvcc
NVCFLAGS = -cuda -maxrregcount 32 -ccbin $(CXX) $(NVCCFLAGS)
SUFFIXES = .cu .cpp
.cu.cpp:
	$(NVCC) $(NVCFLAGS) $(INCLUDES) $(CPPFLAGS) --output-file $@.ii $<
	mv $@.ii $@

ncclcollectivedir = $(pkglibexecdir)/nccl/collective
ncclcollective_PROGRAMS = osu_nccl_allgather osu_nccl_allreduce osu_nccl_bcast osu_nccl_reduce osu_nccl_reduce_scatter

AM_CFLAGS = -I${top_srcdir}/util

if CUDA_KERNELS
if BUILD_USE_PGI
AM_CXXFLAGS = --nvcchost --no_preincludes
endif
endif

osu_nccl_allgather_SOURCES = osu_nccl_allgather.c
osu_nccl_allgather_LDADD = $(top_builddir)/util/libutil.la
osu_nccl_allreduce_SOURCES = osu_nccl_allreduce.c
osu_nccl_allreduce_LDADD = $(top_builddir)/util/libutil.la
osu_nccl_bcast_SOURCES = osu_nccl_bcast.c
osu_nccl_bcast_LDADD = $(top_builddir)/util/libutil.la
osu_nccl_reduce_SOURCES = osu_nccl_reduce.c
osu_nccl_reduce_LDADD = $(top_builddir)/util/libutil.la
osu_nccl_reduce_scatter_SOURCES = osu_nccl_reduce_scatter.c
osu_nccl_reduce_scatter_LDADD = $(top_builddir)/util/libutil.la

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

AUTOMAKE_OPTIONS = subdir-objects

NVCC = nvcc
NVCFLAGS = -cuda -maxrregcount 32 -ccbin $(CXX) $(NVCCFLAGS)
SUFFIXES = .cu .cpp
.cu.cpp:
	$(NVCC) $(NVCFLAGS) $(INCLUDES) $(CPPFLAGS) --output-file $@.ii $<
	mv $@.ii $@

pt2ptdir = $(pkglibexecdir)/mpi/pt2pt
pt2pt_PROGRAMS = osu_bibw osu_bw osu_latency osu_mbw_mr osu_multi_lat

AM_CFLAGS = -I${top_srcdir}/util

if CUDA_KERNELS
if BUILD_USE_PGI
AM_CXXFLAGS = --nvcchost --no_preincludes
endif
endif

osu_bw_SOURCES = osu_bw.c
osu_bw_LDADD = $(top_builddir)/util/libutil.la
osu_bibw_SOURCES = osu_bibw.c
osu_bibw_LDADD = $(top_builddir)/util/libutil.la
osu_latency_SOURCES = osu_latency.c
osu_latency_LDADD = $(top_builddir)/util/libutil.la
osu_mbw_mr_SOURCES = osu_mbw_mr.c
osu_mbw_mr_LDADD = $(top_builddir)/util/libutil.la
osu_multi_lat_SOURCES = osu_multi_lat.c
osu_multi_lat_LDADD = $(top_builddir)/util/libutil.la
osu_latency_mt_SOURCES = osu_latency_mt.c
osu_latency_mt_LDADD = $(top_builddir)/util/libutil.la
osu_latency_mp_SOURCES = osu_latency_mp.c
osu_latency_mp_LDADD = $(top_builddir)/util/libutil.la

if MPI2_LIBRARY
    pt2pt_PROGRAMS += osu_latency_mt osu_latency_mp
endif

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


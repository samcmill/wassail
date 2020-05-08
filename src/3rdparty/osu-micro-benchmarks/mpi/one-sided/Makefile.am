AUTOMAKE_OPTIONS = subdir-objects

NVCC = nvcc
NVCFLAGS = -cuda -maxrregcount 32 -ccbin $(CXX)
SUFFIXES = .cu .cpp
.cu.cpp:
	$(NVCC) $(NVCFLAGS) $(INCLUDES) $(CPPFLAGS) --output-file $@ $<

one_sideddir = $(pkglibexecdir)/mpi/one-sided
one_sided_PROGRAMS = osu_acc_latency osu_get_bw osu_get_latency osu_put_bibw osu_put_bw osu_put_latency

if MPI3_LIBRARY
    one_sided_PROGRAMS += osu_get_acc_latency osu_fop_latency osu_cas_latency
endif

AM_CFLAGS = -I${top_srcdir}/util

if CUDA_KERNELS
if BUILD_USE_PGI
AM_CXXFLAGS = --nvcchost --no_preincludes
endif
endif

osu_put_latency_SOURCES = osu_put_latency.c
osu_put_latency_LDADD = $(top_builddir)/util/libutil.la
osu_put_bw_SOURCES = osu_put_bw.c
osu_put_bw_LDADD = $(top_builddir)/util/libutil.la
osu_put_bibw_SOURCES = osu_put_bibw.c
osu_put_bibw_LDADD = $(top_builddir)/util/libutil.la
osu_get_latency_SOURCES = osu_get_latency.c
osu_get_latency_LDADD = $(top_builddir)/util/libutil.la
osu_get_bw_SOURCES = osu_get_bw.c
osu_get_bw_LDADD = $(top_builddir)/util/libutil.la
osu_acc_latency_SOURCES = osu_acc_latency.c
osu_acc_latency_LDADD = $(top_builddir)/util/libutil.la
osu_fop_latency_SOURCES = osu_fop_latency.c
osu_fop_latency_LDADD = $(top_builddir)/util/libutil.la
osu_cas_latency_SOURCES = osu_cas_latency.c
osu_cas_latency_LDADD = $(top_builddir)/util/libutil.la
osu_get_acc_latency_SOURCES = osu_get_acc_latency.c
osu_get_acc_latency_LDADD = $(top_builddir)/util/libutil.la

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

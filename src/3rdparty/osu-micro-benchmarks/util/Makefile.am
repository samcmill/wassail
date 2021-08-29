NVCC = nvcc
NVCFLAGS = -cuda -maxrregcount 32 -ccbin $(CXX)
SUFFIXES = .cu .cpp
.cu.cpp:
	$(NVCC) $(NVCFLAGS) $(INCLUDES) $(CPPFLAGS) --output-file $@ $<

noinst_LTLIBRARIES = libutil.la

libutil_la_SOURCES = osu_util.c osu_util.h

if MPI
libutil_la_SOURCES += osu_util_mpi.c osu_util_mpi.h
endif

if NCCL
libutil_la_SOURCES += osu_util_nccl.c osu_util_nccl.h
endif

if OSHM
libutil_la_SOURCES += osu_util_pgas.c osu_util_pgas.h
endif

if UPC
libutil_la_SOURCES += osu_util_pgas.c osu_util_pgas.h
endif

if UPCXX
libutil_la_SOURCES += osu_util_pgas.c osu_util_pgas.h
endif

if CUDA_KERNELS
libutil_la_SOURCES += kernel.cu
CLEANFILES = kernel.cpp
if BUILD_USE_PGI
AM_CXXFLAGS = --nvcchost --no_preincludes
endif
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
    AM_CFLAGS = -acc -ta=tesla:nordc
    AM_CXXFLAGS = -acc -ta=tesla:nordc
endif

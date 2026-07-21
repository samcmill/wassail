if WITH_PYTHON
python_LTLIBRARIES = wassail.la
wassail_la_CPPFLAGS = -I$(top_srcdir)/include \
                      -I$(top_srcdir)/src/3rdparty \
                      $(PYTHON_CPPFLAGS)
wassail_la_CXXFLAGS = $(PYBIND11_EXTRA_CXXFLAGS) \
                      -DWASSAIL_LIBEXECDIR='"$(libexecdir)/$(PACKAGE)"'
wassail_la_LDFLAGS = -avoid-version -module -shared $(PYTHON_LIBS)
wassail_la_LIBADD = $(top_builddir)/src/libwassail.la
wassail_la_SOURCES = check.cpp data.cpp enum.cpp pybind.cpp result.cpp
else
EXTRA_DIST = check.cpp data.cpp enum.cpp pybind.cpp result.cpp
endif

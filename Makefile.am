include $(top_srcdir)/aminclude_static.am

CODE_COVERAGE_IGNORE_PATTERN = "$(abs_top_srcdir)/src/3rdparty/*"
CODE_COVERAGE_IGNORE_PATTERN += "$(abs_top_srcdir)/test/*"
CODE_COVERAGE_IGNORE_PATTERN += "$(abs_top_srcdir)/include/wassail/fmt/*"
CODE_COVERAGE_IGNORE_PATTERN += "$(abs_top_srcdir)/include/wassail/json/*"
CODE_COVERAGE_LCOV_SHOPTS = --no-external

@DX_RULES@

SUBDIRS = include/wassail src test

dist_doc_DATA = README.md
EXTRA_DIST = .clang-format autogen.sh Doxyfile.in LICENSE \
             wassail.spec wassail.spec.in

.PHONY: check-format format
if HAVE_CLANG_FORMAT
check-format:
	@for subdir in $(SUBDIRS); do \
		find $$subdir \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -not -path "*3rdparty*" -exec sh -c '$(CLANG_FORMAT) -style=file {} | diff -q {} -' \; ; \
	done

format:
	@for subdir in $(SUBDIRS); do \
		find $$subdir \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -not -path "*3rdparty*" -exec $(CLANG_FORMAT) -i -style=file {} + ; \
	done
else
check-format format:
	@echo "clang-format not found"
endif

clean-local: code-coverage-clean
	test -z "$(DX_DOCDIR)" || rm -rf $(DX_DOCDIR)

distclean-local: code-coverage-dist-clean

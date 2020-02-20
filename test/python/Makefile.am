EXTRA_DIST = test_check_cpu_core_count.py \
             test_check_disk_amount_free.py \
             test_check_disk_percent_free.py \
             test_check_file_permissions.py \
             test_check_memory_physical_size.py \
             test_check_misc_load_average.py \
             test_check_rules_engine.py \
             test_data.py \
             test_result.py \
             test_version.py

if WITH_PYTHON
# The Python library depends on libwassail being installed, so this
# is an installcheck rather than a check.
installcheck-local:
	@export PYTHONPATH=$(pythondir); \
        $(PYTHON) -m unittest discover
endif

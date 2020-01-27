# Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This sample reads standard input for matching JSON strings. A match
# is based on the "name" field. When a match is detected, the corresponding
# check is performed. Checks verify 1) at least 5% of the "/" filesystem
# is free, 2) at least 4 MB of the "/" filesystem is free, and 3) the amount
# of physical memory is 4 GB.
# 
# Pipe the output from the "dump" sample to this sample to demonstate
# a potential workflow.

from __future__ import print_function

import json
import sys

try:
    import wassail
except ModuleNotFoundError:
    # If the wassail module is installed in a non-default location,
    # you may need to set the PYTHONPATH environment variable to that
    # location.  Also use the same python version the module was built
    # for, typically the latest version of python present on the
    # system.
    print('Unable to load the wassail module')
    exit(1)

def str_issue(issue):
    if issue == wassail.issue_t.YES:
        return 'YES'
    elif issue == wassail.issue_t.NO:
        return 'NO'
    elif issue == wassail.issue_t.MAYBE:
        return 'MAYBE'
    else:
        return 'UNKNOWN'

def str_priority(priority):
    if priority == wassail.priority_t.EMERGENCY:
        return 'EMERGENCY'
    elif priority == wassail.priority_t.ALERT:
        return 'ALERT'
    elif priority == wassail.priority_t.ERROR:
        return 'ERROR'
    elif priority == wassail.priority_t.WARNING:
        return 'WARNING'
    elif priority == wassail.priority_t.NOTICE:
        return 'NOTICE'
    elif priority == wassail.priority_t.INFO:
        return 'INFO'
    elif priority == wassail.priority_t.DEBUG:
        return 'DEBUG'
    else:
        return 'UNKNOWN'

def print_result(result, level=0):
    print(' ' * level, 'brief: {0} {1} {2}'.format(
            result.brief, str_priority(result.priority),
            str_issue(result.issue)))
    if result.detail:
        print(' ' * (level + 2), 'detail: {}'.format(result.detail))

    for child in result.children:
        print_result(child, level + 2)

if __name__ == '__main__':
    # Dictionary mapping data source name to check
    check_map = {}
    check_map['sysconf'] = [
        wassail.check.memory.physical_size(4*1024*1024*1024, 1*1024)]
    check_map['sysctl'] = [
        wassail.check.memory.physical_size(4*1024*1024*1024, 1*1024)]
    check_map['sysinfo'] = [
        wassail.check.memory.physical_size(4*1024*1024*1024, 1*1024)]
    check_map['getfsstat'] = [wassail.check.disk.percent_free('/', 5.0),
                              wassail.check.disk.amount_free('/', 4*1024*1024)]
    check_map['getmntent'] = [wassail.check.disk.percent_free('/', 5.0),
                              wassail.check.disk.amount_free('/', 4*1024*1024)]

    # read JSON from standard input
    for line in sys.stdin:
        j = json.loads(line)

        # look for a match
        data_source = j['name']
        if data_source in check_map:
            for c in check_map[data_source]:
                # perform check
                r = c.check(j)
                print_result(r)

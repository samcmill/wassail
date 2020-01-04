# Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This sample is a complete standalone program to check that 1) at least 5%
# of the "/" file system is free and 2) the amount of physical memory
# is 4GB.  Data building blocks are called to collect the raw data,
# and the raw data is verified using check building blocks.
#
# The sample also demonstrates how to use results.  The individual
# results are children of the top level result.

from __future__ import print_function

import json
import logging
import sys

try:
    import wassail
except:
    # If the wassail module is installed in a non-default location,
    # you may need to set the PYTHONPATH environment variable to that
    # location.  Also use the same python version the module was built
    # for, typically the latest version of python present on the
    # system.
    print('Unable to load the wassail module')
    exit(1)

def str_issue(issue):
    """Return a string corresponding the result issue enum"""

    if issue == wassail.issue_t.YES:
        return 'YES'
    elif issue == wassail.issue_t.NO:
        return 'NO'
    elif issue == wassail.issue_t.MAYBE:
        return 'MAYBE'
    else:
        return 'UNKNOWN'

def str_priority(priority):
    """Return a string corresponding the result priority enum"""

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
    """Print a wassail result"""

    print(' ' * level, 'brief: {0} {1} {2}'.format(
            result.brief, str_priority(result.priority),
            str_issue(result.issue)))
    if result.detail:
        print(' ' * (level + 2), 'detail: {}'.format(result.detail))

    for child in result.children:
        print_result(child, level + 2)

if __name__ == '__main__':
    overall_result = wassail.result('Standalone wassail sample')

    # checks
    disk = wassail.check.disk.percent_free('/', 5.0)
    memory = wassail.check.memory.physical_size(4 * 1024*1024*1024, 1 * 1024)

    # data sources
    getfsstat = wassail.data.getfsstat()
    getmntent = wassail.data.getmntent()
    sysconf = wassail.data.sysconf()
    sysctl = wassail.data.sysctl()
    sysinfo = wassail.data.sysinfo()

    # perform disk check
    if getfsstat.enabled():
      r = disk.check(getfsstat)
      overall_result.add_child(r)
    elif getmntent.enabled():
      r = disk.check(getmntent)
      overall_result.add_child(r)
    else:
      print("Unable to perform disk check - no data")

    # perform memory check
    if sysconf.enabled():
      r = memory.check(sysconf)
      overall_result.add_child(r)
    elif sysinfo.enabled():
      r = memory.check(sysinfo)
      overall_result.add_child(r)
    elif sysctl.enabled():
      r = memory.check(sysctl)
      overall_result.add_child(r)
    else:
      print("Unable to perform memory check - no data")

    # update the overall result based on the children
    overall_result.issue = overall_result.max_issue()
    overall_result.priority = overall_result.max_priority()

    # print the result
    print_result(overall_result)

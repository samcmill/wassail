# Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This sample is a standalone program to demonstrate some of the
# possible ways to perform a check, including use of the rules engine. 
#
# The sample also demonstrates overriding the default format strings
# to customize the output messages.

from __future__ import print_function

import json
import logging
import re
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
        return 'NOT OK'
    elif issue == wassail.issue_t.NO:
        return 'OK'
    elif issue == wassail.issue_t.MAYBE:
        return '???'
    else:
        return 'UNKNOWN'

def print_result(result):
    """Print a wassail result"""

    print('{0}: {1} '.format(result.brief, str_issue(result.issue)))
    if result.detail:
        print('  {}'.format(result.detail))

    for c in result.children:
        print_result(c)

if __name__ == '__main__':
    # Collect some reference data.  Run STREAM 3 times and save the results
    # to a list.
    stream = wassail.data.stream()
    streamlist = []
    for _ in range(3):
        stream.evaluate(True)
        print(json.dumps(json.loads(str(stream)), indent=2))
        streamlist.append(json.loads(str(stream)))

    # Check that all STREAM triad bandwidth values are greater than 10000 MB/s
    r1 = wassail.make_result()
    r1.brief = "Checking STREAM Triad bandwidth"

    c1 = wassail.check.rules_engine(
      'Checking STREAM Triad bandwidth is greater than 10000 MB/s',
      'Measured STREAM Triad bandwidth of {0} MB/s is less than bandwidth '
      'threshold of 10000 MB/s',
      'Error performing check: {0}',
      'Measured STREAM Triad bandwidth of {0} MB/s is greater than or equal to '
      'bandwidth threshold of 10000 MB/s')
    c1.add_rule(lambda j: j['data']['triad'] >= 10000.0)

    for s in streamlist:
        r1i = c1.check(s)
        r1i.detail = r1i.detail.format(s['data']['triad'])
        r1.add_child(r1i)

    r1.propagate()
    print_result(r1)

    # Check that each STREAM triad bandwidth value is in the range 12000
    # +/- 500 MB/s
    tolerance = 500
    outlier = lambda j: j['data']['triad'] <= 12000 + tolerance and j['data']['triad'] >= 12000 - tolerance

    r2 = wassail.make_result()
    r2.brief = 'Checking STREAM Triad bandwidth'

    c2 = wassail.check.rules_engine(
      'Checking STREAM Triad bandwidth is in the range 12000 +/- 500 MB/s',
      'STREAM Triad bandwidth of {0} MB/s is outside the acceptable range',
      'Error during comparison: {0}',
      'STREAM Triad bandwidth of {0} MB/s is in the acceptable range')
    c2.add_rule(outlier)

    for s in streamlist:
        r2i = c2.check(s)
        r2i.detail = r2i.detail.format(s['data']['triad'])
        r2.add_child(r2i)

    r2.propagate()
    print_result(r2)

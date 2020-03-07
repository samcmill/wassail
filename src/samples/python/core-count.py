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
import re

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

if __name__ == '__main__':
    # Collect some reference data
    sysconf = wassail.data.sysconf()
    sysconf.evaluate()
    jdata = json.loads(str(sysconf))
    print('The observed data is:\n{0}\n'.format(
      json.dumps(jdata, indent=2, sort_keys=True)))

    # Set the reference value to the observed number of cores
    ref_value = jdata['data']['nprocessors_onln']

    # Purpose built check
    c1 = wassail.check.cpu.core_count(ref_value)
    r1 = c1.check(sysconf)
    print('Example 1: ', end=''), print_result(r1)

    # Purpose built check, bad value
    c2 = wassail.check.cpu.core_count(ref_value * 2 - 1)
    r2 = c2.check(sysconf)
    print('Example 2: ', end=''), print_result(r2)

    # Purpose built check, override default format templates */
    c3 = wassail.check.cpu.core_count(ref_value,
      'Purpose built core count check',
      'Observed {0} cores, expected {1} cores',
      'Issue checking number of cores: "{0}"',
      'Observed and expected {0} cores')
    r3 = c3.check(sysconf)
    print('Example 3: ', end=''), print_result(r3)

    # Simple custom rule
    c4 = wassail.check.rules_engine()
    c4.add_rule(lambda j: j['data']['nprocessors_onln'] == ref_value)
    r4 = c4.check(jdata)
    print('Example 4: ', end=''), print_result(r4)

    # Simple custom rule, manully set result brief
    c5 = wassail.check.rules_engine()
    c5.add_rule(lambda j: j['data']['nprocessors_onln'] != 7)
    r5 = c5.check(jdata)
    r5.brief = 'Checking number or cores is not {0}'.format(7)
    print('Example 5: ', end=''), print_result(r5)

    # Simple custom rule with custom result format templates
    c6 = wassail.check.rules_engine(
      'Checking number of cores is at least {1}',
      'Obseved number of cores {0} is less than {1}',
      'Error during comparision: {0}',
      'Observed number of cores {0} is greater than or equal to {1}')
    c6.add_rule(lambda j: j['data']['nprocessors_onln'] >= ref_value)
    # The C++ templated function signature variant is not supported,
    # so apply the template arguments manually
    #r6 = c6.check(jdata), jdata['data']['nprocessors_onln'], ref_value)
    r6 = c6.check(jdata)
    r6.brief = r6.brief.format(jdata['data']['nprocessors_onln'], ref_value)
    r6.detail = r6.detail.format(jdata['data']['nprocessors_onln'], ref_value)
    print('Example 6: ', end=''), print_result(r6)

    # Use a rule to do a regular expression comparison.  This case is
    # contrived, but it illustrates what is possible
    c7 = wassail.check.rules_engine()
    c7.add_rule(lambda j: bool(re.match('^{0}$'.format(ref_value), str(j['data']['nprocessors_onln']))))
    r7 = c7.check(jdata)
    r7.brief = 'Checking number of cores matches "^{0}$"'.format(ref_value)
    print('Example 7: ', end=''), print_result(r7)

    # Use rules to check whether the number of processors is within a range
    tolerance = 2
    c8 = wassail.check.rules_engine(
      'Checking range', 'Observed value {0} is not equal to {1} +/- {2}',
      'Error: "{0}"', 'Observed value {0} is equal to {1} +/- {2}')
    c8.add_rule(lambda j: j['data']['nprocessors_onln'] <= ref_value + tolerance)
    c8.add_rule(lambda j: j['data']['nprocessors_onln'] >= ref_value - tolerance)
    r8 = c8.check(jdata)
    r8.brief = r8.brief.format(jdata['data']['nprocessors_onln'], ref_value, tolerance)
    r8.detail = r8.detail.format(jdata['data']['nprocessors_onln'], ref_value, tolerance)
    print('Example 8: ', end=''), print_result(r8)

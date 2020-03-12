# Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# The sample is a complete standalone program demonstrating several
# approaches for using wassail.

from __future__ import print_function

import argparse
import json

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
        return 'UNKNOWN'
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

    print(' ' * level, '{0} {1} {2}'.format(
            result.brief, str_priority(result.priority),
            str_issue(result.issue)))
    if result.detail:
        print(' ' * (level + 2), result.detail)

    for child in result.children:
        print_result(child, level + 2)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Sample program demonstrating several approaches for using wassail')
    parser.add_argument('--method', type=int, help='Use method N (0-10)')
    args = parser.parse_args()

    if args.method == 0:
        # No check, just print the raw data
        d = wassail.data.environment()
        d.evaluate()
        j = json.loads(str(d))
        print(json.dumps(j, indent=2))
    elif args.method == 1:
        # Check that the environment variable FOO equals "bar"
        d = wassail.data.environment()
        c = wassail.check.misc.environment('FOO', 'bar')
        r = c.check(d) # implicitly evaluates d
        print(r)
    elif args.method == 2:
        # Check that the environment variable FOO equals "bar"
        d = wassail.data.environment()
        d.evaluate()

        c = wassail.check.misc.environment('FOO', 'bar')
        r = c.check(d) # does not re-evaluate d

        print_result(r)
    elif args.method == 3:
        # Check that the environment variable FOO equals "bar"
        d = wassail.data.environment()
        d.evaluate()
        j = json.loads(str(d))

        c = wassail.check.misc.environment('FOO', 'bar')
        r = c.check(j)

        print_result(r)
    elif args.method == 4:
        # Check that the environment variable PATH contains "/usr/bin"
        d = wassail.data.environment()
        d.evaluate()

        c = wassail.check.misc.environment('PATH', '/usr/bin', True)
        r = c.check(d)

        print_result(r)
    elif args.method == 5:
        # Check that the environment variable FOO equals "bar" and
        # the environment variable PATH contains "/usr/bin"
        d = wassail.data.environment()
        d.evaluate()

        c1 = wassail.check.misc.environment('FOO', 'bar')
        r1 = c1.check(d)
        print_result(r1)

        c2 = wassail.check.misc.environment('PATH', '/usr/bin', True)
        r2 = c2.check(d)
        print_result(r2)
    elif args.method == 6:
        # Check that the environment variable FOO equals "bar" and
        # the environment variable PATH contains "/usr/bin"
        d = wassail.data.environment()
        d.evaluate()
        j = json.loads(str(d))

        r = wassail.make_result(j)
        r.brief = 'Checking environment'

        c1 = wassail.check.misc.environment('FOO', 'bar')
        r1 = c1.check(d)
        r.add_child(r1) # add the FOO check

        c2 = wassail.check.misc.environment('PATH', '/usr/bin', True)
        r2 = c2.check(d)
        r.add_child(r2) # add the PATH check

        # Set the overarching result priority and issue levels based on
        # the FOO and PATH checks
        r.issue = r.max_issue()
        r.priority = r.max_priority()

        print_result(r)
    elif args.method == 7:
        print('No corresponding Python method for C++ method 7')
    elif args.method == 8:
        print('No corresponding Python method for C++ method 8')
    elif args.method == 9:
        # Check that the environment variable FOO equals "bar"
        d = wassail.data.environment()
        d.evaluate()
        j = json.loads(str(d))

        c = wassail.check.rules_engine('Checking environment variable "FOO"',
                                       'Value "{0}" does not match "bar"',
                                       'Unable to perform comparison: "{0}"',
                                       'Value "{0}" matches "bar"')
        c.add_rule(lambda x: 'FOO' in x['data'])
        c.add_rule(lambda x: x['data']['FOO'] == 'bar')
        r = c.check(j)

        # apply actual value to the detail format strings
        if r.issue == wassail.issue_t.YES or r.issue == wassail.issue_t.NO:
          r.detail = r.detail.format(j['data'].get('FOO', ''))

        print_result(r)
    elif args.method == 10:
      # Check that the environment variable FOO equals "bar"
      d = wassail.data.shell_command("printenv FOO")

      c = wassail.check.misc.shell_output("bar\n")
      r = c.check(d)

      print_result(r)
    else:
        print('Please use a valid method')

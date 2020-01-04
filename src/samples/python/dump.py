# Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This sample calls each data building block.  If the building block
# is enabled, then the result is printed as JSON to stdout.  Otherwise,
# an error message is printed to stderr.

from __future__ import print_function

import json
import logging

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

def dump(d):
    try:
        d.evaluate()
        j = json.loads(str(d))
        print(json.dumps(j))
        return True
    except RuntimeError as e:
        logging.warning(str(e))
        return False

if __name__ == '__main__':
    dump(wassail.data.environment())
    dump(wassail.data.getcpuid())
    dump(wassail.data.getfsstat())
    dump(wassail.data.getloadavg())
    dump(wassail.data.getmntent())
    dump(wassail.data.getrlimit())
    dump(wassail.data.pciaccess())
    dump(wassail.data.pciutils())
    dump(wassail.data.ps())
    dump(wassail.data.shell_command('uptime'))
    dump(wassail.data.stat('/tmp'))
    dump(wassail.data.stream())
    dump(wassail.data.sysconf())
    dump(wassail.data.sysctl())
    dump(wassail.data.sysinfo())
    dump(wassail.data.umad())
    dump(wassail.data.uname())

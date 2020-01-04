import json
import os
import unittest
import wassail

class Test(unittest.TestCase):
    def test_environment_json(self):
        """environment json input"""
        j = json.loads('{"name": "environment", "data": {"USER": "ncognito", "HOME": "/home/ncognito", "SHELL": "/bin/bash"}}')

        c1 = wassail.check.misc.environment('USER', 'ncognito')
        r1 = c1.check(j)
        self.assertEqual(r1.issue, wassail.issue_t.NO)

        c2 = wassail.check.misc.environment('SHELL', '/bin/tcsh')
        r2 = c2.check(j)
        self.assertEqual(r2.issue, wassail.issue_t.YES)

        c3 = wassail.check.misc.environment('HOME', '^/home$', True, "Brief {0}", "{0} !~ {1}", ":shrug:", "{0} =~ {1}")
        r3 = c3.check(j)
        self.assertEqual(r3.issue, wassail.issue_t.YES)
        self.assertEqual(r3.brief, "Brief HOME")
        self.assertEqual(r3.detail, "/home/ncognito !~ ^/home$")

    def test_environment(self):
        """environment input"""
        d = wassail.data.environment()
        try:
            d.evaluate()
        except:
            pass
        else:
            c = wassail.check.misc.environment('SHELL', os.environ['SHELL'])
            r = c.check(d)
            self.assertEqual(r.issue, wassail.issue_t.NO)

    def test_invalid_input(self):
        """invalid input"""
        c = wassail.check.misc.environment('SHELL', os.environ['SHELL'])

        with self.assertRaises(RuntimeError):
            c.check("invalid")

    def test_unknown_json(self):
        """unknown json"""
        j = json.loads('{"name": "unknown", "data": {"FOO": "BAR"}}')
        c = wassail.check.misc.environment('FOO', 'BAR')

        with self.assertRaises(RuntimeError):
            c.check(j)

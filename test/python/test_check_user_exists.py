import json
import unittest
import wassail

class Test(unittest.TestCase):
    def test_stat_json(self):
        """getpwent json input"""
        j = json.loads('{"name": "getpwent", "data": {"users": [ { "pw_dir": "/var/root", "pw_gid": 0, "pw_name": "root", "pw_shell": "/bin/sh", "pw_uid":0 } ] }}')

        c1 = wassail.check.user.exists('root')
        r1 = c1.check(j)
        self.assertEqual(r1.issue, wassail.issue_t.NO)

        c2 = wassail.check.user.exists('bob')
        r2 = c2.check(j)
        self.assertEqual(r2.issue, wassail.issue_t.YES)

    def test_getpwent(self):
        """getpwent input"""
        d = wassail.data.getpwent()
        try:
            d.evaluate()
        except:
            pass
        else:
          c = wassail.check.user.exists('root')
          r = c.check(d)
          self.assertEqual(r.issue, wassail.issue_t.NO)

    def test_invalid_input(self):
        """invalid input"""
        c = wassail.check.user.exists('root')

        with self.assertRaises(RuntimeError):
            c.check('invalid')

    def test_unknown_json(self):
        """unknown json"""
        j = json.loads('{"name": "unknown", "data": {"users": ""}}')
        c = wassail.check.user.exists('root')

        with self.assertRaises(RuntimeError):
            c.check(j)

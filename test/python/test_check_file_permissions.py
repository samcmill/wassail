import json
import unittest
import wassail

class Test(unittest.TestCase):
    def test_stat_json(self):
        """stat json input"""
        j = json.loads('{"name": "stat", "data": {"mode": 17407, "path": "/tmp"}}')

        c1 = wassail.check.file.permissions(0o1777)
        r1 = c1.check(j)
        self.assertEqual(r1.issue, wassail.issue_t.NO)

        c2 = wassail.check.file.permissions(0o777)
        r2 = c2.check(j)
        self.assertEqual(r2.issue, wassail.issue_t.YES)

    def test_invalid_input(self):
        """invalid input"""
        c = wassail.check.file.permissions(0o1777)

        with self.assertRaises(RuntimeError):
            c.check("invalid")

    def test_unknown_json(self):
        """unknown json"""
        j = json.loads('{"name": "unknown", "data": {"mode": 17407}}')
        c = wassail.check.file.permissions(0o1777)

        with self.assertRaises(RuntimeError):
            c.check(j)

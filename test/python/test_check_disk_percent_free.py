import json
import unittest
import wassail

class Test(unittest.TestCase):
    def test_getfsstat_json(self):
        """getfsstat json input"""
        j = json.loads('{"name": "getfsstat", "data": {"file_systems": [ {"bavail": 5771575, "blocks": 61228134, "mntonname": "/"}]}}')

        # percent = 100 * 5771575 / 61228134 = 9.43%
        c1 = wassail.check.disk.percent_free('/', 9.4)
        r1 = c1.check(j)
        self.assertEqual(r1.issue, wassail.issue_t.NO)

        c2 = wassail.check.disk.percent_free('/', 9.5)
        r2 = c2.check(j)
        self.assertEqual(r2.issue, wassail.issue_t.YES)

        c3 = wassail.check.disk.percent_free('/', 100, "Brief", "{0:.1f} < {1:.1f}", ":shrug:", "{0} >= {1}")
        r3 = c3.check(j)
        self.assertEqual(r3.issue, wassail.issue_t.YES)
        self.assertEqual(r3.brief, "Brief")
        self.assertEqual(r3.detail, "9.4 < 100.0")

        c4 = wassail.check.disk.percent_free('/invalid', 10.0)
        r4 = c4.check(j)
        self.assertEqual(r4.issue, wassail.issue_t.MAYBE)

    def test_getfsstat(self):
        """getfsstat input"""
        d = wassail.data.getfsstat()
        try:
            d.evaluate()
        except:
            pass
        else:
            c = wassail.check.disk.percent_free('/', 100) # intentionally unlikely
            r = c.check(d)
            self.assertEqual(r.issue, wassail.issue_t.YES)

    def test_getmntent_json(self):
        """getmntent json input"""
        j = json.loads('{"name": "getmntent", "data": {"file_systems": [{"bavail": 76420, "blocks": 1621504, "dir": "/"}]}}')

        # percent = 100 * 76420 / 1621504 = 4.71%
        c1 = wassail.check.disk.percent_free('/', 4.6)
        r1 = c1.check(j)
        self.assertEqual(r1.issue, wassail.issue_t.NO)

        c2 = wassail.check.disk.percent_free('/', 4.8)
        r2 = c2.check(j)
        self.assertEqual(r2.issue, wassail.issue_t.YES)

    def test_getmntent(self):
        """getmntent input"""
        d = wassail.data.getmntent()
        try:
            d.evaluate()
        except:
            pass
        else:
            c = wassail.check.disk.percent_free('/', 100) # intentionally unlikely
            r = c.check(d)
            self.assertEqual(r.issue, wassail.issue_t.YES)

    def test_invalid_input(self):
        """invalid input"""
        c = wassail.check.disk.percent_free('invalid')

        with self.assertRaises(RuntimeError):
            c.check("invalid")

    def test_unknown_json(self):
        """unknown json"""
        j = json.loads('{"name": "unknown", "data": {"disk": 4}}')
        c = wassail.check.disk.percent_free('/', 4)

        with self.assertRaises(RuntimeError):
            c.check(j)

import json
import unittest
import wassail

class Test(unittest.TestCase):
    def test_getfsstat_json(self):
        """getfsstat json input"""
        j = json.loads('{"name": "getfsstat", "data": {"file_systems": [ {"bavail": 5771575, "bsize": 4096, "mntonname": "/"}]}}')

        # percent = 100 * 5771575 / 61228134 = 9.43%
        # amount = 5771575 * 4096 = 23640371200 bytes
        c1 = wassail.check.disk.amount_free('/', 23640371200)
        r1 = c1.check(j)
        self.assertEqual(r1.issue, wassail.issue_t.NO)

        c2 = wassail.check.disk.amount_free('/', 23640371201)
        r2 = c2.check(j)
        self.assertEqual(r2.issue, wassail.issue_t.YES)

        c3 = wassail.check.disk.amount_free('/', 100000000000000, "Brief", "{1} {3} < {2} {3}", ":shrug:", "{1} {3} >= {2} {3}")
        r3 = c3.check(j)
        self.assertEqual(r3.issue, wassail.issue_t.YES)
        self.assertEqual(r3.brief, "Brief")
        self.assertEqual(r3.detail, "23640371200 bytes < 100000000000000 bytes")

        c4 = wassail.check.disk.amount_free('/invalid', 10)
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
            c = wassail.check.disk.amount_free('/', 100000000000000) # intentionally unlikely
            r = c.check(d)
            self.assertEqual(r.issue, wassail.issue_t.YES)

    def test_getmntent_json(self):
        """getmntent json input"""
        j = json.loads('{"name": "getmntent", "data": {"file_systems": [{"bavail": 76420, "bsize": 4096, "dir": "/"}]}}')

        # amount = 76420 * 4096 = 313016320 bytes
        c1 = wassail.check.disk.amount_free('/', 313016320)
        r1 = c1.check(j)
        self.assertEqual(r1.issue, wassail.issue_t.NO)

        c2 = wassail.check.disk.amount_free('/', 313016321)
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
            c = wassail.check.disk.amount_free('/', 100000000000000) # intentionally unlikely
            r = c.check(d)
            self.assertEqual(r.issue, wassail.issue_t.YES)

    def test_invalid_input(self):
        """invalid input"""
        c = wassail.check.disk.amount_free('invalid')

        with self.assertRaises(RuntimeError):
            c.check("invalid")

    def test_unknown_json(self):
        """unknown json"""
        j = json.loads('{"name": "unknown", "data": {"disk": 4}}')
        c = wassail.check.disk.amount_free('/', 4)

        with self.assertRaises(RuntimeError):
            c.check(j)

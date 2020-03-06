import json
import unittest
import wassail

class Test(unittest.TestCase):
    def test_getloadavg_json(self):
        """getloadavg json input"""
        j = json.loads('{"name": "getloadavg", "data": {"load1": 2.0, "load5": 2.1, "load15": 2.2}}')

        c1 = wassail.check.misc.load_average(2.0)
        r1 = c1.check(j)
        self.assertEqual(r1.issue, wassail.issue_t.NO)

        c2 = wassail.check.misc.load_average(2.0, wassail.check.misc.load_average.minute_t.FIVE)
        r2 = c2.check(j)
        self.assertEqual(r2.issue, wassail.issue_t.YES)

        c3 = wassail.check.misc.load_average(2.1234, wassail.check.misc.load_average.minute_t.FIFTEEN, "Brief {0}", "{1} > {2}", ":shrug:", "{1} <= {2}")
        r3 = c3.check(j)
        self.assertEqual(r3.issue, wassail.issue_t.YES)
        self.assertEqual(r3.brief, "Brief 15")
        self.assertEqual(r3.detail, "2.2 > 2.1234")

    def test_getloadavg(self):
        """getloadavg input"""
        d = wassail.data.getloadavg()
        try:
            d.evaluate()
        except:
            pass
        else:
            c = wassail.check.misc.load_average(9999.9) # intentionally unlikely
            r = c.check(d)
            self.assertEqual(r.issue, wassail.issue_t.NO)

    def test_sysctl_json(self):
        """sysctl json input"""
        j = json.loads('{"name": "sysctl", "data": {"vm": {"loadavg": {"fscale": 2048, "load1": 3010, "load5": 3225, "load15": 3063}}}}')

        c1 = wassail.check.misc.load_average(1.0)
        r1 = c1.check(j)
        self.assertEqual(r1.issue, wassail.issue_t.YES)

        c2 = wassail.check.misc.load_average(2.0, wassail.check.misc.load_average.minute_t.FIVE)
        r2 = c2.check(j)
        self.assertEqual(r2.issue, wassail.issue_t.NO)

        c3 = wassail.check.misc.load_average(1.1234, wassail.check.misc.load_average.minute_t.FIFTEEN, "Brief {0}", "{1} > {2}", ":shrug:", "{1} <= {2}")
        r3 = c3.check(j)
        self.assertEqual(r3.issue, wassail.issue_t.YES)
        self.assertEqual(r3.brief, "Brief 15")
        self.assertEqual(r3.detail, "1.49561 > 1.1234")

    def test_sysctl(self):
        """sysctl input"""
        d = wassail.data.sysctl()
        try:
            d.evaluate()
        except:
            pass
        else:
            c = wassail.check.misc.load_average(9999.9) # intentionally unlikely
            r = c.check(d)
            self.assertEqual(r.issue, wassail.issue_t.NO)

    def test_sysinfo_json(self):
        """sysinfo json input"""
        j = json.loads('{"name": "sysinfo", "data": {"load1": 80672, "load5": 87520, "load15": 101664, "loads_scale": 65536}}')

        c1 = wassail.check.misc.load_average(1.0)
        r1 = c1.check(j)
        self.assertEqual(r1.issue, wassail.issue_t.YES)

        c2 = wassail.check.misc.load_average(2.0, wassail.check.misc.load_average.minute_t.FIVE)
        r2 = c2.check(j)
        self.assertEqual(r2.issue, wassail.issue_t.NO)

        c3 = wassail.check.misc.load_average(1.5678, wassail.check.misc.load_average.minute_t.FIFTEEN, "Brief {0}", "{1} > {2}", ":shrug:", "{1} <= {2}")
        r3 = c3.check(j)
        self.assertEqual(r3.issue, wassail.issue_t.NO)
        self.assertEqual(r3.brief, "Brief 15")
        self.assertEqual(r3.detail, "1.55127 <= 1.5678")

    def test_sysinfo(self):
        """sysinfo input"""
        d = wassail.data.sysinfo()
        try:
            d.evaluate()
        except:
            pass
        else:
            c = wassail.check.misc.load_average(9999.9) # intentionally unlikely
            r = c.check(d)
            self.assertEqual(r.issue, wassail.issue_t.NO)

    def test_invalid_input(self):
        """invalid input"""
        c = wassail.check.misc.load_average(1.0)

        with self.assertRaises(RuntimeError):
            c.check("invalid")

    def test_unknown_json(self):
        """unknown json"""
        j = json.loads('{"name": "unknown", "data": {"load1": 4.0}}')
        c = wassail.check.misc.load_average(4.0)

        with self.assertRaises(RuntimeError):
            c.check(j)

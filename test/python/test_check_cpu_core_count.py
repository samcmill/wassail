import datetime
import json
import unittest
import wassail

class Test(unittest.TestCase):
    def test_sysconf_json(self):
        """sysconf json input"""
        j = json.loads('{"name": "sysconf", "data": {"nprocessors_onln": 4}, "hostname": "localhost", "timestamp": 1546300800}')

        c1 = wassail.check.cpu.core_count(4)
        r1 = c1.check(j)
        self.assertEqual(r1.issue, wassail.issue_t.NO)
        self.assertEqual(r1.system_id, ['localhost'])
        self.assertEqual(r1.timestamp, datetime.datetime.fromtimestamp(1546300800))

        c2 = wassail.check.cpu.core_count(8)
        r2 = c2.check(j)
        self.assertEqual(r2.issue, wassail.issue_t.YES)

        c3 = wassail.check.cpu.core_count(4, "Brief", "{0} != {1}", ":shrug:",
                                          "{0} == {1}")
        r3 = c3.check(j)
        self.assertEqual(r3.issue, wassail.issue_t.NO)
        self.assertEqual(r3.brief, "Brief")
        self.assertEqual(r3.detail, "4 == 4")

    def test_sysconf(self):
        """sysconf input"""
        d = wassail.data.sysconf()
        try:
            d.evaluate()
        except:
            pass
        else:
            c = wassail.check.cpu.core_count(9999) # intentionally unlikely
            r = c.check(d)
            self.assertEqual(r.issue, wassail.issue_t.YES)

    def test_sysctl_json(self):
        """sysctl json input"""
        j = json.loads('{"name": "sysctl", "data": {"machdep": {"cpu": {"core_count": 8}}}, "hostname": "localhost", "timestamp": 0}')

        c1 = wassail.check.cpu.core_count(4)
        r1 = c1.check(j)
        self.assertEqual(r1.issue, wassail.issue_t.YES)

        c2 = wassail.check.cpu.core_count(8)
        r2 = c2.check(j)
        self.assertEqual(r2.issue, wassail.issue_t.NO)

    def test_sysctl(self):
        """sysctl input"""
        d = wassail.data.sysctl()
        try:
            d.evaluate()
        except:
            pass
        else:
            c = wassail.check.cpu.core_count(9999) # intentionally unlikely
            r = c.check(d)
            self.assertEqual(r.issue, wassail.issue_t.YES)

    def test_invalid_input(self):
        """invalid input"""
        c = wassail.check.cpu.core_count(4)

        with self.assertRaises(RuntimeError):
            c.check("invalid")

    def test_unknown_json(self):
        """unknown json"""
        j = json.loads('{"name": "unknown", "data": {"num_cores": 4}}')
        c = wassail.check.cpu.core_count(4)

        with self.assertRaises(RuntimeError):
            c.check(j)

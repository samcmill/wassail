import json
import unittest
import wassail

class Test(unittest.TestCase):
    def test_sysconf_json(self):
        """sysconf json input"""
        j = json.loads('{"name": "sysconf", "data": {"page_size": 4096, "phys_pages": 2097152}}')

        c1 = wassail.check.memory.physical_size(8*1024*1024*1024)
        r1 = c1.check(j)
        self.assertEqual(r1.issue, wassail.issue_t.NO)

        c2 = wassail.check.memory.physical_size(4*1024*1024*1024)
        r2 = c2.check(j)
        self.assertEqual(r2.issue, wassail.issue_t.YES)

        c3 = wassail.check.memory.physical_size(8*1024*1024*1024, 1024, "Brief", "{0} {1} {2} {3}", ":shrug:", "{0} {1} {2} {3}")
        r3 = c3.check(j)
        self.assertEqual(r3.issue, wassail.issue_t.NO)
        self.assertEqual(r3.brief, "Brief")
        self.assertEqual(r3.detail, "8589934592 8589934592 1024 bytes")

    def test_sysconf(self):
        """sysconf input"""
        d = wassail.data.sysconf()
        try:
            d.evaluate()
        except:
            pass
        else:
            c = wassail.check.memory.physical_size(0) # intentionally unlikely
            r = c.check(d)
            self.assertEqual(r.issue, wassail.issue_t.YES)

    def test_sysctl_json(self):
        """sysctl json input"""
        j = json.loads('{"name": "sysctl", "data": {"hw": {"memsize": 8589934592}}}')

        c1 = wassail.check.memory.physical_size(4*1024*1024*1024)
        r1 = c1.check(j)
        self.assertEqual(r1.issue, wassail.issue_t.YES)

        c2 = wassail.check.memory.physical_size(7*1024*1024*1024, 1*1024*1024*1024)
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
            c = wassail.check.memory.physical_size(0) # intentionally unlikely
            r = c.check(d)
            self.assertEqual(r.issue, wassail.issue_t.YES)

    def test_sysinfo_json(self):
        """sysinfo json input"""
        j = json.loads('{"name": "sysinfo", "data": {"totalram": 1040621568, "mem_unit": 1}}')

        c1 = wassail.check.memory.physical_size(1*1024*1024*1024)
        r1 = c1.check(j)
        self.assertEqual(r1.issue, wassail.issue_t.YES)

        c2 = wassail.check.memory.physical_size(1*1024*1024*1024, 64*1024*1024)
        r2 = c2.check(j)
        self.assertEqual(r2.issue, wassail.issue_t.NO)

    def test_sysinfo(self):
        """sysinfo input"""
        d = wassail.data.sysinfo()
        try:
            d.evaluate()
        except:
            pass
        else:
            c = wassail.check.memory.physical_size(0) # intentionally unlikely
            r = c.check(d)
            self.assertEqual(r.issue, wassail.issue_t.YES)

    def test_invalid_input(self):
        """invalid input"""
        c = wassail.check.memory.physical_size(4*1024*1024*1024)

        with self.assertRaises(RuntimeError):
            c.check("invalid")

    def test_unknown_json(self):
        """unknown json"""
        j = json.loads('{"name": "unknown", "data": {"mem_size": 4}}')
        c = wassail.check.memory.physical_size(4)

        with self.assertRaises(RuntimeError):
            c.check(j)

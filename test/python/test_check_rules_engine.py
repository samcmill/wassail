import datetime
import json
import unittest
import wassail

class Test(unittest.TestCase):
    def test_basic_rules_engine(self):
        """basic rules_engine"""
        j = json.loads('{"name": "sysconf", "data": {"nprocessors_onln": 4}, "hostname": "localhost", "timestamp": 1546300800}')

        c1 = wassail.check.rules_engine()

        c1.add_rule(lambda x: x["data"]["nprocessors_onln"] == 4)

        r1a = c1.check(j)
        self.assertEqual(r1a.issue, wassail.issue_t.NO)
        self.assertEqual(r1a.timestamp,
                         datetime.datetime.fromtimestamp(1546300800))

        c1.add_rule(lambda x: x["data"]["nprocessors_onln"] > 10)

        r1b = c1.check(j)
        self.assertEqual(r1b.issue, wassail.issue_t.YES)

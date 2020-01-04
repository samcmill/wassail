import datetime
import json
import unittest
import wassail

class Test(unittest.TestCase):
    def test_result_instantiation(self):
        """result instantiation"""
        r = wassail.result()
        r.action = 'Action'
        r.brief = 'Brief'
        r.detail = 'Detail'
        r.issue = wassail.issue_t.YES
        r.priority = wassail.priority_t.CRITICAL
        r.system_id = ['localhost']
        r.timestamp = datetime.datetime(2019, 1, 1, 12, 0, 0)

        self.assertEqual(r.action, 'Action')
        self.assertEqual(r.brief, 'Brief')
        self.assertEqual(len(r.children), 0)
        self.assertEqual(r.detail, 'Detail')
        self.assertEqual(r.issue, wassail.issue_t.YES)
        self.assertEqual(r.priority, wassail.priority_t.CRITICAL)
        self.assertEqual(r.system_id, ['localhost'])
        self.assertEqual(r.timestamp, datetime.datetime(2019, 1, 1, 12, 0, 0))

    def test_result_instantiation2(self):
        """result instantiation"""
        r = wassail.result('Brief2')
        self.assertEqual(r.brief, 'Brief2')

    def test_result_add_child(self):
        """result add_child"""
        a = wassail.result('A')
        b1 = wassail.result('B1')
        b2 = wassail.result('B2')
        a.add_child(b1)
        a.add_child(b2)
        self.assertEqual(len(a.children), 2)

    def test_result_add_child_two_level(self):
        """second level add_child"""
        a = wassail.result('A')
        b = wassail.result('B')
        a.add_child(b)
        c = wassail.result('C')
        b.add_child(c)

        self.assertEqual(len(a.children), 1)
        self.assertEqual(len(b.children), 1)

        for i in a.children:
            self.assertEqual(len(i.children), 1)
            for j in i.children:
                self.assertEqual(j.brief, 'C')

    def test_result_max(self):
        """result match_issue, match_priority, max_issue, and max_priority"""
        a = wassail.result('A')

        b1 = wassail.result('B1')
        b1.issue = wassail.issue_t.YES
        b1.priority = wassail.priority_t.INFO
        a.add_child(b1)

        b2 = wassail.result('B2')
        b2.issue = wassail.issue_t.MAYBE
        b2.priority = wassail.priority_t.ERROR
        a.add_child(b2)

        self.assertEqual(a.max_issue(), wassail.issue_t.YES)
        self.assertEqual(a.max_priority(), wassail.priority_t.ERROR)

        self.assertTrue(a.match_issue(wassail.issue_t.YES))
        self.assertTrue(a.match_issue(wassail.issue_t.MAYBE))
        self.assertFalse(a.match_issue(wassail.issue_t.NO))

        self.assertFalse(a.match_priority(wassail.priority_t.EMERGENCY))
        self.assertFalse(a.match_priority(wassail.priority_t.ALERT))
        self.assertTrue(a.match_priority(wassail.priority_t.ERROR))
        self.assertFalse(a.match_priority(wassail.priority_t.WARNING))
        self.assertFalse(a.match_priority(wassail.priority_t.NOTICE))
        self.assertTrue(a.match_priority(wassail.priority_t.INFO))
        self.assertFalse(a.match_priority(wassail.priority_t.DEBUG))

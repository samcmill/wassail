import json
import os
import unittest
import wassail

class Test(unittest.TestCase):
    def test_shell_output_shell_command_json(self):
        """shell_output shell_command json input"""
        j = json.loads('{"name": "shell_command", "data": {"command": "echo \'bar\'", "stdout": "bar\\n"}}')

        c1 = wassail.check.misc.shell_output("bar\n")
        r1 = c1.check(j)
        self.assertEqual(r1.issue, wassail.issue_t.NO)

        c2 = wassail.check.misc.shell_output("foo\n")
        r2 = c2.check(j)
        self.assertEqual(r2.issue, wassail.issue_t.YES)

        c3 = wassail.check.misc.shell_output("^bar$", True, "Brief", "'{0}' !~ '{1}'", ":shrug:", "'{0}' =~ '{1}'")
        r3 = c3.check(j)
        self.assertEqual(r3.issue, wassail.issue_t.YES)
        self.assertEqual(r3.detail, "'bar\n' !~ '^bar$'")

    def test_shell_output_remote_shell_command_json(self):
        """shell_output remote_shell_command json input"""
        j = json.loads('{"name": "remote_shell_command", "data": [{"data": {"command": "echo \'bar 1\'", "stdout": "bar 1\\n"}, "hostname": "node1"}, {"data": {"command": "echo \'bar 2\'", "stdout": "bar 2\\n"}, "hostname": "node2"}]}')

        c1 = wassail.check.misc.shell_output("bar\n")
        r1 = c1.check(j)
        self.assertEqual(r1.issue, wassail.issue_t.YES)
        self.assertEqual(len(r1.children), 2)

        c2 = wassail.check.misc.shell_output("bar 2\n")
        r2 = c2.check(j)
        self.assertEqual(r2.issue, wassail.issue_t.YES)
        self.assertEqual(len(r2.children), 2)
        for child in r2.children:
          if child.system_id[0] == 'node1':
            self.assertEqual(child.issue, wassail.issue_t.YES)
          elif child.system_id[0] == 'node2':
            self.assertEqual(child.issue, wassail.issue_t.NO)

        c3 = wassail.check.misc.shell_output("^bar \d\n$", True)
        r3 = c3.check(j)
        self.assertEqual(r3.issue, wassail.issue_t.NO)

    def test_shell_command(self):
        """shell_command input"""
        d = wassail.data.shell_command('echo "foo"')
        try:
            d.evaluate()
        except:
            pass
        else:
            c = wassail.check.misc.shell_output("foo\n")
            r = c.check(d)
            self.assertEqual(r.issue, wassail.issue_t.NO)

    def test_invalid_input(self):
        """invalid input"""
        c = wassail.check.misc.shell_output("foo")

        with self.assertRaises(RuntimeError):
            c.check("invalid")

    def test_unknown_json(self):
        """unknown json"""
        j = json.loads('{"name": "unknown", "data": {"command": "echo \'foo\'", "stdout": "foo"}}')
        c = wassail.check.misc.shell_output("foo\n")

        with self.assertRaises(RuntimeError):
            c.check(j)

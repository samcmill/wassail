import unittest
import wassail

class Test(unittest.TestCase):
    def test_version(self):
        """version instantiation"""
        v = wassail.version()
        major = wassail.version_major()
        minor = wassail.version_minor()
        micro = wassail.version_micro()

        self.assertNotEqual(v, '')
        self.assertEqual(v, '{0}.{1}.{2}'.format(major, minor, micro))

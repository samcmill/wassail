import json
import os
import unittest
import wassail

class Test(unittest.TestCase):
    def test_environment(self):
        """environment data source"""
        d = wassail.data.environment()
        if d.enabled():
            os.environ['__ZZZ'] = 'FOO=BAR'
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'environment')
            self.assertGreaterEqual(len(j['data']), 1)
            self.assertEqual(j['data']['__ZZZ'], "FOO=BAR")
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

    def test_getcpuid(self):
        """getcpuid data source"""
        d = wassail.data.getcpuid()
        if d.enabled():
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'getcpuid')
            self.assertGreater(j['data']['family'], 0)
            self.assertGreater(j['data']['model'], 0)
            self.assertGreater(len(j['data']['name']), 0)
            self.assertGreater(j['data']['stepping'], 0)
            self.assertGreaterEqual(j['data']['type'], 0)
            self.assertGreater(len(j['data']['vendor']), 0)
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

    def test_getfsstat(self):
        """getfsstat data source"""
        d = wassail.data.getfsstat()
        if d.enabled():
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'getfsstat')
            self.assertGreaterEqual(len(j['data']['file_systems']), 1)
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

    def test_getloadavg(self):
        """getloadavg data source"""
        d = wassail.data.getloadavg()
        if d.enabled():
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'getloadavg')
            self.assertGreaterEqual(j['data']['load1'], 0)
            self.assertGreaterEqual(j['data']['load5'], 0)
            self.assertGreaterEqual(j['data']['load15'], 0)
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

    def test_getmntent(self):
        """getmntent data source"""
        d = wassail.data.getmntent()
        if d.enabled():
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'getmntent')
            self.assertGreaterEqual(len(j['data']['file_systems']), 1)
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

    def test_getrlimit(self):
        """getrlimit data source"""
        d = wassail.data.getrlimit()
        if d.enabled():
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'getrlimit')
            self.assertGreaterEqual(j['data']['hard']['core'], 0)
            self.assertGreaterEqual(j['data']['hard']['core'],
                                    j['data']['soft']['core'])
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

    def test_mpirun(self):
        """mpirun data source"""
        d = wassail.data.mpirun(2, 'echo "foo"')
        if d.enabled():
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'mpirun')
            if j['data']['returncode'] == 0:
                self.assertEqual(j['data']['stdout'], 'foo\nfoo\n')
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

    def test_nvml(self):
        """nvml data source"""
        d = wassail.data.nvml()
        if d.enabled():
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'nvml')
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

    def test_osu_micro_benchmarks(self):
        """osu_micro_benchmarks data source"""
        d = wassail.data.osu_micro_benchmarks()
        if d.enabled():
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'osu_micro_benchmarks')
            if j['data']['returncode'] == 0:
                self.assertGreaterEqual(j['data']['avg'], 0)
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

    def test_pciaccess(self):
        """pciaccess data source"""
        d = wassail.data.pciaccess()
        if d.enabled():
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'pciaccess')
            self.assertGreaterEqual(len(j['data']['devices']), 1)
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

    def test_pciutils(self):
        """pciutils data source"""
        d = wassail.data.pciutils()
        if d.enabled():
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'pciutils')
            self.assertGreaterEqual(len(j['data']['devices']), 1)
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

    def test_ps(self):
        """ps data source"""
        d = wassail.data.ps()
        if d.enabled():
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'ps')
            self.assertGreaterEqual(len(j['data']['processes']), 1)
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

    # May fail if ssh is not setup
    @unittest.skipIf(True, 'do not assume ssh is setup')
    def test_remote_shell_command(self):
        """remote_shell_command data source"""
        d = wassail.data.remote_shell_command('localhost', 'echo "foo"')
        if d.enabled():
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'remote_shell_command')
            self.assertLessEqual(j['data']['elapsed'], 1)
            self.assertEqual(j['data']['host'], 'localhost')
            self.assertEqual(j['data']['returncode'], 0)
            self.assertEqual(j['data']['stderr'], '')
            self.assertEqual(j['data']['stdout'], 'foo\n')
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

    def test_shell_command(self):
        """shell_command data source"""
        d = wassail.data.shell_command('echo "foo"')
        if d.enabled():
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'shell_command')
            self.assertLessEqual(j['data']['elapsed'], 1)
            self.assertEqual(j['data']['returncode'], 0)
            self.assertEqual(j['data']['stderr'], '')
            self.assertEqual(j['data']['stdout'], 'foo\n')
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

    def test_stat(self):
        """stat data source"""
        d = wassail.data.stat('/tmp')
        if d.enabled():
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'stat')
            self.assertEqual(j['data']['path'], '/tmp')
            self.assertGreater(j['data']['atime'], 0)
            self.assertNotEqual(j['data']['inode'], 0)
            self.assertEqual(j['data']['uid'], 0)
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

    def test_stream(self):
        """stream data source"""
        d = wassail.data.stream()
        if d.enabled():
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'stream')
            self.assertGreaterEqual(j['data']['triad'], 1)
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

    def test_sysconf(self):
        """sysconf data source"""
        d = wassail.data.sysconf()
        if d.enabled():
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'sysconf')
            self.assertGreaterEqual(j['data']['nprocessors_onln'], 1)
            self.assertGreaterEqual(j['data']['nprocessors_conf'],
                                    j['data']['nprocessors_onln'])
            self.assertGreaterEqual(j['data']['page_size'], 1)
            self.assertGreaterEqual(j['data']['phys_pages'], 1)
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

    def test_sysctl(self):
        """sysctl data source"""
        d = wassail.data.sysctl()
        if d.enabled():
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'sysctl')
            self.assertGreater(j['data']['hw']['cpufrequency_max'], 0)
            self.assertGreater(j['data']['hw']['memsize'], 0)
            self.assertGreater(j['data']['hw']['packages'], 0)
            self.assertGreater(len(j['data']['kern']['version']), 0)
            self.assertGreater(j['data']['machdep']['cpu']['core_count'], 0)
            self.assertGreaterEqual(j['data']['vm']['loadavg']['load1'], 0)
            self.assertGreaterEqual(j['data']['vm']['swapusage']['xsu_total'],
                                    0)
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

    def test_sysinfo(self):
        """sysinfo data source"""
        d = wassail.data.sysinfo()
        if d.enabled():
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'sysinfo')
            self.assertGreater(j['data']['totalram'], 0)
            self.assertLess(j['data']['freeram'], j['data']['totalram'])
            self.assertGreaterEqual(j['data']['totalswap'], 0)
            self.assertLessEqual(j['data']['freeswap'], j['data']['totalswap'])
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

    def test_udev(self):
        """udev data source"""
        d = wassail.data.udev()
        if d.enabled():
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'udev')
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

    def test_umad(self):
        """umad data source"""
        d = wassail.data.umad()
        if d.enabled():
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'umad')
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

    def test_uname(self):
        """uname data source"""
        d = wassail.data.uname()
        if d.enabled():
            d.evaluate()
            s = str(d)
            j = json.loads(s)
            self.assertEqual(j['name'], 'uname')
            self.assertGreater(len(j['data']['machine']), 0)
            self.assertGreater(len(j['data']['nodename']), 0)
            self.assertGreater(len(j['data']['release']), 0)
            self.assertGreater(len(j['data']['sysname']), 0)
            self.assertGreater(len(j['data']['version']), 0)
        else:
            with self.assertRaises(RuntimeError):
                d.evaluate()

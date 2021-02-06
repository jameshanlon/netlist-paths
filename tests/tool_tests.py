import os
import subprocess
import unittest
import definitions as defs

class TestTool(unittest.TestCase):

    NETLIST_PATHS = os.path.join(defs.INSTALL_PREFIX, 'netlist-paths')

    def setUp(self):
        pass

    def run_np(self, args):
        command = [self.NETLIST_PATHS] + args
        proc = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if proc.returncode != 0:
            print('Error executing {}'.format(' '.join(proc.args)))
            print('Stdout:\n{}\nStderr:\n{}'.format(proc.stdout.decode('utf-8'),
                                                    proc.stderr.decode('utf-8')))
        return proc.returncode

    def test_netlist_paths_bin(self):
        self.assertTrue(os.path.exists(self.NETLIST_PATHS))

    def test_xml_output(self):
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'adder.sv')
        xml_path = os.path.join(defs.CURRENT_BINARY_DIR, 'adder.xml')
        if os.path.exists(xml_path):
            os.remove(xml_path)
        self.assertEqual(self.run_np(['--compile', test_path, '--output', xml_path]), 0)
        self.assertTrue(os.path.exists(xml_path))

    def test_adder(self):
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'adder.sv')
        self.assertEqual(self.run_np(['--compile', test_path]), 0)

    def test_dump_names(self):
        tests = ['adder.sv',
                 'counter.sv',
                 'dtypes.sv',
                 'empty_module.sv',
                 'ripple_carry_adder.sv', ]
        for test in tests:
            test_path = os.path.join(defs.TEST_SRC_PREFIX, test)
            self.assertEqual(self.run_np(['--compile', test_path, '--dump-names']), 0)

    def test_dump_any_path(self):
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'multiple_paths.sv')
        self.assertEqual(self.run_np(['--compile', test_path, '--from', 'in', '--to', 'out']), 0)

    def test_dump_all_paths(self):
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'multiple_paths.sv')
        self.assertEqual(self.run_np(['--compile', test_path, '--from', 'in', '--to', 'out', '--all-paths']), 0)

    def test_dump_fan_out_paths(self):
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'counter.sv')
        self.assertEqual(self.run_np(['--compile', test_path, '--from', 'counter.counter_q']), 0)

    def test_dump_fan_in_paths(self):
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'counter.sv')
        self.assertEqual(self.run_np(['--compile', test_path, '--to', 'counter.counter_q']), 0)


if __name__ == '__main__':
    unittest.main()

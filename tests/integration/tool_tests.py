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
        return (proc.returncode, proc.stdout.decode('utf-8'))

    def test_netlist_paths_bin(self):
        self.assertTrue(os.path.exists(self.NETLIST_PATHS))

    def test_xml_output(self):
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'adder.sv')
        xml_path = os.path.join(defs.CURRENT_BINARY_DIR, 'adder.xml')
        if os.path.exists(xml_path):
            os.remove(xml_path)
        returncode, _ = self.run_np(['--compile', test_path, '--output', xml_path])
        self.assertEqual(returncode, 0)
        self.assertTrue(os.path.exists(xml_path))

    def test_adder(self):
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'adder.sv')
        returncode, _ = self.run_np(['--compile', test_path])
        self.assertEqual(returncode, 0)

    def test_option_ignore_hierarchy_markers(self):
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'counter.sv')
        returncode, stdout = self.run_np(['--compile', test_path,
                                          '--from', 'counter/counter_q',
                                          '--to', 'o_count',
                                          '--ignore-hierarchy-markers'])
        self.assertEqual(returncode, 0)

    def test_dump_names(self):
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'counter.sv')
        returncode, stdout = self.run_np(['--compile', test_path, '--dump-names'])
        self.assertEqual(returncode, 0)
        self.assertEqual(len(stdout.split('\n')), 15)

    def test_dump_regs(self):
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'counter.sv')
        returncode, stdout = self.run_np(['--compile', test_path, '--dump-regs'])
        self.assertEqual(returncode, 0)
        self.assertEqual(len(stdout.split('\n')), 4)

    def test_dump_ports(self):
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'counter.sv')
        returncode, stdout = self.run_np(['--compile', test_path, '--dump-ports'])
        self.assertEqual(returncode, 0)
        self.assertEqual(len(stdout.split('\n')), 7)

    def test_dump_nets(self):
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'counter.sv')
        returncode, stdout = self.run_np(['--compile', test_path, '--dump-nets'])
        self.assertEqual(returncode, 0)
        self.assertEqual(len(stdout.split('\n')), 9)

    def test_dump_dtypes(self):
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'dtypes.sv')
        # Dump a table of all data types.
        returncode, stdout = self.run_np(['--compile', test_path, '--dump-dtypes'])
        self.assertEqual(returncode, 0)
        self.assertEqual(len(stdout.split('\n')), 18)
        # Filter by 'packed' regex.
        returncode, stdout = self.run_np(['--compile', test_path, '--dump-dtypes', 'packed', '--regex'])
        self.assertEqual(returncode, 0)
        self.assertEqual(len(stdout.split('\n')), 11)

    def test_dump_any_path(self):
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'multiple_paths.sv')
        returncode, _ = self.run_np(['--compile', test_path, '--from', 'in', '--to', 'out'])
        self.assertEqual(returncode, 0)

    def test_dump_all_paths(self):
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'multiple_paths.sv')
        returncode, _ = self.run_np(['--compile', test_path, '--from', 'in', '--to', 'out', '--all-paths'])
        self.assertEqual(returncode, 0)

    def test_dump_fan_out_paths(self):
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'counter.sv')
        returncode, _ = self.run_np(['--compile', test_path, '--from', 'counter.counter_q'])
        self.assertEqual(returncode, 0)

    def test_dump_fan_in_paths(self):
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'counter.sv')
        returncode, _ = self.run_np(['--compile', test_path, '--to', 'counter.counter_q'])
        self.assertEqual(returncode, 0)

    def test_compile_single_include(self):
        """
        Test passing an include path to Verilator
        """
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'single_include.sv')
        includes  = os.path.join(defs.TEST_SRC_PREFIX, "include_a")
        returncode, _ = self.run_np(['--compile', test_path, '-I', includes])
        self.assertEqual(returncode, 0)

    def test_compile_multiple_includes(self):
        """
        Test passing multiple include paths to Verilator
        """
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'multiple_includes.sv')
        includes  = [os.path.join(defs.TEST_SRC_PREFIX, "include_a"), os.path.join(defs.TEST_SRC_PREFIX, "include_b")]
        returncode, _ = self.run_np(['--compile', test_path, '-I'] + includes)
        self.assertEqual(returncode, 0)

    def test_compile_single_define(self):
        """
        Test passing a define to Verilator
        """
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'single_define.sv')
        defines   = 'EXPR_A=data_i'
        returncode, _ = self.run_np(['--compile', test_path, '-D', defines])
        self.assertEqual(returncode, 0)

    def test_compile_multiple_defines(self):
        """
        Test passing multiple defines, with and withput assignment to Verilator
        """
        test_path = os.path.join(defs.TEST_SRC_PREFIX, 'multiple_defines.sv')
        defines   = ['MY_DEFINE', 'EXPR_A=data_i', 'EXPR_B=data_o']
        returncode, _ = self.run_np(['--compile', test_path, '-D'] + defines)
        self.assertEqual(returncode, 0)

    def test_compile_multiple_files(self):
        """
        Test passing multiple files to compile with Verilator
        """
        paths = ['multiple_files.sv', 'include_a/include_a.sv', 'include_b/include_b.sv']
        full_paths = list(map(lambda p: os.path.join(defs.TEST_SRC_PREFIX, p), paths))
        returncode, _ = self.run_np(['--compile'] + full_paths)
        self.assertEqual(returncode, 0)


if __name__ == '__main__':
    unittest.main()

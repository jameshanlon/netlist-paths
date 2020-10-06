import os
import subprocess
import unittest
import definitions as defs

NETLIST_PATHS=os.path.join(defs.INSTALL_PREFIX, 'netlist_paths.py')

def run(args):
    return subprocess.check_call([NETLIST_PATHS]+args)

class TestTool(unittest.TestCase):

    def setUp(self):
        pass

    def test_netlist_paths_bin(self):
        self.assertTrue(os.path.exists(NETLIST_PATHS))

    def test_adder(self):
        self.assertEqual(run(['--compile', os.path.join(defs.TEST_SRC_PREFIX, 'adder.sv')]), 0)

if __name__ == '__main__':
    unittest.main()

import os
import pytest
import subprocess
import definitions as defs

NETLIST_PATHS=os.path.join(defs.INSTALL_PREFIX, 'netlist-paths')

def run(args):
    return subprocess.check_output([NETLIST_PATHS]+args)

def test_adder(tmpdir):
    run(['--compile', os.path.join(defs.TEST_SRC_PREFIX, 'adder.sv')])


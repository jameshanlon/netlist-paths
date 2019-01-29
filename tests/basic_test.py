import pytest
import definitions as defs
import subprocess
import os

NETLIST_PATHS=os.path.join(defs.INSTALL_PREFIX, 'netlist-paths')

def run(args):
    return subprocess.check_output([NETLIST_PATHS]+args)

def test_fsm(tmpdir):
    run(['--compile', os.path.join(defs.TEST_SRC_PREFIX, 'fsm.sv')])

import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

class ControlsTests(unittest.TestCase):
    def test_arrows_enter_space_and_legacy_numbers(self):
        with tempfile.TemporaryDirectory() as folder:
            exe = Path(folder)/'controls-test'
            subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-pedantic',
                            str(ROOT/'tests/controls_host.c'),'-o',str(exe)],check=True)
            subprocess.run([str(exe)],check=True,timeout=30)

if __name__ == '__main__':
    unittest.main()

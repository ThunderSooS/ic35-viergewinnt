import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

class GraphicsTests(unittest.TestCase):
    def test_pixel_renderer_and_dirty_updates(self):
        with tempfile.TemporaryDirectory() as folder:
            exe=Path(folder)/'graphics-test'
            subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-pedantic',
                            str(ROOT/'tests/graphics_host.c'),'-o',str(exe)],check=True)
            subprocess.run([str(exe)],check=True,timeout=30)

if __name__=='__main__':
    unittest.main()

import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

class AppNameTests(unittest.TestCase):
    def test_organizer_name_is_vier_gewinnt(self):
        source = (ROOT/'tools/build_game.py').read_text()
        self.assertIn("app[14:44]=b'Vier Gewinnt'.ljust(30,b'\\0')", source)

if __name__ == '__main__':
    unittest.main()

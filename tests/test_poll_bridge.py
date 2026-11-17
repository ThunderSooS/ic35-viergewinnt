import ast
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

class PollBridgeTests(unittest.TestCase):
    def test_poll_preserves_registers_and_returns_hl(self):
        tree = ast.parse((ROOT/'tools/build_game.py').read_text())
        function = next(n for n in tree.body if isinstance(n, ast.FunctionDef) and n.name == 'bridge_source')
        ns = {'SAVE':['push af','push bc','push de','push hl','push ix','push iy'],
              'RESTORE':['pop iy','pop ix','pop hl','pop de','pop bc','pop af','ret']}
        exec(compile(ast.Module(body=[function],type_ignores=[]),'bridge','exec'),ns)
        source = ns['bridge_source']()
        body = source.split('_fw_poll::\n',1)[1].strip().splitlines()
        self.assertEqual(body,['push af','push bc','push de','push ix','push iy',
                               'call 0x3c6d','pop iy','pop ix','pop de','pop bc','pop af','ret'])
        self.assertIn('call 0x3bec',source)

    def test_old_search_removed(self):
        source = (ROOT/'native/game.c').read_text()
        self.assertNotIn('static int search(',source)
        self.assertNotIn('static int evaluate(',source)
        self.assertIn('key=fw_poll();',source)
        self.assertNotIn('sign*dr',source)

if __name__ == '__main__':
    unittest.main()

import ast
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

class RuntimeAbiTests(unittest.TestCase):
    def test_c_compilation_and_link_match_runtime_abi(self):
        tree = ast.parse((ROOT/'tools/build_game.py').read_text())
        calls = []
        for node in ast.walk(tree):
            if not isinstance(node, ast.Call) or not isinstance(node.func, ast.Name):
                continue
            if node.func.id != 'run' or not node.args:
                continue
            if not isinstance(node.args[0], ast.Constant) or node.args[0].value != 'sdcc':
                continue
            args = [item.value if isinstance(item, ast.Constant) else None for item in node.args]
            self.assertIn('--sdcccall', args)
            self.assertEqual(args[args.index('--sdcccall')+1], '1')
            calls.append(args)
        self.assertEqual(len(calls), 2)

    def test_firmware_keeps_explicit_stack_abi(self):
        source = (ROOT/'native/game.c').read_text()
        self.assertIn('#define CALL0 __sdcccall(0)', source)
        declarations = [line for line in source.splitlines() if line.endswith(' CALL0;')]
        self.assertEqual(len(declarations), 4)
        for name in ['fw_clear', 'fw_text', 'fw_char', 'fw_key']:
            self.assertTrue(any(name+'(' in line for line in declarations))

if __name__ == '__main__':
    unittest.main()

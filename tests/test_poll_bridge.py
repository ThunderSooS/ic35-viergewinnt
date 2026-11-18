import ast
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

def source():
    tree=ast.parse((ROOT/'tools/build_game.py').read_text())
    function=next(n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name=='bridge_source')
    ns={'SAVE':['push af','push bc','push de','push hl','push ix','push iy'],
        'RESTORE':['pop iy','pop ix','pop hl','pop de','pop bc','pop af','ret']}
    exec(compile(ast.Module(body=[function],type_ignores=[]),'bridge','exec'),ns)
    return ns['bridge_source']()

class PollBridgeTests(unittest.TestCase):
    def test_poll_preserves_registers_and_returns_hl(self):
        body=source().split('_fw_poll::\n',1)[1].strip().splitlines()
        self.assertEqual(body,['push af','push bc','push de','push ix','push iy',
                               'call 0x3c6d','pop iy','pop ix','pop de','pop bc','pop af','ret'])
        self.assertIn('call 0x3bec',source())

    def test_fill_bridge_layout(self):
        body=source().split('_fw_fill::\n',1)[1].split('_fw_key::\n',1)[0].splitlines()
        expected=['push af','push bc','push de','push hl','push ix','push iy','ld ix,#0','add ix,sp']
        for offset in [22,20,18,16,14]:
            expected+=['ld l,'+str(offset)+'(ix)','ld h,'+str(offset+1)+'(ix)','push hl']
        expected+=['call 0x3799']+['pop af']*5+['pop iy','pop ix','pop hl','pop de','pop bc','pop af','ret']
        self.assertEqual(body,expected)
        self.assertIn('short h) CALL0;',(ROOT/'native/graphics.h').read_text())

    def test_old_search_removed(self):
        text=(ROOT/'native/game.c').read_text()
        self.assertNotIn('static int search(',text)
        self.assertNotIn('static int evaluate(',text)
        self.assertIn('key=fw_poll();',text)
        self.assertNotIn('sign*dr',text)

if __name__=='__main__':unittest.main()

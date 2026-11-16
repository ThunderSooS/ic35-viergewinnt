import importlib.util
import re
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
import build_game

class GameTests(unittest.TestCase):
    def test_host_game_and_ai(self):
        with tempfile.TemporaryDirectory() as folder:
            exe=Path(folder)/'game-test'
            subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-pedantic',
                            str(ROOT/'tests/game_host.c'),'-o',str(exe)],check=True)
            subprocess.run([str(exe)],check=True)

    def test_bridge_stack_and_registers(self):
        source=build_game.bridge_source()
        for name,args,address,expected in [
            ('fw_clear',[],0x3799,[0,0,0,239,160]),
            ('fw_text',[0x4567,12,34],0x3b0d,[1,0x4567,12,34]),
            ('fw_char',[88,56,78],0x389f,[1,88,56,78]),
            ('fw_key',[],0x3bec,[])]:
            body=source.split('_'+name+'::\n',1)[1].split('::',1)[0].splitlines()
            mem=bytearray(65536);sp=0xf000
            def put(at,value): mem[at:at+2]=value.to_bytes(2,'little')
            def get(at): return int.from_bytes(mem[at:at+2],'little')
            put(sp,0x7777)
            for i,value in enumerate(args): put(sp+2+2*i,value)
            regs=dict(af=0x1234,bc=0x2345,de=0x3456,hl=0x4567,ix=0x5678,iy=0x6789)
            before=regs.copy();calls=0;returned=False
            for raw in body:
                op=raw.strip()
                if returned: break
                if op.startswith('push '): sp-=2;put(sp,regs[op[5:]])
                elif op.startswith('pop '): regs[op[4:]]=get(sp);sp+=2
                elif op=='ld ix,#0': regs['ix']=0
                elif op=='add ix,sp': regs['ix']+=sp
                elif op.startswith('ld hl,#'): regs['hl']=int(op[7:])
                elif re.fullmatch(r'ld [lh],[0-9]+\(ix\)',op):
                    part=op[3];offset=int(op[5:].split('(')[0]);v=mem[regs['ix']+offset]
                    regs['hl']=((regs['hl']&0xff00)|v) if part=='l' else ((v<<8)|(regs['hl']&255))
                elif op.startswith('call '):
                    self.assertEqual(int(op[5:],16),address)
                    self.assertEqual([get(sp+2*i) for i in range(len(expected))],expected)
                    regs={key:0xa55a for key in regs}
                    if name=='fw_key': regs['hl']=0x1234
                    calls+=1
                elif op=='ret': self.assertEqual(get(sp),0x7777);sp+=2;returned=True
                else: self.fail('Unexpected bridge instruction: '+op)
            if name=='fw_key': before['hl']=0x1234
            self.assertTrue(returned);self.assertEqual(calls,1)
            self.assertEqual(sp,0xf002);self.assertEqual(regs,before)

    def test_hex_validation(self):
        def record(address,kind,data):
            raw=bytes([len(data),address>>8,address&255,kind])+data
            return ':'+(raw+bytes([-sum(raw)&255])).hex()
        good=record(0x4000,0,b'\xc9')+'\n'+record(0,1,b'')
        bank,size=build_game.read_image(good)
        self.assertEqual(size,1);self.assertEqual(bank[0],0xc9);self.assertEqual(len(bank),16384)
        for bad in [good+'\n'+good,record(0x8000,0,b'x')+'\n'+record(0,1,b''),good[:-2]+'01']:
            with self.assertRaises(ValueError): build_game.read_image(bad)

if __name__=='__main__':
    unittest.main()

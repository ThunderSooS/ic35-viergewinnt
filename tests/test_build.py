import importlib.util
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('builder', ROOT / 'tools/build_test.py')
builder = importlib.util.module_from_spec(spec)
spec.loader.exec_module(builder)

class Machine:
    """Only the emitted instruction subset; firmware is a destructive mock."""
    def __init__(self, code, keys):
        self.mem = bytearray(65536)
        self.mem[0x4000:0x4000+len(code)] = code
        self.r = {name: value for name,value in zip(['af','bc','de','hl','ix','iy'],
                                                   [0x1234,0x2345,0x3456,0x4567,0x5678,0x6789])}
        self.initial = self.r.copy()
        self.pc=0x4000; self.sp=0xf000; self.lowest=self.sp
        self.keys=iter(keys); self.characters=[]; self.texts=[]; self.reads=0
        self.push(0xffff)

    def push(self, value):
        self.sp-=2; self.lowest=min(self.lowest,self.sp)
        self.mem[self.sp:self.sp+2]=value.to_bytes(2,'little')

    def pop(self):
        value=int.from_bytes(self.mem[self.sp:self.sp+2],'little'); self.sp+=2
        return value

    def word(self):
        value=int.from_bytes(self.mem[self.pc:self.pc+2],'little'); self.pc+=2
        return value

    def byte(self):
        value=self.mem[self.pc]; self.pc+=1; return value

    def a(self):
        return self.r['af']>>8

    def set_a(self, value, flags=None):
        self.r['af']=((value&255)<<8) | ((self.r['af']&255) if flags is None else flags)

    def firmware(self):
        address=self.pc
        def arg(i):
            start=self.sp+2+2*i
            return int.from_bytes(self.mem[start:start+2],'little')
        key=None
        if address==0x3799:
            assert [arg(i) for i in range(5)]==[0,0,0,239,160]
        elif address==0x3b0d:
            assert arg(0)==1 and arg(2)==4 and arg(3) in [4,28,52,76]
            start=arg(1); end=self.mem.index(0,start,start+128)
            self.texts.append(bytes(self.mem[start:end]).decode('ascii'))
        elif address==0x389f:
            assert arg(0)==1 and arg(2) in [4,24,44,64] and arg(3)==104
            self.characters.append(chr(arg(1)))
        elif address==0x3bec:
            key=next(self.keys); self.reads+=1
        else:
            raise AssertionError('Unknown firmware address')
        for name in self.r: self.r[name]=0xa55a
        if key is not None: self.r['hl']=key
        self.pc=self.pop()

    def run(self):
        for step in range(30000):
            if self.pc==0xffff:
                assert self.sp==0xf000 and self.r==self.initial
                return
            if self.pc in [0x3799,0x3b0d,0x389f,0x3bec]:
                self.firmware(); continue
            op=self.byte()
            if op in [0xdd,0xfd]:
                name='ix' if op==0xdd else 'iy'; sub=self.byte()
                if sub==0xe5: self.push(self.r[name])
                elif sub==0xe1: self.r[name]=self.pop()
                else: raise AssertionError('Unsupported indexed instruction')
            elif op in [0xf5,0xc5,0xd5,0xe5]:
                self.push(self.r[{0xf5:'af',0xc5:'bc',0xd5:'de',0xe5:'hl'}[op]])
            elif op in [0xf1,0xc1,0xd1,0xe1]:
                self.r[{0xf1:'af',0xc1:'bc',0xd1:'de',0xe1:'hl'}[op]]=self.pop()
            elif op in [0x21,0x01,0x11]:
                self.r[{0x21:'hl',0x01:'bc',0x11:'de'}[op]]=self.word()
            elif op==0x26: self.r['hl']=(self.byte()<<8)|(self.r['hl']&255)
            elif op==0x7c: self.set_a(self.r['hl']>>8)
            elif op==0x7d: self.set_a(self.r['hl']&255)
            elif op==0x6f: self.r['hl']=(self.r['hl']&0xff00)|self.a()
            elif op==0x0f:
                a=self.a(); self.set_a((a>>1)|((a&1)<<7),(self.r['af']&0xfe)|(a&1))
            elif op in [0xe6,0xc6,0xfe,0xb7]:
                a=self.a(); n=a if op==0xb7 else self.byte()
                if op==0xe6: result=a&n; carry=0
                elif op==0xc6: result=a+n; carry=int(result>255)
                elif op==0xfe: result=a-n; carry=int(result<0)
                else: result=a; carry=0
                flags=(0x40 if (result&255)==0 else 0)|carry
                if op==0xfe: self.set_a(a,flags)
                else: self.set_a(result,flags)
            elif op==0xcd:
                target=self.word(); self.push(self.pc); self.pc=target
            elif op==0xc9: self.pc=self.pop()
            elif op in [0xc3,0xc2,0xca,0xda]:
                target=self.word(); flags=self.r['af']&255
                if op==0xc3 or (op==0xc2 and not flags&64) or (op==0xca and flags&64) or (op==0xda and flags&1): self.pc=target
            else:
                raise AssertionError('Unsupported opcode %02x at %04x' % (op,self.pc-1))
        raise AssertionError('Instruction limit reached')

class BuildTests(unittest.TestCase):
    def setUp(self):
        self.code,self.labels=builder.assemble((ROOT/'test001.asm').read_text())

    def test_package(self):
        app=builder.package(self.code)
        self.assertEqual(len(app),81988)
        self.assertEqual(self.labels['entry'],0x4000)
        self.assertEqual(app[68:68+len(self.code)],self.code)
        self.assertTrue(all(v==255 for v in app[68+len(self.code):]))

    def test_keys_and_exit_with_clobbering_firmware(self):
        for quit_key in [ord('q'),ord('Q')]:
            m=Machine(self.code,[0x31,0x37,0x1234,quit_key]); m.run()
            self.assertEqual(''.join(m.characters),'0000003100371234')
            self.assertEqual(m.reads,4)
            self.assertEqual(m.texts.count('IC35 TEST 001'),4)
            self.assertLess(0xf000-m.lowest,128)

    def test_reject_unknown_instruction(self):
        with self.assertRaises(ValueError): builder.assemble('unknown')

if __name__=='__main__':
    unittest.main()

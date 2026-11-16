import hashlib
import json
import os
import re
import subprocess
from pathlib import Path
from build_test import package

ROOT = Path(__file__).resolve().parents[1]
WORK = ROOT / 'build' / 'game-work'
OUT = ROOT / 'build' / 'IC35-VIER-KI-001'
SAVE = ['push af','push bc','push de','push hl','push ix','push iy']
RESTORE = ['pop iy','pop ix','pop hl','pop de','pop bc','pop af','ret']

def bridge_source():
    lines=['.module game_bridge','.area _CODE']
    for name,address,indices in [('fw_clear','0x3799',None),
                                  ('fw_text','0x3b0d',[2,1,0]),
                                  ('fw_char','0x389f',[2,1,0])]:
        lines+=['_'+name+'::']+SAVE
        if indices is None:
            for value in [160,239,0,0,0]: lines+=['ld hl,#'+str(value),'push hl']
            words=5
        else:
            lines+=['ld ix,#0','add ix,sp']
            for index in indices:
                offset=14+2*index
                lines+=['ld l,'+str(offset)+'(ix)','ld h,'+str(offset+1)+'(ix)','push hl']
            lines+=['ld hl,#1','push hl']; words=4
        lines+=['call '+address]+['pop af']*words+RESTORE
    lines+=['_fw_key::','push af','push bc','push de','push ix','push iy',
            'call 0x3bec','pop iy','pop ix','pop de','pop bc','pop af','ret']
    return '\n'.join(lines)+'\n'

ENTRY = '''.module game_entry
.globl _game_main
.area _ENTRY (ABS)
.org 0x4000
_entry::
 push af
 push bc
 push de
 push hl
 push ix
 push iy
 call _game_main
 pop iy
 pop ix
 pop hl
 pop de
 pop bc
 pop af
 ret
.area _CODE
'''

def read_image(text):
    image=bytearray([255]*16384); used=set(); ended=False
    for line in text.splitlines():
        if not line.strip(): continue
        if ended or not line.startswith(':'): raise ValueError('Invalid HEX framing')
        record=bytes.fromhex(line[1:])
        if len(record)<5 or len(record)!=record[0]+5 or sum(record)&255:
            raise ValueError('Invalid HEX record')
        size=record[0]; address=int.from_bytes(record[1:3],'big'); kind=record[3]
        if kind==1:
            if size or address: raise ValueError('Invalid EOF')
            ended=True
        elif kind==0:
            for i,value in enumerate(record[4:-1]):
                target=address+i
                if not 0x4000<=target<0x8000 or target in used:
                    raise ValueError('Code overlap or out of first ROM bank')
                image[target-0x4000]=value; used.add(target)
        else: raise ValueError('Unsupported HEX record type')
    if not ended or 0x4000 not in used: raise ValueError('Missing entry or EOF')
    return bytes(image),len(used)

def run(*args):
    subprocess.run([str(a) for a in args],cwd=ROOT,check=True)

def main():
    WORK.mkdir(parents=True,exist_ok=True); OUT.mkdir(parents=True,exist_ok=True)
    (WORK/'entry.s').write_text(ENTRY)
    (WORK/'bridge.s').write_text(bridge_source())
    for name in ['entry','bridge']:
        run('sdasz80','-plosgff','-o',WORK/(name+'.rel'),WORK/(name+'.s'))
    run('sdcc','-mz80','--sdcccall','0','--std-c89','--opt-code-size',
        '-c',ROOT/'native/game.c','-o',WORK/'game.rel')
    run('sdcc','-mz80','--sdcccall','0','--no-std-crt0',
        '--code-loc','0x4200','--data-loc','0x8000','-Wl-m',
        WORK/'entry.rel',WORK/'bridge.rel',WORK/'game.rel','-o',WORK/'game.ihx')
    mapping=(WORK/'game.map').read_text()
    for section in ['DATA','INITIALIZED','BSS']:
        for value in re.findall(r'\b([0-9A-Fa-f]{8})\s+l__'+section+r'\b',mapping):
            if int(value,16): raise ValueError('Unexpected global RAM allocation: '+section)
    bank,used=read_image((WORK/'game.ihx').read_text())
    if not bank.startswith(bytes.fromhex('f5c5d5e5dde5fde5')):
        raise ValueError('Missing entry register preservation')
    app=bytearray(package(bank)); app[14:44]=b'VIER KI 001'.ljust(30,b'\0')
    (OUT/'VIER.APP').write_bytes(app)
    (OUT/'BUILD-INFO.json').write_text(json.dumps({
        'version':'VIER KI 001','status':'NOT_DEVICE_VERIFIED',
        'commit':os.environ.get('GITHUB_SHA','local'),
        'bytes':len(app),'sha256':hashlib.sha256(app).hexdigest(),
        'source_sha256':hashlib.sha256((ROOT/'native/game.c').read_bytes()).hexdigest(),
        'linked_bytes':used,'state':'stack-local Game; no RAM bank switch',
        'firmware_bridge':'all general register pairs preserved; GetKey returns HL',
        'limitations':['Host tests do not execute IC35 firmware.',
                       'C stack usage and linked Z80 game require device verification.']
    },indent=2)+'\n')
    (OUT/'START.txt').write_text(
        'VIER KI 001 - EXPERIMENTAL NATIVE APP, NOT BASIC\n'
        'Keep the working TEST001.APP and back up organizer data and card contents.\n'
        'Copy VIER.APP to IC35/APP/VIER.APP; do not replace TEST001.APP.\n'
        'You are X, AI is O. 1-7: column. N: restart. Q: exit.\n'
        'AI: win immediately, otherwise block an immediate win, otherwise prefer center.\n'
        'No sound, animation or graphical board. Header: VIER KI 001.\n'
        'Stop on a hang; do not repeatedly restart. Host tests are not firmware tests.\n',encoding='ascii')
    for name in ['game.map','game.ihx','game.asm','entry.s','bridge.s']:
        (OUT/name).write_bytes((WORK/name).read_bytes())
    (OUT/'compiler-version.txt').write_text(subprocess.check_output(['sdcc','--version'],text=True))
    print(json.dumps({'app':'VIER.APP','bytes':len(app),'sha256':hashlib.sha256(app).hexdigest()}))

if __name__=='__main__':
    main()

import ast
import hashlib
import json
import re
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'build' / 'IC35-TEST-001'
ORIGIN = 0x4000

FIXED = {
    'push af': b'\xf5', 'push bc': b'\xc5', 'push de': b'\xd5',
    'push hl': b'\xe5', 'push ix': b'\xdd\xe5', 'push iy': b'\xfd\xe5',
    'pop af': b'\xf1', 'pop bc': b'\xc1', 'pop de': b'\xd1',
    'pop hl': b'\xe1', 'pop ix': b'\xdd\xe1', 'pop iy': b'\xfd\xe1',
    'ret': b'\xc9', 'ld a,h': b'\x7c', 'ld a,l': b'\x7d',
    'ld l,a': b'\x6f', 'or a': b'\xb7', 'rrca': b'\x0f',
}

def assemble(source):
    code = bytearray()
    labels = {}
    fixups = []
    for number, raw in enumerate(source.splitlines(), 1):
        line = raw.split(';', 1)[0].strip()
        if not line:
            continue
        if line.endswith(':'):
            name = line[:-1]
            if not re.fullmatch(r'[a-z_]+', name) or name in labels:
                raise ValueError('Invalid or duplicate label on line %d' % number)
            labels[name] = ORIGIN + len(code)
        elif line in FIXED:
            code.extend(FIXED[line])
        elif line.startswith('db '):
            text = ast.literal_eval(line[3:])
            code.extend(text.encode('ascii') + b'\0')
        else:
            match = re.fullmatch(r'(ld (?:hl|bc|de|h),|call |jp (?:nz,|z,|c,)?|cp |and |add a,)([a-z_]+|[0-9]+)', line)
            if not match:
                raise ValueError('Unsupported instruction on line %d: %s' % (number, line))
            op, arg = match.groups()
            table = {'ld hl,': (0x21,2), 'ld bc,': (0x01,2), 'ld de,': (0x11,2),
                     'ld h,': (0x26,1), 'call ': (0xcd,2), 'jp ': (0xc3,2),
                     'jp nz,': (0xc2,2), 'jp z,': (0xca,2), 'jp c,': (0xda,2),
                     'cp ': (0xfe,1), 'and ': (0xe6,1), 'add a,': (0xc6,1)}
            opcode, width = table[op]
            code.append(opcode)
            if arg.isdigit():
                code.extend(int(arg).to_bytes(width, 'little'))
            elif width == 2:
                fixups.append((len(code), arg))
                code.extend(b'\0\0')
            else:
                raise ValueError('Byte immediate must be numeric')
    for offset, name in fixups:
        code[offset:offset+2] = struct.pack('<H', labels[name])
    if not code or len(code) > 16384:
        raise ValueError('Program exceeds the first 16 KiB bank')
    return bytes(code), labels

def package(code):
    if not code or len(code) > 16384:
        raise ValueError('Invalid code length')
    header = (bytes.fromhex('80f60040040000400000ffff1e00')
              + b'IC35 TEST 001'.ljust(30, b'\0')
              + bytes.fromhex('1600') + bytes(22))
    return header + code + b'\xff' * (81920 - len(code))

def main():
    code, labels = assemble((ROOT / 'test001.asm').read_text())
    app = package(code)
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'TEST001.APP').write_bytes(app)
    (OUT / 'BUILD-INFO.json').write_text(json.dumps({
        'version': 'IC35 TEST 001', 'status': 'EXPERIMENTAL_NOT_DEVICE_VERIFIED',
        'app_bytes': len(app), 'code_bytes': len(code),
        'sha256': hashlib.sha256(app).hexdigest(), 'labels': labels,
        'source': 'test001.asm; assembled by the restricted assembler in tools/build_test.py',
        'sdk_binaries_used': False, 'basic_runtime_required': False,
        'limitations': ['APP header follows the previously observed project layout.',
                       'Firmware addresses and argument ABI need device verification.',
                       'Host simulation does not execute IC35 firmware.']
    }, indent=2) + '\n')
    (OUT / 'START.txt').write_text(
        'IC35 TEST 001 - EXPERIMENTAL KEYBOARD TEST\n'
        'Back up organizer data and card contents before any device test.\n'
        'Copy TEST001.APP to IC35/APP/TEST001.APP. Do not overwrite another APP.\n'
        'Expected: IC35 TEST 001, Q: Ende, VOR GETKEY and initial code 0000.\n'
        'A key updates the four hexadecimal digits. Q or q returns to the organizer.\n'
        'No game, animation, sound, BASIC runtime or private SDK binaries.\n'
        'VOR GETKEY is a phase label, not proof that GetKey has been entered.\n'
        'On rejection or a hang, stop; do not repeatedly restart the test.\n', encoding='ascii')
    print(json.dumps({'app': 'TEST001.APP', 'bytes': len(app), 'sha256': hashlib.sha256(app).hexdigest()}))

if __name__ == '__main__':
    main()

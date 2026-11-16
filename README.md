# IC35 TEST 001

Fresh native IC35 bring-up: text, hexadecimal key codes, and Q/q to return to the organizer. No game, graphics board, sound, animation or BASIC runtime.

## Build

Run `python3 -m unittest discover -s tests -v`, then `python3 tools/build_test.py`. No downloads, compiler packages, SDK binaries or apilib.obj are used. The deliberately restricted assembler accepts only the instructions needed by test001.asm.

One workflow creates `IC35-TEST-001-NATIVE-APP`, containing TEST001.APP, BUILD-INFO.json and START.txt. Copy only TEST001.APP to IC35/APP/TEST001.APP, without replacing existing apps.

## Status and limits

EXPERIMENTAL, NOT DEVICE VERIFIED. Back up organizer data and card contents before a device test. Stop on rejection or a hang; do not repeatedly restart it. Expected: IC35 TEST 001, Q: Ende, VOR GETKEY, KEY HEX and initial digits 0000. Keys update the digits; Q/q exits. VOR GETKEY is only a phase label, not proof that firmware keyboard code has been entered.

Host tests execute the emitted instruction subset with destructive firmware mocks. They do not emulate IC35 firmware, interrupts, loader or display. Firmware addresses and word-stack arguments, and the previously observed 68-byte APP header/five-bank layout, still need device verification. No global RAM or RAM-bank switch is used.

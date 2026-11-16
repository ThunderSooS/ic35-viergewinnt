# Native IC35 bring-up and Vier Gewinnt

## Confirmed baseline

TEST001.APP (source 2329688) was confirmed working by the user on the IC35. Its assembly source, builder and tests remain unchanged. It displays key codes and exits with Q/q. Keep the working binary separately.

## New stage: VIER KI 001

Native VIER.APP: human X versus automatic AI O, text board, columns 1-7, N to restart, Q to exit. AI first takes an immediate win, otherwise blocks one, otherwise prefers the center. No sound, animation, bitmap board or BASIC runtime.

Game state is a 44-byte stack-local structure, plus normal call frames; no global game RAM or RAM-bank switch. The C-to-firmware bridge preserves general register pairs, except the returned HL key value. This bridge adapts the tested assembly approach, but the linked C game is NOT yet device verified.

## One workflow, two distinct artifacts

- IC35-TEST-001-NATIVE-APP: unchanged baseline.
- IC35-VIER-KI-001-NATIVE-APP: new VIER.APP plus build identification and linker/assembly diagnostics.

Back up organizer data and card contents. Copy the game to IC35/APP/VIER.APP without replacing TEST001.APP. Expected game header: VIER KI 001. Stop on a hang; do not repeatedly restart.

## Build and limitations

Run python3 -m unittest discover -s tests -v. Build the baseline with python3 tools/build_test.py. The game additionally needs the open-source SDCC package: python3 tools/build_game.py. No proprietary SDK binaries or apilib.obj are uploaded or used.

Host tests cover game logic, AI, UI mocks, the bridge stack model and HEX validation. They do not execute IC35 firmware or validate the complete linked Z80 game. Device compatibility is not implied by a green build. APP packaging follows the observed header/five-bank layout from the baseline.

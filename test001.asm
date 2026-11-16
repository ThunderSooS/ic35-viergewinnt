; IC35 TEST 001: no C runtime, globals, RAM-bank switch, sound or animation.
; Firmware addresses and word-stack ABI remain device-test assumptions.
entry:
 push af
 push bc
 push de
 push hl
 push ix
 push iy
 ld hl,0
redraw:
 call draw
wait_key:
 call getkey
 ld a,h
 or a
 jp nz,redraw
 ld a,l
 cp 81
 jp z,exit
 cp 113
 jp nz,redraw
exit:
 pop iy
 pop ix
 pop hl
 pop de
 pop bc
 pop af
 ret

draw:
 push hl
 call clear
 ld hl,title
 ld bc,4
 ld de,4
 call text
 ld hl,instructions
 ld de,28
 call text
 ld hl,stage
 ld de,52
 call text
 ld hl,caption
 ld de,76
 call text
 pop hl
 call hexkey
 ret

hexkey:
 ld de,104
 ld bc,4
 ld a,h
 rrca
 rrca
 rrca
 rrca
 call digit
 ld bc,24
 ld a,h
 call digit
 ld bc,44
 ld a,l
 rrca
 rrca
 rrca
 rrca
 call digit
 ld bc,64
 ld a,l
 call digit
 ret

digit:
 and 15
 cp 10
 jp c,decimal
 add a,7
decimal:
 add a,48
 call character
 ret

; All drawing wrappers save every general register used by the test.
; Arguments are pushed right-to-left as 16-bit words, caller cleanup.
clear:
 push af
 push bc
 push de
 push hl
 push ix
 push iy
 ld hl,160
 push hl
 ld hl,239
 push hl
 ld hl,0
 push hl
 push hl
 push hl
 call 14233
 pop af
 pop af
 pop af
 pop af
 pop af
 pop iy
 pop ix
 pop hl
 pop de
 pop bc
 pop af
 ret

text:
 push af
 push bc
 push de
 push hl
 push ix
 push iy
 push de
 push bc
 push hl
 ld hl,1
 push hl
 call 15117
 pop af
 pop af
 pop af
 pop af
 pop iy
 pop ix
 pop hl
 pop de
 pop bc
 pop af
 ret

character:
 push af
 push bc
 push de
 push hl
 push ix
 push iy
 push de
 push bc
 ld l,a
 ld h,0
 push hl
 ld hl,1
 push hl
 call 14495
 pop af
 pop af
 pop af
 pop af
 pop iy
 pop ix
 pop hl
 pop de
 pop bc
 pop af
 ret

getkey:
 push af
 push bc
 push de
 push ix
 push iy
 call 15340
 pop iy
 pop ix
 pop de
 pop bc
 pop af
 ret

title:
 db "IC35 TEST 001"
instructions:
 db "Q: Ende"
stage:
 db "VOR GETKEY"
caption:
 db "KEY HEX"

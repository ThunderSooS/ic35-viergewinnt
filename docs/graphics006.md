# VIER KI 006: integrated graphics

The active game now includes graphics.h and calls gfx_draw() and gfx_menu().
The five-word CALL0 fw_fill bridge uses the existing FillBlock API at 0x3799.
AI, game rules and arrow/Enter/Space controls remain unchanged.

Build artifact: IC35-VIER-KI-006-APP, containing VIER.APP.
The build checks for an actual renderer call and hashes game.c and graphics.h.
Game/control tests now read pixels, not the old text layout. The host-only
framebuffer in tests/pixel_mock.h is not compiled into the APP.

GfxView lives on the stack. It is invalidated after menus; normal moves and
selection changes update only dirty regions. Status strings are immutable.
No SDK binary, external image, animation or sound is added.

Keep working VIER KI 005 and TEST001.APP. GitHub build success and actual IC35
font metrics, speed, stack use and flicker remain unverified until tested.

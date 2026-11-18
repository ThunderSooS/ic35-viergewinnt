# VIER KI 006 graphics component (staged)

This commit adds a tested renderer, not a new device APP. The active game,
firmware bridge, AI, controls, TEST001 and build/artifact version remain 005.
Do not treat the existing 005 artifact as a graphics build.

## Renderer contract

Include native/graphics.h after the Game definition, CALL0 firmware declarations
and level_name(). Provide fw_fill(color,x,y,width,height) with the explicit CALL0
ABI. It must use the existing FillBlock firmware API with preserved registers;
no private SDK object is to be linked or uploaded.

Keep one GfxView on the application stack. Call gfx_reset() before its first use
and after a menu or other operation overwrites the screen. Pass valid levels
1..3 and selected columns 0..6 to gfx_draw(). The Game is never modified.

The board uses x=8..134, y=28..136, 18-pixel cells. The selected column has a
triangle above it. Human stones are black, AI stones have a white center, empty
cells have a small dot. Four winning cells get inset square outlines. The side
panel shows difficulty, turn/result and a legend; a draw is shown as Remis.
The footer and all drawing rectangles stay inside 239 by 160 pixels.

gfx_draw() caches cells, win masks, selection and status, not a framebuffer.
Unchanged frames do not draw. Selection changes only redraw the arrow band;
a stone change repaints its cell. gfx_menu(selected,previous) updates only the
selection frame when previous is nonzero. Use previous=0 for a full menu draw.

## Before device release

Integrate the renderer into game_main and the menu, add and test the fw_fill
bridge, adapt existing screen mocks to pixels, and bump the build to 006.
Run the complete suite and inspect the actual SDCC/Z80 output and stack usage.
Speed, real font metrics and flicker are not yet verified on the IC35.
Keep the device-confirmed 005 build as fallback. No animation or sound is added.

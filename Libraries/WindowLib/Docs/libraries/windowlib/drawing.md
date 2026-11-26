# Drawing and Speed

A window is painted when something changed, at most 60 times a second.  A paint lays the window out only if its
layout was invalidated or its size changed, then draws only what was damaged, then presents only that.  Everything
that changes how a component looks has to say so; [`CWindow::SetVerifyRedraw`] checks that it did.

## A paint

[`CWindow::Paint`] does, in order:

1. **Lay out if needed** ([`CWindow::Layout`]): the layout is kept between paints unless it was invalidated or the
   client area changed size.  A layout runs up to three passes while components ask for another
   ([`CWindow::LayoutAgain`]: a word wrapped text area whose height changed with its width).
2. **Prepare** ([`CComponent::PrepareDraw`]): walk the window, passing the clip down, and work out where each
   component will draw this paint ([`CComponent::PrepareArea`]: a canvas's visible part, a popup's background).
   Where a component drew last time and will draw now are both damaged if they differ: it moved, resized, appeared
   or went.  Containers draw nothing themselves, so their drawn area is empty.
3. **Decide** ([`CWindow::CanPaintPartially`]): the whole window is drawn when partial redraw is off, on the first
   show or a resize, or when the window's background isn't one solid colour ([`CClearCanvasDraw`]), since nothing
   else would clear a damaged part before what is over it is drawn again.
4. **Composite** ([`CWindow::Composite`]): each damaged rectangle in turn is drawn with the window's canvas clipped
   to it: the background, then everything over it in order.  A canvas whose content didn't change copies its
   retained native canvas instead of drawing again.
5. **Verify**, if on ([`CWindow::VerifyRedraw`], below).
6. **The caret** ([`CWindow::PaintCaret`]), over what was drawn.
7. **Present** ([`CWindow::PresentDamage`]): the damaged parts and the caret, or the whole window.

The damage is a [`CDamageArea`]: up to eight rectangles, a ninth merging them all into their bounds, so it never
misses anything, it only grows.

## Saying what changed

| Call | Means | For |
|---|---|---|
| [`CComponent::Invalidate`] | The component and its children draw again. | A look that changed without a size change. |
| [`CComponent::InvalidateLayout`] | Lay out again before the next paint, and draw the component and its children again. | Leaves whose look and size change: a label's text, an icon, a text area's style setters. |
| [`CComponent::LayoutChanged`] | Lay out again, but don't redraw the component: it invalidates what changed itself. | Containers that only move things; a text area that redraws only its changed rows; a code editor's marks and bars. |
| [`CComponent::ChildrenChanged`] | Lay out again; nothing drawn changed. | Adding and removing children: what appears or goes is found by the next prepare. |
| [`CCanvas::InvalidateArea`] | Draw part of the canvas again. | A text area's changed rows. |
| [`CWindow::RequestPaint`] | Paint soon, drawing only what was damaged. | Pointer moves; state found at the paint ([`CTextArea::PrepareArea`]). |
| [`CCanvas::SetDrawnEveryPaint`] | The canvas draws all of itself every paint. | Draws that read state nobody announces: LogiCim's circuit, the emulator's screen, WindowInputTester's device views. |

The rules that keep partial redraw right:

- A component whose look changes must invalidate.  [`CComponent::InvalidateLayout`] also marks the look changed.
- A canvas draw ([`CCanvasDraw`]) can't see its canvas, so whoever changes a draw's state invalidates the canvas:
  [`CClearCanvasDraw::SetColour`], [`CImageCanvasDraw::SetCel`], [`CImageBorderContainer::SetBorderState`].
  [`CTabComponent::UpdateLook`] once missed it, and a tab's hover never redrew.
- Invalidating a container or a window marks every canvas under it wholly dirty.  When the look didn't change, use
  [`CComponent::LayoutChanged`] or [`CWindow::RequestPaint`].
- Moved components need nothing: the prepare compares drawn areas.
- In tests, damage is checked against [`CComponent::GetActualBounds`], not the drawn area (a container's is empty).

## Which input lays the window out

Input can change anything, so the window is painted after it.  Keys, buttons, commands and the wheel also lay it out,
except:

- a key typing a character ([`CComponentInput::IsTyping`]): it only changes text, and the text area lays itself out
  again if its size changed;
- input taken by actions that keep the layout ([`CInputAction::SetKeepsLayout`]): the text editing commands, so
  moving the caret doesn't lay the window out;
- a key's release that nothing took.

A focus change still lays out.  Pointer moves and the mouse's raw movement ([`WIET_Delta`]) only request a paint; a
component that changes as the pointer moves invalidates itself.  Shift's press is held back and sent with the next
key, so the first capital after Shift still lays out once.

Debug builds lay the window out at every paint anyway and log each component that moved without its layout being
invalidated ("moved from ... but the layout wasn't invalidated").  Tests that count layouts turn this off
([`CWindow::SetCheckLayoutCache`]); it also hides bugs that only show in Release, where the kept layout is used.

## Canvases

A [`CCanvas`] keeps a native canvas of only its visible part: its bounds cut to its clip
([`CCanvas::UpdateNativeArea`], [`CCanvas::GetNativeOrigin`], [`CCanvas::GetNativeSize`]).  A canvas draw works in
the canvas's space; [`CCanvas::DrawBox`], [`CCanvas::DrawPixel`] and [`CCanvas::DrawImage`] cut to the native
canvas, and a [`CMapsCanvasDraw`] moves its maps' viewport by the native origin for the blit.  A code editor's text
canvas in a 3,457-line file is 2091x797 pixels, the view, not 2790x72618, the document.

The native canvas holds what the canvas last drew until its content is dirty.  A canvas can draw only part of itself
([`CCanvas::InvalidateArea`]) when its draw can ([`CCanvasDraw::CanDrawPart`]): a draw that only uses the canvas's
own drawing calls, which keep to its paint clip, or keeps to the paint clip itself.  [`CMapsCanvasDraw`] can, on a
canvas with its own image: it blits the maps cut to the clip ([`CMaps::Blit`], [`CBaseMap::BlitClipped`]), and the
native canvas is cleared only there ([`CNativeCanvas::Clear`]).  Other draws are drawn whole.

Copying a transparent canvas into the window blends it; the blend ([`BlendPremultipliedImage`]) copies an opaque
pixel and skips an empty one rather than blending them, which gives the same bytes.  An opaque canvas drawn in one
colour ([`CClearCanvasDraw`]) fills its part of the window itself, with no native canvas to fill and copy.

### Rows of a text area

A text area doesn't track its edits to find what to redraw.  At each paint [`CTextArea::PrepareArea`] lays out what
changed, then cuts the visible part into bands, one per row from its top to the next row's (a line's gap goes with
its last row), each with a hash of the row's text, its styles and the shapes crossing it: backgrounds, selection,
decorations ([`CTextAreaBands`]).  Comparing them with the bands of what was drawn
([`CTextAreaBands::Compare`]) finds the rows that moved, resized or changed, and only those are drawn again, where
they were and where they are.  All of it is drawn when there is nothing to compare (no rows, or nothing drawn yet),
when the canvas was invalidated, moved or scrolled, or when its width changed.

Hashing beats tracking edits here because the owners rewrite more than changed: the IDE clears and adds every span
again when it recolours code, and the code editor's gutter its backgrounds; the hash finds the rows that really
look different.

## Presenting

What a paint drew is handed to the native window ([`CNativeWindow::SignalPresentDamage`]), rectangle by rectangle.
Each backend shows them its own way ([Backends](backends.md)):

| Backend | Present |
|---|---|
| X11 | The rectangles merged into their bounds, put with MIT-SHM (`XShmPutImage`) without waiting for the server per frame, or `XPutImage` without it ([`X11Lib/X11Window.h`]). |
| Windows GDI | Each rectangle added to the update region (`InvalidateRect`); `WM_PAINT` copies only that. |
| WinRef | One frame per paint, saved as an image: tests that paint must give it an output folder. |

A backend without an area present ([`CNativeWindow::SignalPresentArea`]'s default) presents the whole window.

## The caret

The window draws the focussed component's caret, not the component: [`CComponent::GetCaretRect`] says where (window
space, clipped) and in what colour.  [`CWindow::PaintCaret`] keeps the pixels under the caret in a backing canvas,
then draws the caret over them.  Blinking ([`CWindow::BlinkCaret`]) swaps those pixels and presents only that
rectangle, without painting the window.  The caret is hidden under an open popup (a menu over the text).

The blink clock is the focus's ([`CFocus`]): shown and hidden every 500 ms, forever, as IntelliJ's
([`CFocus::SetCaretBlinkPeriod`], 0 for a solid caret).  It is shown again whenever the caret moves, the text is
edited, the focus changes or the window is activated ([`CFocus::ResetCaretBlink`], which only requests a paint).
Anything else that blinks or animates should use the same overlay and area present, not whole repaints.

## Checking a redraw

[`CWindow::SetVerifyRedraw`] draws all of the window again after each partial paint and logs the pixels and canvases
that differ: a canvas that changed without being invalidated, or something that moved without being damaged.  The
window then shows the full drawing, and the mismatches are counted ([`CWindow::GetRedrawMismatches`]).

- TestWindowLib runs every window test twice, the second time with every window drawing partially and verifying
  ([`CWindow::SetDefaultRedraw`]), and fails on any mismatch ([`TestWindowLib/TestWindowLib.cpp`]).
- Dialogs and floating dock windows redraw as their owner does ([`CWindow::SetRedrawLike`]).
- The applications all draw partially; each turns verifying on with an environment variable:
  `HCP65_IDE_VERIFY_REDRAW` (the IDE's checks then fail on a mismatch), `LOGICIM_VERIFY_REDRAW`,
  `HCP65_EMULATOR_VERIFY_REDRAW`, `HCP65_TESTER_VERIFY_REDRAW`, `WINDOW_INPUT_TESTER_VERIFY_REDRAW`.

Verifying only checks paints that happen.  A change that neither damages nor requests a paint is never painted, so
it can't be seen: keyboard selection in a text area once never redrew, and verify passed.  Test that kind of change
by reading the window's pixels after it ([`TestWindowLib/TestPartialRedraw.cpp`]).

## What it costs

The HCP65 IDE at 2478x1371, Release, on Linux with X11, 2026-11-07, is the reference.

Before any of this, a frame was about 28 ms for a 419-line file (layout 0.9, draw 18.8, present 8.7 ms) and the draw
96 ms for a 3,457-line one, 59 ms of it clearing: the editor's text and gutter canvases were the size of the whole
document (2790x72618, about 810 MB) and were cleared and drawn whole every paint.  What each change did:

| Change | Before | After |
|---|---|---|
| Native canvas only the visible part | Clears 59 ms (3,457 lines); maps 25.6 ms | Clears 0.6 ms; maps 1.0-1.1 ms whatever the file's length |
| Blend copying opaque and skipping empty pixels | Text canvas copy 6.3 ms | 2.3 ms |
| One-colour canvases filled into the window, MIT-SHM present | Quiet frame: draw 10.4 ms, present 7.5 ms | Draw 4.2 ms, present 1.1-3.1 ms; frame 6-8 ms, was about 19 |
| Partial redraw (damage only) | Hover: draw 16 ms, present 1.7 ms | Draw 1.9 ms, present 0.06 ms |
| Text area rows (bands) | Typing: draw 4.3-4.9 ms, 1.7 M canvas pixels a key | Draw 1.0 ms, 44 K pixels; arrows 7.6-9.7 to 1.1-1.7 ms |
| Mouse movement only requests a paint | Draw 6.25 ms a pointer move | 1.62 ms |
| Typed characters don't lay out | Layout 5.5 ms a key (9.9 KB file) | 1.7 ms; 3.0 to 0.06 ms in a 104 KB file |
| 60 frames a second, not 30 | Typing waited 21 ms on average for its paint to start | 8 ms |

Damage-only redraw was chosen from counting what changes: of 3.4 M pixels a frame, typing changed 1.8 K, hovering in
a tree 12.6 K, a menu 27 K, scrolling 21-74 K, an idle frame none (a caret blink 21 pixels).  A caret blink costs
0.0013 ms and a 1x21 present about 0.1 ms, against 19-22 ms for drawing the whole window.

Typing's own cost is in [The Text Area](text-area.md#decisions).  The profiles were taken with `perf` on the main
thread only (`--tid`), as the IDE's indexing threads swamp the samples, and the CPU's powersave governor swings paint
times about three times with the clock: compare builds on pinned cores.

## Decisions

- The caret blinks as IntelliJ's, every 500 ms, forever, drawn by the window as an overlay and blinked by patching
  its pixels (the user asked, 2026-11-06).
- Partial redraw (the user asked, 2026-11-07): damage areas, retained canvases, verify mode.  On in every application
  since 2026-11-07.
- Printable keys don't lay the window out; commands, chords, buttons, the wheel and modifiers still do (the user,
  2026-11-07; the old rule that every key lays out was dropped for typed characters only).
- Text area rows are found by hashing bands, not by tracking edits (2026-11-07; reason above).
- Not done because they wouldn't pay (2026-11-07): culling sprite maps (the maps draw in 1 ms with the visible
  canvas), passing the clip down further (2% of paint time), writing only the changed tile rows (0.8 of 3.3 ms a
  key).
- Windows paint at up to 60 frames a second by default (`WINDOW_DEFAULT_FRAMES_PER_SECOND`, 2026-11-07): at 30 a key
  typed just after a paint waited up to 34 ms, and a frame takes well under 17 ms.

## Not done

- Scrolling redraws the whole view: [`CScrollPane::SetScrollPosition`] invalidates the layout, which marks the
  content wholly dirty.  Scrolling by copying the pixels that stay is the idea.
- A paint after a pointer move with nothing damaged still takes about 1.6 ms (the prepare walk and composite?);
  not profiled.
- The first capital after Shift lays the window out once (the held-back Shift press).
- Large files cost about 3 ms a key ([The Text Area](text-area.md#not-done)).
- The code editor's gutter is made again whenever the line count changes.
- LogiCim's circuit canvas draws all of itself every paint ([`CCanvas::SetDrawnEveryPaint`]); drawing less needs
  change tracking in LogicimLib.
- Partial redraw hasn't been checked by hand in LogiCim, the HCP65 emulator and tester, or WindowInputTester;
  LogiCim's checks never show the window, so its verify variable checks nothing there.
- Nothing has been run on Windows: the GDI area present is only compiled (MinGW).  If it is wrong there, the IDE can
  turn partial redraw off ([`CWindow::SetPartialRedraw`]).

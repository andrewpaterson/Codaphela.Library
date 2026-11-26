# The Text Area

[`CTextArea`] is WindowLib's multi-line text: a log, a message, a code editor's code.  It is a transparent
[`CCanvas`] that draws its text, its backgrounds and its selection as maps ([`CMaps`], drawn by a
[`CMapsCanvasDraw`]), at its top left, clipped to its bounds.  It is meant to be the content of a [`CScrollPane`]:
the caret is kept in view and a page is the view's height.  [`CCodeEditor`] puts a gutter of line numbers beside
one, and [`CTextField`] shares its editing.

The text area was one class of 5,200 lines doing seven jobs.  It is now a thin canvas over the parts below, and the
parts that don't draw anything are in BaseLib, so the same editing runs in a text field and is tested without a
window.

## The parts

| Class | Library | Does |
|---|---|---|
| [`CTextDocument`] | BaseLib | The UTF-8 text, where its lines start, its undo history and its change count; tells its listeners each edit. |
| [`CTextNavigation`] | BaseLib | Characters, words, lines, brackets, quotes and comments of a text, for moving and selecting. |
| [`CTextUndo`] | BaseLib | The history: a snapshot of the text after each edit, typing merged into one edit. |
| [`CTextEditing`] | BaseLib | The caret, the selection and every edit, as commands; asks its view ([`CTextEditingView`]) about rows. |
| [`CTextEditingInput`] | WindowLib | The input actions (keys, pointer, clipboard, drags) that reach a [`CTextEditing`]; one list for area and field. |
| [`CTextAreaLayout`] | WindowLib | The text in rows: read in a font family's styles, laid out as sprites or tiles, with gaps below lines. |
| [`CTextAreaTiles`] | WindowLib | A fixed width font's text laid out as tiles, one character a cell. |
| [`CTextSpans`] | WindowLib | Styles over ranges of the text ([`CTextArea::AddSpan`]). |
| [`CTextBackgrounds`] | WindowLib | Colours under ranges, lines and line boundaries ([`CTextArea::AddBackground`]). |
| [`CTextLineGaps`] | WindowLib | Empty gaps below lines for an owner to draw in ([`CTextArea::SetLineGap`]). |
| [`CTextAreaOccurrences`] | WindowLib | The selected text's other occurrences, ranges given (a symbol's usages) and highlight layers. |
| [`CTextAreaShapes`] | WindowLib | Draws backgrounds, the selection and highlights as rectangles into geometry maps. |
| [`CTextAreaBands`] | WindowLib | A hash of each row as drawn, to find the rows that changed ([Drawing and Speed](drawing.md)). |
| [`CRevealingUndo`] | WindowLib | Undo that shows a change out of view before undoing it. |

[`CTextArea`] holds one of each, is the editing's view, and forwards its public editing methods
([`CTextArea::SelectAll`], [`CTextArea::ReplaceRanges`], [`CTextArea::Undo`] ...) in one block.  New text area work
goes into the part it belongs to, not into [`CTextArea`].

## Positions and lines

Positions are bytes of the UTF-8 text, always at the start of a character.  A character is a code point, or code
points joined by zero width joiners, and a carriage return with its line feed is one character
([`CTextNavigation::NextPosition`]).  Lines end at line feeds and are numbered from 0; rows are what the text is laid
out in, so a wrapped line has several.

Words are found two ways, on purpose.  [`CTextNavigation::FindWordAround`] is the word a double click selects: a run
of letters, digits and underscores, a run of spaces, or one other character.  [`CTextNavigation::FindWordOver`] is
the word Extend Selection grows to: only a run of letters, digits and underscores, so from spaces or punctuation it
goes on to the quotes, brackets or line around them.  Any character that isn't ASCII counts as a letter.

## The document and its listeners

[`CTextDocument::Insert`] and [`CTextDocument::Remove`] are the only edits.  After each, every listener
([`CTextDocumentListener`]) is told what changed in one call, [`CTextDocumentListener::DocumentChanged`], with an
[`STextDocumentChange`]: inserted or removed, the byte range, the line it starts on, how many line feeds went in or
out, and whether it was at the start of a line.  The listeners keep what is anchored to the text in step:

- the caret and selection ([`CTextEditing`]);
- spans, which move and stretch, so text put in takes the style of the character before it, and go when all their
  text is taken out;
- range backgrounds, which move and grow only with text put strictly inside them;
- line backgrounds, boundaries and gaps, which stay with their lines: pushed down by line feeds put in at a line's
  start, and gone with a line that is deleted or joined onto the line above;
- found occurrences and highlight ranges, which an edit makes stale (they are asked for again).

[`CTextDocument::SetText`] and [`CTextDocument::Append`] are not edits: nothing anchored moves, nothing goes into the
history, and setting the text removes backgrounds, gaps and highlights.  A log that grows by appends doesn't keep a
history of them.

## Editing

[`CTextEditing`] works as IntelliJ IDEA's editor does, for text in proportional fonts and styles too:

- Up and down keep the caret's x; home is smart (first character that isn't a space, then the line's start); a
  second home or end on a wrapped row goes to the ends of the line.  These work on the view's rows, so on wrapped
  lines.
- Tab inserts spaces to the next tab stop, or indents the lines of a selection over more than one line; back tab
  takes a tab's worth of spaces off them.
- Extend Selection ([`CTextEditing::ExtendSelection`]) grows to the smallest bigger range around the selection: the
  word, inside then with quotes, inside then with brackets, the line without its indent, whole lines, a comment,
  the whole text, and what a [`CTextSelectionRanges`] adds (the IDE adds its parse's syntax).  Shrink Selection goes
  back through the selections it grew from, until the selection changes another way or the text is edited.
- A box selection ([`CTextEditing::BoxSelect`]), as Visual Studio's, is the rows between two corners, each from the
  box's left x to its right x.  Typing, tab, backspace, delete, paste and cut act on every row as one edit; a box
  with no width is a caret on every row.  What a box copies is a block, and [`CTextEditing::PasteBlock`] pastes a
  block back as a column from a plain caret, filling short lines with spaces.  Any other move or edit ends the box.
- Cut with nothing selected cuts the caret's line ([`CTextEditing::Cut`]); a single line text cuts nothing.
- [`CTextEditing::MoveText`] moves or copies a range, as one undo step: dragging the selection uses it.
- A text that isn't editable can still be navigated, selected and copied from; a read-only cut copies what it would
  cut.

Each edit begins with the caret and selection as they are (for undo to put back), changes the document and ends: it
is kept in the history, then the view is told ([`CTextEditingView::UndoChanged`], [`CTextEditingView::TextEdited`]).
An edit that changed nothing (backspace at the start) isn't kept or told.

The view, [`CTextEditingView`], answers questions about rows: how many, which a position is on, where a row starts
and ends, the position at an x, the x of a position.  A position at the start of a wrapped row can be drawn at the
end of the row above (`bRowEnd`).  [`CTextArea`] is that view; a single line [`CTextField`] is a view of one row.

### Input

[`CTextEditingInput::Init`] adds every input action a text needs to its component, so a text area and a text field
have one list.  The keys go to [`CTextEditing`] directly, as commands; the pointer puts the caret and selects:

- a double click selects the word, a triple click the line with its line feed, and dragging on from either selects
  by whole words or lines;
- a shift press extends the selection; Shift+Alt (or Alt) and button one drag a box, and Shift+Alt with the arrows,
  Home and End move its corner;
- with text drags on ([`CTextEditingInput::SetTextDragEnabled`], on in a text area), a press in the selection that
  moves more than 4 pixels drags the selected text, with a drop caret, and Control copies it;
- a press beside the text ([`CTextEditingInput::StartLineDrag`], from a code editor's gutter) selects whole lines;
- Paste from History (Control+Shift+V) asks [`CClipboardHistory`]'s chooser which earlier text to paste, at the next
  tick as it shows a dialog.

A single line text has no up and down, pages, tabs, line feeds or line commands; they are left to the parent.  The
text editing commands keep the window's layout ([`CInputAction::SetKeepsLayout`]): a key that only moves the caret
doesn't lay the window out.

Four interfaces let an owner take a press instead: [`CTextUndoPresser`] (undo and redo, for a revealing undo or an
application's undo across texts), [`CTextEnterPresser`] (enter sends what was typed, as a request box does),
[`CTextLinkPresser`] (Control+press goes to a declaration) and [`CTextDoubleClickPresser`] (a double click goes to a
build message's line).  [`CTextArea`] implements all four and turns them into its events
([`CTextArea::SetEnterEvent`], [`CTextArea::SetLinkEvent`], [`CTextArea::SetDoubleClickEvent`]).

## Undo

The history is Logicim's kind: [`CTextUndo`] keeps the whole text after each edit, with the caret and selection from
before and after it.  Typing is merged into the state of the typing before it while each character is typed where
the last left the caret, within the typing pause (1000 ms, [`CTextUndo::SetTypingPause`]), and up to a new line or
the space after a word ([`CTextUndo::MergesTyping`]).

Undoing loads a snapshot, but [`CTextDocument::Load`] changes only the part of the text that differs from it
([`CTextUndo::FindDifference`]: common prefix and suffix), so the listeners move as little as they can and spans
outside the change survive.  A text area that isn't editable keeps no history ([`CTextDocument::SetUndoable`]).  The
undo event ([`CTextArea::GetUndoEvent`]) reports each change as an [`STextUndoChange`]: [`TUC_Kept`],
[`TUC_Merged`], [`TUC_Undone`] or [`TUC_Redone`], with the state ids before and after, which the IDE uses to undo
across files.

With [`CTextArea::SetRevealsUndo`], undo never changes what can't be seen: if the next undo's change is out of view,
the press scrolls to it and says so ("Ctrl+Z again to undo the change at line 120", through
[`CTextArea::GetUndoMessageEvent`]); the next press, finding it in view, undoes it.  [`CRevealingUndo`] only decides
which; its host ([`CRevealingUndoHost`]) says what is in view, shows it and undoes.

## Layout

[`CTextAreaLayout`] turns the document into rows.  Its setters (font family, style, spans, gaps, word wrap, tiles,
tab width, width) only mark it dirty, so many changes are laid out once: [`CTextAreaLayout::ReadText`] and
[`CTextAreaLayout::LayoutText`] do the work, when the text area is laid out, drawn or asked its size.

- **Sprites.**  The text is read into a [`CText`] in the faces, sizes and colours of a [`CFontFamily`], with the
  spans over the text area's style, and laid out by SupportLib's [`CTextLayout`] into a [`CSpriteMap`] of glyph
  cels.  Underlines, overlines and strike-throughs go into a [`CGeometryMap`] over the text.
- **Tiles.**  With [`CTextArea::SetTiledText`], a fixed width font without word wrap is written into a tile layer
  instead ([`CTextAreaTiles`], below).  The IDE's editors use tiles.

Word wrap is off by default ([`CTextArea::SetWordWrap`]).  When on, lines break at spaces to fit the width the text
area is laid out to, a word too wide for a line by itself is broken between glyphs, and trailing spaces don't count
toward the width.  Before the first layout (width 0) nothing is wrapped.

The text area's required size is the size of the laid out text, at least one line high
([`CTextArea::GetTextSize`]); word wrapped, its required width is -1 (it takes the width it is given) and its
height follows the wrapped lines.  [`CTextArea::GetDesiredSize`] overrides the required size per axis, as
[`CLabel`]'s does.  Wrapping makes height depend on width: when [`CTextArea::Layout`] lays the text out at a new
width and its height changes, it asks for another pass ([`CWindow::LayoutAgain`]); [`CWindow::Layout`] runs up to
three passes, so the first frame is right.  An edit that changes the text's size outside a layout calls
[`CComponent::LayoutChanged`] instead: [`CWindow::LayoutAgain`] is only valid mid-layout, and using it there once
left the window unlaid after edits (Debug's layout check hid it).

Appending ([`CTextArea::AppendText`], a build log) lays out only from the start of the line the old end was on, as
sprites or tiles, unless something else is dirty.

### Tiles

A fixed width font's text is a grid, so each character is a cell of a [`CTileLayerCel`]:

- The layer stores each row as a run of plain 32 bit tile values (cel index plus one, 0 empty) and keeps each cel
  once in a deduplicated table, with one blitter per cel.  A [`Ptr`] per tile made setting tiles quadratic (each
  pointer's dist-to-root bookkeeping), so tiles are data.  Rows are set whole ([`CTileLayerCel::SetRowTiles`]).
- [`CTileTextWriter`] writes UTF-8 text into the layer: a tab to the next tab stop, a carriage return ignored, a
  character the font hasn't got as '?', colours from sorted byte spans.  It keeps each character's tile value per
  colour in pages of 256 code points, so writing a character again is reading it back; any code point, not just
  ASCII.  [`TileTextColumns`] is the one column rule, shared by the writer and the text area's caret.
- Glyphs are looked up by code point ([`CFont::GetGlyph`] with a [`CUnicodeReader`]), never by UTF-8 bytes, for
  anything outside ASCII.
- [`CTextAreaTiles`] builds the rows and stops ([`STextLayoutLine`], [`STextLayoutStop`]) from the columns itself,
  so the caret and selection code reads the same arrays for tiles as for sprites.  A line's gap is the fewest whole
  rows of tiles at least as high as it.
- Span colours apply to tiles; bold, italic and size spans are ignored.  Decorations are drawn over the tiles into
  the geometry map exactly where [`CTextLayout::DecorateLine`] puts them, so they run into neighbouring cells.

Every edit still writes the whole text into the layer, but from the cached values: 0.8 ms for a 9.9 KB file (below).

## Styles and spans

The text is in the text area's style ([`CTextArea::SetStyle`]: regular, the family's default size, opaque white
unless set) except where spans ([`CTextArea::AddSpan`]) override parts of it with an [`STextStyleOverride`]:
colour, bold, italic, size, position (superscript, subscript), underline, overline, strike-through.  Where spans
overlap, each property takes the value of the last span added that sets it.  The style payloads are SupportLib's, so
the spans stay in WindowLib keyed by range rather than in the document.  The IDE colours code by clearing all spans
and adding them again ([`CTextArea::ClearSpans`]), 400 ms after typing stops.

## What is drawn

The maps are drawn in this order, each over the one before:

| Map | Holds |
|---|---|
| Backgrounds | The caret's line ([`CTextArea::SetCaretLine`]), then [`CTextArea::AddBackground`] ranges, [`CTextArea::AddLineBackground`] bands across the whole width and [`CTextArea::AddLineBoundary`] thin lines between lines, in the order added.  Line backgrounds don't cover gaps. |
| Selection | Highlight layers, found occurrences, then the selection or box; ranges given by [`CTextArea::SetOccurrenceRanges`] while nothing is selected. |
| Sprites | The glyphs, when not tiled. |
| Tiles | The glyphs, when tiled. |
| Lines | Underlines, overlines and strike-throughs. |

The background map is rebuilt lazily, when it is drawn after the backgrounds or the layout changed.  The selection
map is rebuilt when the marks change ([`CTextArea::UpdateMarks`]).  The caret isn't in any map: the window draws the
focussed component's caret over what it paints and blinks it ([Drawing and Speed](drawing.md#the-caret)), so only
the text area with focus shows one ([`CTextArea::GetCaretRect`]).

Occurrences work as IntelliJ's: with [`CTextArea::SetHighlightOccurrences`], the selected text's other places
(exactly, case and all) are found at the next tick after the selection changes, so a burst of keys finds them once;
at most 1000, and none for a selection over several lines or of only spaces and tabs.  Highlight layers
([`CTextArea::AddHighlightLayer`], [`CTextArea::AddUnderlineLayer`]) are ranges an owner gives, a Find bar's
matches say, with one current range in its own colour; of a layer with very many ranges only the 2000 nearest the
current one are drawn.  An edit, or setting or appending text, drops given ranges and layers; their owner gives them
again for the text as it is.

Changes are drawn row by row: [`CTextArea::PrepareArea`] brings the layout and maps up to date as the window is
painted, hashes each row as it will be drawn ([`CTextAreaBands`]) and redraws only the rows whose hash, top or height
changed.  [Drawing and Speed](drawing.md#rows-of-a-text-area) has the rules and the numbers.

## Gaps below lines

[`CTextArea::SetLineGap`] leaves an empty gap under a line for its owner to draw in: a code editor's line component,
the code a line compiled to.  A gap goes under the last row of a wrapped line, and is rounded up to whole rows in
tiles.  The caret and selection go over gaps as if they weren't there.  Each gap has an id
([`CTextArea::GetLineGapId`]) it keeps while it exists, through changes of height, for finding its line after edits
([`CTextArea::FindLineGap`]).

## The code editor

[`CCodeEditor`] is a container of two text areas: a gutter on the left and the code ([`CCodeEditor::GetTextArea`]).
The gutter is a text area too, in the code's font, tiled when the code is, with the code's gaps copied in, so its
numbers sit on the code's rows; for that the code isn't word wrapped and its spans mustn't change the font's size.
It is meant to be a scroll pane's content: scrolled sideways the gutter stays at the view's left on its own
background, and the caret is kept in view right of it.

The gutter's row for each line is made of columns, left to right:

| Column | Set with | By |
|---|---|---|
| Glyph (optional) | [`CCodeEditor::SetGlyphColumn`], [`CCodeEditor::SetLineGlyph`] | A breakpoint or the current line, as Visual Studio's margin. |
| Icons (optional) | [`CCodeEditor::SetIconColumns`], [`CCodeEditor::SetLineIcon`] | One column per kind of mark: a bookmark, an override. |
| Mark | [`CCodeEditor::SetLineMark`] | One character per line, a space unless set. |
| Number | | The line number from 1, right aligned. |
| Change bar | [`CCodeEditor::AddChangeBar`] | Lines changed since a commit; a line between rows where lines were removed. |

Marks, glyphs, icons and change bars are by line number: they don't move as the code is edited.
[`CCodeEditor::AddChangeBand`] colours the gutter's whole rows instead, with brighter numbers
([`CCodeEditor::SetBandNumberColour`]); the diff view uses bands.  A press on a column calls its event (glyph, icon,
mark, change bar); a press on a number selects its line, and dragging on selects whole lines, in the gutter or into
the code.

Line components ([`CCodeEditor::AddLineComponent`]) are shown under lines, each in a gap the code leaves below its
line, as wide as the code and as high as the component wants.  They stay with their lines as the code is edited and
go when their line is deleted.  [`CLineZones`] uses them to show a build's listing pages under the lines they came
from: a mark in the gutter for a line with pages, clicked to open them; editing the code greys the zones until the
source is set again.

Changing marks, glyphs, bars or line components lays the editor out again without drawing all of it
([`CComponent::LayoutChanged`]); change bars set between layouts are drawn at the paint
([`CCodeEditor::PrepareArea`]), so only the gutter rows they change are drawn again.

## The text field

[`CTextField`] has its own [`CTextDocument`], a single line [`CTextEditing`] and a [`CTextEditingInput`] made
without multi-line commands, so it edits, moves and undoes exactly as a text area does.  It draws its selection with
a [`CTextMarksCanvasDraw`]; its caret, like the text area's, is drawn by the window.

## Tests

| Test | Covers |
|---|---|
| [`TestBaseLib/TestTextDocument.cpp`], [`TestBaseLib/TestTextEditing.cpp`], [`TestBaseLib/TestTextUndo.cpp`] | The document, editing and undo, also compiled for and run on the 65C64 and W65C816. |
| [`TestWindowLib/TestTextArea.cpp`] | Layout, word wrap, spans, styles, decorations, tiles matching sprites pixel for pixel, appending. |
| [`TestWindowLib/TestTextAreaEdit.cpp`], [`TestWindowLib/TestTextEditingInput.cpp`] | Edits through the text area and through its input actions. |
| [`TestWindowLib/TestTextAreaRows.cpp`], [`TestWindowLib/TestTextAreaBackgrounds.cpp`], [`TestWindowLib/TestTextAreaBands.cpp`] | Rows, backgrounds following edits, band comparison. |
| [`TestWindowLib/TestCodeEditor.cpp`], [`TestWindowLib/TestTextField.cpp`] | The gutter, line components, change bars; the field on the shared editing. |
| [`TestSupportLib/TestMaps.cpp`] | Tile rows, the tile writer, blits after edits and in scrolled viewports, tiles equal to sprites. |

## Decisions

- Word wrap is off by default (the user, 2026-10-21).
- A text area's required size is its laid out text's; word wrapped, its required width is -1 (the user,
  2026-10-21).
- Decorations on tiles are drawn where [`CTextLayout::DecorateLine`] puts them, running into the next cells, not cut
  to the cell (the user, 2026-11-03).
- Tiles are plain data in a deduplicated cel table, not a [`Ptr`] per tile: setting a tile with pointers was
  quadratic (2026-11-03).
- The full split (the user, 2026-11-07): [`CTextDocument`] and [`CTextEditing`] in BaseLib, graphics-free like
  [`CTextUndo`] and [`CTextNavigation`], shared by [`CTextArea`] and [`CTextField`] with one input-action list;
  [`CTextAreaLayout`] for rows, sprites and tiles; the canvas thin.  The old `CPlainTextEditor` and `CTextEditor`
  were deleted.  Style payloads stay in WindowLib, keyed by range.  The public [`CTextArea`] API used by the IDE,
  LogiCim and WindowInputTester was kept.  Reason: 4,100 lines doing seven jobs, and the text field duplicating
  undo, caret, editing and input.
- [`CTextField`] behaves as the text area does (the user, 2026-11-07, accepting the changes): smart Home,
  Control+Backspace and Control+Delete delete a selection, overwrite replaces a tab character, a right press outside
  the selection moves the caret, and pointer positions step by whole UTF-8 characters.
- Editing keys (the user, 2026-11-07): Redo is Control+Shift+Z, as IntelliJ's; Control+Y is Delete Line, never Redo;
  Cut Line has no key of its own, it is Control+X with nothing selected.  Applications supply their own keymaps.
- The tile writer caches tile values in 256-code-point pages for all of Unicode, not an ASCII table (the user,
  2026-11-07).
- Typing cost: the tile rewrite was fixed (a key in a 9.9 KB file 19.6 to 3.2 ms, of which the tile write 17.4 to
  0.8 ms; 104 KB file 187 to 18.5 ms; Release, median of 100 keys, 2026-11-07), then the stop starts
  ([`CTextAreaLayout::FindStopStarts`], 104 KB file 7.4 to 4.6 ms at full clock).  Writing only the changed tile
  rows would save at most 0.8 of 3.3 ms, and incremental text layout isn't needed while 99% of the repositories'
  sources are under 47 KB: the user chose the stop starts fix and neither of the others (2026-11-07).
- Rows are redrawn by comparing hashes of what each row draws, not by tracking edits: the IDE clears and re-adds
  every span when it recolours, and the gutter its backgrounds, so exact edit tracking would redraw everything
  anyway (2026-11-07).

## Not done

- Every edit reads and lays out the whole text again, and writes the whole tile layer: about 3 ms a key in a 104 KB
  file (stops and map, tile write, spans).  Incremental layout would fix it; not needed yet.
- The gutter's text is made again, every line number, whenever the number of lines changes (each line feed).
- Bold, italic and size spans don't show in tiled text.
- A text area can't be saved ([`CTextArea::Save`] returns false).  Tile maps and image cels, which it draws with,
  can't be saved either; making them saveable needs a decision about their blitters, cache and viewport.

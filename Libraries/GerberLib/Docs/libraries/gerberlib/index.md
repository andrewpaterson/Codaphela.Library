# GerberLib

GerberLib reads and writes the files a circuit board is made from: Gerber (RS-274X, Ucamco's current specification)
for the layers and Excellon for the drills, as KiCad writes them. It parses each into a tree of commands, interprets
the tree into the operations it draws, turns those into vector images, and from them writes SVG and rasterises
layers. On top of that it packs several boards into one panel, joins them with tabs and mouse bites, and writes the
panel's fabrication files. The raster is for checking a panel before it is ordered.

GerberLib depends on BaseLib, StandardLib and SupportLib, whose vector paths and rasteriser it draws with. It isn't in
an indexed solution, so its classes are named here by their headers. Its tests are TestGerberLib
(`Tests/TestGerberLib`).

## The stages

| Stage | Headers | Is |
|---|---|---|
| Parse | [`GerberLib/GerberParser.h`], [`GerberLib/ExcellonParser.h`] | text to a tree of commands: CGerberCommands, CExcellonCommands |
| Interpret | [`GerberLib/GerberInterpreter.h`], [`GerberLib/ExcellonInterpreter.h`] | walks the commands with a graphics state; reports operations to a listener |
| Write | [`GerberLib/GerberWriter.h`], [`GerberLib/ExcellonWriter.h`] | commands back to text |
| Vectorise | [`GerberLib/GerberVectoriser.h`], [`GerberLib/ExcellonVectoriser.h`] | listeners making a CGerberVectorImage of CGerberVectorObjects |
| Draw | [`GerberLib/GerberSvgWriter.h`], [`GerberLib/GerberRasteriser.h`], [`GerberLib/GerberBoardRenderer.h`] | SVG, an 8-bit opacity raster per layer, a board or panel as it looks when made |
| Boards | [`GerberLib/GerberFileSet.h`], [`GerberLib/GerberFileSets.h`], [`GerberLib/GerberOutlineBounds.h`] | one board's files from a directory, and its outline |
| Panel | [`GerberLib/GerberPanel.h`], [`GerberLib/GerberPanelOutlines.h`], [`GerberLib/GerberPanelTabs.h`], [`GerberLib/GerberPanelEdgeCuts.h`], [`GerberLib/GerberLayerMerger.h`], [`GerberLib/ExcellonDrillMerger.h`], [`GerberLib/GerberPanelFiles.h`] | placing boards, tabs, merged layers and drills, the panel's files |

## Parsing Gerber

CGerberParser turns Gerber text into CGerberCommand subclasses in a CGerberCommands. Every command in the official
grammar parses, and the deprecated G74. Its structure follows Ucamco's PEG grammar: Parse reads MO, FS, SR and block
statements until M02; a block body (AB, SR) is block statements only (no MO, FS, SR or M02); a region body (G36 to
G37) is G04, G01 to G03, D01 and D02 only, and its first operation must be a D02.

The region, AB and SR statements are compound commands (CGerberCommandCompound,
[`GerberLib/GerberCommandCompound.h`]): each owns a nested CGerberCommands, allocated with [`NewMalloc`] and
forward-declared to avoid an include cycle. The parser swaps its current commands while it parses a body.

Points to know:

- [`CTextParser`] rejects text with a carriage return, and CGerberParser's Init returns that: callers make newlines
  plain first ([`CCharsBase::PassifyNewlines`]; CGerberLayerFile does, the tests read through a text file).
- Aperture macro expressions are parsed by BaseLib's [`CCalculator`] with Gerber's own symbols
  (InitGerberCalculatorSymbols, shared, as expressions keep a pointer to them): `x` multiplies, unary `+` and `-`
  exist for `-$3+...`, and `/` binds tighter than `x`. Identifiers in expressions are macro variables (`$` and
  digits, ParseGerberMacroVariable), so `$1x0.75` isn't read as one name; names elsewhere may start with `$`
  (ParseGerberIdentifier). `()` is an error.
- `\u` and `\U` escapes in strings are decoded to UTF-8 ([`GerberLib/GerberString.h`]); the two are case-sensitive.

## Interpreting

CGerberInterpreter walks the command tree with a CGerberGraphicsState (the current point with modal X and Y, the
current aperture, plot and quadrant modes, polarity, mirroring, rotation and scaling, the unit) and the aperture
dictionary CGerberApertures (standard apertures with their parameters checked, macros, AB blocks). It reports to a
CGerberInterpreterListener: Move, Draw, Arc and Flash, and the beginning and end of each region, block and step and
repeat. A step and repeat's body is reported once; the listener replicates it.

Macros are evaluated with CGerberMacroVariables (`$n`, an undefined variable is 0, no redefinition): each primitive's
Evaluate gives a CGerberEvaluatedPrimitive, its values in the specification's order. CGerberExpression walks the
[`CCalcExpression`] tree itself (the calculator binds variables per expression at parse time), using
[`CCalcExpression::IsVariable`].

Attributes don't affect the image and are ignored by the interpreter; the panel code reads TF and TA itself.

CExcellonParser and CExcellonInterpreter read KiCad's drill files: M48, `;` comments and `; #@!` TF, TA and TD
attributes, FMAT, INCH or METRIC, tool definitions (TnCd), `%` or M95, G90, G05, tool selection, X Y hits (modal) and
M30. Slots are read in both forms KiCad writes, routed (G00 X Y, M15, G01 X Y, M16, G05) and canned (X Y G85 X Y),
and reported as Slot(tool, start, end); holes as Hole(tool, point). The slot forms were checked against KiCad's
`gendrill_excellon_writer.cpp`. Excellon accepts CRLF itself.

## Writing

CGerberWriter writes one command per line. Coordinates are integers in the file's FS format and must fit it exactly:
a value that doesn't is an error, never rounded. Macro expressions are printed from the calculator tree with as few
parentheses as parse back to the same tree. Strings escape `%`, `*`, `\`, `,` in fields, and everything not ASCII as
`\uXXXX` or `\UXXXXXXXX`.

CExcellonWriter always writes METRIC: inch coordinates and diameters are multiplied by 25.4 (exact, in [`CNumber`]),
coordinates always have a `.`, and KiCad's FORMAT comment is updated to match.

Every Gerber and drill file of the user's 658-Computer boards (1426 Gerber and 336 drill files) round-tripped (write,
parse, write identical, the same interpreter events), and gerbonara 1.6.3 read the originals and the written files
identically, 1.05 million objects (2026-10-21). gerbonara can't read AB block apertures at all.

## Vector images, SVG and rasters

CGerberVectoriser, an interpreter listener, makes a CGerberVectorImage: CGerberVectorObjects in the order they were
created, each a SupportLib [`CVectorPath`] with a polarity (dark or clear). A path is parts of closed contours of
straight and circular edges ([`SVectorSegment`]) in float64 millimetres, filled non-zero, holes winding the other way;
a part with exposure off erases the earlier parts of the same object only. Inches are converted to millimetres.

Strokes, obrounds, polygons and rings are SupportLib's shape builders ([`AddVectorStroke`], [`AddVectorArcStroke`],
[`AddVectorObround`], [`AddVectorRegularPolygon`], [`AddVectorRing`]); the Gerber-only ones (a standard aperture's
hole, the thermal as four quarter pieces, every macro primitive, aperture shapes, cached) are in
[`GerberLib/GerberVectorShapes.cpp`]. Draws and arcs are stroked with a solid circle, which LS scales. Blocks are
expanded with a clear flash toggling their objects' polarity; step and repeat copies step in Y then X; LM, then LR,
then LS apply, LS to stroke widths too, regions unaffected. A zero-width draw (C0) is an object with no contour.
CExcellonVectoriser makes drills' holes and slots dark objects. Object counts match gerbonara's, and areas are tested
in closed form.

CGerberSvgWriter writes layers as SVG groups in millimetres, y negated; clear objects and exposure-off parts become
nested SVG masks over what is before them.

CGerberRasteriser fills a CGerberRaster (8-bit opacity, y down, its origin the top left in millimetres) object by
object, through a per-object window, with SupportLib's [`CVectorRasteriser`] over [`CCoverageRasteriser`] (the
coverage core taken out of [`CTrueTypeRasteriser`], which uses it too). Arcs are flattened to about 0.05 pixels, with
vertices at r times the root of a over sin a so areas are kept (inscribed chords were 0.4% low on drills). Each part is
combined separately, as primitives can wind oppositely, and objects composite in integers: dark d + c(1 - d), clear
d(1 - c). Checked against closed-form areas (drills within 0.011% at 40 pixels a millimetre) and against librsvg
(copper, silk and edge within 0.1 in 255 on average; librsvg is itself 0.13% small on circles); about 0.1 s a copper
layer at 40 pixels a millimetre (2026-10-21). SupportLib's shared vector drawing is described with its images
([Images and Drawing](../supportlib/images.md#vector-drawing)).

CGerberBoardRenderer draws a side of a board or a panel as it looks when made: the board shape cut from the edge
cuts, copper, a translucent solder mask (copper in an opening shows gold), silkscreen, then the holes; paste is left
out; the back is mirrored.

## Boards and panels

CGerberFileSet loads one board's directory, named after the directory: CGerberLayerFile for each Gerber file (layer
and side from `%TF.FileFunction`, as the EGerberBoardLayer and EGerberBoardSide enums) and CGerberDrillFile for each
drill file (plating from its TF.FileFunction). CGerberFileSets holds sets by pointer, and AddDirectories loads every
subdirectory. A board's outline bounds come from CGerberOutlineBounds, a listener building the edge cuts' centre lines
as a [`CVectorPath`] (stroke width excluded, arcs' extremes included), in millimetres with Gerber's y up: its
msTopLeft is the minimum corner, so its y is the board's bottom. A board with no edge cuts layer has no bounds.

CGerberPanel packs boards by those bounds with SupportLib's [`CRectangleBestPacker`] or
[`CRectangleBestPackerWithRotation`], in whole micrometres rounded outwards so gaps are never less than asked for
(defaults: 1.6 mm gap, the router bit, and 1.6 mm edge). The packer's y-down is used directly as Gerber's y-up. A
rotated board turns 90 degrees anticlockwise, (x, y) to (-y, x); CGerberPanelPlacement maps a board's points. The
packer's areas are 64-bit (square micrometres overflowed 32).

CGerberPanelOutlines chains each board's edge cuts into closed loops (ends within 0.01 mm joined; the biggest loop
the outside, anticlockwise; others cut outs, clockwise). CGerberPanelTabs joins facing straight edges across the
router gap; CGerberPanelEdgeCuts does the loop surgery (a tab joins two loops or, between boards already joined, splits
one, leaving the channel between tabs as its own loop). CGerberLayerMerger merges one layer of every board at command
level: exact [`CNumber`] coordinates moved and turned, both coordinates always written, D-codes renumbered from 10,
identical macros written once and clashing names renamed, a rotated board's R and O apertures' sizes swapped, P's
rotation plus 90 and macros copied as `<name>R90` with 90 added to every primitive's rotation. Its header keeps only
FileFunction and FilePolarity and adds `GenerationSoftware,Codaphela,GerberLib` and `Part,Array`; each board is
marked `G04 Board <name>` and followed by `%TD*%`. CExcellonDrillMerger merges drills in millimetres, a tool for each
diameter and TA.AperFunction, smallest first, slots canned (G85), and takes the mouse bites as extra holes.
CGerberPanelFiles writes `<panel>-<KiCad suffix>` files.

The tabs follow the user's rulings, as CGerberPanelTabs' defaults:

| Rule | Value |
|---|---|
| Frame | none: boards tab only to each other; a board joined to nothing fails Calculate |
| Tab width | 5 mm |
| Mouse bites | 0.5 mm holes at 0.8 mm pitch, just outside the board edge in the tab (offset minus the radius), leaving stubs to file |
| Tabs per edge | one per 30 mm of the whole facing edge, rounded up, so a notch doesn't count an edge twice |
| Placing | each slid to the nearest place straight on both edges and clear of copper; dropped if there is none |
| Copper clearance | 0.2 mm from a hole, against copper rasterised at 20 pixels a millimetre, tabs slid in 0.1 mm steps |
| Edge cuts | closed outlines round the channels |

The long edges of the test boards have copper 0.27 to 0.32 mm from the edge. Packing for least area makes long strips
(201 by 71 mm for the 11 test boards). Merged panels were checked with gerbonara (object counts per layer the sum of
the boards') and by renders.

## Test inputs

TestGerberLib's `Input` folder ([`TestGerberLib/TestGerberParser.cpp`] and the files beside it):

- `Ucamco`: Ucamco's official gerber-layer-format-test-files (April 2021), fetched through the Wayback Machine as
  ucamco.com returned 503; specification 2024.05 and its PEG grammar are there too. The specification's examples are
  named by paragraph ([`TestGerberLib/Input/Ucamco/Readme.txt`]). They are kept, with the official grammar's structure.
- `KiCad`: 11 boards' fabrication files from the 658-Computer's component boards. Adder 16bit's drills are in inches on
  purpose, for the writer's conversion; the other 10 boards are metric.
- `Panel`: the expected renders of the panel's front and back.

Renders were checked with librsvg (through Python's gi Rsvg): cairosvg ignores SVG masks.

## Deviations from the specification

- A plot with no plot mode set is linear: Ucamco's own 4.6.4 and `sample_macro` plot with no G01.
- G75 is required before the first circular D01. Single quadrant (G74) arcs are parsed and are an interpreter error.
- Draws and arcs need a solid circle aperture.
- MO and FS exactly once, before the first operation.
- Only `FS LA`; coordinates without an operation code (deprecated) are rejected, and deprecated commands other than
  G74 don't parse.
- In macro expressions `/` binds tighter than `x`; the specification gives them one precedence, left to right.
- Excellon: coordinates must have a `.`, which KiCad's decimal format always writes; its keep-zeros format writes bare
  INCH and integers, which would be misread, so it is rejected. G02 and G03 routing, G91 and zero suppression are
  errors.

## Decisions

- Keep the Ucamco files and the official grammar's structure (the user, 2026-10-21).
- GerberLib may depend on SupportLib and its images; the coverage rasteriser moved out of BaseLib into SupportLib
  (the user, 2026-10-21).
- The vector code is shared with SupportLib's [`CImageDraw`] and font rasteriser: the user's "do them all" for the
  shared vector drawing (2026-10-21).
- Panels: boards are merged into one panel per layer with mouse bites; boards turn in 90 degree steps, exactly (swap
  and negate; R and O swap sizes, P's rotation, macros as rotated copies rather than LR) (the user, 2026-10-21).
- The packing outline is the centre line of the edge cuts (the board's edge) and the gap is the router bit (the user,
  2026-10-22).
- The tab rulings in the table above (the user, 2026-10-22; one tab per 30 mm changed to the user's request the same
  day).

## Not done

- A latent bug: an outline primitive's vertices are a [`CArrayTemplate`] of CGerberExpressionPositions, each holding
  two CGerberExpressions with their own [`CCalculator`]; growing the array moves them with memcpy, while a variable
  parsed earlier ([`CCalcVariable`]) points into its calculator's variables. Noted, not fixed.
- Packing has no maximum panel size or aspect ratio.
- The merger needs every board in millimetres with one format, and can't merge step and repeat or block apertures,
  or rotate a board with LM, LR or LS or single quadrant mode.
- The raster conflates where anti-aliased edges of a clear and then a dark object meet (about 0.15% of a re-covered
  hole).

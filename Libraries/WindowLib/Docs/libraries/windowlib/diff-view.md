# The Diff View

[`CDiffView`] shows two texts side by side with their differences, as IntelliJ IDEA's diff viewer does in its
default, unaligned mode: the old text on the left, the new on the right, each in a [`CCodeEditor`] with its own line
numbers, and between them a divider of curved shapes joining each changed block's lines on the left to its lines on
the right.  It is read only: both sides can be selected and copied, not edited.

The view doesn't compare anything.  StandardLib's text diff does ([`CTextDiffProducer`], [`CTextComparison`],
[`CMyersDiff`]); the view owns a [`CSimpleTextDiff`] and its producer, and only reads the diff.

## Layout

[`CDiffView`] is a [`CGridContainer`]:

```text
[ toolbar: previous, next | whitespace | highlight |            n differences ]
[ left title             ] [       ] [ right title             ]
[ CScrollPane            ] [divider] [ CScrollPane             ]
[   CCodeEditor (left)   ] [       ] [   CCodeEditor (right)   ]
```

- Each side is an unchanged [`CScrollPane`] holding an unchanged [`CCodeEditor`], gutter and all, with its text area
  made read only ([`CTextArea::SetEditable`]).  Nothing in either is special to diffs: what the view needed was added
  to them as general features (below).
- The titles are the contents' titles ([`CTextContent::GetTitle`]): a file name, or as the IDE makes them, a file
  name and revision.
- The divider is a [`CCanvas`] 40 pixels wide drawn by a [`CDiffDividerDraw`], which the view gives the shapes.
- The toolbar has previous and next difference (also Shift+F7 and F7, [`CIC_PreviousDifference`] and
  [`CIC_NextDifference`], from anywhere in the view), the whitespace policy ([`TDW_Default`], [`TDW_Trim`],
  [`TDW_Ignore`]), the highlight policy ([`TDH_Lines`], [`TDH_Words`], [`TDH_Chars`]) and the number of
  differences.  In a narrow view the space before the count and then the count give way before the buttons and
  combo boxes are squeezed.

## Showing a diff

[`CDiffView::ShowTexts`] and [`CDiffView::ShowFiles`] make the diff through the producer and show it;
[`CDiffView::Update`] shows the diff again after whatever made it changed it.  Changing a policy
([`CDiffView::SetWhitespace`], [`CDiffView::SetHighlight`]) runs [`CTextDiffProducer::Produce`] on the texts the diff
already has, without reading them again.  [`CDiffView::SetTexts`] gives new texts in place, keeping the titles and
where the sides are scrolled: the IDE uses it to follow edits and new commits.

The diff's lines and the text area's must be the same lines.  [`CTextContent`] splits lines at LF, CRLF and a lone
CR, while a text area splits only at LF, so each side is given its content's lines joined with LF
([`CDiffView::ShowSide`]): the text area's line n is the content's line n whatever the file's separators.  A
fragment's offsets are into the content's text, so they are mapped ([`CDiffView::MapOffset`]): the line from
[`CTextContent::FindLine`] (a binary search), the column from the line's start, at most the line's length plus one
(an offset inside a CRLF lands on the LF), then [`CTextArea::GetLineStart`] plus the column.

When a side's text is unchanged it is kept, with its spans, and only its diff colours are cleared.  So the syntax
highlighting an owner adds as spans ([`CTextArea::AddSpan`] on [`CDiffView::GetEditor`]'s text area) survives a
policy change or an edit of the other side.  The diff colours are backgrounds, so they don't fight with the spans.

## Colours

The colours are an [`SDiffViewColours`], by block type ([`TDBT_Inserted`], [`TDBT_Deleted`], [`TDBT_Modified`]):
given to [`CDiffView::Init`], or taken from the window theme ([`SDiffViewColours::Init`], [`TC_DiffInsertedLines`]
to [`TC_DiffDivider`]; [`CJewelTheme`] has IntelliJ's dark colours).

| What | Drawn with |
|---|---|
| A block's lines | A band behind them across the text area ([`CTextArea::AddLineBackground`]) in the type's line colour. |
| Their line numbers | A change band on the gutter ([`CCodeEditor::AddChangeBand`]), the numbers on it in the theme's active line number colour so they can be read. |
| The characters that changed | A stronger shade over each fragment ([`CTextArea::AddBackground`]); fragments exist only for modified blocks under word or character highlighting. |
| A side with no lines in a block | A 2 pixel line in the stronger shade where its lines would be ([`CTextArea::AddLineBoundary`]), and a line between the gutter's rows there. |
| The divider | Each block's shape filled in its line colour and outlined in its stronger shade, on the divider colour. |

The gutter's bands are change bands rather than backgrounds on the gutter's text area because the gutter's text is
made again whenever its line count changes, which would clear backgrounds; change bands are the code editor's own and
are drawn again on it.

## Scrolling together

[`CTextDiffLineMapper`] (StandardLib, tested without windows) maps a fractional line of one side to the other
([`CTextDiffLineMapper::TransferLine`]): before, between and after blocks, lines map one to one, shifted by what the
blocks so far added or removed; inside a block a line maps proportionally across it; a block with no lines on the
side mapped from is a point there, and its lines on the other side are skipped.

Each pane's scroll event ([`CScrollPane::GetScrollEvent`]) calls [`CDiffView::Scrolled`], which
([`CDiffView::ScrollToMatch`]):

1. takes the fractional line at the vertical middle of the scrolled side's view ([`CDiffView::GetMiddleLine`]): the
   scroll position plus half the view's height, over the row height;
2. maps it to the other side;
3. scrolls the other side to put that line at its middle (as far as it can), and sideways to the same x.

Rows are all the font's line height, as the code isn't wrapped and spans don't change the font's size, so a line's y
is its number times the row height.

A side the view scrolled must not scroll the first back.  A flag set while syncing can't work, because the scroll
event comes later, while the window is laid out; so the view remembers the position it scrolled each side to
([`SDiffViewScrolls`]) and ignores that side's next event if it reports exactly that position.  Anything else is the
user scrolling.

The divider is given new shapes after every layout ([`CDiffView::Layout`]), and so after any scroll.  For each block
whose lines can be seen on either side, the shape's four corners are the y of the block's first row and of the row
after its last, on the left and on the right, as scrolled now.  [`CDiffDividerDraw`] makes each shape's top and
bottom edges cubic Beziers flat at both ends ([`CVectorPath::CubicTo`], flattened into lines for SupportLib's
[`CVectorRasteriser`]); a side with no lines has top equal to bottom, so the shape pinches to a point there.  The
shapes are painted into a cel the size of the canvas ([`CCelCanvasDraw`]), again only when the shapes or the size
change, and the divider is invalidated only then.

## Going to differences

Showing texts scrolls to the first difference, with its first lines a quarter of the way down each side, as IntelliJ
does.  [`CDiffView::ScrollToBlock`] goes to any block; it waits for the next layout, when the sides' heights are
known, and scrolls each side itself, as a block with no lines on one side would otherwise put that side after the
block.  [`CDiffView::NextDifference`] and [`CDiffView::PreviousDifference`] go on from the block scrolled to last,
or, once the user has scrolled, from the line a quarter of the way down the left side.

## In the IDE

[`CIDEDiffs`] opens a diff in a document tab of its own ([`CIDEDiffTab`]): Git, Compare with HEAD, with a branch or
a revision, and File, Compare with File.  A revision's text comes from git ([`CGitRepository::ReadRevisionText`]) and
is given to the view as text.  Both sides are tiled ([`CTextArea::SetTiledText`]) and highlighted as the editors are
([`CIDEDiffs::HighlightDiffSide`]).  The tab follows its sides: an edit in a side's editor is shown a little later
([`CDiffView::SetTexts`]), and a revision is read again when its repository's last commit changes.

## Features added for it

These were written for the diff view but are general, and other code uses them:

- [`CTextArea`] backgrounds: ranges, line bands and line boundaries under the text ([The Text Area](text-area.md)).
- [`CScrollPane::GetScrollEvent`]: called when the content has been laid out at a new scroll position, however it was
  scrolled.  [`CScrollPane::SetScrollPosition`] asks for another layout pass, so a handler can scroll another pane
  in the middle of a layout.
- [`CCodeEditor::AddChangeBand`]: whole gutter rows in a colour, surviving the gutter being numbered again.
- [`CTextContent::FindLine`], [`CTextDiffLineMapper`] (StandardLib) and [`CVectorPath::CubicTo`] (SupportLib).
- Tabs to tab stops in text drawn as sprites ([`CTextLayout::SetTabWidth`]), as tiles had them already.

## Tests

[`TestWindowLib/TestDiffView.cpp`] checks the divider's shapes for given scroll positions, a small diff drawn (both
against input images), and scrolling kept in step, with next and previous
difference.  [`TestStandardLib/TestTextDiff.cpp`] covers the diff, the line mapper and [`CTextContent::FindLine`];
[`TestSupportLib/TestVectorPath.cpp`] the curves; [`TestWindowLib/TestTextAreaBackgrounds.cpp`] and
[`TestWindowLib/TestScrollPane.cpp`] the backgrounds and the scroll event.

## Decisions

- Unaligned, as IntelliJ's default (the user, 2026-11-05): no blank spacers; each side shows its own text and line
  numbers, scrolling maps lines through the blocks, and the divider's curved shapes join the blocks.
- Read only first (the user, 2026-11-05): both sides selectable and copyable, not editable; an editable right side
  later.
- The view only reads [`CSimpleTextDiff`] (its mutators are for [`CTextDiffProducer`] only) and reuses
  [`CScrollPane`] and [`CCodeEditor`] unchanged; what it needed went into them as general features (2026-11-05).
- The divider is a canvas draw given its shapes by the view, not a component that works them out (2026-11-05).
- A side scrolled by the view is recognised by the position it was scrolled to, not by a syncing flag, as the scroll
  event comes during the next layout (2026-11-05).
- Showing texts goes to the first change, as IntelliJ does (2026-11-05).

The plan the view was built from, `WindowLib/DiffView.txt`, was last changed in CommonLibraries 0d3c1f311.

## Not done

- An editable right side: the code editor's change event ([`CCodeEditor::GetChangeEvent`]) setting the content
  ([`CTextContent::Set`]), then [`CTextDiffProducer::Produce`] and [`CDiffView::Update`], delayed as the IDE's
  recolouring is.
- Apply and revert arrows on the divider's shapes, once the right side can be edited.
- IntelliJ's "Align changes" mode: blank spacers under the shorter side (code editor line gaps, plus a gap above line
  0), straight divider bands and identical scroll positions.
- A git revision source in [`CTextDiffProducer`] itself: the IDE reads revisions and passes text, so only another
  user would need it.
- Rows are taken to be the font's line height: a side with gaps under its lines (line components) would be mapped
  and drawn wrongly.

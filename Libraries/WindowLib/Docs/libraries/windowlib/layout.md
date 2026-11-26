# Layout

A window is laid out from the top down.  The window gives its whole client area to its canvas, the canvas gives it
to the window's container, and each container decides where its children go and lays each one out into an area of
its own.  A child is told only its area, a position and a size in the window's client space
([`CComponent::Layout`]); what it does inside that area is up to it.  To decide, a container asks its children how big
they want to be: their desired sizes and, for containers among them, their required sizes.

## Sizes

| Size | Read with | Is |
|---|---|---|
| Actual bounds | [`CComponent::GetActualBounds`] | The area the component was laid out in by its parent.  For a container, insets included and not clipped. |
| Container bounds | [`CContainer::GetContainerBounds`] | A container's area inside its insets, clipped to its parent container's bounds: where its children are laid out and cut off. |
| Desired size | [`CComponent::GetDesiredSize`] | The size the component wants.  -1 on an axis means it has none there: it is variable sized and takes the space it is given. |
| Required size | [`CContainer::GetRequiredSize`] | Containers only: the space the container's children need, plus its insets, worked out from them by [`CContainer::SetRequiredSize`]. |

### Desired size

A component's desired size is -1, -1 until it is set with [`CComponent::SetDesiredSize`].  A component whose content
has a natural size gives it as its desired size, by overriding [`CComponent::GetDesiredSize`], and a desired size set
on an axis wins over it on that axis:

- A [`CCanvas`] with no desired size of its own takes its draw's ([`CCanvasDraw::GetDesiredSize`], -1, -1 unless the
  draw has a size): a [`CImageCanvasDraw`] is the size of its cel.
- A [`CLabel`] is as wide as its laid out text and as tall as its font's line.
- A [`CTextArea`] is the size of its laid out text, except that a word wrapped one has no width (-1): it takes the
  width it is given and is as tall as its text wraps to at that width.

So whether a component is fixed or variable sized is a property of its content: an image or a label is fixed, a word
wrapped text area is variable in width.

### Required size

Required size belongs to containers.  A container works it out from its children into its own required size; it never
writes a desired size.  A component that isn't a container never has a required size as far as its container is
concerned: its natural size is its desired size.  ([`CLabel`] and [`CTextArea`] keep a required size of their own,
their text's size ([`CLabel::GetRequiredSize`], [`CTextArea::GetRequiredSize`]), but only as the base of their
desired size; no container reads it.)

A container without a desired size on an axis is variable sized there: it takes whatever area it is given, even one
smaller than it requires.  A container that should keep the size its children need, as a button, a list, a check box
or a menu item does, is sized by its content ([`CContainer::SetSizedByContent`]): its desired size is then its required
size on each axis without a desired size set ([`CContainer::GetDesiredOrRequiredSize`]).

### Measuring a child

A container measures each child with [`CContainer::GetMeasuredSize`]: the child's desired size or, on an axis where a
container child has none, that container's required size.  A container without a desired size still fills the area
it is laid out in, but it needs at least what its own children need; without this a grid in an unweighted row of
another grid was given no height at all.  A child that isn't a container keeps its -1, and needs nothing there.

## Alignment

Where a component goes within the area its container gives it is the component's own: its horizontal and vertical
alignment, [`CComponent::SetAlignment`], centred on both axes by default.  [`CContainer::LayoutComponentInArea`] does
this for every container:

- On an axis where the component's desired size is -1, or its alignment is [`CSH_Stretch`] or [`CSV_Stretch`], it
  fills the area.
- Otherwise it keeps its desired size and is placed in the area by its alignment: [`CSH_Left`], [`CSH_Center`],
  [`CSH_Right`] and [`CSV_Top`], [`CSV_Center`], [`CSV_Bottom`].  [`CSH_Unknown`] and [`CSV_Unknown`] place it at the
  left or top.
- A fixed size component bigger than its area keeps its size: centred, it hangs over both sides.  The container clips
  it (see [Clipping and finding components](#clipping-and-finding-components)).

[`AlignComponentHorizontal`] and [`AlignComponentVertical`] give the offset for an alignment.

## Insets

Every container has insets ([`CContainer::SetInsets`], an [`SInsets`] of top, right, bottom and left): its children are
laid out inside them and its required size includes them.  After laying its children out a container sets its actual
bounds to the whole area it was given and its container bounds to the area inside the insets.

## Containers

| Container | Lays out its children |
|---|---|
| [`CFillContainer`] | All in the same area, on top of each other. |
| [`CFlowContainer`] | One after another along a line, wrapping onto new lines. |
| [`CGridContainer`] | In columns and rows, with weights sharing out the spare space. |
| [`CScrollPane`] | One child, the content, moved by the scroll position. |
| [`CSplitContainer`] | Two side by side or one above the other, with a sash between them. |
| [`CBarContainer`] | In a row or column at their own sizes, as a toolbar's or a status bar's. |

Many widgets are grids, [`CListContainer`] (below), [`CImageBorderContainer`] and [`CBarContainer`] among them.  The
dock area's containers are on the [Docking](docking.md) page.

### Fill

A [`CFillContainer`] lays every child into its whole area inside the insets, drawn in the order they were added: a
variable sized child fills the area, a fixed one is placed by its own alignment.  It is the one cell of a grid, and the
way to place a single component, a label say, by its alignment.  Its required size is the widest child's width by the
tallest child's height, plus the insets.

### Flow

A [`CFlowContainer`] places its children one after another along a line, in a direction set with
[`CFlowContainer::SetFlowStyle`]: [`CSD_Right`] (the default), [`CSD_Left`], [`CSD_Down`] or [`CSD_Up`].  Left and up
start at the far end.

Each child is as long as its measured size along the line, and a child without a size takes no space at all
([`CFlowContainer::GetLineSize`]).  It is given an area that long and as thick as its line, and its own alignment
places it across the line, or stretches it to the line's thickness.

- [`CSW_Wrap`] (the default): a line takes children until the next would make it longer than the area, but at least
  one.  Each line is as thick as its thickest child.  The block of lines is placed across the area by the container's
  own alignment, the one given to [`CFlowContainer::SetFlowStyle`] (left and top by default); this is the only thing
  the container's alignment does.
- [`CSW_Continue`]: one line, as thick as the whole area, which runs off the end if it is too long.

A flow never shrinks its children: what doesn't fit wraps, and what still doesn't runs off the end (the bottom, for a
flow to the right).  Its required size is that of a single unwrapped line: the children's lengths added up by the
thickest, plus the insets.

### Grid

A [`CGridContainer`] lays its children out in columns and rows, like Java's GridBagLayout.  A child is added with
its cell, [`CGridContainer::AddComponent`]: column, row, column span and row span (1 by default), and a width weight
and a height weight (0 by default).  Adding a component without a cell fails with an error.  The cells
([`CGridCell`]) are kept at the same indices as the children, and removing a child removes its cell.

Columns and rows (tracks) are worked out the same way, each axis on its own ([`CGridContainer::CalculateTracks`]):

1. There are as many columns as the furthest cell reaches.  A column no cell is in is 0 wide.
2. Each column is as wide as the widest child that sits only in it, and its weight is the largest weight of those
   children.  In a weighted cell a child without a desired width needs nothing: it takes its share.  In an unweighted
   cell a container child needs its required width ([`CContainer::GetMeasuredSize`]).
3. Then children spanning several columns, the narrowest spans first: a weight greater than the columns' together
   adds what they lack, and so does a width greater than theirs.  What is added is shared by the columns' weights or,
   when none of them has a weight, given to the last of them.
4. The grid's required size is the columns' widths and the rows' heights added up, plus the insets.

Laying out ([`CGridContainer::LayoutTracks`]) then fits the columns to the area:

- **Spare space** goes to the weighted columns, in proportion to their weights.  With no weighted columns the grid
  keeps its required width and is centred in the area.
- **Too little space**: the weighted columns give it up first, in proportion to their weights, none going below
  nothing; what one couldn't give is shared again between the others.  If that isn't enough, every column shrinks in
  proportion to its width.  There are no minimum sizes.
- **Slivers**: the pixels left over from sharing in proportion (spare space, a span's shortfall, shrinking) are handed
  out, or taken, one at a time, each to the largest column that hasn't had one yet, ties to the first.  The arithmetic
  is integer, so every build lays out on the same pixels.

Each child is then laid out in its cell, the columns and rows it spans, by [`CContainer::LayoutComponentInArea`].
Cells may overlap; they are drawn in the order they were added, and the last drawn is hit first.  The grid keeps where
its columns and rows went at its last layout ([`CGridContainer::GetTrackBounds`]): a list finds its rows by them.

### Border layout

[`CBorderLayoutHelper`] lays out an existing grid as a border round a centre, as Java's BorderLayout does.
[`CBorderLayoutHelper::Init`] takes the grid, which the caller holds, and [`CBorderLayoutHelper::AddComponent`] puts a
component in one of nine cells, [`BLC_TopLeft`] to [`BLC_BottomRight`] (column = cell % 3, row = cell / 3).  The
centre has a weight of 1, 1, so its column and row take all the spare space, and keeps its own alignment.  The outer
cells have no weight, so their columns and rows keep their required sizes, and a column or row with nothing in it
takes no space.  Each outer component is aligned towards the centre:

| | Left column | Centre column | Right column |
|---|---|---|---|
| Top row | right, bottom | centre, bottom | left, bottom |
| Middle row | right, centre | (its own) | left, centre |
| Bottom row | right, top | centre, top | left, top |

[`CImageBorderContainer`], what buttons, text fields and combo boxes are made of, lays its eight border pieces round
its face this way.

## Clipping and finding components

Every container clips its children to its container bounds unless told not to ([`CContainer::SetClipsChildren`]), so
a component overflowing its area, a fixed size one in a shrunk grid cell say, is cut off at the container's edge
rather than drawn over whatever is beside the container.  Turn it off for an overlay that should draw outside its
container.  A [`CScrollPane`] always clips.  A component's clip ([`CComponent::GetClip`]) is the overlap of the
container bounds of every clipping container it is in, and a container's bounds are already cut to its parent's
([`CContainer::SetContainerBounds`], with [`CRectangle::ClipTo`]).

[`CComponent::FindComponentAt`] finds the component under a point the same way it is drawn: children drawn later are
searched first, a point outside a clipping container's bounds (but inside its actual bounds) finds the container
itself, and a component hit as a whole ([`CComponent::SetHitAsWhole`], as a button or a list is) is found rather
than its parts.

## Scroll panes

A [`CScrollPane`] shows part of one component, its content ([`CScrollPane::SetContent`]; adding a component sets it),
in the style of IntelliJ IDEA: its scroll bars are translucent thumbs drawn over the content, not beside it, so the
view is the whole scroll pane.

The scroll pane's required size is nothing: it can be as small as its container makes it.  The content is laid out:

- on an axis whose policy is [`SPP_Never`] ([`CScrollPane::SetPolicy`]), to the view's size, shrunk to fit if it is
  bigger;
- on an axis whose policy is [`SPP_AsNeeded`] (the default), to its desired size, or the view's where its desired size
  is -1 or smaller than the view.  So a word wrapped text area gets the view's width and its own height.

Content smaller than the view on an axis and not stretched keeps its size and is placed by its alignment.  The
content is moved by the scroll position and laid out at its full size, however little of it shows; its canvases only
hold the part that shows (see [Drawing and Speed](drawing.md)).

A scroll bar ([`CScrollBar`]) is shown along the bottom or right while the content is bigger than the view on its
axis.  Each is laid out to its track, inset from the pane's edges by a margin of 2 pixels, and stops short of the other
when both are shown.  The thumb is as thick as the theme's scroll bar images ([`CScrollBarImages`]), as long as the
view is a part of the content but at least twice its thickness, and placed with integer arithmetic.  A track too
short for the thumb's two round ends has no thumb.  The whole track is hit: pressing the thumb drags it, and pressing
the track elsewhere jumps the thumb's middle to the pointer and drags it from there.  A note
([`CScrollPane::SetNote`]) is a component shown over the view's top right corner, inside the vertical bar's track, at
its desired size: what a list shows its typed search in.

The scroll position:

- [`CScrollPane::SetScrollPosition`] keeps the position as asked for, and it is limited to the content's size when it
  is used ([`CScrollPane::GetScrollPosition`]).  Before the first layout the content's size isn't known, and a word
  wrapped text area only knows its height after a first pass, so a position set early survives.
- [`CScrollPane::ScrollToShow`] scrolls as little as shows a rectangle (in the window's space), or its top left if it
  is bigger than the view, then asks scroll panes further out to show what is left of it.  A component asks with
  [`CComponent::ScrollToShow`], which goes up its parents to the nearest scroll pane; [`CComponent::GetScrollView`]
  gives the part of the window it is seen through.
- The content moves when the pane is next laid out.  Scrolled while the window is being laid out, the pane asks for
  another pass ([`CWindow::LayoutAgain`]).
- The scroll event ([`CScrollPane::GetScrollEvent`]) is called once the content has been laid out at a new position,
  however it was scrolled.  Its listeners may scroll other scroll panes then, as the diff view keeps its two sides
  together.

The wheel goes to the component under the pointer, not the focussed one.  A scroll pane scrolls by its wheel step
(48 pixels, [`CScrollPane::SetWheelStep`]), vertically or, with Shift, horizontally.  It takes the wheel on any axis
it can scroll on, even when it is already at the end, so a scroll pane it is inside doesn't suddenly start scrolling
instead; on an axis it can't scroll the wheel goes on to its parent, and with Control held it always does (Control
and the wheel zoom what has a zoom).

## Lists

A [`CListContainer`] is a grid whose rows are the list's rows; what menus, combo boxes, list boxes, trees and tables
are made of.  [`CListContainer::AddRow`] adds an empty row, [`CListContainer::AddToRow`] puts a component in one of its
columns (with a width weight, never a height weight), and [`CListContainer::AddItem`] adds a row holding one
component in column 0 with a width weight of 1, so it takes the list's whole width.  Because the rows share the
grid's columns, the parts of every row line up: a menu's captions and its shortcuts.  A disabled row, a separator
say, can't be selected.

For layout:

- A list is sized by its content: unless a desired size is set on an axis, it is the size its rows need.  So in a
  scroll pane it is as tall as its rows, and scrolls.
- The selected and hover highlights are two canvases, the list's first children so the rows are drawn over them, in
  cell (0, 0) with a desired size of -1, -1.  In an unweighted cell they need nothing, so they don't change the grid.
  After the grid is laid out each is moved over its row across the list's whole width, and only drawn while it marks
  a row.
- A row's place is the grid's row as last laid out ([`CListContainer::GetRowBounds`]); a list whose rows aren't the
  grid's, a tree with components only for the rows in view, overrides it.
- Moving the selection shows its row ([`CListContainer::ShowRow`], through [`CComponent::ScrollToShow`]).  Page up and
  page down move by the rows the scroll pane's view shows, less one so the row moved from is still in sight; with no
  scroll pane, by [`CListContainer::SetPageRows`] (10).

Selection, the keys, multiple selection and finding a row by typing are on the [Components](components.md) page.

## When layout runs

The window keeps its layout between paints.  [`CWindow::Paint`] lays the window out only when its client area has
changed size or its layout was invalidated: a component calls [`CComponent::InvalidateLayout`] whenever something its
layout depends on changes, its text or its size, and [`CComponent::SetDesiredSize`], [`CComponent::SetAlignment`],
[`CContainer::SetInsets`], [`CScrollPane::SetContent`] and adding or removing children do it themselves.  (Which parts
are drawn again afterwards is on the [Drawing and Speed](drawing.md) page.)

Some components only know their size once they have been given one: a word wrapped text area needs more lines when it
is narrower.  One whose size changed while it was laid out asks for another pass ([`CWindow::LayoutAgain`]), and
[`CWindow::Layout`] lays the window out again, at most three passes in all, in case components keep changing each
other's sizes.

In Debug builds the window lays itself out on every paint anyway, and logs each component that moved although the
layout wasn't invalidated: the kept layout would have been wrong.  A test that counts layouts turns this off
([`CWindow::SetCheckLayoutCache`]).

The tests are in [`TestWindowLib/TestFillContainer.cpp`], [`TestWindowLib/TestFlowContainer.cpp`],
[`TestWindowLib/TestGridContainer.cpp`], [`TestWindowLib/TestBorderLayoutHelper.cpp`],
[`TestWindowLib/TestNestedContainers.cpp`], [`TestWindowLib/TestScrollPane.cpp`] and [`TestWindowLib/TestList.cpp`].

## Decisions

- Required size is the container's alone, worked out from its children; it never writes the desired size.  A
  component that isn't a container gives its natural size as its desired size, never a required size (the user,
  2026-10-16 and 2026-10-17).  WindowLib_Old's "best size" design isn't used.
- A canvas with a desired size of -1, -1 takes its draw's ([`CCanvasDraw::GetDesiredSize`]) (the user, 2026-10-17).
- A desired size overrides the required size, on each axis it is set on (the user, 2026-10-17).
- A component with a desired size of -1 on an axis is variable sized and takes all the space it is given; a fixed size
  one keeps its natural size and is placed by its own alignment, centred by default.  Which one a component is follows
  from its content: image cels and labels are fixed, text areas variable (the user, 2026-10-17).
- Alignment is the component's, not its container's or its cell's (2026-10-18).  Stretch on an axis fills it, as -1
  does (Claude's choice, not confirmed by the user).  Unknown places at the left or top.
- Actual bounds are the whole area the parent gave, insets included and not clipped; the area inside the insets,
  clipped, is the container bounds.  A container sets both once it has laid its children out (the user, 2026-10-18).
- The fill container is an overlay only, with no split modes (the user, 2026-10-17; the split modes were removed
  2026-10-18).  It logically holds one component.
- A flow wraps, and runs off the end when there isn't room; it doesn't shrink its children (the user, 2026-10-17).
  Each child's own alignment places it within its line; the container's alignment only places the block of wrapped
  lines (the user, 2026-10-19).
- A grid cell has a position and a span in grid units, and weights: 0 means the cell gets its required size, a
  positive weight shares the space left over in proportion to the other positive weights (the user, 2026-10-17).
- The rest of the grid follows GridBagLayout (Claude's proposal, built 2026-10-17, not confirmed by the user): one
  weight per axis; a track's weight is the largest of its single span cells'; spare space goes only to weighted
  tracks, and with none the grid keeps its required size; a span's shortfall with no weights goes to the last track
  (GridBag's quirk; evenly was the alternative); cells may overlap, drawn in the order added; no minimum sizes, so too
  little space is taken from the weighted tracks first, then from all in proportion.
- Pixels left over from any sharing in proportion are handed out or taken one at a time, each to the largest track
  that hasn't had one yet, ties to the first (the user, 2026-10-17).
- In a border layout the top left component sits at the bottom right of its cell (the user, 2026-10-19); the other
  outer cells' alignments, towards the centre, were extrapolated from it.
- Every container clips its children to its container bounds unless told not to; a scroll pane always clips; a point
  outside a clipping container's bounds finds the container (the user, 2026-10-22).
- A label draws its text at its top left; it is aligned only by the container it is in (the user, 2026-10-20).
- A container measures a container child without a desired size by its required size, in the fill, the flow and an
  unweighted grid track (2026-11-07, from the WindowLib review); a container without a desired size still fills the
  area it is laid out in.
- Scroll panes are IntelliJ IDEA's: translucent thumbs over the content; both axes, each with a policy of as needed or
  never; a bar shown whenever its content overflows, with no fading; dragging the thumb and
  [`CScrollPane::SetScrollPosition`]; a press on the track jumps the thumb there and drags it; the wheel goes to the
  component under the pointer (the user, 2026-10-21).

## Not done

- Container.txt's rule that a container shrinks its variable sized components first and, when even that isn't
  enough, makes every component smaller in proportion is only partly built.  A grid shrinks its weighted tracks first,
  then all of them in proportion, but a fixed size component in a shrunk cell keeps its size and is clipped; a fill
  container never makes a fixed size component smaller.  Shrinking variable components first, with whether a component
  is variable a property of its content (likely on [`CCanvasDraw`]), was proposed on 2026-10-17 and not built.
- Two grid choices are still the user's to confirm: a span's shortfall with no weights going to its last track rather
  than shared evenly, and whether components should have a minimum size.
- [`CScrollPane::SetScrollPosition`] invalidates the scroll pane's layout and content, so scrolling draws the whole
  pane again.
- The scroll pane's margin (2 pixels) and wheel step (48), and the least a split container's sash leaves either side
  (40 pixels, a `#define` in [`WindowLib/SplitContainer.h`]), are set in code rather than read from the theme, against
  the ruling that every pixel size is the theme's (see [WindowLib](index.md#decisions)).
- Containers aren't saved: [`CFillContainer::Save`], [`CFlowContainer::Save`], [`CGridContainer::Save`] and
  [`CScrollPane::Save`] return false.

# Docking

WindowLib has Visual Studio's docking: movable, sizable panes that can be tabs in a group, split beside other groups,
floated in windows of their own, collapsed to tabs on the main window's edges, or hidden, and dragged between all of
these with docking guides showing where they would go.  The layout is written to and read from an XML file.  It looks
like IntelliJ IDEA's Islands theme.  The HCP65 IDE and LogiCim use it: their tool windows and editors are panes.

## Words

| Word | Means |
|---|---|
| Pane | One dockable thing ([`CDockPane`]): a component (its content), an ID unique in the manager, a title and an icon.  A tool pane ([`DPK_Tool`]: Solution Explorer, Build) or a document pane ([`DPK_Document`]: an editor's file, a diff). |
| Group | Panes shown as tabs, one at a time ([`CDockGroup`]).  A tool group has a title bar and its tabs along the bottom, none while it holds one pane; a document group has its tabs along the top and no title bar. |
| Split | Two or more nodes side by side or one above the other, with a sash between each pair ([`CDockSplit`]). |
| Document well | The one place documents live in the main window ([`CDocumentWell`]), with its own tree of splits and document groups.  Tool panes dock around it.  It is always there, even empty. |
| Dock area | The root of one window's tree ([`CDockArea`]) and, in the main window, the four auto hide strips. |
| Floating | A pane or a whole group in a window of its own ([`CDockFloatingWindow`]), which holds a dock area of its own, so panes can be docked together in it. |
| Auto hide | A pane collapsed to a tab on one of the main window's edges ([`CAutoHideStrip`]); resting on or pressing the tab slides it out ([`CAutoHideFlyout`]). |
| Hidden | A closed tool pane.  It keeps where it was, so showing it again puts it back. |

[`EDockPaneState`] is a pane's state: [`DPS_Docked`], [`DPS_Floating`], [`DPS_AutoHidden`], [`DPS_Hidden`].

## Using it

An application keeps one [`CDockManager`] (it is an object, held from the root), puts its dock area
([`CDockManager::GetArea`]) in its window, under its menu bar and toolbar say, adds its panes and listens for what
happens to them:

```{.cpp .no-check}
pManager = OMalloc<CDockManager>(pWindow, pTheme, pTurnedFont);
sPlace.BesideWell(DS_Left, 300);
pManager->AddPane("SolutionExplorer", "Solution Explorer", pIcon, pTree, DPK_Tool, &sPlace);
pManager->SetListener(&cListener);
pManager->LoadLayout("Project.layout.user");
```

[`CDockManager::AddPane`] takes the pane's default place ([`SDockPlace`]): beside the document well on a side with a
size ([`SDockPlace::BesideWell`]), in the group of another pane ([`SDockPlace::WithPane`]), in the well
([`SDockPlace::InWell`]) or hidden.  The turned font, made with [`FT_Clockwise`], writes the left and right auto hide
tabs top to bottom ([`CTurnedLabel`]); without one they are upright.

Everything that moves panes goes through the manager, so the panes' menus, title bars, tabs, drags, the layout file
and the application all do the same things:

| Call | Does |
|---|---|
| [`CDockManager::Show`] | Puts the pane back where it was (docked, slid out, or its floating window raised) and selects its tab. |
| [`CDockManager::Activate`] | Shows it and gives it the keys. |
| [`CDockManager::Hide`] | Hides a tool; it remembers where it was.  A document is closed. |
| [`CDockManager::Close`] | Closes a document if the listener lets it, or hides a tool. |
| [`CDockManager::CloseMultiple`] | IntelliJ's Close All, Close Others, Close Unmodified, Close Tabs to the Left and Right. |
| [`CDockManager::Float`], [`CDockManager::FloatGroup`] | Floats a pane, or a whole group, where it last floated. |
| [`CDockManager::Dock`] | Docks a pane back where it was. |
| [`CDockManager::DockAsDocument`] | Puts a tool in the document well as a tab. |
| [`CDockManager::AutoHide`], [`CDockManager::AutoHideGroup`] | Collapses a pane, or a group, to the edge of the main area nearest its group. |
| [`CDockManager::DockPaneAt`] | Docks at a target ([`SDockTarget`]): an area's edge, beside or into a group, beside or into the well. |
| [`CDockManager::SaveLayout`], [`CDockManager::LoadLayout`] | Writes and reads the layout file. |
| [`CDockManager::ResetLayout`] | Every pane back to its default place; floating windows close. |

The listener ([`CDockManagerListener`]) hears that a document is about to close (and can keep it, for unsaved
changes), that a pane closed, that the active pane or the document in front changed, that the layout changed (to save
it later), whether a document is modified (for Close Unmodified), and is asked to make a pane a layout names that
isn't added ([`CDockManagerListener::CreatePane`]).

## Inside

[`CDockManager`] is a facade.  What docking shares is in a [`CDockState`] the manager holds: the main window, the dock
area and its well, every pane, the floating windows, the next group ID, the listener, and the parts that do the work:

| Part | Does |
|---|---|
| [`CDockTree`] | Edits the trees: docks a node at a target or beside another, takes one out, and keeps them tidy. |
| [`CDockRestorer`] | Records where a docked pane was as it leaves and puts it back there. |
| [`CDockMover`] | Moves panes and groups between docked, floating and auto hidden; makes and places floating windows. |
| [`CDockFocus`] | Keeps the active pane and document, following the focus, and opens and closes the flyout. |
| [`CDockDrag`] | Drags, with [`CDockGuides`] and [`CDockDragTracker`]. |
| [`CDockLayoutWriter`], [`CDockLayoutReader`] | The layout file. |

The parts hold the state, not each other, and there are no friend classes.  A pane keeps its own place: its state,
group, default place, restore record, floating rectangle and auto hide side and size.

A pane's content is a child of whichever group, flyout or floating window shows it; it is never in two places.  When
a pane moves to another window its content moves with [`CComponent::MoveToWindow`], which also moves what the content
points to that belongs to the old window but isn't in its tree (pages of tabs not shown, menus, scroll bars).

### Trees and sizes

A dock area's root is a node ([`CDockNode`]): a split, a group or the document well.  A node's dock size is what it
wants along its parent split, in pixels.  In a split holding the document well, the node with the well takes whatever
the others leave, so when the main window grows or shrinks the tool groups keep their sizes and the documents get the
change.  In a split without the well every node shares the space by its size.  No node is made smaller than
`DOCK_MINIMUM_SIZE` (40) while there is room.

A split is never left with one node (it is replaced by that node) nor holds a split of its own direction (that is
merged into it); [`CDockTree::Tidy`] keeps them so after every edit, and the layout reader does the same as it reads.

Sashes resize live: each pointer move lays the tree out again, which is cheap because layout is cached.  A sash moves
the space between its two neighbours only, and every node's dock size becomes its laid out size, so the others don't
move.

### Groups and tabs

A group reuses [`CTabbedContainer`], which has what docking needed: tabs along the bottom
([`CTabbedContainer::SetTabSide`]), no tab strip while there is one tab, a drag event when a tab is pulled a few
pixels, a close request event the group answers, and an overflow menu.  Tabs that don't fit a document group's bar go
to the overflow menu, and a selected tab that doesn't fit moves to the front.  The group maps panes to tabs by page,
because the tabbed container moves tabs itself.

A tool group's [`CDockTitleBar`] shows the selected pane's title, highlighted while the active pane is in the group,
and three buttons: the window position menu's chevron, the pin (auto hide) and the close cross (hide).  The window
position menu ([`EDockMenuItem`]) has Float, Dock, Dock as Tabbed Document, Auto Hide and Hide for a tool; Float, Dock,
Close and the Close Multiple sub menu for a document.  The same menu is every tab's context menu.  A group has one
menu for its documents' tabs and one for its tools' (a document group can hold tools); the pane it acts on is the
menu's opener ([`CMenuContainer::GetOpener`]), valid only while the menu opens, so the menu holds no tab alive.  Items
are told apart by tag, not text.  [`CDockManagerListener::DocumentMenuCreated`] lets the application add its own items
to the documents' menu.

Titles and icons follow the pane ([`CDockPane::SetTitle`]), so a renamed file's tab and title change with it.

### Activation and keys

Clicking in a pane, or focusing a component in it, makes it the active pane ([`CDockManager::WatchFocus`] follows the
focus in the manager's windows).  The active tool group's title bar is highlighted (IntelliJ blue in the Jewel
theme, [`TC_DockActiveTitleBar`]) and the active document's tab is drawn selected.  Shift+Escape
([`CIC_HideToolWindow`]) hides the active tool and Control+F4 ([`CIC_CloseDocument`]) closes the active document; the
dock area and the flyout take them.

### Dragging

A tab or title bar pulled `DOCK_DRAG_DISTANCE` (6) pixels floats what it pulls straight away, as Visual Studio does,
and every drag is then of a floating window ([`CDockManager::BeginPaneDrag`], [`CDockManager::BeginGroupDrag`],
[`CDockManager::BeginFloatingDrag`]).  A [`CDockDragTracker`], outside any window's tree, takes the rest of the drag
from the tab or title bar that started it ([`CFocus::HandOverDrag`]), so the drag survives the tab being taken off its
group.  The window follows the pointer, held where it was grabbed, and the window under the pointer shows its guides:

- an edge marker in the middle of each of the dock area's edges, to dock along that whole edge;
- a compass over the group under the pointer: its centre tabs into the group, its arrows dock beside it;
- over the document well a second ring of arrows: the inner ones split the document group, the outer ones dock beside
  the whole well;
- a group's title bar and tabs are its centre guide too;
- a translucent preview ([`TC_DockPreview`]) of where the guide under the pointer would put what is dragged, at the
  size it would have.

A document can only go into the document well, or float: over tool groups its compass has no targets.  A tool can go
anywhere.  Dropping on a guide docks there; dropping anywhere else leaves the window floating where it is.  Holding
Control keeps the guides away, to float over a dock area without docking.  Escape puts a torn off pane back where it
was.  Double clicking a docked group's title bar or a document tab floats it; double clicking a floating window's title
docks it back where it was last docked ([`CDockManager::DockFloating`]).

The guides are drawn in the dock area's window, which the floating window being dragged would cover.  So while guides
show, the dragged window shrinks to its title, `DOCK_DRAG_SMALL_WIDTH` (200) wide, beside the pointer, and has its
size back away from them or if it is let go there.  The guides are drawn in code ([`CDockGuides::DrawGuides`]) in the
theme's [`TC_DockGuide`] colours, at the sizes of the [`TM_DockGuideSize`] metrics.

### Floating windows

A floating window is a tool window ([`CWindow::ShowOwned`]): it stays over the main window, has no task bar button,
closes with it, and is run by the main window's loop.  It has no platform frame: WindowLib draws its border, which
resizes it ([`CDockEdge`], with the pointer's resize shapes), and, unless it holds a single tool group whose title bar
does the job, a title bar that moves it, docks it back on a double click and closes it.  A frame moved by the window
manager would be a drag WindowLib never sees, so it couldn't show the guides; drawing the frame works the same on X11
and Windows.

The main window's component input follows whichever of its windows is active ([`CWindow::ClaimInput`]).  A dialog
shown over a floating window disables the main window and all its tool windows.

A floating window is made at the screen rectangle its pane last floated at, or where its group was, the size it had
docked, and kept on the screen when the screen's area is known ([`CDockMover::KeepOnScreen`]).

### Auto hide

[`CDockManager::AutoHide`] takes a pane, or with [`CDockManager::AutoHideGroup`] a whole group, to the strip on the
main area's edge nearest its group (a group touching two edges goes to the one it runs along).  A strip shows while it
has tabs.  Resting the pointer on a tab or pressing it slides the pane out over the dock area as a flyout: a popup with
a title bar (pin and close) and an edge on its inner side that resizes it.  The flyout slides back when the keys go
somewhere outside it or there is a press outside it.  The pin docks it back.

### Restoring

When a docked pane leaves its group, to float, to auto hide or to be hidden, it records ([`SDockRestore`]):

1. the group's ID and its tab's index (every group has an ID unique in the manager, written to the file);
2. a neighbour: another pane in that group, or failing that a pane beside the group, and the side;
3. its default place.

Putting it back tries them in order: the group if it still exists; else beside or with the neighbour, if the
neighbour is docked; else, if the neighbour isn't docked either, where the neighbour would go back to, followed up to
`DOCK_RESTORE_DEPTH` (4) neighbours; else the default place.  A pane beside the document well records the well as its
neighbour (`DOCK_WELL_NEIGHBOUR`), and one beside a node holding the well and more records that
(`DOCK_AROUND_WELL_NEIGHBOUR`); restoring climbs from the neighbour to the node in a split of the same direction and
docks beside that ([`CDockTree::ClimbToSplit`]).  A floating group docked back gives up its own ID so its panes go
back to their group, rather than each making one.

## Layout file

The layout is XML, written with [`CXMLWriter`] and read through [`CXMLFile`] and [`CMarkupTag`], as Visual Studio keeps
a project's per user settings beside it.  The application chooses the file; the HCP65 IDE keeps
`<workspace>.layout.user` beside its workspace file (or solution or project), never committed.

```xml
<DockLayout Version="1" NextGroup="6">
  <MainWindow X="40" Y="30" Width="1600" Height="1000">
    <Split Direction="Horizontal">
      <Group ID="1" Kind="Tool" Size="300" Selected="SolutionExplorer">
        <Pane ID="SolutionExplorer"/>
      </Group>
      <DocumentWell>
        <Group ID="2" Kind="Document" Selected="Document:Main.cpp">
          <Pane ID="Document:Main.cpp"/>
        </Group>
      </DocumentWell>
    </Split>
    <AutoHide Side="Right">
      <Pane ID="Claude"/>
    </AutoHide>
  </MainWindow>
  <FloatingWindow X="1700" Y="100" Width="400" Height="600">
    <Group ID="5" Kind="Tool" Selected="Registers">
      <Pane ID="Registers"/>
    </Group>
  </FloatingWindow>
  <Panes>
    <Pane ID="Output" State="Hidden" RestoreGroup="1" RestoreIndex="1" RestoreNeighbour="SolutionExplorer"
          RestoreSide="Centre"/>
  </Panes>
</DockLayout>
```

It has only layout: windows, splits, sizes, groups, which panes are where and selected, auto hidden panes, and in
`<Panes>` every pane's state, where it goes back to, where it last floated and its auto hide edge and size.  It has
no pane's contents (an editor's caret, a tree's expansion); an application that wants those writes them itself.  A
node's size is its dock size along its parent split, left out when 0.  Positions are screen pixels of client areas.
The comment at the top of [`WindowLib/DockLayoutFile.cpp`] is the format's reference.

Panes are named by ID.  Reading one that isn't added asks the listener to make it (the IDE opens the file for
`Document:<path>`); if it doesn't, the pane is dropped and the layout around it closes up.  Panes added but not in
the file go to their default places.  A file that doesn't read, or is of a later version than the reader knows,
changes nothing and the reader returns false.  The main window's rectangle is applied only when the layout is read
before the window is shown; floating windows are kept on the screen when its area is known.

## Look

Docking looks like IntelliJ IDEA's Islands theme (its default since 2025.3).  Each group, the empty document well and
an auto hide flyout is an island with a background of its own ([`TC_DockIsland`], the editor's colour) and corners
rounded ([`TM_DockIslandRadius`], 10 as IntelliJ's) by a [`CRoundedCorners`] laid over it, which fills what lies
outside each quarter circle with the dock area's background and takes no clicks.  The area's background
([`TC_DockBackground`], the toolbar's grey in the Jewel theme) shows between the islands: the sashes are its colour
([`TC_DockSash`], [`TM_DockSashSize`] 5, IntelliJ's gap) and the main area keeps a margin of it round its tree, inside
the auto hide strips ([`TM_DockIslandMargin`]).  Inactive title bars are the island's colour; the active one is
highlighted.  The code editor's gutter is the editor's colour ([`TC_Gutter`]), so the editor's island is one colour to
its edge.  A [`CSplitContainer`] inside a pane has its own sash ([`TC_SplitSash`], [`TM_SplitSashSize`]), so it isn't
a gap.  Floating windows have no margin; a floated group's corners are rounded inside the window's frame.

Every colour and size is the window theme's ([`CWindowTheme::GetColour`] with the `TC_Dock` colours,
[`CWindowTheme::GetMetric`] with the `TM_Dock` metrics).

## Tests

[`TestWindowLib/TestDocking.cpp`] drives the API, compares the layout files written, and feeds drags in as pointer
events: defaults, hide, float and auto hide, drags onto every kind of guide, the shrunk window over guides, splits
merged, menus, keys, a dialog over a floating window, moving content between windows, the layout file, Close Multiple,
docking a floated group back, and the islands' gaps and corners (with partial redraw verified).  Its tests set the
island margin to 0 so older position checks hold, except the islands test.  Real drags were checked on 2026-11-08 in
the HCP65 IDE under Xephyr driven by XTest, with no window manager: tearing off tabs and title bars, the floating
window following the pointer out of the main window and back, drops on edge markers, compass arms and centres and
outside the main window, Escape, Control, live sash drags, the floating border, auto hide and double clicks.

## Decisions

- **Visual Studio's docking**: guides (the compass and edge markers) while dragging, tool window title bars with the
  window position menu, pin and close, tool tabs along the bottom of their group, document tabs along the top (the
  user, 2026-11-05).
- **The layout is kept as XML** through [`CXMLWriter`] and [`CXMLFile`], **not with [`CObject`] Save and Load**, as
  Visual Studio keeps its per user state beside a project (the user, 2026-11-05).  Components still save and load
  as before; that isn't how a layout is kept.
- **Sashes resize live**, since layout is cached (the user took the recommendation, 2026-11-05; the fallback was
  Visual Studio's bar that resizes on release).
- **Floating windows have no platform frame**; WindowLib draws it, so a floating window's drag is one WindowLib sees and
  can show guides for, alike on X11 and Windows (the user took the recommendation, 2026-11-05).
- **Documents are in the layout file**, reopened by the application through the unknown pane event, as Visual Studio
  reopens a solution's files (the user took the recommendation, 2026-11-05).
- **Documents can float** (the user, 2026-11-05).
- **A pulled tab or title bar floats straight away** and every drag is of a floating window, as Visual Studio does;
  Escape docks a torn off pane back (2026-11-05).
- **The layout file lists every pane's state and restore place in one `<Panes>` list** rather than separate hidden and
  restore lists (2026-11-05).
- **The manager's shared state is a [`CDockState`] holding the parts; no friend classes; panes keep their own placement
  records** (the user chose this option, and agreed panes keep their records, 2026-11-07).
- **A dragged floating window shrinks to its title while guides show** (2026-11-09), so the guides drawn in the dock
  area's window aren't hidden under it.  The other ways were a translucent dragged window (needs a compositor on
  Linux) and guides in top-level windows of their own.
- **Docking looks like IntelliJ IDEA's Islands** (the user, 2026-11-23: "make it similar to IDEA").
- **The active tool group's title bar stays highlighted blue**, though IntelliJ's islands have no highlighted header
  (the user, 2026-11-23: keep it).
- **Not planned** (2026-11-05): Ctrl+Tab's window switcher, pinned tabs, the preview tab, tab colours by project; more
  than one main window or document well; saved named layouts (the file format allows them later).

The plan this was built from, `WindowLib/Docking.txt`, is in the history up to commit 7ec3c7d5d.

## Not done

- Nothing has been run on Windows: the WinGDI side (frameless windows, cursors, pointer capture, screen rectangles) is
  written and compiles with MinGW.  The user has an 11-step checklist for trying it.
- A floating window keeps its docked size (cut to the screen when the screen's area is known), so one floated from a
  wide group is wide.
- [`CComponent::MoveToWindow`] finds what to move by a heuristic: it walks the content's pointers for components of
  the old window outside its tree, which could move too much or miss something.
- The order the manager, panes and windows are freed in isn't settled.
- The menu bar's colour (#27282E) differs from the toolbar's and the dock area's, unlike IntelliJ's Islands.
- Floating windows have no island margin.
- The flyout appears at once; Visual Studio slides it out.

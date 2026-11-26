# Components

WindowLib's widgets look and behave like IntelliJ IDEA's (JetBrains' Jewel dark theme).  Each is a [`CComponent`]
made with [`OMalloc`] and given its window, a [`CWindowTheme`] and its own content: a caption, a text, a ticked state.
A widget never takes images or colours of its own; the theme has them, so one theme makes every window of an
application look the same.  [`CJewelTheme::CreateDarkTheme`] makes IntelliJ's; a test puts a theme together from
other images.  How components are sized and placed is in [Layout](layout.md), how they draw in
[Drawing](drawing.md), and how keys, commands and focus reach them in [Input](input.md).

A widget made of parts (a button is a grid of border pieces, a face and a label) is *hit as a whole*: it, not its
parts, has the pointer over it and takes focus ([`CComponent::SetHitAsWhole`]).

## Labels

A [`CLabel`] is one line of text in one font.  It is a [`CCanvas`], not a bare component, because the text is drawn
by a [`CMapsCanvasDraw`], which draws onto a canvas.  Its required size is the laid out text's width by the font's
line height; a desired size set on an axis wins on that axis ([`CLabel::GetDesiredSize`]).  Line feeds in the text
are dropped.

The text is always drawn at the label's top left.  A label has no alignment of its own: put it in a
[`CFillContainer`] to centre it or right align it, as every other component is placed.

A label is a transparent canvas ([`CCanvas::SetTransparent`]), so the text is drawn over whatever is behind it.
Transparency is opt-in and canvases are opaque by default, because blending a canvas onto what is behind it is slower
than copying it.  A transparent canvas has an 8 bit opacity channel and keeps its colours multiplied by their
opacity, so only canvases with 8 bit colour channels can be transparent; it is cleared before each draw.  What is
behind a transparent canvas must be drawn again whenever the canvas is, or the translucent edges of its text build up
frame after frame; [Drawing](drawing.md) has how partial redraw keeps to that.

## Buttons

A [`CButton`] is a [`CImageBorderContainer`]: eight border pieces cut from [`CButtonImages`], laid out by
[`CBorderLayoutHelper`] around a face.  The face is filled with the state's face colour and holds the content, an
icon ([`CIconComponent`]) and a caption ([`CLabel`]):

| Content | Placed |
|---|---|
| Caption only, or icon only | Centred on the face. |
| Icon and caption | The icon on the left in a column the icon's width plus a gap ([`TM_ButtonIconGap`]), the caption left aligned beside it. |

The face needs the content's size plus the padding ([`TM_ButtonPaddingX`], [`TM_ButtonPaddingY`]) and stretches with
the button, so a button is never smaller than its caption but can fill a cell.

[`EButtonLook`] chooses the images from the theme: [`BL_Normal`] is the outlined button, [`BL_Small`] the outlined one
on small tiles without a focus ring (normal if the theme has none), [`BL_Flat`] a toolbar's flat button with only a
rounded block under the pointer.

### States and clicks

A button is in one of four [`EButtonState`]s, chosen in this order: [`BS_Pushed`] while pressed with the pointer over
it, [`BS_MouseOver`] while the pointer is over it (or it is highlighted, [`CButton::SetHighlighted`]),
[`BS_Focussed`] while it has focus, else [`BS_Normal`].  A button that doesn't show being pushed
([`CButton::SetShowsPushed`], a menu bar's) keeps the mouse over look while pressed.

A click is a press and a release both over the button; the release goes to the focussed component wherever the
pointer is, so the button checks the pointer is still over it.  Space and enter click a focussed button.  Losing focus
(the window deactivated, say) ends a press.  A click calls the button's click event ([`CButton::GetClickEvent`]),
and runs its command if it has one ([`CButton::SetCommand`]) and the command can run.  A button with a command has the
command's caption and keys as its tool tip, "Build (Control+Shift+B)"; one with a caption only shows the keys'
tip if the command has keys.

### Default and cancel buttons

A window can have a default button ([`CWindow::SetDefaultButton`]), clicked by enter, and a cancel button
([`CWindow::SetCancelButton`]), clicked by escape.  [`CFocus`] routes them: enter goes to the focussed component if it
wants it ([`CComponent::WantsEnter`]: a button does, a text field, check box and combo box don't), otherwise to the
default button; escape ([`CIC_ClearSelection`]) first closes any open popups, then clicks the cancel button unless the
focussed component wants escape ([`CComponent::WantsEscape`]).  While a normal button is its window's default it is
drawn with the theme's default images, IntelliJ's blue filled button; small and flat buttons have no default look
because the default images' tile size is the normal one's ([`CWindowTheme::GetDefaultButtonImages`]).

### Button images

[`CButtonImages`] holds each state's border as eight cels and a face colour per state (or one for all).  It is an
object buttons share.  Each state's border comes from a strip of eight square tiles, one tile wide, from the top
down: the top, right, bottom and left edges, then the top left, top right, bottom right and bottom left corners
([`EButtonBorderPiece`] is the order the pieces are laid out in).  Every edge has its outside along its top, so the
right, bottom and left edges are turned clockwise by one, two and three quarters.  The edges are stretched along the
button (a stretching [`CImageCanvasDraw`], [`CImageCanvasDraw::Init`]).

Every cel is a whole tile in size, but its transparent borders are cropped ([`CButtonImages::CropCel`]), so a band
that is empty in some states, such as the focus ring's, keeps its space without its empty pixels being blitted.
The corners are transparent outside their curves, so what is behind a button shows round them.

[`CJewelImageGenerator`] draws the Jewel strips at runtime with [`CImageDraw`] instead of loading them: 8 by 8 tiles
with, from the outside in, a two pixel focus ring (empty unless focussed), a one pixel border and the face, with
corners of radius six, four and three, each layer blended over the last.  It also draws the default (blue), small and
flat button strips (7 by 7 tiles), the check box images, chevrons, the close cross, the pin and the file chooser's
icons, so all of them follow the theme's colours.

The first images, from the old WindowLib's labels, are kept as test input: `Input/BlueButton/BlueBorder*.png` in
TestWindowLib, one 8 by 64 strip per state in the same layout, converted to RGBA PNGs with the pixels outside each
corner's curve transparent and its three anti-aliased curve pixels translucent.
[`TestWindowLib/TestBorderLayoutHelper.cpp`] builds buttons from them with [`CBorderLayoutHelper`] directly, with a
plain blue (115, 151, 255) face.

## Form widgets

[`CImageBorderContainer`] is what a button, a text field and a combo box are made of: a border drawn from
[`CButtonImages`] in one state ([`CImageBorderContainer::SetBorderState`]) around a padded face.  A text field and a
combo box use the outlined images: the focus ring while focussed, the mouse over look while the pointer is over them.
[`CImageBorderContainer::RemoveBorder`] leaves only the face, for a field put where there is no room for a border.

### Text field

A [`CTextField`] is one line of editable text.  The text is kept in a [`CTextDocument`] and edited by a single line
[`CTextEditing`] through the same input actions as a [`CTextArea`] ([`CTextEditingInput`]): moving and selecting by
character and word, home and end, deleting, cut, copy and paste through the window's [`CClipboard`], select all, and
undo (Control+Z) and redo (Control+Shift+Z) with typing grouped until a pause.  Line breaks pasted or set become
spaces.

The text is a [`CLabel`] over a transparent canvas drawn by [`CTextMarksCanvasDraw`], which draws the selection's
block; the caret's place is kept there too, but the window draws it, blinking, over what it paints
([`CTextField::GetCaretRect`]).  When the text is wider than the field it scrolls sideways to keep the caret in view.
The selection is hidden while the field hasn't focus.  A press places the caret at the nearest gap between
characters, a drag selects from there and a shift press selects from the caret.  The field doesn't want enter, so
enter clicks the dialog's OK.  Its width is a number of the font's average characters ([`CTextField::SetColumns`]),
not its text's, unless a desired width is set.  Its change event is [`CTextField::GetChangeEvent`].

### Check box

A [`CCheckBox`] is a grid of the box (an [`CIconComponent`]) and a caption.  Pressing and releasing over it, or space,
toggles it and calls its change event.  Its box keeps its focus ring while the pointer is over it, so unlike a
button's, focus and the pointer show together: [`ECheckBoxState`] has [`CBS_Normal`], [`CBS_MouseOver`],
[`CBS_Focussed`] and [`CBS_FocussedMouseOver`], and [`CCheckBoxImages`] a cel for each, ticked and not.

### Combo box

A [`CComboBox`] shows the chosen item and a chevron, and opens its list below itself.  The list is a
[`CMenuContainer`] in a popup as wide as the combo box, so it looks like the other menus, with the theme's combo menu
style (a narrower icon column, [`TM_ComboMenuIconWidth`], so the text lines up with the combo box's).  Pressing opens
or closes the list, space opens it, escape closes it, and up and down choose the item before or after while it is
closed.  Choosing another item calls the selection event ([`CComboBox::GetSelectionEvent`]).  It is as wide as its
widest item plus the chevron.

The combo box doesn't listen to its menu: a menu's item click calls [`CComponent::PopupItemClicked`] on the popup's
owner, found before the popup closes.  So nothing has to remove a listener when the combo box goes, which matters
because [`CBaseObject::Free`] must not touch other objects (objects are freed in chunks, and the menu may already be gone).

## Popups

A [`CPopup`] holds one component over everything else in the window: a menu, a combo box's list, a tool tip, an
auto hide flyout.  [`CWindow::OpenPopup`] makes one at a point with an owner; the window's canvas draws popups
after its container, so they are drawn over it and hit before it.  A popup is the size its content wants, moved back
inside the window if it would cross the right or bottom edge.  [`CWindow::ClosePopup`] closes a popup and every popup
opened after it, top down, then tells each owner ([`CComponent::PopupClosed`]).

A press on a popup or its owner leaves the popups open; a press anywhere else closes them
([`CWindow::ClosePopupsOutside`]).  A popup whose owner is large, like a context menu's page, closes on a press on the
owner too ([`CPopup::SetOwnerPressKeepsOpen`]).  A tool tip's popup lets the pointer through to what is under it
([`CPopup::SetPointerPasses`]).

A popup taller than the window doesn't squash its content.  One that scrolls ([`CPopup::SetScrolls`], as every menu
does) has a [`CScrollPane`] between it and its content, so the content keeps its height and scrolls; one with a
maximum height ([`CPopup::SetMaximumHeight`]) scrolls past that.  A popup that keeps its top ([`CPopup::SetKeepsTop`],
a menu bar's menus) is made shorter rather than moved up when there is too little room below it.  Popup corners are
square.

## Menus

| Class | Is |
|---|---|
| [`CMenuBar`] | A row of [`CMenuBarItem`] buttons along the top of a window, each opening a menu below it; at most one open at a time. |
| [`CMenuContainer`] | A menu: a [`CListContainer`] of [`CMenuItem`]s and separators, shown in a popup. |
| [`CMenuItem`] | One row: an icon column, the label, the shortcut (right aligned, in the dim font) and a chevron column if it opens a sub menu. |
| [`CMenuStyle`] | How a bar and its menus look, read from the theme as it is made; shared. |
| [`CContextMenu`] | A component's menu opened by the right button, the Menu key or Shift+F10 ([`CComponent::SetContextMenu`]). |

The icon and chevron columns are fixed widths ([`TM_MenuIconWidth`], [`TM_MenuChevronWidth`]) whether an item has an
icon or a chevron or not, so the labels of every item line up down the menu, as IntelliJ's do.  The row under the
pointer is selected and drawn with a rounded block inset from the menu's sides ([`CRoundedBoxCanvasDraw`], given to the
list by [`CListContainer::SetHighlightDraws`]); separators stop short of the sides.  The bar's buttons are flat, with a
small rounded block under the pointer.  The Jewel values ([`CJewelTheme::SetMetrics`]) copy IntelliJ 2026's: a 32
pixel icon column, a 24 pixel chevron column, radius 4 highlights inset 6 from the menu's sides.

Behaviour, as IntelliJ's:

- Pressing a bar button opens its menu, or closes it; while one is open, moving over another bar button opens that
  one's menu.  Space, enter and down open it with its first item selected.
- An item with a sub menu opens it to its right when the pointer rests on it, when it is clicked, or with right; left
  closes a sub menu and goes back to its parent.  On a bar's menu, left and right with no sub menu open move to the
  neighbouring bar menus.
- Clicking an item, or enter or space, closes every menu it was opened from, then calls the item's own click event
  ([`CMenuItem::GetClickEvent`]), then the item click event of the first menu
  ([`CMenuContainer::GetItemClickEvent`]) and of its bar if it has one.  An item made from a command
  ([`CMenuContainer::AddItem`] with a [`CCommand`]) runs it and shows its keys.
- A disabled item ([`CMenuItem::SetEnabled`]) is drawn in the dim font and can't be selected or clicked; the keys skip
  it.  The opening event ([`CMenuContainer::GetOpeningEvent`]) is called before the menu is laid out, so items can be
  enabled for what can be done now.  During that event only, [`CMenuContainer::GetOpener`] says what the menu is
  opening for: one menu can serve many owners (every tab of a dock group) without holding any of them.
- An item can carry a number ([`CMenuItem::SetTag`]) so a handler can tell items apart without matching their text.
- A bar too narrow for its buttons cuts the last ones off at its right edge; the buttons keep their sizes (they sit in
  a [`CFlowContainer`] that continues past the edge).
- A menu taller than the window scrolls; a bar's menu stays below the bar, cut to the space there; the wheel over a
  menu closes its open sub menu.

A window's text menu ([`CWindow::SetTextMenu`]) is a context menu of Cut, Copy, Paste and Select All for whichever text
area or field has the keys, each item enabled as that component can do it.

## Dialogs

Dialogs are real native windows, each with its own loop.  [`CWindow::ShowModal`] shows a window as a dialog over its
owner and returns when it stops: the owner (and, if the owner is a floating tool window, the main window and all its
tool windows) is disabled, gets no keys or pointer and isn't ticked, though it is still drawn; the owner's component
input comes to the dialog until it closes, and then focus goes back to the owner.

**A modal dialog is shown from a tick, never from an input action** (a key, a click, a menu item).  The input isn't
re-entrant: reading it while it is being read isn't supported.  Code that wants a dialog in answer to input posts
the call to the next tick ([`CWindow::Post`]) or sets a pending flag its tick reads, as [`CFileChooser`] does for
New Folder, Delete and "replace the existing file?".

| Class | Is |
|---|---|
| [`CDialog`] | A form in IntelliJ's style: a label and an editor per row, dividers with titles between groups, buttons along the bottom on the right.  Or a body of its own ([`CDialog::SetBody`]). |
| [`CMessageDialog`] | A message, a line per line feed, and buttons to answer it; optionally a list of items under it that scrolls past `MESSAGE_DIALOG_MAX_ITEMS` (10). |
| [`CProgressDialog`] | A line saying what is being done, a [`CProgressBar`] and Cancel, for work done on other threads. |
| [`CFileChooser`] | Open and Save, laid out as Swing's JFileChooser in its Metal look, with IntelliJ's toolbar icons. |

### CDialog

[`CDialog`] is an object holding its window: make it with [`OMalloc`] and keep it in the root, or in an object that is,
while it is shown.  It draws with an [`SDialogLook`], made once by its owner (the theme and a dim font for divider
titles).  Rows come from [`CDialog::AddTextField`], [`CDialog::AddCheckBox`], [`CDialog::AddComboBox`],
[`CDialog::AddEditor`] and [`CDialog::AddDivider`].

OK is the default button, Cancel the cancel button.  [`CDialog::SetOKCaption`] renames OK (Open, Refactor), which still
closes the dialog as OK; with NULL there is no OK and no default button, so only what the dialog waits for closes it
([`CDialog::Close`]).  [`CDialog::SetCancelCaption`] renames Cancel, and with NULL there is no Cancel and escape clicks
OK, as in a dialog that only tells something.  [`CDialog::AddExtraButton`] puts a button before OK (Don't Save).  The
buttons are at least [`TM_DialogButtonWidth`] wide, wider if a caption needs it.  An approver
([`CDialog::SetApprover`]) can keep OK from closing the dialog, as the file chooser opens a folder typed into it
instead.  [`CDialog::GetClicked`] says which button closed it ("OK" whatever OK's caption).

[`CDialog::Show`] sizes the dialog to its rows (or body) and buttons, no narrower than [`TM_DialogMinWidth`]; the
other overloads take a width, or a width and height.  A dialog can't be resized unless
[`CDialog::SetResizable`] gives it a minimum size.  In a dialog, tab and shift tab move the focus round the
components that can take it.

A dialog runs its own tick ([`CDialogTick`]); its ticker ([`CDialog::SetTicker`]) is called each tick, outside the
dialog's input, so a dialog can show another dialog over itself.

### Message and progress dialogs

[`CMessageDialog`] starts with OK and Cancel; its dialog ([`CMessageDialog::GetDialog`]) renames or removes them and
adds the extra button, as the IDE's Save, Don't Save and Cancel.  It sizes itself to its message, items and buttons.

[`CProgressDialog`] has Cancel and no OK.  While it is shown it calls its ticker ([`CProgressDialog::SetTicker`])
every `PROGRESS_DIALOG_TICK_MS` (15) milliseconds, asking for the next tick each time (a window only promises one);
the ticker reads how the work is going on its threads, calls [`CProgressDialog::SetText`] and
[`CProgressDialog::SetProgress`], and calls [`CProgressDialog::Finish`] once the work is done.
[`CProgressDialog::Show`] is true if the work finished and false if Cancel, escape or the close button closed it
first; stopping the work is then the caller's.  LogiCim's KiCad import was its first user.

### File chooser

[`CFileChooser`] has Look In (Save In when saving) with every drive and the folders down to the one shown, a toolbar
(Up One Level, Home, Desktop, New Folder, Delete, Refresh and a Show Hidden toggle, each with a tool tip), the folder's
folders then its files, and File Name and Files of Type ([`SFileChooserType`], the caller's array, which must outlast
the chooser) along the bottom.  Selecting a file puts its name in File Name; a double click or enter opens a folder or
chooses a file; Backspace goes up a folder unless File Name has the keys.  Open on a name that is a folder opens the
folder, as JFileChooser does.  Saving adds the chosen type's extension to a name without it, and asks before replacing
a file.  [`CFileChooser::SetChoosesFolder`] makes it choose a folder instead.  Its icons are the theme's, drawn by
[`CJewelImageGenerator::CreateFolder`] and the like.

[`TestWindowLib/TestFileChooser.cpp`] drives the chooser's handlers directly; [`TestWindowLib/TestDialog.cpp`] shows
dialogs, message and progress dialogs for real from a test window's tick.

## Toolbars and status bars

Both are [`CBarContainer`]s: components in a row (or a column) in the order they are appended, each its own size and
centred across the bar.  [`CBarContainer::AddStretch`] takes the space left over, so what is appended after it goes to
the far end; without one the space is after the last component.  An end gap stays after the last component (a filler
that moves along as components are appended).  Components go in with [`CBarContainer::Append`]; the grid's one
argument [`CGridContainer::AddComponent`] deliberately fails, since a grid needs a cell.

A [`CToolbar`] is a row, or a vertical stripe like IntelliJ's tool window bars, of command buttons
([`CToolbar::AddCommand`]: a flat button with the command's icon, or its caption if it has none) and any other
components, with separator lines.  A command's button runs it and has its caption and keys as its tool tip.
[`CToolbar::Refresh`], called by whoever knows a command changed and after each click of one of its buttons, shows a
command that is on (one with a checked state) highlighted, and one that can't run with its disabled icon if it was
given one.  Its sizes are the [`TM_ToolbarThickness`], [`TM_ToolbarInset`], [`TM_ToolbarSeparatorSpace`] and
[`TM_ToolbarSeparatorLength`] metrics.

A [`CStatusBar`] runs along the bottom of a window as IntelliJ's: fields of text ([`CStatusBar::AddField`], a width in
pixels or -1 for as wide as its text), separator lines, and the message field ([`CStatusBar::AddMessageField`]), the
stretch.  [`CStatusBar::ShowMessage`] shows a message for a time from now; it asks the window for a tick at the
end, and the application's tick calls [`CStatusBar::UpdateMessage`], which clears it.  Times are passed in, so tests can
drive them.  Its sizes are [`TM_StatusBarHeight`], [`TM_StatusFieldGap`] and [`TM_StatusSeparatorLength`].

## Tool tips

A window shows no tool tips until it is given a [`CToolTips`] ([`CWindow::SetToolTips`]).  When the pointer rests
`TOOL_TIPS_DELAY` (700) milliseconds on a component with a tip ([`CComponent::SetToolTip`], or an override of
[`CComponent::GetToolTip`] as [`CButton`]'s), or on one whose nearest parent has one, the tip shows under the pointer
([`TM_ToolTipOffset`]) in a popup the pointer passes through.  It goes when the pointer leaves the component; a press, a
key or the wheel takes it away until the pointer moves onto another component.  A component removed from the window
is forgotten ([`CFocus::Forget`] passes it on).  Times are passed in, as for the caret's blink, so the tests drive them
([`TestWindowLib/TestToolTips.cpp`]).

## More components

| Class | Is |
|---|---|
| [`CListContainer`] | Rows one of which is selected; what menus, combo boxes and lists are made of.  With [`CListContainer::SetMultiSelect`] several can be ([`CListContainer::IsRowSelected`]), Shift and Control with keys and pointer as a list box's, all drawn by one canvas under the rows (so components added later can't go under them). |
| [`CTreeContainer`] | A tree of rows, optionally with columns after the names. |
| [`CTreeTable`] | A tree whose rows have columns under captions, as Visual Studio's Locals and Watch. |
| [`CTable`] | Captions over a scrolling list of rows of cells. |
| [`CTabbedContainer`] | Pages under tabs, with a tab side, an overflow menu and tabs that can be dragged off (see [Docking](docking.md)). |
| [`CNumberField`] | A text field a number is typed into. |
| [`CCompletionField`] | A text field offering completions in a list under it. |
| [`CKeyCaptureField`] | A field showing the keys pressed in it, for a keymap. |
| [`CColourField`] | A colour chosen by its numbers: red, green, blue, hue, saturation, value and hex. |
| [`CProgressBar`] | A thin bar filling from the left. |
| [`CChooserPopup`] | A popup list to choose one of, filtered as you type, as IntelliJ's usages popup.  A driven one ([`CChooserPopup::SetDriven`]) leaves the keys with what opened it, which filters it, moves through it and chooses, and can add items while it is open ([`CChooserPopup::ItemsChanged`]): the IDE's code completion. |
| [`CInfoPopup`] | A few lines of text by something in the window, as a parameter hint.  It takes no keys and the pointer passes through it; with a maximum height ([`CInfoPopup::SetMaximumHeight`]) more text scrolls ([`CInfoPopup::Scroll`], [`CInfoPopup::ScrollPages`]), and taking the pointer ([`CInfoPopup::SetTakesPointer`]) the wheel scrolls it and a press on it calls its press event with the character pressed. |
| [`CPromptPopup`] | A popup asking for one line of text, as Go to Line. |
| [`CGoToDialog`], [`CFindInFilesDialog`], [`CPasteHistoryDialog`], [`CSettingsDialog`] | IntelliJ's Go to, Find in Files, Paste from History and Settings dialogs. |

## Decisions

- **A label is a canvas** (2026-10-20): the user asked for a label extending [`CComponent`] drawn with
  [`CMapsCanvasDraw`]; it extends [`CCanvas`] because that draw draws onto a canvas.
- **A label's text is drawn at its top left; alignment comes only from the container it is in** (the user,
  2026-10-20).
- **Transparency is opt-in; canvases are opaque by default**, for speed (the user chose this option, 2026-10-20).
- **A button is a container of parts**, not one canvas drawing itself; **a click is an event**, not a virtual to
  override; **its look's priority is pushed, then mouse over, then focussed, then normal** (the user, 2026-10-21).
- **A button image's cels are cropped to their opaque pixels but keep the tile's size**, rather than blitting empty
  pixels (the user, 2026-10-21).
- **A window has one default button clicked by enter unless the focussed component wants enter**, and a button drawn
  with default images while it is the default (the user's choices, 2026-10-21).
- **The Jewel images are drawn at runtime** ([`CJewelImageGenerator`], moved into WindowLib from the button test) and
  the PNG strips deleted (the user, 2026-10-21); their pixels matched the PNGs to within one level in a few
  focussed-state pixels, re-accepted by the user.
- **The old blue button images stay as PNG test input**, converted from BMP, and edges stretch through
  [`CImageCanvasDraw`]'s stretch option rather than a test's own class (the user, 2026-10-19).
- **Menus are [`CMenuBar`], [`CMenuContainer`] (a [`CListContainer`]) and [`CMenuItem`], with bar items that are
  buttons and item columns aligned by a grid** (the user named them, 2026-10-22).  CList, CMenu and CImageCelCanvas
  were renamed [`CListContainer`], [`CMenuContainer`] and [`CIconComponent`] at the user's request the same day.
- **Menus look like IntelliJ's**: a smaller Inter font and rounded highlights (the user, 2026-10-22), the highlight a
  [`CRoundedBoxCanvasDraw`] holding an image draw rather than a subclass of one (the user chose composition).
- **Popup corners stay square**, as IntelliJ's on Linux (the user, 2026-10-22).
- **Menus that don't fit on the bar are cut off on the right, not squashed** (the user, 2026-11-23).
- **Tall menus scroll in a scroll pane; a bar's menus stay below the bar**, cut to the space there, rather than move up
  (the user, 2026-11-23).
- **Every widget takes the theme and its content** in its Init, never images or colours of its own ("theme
  everywhere", the user, 2026-10-23).  The theme's own rules are in [WindowLib](index.md).
- **A combo box hears its list's clicks through [`CComponent::PopupItemClicked`] on the popup's owner**, not by
  listening to the menu, so [`CBaseObject::Free`] touches no other object (the user chose this over changing how
  objects are freed, 2026-10-23).
- **Dialogs are real native windows** (the user, 2026-10-23), shown modally from a tick, never from input, because
  input isn't re-entrant.
- **[`CDialog`] and [`CFileChooser`] live in WindowLib** (moved from the HCP65 tester, 2026-10-27); the chooser follows
  JFileChooser's Metal layout with IntelliJ's toolbar icons.
- **[`CMessageDialog`] and dock groups' shared tab menus** were the user's WindowLib cleanup items 5 and 6
  (2026-11-07).
- **Toolbars, status bars and tool tips are WindowLib classes** shared by LogiCim and the IDE (the user, 2026-11-08).

## Not done

- [`CButton`] has no disabled state.  A toolbar shows a command that can't run with its disabled icon, if it was given
  one, but a dialog's buttons and icon-less tool buttons can't be greyed.
- A menu can't be opened by pressing on a bar button, dragging to an item and releasing there.
- The HCP65 emulator, tester and WindowInputTester have no toolbars or status bars.
- Nothing here has been run on Windows: WinGDI dialogs, tool tips, toolbars and status bars are only compile checked
  with MinGW.

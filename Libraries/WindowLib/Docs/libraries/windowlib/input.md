# Input, Focus, Keys and the Clipboard

A key press, a click or a turn of the wheel reaches a component in two halves.  The native window says where the
pointer is, in the window's client space.  The keys, buttons and wheel come through InputLib, as one stream for every
keyboard and mouse, and WindowLib's [`CComponentInput`] says what each means: a key event, then a command
([`EComponentInputCommand`]) or a character.  The window's [`CFocus`] gives that to the focussed component, or the one
under the pointer, and up through its parents.  What no component takes runs the application's commands
([`CCommand`]), which menus and tool buttons are made from too.

The native side of this, per platform, is on the [Backends](backends.md) page.

## The window's input API

A native window drives its [`CWindow`] through these, all in window client space; nothing in them is the platform's:

| Function | Called when |
|---|---|
| [`CWindow::PointerMoved`] | The pointer moved over the window (or anywhere, while a button is held and the window has the pointer). |
| [`CWindow::PointerLeft`] | The pointer left the window. |
| [`CWindow::Input`] | A queued input event is to be translated and dispatched (from [`CComponentInput::Update`]). |
| [`CWindow::RepeatInput`] | A held key's command or printable is given again (see [Key repeat](#key-repeat)). |
| [`CWindow::Activate`] | The window became the active one. |
| [`CWindow::PlatformDeactivated`] | The platform's focus left it; it calls [`CWindow::Deactivate`] unless the window keeps active. |
| [`CWindow::InputCommand`], [`CWindow::InputKeys`], [`CWindow::InputPrintable`] | A test or script gives a command, keys ("Control+Shift+N") or a character without any device. |

They are named `InputCommand` and `InputPrintable` rather than `Command` and `Printable` because [`CWindow`] is a
[`CComponent`], and a `Printable(char, bool)` would override the component listener's virtual of that name.  Code that
feeds a window calls these, never [`CFocus`] directly.

[`CWindow::Activate`] gives the window the shared component input ([`CWindow::ClaimInput`]), looks at the system's
clipboard for text another program copied ([`CClipboard::Sync`]), gives the focussed component its focus back and
calls the activation event ([`CWindow::GetActivationEvent`]), as an IDE uses to see whether its files changed.
[`CWindow::Deactivate`] dismisses the tool tip, stops any drag and releases every key and button held
([`CComponentInput::ReleaseAll`]): releases that happen while another window has the platform's focus are never
seen.  Each native window sets the caps and num locks from the platform when it is activated
([`CComponentInput::SetLocks`]), as they can be pressed while another window has the keys.

A window driven by a script rather than a person can keep active whatever has the platform's focus
([`CWindow::SetKeepsActive`]): an inactive window takes no input, so otherwise another program's window in front of
it would make it ignore what its script gives it.  Its dialogs and tool windows keep active as it does.

## Setting up input

An application makes the input once, for its main window, in this order (the HCP65 IDE's `IDEApp.cpp` is an
example):

1. A [`CInput`], reading the device descriptions from WinInputLib's XML folder, with the platform's [`CNativeInput`]
   added ([`CWinInput`], X11Lib's input or WinRefLib's [`CWinRefInput`]).
2. A [`CWindowInputSource`] over it: every keyboard's, mouse's and controller's sources in one virtual device, so a
   chord can be made of keys on any of them.
3. A [`CComponentInput`] initialised with the source and the window, then [`CComponentInput::AddKeyboards`] and
   [`CComponentInput::AddMice`].
4. The application's keys bound ([`CCommandSet::BindKeys`]), then [`CWindow::SetComponentInput`].

The window doesn't own the component input; it is killed after the window stops showing, and before the source.  One
component input serves a window, its dialogs and its tool windows.  [`CWindow::ShowModal`] releases what is held and
hands it to the dialog until the dialog closes; a tool window shown with [`CWindow::ShowOwned`] shares its owner's,
and takes it ([`CWindow::ClaimInput`]) when it is activated or pressed.

[`CWindow::Show`]'s loop calls [`CComponentInput::Update`] and [`CComponentInput::Repeat`] each pass.  Update has the
source update InputLib and gives each queued [`SWindowInputEvent`] to the window.

## What a key means

[`CComponentInputDevice`] holds, for one keyboard or mouse, what its sources mean, found by their friendly names
("Left Shift", "Button 1"), so any keyboard or mouse that has those names works: keys (the modifiers, the locks,
Insert), printables (with their shifted and caps-locked characters), commands given on press or release, and the
numpad's keys standing in for others when num lock is off.  The standard meanings are WindowLib's own, in
[`CComponentInputDevice::InitKeyboard`], [`CComponentInputDevice::InitMouse`] and the table of chords at the top of
[`WindowLib/ComponentInput.cpp`]; the device XML only names the sources.

Modifiers only mean something in chords.  Control, Alt or Shift with a key is a chord named for its modifiers and key,
"Control+C" or "Control+Alt+Shift+N", made for each left and right modifier the keyboard has, and the standard ones
give a command (Control+C is [`CIC_Copy`]).  Shift and caps lock still choose a printable's case.  A key with Control
or Alt held that isn't a chord means nothing.  Control with the wheel gives the wheel's command with
[`SComponentInputEvent::bControl`] set, which a scroll pane passes on to its parents so that what has a zoom can zoom.

[`CWindowInputSource`] holds back an input that could start a chord, a modifier pressed alone, until the chord
matches (then only the chord is queued), stops matching, or, once let go, its eviction age passes (0.3 s unless the
chord says otherwise).  The window never sleeps past that age ([`CComponentInput::GetEvictionDelay`]).  A key after a
chord, "Control+Shift+F, H", is a sequence chord: the second key must come within 500 ms.

[`CComponentInput::Translate`] turns one queued event into at most two [`SComponentInputEvent`]s, in the order they
are offered:

| Queued | Events |
|---|---|
| A key or button of a keyboard or mouse added | Its key event ([`CIET_KeyDown`] or [`CIET_KeyUp`]), then the command or printable it means alone. |
| A chord | Its chord event ([`CIET_Chord`]), then its command, if it has one. |
| Movement of a mouse added | A movement event ([`CIET_Movement`]). |
| A key or button of another device | Its key event only. |

The wheel's commands become [`CIET_Wheel`] events for the component under the pointer.  The pointer's position is
never read from InputLib: the focus puts it on each event, relative to the top left of the component the event is
given to.

## Input actions

A component listens with input actions ([`CInputAction`], added with [`CComponent::AddInputAction`]): each has a
filter ([`EInputActionFilter`]: a named key down or up, any key, a named chord, any chord, a printable, a given
command, any command, movement, the wheel) and a member function called with the event, nothing, or the pointer's x
and y.  All of a component's actions that match are called.  An action that can't use the event sets
[`SComponentInputEvent::bPass`] so it goes on to the parent, and a disabled action ([`CInputAction::SetEnabled`])
matches nothing.

An action that goes first ([`CInputAction::SetGoesFirst`]) is called before the component's actions that don't,
whenever it was added, and when it takes the event those aren't called.  It is for something outside a component that
sometimes has a use for one of its keys: the IDE's Quick Documentation takes an editor's Page Up and Page Down to
scroll itself while it shows more than fits, and passes them to the editor otherwise
([`TestWindowLib/TestInputAction.cpp`]).

An action that keeps its component's layout, or lays it out again itself, says so
([`CInputAction::SetKeepsLayout`]); when every action that took an event keeps the layout, the window only paints.
A key that types a character also keeps it.  Anything else lays the window out again before the next paint.

## Focus

[`CFocus`] keeps the component under the pointer, the focussed component (the one with the keys), the component
dragging, and while a key is dispatched, the component under the pointer it is for, if that isn't the focussed one.

- A click focusses the nearest component under the pointer that can take the focus
  ([`CComponent::CanGetFocus`]); if there is none, as over the background, nothing has the focus.  Popups the click
  isn't in close first.  A click outside the client area, on the title bar, goes to no component.
- [`CFocus::SetFocus`] gives a component the focus, or takes it from everything (a menu does, to take the keys while
  it is open).  The component losing it is told after it has gone, so it doesn't still look focussed.
- An inactive window remembers its focussed component and gives it nothing; activating it gives the component its
  focus back.
- [`CFocus::Forget`] is called before a component is removed: nothing in it stays hovered, focussed or dragging.
- [`CFocus::GetFocusChangeEvent`] tells listeners when the focussed component changes, and [`CFocus::GetPressEvent`]
  when a press gives a component the keys, changed or not (a window's component pressed again after another window
  had the keys).
- The focussed component's caret blinks, shown then hidden every 500 ms as IntelliJ IDEA's does, and shown again
  whenever it moves, its text is edited or the focus changes ([`CFocus::ResetCaretBlink`]).  The window draws the
  caret over what it paints ([The Text Area](text-area.md)).

[`CFocus::DispatchEvent`] gives one event, in this order:

1. The window's own: Escape ([`CIC_ClearSelection`]) closes the open popups, or else clicks the window's cancel
   button unless the focussed component wants Escape ([`CComponent::WantsEscape`], as a list with a filter does);
   Enter clicks the default button unless the focussed component wants Enter ([`CComponent::WantsEnter`]).
2. A drag's start goes to the focussed component ([`CFocus::StartDrag`]); the rest of the drag, wherever the pointer
   goes, to the component that started it, or the one it was handed to ([`CFocus::HandOverDrag`], as a dock
   manager's drag tracker takes a tab's drag).  A select drag held still outside its view is told where the pointer
   is again every 50 ms, so the selection goes on scrolling.
3. Movement and the wheel go to the component under the pointer, then up its parents.
4. Everything else goes to the focussed component, or the component under the pointer if the key was read in its
   context (see [Contexts](#contexts)), then up its parents to the first with an action that takes it.
5. A command nothing took ([`CFocus::DispatchUnhandledCommand`]): keys bound to an application's command
   ([`CIC_Command`]) run it from the window's command set, or its owner's; Tab and Shift+Tab in a dialog move the
   focus to the next or previous tab stop ([`CComponent::IsTabStop`]) in the window's tree, round from the last to the
   first; otherwise the window's content is offered it, then the content of the window a tool window floats over, so
   an application's keys work wherever the keys are.

Of the two events a press makes, the second is only given if the first wasn't taken: a component that takes a key's
key event stops it being a command too.

### Key repeat

A held key repeats in WindowLib, not from the platform: InputLib gives a held key one press however often the
platform sends it.  With [`CComponentInput::SetRepeat`] (off until it is called; the IDE's settings give a delay and
a rate), once a key has been held for the delay, the command or printable its press gave is given again to the
component that took it, each period, once however late the window is.  Only that is given again, never the key or
chord event, so a component that reads keys as they go down and up (a game's, the emulator's keyboard) sees one press
and one release.  It repeats only if the press's command or printable was what was taken, the command moves or edits
a step at a time ([`CComponentInput::IsRepeatCommand`]: the caret's moves, Page Up and Down, Backspace, Delete,
Enter, Tab, Undo; not Copy or a window's command), and the component allows it ([`CComponent::SetKeyRepeat`]).  Any
other key or button, the keys going to another component, the window changing and
[`CComponentInput::ReleaseAll`] stop it.

## Commands

A [`CCommand`] is something the application does, Build or Step Over: a name (how scripts and menus find it, such as
"Debug.StepOver"), a caption, an icon, its keys, the function it runs, and optionally whether it can run now
([`CCommand::SetEnabledCheck`]) and whether what it turns on and off is on ([`CCommand::SetCheckedCheck`]).  Several
commands can run one function that is given the command run, as an application's table of actions does.  A command
that can't run does nothing when its keys are pressed; the key isn't taken.

The commands are in a [`CCommandSet`], given to the window with [`CWindow::SetCommands`]; dialogs and tool windows use
their owner's.  Menu items and tool buttons are made from commands, so a menu shows the keys that really run its
command and is greyed out when the command can't run ([Components](components.md)).  The keys an event carries are
named the one way [`CComponentInput::NameKeys`] names them, modifiers in the order Control, Alt, Shift, so the
command set finds them whatever order they were written in.  Menus show keys as IntelliJ IDEA writes them,
"Ctrl+Shift+Left" for "Control+Shift+Cursor Left" ([`CComponentInput::AppendShortcutText`]).

A command can stand for a component's standard command ([`CCommand::SetStandardCommand`]), as the Edit menu's Copy
stands for [`CIC_Copy`]: it shows that command's keys, and with [`CCommand::SetGivesStandard`] its keys give the
standard command for the focussed component to take, rather than running it.

[`CEditCommands`] adds an Edit menu's commands to a window's set: Undo, Redo, Cut, Copy, Paste, Paste from History
and Select All (Edit.Undo and so on).  They act on the component the keys last went to that can take them, not on
the menu that has the keys while it is open: running one gives that component the keys again and the standard
command, and each can run while it would do something there ([`CComponent::CanDoCommand`]).  That component can be
in a tool window floating over the window; its window is made the active one again ([`CWindow::RequestActivate`])
and the command runs there.  Once it has gone, they act on [`CEditCommands::GetDefaultTarget`]'s (an IDE's active
document).  For an application whose user can change keys, [`CEditCommands::AddKeymapCommands`] gives every standard
action an editor or window has a command, so all of them can be changed in the keymap.

The standard keys follow IntelliJ IDEA: Redo is Control+Shift+Z, Control+Y deletes the line, Control+X with nothing
selected cuts the line, and Control+Shift+V is Paste from History.

## Keys and contexts

What a key does depends on where it is pressed.  The words:

| Word | Meaning |
|---|---|
| Context | Where a key's binding is looked for: the global keys (the whole window), or a named part of the window, a code editor's ("Editor") or LogiCim's circuit canvas ("Circuit").  Contexts nest. |
| Binding | A key given to a command in a context.  A command can have keys in more than one: R over the circuit and Control+R everywhere. |
| Bare key | A key without Control or Alt, which types a character in a text field.  Only a typing context can bind one. |

### Contexts

The global keys are bound with [`CComponentInput::BindKeys`] and no context.  They are given to every keyboard, in
place of whatever the keys gave before ([`CComponentInput::UnbindKeys`] gives that back), so a global key that types
can't be bound: it would stop typing everywhere.

A context ([`SComponentInputContext`], made with [`CComponentInput::AddContext`]) has a name, a parent, its bindings,
whether it is a typing context, and whether it is an editor context.  A key read in a context is looked for in its
bindings, then its parent's, out to the global keys and then the standard keys; the innermost binding wins.  Its
bindings aren't put on the keyboards: they are looked up as the key is translated
([`CComponentInput::GetContextCommand`]), and a chord only a context binds is made on the keyboards giving nothing
elsewhere.  In a typing context a bound bare key gives its command instead of its character (a key typed with Shift
is bound as "Shift+A"); an unbound one still types.  Once an editor context has bindings, the standard editor
commands (caret, selection, editing; [`CComponentInput::IsEditorCommand`]) are its own: one it doesn't bind gives
nothing, so a key moved off Duplicate really stops duplicating in the editor.

The Editor context is always there ([`CComponentInput::Init`] adds it, an editor context inside the global keys).
LogiCim adds "Circuit", a typing context inside the global keys.

### Which context a key is read in

A component names its context ([`CComponent::GetKeyContext`]; a text area's is Editor), or has its parent's
([`CFocus::GetKeyContext`] walks up), and says whether it takes typing ([`CComponent::TakesTyping`]: text fields and
text areas).  [`CFocus::FindKeyComponent`] picks the context for each key:

- In a dialog, always the global keys, so what a user does to a context's keys can't stop a dialog working.
- Over a component in a typing context, while the focussed component doesn't take typing, the hovered component's
  context, and the key's event goes to the hovered component: A over LogiCim's circuit places an AND gate there
  while the component list has the focus, but typed in a text field it is an A.
- Otherwise the focussed component's context.

Keys a test gives ([`CWindow::InputKeys`]) are read in the context a press of them would be.

### Commands' keys in contexts

A command keeps its keys as a [`CCommandKeyList`], each key with its context, three times over: the keys it has, the
keys it was made with (so a user's changes can be told and reset) and the keys it last bound (so they can be
unbound).  The contexts it may have keys in are its own ([`CCommand::AllowContext`]); the first, the global keys
unless [`CCommand::SetContext`] says otherwise, is where keys given without a context go.  [`CCommand::Bind`] binds
each key in its context, as [`CIC_Command`] or the command's standard command.

A key belongs to one command in each context.  [`CCommandSet::FindKeys`] finds the command with keys in a context,
and [`CFocus`] runs the one bound in the context the event came from (the event's
[`SComponentInputEvent::szContext`]).  A context's key shadows a global one only in that context.

A menu item or tool tip shows a command's first global key, else its first key in any context, plain: Rotate Left
shows "R", not "R (Circuit)" ([`CCommand::GetKeys`], [`CCommand::AppendKeysText`]).  Lists of keys, in the keymap and
LogiCim's component chooser, say where each works unless that is everywhere or the editor: "R (Circuit), Ctrl+R"
([`AppendKeyListText`]).

### The keymap file

A user's keys are kept in their own file, only for the commands whose keys differ from those they were made with, so
a command added later comes with its own ([`CCommandSet::ReadKeys`], [`CCommandSet::WriteKeys`]).  A key says its
context where it isn't the command's first, so files written before contexts read as they were written:

```xml
<Keymap>
	<Command Name="Edit.Copy"><Keys>Control+Shift+C</Keys><Keys>Control+Insert</Keys></Command>
	<Command Name="Edit.RotateLeft"><Keys Context="Circuit">R</Keys><Keys>Control+R</Keys></Command>
	<Command Name="Debug.StepOver"/>
</Keymap>
```

A command with no keys there has had them all taken away.  If the file gives two commands the same keys in a context,
the first has them.  Commands whose keys aren't the user's ([`CCommand::SetKeptByKeymap`] false: LogiCim's
subcircuits', which the project keeps) are neither written nor read.  A command can also be left out of Settings'
Keymap's list ([`CCommand::SetListedInKeymap`] false) because its keys are given elsewhere (LogiCim's Place commands,
given in its component chooser); the page still knows its keys, so a key given there that it has moves from it, and
Reset All leaves its keys alone.

### Giving keys

Every place that changes a command's keys gives them through a [`CKeyGiver`], so the rules and what is said are the
same everywhere: Settings' Keymap, its Add Shortcut, and LogiCim's component chooser.  The giver works on key lists
([`CKeyListSource`]): the keymap page's copies of the commands' keys until OK, or a command set's own keys
([`CCommandSetKeys`], the chooser's).  It only changes the lists; whoever gave the keys binds them and saves them.

- [`CheckCommandKeys`] refuses keys the command can't have in that context: a context the command doesn't allow,
  keys it already has there, a key that types outside a typing context ("types a character: add Ctrl or Alt"), and
  outside the Editor context a key dialogs and text fields need ([`CComponentInput::IsWidgetCommand`]), as the global
  keys reach dialogs and text fields too.
- [`CKeyGiver::Describe`] says what giving them would mean: free, refused, moved from another command, or shadowing.
  Shadowing is said both ways and refused neither: keys given in a context that a global command has ("It shadows
  Open Project there."), and global keys an inner context's command keeps ("Editor Actions: Duplicate keeps it over
  the Editor.").
- [`CKeyGiver::Give`] asks before moving keys another command has in that context, with a message dialog, "Shortcut
  in Use": "Ctrl+Alt+A is Open Project.  Give it to AND Gate instead?", with Give and Cancel.  A check or a scripted
  edit answers it without showing it ([`CKeyGiver::SetAnswer`]).

### Settings' Keymap

[`CKeymapPage`] is a page of the settings dialog ([`CSettingsDialog`], [`CSettingsPage`]), as IntelliJ IDEA's Keymap:
every command, in the groups the application names by the part of a command's name before its dot ([`SKeymapGroup`];
the rest under "Other"), with a filter and each command's keys.  Its rows are copies of the commands' keys; Add
Shortcut, Remove, Reset and Reset All change the copies, and OK keeps them ([`CKeymapPage::Keep`]).  Keys are given
as they are added, so a clash asks then, not at OK.

Add Shortcut is a [`CKeyCaptureDialog`]: a [`CKeyCaptureField`] shows the keys pressed in it rather than typing them
(it has them before they can mean anything else, so a command bound to them doesn't run; plain Escape and Enter go on
to Cancel and OK), a Where combo box for a command that may have keys in more than one context, and what giving them
would mean, from its [`CKeyCaptureDescriber`].  OK isn't allowed for keys that can't be given.

For the checks, [`CKeymapPage::ApplyEdits`] does what a user would, separated by semicolons: `select:<name>`,
`add:<keys>` or `add:<context>:<keys>`, `capture:<keys>` (through Add Shortcut), `#!text remove`, `reset`, `resetall`,
`filter:<text>` and `show`.

### What was built differently from the keymap contexts plan

The keymap contexts were planned in WindowLib's `Keymap.txt` and built in nine steps on 2026-11-14 (the plan's last
version is in CommonLibraries cf0a585d1).  What was built differs from the plan in these ways:

- The key giver gives to key lists, not commands, and has Describe and Give only: removing and resetting stay the
  page's and the chooser's, and there is no listener telling the application what changed.  Whoever gave the keys
  binds and keeps them (the IDE's [`CIDESettings`], LogiCim's content).
- The plan's `CCommand::GetShownKeys` is [`CCommand::GetKeys`]; the plan's `SCommandKeys` is [`CCommandKeyList`]; a
  context owning its standard commands is a flag given to [`CComponentInput::AddContext`], not a list.
- The page shows a key's context after it except for the global keys and the Editor's.
- LogiCim's Place commands are captioned "Add <component>", as its toolbar's are, and every library component has
  one, the 74 series' too.  Its subcircuit commands are made for every circuit of the project, not only those that
  can go in the one being edited, and are in the Place group.  A subcircuit key dropped on loading because the user's
  keymap gives it to another command is dropped from the project too.  Its circuit file's `placeKeys` attribute is
  `<context>:<keys>` items separated by semicolons.
- Later the same day, Settings' Keymap stopped listing LogiCim's Place commands ([`CCommand::SetListedInKeymap`]):
  their keys are given in the component chooser.
- The chooser's Shortcut line is blank on a library's row, but its buttons stay, as components can't be hidden or
  disabled yet; with nothing to give to they say "Select a component first.".
- Along the way [`CDialog::Show`] came to measure its body (a long message was cut off), and [`CCommandSet::Remove`]
  was added.

## The clipboard

Each window has a [`CClipboard`] ([`CWindow::GetClipboard`]), all of them the system's one clipboard through a
[`CNativeClipboard`] the window's factory makes, so text copied in one window, or another program, can be pasted in
any.  The text is UTF-8 with line feeds whatever the system uses; the native clipboard converts it
([Backends](backends.md#the-clipboard-on-each-backend)).  Only text is copied.

Every text component cuts, copies and pastes through [`CTextEditingInput`] ([`CTextEditingInput::CutToClipboard`],
[`CTextEditingInput::CopyToClipboard`], [`CTextEditingInput::PasteFromClipboard`]), text areas and text fields alike.
A window's text menu ([`CWindow::SetTextMenu`]) gives text with no menu of its own Cut, Copy, Paste and Select All on
a right click; its dialogs and tool windows get it too.

- [`CClipboard::SetText`] puts text on the system's clipboard and makes it the history's newest entry, as a block if
  it was copied from a box selection.
- [`CClipboard::GetText`] reads the system's text.  [`CClipboard::TakeText`] is the paste's read: it also makes the
  text the history's newest entry if it isn't, as when another program put it there.
- [`CClipboard::Sync`] looks at the system's clipboard and adds its text to the history if it isn't the newest
  entry: how another program's copies get in.  It runs when a window is activated and before Paste from History asks
  which entry to paste.
- [`CClipboard::SetDetached`]: the clipboard holds its own text, and so do the clipboards sharing its history.
  Scripted runs (the IDE's checks) use it, so their cuts and copies never replace what the person at the machine has
  copied, nor depend on it.

## Clipboard history and Paste from History

The history and its chooser copy IntelliJ IDEA's Paste from History (Edit, Paste from History..., Control+Shift+V),
as closely as could be done from memory of it, not checked against a running copy.  The plan they were built from,
`WindowLib/ClipboardHistory.txt`, is in the history up to CommonLibraries a7b427686.

### The history

[`CClipboardHistory`] keeps the texts cut and copied, newest first.  Text already in it moves to the front rather
than being kept twice ([`CClipboardHistory::MoveToFront`]); past the limit (20, `CLIPBOARD_HISTORY_LIMIT`, as
IntelliJ's default; [`CClipboardHistory::SetLimit`]) the oldest goes.  Empty text isn't kept, nor a text bigger than
1 MB (`CLIPBOARD_HISTORY_MAX_ENTRY_BYTES`, [`CClipboardHistory::SetMaxEntryBytes`]), which is still copied to the
system.  It is for the session: it isn't saved.

There is one for an application.  A window's clipboard makes its own the first time it is wanted
([`CClipboard::GetHistory`]), and a window shown over or owned by another, a dialog ([`CWindow::ShowModal`]) or a tool
window ([`CWindow::ShowOwned`]), shares its owner's ([`CClipboard::ShareHistory`]), as it shares its owner's component
input.

An entry copied from a box (column) selection is a block ([`CClipboardHistory::IsBlock`]): pasted at a plain caret,
from the clipboard or chosen from the history, it goes in as a column ([`CTextEditing::PasteBlock`]): each line at the
caret's column of the next line down, short lines filled out with spaces, lines added past the text's end.  The mark
can't cross to another program, so the same lines copied elsewhere paste as lines, and a block only pastes as one
while it is the history's newest entry.

### Choosing what to paste

What asks which entry to paste is the application's, as every dialog's look is: a [`CClipboardChooser`], set on the
history ([`CClipboardHistory::SetChooser`]).  With none, Paste from History does nothing.  WindowLib's chooser is
[`CPasteHistoryDialog`], made with a dialog look and a font for the preview; the HCP65 IDE's [`CIDEPasteHistory`]
shows it with the code font (and lets the IDE's script watch it), and LogiCim and WindowInputTester set one too.

Paste from History is [`CIC_PasteFromHistory`], a standard chord (Control+Shift+V), and the keymap's
"Edit.PasteFromHistory" row in [`CEditCommands`], so it can be rebound and shows in menus with its keys.  A text
component's [`CTextEditingInput::PasteFromHistory`] posts [`CTextEditingInput::ChooseAndPaste`] to the next tick
([`CWindow::Post`]): a dialog can only be shown from a tick, not from an input action, as reading the input while it
is being read isn't supported.  At the tick it syncs the clipboard, asks the chooser over the component's window,
puts what was chosen on the clipboard (a block if the entry is one) and pastes it, over the selection.  So what was
chosen is the newest entry and the clipboard's text, and a plain paste after it pastes the same.  Nothing happens if
the text can't be edited or the history is empty.

### The dialog

[`CPasteHistoryDialog`], "Choose Content to Paste", is a resizable dialog with the list over a preview:

- Each row is the start of its entry's text on one line, up to 100 characters (`PASTE_HISTORY_ROW_CHARACTERS`), line
  breaks and tabs shown as spaces (so no font needs a glyph for a mark) and "..." where it is cut.  The first nine
  rows have their number before them.
- The preview is a read-only text area with the selected entry's whole text, not syntax coloured; with several rows
  selected, what would be pasted.
- The list has the keys.  Shift with the moves or a press, and Control with a press, select several rows; their
  texts are pasted in the list's order, a line feed between them.
- Enter, OK or a double click chooses; Escape or Cancel closes with nothing chosen.  Space activates the selected row,
  as in every list.
- Delete takes the selected entries out of the history; the row after them is selected.
- Typing filters the list as a speed search does (rows whose text contains what was typed, case not mattering), shown
  as "Search for: ..." over the list.  Backspace takes a character off it.  Escape clears it, and with nothing typed
  closes the dialog: the list wants Escape only while there is a filter ([`CListContainer::SetWantsEscape`]), so
  otherwise Escape reaches the dialog's Cancel.
- With nothing typed, 1 to 9 choose that row at once; typed after something else, a digit is part of the filter.

Two things about dialogs that this one met:

- A dialog's body is built into a local before [`CDialog::SetBody`] is called with it and the component that is to
  have the keys: in `SetBody(CreateBody(), mpList)` g++ read `mpList` before `CreateBody` had made it (the order
  arguments are evaluated in isn't fixed), and the dialog opened with nothing focussed.
- [`CDialog::Close`] closes as if a button was clicked without asking the approver; a script that chooses should
  call [`CDialog::OKClicked`], which does.

## Tests

TestWindowLib checks input without a display, on the reference screen:

| Test file | Checks |
|---|---|
| [`TestWindowLib/TestComponentInput.cpp`] | The window's input API and the focus: hover, focus, drags, dispatch, focus on click. |
| [`TestWindowLib/TestComponentInputDevices.cpp`] | [`CComponentInput`] with InputLib's program devices ([`CProgramInput`]): keys, chords, sequences, repeat. |
| [`TestWindowLib/TestComponentInputWinRaw.cpp`] | Windows raw input structures through WinInputLib's real XML, as the X11 and Windows inputs give them. |
| [`TestWindowLib/TestKeymap.cpp`] | Contexts, fallback, typing contexts, commands' keys, the keymap file, the key giver and the page; which context the focus reads a key in. |
| [`TestWindowLib/TestClipboardHistory.cpp`] | The history's order, moves, limit, removal and blocks. |
| [`TestWindowLib/TestPasteHistoryDialog.cpp`] | The dialog through its window, and a chooser dialog over a dialog through the modal loops. |
| [`TestWindowLib/TestTextEditingInput.cpp`] | Cut, copy, paste and Paste from History with a chooser that answers from the test. |

The HCP65 IDE's checks drive keys and the clipboard through its script (the keymap-* checks, commands,
editor-paste-history), with a detached clipboard; LogiCim's check which command keys run in a context
(`keys:<context>:<keys>`) and give keys in its chooser.

## Decisions

- Events, not listener interfaces (the user, 2026-11-05): a component offers a [`CComponentEvent`] per thing that
  happens ([`CButton::GetClickEvent`], [`CScrollPane::GetScrollEvent`]); a handler is added with the object and its
  member function and removed by the object before it goes.  The listener classes were taken out.
- Raw input on Windows, not window messages, for the keys and buttons (the user, 2026-10-21): one stream for every
  device, with the device it came from, as InputLib wants.
- Clicking the background, where nothing under the pointer can take the focus, clears the focus; a pointer event's
  coordinates are relative to the top left of the component it is given to (the user, 2026-10-21).
- An application's commands and keymap are its own (the user, 2026-11-07): WindowLib knows nothing of Build, leaving
  a sheet, the debugger's keys or Go to Class; it has only the standard widget commands.
- Commands are "hybrid D" (the user, 2026-11-07): [`CCommand`] objects per window for the application's commands
  (name, caption, icon, default keys, enabled and run callbacks), menu items and tool buttons made from them, and a
  bridge ([`CComponent::CanDoCommand`]) for Undo, Redo, Cut, Copy, Paste and Select All to whatever has the keys.
  Widgets keep their input actions and the generic command enum.
- The keys are IntelliJ IDEA's (the user, 2026-11-07): Redo is Control+Shift+Z, not Control+Y; Control+Y is Delete
  Line; Cut Line is Control+X with nothing selected, with no key of its own.
- Keymap contexts (the user, 2026-11-14):
  - Contexts are real: what a key does depends on the focussed component and on the component under the pointer.
  - Over a context that takes bare keys (the circuit canvas), its keys work wherever the focus is, unless the focus
    is in a text field, which keeps typing; elsewhere the focussed component decides.
  - A key not bound in a context is looked for in the contexts around it, out to the global keys, as IntelliJ's
    editor keymap falls back to its main one; the innermost binding wins.
  - A key belongs to one command in each context; a context's key shadows a global one only there, and the keymap
    says so and refuses nothing for it.
  - Keys are given by one set of rules, through one path, wherever they are given (the keymap page, the component
    chooser).  A clash asks, naming the command that has the key, and offers to move it (Give or Cancel); giving
    moves the key and saves both files it touches.
  - A subcircuit's placing key is the project's, kept in the project file so it goes with the project and follows a
    rename; every other key is the user's, in their keymap, the same in every project.  Loading a project silently
    clears any of its keys that clash with the user's: the user's keys win.
  - The HCP65 IDE's editor keys became the Editor context, with nothing changed for its users.
  - A menu item shows a command's global key if it has one, else its context key, plain.
- In a dialog keys are always read in the global keys, so nothing done to a context's keys can stop a dialog
  working; and keys dialogs and text fields need can't be given globally, as the global keys reach them.
- Clipboard history (the user, 2026-11-12, asking for it "as close to IntelliJ as possible"): block copies paste as
  a column; the preview isn't syntax coloured; LogiCim and WindowInputTester get the chooser too.  The limit of 20,
  a constant, and the digits choosing a row while nothing is typed were defaults the user didn't answer.
- The history is made the first time it is wanted, not with every clipboard: many TestWindowLib tests count the
  objects a window makes.
- The chooser is the application's to give, as every dialog's look is; WindowLib gives one it can use.
- Keys repeat in WindowLib, not from the platform (2026-11-23): only the command or printable repeats, so components
  that read keys going down and up see one press, and the repeat is the same on every platform.

## Not done

- A subcircuit's keys following its rename isn't checked by LogiCim's checks (the keys are on the circuit, so they
  should).
- Add Shortcut's Where combo box is centred in its row rather than left aligned.
- LogiCim's component chooser can't disable its Add Shortcut and Remove buttons on a library's row: buttons have no
  disabled look yet ([Components](components.md)).
- The first capital typed after Shift lays the window out once, as the Shift press held back for a chord isn't a
  typing key (seen 2026-11-08).
- The clipboard copies text only.
- What the history and dialog copy from IntelliJ IDEA was written from memory: several selected entries joined by
  line feeds, the digits pasting at once, and block copies pasting as a column haven't been checked against IntelliJ.

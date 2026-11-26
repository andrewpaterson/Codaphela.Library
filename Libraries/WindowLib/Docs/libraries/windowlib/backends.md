# Backends

WindowLib draws every pixel itself, into images in memory.  What it needs from the platform is small: a window to
present those pixels in, told where the pointer is and when it is activated; the keyboard's and mouse's input; a
clipboard; a pointer shape.  A backend is a library that gives these through WindowLib's native classes, one for each
platform, and one, WinRefLib, that is no platform at all but a screen in memory for the tests and checks.

| Backend | Library | Input through | For |
|---|---|---|---|
| [`CWinGDIWindowFactory`] | WinGdiLib | WinInputLib's [`CWinInput`] (raw input) | Windows. |
| X11 ([`X11Lib/X11WindowFactory.h`]) | X11Lib | X11Lib's input ([`X11Lib/X11Input.h`]), faking raw input | Linux, with plain Xlib. |
| [`CWinRefWindowFactory`] | WinRefLib | [`CWinRefInput`], whose devices are never pressed | Windows on a reference screen in memory: TestWindowLib and the applications' scripted checks. |

An application picks one in its `main` and hands it to the same code.  The HCP65 IDE has three mains,
[`HCP65IDE/HCP65IDE.cpp`] (Windows), [`HCP65IDE/HCP65IDELinux.cpp`] (X11) and [`HCP65IDE/HCP65IDERef.cpp`] (the
reference screen, which its checks run), each making its factory and native input and calling the one [`RunIDE`].  The
HCP65 emulator is built the same way.

## The native classes

| Class | Does |
|---|---|
| [`CNativeWindowFactory`] | Makes the other three, allocated with the mallocator it is given ([`NativeFactoryMalloc`]). |
| [`CNativeWindow`] | The platform's window: made and destroyed, its events executed, waited on, presented to. |
| [`CNativeCanvas`] | A canvas's pixels and the drawing calls on them: boxes, pixels, other canvases, cels and images. |
| [`CNativeClipboard`] | The system's clipboard's text, as UTF-8 with line feeds. |

The factory is a StandardLib object, though what it makes isn't: [`CWindow`], [`CCanvas`] and [`CClipboard`] hold it
with a pointer marked `Nes`, so the object system frees it after everything that uses what it made, in whatever
order an application shuts down.  An application makes it with [`OMalloc`] after the objects are initialised, adds
it to the root while it runs, and lets go of it rather than killing it.

A [`CWindow`] uses its native window like this:

- [`CWindow::Show`]'s loop asks it to wait for events ([`CNativeWindow::WaitForEvents`], if
  [`CNativeWindow::CanWaitForEvents`]) no longer than until something is due, then to execute them
  ([`CNativeWindow::ExecuteNativeWindow`]).  Executing them is when the native window calls the window's input API
  ([Input](input.md#the-windows-input-api)): the pointer's moves and leaving, activation and deactivation.  The keys
  and buttons go through InputLib instead and are read when the component input updates, later in the same pass.
- A paint draws into the canvases, then presents ([`CNativeWindow::Present`]): the native window is told what changed
  ([`CNativeWindow::SignalPresentArea`], [`CNativeWindow::SignalPresentDamage`]) and puts those pixels on the screen,
  at once or when the platform asks it to.  [`CWindow::PaintNow`] presents straight away
  ([`CNativeWindow::PresentPending`]), for long work that doesn't come back to the loop.
- A dialog's native window is given its owner ([`CNativeWindow::SetOwner`]) and destroyed when it closes; the owner
  is disabled while it is shown ([`CNativeWindow::SetEnabled`]) and ignores the keyboard and pointer.  A tool window
  floating over a window is owned but not modal, and has no frame ([`CNativeWindow::SetTool`]); WindowLib draws it.
- It also sets the pointer's shape ([`CNativeWindow::SetCursor`], [`ENativeCursor`]), moves and raises the window,
  asks for it to be made active ([`CNativeWindow::RequestActivate`]), and says where it and the screen are.

Each canvas is memory WindowLib owns, 32 bits a pixel laid out B, G, R, X: the layout of a 32 bit Windows DIB, and
what a 24 bit TrueColor X visual with red in 0xFF0000 holds.  Everything is drawn on those pixels directly, never with
the platform's drawing: GDI queues its drawing and would have to be flushed before every direct write, of which there
are hundreds a frame.  The only thing that reads them is the present.  The image owns the memory, so whoever holds
the image after the canvas is resized or freed still has valid pixels.  How a window decides what to draw and
present is on [Drawing and Speed](drawing.md).

Keys, buttons and the wheel come through InputLib, not the native window: the application gives its [`CInput`] the
platform's [`CNativeInput`], whose devices are described by WinInputLib's XML.  Every backend gives the same two
devices, WinInputLib's "Standard 104 Button Keyboard" and "Standard 5 Button Mouse" (raw input's), so what a key
means ([`CComponentInput`]) is the same on every platform.

## Windows: WinGdiLib and WinInputLib

[`CWinGDIWindowFactory`] makes [`CWinGDIWindow`]s, [`CWinGDICanvas`]es and [`CWinGDIClipboard`]s.

- A canvas is a DIB's pixels; the window presents them with `SetDIBitsToDevice` when Windows sends `WM_PAINT`, after
  a present has invalidated the area that changed.  Waiting for events is `MsgWaitForMultipleObjectsEx`.
- The pointer comes from `WM_MOUSEMOVE` ([`CWinGDIWindow::MouseMoved`]), with `TrackMouseEvent` asking for
  `WM_MOUSELEAVE` ([`CWinGDIWindow::MouseLeft`]).  A button press captures the pointer, so a drag goes on outside the
  window, and its release lets go ([`CWinGDIWindow::MouseButton`]); the buttons themselves come through raw input.
- `WM_ACTIVATE` sets the caps and num locks from `GetKeyState` and activates the window
  ([`CWinGDIWindow::Activated`]), or tells it the platform's focus left ([`CWinGDIWindow::Deactivated`]).
- Keys, buttons and the wheel are raw input (`WM_INPUT`).  The window hands each `RAWINPUT` to the factory's raw input
  callback ([`CWinGDIWindowFactory::SetRawInput`]), which is [`WinRawInputCallback`] with [`CWinInput::GetRaw`] as
  its context; [`CWinRawInput`] queues the events and [`CWinInput::Update`] gives them to InputLib.  Without the
  callback `WM_INPUT` is ignored.
- A window with an owner (a dialog or a tool window) is a `WS_POPUP` window owned by its owner's, and its owner is
  disabled while it is shown modally.  Only the main window ends the message loop (`PostQuitMessage`): a dialog
  closing just stops its own, and a `WM_QUIT` a dialog's loop sees is posted again for its owner's.

[`CWinInput`] is a [`CNativeInput`] over raw input for keyboards and mice, DirectInput for joysticks and XInput for
gamepads; `Init(NULL, false)` is enough for keyboards and mice.  Raw input says which device each event came from by
its handle ([`CWinRawInput::GetDetailForHandle`]).  Input injected by a virtual machine, remote desktop or KVM
software has no handle (every event had none when it was first run on Windows, 2026-10-21); such input is given to the
first device of its type.

WinGdiLib and WinInputLib are compiled on Linux with MinGW, compile only (CommonLibraries' `CLAUDE.md` has the
command); they are run on Windows.

## X11: X11Lib

X11Lib is plain Xlib: the user asked for a simple library, and Xlib was chosen over SDL, GLFW and xcb.  It isn't in
any Visual Studio solution; its `X11Lib.vcxproj` is only there so CMake can read its list of sources.  It is built
64-bit only, as there is no 32-bit libX11 on the Linux machine, and needs libx11-dev and libxext-dev.  The X headers
`#define` `None`, `#!text Bool`, `True`, `Status` and more, so only X11Lib's `.cpp` files include them, after everything
else; its headers forward declare the X structures and hold X's IDs as `X11ID` (`unsigned long`).

[`X11Lib/X11WindowFactory.h`] opens the display (`DISPLAY` unless given another) and every window shares it.  The
factory reads the display's events and gives each to the window it is for, so dialogs and tool windows work as on
Windows; a window shown as a dialog runs the factory's events while its owner's loop waits, so the owner is still
drawn.  Xlib's own error handler would end the process on any error; the factory's logs it instead, as
"X request [n] failed [...]".  A dialog is a transient window of its owner's, typed `_NET_WM_WINDOW_TYPE_DIALOG`
with `_NET_WM_STATE_MODAL`, and centred on its owner's client area; a disabled owner drops its input and its close.

[`X11Lib/X11Window.h`] presents with MIT-SHM where the X server has it: the pixels that changed are copied into a
shared image and the server reads them from there, not through the connection.  The server sends an event when it has
read them, and until then they mustn't be copied over, so a window waits for its last present just before its next
one (its own completion event only, at most 250 ms, then `XSync` with one warning), drawing the next frame meanwhile.
One present at a time also stops a window that paints every tick queueing frames faster than they are shown.  Without
MIT-SHM, if attaching fails, after any MIT-SHM error (shared memory is then off for every window), or with
`SetSharedMemory(false)` on the factory, it presents with `XPutImage` and waits with `XSync`.  The default visual must
be 24 bit TrueColor with red in 0xFF0000.

Input is faked as Windows raw input.  [`X11Lib/X11Input.h`] turns X's key codes (with evdev or libinput, the Linux
input event code plus 8) into raw input keyboard records, with Windows' virtual key, make code and E0 and E1 flags,
and X's buttons into raw input's mouse button flags and wheel, so WinInputLib's standard keyboard and mouse
descriptions and [`CComponentInput`] work unchanged.  Raw input gives the numpad's keys as if num lock were off, and
so does the fake.  X merges every keyboard and every mouse into one of each, so there is one of each.  Detectable
auto repeat makes a held key's repeats presses only, as raw input's are, and InputLib gives a held key one press
(WindowLib repeats keys itself, [Input](input.md#key-repeat)).  The window also gives the input the pointer's moves
as deltas, and on activation reads the locks from XKB.

## The reference screen: WinRefLib

[`CWinRefWindowFactory`] makes windows that show nothing anywhere: they are on a [`CWinRefScreen`] of the factory's
own, stacked in the order they were made or raised, and draw into [`CWinRefCanvas`]es, images drawn with SupportLib's
[`CImageDraw`].  Nothing outside the program sends them events, so a program on the reference screen is driven by its
tests or its script, through the window's input API ([`CWindow::InputKeys`] and the rest) or a keyboard of InputLib's
program input ([`CProgramInput`]), as the IDE's script keyboard is.  [`CWinRefInput`] gives the program the standard
keyboard and mouse a real platform has, never pressed.

- By default the windows are ticked and painted continuously.  [`CWinRefWindowFactory::SetWaitForEvents`] makes them
  sleep until something is due instead, as a real window does, but never longer than 50 ms (`WIN_REF_MAX_WAIT`), as
  nothing can wake them.
- Given an image path, each window saves its frames there as PNGs (`Frame<n>`, `DialogFrame<n>`, `ToolFrame<n>`),
  every frame or every [`CWinRefWindowFactory::SetFrameInterval`]'th.  With no path nothing is saved.
- [`CWinRefWindowFactory::SetCheckPresents`] makes each window keep what it shows on the screen, copied from its canvas
  only where a present said it changed, as a real screen would, and checks after each present that what it shows is
  what it drew; mismatches are counted ([`CWinRefScreen::NumMismatches`]).  This is what catches a partial redraw
  that drew something it didn't present.
- [`CWinRefWindowFactory::SetHashPresents`] writes a hash of each present to a file, a line a present: of the areas
  presented, or of the whole screen ([`EWinRefHashArea`]), so two runs can be compared present by present.
- The clipboard is text the factory keeps ([`CWinRefWindowFactory::GetClipboardText`]), shared by every window it
  makes as a system clipboard would be, and seen by nothing outside the process.  A test can change it behind the
  clipboard's back, as another program would.

The HCP65 IDE on the reference screen ([`HCP65IDE/HCP65IDERef.cpp`]) is set up from its environment:
`HCP65_IDE_VERIFY_PRESENT` checks every present, `HCP65_IDE_PRESENT_HASH=<file>` hashes them
(`HCP65_IDE_PRESENT_HASH_AREA=screen` for the whole screen), `HCP65_IDE_FRAMES=<folder>` and
`HCP65_IDE_FRAME_INTERVAL` save frames, and `HCP65_IDE_CONTINUOUS` paints continuously.  Its screen is 2560 by 1440.

## The clipboard on each backend

| Backend | Clipboard |
|---|---|
| Windows ([`CWinGDIClipboard`]) | `CF_UNICODETEXT`: UTF-8 converted to and from UTF-16, line feeds to and from carriage return and line feed pairs, with BaseLib's [`CUnicodeWriter`].  Text that isn't valid is neither copied nor pasted.  Windows keeps the text after the program has gone. |
| X11 ([`X11Lib/X11Clipboard.h`]) | The display's `CLIPBOARD` selection.  X keeps no copy: the program that copied owns the selection and answers requests for the text, so the factory keeps it, with a hidden window owning the selection, for as long as the factory lives; every window's clipboard shares it, so text copied in a dialog can be pasted after it closes.  Given and taken as `UTF8_STRING`, or `STRING` (Latin-1) for older programs.  While this program owns the selection its own text is read without asking the server, so looking at the clipboard on activation costs nothing. |
| Reference ([`CWinRefClipboard`]) | The factory's text, see above. |

## Sound

Sound is a separate factory, [`CNativeSoundFactory`], given to a window that plays sound: Windows' waveOut
([`CWinWaveoutSoundFactory`]) or PulseAudio on Linux ([`PulseAudioLib/PulseAudioSoundFactory.h`], loaded when the
program runs).

## Vulkan

[`WindowLib/Vulkan.txt`] plans Vulkan as WindowLib's renderer; none of it is built yet (2026-11-26).  Today Vulkan
(VulkanLib, [`VulkanLib/VulkanFrame.h`]) draws a 3D scene offscreen and reads it back into an image, which a canvas
draw blits onto the window's canvas, which GDI copies to the screen; Goober's frame takes about 40 ms that way in
WindowInputTester, not yet broken down (the readback's copy from uncached memory is the suspect).

The user's decisions (2026-11-14): a window presents all of its client area through a Vulkan swapchain, not a child
window per 3D view, which would cover WindowLib's popups, menus, tool tips and docking guides; Vulkan becomes the
renderer for every native canvas, with the software backends kept for machines without Vulkan 1.3 and for tests; and
nothing outside a native canvas touches its pixels ([`CNativeCanvas::GetImageOrNull`] goes): a drawer draws its own
image, or writes every pixel of an area it locks, and reading pixels is only for snapshots, redraw checks and tests.
The plan's six steps start with that API on the software canvases, WinRefLib poisoning a locked area to catch a drawer
that reads or misses pixels; its open questions are listed there.

## Testing a backend

- X errors only show in the log: after changing X11Lib, look for "X request" in a run's output.
- The IDE's and LogiCim's checks read the window's canvas, not the screen, so they don't see a present that went
  wrong; X11Lib's presents were checked by reading back the window's pixmap, as a screenshot of the real window
  grabs whatever covers it.
- Real input on X11 is tried on an isolated X server (Xephyr) driven with XTest, so nothing touches the desktop.
- A reference window ticks every pass of its loop unless its factory waits for events
  ([`CWinRefWindowFactory::SetWaitForEvents`]); a test of something that happens only on an event, a dialog that
  waits for its answer, wants it.

## Decisions

- Raw input on Windows, not window messages, for keys and buttons (the user, 2026-10-21).
- X11Lib is plain Xlib, a simple library (the user, 2026-10-22, Xlib chosen over SDL, GLFW and xcb).
- X input is faked as Windows raw input, so one set of device descriptions and one keyboard map serve every platform.
- Canvases are memory WindowLib draws into directly, never the platform's drawing; the platform only presents.
- The window factory is an object, held by everything it made, so it is freed after them.
- Vulkan as the renderer of every native canvas, presenting through a swapchain per window, with the software
  backends kept (the user, 2026-11-14; not built).

## Not done

- Vulkan as the renderer: [`WindowLib/Vulkan.txt`], none of its steps built.
- X11 text bigger than one X request (INCR transfers) can't be copied or pasted.
- X11Lib is 64-bit only.
- As of 2026-11-08 the Windows side of the later work (docking, partial redraw, dialogs, tool tips, toolbars, status
  bars) had only been compiled with MinGW, not run on Windows.

# WindowLib

WindowLib is the project's windowing toolkit: windows, a tree of components laid out by containers, widgets in the
style of IntelliJ IDEA's new UI, input, focus and commands, popups and menus, Visual Studio style docking, dialogs
and a sound mixer.  The HCP65 IDE, LogiCim, the HCP65 emulator and HCP65-Sculpt are built on it.

WindowLib draws every pixel itself, in software, into images it owns.  The host's window system is only asked for a
window to present those pixels in, its input, a clipboard and a cursor, through a small set of native classes that a
backend library implements for each platform.  So a window looks the same on every platform, and a test can run a
window off screen and compare its frames pixel for pixel.

## The window

A [`CWindow`] is the root of a tree of components.  It holds:

- its native window, made by the [`CNativeWindowFactory`] it was given;
- its canvas ([`CWindow::GetCanvas`]), which covers the client area, draws the window's background with the
  [`CCanvasDraw`] given to [`CWindow::Init`], and holds the window's one container ([`CWindow::SetContainer`]) and,
  over it, any open popups ([`CPopup`], opened with [`CWindow::OpenPopup`]);
- its focus ([`CFocus`]): which component has the keys, which has the pointer over it, and drags;
- a [`CWindowTick`], called each pass of the window's loop with the time since the last.

[`CWindow::Show`] runs the loop: each pass executes the native window's events, gives the input to the components,
lays the window out and paints it if anything changed, then ticks it.  Between passes the window sleeps until there
are events or something is due, unless it is continuous ([`CWindow::SetContinuous`]), as a game or an emulator is.
A dialog is a window shown with [`CWindow::ShowModal`] over its owner.

## Components and containers

Everything in a window is a [`CComponent`]: it has a parent and children, the area it was laid out in
([`CComponent::GetActualBounds`]), a desired size and an alignment, the input actions it takes
([`CComponent::AddInputAction`]), and optionally a context menu, a tool tip and a pointer shape.  Components are
StandardLib objects: made with [`OMalloc`], held by [`Ptr`], and freed when nothing points at them.  They aren't
saved ([`CComponent::Save`] returns false).

A [`CContainer`] is a component that places its children.  It works out the size its children need (its required
size), and is laid out into an area in which it lays them out in turn.  The kinds of container are on the
[Layout](layout.md) page: the fill (one area for all), the flow (rows that wrap), the grid (columns and rows with
weights), and the scroll pane.  Most widgets are containers themselves, put together from smaller parts: a
[`CButton`] is a [`CImageBorderContainer`], a grid of border pieces round a face; a [`CListContainer`] is a grid with
a row for each item, and menus, trees and tables are lists.

## Canvases

A [`CCanvas`] is a component with pixels of its own: its native canvas, an image it is drawn into.  What it draws
is a [`CCanvasDraw`], called with the canvas whenever the canvas has to be drawn again: [`CClearCanvasDraw`] fills it
with a colour, [`CImageCanvasDraw`] blits an image cel, [`CMapsCanvasDraw`] draws text from a font's glyphs, and
widgets add their own.  The text components, [`CLabel`] and [`CTextArea`], are canvases.

A paint draws what changed into the canvases that changed, then copies each canvas into the window's canvas, back to
front, and the native window presents the result.  A canvas can be transparent ([`CCanvas::SetTransparent`]), and is
then blended over what is behind it.  How this is kept quick, drawing again only the damaged parts of a window, is
on the [Drawing and Speed](drawing.md) page.

## Themes

How widgets look comes from a [`CWindowTheme`]: every colour ([`EThemeColour`]), every size in pixels
([`EThemeMetric`]), every icon ([`EThemeImage`]), and the images and styles of buttons, check boxes, menus and
scroll bars.  A widget is given the theme when it is made and reads what it needs from it; it has no colours or
sizes of its own.  [`CJewelTheme::CreateDarkTheme`] makes IntelliJ IDEA's dark (Jewel) theme, its images drawn at
runtime rather than loaded; a test can put a theme together from other images.

## Input

The native window's input arrives as events in the window's client space.  [`CComponentInput`] turns keys, buttons
and the wheel into commands ([`EComponentInputCommand`]) and characters, through keymaps.  The window's focus gives
each to the focussed component, or the one under the pointer, which runs the input actions added for it.  What no
component takes goes to the application's commands ([`CCommandSet`], set with [`CWindow::SetCommands`]), which menus
and toolbar buttons are made from too.  See [Input, Focus, Keys and the Clipboard](input.md).

## Native backends

The native classes are a [`CNativeWindowFactory`] and what it makes: a [`CNativeWindow`], a [`CNativeCanvas`] per
canvas and a [`CNativeClipboard`].  There are three backends:

| Backend | Library | What for |
|---|---|---|
| [`CWinGDIWindowFactory`] | WinGdiLib | Windows, with GDI and raw input. |
| X11 ([`X11Lib/X11WindowFactory.h`]) | X11Lib | Linux. |
| [`CWinRefWindowFactory`] | WinRefLib | Windows drawn off screen on a reference screen of its own, ticked and painted continuously, each frame optionally saved as an image: what the tests use. |

Sound works the same way: a window given a [`CNativeSoundFactory`] has a [`CSoundMixer`] mixing on its own thread,
played by [`CWinWaveoutSoundFactory`] on Windows or PulseAudio ([`PulseAudioLib/PulseAudioSoundFactory.h`]) on
Linux.  See [Backends](backends.md).

## Pages

- [Layout](layout.md): sizes, containers, the grid, alignment, clipping, scroll panes and lists.
- [Components](components.md): the widgets.
- [Docking](docking.md): tool windows and documents docked, tabbed, floated and auto hidden.
- [The Text Area](text-area.md): the text editor component.
- [The Diff View](diff-view.md): two texts side by side with their differences.
- [Drawing and Speed](drawing.md): canvases, painting, partial redraw.
- [Input, Focus, Keys and the Clipboard](input.md).
- [Backends](backends.md): Windows, X11 and the off-screen reference.

## Decisions

These hold for every widget (the user, 2026-11-07):

- Every pixel size and colour a widget uses belongs in a [`CWindowTheme`]: no sizes as `#define`s in generic
  widgets, no Jewel colours outside [`CJewelTheme`], no colours read once and kept.
- A window doesn't own its theme.  A theme can be shared by the widgets of many windows; each widget keeps the theme
  it was made with.
- A theme has no working defaults: whoever makes it sets every colour, size and image its widgets use, and reading one
  that was never set is logged as an error.  So the tests configure what they use, through [`CJewelTheme`] or a test
  theme of their own.
- No live theme changes: a theme is set up before the widgets that use it are made.  [`CWindowTheme::Changed`] only
  tells its listeners, an application that remakes its widgets say.

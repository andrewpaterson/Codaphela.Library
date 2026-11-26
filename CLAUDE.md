# CommonLibraries

The Codaphela project's shared C++ libraries and their tests.  The project's repositories are checked out
side by side in one folder (CommonLibraries, HCP65, HCP65-IDE, LogiCim, HCP65-Sculpt); the others find this
one at `../CommonLibraries`, and builds go in a shared `../build`.

## Layout

- `Libraries/<Lib>`: one folder per library, each with a Visual Studio project (`<Lib>.vcxproj`).  BaseLib
  (containers, text, files, parsers) is the base; StandardLib adds objects and persistence (Codabase),
  SupportLib images, fonts, sound and geometry, WindowLib the windowing toolkit, CppCompilerLib the C++
  compiler for the HCP65's processors, MicroprocessorLib the processor simulators.
- `Tests/Test<Lib>`: each library's test program, a `CMakeLists.txt` beside its `.sln`.  `TestBaseLib65`,
  `TestStandardLib65` and `TestSupportLib65` run those libraries compiled for the 65C64 and W65C816.
- `Tools/`: small programs.  `repos.py`: status, pull and push across all the repositories
  (`python3 repos.py status`); its `commit` stages everything.

## Documentation

- A library's documentation pages are in its `Docs` folder, laid out as the website's paths and listed as items of
  its `.vcxproj`: `Libraries/CppCompilerLib/Docs/compiler` (the compiler, and `65c64/calling-convention.md`),
  `Libraries/MicroprocessorLib/Docs/65c64/emulator.md` (the 65C64 simulators), `Libraries/WindowLib/Docs/
  libraries/windowlib` (WindowLib's design: layout, components, docking, the text area, drawing, input, backends).
  The website (MkDocs, `../HCP65/Website`) gathers every repository's `Docs` folders through its `mkdocs.yml`'s
  `extra.sources`.
  `../HCP65/Website/DocsPlan.txt` is the plan for the documentation.
- Name code in a page with a reference: [`CTextArea`], [`CTextArea::ReplaceRanges`], [`CShape::Init(int)`] or
  [`BaseLib/Chars.h`].  The site build links it to its line on GitHub and fails on one that names nothing; a plain
  `CTextArea` naming a real class is a warning.  A span that isn't C++ but looks it is written `#!text abs`.
- A finished plan becomes a design page and is deleted, the page naming its last commit (`DocsPlan.txt` decision 17).
- Plans and design notes are text files beside their code; list them with
  `find Libraries Tests -name "*.txt" -not -path "*/Input/*" -not -path "*/Output/*"`.  The larger ones:
  `Libraries/CppCompilerLib/Cpp14CompliancePlan.txt`, `Libraries/CppCompilerLib/W65C816.txt`,
  `Libraries/CppCompilerLib/CodeGenerationHandoff.txt`, `Libraries/StandardLib/ThreadSafetyPlan.txt`,
  `Libraries/StandardLib/WeakReferencesPlan.txt`, `Libraries/BaseLib/CodabaseTransactions.txt`,
  `Libraries/MicroprocessorLib/65C64Simulator.txt` (the 65C64 simulators' rulings), `Libraries/WindowLib/Vulkan.txt`.

## Building and testing

- CMake reads each target's sources from its `.vcxproj` (`CMake/VcxprojSources.cmake`): a new file goes in
  the `.vcxproj` and its `.vcxproj.filters`, never only in CMake.
- A test suite, configured into the shared build folder:

  ```sh
  cmake -S Tests/TestBaseLib -B ../build/TestBaseLib64Release -DCMAKE_BUILD_TYPE=Release -DBASELIB_64=ON
  cmake --build ../build/TestBaseLib64Release -j
  ctest --test-dir ../build/TestBaseLib64Release -V
  ```

  Options: `BASELIB_64` (otherwise 32-bit), `BASELIB_ASAN`, `BASELIB_TSAN` (64-bit, run under `setarch -R`),
  `BASELIB_SIZE16`, `BASELIB_MALLOC`.  Build folder names follow them: `TestBaseLib`, `TestBaseLib64`,
  `TestBaseLib64Release`, `TestBaseLib64Asan`.  The libraries build 32- and 64-bit, Debug and Release.
- A test runs in its project folder (`Tests/Test<Lib>/Test<Lib>`) and writes `Output/` there.
- The `65` suites are slow and built Release only.

## Rules for the code

- House style is BaseLib's and SupportLib's: heap objects with Init/Kill through `NewMalloc<T>()` and `SafeKill(p)`;
  arrays of pointers as `CArrayTemplatePtr<T>`; `StringCompare`, not `strcmp`; constant tables at file scope with a
  `g` prefix (`gaszNames`); declarations aligned in one tab column per block; tabs, no trailing white space.
  `CChars::WriteString` pairs with `CChars::ReadString`.  File readers parse headers through `CFileBasic` over a
  `CMemoryFile`.  Bit sizes come from `gcTypeNames.GetBitSize`.
- Member prefixes follow the field's type: `mc` for a class, `ms` for a struct.
- Objects (StandardLib): a class holding a `CObject`, a `Ptr` or a collection must itself be a `CObject` (made with
  `OMalloc`, reachable from the root).  A `Ptr` is a field of a `CObject` declared in `Class()`, or a brief stack
  local; never inside a non-`CObject`.  `CObject`s live only briefly on the stack.  Root object graphs
  (`ORoot()->Add`) in code and tests: an unrooted graph walks itself on every `Ptr` change.  An object with no other
  holder (a dialog, an app) is `OMalloc`'d and added to the root, and removed from it to destroy it.
- In `Class()`, a non-`CObject` field holding pointers (events, `SMemberCall`, structs with pointers) is
  `U_Unknown`; `U_Data` is for pointer-free data.
- `AllocateReplacingNamed` keeps the replaced object's index: the name is the identity.
- Layouts match MSVC: on 32-bit g++ `CObject` has a tail-padding filler (`LINUX_GNU && !BUILD_64`); if base class
  fields change, TestClass's check says whether the filler is still right (sizes in `TestBaseObject.h`, all branches).
- `ARGB32` (SupportLib) keeps red in the low byte: write colours with `Set32BitColour`, not `0xFFRRGGBB` literals.
- StandardLib object code doesn't include `BaseLib/SetString.h` (its `SSetIterator` clashes with StandardLib's): use
  `CMapStringInt`.
- Text drawing uses SupportLib's fonts (`CFontFamily`, `CFont`): extend them rather than writing another.
- MicroprocessorLib depends on BaseLib and StandardLib only.  WorldLib has rotted: leave it out of surveys and builds.
  DirectXLib is dead code kept as a reference: don't delete it or port it.
- Optimised g++ builds contract `a * b + c` into FMA, so Release and Debug floats differ in the last bits: compare
  floats within a tolerance across configurations.

## Testing recipes

- After a StandardLib change run TestStandardLib, TestWindowLib and TestSupportLib.
- "All tests, all configurations": every native suite 32- and 64-bit, Debug and Release, plus 64-bit ASan and the
  Size16 builds; TestCppCompilerLib Release x64 only; the `65` suites Release on the 65C64 and the W65C816
  (argument `W65C816`); TestLibc65 also on the W65C816; the apps built Release x64, not run.  Show the matrix and wait
  for the go-ahead.  Run suites under gdb with `handle SIGTRAP nostop noprint nopass` (asserts break), ASan ones
  without it.  A sweep's builds take about 9 GB each: check free space first (`df -h`) and delete them after.
- Leaks: a `BASELIB_ASAN` build, run from the test's folder with
  `ASAN_OPTIONS=detect_leaks=1:replace_intrin=0:detect_stack_use_after_return=0`.
- W65C816 size bugs: try the mechanical catchers before bisecting on the simulator: the `BASELIB_SIZE16` host build
  (`-Wconversion`), the front end's widening and loop-counter warnings, the linker's missing names.
- Windows code compiles here with MinGW (compile only): `i686-w64-mingw32-g++` and `x86_64-w64-mingw32-g++ -std=c++17
  -c -DWIN_GNU_32 -D_DEBUG -I.` over the files each `.vcxproj` lists, from `Libraries`.  Windows SDK headers in lower
  case; no `##` pasting of string literals.
- Warning work leaves CppCompilerLib out.

## Working in these repositories

<!-- This section is the same in every Codaphela repository's CLAUDE.md: change them together. -->

- Several Claude sessions work in these checkouts at once, on the main branch (no private branches or worktrees).
  Run `git status` before touching a file and leave other sessions' uncommitted work alone.  Stage and commit only
  your own files, by path: `git add <paths>` and `git commit -m "..." -- <paths>`; never `git add -u`/`-A` or
  `commit -a`, and not `repos.py commit` (it stages everything).  Check `git show --stat HEAD` after.
- Commit messages have no Co-Authored-By line.  Make commits logical: one change each, its tests with it.
- Bring work from a copy into the main checkout with a 3-way merge (`git merge-file`, or `diff -u base mine |
  patch`), never by copying files over it: another session may have changed the target since.
- Before building a step of a shared plan, `git fetch` and read `git log HEAD..origin/<branch>` for the plan's
  files: a session on another machine may have done it.
- The shared build folders (`../build/*`) can hold binaries linked from another session's half-finished edits.  A
  failure seen only there: build the same sources in a private build folder; if that passes, rebuild the shared one
  clean.  Tests rewrite `Output/` in their project folder: run from a copy of that folder when another session may
  be running the same suite.
- Build, run and measure in Release x64 by default; Debug for full test sweeps or chasing a problem.  Say which build
  a number came from.  A speed claim that feeds a decision needs a like-for-like A/B (same commit but the change, in
  the whole test program) judged by instruction counts (callgrind) with the clock beside them.
- A decision the user states while planning settles that question only: plan with them, and wait to be told to
  build.
- Put decisions to the user one at a time, each explained plainly (what changes, what it costs), with the
  recommended option first, especially outside their speciality.
- Show images as full `file://` paths to the real files; don't make enlarged copies.
- Code files are LF.  `.sln`, `.vcxproj` and `.filters` are CRLF with a BOM: edit them as bytes and check `git diff
  --stat` shows only your lines.  Test `Input` files are kept byte for byte.

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

- The website (MkDocs) is in `../HCP65/Website/docs`: `libraries/`, `compiler/` (CppCompilerLib) and
  `65c64/` (the processor MicroprocessorLib simulates).  `../HCP65/Website/DocsPlan.txt` is the plan for the
  documentation: pages are to move beside the code they describe.
- Plans and design notes are text files beside their code; list them with
  `find Libraries Tests -name "*.txt" -not -path "*/Input/*" -not -path "*/Output/*"`.  The larger ones:
  `Libraries/CppCompilerLib/Cpp14CompliancePlan.txt`, `Libraries/CppCompilerLib/W65C816.txt`,
  `Libraries/CppCompilerLib/CodeGenerationHandoff.txt`, `Libraries/StandardLib/ThreadSafetyPlan.txt`,
  `Libraries/StandardLib/WeakReferencesPlan.txt`, `Libraries/BaseLib/CodabaseTransactions.txt`,
  `Libraries/MicroprocessorLib/65C64Simulator.txt` (the 65C64 simulators' rulings), `Libraries/WindowLib/*.txt`.

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
- A test runs in its project folder (`Tests/Test<Lib>/Test<Lib>`) and rewrites `Output/` there.  Other
  sessions may run the same suite at the same time: run from a copy of that folder to keep out of their way.
- The `65` suites are slow and built Release only.
- Code files are LF (`.gitattributes`); test `Input` files are kept byte for byte.

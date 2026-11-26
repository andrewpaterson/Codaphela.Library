# Listings, Profiling and Testing

## Listings

A listing is a unit's assembly with each C++ line as a comment above its code.  `hcp65build`
writes one for each unit into the listing directory (`-listings`, or `Listing/` beside a
solution); from C++ it is [`CCPP65Builder::SetListingDirectory`].  Compiling without linking is the
quickest way to see what the compiler does with a piece of C++:

```text
hcp65build -program Sum Sum.cpp -listings Listing
hcp65build -program Sum Sum.cpp -listings Listing -target 65C64
```

```asm
		.section text._Sum__FPKij
		.global _Sum__FPKij
_Sum__FPKij:
		...
		.line "/home/me/Sum.cpp", 9
		; Sum.cpp(9): for (i = 0; i < uiCount; i++)
		lda 0
		cmp [D + 41]
		...
		.fingerprint "_Sum__FPKij", 3061804840, 1794541092
```

The code is the same with or without listings; `.line` and `.fingerprint` are for the
[line table](linker.md#debug-info).  [W65C816 Code](w65c816.md#an-example) and
[65C64 Code](65c64.md#an-example) walk through one.

## The test suites

| Suite | Tests |
|---|---|
| `CommonLibraries/Tests/TestCppCompilerLib` | The preprocessor, parser, highlighter, generator, IR, optimiser and both back ends: IR programs run on both processors, C++ programs run on the emulators, exact assembly where it matters. |
| `HCP65/Tests/TestAssembler65Lib` | Both assemblers (every instruction and mode, checked against the simulators' tables), the linker, libraries, executables, images and modules.  It also assembles every `asm` example on this site's 65C64 pages. |
| `HCP65/Tests/TestLibc65` | Libc65 on both processors. |
| `CommonLibraries/Tests/TestBaseLib65`, `TestStandardLib65`, `TestSupportLib65` | All of BaseLib, StandardLib or SupportLib and their own test suites, compiled for each processor and run on the emulators: one program per test. |
| `HCP65/HCP65-Emulator/TestHCP65Lib` | The kernal, drivers and programs, built with the compiler and run on the HCP65 emulator. |

The Lib65 suites are the widest check that compiled code does what the C++ says, since the same
tests pass natively.  They are slow, so they are built in Release only.  Compiled programs can run
in *lockstep* on the instruction level emulator and the cycle level simulator together, so a
difference between the two shows the moment it happens.

## Running the Lib65 suites

The Lib65 suites (and TestLibc65) share a runner, `TestProject65`, which reads these environment
variables:

| Variable | Does |
|---|---|
| `TEST_PROJECT65_ONLY=<tests>` | Run only these tests' programs. |
| `TEST_PROJECT65_LISTINGS=<dir>` | Write each unit's listing. |
| `TEST_PROJECT65_LAYOUTS=<file>` | Write each class's layout, a line a class (see [Class layouts](building.md#class-layouts)). |
| `TEST_PROJECT65_CYCLES=1` | Print each program's cycles and size. |
| `TEST_PROJECT65_COMPILE_STATS=1` | Time the compile by phase and print its counts. |
| `TEST_PROJECT65_PROFILE`, `TEST_PROJECT65_PROFILE_REPORT=<file>` | Profile the programs and write a report: the cycles in each function and line, and in the calls made from each line. |
| `TEST_PROJECT65_FRAME_PROFILE=<file>` | Build the W65C816 code with a measured profile for its aligned frames. |
| `TEST_PROJECT65_PREFER_SIZE=1` | Prefer size to speed. |
| `TEST_PROJECT65_STACK_ARGUMENTS=1` | Push every W65C816 argument, with no register arguments, to compare. |
| `TEST_PROJECT65_CPP=1` | Compile the C++ of units that have hand written W65C816 assembly, not the assembly. |
| `TEST_PROJECT65_STEPS=<millions>` | Give a slow program longer. |
| `TEST_PROJECT65_TRACE=1` | Run one instruction at a time, and print where the program was when it went wrong. |
| `TEST_PROJECT65_THREADS=<n>`, `TEST_PROJECT65_SERIAL=1` | Compile on n threads, or on one. |
| `TEST_PROJECT65_NO_HEADER_CACHE=1` | Preprocess every header in every unit (it builds the same). |

## Profiling

[`CAsm65Profile`] runs a program on either processor's emulator (the W65C816 or the 65C64, through
[`CAsm65ProfiledCore`]) and counts the cycles in each function and each source line, through the line
table; its report also gives each line's *call sites*, the cycles
inside the calls made from that line with everything they called.  The same profile, saved, guides
the W65C816's [aligned frames](w65c816.md#aligned-frames).

`CommonLibraries/Tests/CompilerMeasure.py` collects the suites' cycles, sizes and compile statistics
into tables, including each optimiser pass's runs and changes.

!!! note
    A test's cycle count depends a little on the length of the path it was built in, because
    `__FILE__` strings are part of the program.  Compare cycle counts from folders whose paths are
    the same length.

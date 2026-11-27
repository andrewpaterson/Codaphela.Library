# The 65 Test Suites

The libraries' own test suites, compiled by this compiler and run on the simulators: TestBaseLib65,
TestStandardLib65 and TestSupportLib65 (in `CommonLibraries/Tests`) run every test of TestBaseLib, TestStandardLib
and TestSupportLib, and HCP65's TestLibc65 tests Libc65.  Each runs on the 65C64 by default and on the W65C816 when
given `W65C816` (or `816`).  How to run them and the harness's switches are in [Listings, Profiling and
Testing](../testing.md); this page is why they are built the way they are.

## Why the libraries' own tests

The compiler's own suite (TestCppCompilerLib) checks small programs written for it.  The 65 suites compile real
code nobody wrote for the compiler: BaseLib, StandardLib and SupportLib from the source files their `.vcxproj`
files list, with the test projects that already pass natively.  The same test passing natively and on a simulator
is the widest check there is that compiled code does what the C++ says, and a test that fails on one only points
straight at the compiler (or at a library assuming a host).

The user asked for them on 2026-10-24 ("same as compiling and running all BaseLib tests for the 65C64, make the
compiler support all tests in StandardLib"), and for TestSupportLib's the same day.  They paid for themselves at
once.  Making them pass found, among others: by-value `operator[]` on an object, qualified member calls
(`obj.Base::f`), local anonymous unions, `alignof` and `static_assert` in functions, a virtual function dropped
from the vtable when a parameter's type was an undefined template instance, colliding labels of function template
instances, `= default` assignment, pointers to virtual member functions, `#pragma pack`, `sizeof` of a member
array, temporaries in conditions that leaked, temporaries made on one side of `&&`, `||` or `?:` destroyed on both,
by-value parameters never destroyed when a function had no `return`, and nested local array initialisers with brace
elision.

The W65C816 ports did the same for the libraries: a 16 bit `int` and [`size`] found every count that could pass
64KB (see [The W65C816 ports](#the-w65c816-ports)).

## The harness

TestBaseLib65, TestStandardLib65 and TestSupportLib65 share one harness, [`TestBaseLib65/TestProject65.cpp`].  A
suite is a table: the test project's name, its libraries in link order, the tests left out (on both processors, and
on the W65C816 only) and the minimum programs and asserts each processor must run.

### Compile once, link a program per test

The test project and its libraries are each added to one [`CCPP65Builder`] as a library of units linked only when
used ([`CCPP65Builder::AddVisualStudioProject`]), with Libc65's units, the start up and the maths tables, and
compiled once.  The harness then reads the test project's `main` file, takes each `Test...();` call inside `main`
as a test, and links each as its own program: the same file with only that call left in `main`
([`CCPP65Builder::LinkWith`]).

A program per test, rather than the whole test project as one program, because the whole TestBaseLib was 3.4MB on
the 65C64 (2026-10-24) and doesn't fit below the stack.  It also means a test that crashes or runs out of
instructions costs only itself, and each program's failure is reported on its own.

On the 65C64 a program may go up to 0x1C0000 ([`CCPP65Builder::SetLimit`]), under the stack, which starts at
001F:FFFF.  The W65C816's code is bigger, and its programs may go to 0080:0000.  For the W65C816 the 65C64 assembly
units of Libc65 are replaced by their W65C816 assembly (`XW65C816.asm` and `XW65C816_16.asm` for `X.asm`) where
there is some, and by their C++ where there isn't.

The libraries' host code is replaced, not compiled around: BaseLib has `*_65C64.cpp` platform files (for example
[`BaseLib/Mutex_65C64.cpp`]), compiled under `__LIBC65__` for both processors, with one thread, no shared libraries
and files through the host file device.

### The programs' files

Tests read and write files beside them (TestFileUtil's directories, the FAT images), so a program runs in a copy of
its test project's folder, never in the folder itself.  Libc65's `stdio` and `host.h` reach the host's files
through the simulator's host file device at 00FF:0200 ([`CAsm65HostFiles`]), rooted at that copy, and print through
the console at 00FF:0100.  The copy is `Output/<target>/<project>` (`Output/<target>/<project>.<n>` for worker n),
without the folder's own `Output`, which is made empty: what an earlier run left there isn't copied.  The copies of
TestBaseLib's folder are 147MB each (2026-11-19).

Each processor has its own copies, so a suite's 65C64 and W65C816 runs can go at once.  Two runs of the same suite
on the same processor share them and must not run at once.

### Workers

Linking stays one program after another (the builder's objects are reused), but each program, once linked, is
loaded into a memory of its own with a copy of its map and run by the next free worker while the next is linked
([`CWorkerThreadJobs`], the compile's own jobs).  There are 4 workers unless `TEST_PROJECT65_RUNS` says otherwise,
each with its own copy of the folder.  The results are kept and asserted in the programs' order once all have run,
so `Log.txt` and the counts read as if they had run one after another.  The 65C64s share a table of decoded
opcodes, which [`CFast65C64::DecodeAll`] fills before they run side by side.  The profile, `TEST_PROJECT65_CYCLES`
and `TEST_PROJECT65_TRACE` print as they go or share one profile, so they run the programs one at a time.

Measured on 2026-11-19 (Release x64, the same programs and asserts, one at a time against four workers):

| Suite | 65C64 | W65C816 |
|---|---|---|
| TestBaseLib65 | 123 s to 46 s | 225 s to 84 s |
| TestStandardLib65 | 191 s to 85 s | 170 s to 71 s |
| TestSupportLib65 | 464 s to 167 s | 664 s to 214 s |

Before that, 84% of TestSupportLib65's W65C816 run was its programs' runs (2026-11-19).  A suite's own *Tests Run*
count is higher with workers: each worker's copy of the folder asserts as it copies.

### Steps and cycles

Each program gets 4,000 million instructions; one still running then has failed.  The limit keeps a whole run to
minutes, and a test that needs more is excluded, with the count it took when given longer (`TEST_PROJECT65_STEPS`).
The programs run a million instructions at a time.  On the W65C816 each function's stores to the direct page are
checked as it runs ([`CAsmW65C816FrameCheck`]): a store into a caller's frame is a failure even when the test
passes.  It about doubles a run (TestSupportLib65 on the W65C816, 2026-11-19: 335 s without it, 664 s with it).

### Memory and the heap

The simulator's devices are in 00FF:0000 to 00FF:FFFF, which is never memory ([`CFast65C64::SetHole`]); the memory
size register at 00FF:0300 tells Libc65's [`malloc`] how much there is, and [`Libc65/stdlib.cpp`] places the heap
from it:

| | 65C64 | W65C816 |
|---|---|---|
| Memory | 64MB | 16MB (24 bit addresses) |
| Program | from 0001:0000, at most to 001C:0000 | at most to 0080:0000 |
| Heap | 0100:0000 to 0400:0000, 48MB, past the devices | from the program's end (or 0020:0000) to the devices, by 64KB banks ([`CBankMallocator`]) |

Libc65's heap is BaseLib's [`CTrueMallocator`]: [`malloc`] and [`free`] take the same time however many blocks
there are, and a freed block is joined to the free blocks either side of it.  With 2MB of memory or less the heap
is 0008:0000 to 000F:FFFF, which TestLibc65 uses.

### What a failure looks like

Asserts in the host program break (`int3`), so run a suite with `BASELIB_BREAK=0` in the environment to see every
failure ([BaseLib](../../libraries/baselib/index.md#stopping-on-an-error)).  Each program is checked with one assert: its test's name, expected, against what happened.
What happened is the name alone when the program linked, returned with the stack where it started, printed no
failed asserts and made no bad stores; otherwise the name and one of:

- *did not link*, with the linker's errors;
- *is missing*, with the labels it uses that no object defines ([`CCPP65Builder::GetMissingNames`]): they would be
  linked as address 0, and a call to one would run off into bank 0, so the program isn't run;
- *failed*, with the W65C816's bad stores and the program's console, where its own failed asserts are;
- *did not finish*, with its console and where it stopped: the address and the symbol at or before it, from the
  link's map, S, and how many million instructions ran; on the W65C816 also the return addresses of the calls on
  the stack, innermost first.

The compile's errors are printed before its assert, and its warnings always: on the W65C816 the warnings were the
list of 16 bit problems to fix.  After the programs, a suite asserts that it ran at least its minimum programs and
asserts on that processor, so a test silently not run (it stopped linking, the `main` file changed shape) fails the
suite.  `TEST_PROJECT65_ONLY` runs the tests it names, excluded or not, and skips the minimums.

## The suites

| Suite | Libraries | Test calls in `main` | Minimums 65C64 | Minimums W65C816 |
|---|---|---|---|---|
| TestBaseLib65 | TestLib, BaseLib | 204 | 101 programs, 30,001 asserts | 155 programs, 30,001 asserts |
| TestStandardLib65 | TestLib, StandardLib, BaseLib | 73 | 57, 4,877 | 55, 4,662 |
| TestSupportLib65 | TestLib, SupportLib, StandardLib, BaseLib | 86 | 51, 25,533 | 47, 17,051 |

(Test calls counted 2026-11-26.)  Some tests are empty and print *No Tests Run!*, such as TestMeshColours.  Each
suite's tests left out are in its own file ([`TestBaseLib65/TestBaseLib65Tests.cpp`],
[`TestStandardLib65/TestStandardLib65Tests.cpp`], [`TestSupportLib65/TestSupportLib65Tests.cpp`]), each with its
reason.  The counts are the 65C64's instructions or the W65C816's cycles each took when given longer, as the lists
record them (Release).

### TestBaseLib65

TestBaseLib65 also makes `BaseLib/FatW65C816.inc` ([`TestBaseLib65/TestFatW65C816Layout.cpp`]): the FAT assembly
reads and writes the FAT classes' members, so the W65C816 compiler itself works out their offsets, sizes and
constants, and the test fails when the generated file no longer matches the one in BaseLib.

| Left out | On | Why |
|---|---|---|
| TestCodabase, TestWriteTransaction, TestReadTransaction, TestTransactionConcurrency | both | Threads: the 65C64 runs one, and [`BaseLib/NativeThread_65C64.cpp`] starts none. |
| TestThreadMallocator, TestThreadMallocatorSpeed | both | [`CThreadMallocator`] isn't built for the 65: Libc65's [`malloc`] is a [`CTrueMallocator`] alone. |
| TestNumber, TestFloatPrinter, TestNamedIndexedData, TestIndexTreeAccess, TestFat32, TestFatStress | both | Hundreds of millions to billions of instructions natively (0.7G to 9.5G, 2026-10-24): far too many to simulate. |
| TestFat32Bugs | both | 14G cycles on the 65C64, 44G on the W65C816. |
| TestFatImageFile | both | Reads and writes an 8MB disk image whole. |
| TestFatBench | both | Private: run with `TEST_PROJECT65_ONLY`. |
| TestTemporaryMemory | both | Expects a bigger block allocated after a [`free`] to be somewhere else, as MSVC and glibc give it; Libc65 joins the freed block to the free memory after it, so the bigger block starts in the same place. |
| TestListVariable | W65C816 | Allocates more than 16MB. |
| TestMapStringIntAndPointer | W65C816 | Over 4G instructions; passes given longer. |

### TestStandardLib65

| Left out | On | Why |
|---|---|---|
| TestCompressionZLib, TestMapObject, TestSetObject, TestArrayObject | both | Over 4G instructions: 5.1G, 9.7G, 12.7G and 20.0G on the 65C64; they pass given longer. |
| TestObjectsSpeed | both | Times the host's object system, and reads how long to run from the environment. |
| TestObjectsThreads | both | Threads. |
| TestSetUnknown, TestMapUnknownUnknown | W65C816 | Over 4G instructions (TestSetUnknown is 3.4G on the 65C64); pass given longer. |

TestSetUnknown was 5.0G on the 65C64 when the suite was made.  Rather than leave it out,
[`CFreeList::FindNextAllocatedElement`] was made to scan its bit array a word at a time (2026-10-24: 4.77G to
3.43G, and TestBaseLib65 308 s to 245 s), so the suite runs it.

### TestSupportLib65

| Left out | On | Why |
|---|---|---|
| TestTrueTypeFont, TestModPlayer, TestFont, TestSoundResampler, TestMeshShapes, TestImageResampler, TestImageOpacity, TestNormalGeneration | both | Over 4G instructions: 5.1G, 9.8G, 16.6G, 16.8G, under 20G, 24.0G, 24.6G and 35.2G on the 65C64; they pass given longer. |
| TestImageCombiner | both | Its 18MB combined image doesn't fit in the 48MB heap: [`SavePNG`] can't allocate the memory to write it, after 51.6G instructions. |
| TestImageSFTWriter, TestRectangleBestPackerWithRotation | W65C816 | Over 4G instructions: 17.3G and 18.6G cycles; they pass given longer. |
| TestSceneGoober, TestSceneGooberResave | W65C816 | Their images are 33MB, more than 16MB. |

TestModPlayer passes on the W65C816 too, in 76.3G cycles (2026-11-06), since [`CModPlayer::RenderSong`] renders
into the sound a block at a time: it kept the whole song as [`float64`]s, about 30MB, more than the W65C816 has.

### TestLibc65

TestLibc65 (HCP65, 2026-10-23) has a harness of its own, [`TestLibc65/TestLibc65Context.cpp`]; it shares only the
choice of processor ([`TestBaseLib65/TestTarget65.cpp`]) with the others.  Each test is a small C++ program in a
string, built with the Libc65 units it names, BaseLib's [`CTrueMallocator`] (and [`CBankMallocator`] on the
W65C816) and the start up.  The labels it uses that nothing defines must be exactly the list the test gives; the
linker makes each a `stp`, so calling one stops the program.  The program writes its results as 32 bit values from
0010:0000 and bytes from 0010:8000, and the host reads them back.  Memory is the default 2MB, so the heap is
0008:0000 to 000F:FFFF.  A program that uses files gets the host file device in `Output/Host<target>`, emptied
first.  Every test ends by asserting that the program made no access outside memory
([`CAsm65Memory::NumBadAccesses`]).  The W65C816 gets 400 times the 65C64's instruction limit: with a 16 bit `int`
and soft float in C++ it runs many more instructions for the same C++.

It covers `<string.h>`, `<ctype.h>`, `<stdlib.h>` (conversions, [`rand`], [`qsort`], allocation, its overflow and
fragmentation, banks, [`exit`], [`_Exit`], [`atexit`], [`abort`], [`signal`]), `<time.h>`, `<stdarg.h>`, the
limits, exceptions, `<stdio.h>` on the host file device, and soft float (arithmetic, conversions and the exactly
rounded functions).

The first run had 1,227 checks and 35 failures, all in Libc65 and all fixed that day: [`rand`]'s multiplier,
[`strtol`] and [`strtoul`] (white space, `0x`, a sign alone, overflow) and [`mktime`]'s normalising.  The
allocation tests then made [`malloc`] refuse more than the heap before rounding, [`calloc`] check its count times
size, and [`free`] keep the free list in address order, joining neighbours and giving back the top block.  [`exit`]
looped for ever until the start up grew an [`_Exit`] that stops at once ([`CppCompilerLib/CPPTarget65C64.cpp`]).

## The W65C816 ports

All four suites took the W65C816 as their second processor once the back end could run them; TestSupportLib65 was
ported last (2026-10-30, the user asked), and passed with 44 programs and 12,258 asserts that day.  The work is how
the libraries became correct at a 16 bit `int` and, later, a 16 bit [`size`]; the rules themselves are in [W65C816
Back End Design](w65c816.md#widths-int-size-and-lsize).

What the libraries changed, from SupportLib's port:

- **Counts past 64KB are [`lsize`]**: [`CChannels`]' size, positions and byte sizes, its data a
  [`CArrayLargeChar`], [`CChannelsCopier`] and bit offsets; [`CImage::GetByteSize`] and pixel sizes; the PNG, TGA,
  BMP, RAD, WAV, SFT and TrueType file offsets; [`CSound`]'s byte sizes, and later its frames (2026-10-30, the user
  asked: as [`size`] a sound stopped at 65,535 frames and TestModPlayer failed on the `BASELIB_SIZE16` host build).
  Channel IDs stay [`size`].
- **32 bit values say so**: packers' coordinates, image resampler sizes, tile layer positions and BMP/TGA/PNG
  widths became [`int32`], and their loop counters with them; [`CInt32Rect`]'s [`int64`] initialiser was removed.
- **Shifts and products widened before they wrap**: an [`ARGB32`] colour's `255 << 24` is cast to [`ARGB32`] first,
  row strides are [`lsize`], and tests' sums such as `200 * 255` moved out of `int`.
- **Sound purposes fit 16 bits**: [`ESoundPurpose`]'s [`SP_Speaker`] went from 0x100 to 0xF0, so an [`EChannel`]
  fits.
- **Variadic counts**: a 16 bit [`size`] reaches `...` as an `int`, and g++ traps `va_arg` of a 16 bit type, so
  every `va_arg` of a size or channel is `VA_ARG_SIZE` ([`BaseLib/PrimitiveTypes.h`]).
- **Tests assert 32 bit actuals as 32 bit** (`AssertInt((int32)...)`) and expected WAV sizes are [`uint32`].

The compiler grew catchers from the same work, because every one of these bugs was silent: a warning or error
inside a macro names the outermost macro use ([`CCPPDiagnostics::AppendMacroUse`]: *(in AssertInt at TestX.cpp (l,
c))*), a shift by at least the operand's width ([`CCPPWarnings::ShiftCount`]), a comparison always true against the
type's largest or smallest value ([`CCPPWarnings::ComparisonAtLimit`]), and arithmetic widened after it may have
wrapped, looking through `+`, `-`, `|`, `^` and `&` ([`CCPPWarnings::NarrowMultiply`]).  Fixing what they found
reached BaseLib's and StandardLib's tests and libraries too (FAT sector counts as [`uint64`], the memory queue,
Codabase, a 32 bit LCG in TestTransactionConcurrency).  None of them sees a wrap within one width (`uiRow *
uiRowFrames` both [`size`]): the last TestModPlayer bug was one.

Profiling the ported suite on the W65C816 (2026-10-30) put the [`CChannels`] accessors under 0.5% of its cycles and
C++ soft float at about 90%, most of it 64 bit shifts by a variable count.  So variable shifts were made fast (the
user asked): a count below 5 loops a bit at a time, a larger one moves whole bytes and words first and then jumps
into 7 unrolled one bit steps ([`CIRW65C816Selector::ShiftInA`]); 64 bit shifts and far frames do the same in
memory ([`CIRW65C816Selector::ShiftInMemory`]), which took TestSupportLib65 on the W65C816 from 77.0G to 54.2G
cycles.

## Decisions

- Libc65 is tested on the 65C64, not natively; the Libc65 tests already in TestCppCompilerLib stay, and new ones go
  in TestLibc65 (the user, 2026-10-23).
- The libraries' real tests run on the simulators, from the libraries' real sources (the user, 2026-10-24).
- One program per test: the whole of TestBaseLib (3.4MB) doesn't fit below the 65C64's stack (2026-10-24).
- Free C++ functions are labelled with their parameter types (`_Sum__FPKij`); `extern "C"` functions, `main` and
  the global `new` and `delete` are plain (2026-10-24, my choice for the libraries' overloads; the user hasn't
  ruled on it).
- The 65C64's heap is above 2MB, past the devices when there is more than 16MB (2026-10-24, my choice; the user
  hasn't ruled on it).
- A test too slow for 4G instructions is left out with the count it took, not given a longer limit: a whole run
  stays minutes (2026-10-24).  Speeding the library up instead is preferred where it is the library's cost
  (TestSetUnknown, 2026-10-24).
- [`CChannels`] counts in [`lsize`]; a "natural + large" choice inside [`CImage`] was tried and dropped by the user
  (2026-10-30): the accessors were under 0.5% of the W65C816's cycles.  Pixel speed is to come from W65C816
  blitters over image cels, not from the generic accessors (the user, 2026-10-30: "The real optimisation is going
  to be needed when we start using ImageCels to do rendering ... 816 specific blitters").
- [`CSound`]'s frames are [`lsize`] (the user, 2026-10-30).
- A variable shift loops below 5 bits (the user's choice, 2026-10-30).
- Programs run side by side on workers, each with its own copy of the folder (TestSpeedPlan.txt step 3,
  2026-11-19).
- Each processor writes its own `Output/<target>`, and an unknown argument exits 2 instead of quietly running the
  65C64 (2026-11-14).
- The 65 suites are built and run in Release only, on both processors (the user, 2026-11-08 and 2026-11-19).

## Not done

- The tests left out above run only by hand (`TEST_PROJECT65_ONLY` with `TEST_PROJECT65_STEPS`); TestImageCombiner
  and the W65C816's TestListVariable and TestSceneGoober can't pass at all in these memories.
- The 65 suites aren't in the ASan sweep; on 2026-11-09 that had hidden use-after-frees in the compiler, which a
  run of them in an ASan build found.
- TestLibc65 doesn't test the transcendental functions ([`sin`] to [`pow`]), [`difftime`] or [`strftime`]; `assert`
  does nothing on the 65 yet.
- The 65C64's minimum of 101 programs in TestBaseLib65 is far under the 188 it runs; the minimums are floors set
  when each was last raised.

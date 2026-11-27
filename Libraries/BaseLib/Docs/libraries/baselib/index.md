# BaseLib

BaseLib is the library every other Codaphela library is built on: containers, strings, memory allocators, files and
file systems, text parsers, numbers, the line geometry ported from Logicim, logging, threads and the Codabase
database.  It depends on nothing but the C runtime and the platform's headers.  The same sources build for Windows
(MSVC, Win32 and x64), Linux (g++, 32 and 64 bit) and the HCP65's two processors, the 65C64 and the W65C816, through
the project's own C++ compiler; the W65C816's 16 bit counts shape much of its design.

Its house style is the one every library follows: classes with Init and Kill instead of constructors and
destructors, the library's own primitive types ([`BaseLib/PrimitiveTypes.h`]), memory through a [`CMallocator`], and
a template written once and compiled once where it can be.

## The parts

The project's filter tree ([`BaseLib/BaseLib.vcxproj.filters`]) groups the files like this:

| Part | What is in it |
|---|---|
| Primitive types | [`BaseLib/PrimitiveTypes.h`], strings ([`CChars`], immutable and external strings), dates, GUIDs, type names and conversion, pointer remapping. |
| Collections | Arrays ([`CArrayTemplate`], primitive, embedded, bit, sorted, sparse and dense), lists and linked lists, the index trees, sorted maps ([`CMapBlock`]) and sets, hash maps and sets ([`CHashMapTemplate`]), a queue ([`CQueueTemplate`]), a binary heap ([`BinaryHeapPush`]), a union-find ([`CUnionFind`]), a red-black tree ([`CRedBlackTree`]), searching and sorting.  See [Containers](containers.md). |
| Geometry | Vectors in each primitive width, 4x4 matrices, quaternions; lines on an integer grid ([`CLine`]).  See [Numbers and Lines](numbers-and-lines.md). |
| Memory | Mallocators, the general and data memories, free lists, slabs, the TLSF allocator ([`CTrueMallocator`]) and the allocators built on it, memory caches, leak tracking.  See [Memory](memory.md). |
| Object life cycle | Constructors, [`CKillable`], life cycles. |
| Files | File chains (disk, memory, buffered, hashing), readers and writers, chunked and packed files, CSV, text, Intel hex, properties and XML files, durable files, log files, the indexed data files under the Codabase. |
| File systems | FAT16 and FAT32 (also written by hand as W65C816 assembly, [`BaseLib/FatVolumeW65C816.asm`] and its neighbours), over a file or memory drive. |
| Database | The Codabase and sequences: see [The Codabase](codabase.md). |
| Text | [`CTextParser`] and its configurations, the XML, JSON and YAML parsers and the markup tree they make, Unicode encodings, a plain text editor.  See [Parsers and Markup](parsers-and-markup.md). |
| Math | Integer, float, character and fixed point helpers, arbitrary precision numbers ([`CNumber`]), random numbers, a calculator. |
| Errors and logging | [`CLogger`], error handling, debug output. |
| Platform | Threads, mutexes, condition variables, jobs, sockets, timers. |
| Function calling | The function callers made from generated `*_Call.inl` files. |

## Stopping on an error

Two things stop a program where something went wrong, each with a breakpoint (`int3`: in a debugger it stops on the
line, and with none the run ends, on Linux with SIGTRAP):

- The logger, on an error or a warning it is given ([`CLogger::Break`]), when its configuration says so.  A build
  with `BREAK_ON_ERROR` or `BREAK_ON_WARNING` defined starts with it on; [`CLogger::SetBreakOnError`],
  [`CLogger::SetBreakOnWarning`] and [`CLogger::SetSilent`] change it while the program runs, as a test that expects
  an error does.
- A failed test assert: every `Assert...` of TestLib is a `Validate` ([`BaseLib/Validation.h`]), in every build.

The environment variable `BASELIB_BREAK` decides for one run without building again ([`GetBreakSwitch`], read once):

| `BASELIB_BREAK` | The logger | A failed assert |
|---|---|---|
| not set | as built | breaks |
| `0` | doesn't break | is counted, and the run goes on to its totals |
| `1` | breaks on errors and warnings | breaks |

So a run that has to get through its failures (a sweep, `ctest`) sets `BASELIB_BREAK=0`, and one that should stop at
the first is left alone.  A program for the HCP65 has no environment and breaks as built.

## Sizes on every target

Counts and sizes are written in two types from [`BaseLib/PrimitiveTypes.h`]:

- [`size`] is the target's natural count: [`size_t`] on the hosts (`unsigned long long` on 64 bit Linux, so that it
  is the same type as on MSVC and overloads resolve the same way), and 16 bits on the W65C816, where `SIZE_16` is
  defined.
- [`lsize`] counts what can pass 64KB.  It is [`size`] everywhere but where `SIZE_16` is defined, where it is 32
  bits. So only on the 16 bit target can a function be overloaded on [`size`] and [`lsize`] (elsewhere
  `SIZE_IS_LSIZE` is defined).

The W65C816's size model is checked on the host by the `BASELIB_SIZE16` build: 32 bit g++ with [`size`] 16 bits and
`-Wconversion`, which warns of every [`lsize`] narrowed to a [`size`].  Arrays, strings and hash maps come in a
natural form counted in [`size`] and a large one counted in [`lsize`] for this reason ([Containers](containers.md)).

`BUILD_64` is defined when pointers are 8 bytes (`_WIN64` or `__LP64__`); `LINUX_GNU` on Linux.  g++ in 32 bit
aligns 64 bit types to 4 bytes in structs and MSVC to 8, so `ALIGN_64` and `MSVC_ALIGN` make g++ match MSVC: structs
read and written as raw bytes have one layout on both.

Files are the same in 32 and 64 bit builds.  [`CFileWriter::WriteSize`] writes four bytes, and
[`CFileReader::ReadSize(lsize*)`] sign extends them, so all ones reads back as `MAX_SIZE`.  Container headers are
written field by field in the 32 bit layout (`ARRAY_FILE_POINTER_SIZE` for an array of pointers' element size,
`LINKED_LIST_FILE_NODE_SIZE` and `LINKED_LIST_BLOCK_FILE_NODE_SIZE` for linked list nodes); element payloads are
written as their raw bytes.

The Release builds on Linux add `-U_FORTIFY_SOURCE` and `-fno-lifetime-dse` to match what MSVC does: a fortified
[`strcpy`] aborts on a class's name stored past the end of its object, and g++ drops stores made in a destructor
that [`CObject`]'s destructor relies on (the reasons are in each test's `CMakeLists.txt`).

## Pages

- [Containers](containers.md): the array templates and their size policies, strings, hash maps and sets, the queue,
  heap, union-find and red-black tree.
- [Memory](memory.md): mallocators, the general and data memories, free lists and slabs, the TLSF allocator and what
  profiling found.
- [Numbers and Lines](numbers-and-lines.md): [`CNumber`] in packed BCD, and the line geometry from Logicim.
- [Parsers and Markup](parsers-and-markup.md): the text parser, the XML, JSON and YAML parsers and the markup tree.
- [The Codabase](codabase.md): the object database.

## Decisions

- BaseLib and the libraries above it build 32 and 64 bit, with Visual Studio's x64 targets mirroring its Win32 ones
  (the user, 2026-10-21; before that the rule had been 32 bit only, Linux built with `-m32`).
- One file format in both builds: sizes are written as four bytes and container headers in the 32 bit layout (the
  user's choice, 2026-10-21).
- New code stays 64 bit clean: [`size`] for sizes and counts, explicit casts to fixed width types for file and API
  fields.  MSVC x64 treats narrowing a [`size_t`] (C4267) as an error, where Win32 only warns at level 4
  (2026-10-21).
- Codaphela compiles under both g++ and MSVC (the user, 2026-11-15).

The list of parts above replaces `BaseLib/LibraryBreakdown.txt`, which is in the history up to CommonLibraries
533f71ec5.

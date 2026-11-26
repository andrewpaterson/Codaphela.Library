# Self-Hosting

The user's goal (2026-10-31): "the C++ compiler needs to be self hosted on an '816".  The compiler is
CppCompilerLib, Assembler65Lib's assembler and linker, and the BaseLib under them; self-hosted, the same program
that compiles for the W65C816 on a PC is compiled for the W65C816 and runs there.  There is no plan file: this page
is the goal's design, where it stands and what is left.  How the W65C816 target itself is built is [W65C816 Back
End Design](w65c816.md).

Nothing has run on a W65C816 yet.  The compiler compiles and links for the W65C816 in a survey (below), and the
work so far went into making it need less memory, which on the host it never had to care about.

## What it has to fit

The machine as planned on 2026-10-31: a 16MB address space with 512KB of ROM and 512KB of RAM, and 128MB of SDRAM
as a block device for what doesn't fit.  The numbers that decide the design, measured on 2026-10-31 on the host (32
bit), summed over the 819 units of the whole compiler:

| | |
|---|---|
| Parser tokens | 29M |
| Preprocessed tokens | 1,001MB ([`SPreprocessedToken`] is 28 bytes; macro bodies and output share it) |
| C++ tokens | 1,426MB at the start of the work, 531MB after |
| Syntax tree and symbols | 5,182MB eager at the start, 4,981MB after; with lazy headers 3,860MB, then 3,003MB |
| The biggest unit, `CPP65IRFrontEnd.cpp` | 14MB preprocessed tokens, 7MB C++ tokens, 25MB tree (60MB eager) |
| The compiler linked for the W65C816, unused code removed | 3,368,051 bytes |
| Its biggest parts | [`CIROptimiser`] about 267KB, the W65C816 back end 180KB, [`CCPP65IRFrontEnd`] 171KB, the 65C64 back end about 0.2MB |

So memory and code size decide self-hosting, not the language: everything the compiler compiles already compiles.

## What is built

Each change was checked by the output staying byte for byte the same: the whole compiler built for the W65C816, its
image's hash and its map compared before and after.

- **The assembler reads its source a line at a time** ([`CAsm65Source`], [`CAsm65TextSource`],
  [`CAsm65KeptTokens`]), and the unit's tree and C++ tokens are freed before it assembles.
- **A spill store** ([`CSpillStore`]): keyed blobs of at most 64KB, in memory ([`CMemorySpillStore`]) or in an
  index tree file ([`CIndexTreeSpillStore`], records padded to 1KB size classes, committed after more than the
  cache size has been put).  A unit's assembly ([`CCPP65Assembly`]) keeps only its skeleton in memory, each
  function's code in pieces in the store ([`CCPP65Assembly::PutCode`]); [`CCPP65Builder::SetSpillDirectory`] gives
  it a directory.  The user plans to put the index tree on the SDRAM block device.
- **A unit is preprocessed just before it is compiled**, and its tokens and file are let go after
  ([`CProject::ProcessUnit`]).
- **Lazy headers** ([`CCPPHeaderIndex`], [`CCPPSyntaxLazyDeclaration`], [`CCPPScope::LoadLazy`]): a header's
  declarations are recorded by where they start, with a variant for each set of macros a header was included under,
  and a later unit skips them and declares their names lazily, parsing a declaration only when a name is used.  A
  member function defined outside its class waits on its class.  A class is never skipped when a symbol of its name
  was already parsed.  Forward and friend declarations stay eager: making the class definition after a forward
  declaration lazy changed the image, so it was put back.  Lazy headers are on by default
  ([`CCPP65Builder::SetLazyHeaders`]).
- **Smaller tokens**: identifiers are numbered by a hash set with a name pool ([`CASCIIIndex`]), where two byte
  tries had cost about 530 bytes an identifier.
- **BaseLib**: a sorted array block makes its insertion indices only when it needs them (an empty scope cost 4KB),
  and an evicting index tree flushes deleted nodes when its strategy can't evict.
- **The header cache** ([`CPPHeaderCache`]): a header preprocessed once with the same macros is replayed for every
  unit that includes it.  The 2026-10-31 survey found all 402 headers expanded the same in every unit, which is why
  it pays.  It is kept in memory, for the builder's threads, not on the spill store.

The 16 bit [`size`] ([W65C816 Back End Design](w65c816.md#widths-int-size-and-lsize)) came out of the same goal:
the natural width keeps counts cheap, and what can pass 64KB says so with [`lsize`].

## What blocks it

To run at all:

1. **Natural containers stop at 64KB.**  [`size`] is 16 bits on the W65C816 and a natural array holds 64KB of bytes
at most.  The preprocessor's [`CArrayPreprocessedTokens`] is a natural [`CArrayTemplate`] of 28 byte tokens, about
2,300 of them, where real units have 30,000 to 380,000; the parser's token pointers ([`CCTArrayTokens`]) stop at
about 16,000, maps at about 16,000 entries, and the units' declaration arrays, symbol tables, the assembler's token
arrays and its segments' bytes are likely past it too.  They need the large arrays ([`CArrayLargeTemplate`] and the
others) or containers in chunks.  This is needed whatever else is decided.  2. **Small gaps**:
[`CCPPPreprocessor::InitPlatformSpecific`] is defined only for Windows and Linux hosts, so a W65C816 build of the
preprocessor has none; Libc65 has no `sscanf`, which [`CAsm65Profile::Read`] uses.  3. **Files**: Libc65's `stdio`
goes through the simulator's host file device ([`HostOpen`] ...).  On the machine it needs the FAT32 file system.

To fit the machine:

4. **Code size**: 3.37MB linked against 512KB of ROM.  This is the biggest decision left, and the user's: code
loaded on demand (overlays, or from the SDRAM or the card), or a smaller compiler (without the 65C64 back end and
the optimiser's less essential passes), or both.  5. **Data per unit**: the biggest unit still holds about 46MB
when parsed (2026-10-31), then its IR, assembly and object.  Candidates: - the header cache's tokens on the spill
store, so each header is kept once instead of preprocessed per unit; - a smaller [`SPreprocessedToken`] (24 bytes
without restructuring the preprocessor); - each function's IR on the spill store, instead of the whole unit's kept
for inlining across its functions; - the linker's memory for about 800 units' objects and the map; - the lazy
header gaps: class definitions after a forward declaration (about 2.8M tokens; the generator's class registry would
have to cope with classes loaded late), friend declarations (0.5M), and `Codabase.cpp`, which was bigger lazy
(51MB) than eager (49MB), unexplained.  6. **Speed** is unmeasured.  A big unit takes about half a second on the
host, so on a W65C816 at a few MHz a unit may take minutes to hours.

## Measuring

The survey compiles and links the whole compiler for the W65C816 ([`CPPT_W65C816`]) from inside a private copy of
TestCppCompilerLib, with environment variables to pick a library or a file, to turn lazy headers off, and to write
the map and the listings.  Run it from a snapshot of the repositories (`git archive`), never the shared checkout,
whose `Output` other sessions use.  Compare a change by the image's hash and the map, eager against lazy and old
against new.  The process's peak on the host was 212MB before the lazy header work and hasn't been measured since.

## Decisions

- The goal is the compiler self-hosted on the W65C816 (the user, 2026-10-31).
- Memory and code size come first: language support is already enough (2026-10-31).
- The spill store's index tree is meant to live on the planned 128MB SDRAM block device (the user, 2026-10-31).
- Lazy headers are on by default (2026-10-31).
- Changes to make the compiler smaller must leave its output byte for byte the same (2026-10-31).

## Not done

- Nothing has been run on a W65C816.  The next step offered to the user (2026-10-31, not taken up yet): a smoke
  test, a compiler driver ([`CCPP65Builder`] compiling one small unit) built for the W65C816 and run on the
  simulator with 16MB or more, so the list above becomes a list of failures.
- Every item under [What blocks it](#what-blocks-it), and the user's decision on code size (item 4).
- The survey program isn't in a repository.

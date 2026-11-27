# The Processor Simulators

MicroprocessorLib simulates three processors: the 65C64, the W65C816S and the W65C02S.  Each has a cycle level
simulator, which drives the processor's pins on every clock edge, and the 65C64 and the W65C816S also have an
instruction level one, which does the same an instruction at a time and is what programs run on.

| Processor | Cycle level | Instruction level |
|---|---|---|
| 65C64 | [`C65C64`], run by [`C65C64Meta`] | [`CFast65C64`] |
| W65C816S | [`CW65C816`], run by [`CW65C816Meta`] | [`CFastW65C816`] |
| W65C02S | [`CW65C02`], run by [`CW65C02Meta`] | none |

How to run a program on the 65C64's simulators, and their memory and devices, is on the user page, [65C64
Emulator](../../65c64/emulator.md); what the 65C64's instructions do is in the [65C64](../../65c64/index.md) pages.
This page is how the simulators are built and why.

## The cycle level simulators

All three are built the same way, the W65C816S's first.  An instruction is a list of bus cycles; a bus cycle is a
list of address offsets, which add up to the address it puts on the bus, and a list of operations.  An operation
runs on the initial side of the cycle (after PHI2 falls, before the address is written) or on the trailing side
(when PHI2 falls at the end of the cycle).  An instruction factory makes every instruction once, as a singleton
([`C65C64InstructionFactory`], [`CW65C816InstructionFactory`], [`CW65C02InstructionFactory`]); the 65C64's and the
W65C816S's count their Inits and Kills, so each chip in a circuit can Init the factory for itself.  Code that runs
a meta processor directly must Init the factory first: without it the instructions are NULL and the processor
crashes or never finishes an instruction.

A meta processor ([`C65C64Meta`], [`CW65CMeta`]) owns the processor, its pins and the ideal traces the pins are
wired to ([`CMetaTrace`], [`CMetaBus8`], [`CMetaBus16`], [`CMetaBus32`]), and calls the memory back while PHI2 is
high and while it is low.  [`C65C64Meta::TickInstruction`] and [`CW65CMeta::TickInstruction`] run to the end of an
instruction (a W65C816S that is waiting returns after each cycle); `TestTickCycle` runs one cycle.  The pins hold
[`CLogicSignal`] and [`CLogicBus`] interfaces, not traces, so the same processor runs on ideal traces under a meta
or on analog ports as a chip in a circuit ([Logic Simulation](logic.md#processors-as-chips)).  Each meta embeds a
[`CTimeline`] for the pins' calls; it is never initialised, as ideal traces ignore it.

A processor's state between clock edges is plain data ([`S65C64CoreState`], [`SW65CCoreState`]), so a running chip
can be saved and loaded: the instruction and bus cycle aren't kept, they follow from the opcode and the cycle.

## The 65C64

[`C65C64`] is the reference for what the 65C64 does.  It follows the structure of the W65C816S simulator but shares
none of its processor code (the user, 2026-10-22).

- [`Decode65C64`] decodes a 16 bit opcode into an [`S65C64Decoded`]: the instruction, its operands and their
  widths.
- [`C65C64InstructionFactory`] maps all 65536 opcodes to instructions.  Opcodes whose bus cycles have the same
  shape share one: the key ([`S65C64Form`]) is the instruction, the number of operand words, whether it reads a
  pointer, how it accesses memory, its most transfer words, its most wait cycles and whether it reads a vector,
  hashed into a table of forms.
- [`C65C64InstructionCycleFactory`] builds the cycles from static helpers (`PC`, `E`, `DONE` and so on).  They are
  class members because free functions would clash with the W65C816S builder's of the same names
  ([`MicroprocessorLib/W65InstructionCycleBuilder.h`]).
- Memory data is a list of transfers ([`S65C64Transfer`]): a value of 1 to 8 bytes at any address.  Its aligned
  words follow from the address and the size.  An instruction reads the words of the transfers that are read,
  executes, then writes the words of those that are written; the bus cycles for words a transfer may not need are
  conditional ([`C65C64ExecuteIfLastTransferWord`]).
- Every bus cycle moves one aligned 16 bit word: [`C65C64AddressOffsetArray`] clears A0, so an odd C fetches the
  word below it.

Where the spreadsheet (`HCP65/X65C64-Proposal/65C64 Instruction Architecture.xlsx`) is silent or inconsistent the
user ruled, and [`MicroprocessorLib/65C64Simulator.txt`] lists every ruling: read it before changing what an
instruction does.  The instruction set pages give the behaviour.  The rulings that shape the simulator itself:

- An instruction executes in the cycle its opcode, or its last operand word, is read (the user's assumption).
  Address calculation takes no cycles.  An instruction is done when none of its remaining bus cycles must execute
  ([Timing](../../65c64/timing.md)).
- Repeating instructions (block, vector, `mul` and `div`) leave C on their opcode until their last step, so each
  step is an instruction and an interrupt can be taken between steps.
- Interrupts are taken at instruction boundaries, in place of the opcode fetch
  ([Interrupts](../../65c64/interrupts.md)).
- The processor is always in kernel mode; the interrupt depth is still counted (the user, 2026-10-22).
- An undefined encoding runs as a one cycle `#!text ill` that does nothing.
- A find with the zero register as its index reads 0, scans with an 8 bit index and drops the result.  This was
  Claude's choice (2026-10-24), when the fuzz test below found that the scan never ended; the user hasn't ruled.

The spreadsheet's pinout, user bank remapping, internal SRAM and programmable CPLD are outside the simulator's
scope, and so are the hardware register instructions (the user said to ignore them).  Where the spreadsheet and the
simulator disagreed on what was simulated, the sheet was changed to match the simulator (the user: "take the
simulator as truth", 2026-10-23): the Block Move sheet was rewritten from it and its examples run in
`Test65C64BlockMoveSheet` with the sheet's cycle counts.

## The instruction level simulators

[`CFast65C64`] and [`CFastW65C816`] do what the cycle level simulators do an instruction at a time, without pins,
bus cycles or a timeline.  Each is one `.cpp` (for the 65C64 at the user's request, 2026-10-24): the W65C816S's
switches on the opcode, the 65C64's on the instruction of a table of all opcodes decoded once.  They exist for
speed: the assembler's, the compiler's and Libc65's tests, the [65 suites](../../compiler/design/test-suites.md),
the X65C64 Emulator and the [HCP65 emulator](../../hcp65/emulator.md) all run on them.

| | Instructions a second, Release | Cycle level simulator | Measured |
|---|---|---|---|
| [`CFast65C64`] | about 100 M | about 2 M (about 50 times slower) | 2026-10-24 |
| [`CFastW65C816`] | about 149 M (about 550 M cycles) | about 6.3 M (about 24 times slower) | 2026-10-25, 32-bit |

They must match their cycle level simulator exactly, so they keep these rules:

- One `Step` is one `TickInstruction` of the meta processor: an instruction, a step of a repeating instruction (a
  65C64 block, vector, `mul` or `div` step; one byte of a W65C816S block move), a hardware interrupt, or one cycle
  while a W65C816S waits in WAI.
- An interrupt is taken at the end of the Step it follows.  The W65C816S samples IRQB and NMIB at the end of each
  Step, as the simulator does on an instruction's last cycle, so an interrupt is the next Step; NMIB is edge
  latched in both.
- They make the same memory accesses in the same order: the 65C64 reads aligned words and reads before it writes;
  the W65C816S makes only the accesses with VDA or VPA high (the emulation mode read-modify-write dummy cycle is
  not one).
- They count every clock cycle, internal ones and waits too ([`CFast65C64::NumCycles`]).
  [`CAsm65Memory::NumCycles`] counts only the valid bus cycles the memory saw, so the two differ.
- The 65C64 decodes with [`Decode65C64`] into a table of all opcodes, filled lazily or by
  [`CFast65C64::DecodeAll`].
- The W65C816S forces SH to 01 in emulation mode after each Step.  ABORTB and RDY are not modelled.

Memory is a flat block at address 0, read and written directly, and callbacks for every other address and for a
hole in the block where the devices are (`SetHole`).  The W65C816S looks memory up through a table of its 64KB
banks, a bank that isn't all plain memory going to the callbacks.  [`CFastW65C816::SetSignal`] reports the bus
events a computer around the processor needs to switch its memory map: a vector pull starting and its second byte,
an RTI, leaving emulation mode, a WDM (the HCP65 emulator's processor board uses them).

Both stop at breakpoints ([`CBreakpoints`]) and after an access to a watched address ([`CDataWatches`]), for the
debugger.  Neither costs anything until one is enabled: `Run` has a copy of its loop that doesn't look breakpoints
up, and a watched kind of access is taken off the fast path ([`CFastW65C816::UseWatches`] and
[`CFastW65C816::UseBreakpoints`] choose the sets a machine with a set per mode looks at).

### Keeping them the same

A change to what an instruction does is made in both simulators (`65C64.cpp` and `Fast65C64.cpp`; `W65CCore.cpp` or
the W65C816S's files and `FastW65C816.cpp`), and tests hold them together:

- `TestFast65C64` runs all 65536 opcodes from random state on both, over flat and callback memory, and compares
  everything after each step.  A planted carry bug was caught by it and by the lockstep tests.
- `TestFastW65C816` runs the 512 SingleStepTests files on the fast core, comparing accesses, cycles and the final
  state; a lockstep fuzz of every opcode in native and emulation mode against [`CW65C816`] (both made again for
  each trial, as a reused meta is mid-cycle); and IRQ, NMI, BRK, COP and WAI in lockstep in both modes.
- [`CAsm65C64Lockstep`] and [`CAsmW65C816Lockstep`] (Assembler65Lib) run whole programs on both side by side:
  [`TestAssembler65Lib/TestAsm65C64Fast.cpp`] and the compiler's runner
  ([`TestCppCompilerLib/TestCPP65Context.cpp`]).

## The W65C816S and the W65C02S

The two share their machinery.  [`CW65CCore`] is the engine: the registers and the instruction semantics they have
in common, with virtual hooks for what differs (the instruction set, the reset and interrupt sequences, the signals
on each edge of PHI2, which cycles read, the 8 bit decimal SBC).  [`CW65CPins`], [`CW65CMeta`] and the cycle
builder ([`MicroprocessorLib/W65InstructionCycleBuilder.h`]) are shared too, and all operations and address offsets
take a [`CW65CCore`].  What only one processor has stays out of the core.

- [`CW65C816`] adds native mode, 16 bit registers, banks, the direct page and its own instructions, with
  [`CW65C816Pins`] (VDA, VPA, E, MX) and [`CW65C816Meta`].
- [`CW65C02`] always runs as a W65C816S in emulation mode, with the direct page and both banks 0.  It adds RMB,
  SMB, BBR, BBS, SYNC, SOB and Bruce Clark's decimal SBC ([`CW65C02::Execute8BitBCDSBC`]).  Its 256 entry
  instruction table was generated by a script that wasn't kept.

The names follow one rule (the user, 2026-10-22): shared classes are `CW65<Name>` ([`CW65Instruction`],
[`CW65ReadDataLow`]), processor classes `CW65C816*`, `CW65C02*` and `CW65C*`, and files are named to match.  The
signal classes lost the prefix ([`CMetaTrace`], [`STraceValue`]) when the 65C64 began using them (the user,
2026-10-23).

### Matching the datasheet and the single step tests

On 2026-10-22 the W65C816S simulator was tested against the datasheet (nine test files, one per chapter: logic,
modes, stack, branches, interrupts, block moves, the direct page, decimal and timing) and fixed to match it, at the
user's request.  Then it was run against SingleStepTests' 65816 files (the first 200 tests of each of the 512,
converted by `Input/SingleStep/ConvertSingleStepTests.py`), and the W65C02S against its 65x02 wdc65c02 files (254,
without CB and DB).  Where the two sources disagree the simulator follows SingleStepTests, which record real chips'
bus cycles:

- The read-modify-write modify cycle addresses the last data byte read, not AA+1 as table 5-7 says.
- JMP and JSR (a,x) read their table with VDA, not VPA as table 5-7 says.
- The W65C02S's cycles are those of the 65x02 files where they differ from table 6-5: a read-modify-write reads
  twice then writes; ASL, LSR, ROL and ROR abs,X take 6 cycles without a page crossing, INC and DEC abs,X always 7;
  the decimal extra cycle reads 007F for ADC # and 0000 for SBC #; NOP 5C, DC and FC take 4 cycles, the x3 and xB
  NOPs 1; BBR and BBS re-read PC+3 on a page crossing.
- Decimal ADC and SBC follow bsnes's nibble model, V included ([`CW65CCore::ExecuteDecimal`]).
- In emulation mode P reads bits 5 and 4 as 1; B exists only on the stack (BRK, COP and PHP push it set, hardware
  interrupts clear).  The W65C02S test compares P without bit 4, the reference's bit 4 being an artefact.

The stack is addressed three ways ([`EStackPointerMode`]): S wrapped in page 1 in emulation mode, S+1 wrapped the
same way for 6502 style pulls, and all 16 bits of S for the instructions of the datasheet's caveat 7.1.  The direct
page wraps within the page in emulation mode when DL is 00 ([`CW65DirectPage`]).

NMIB is latched on its falling edge (the user: "Fix NMI in both", 2026-10-25).  WAI waits until an IRQ, an NMI edge
(not NMIB held low), ABORTB or reset; on waking the simulator chooses its next opcode again
([`CW65CCore::ChooseNextOpcode`]), because WAI had chosen the fetch before it waited, so the waking interrupt is
taken at once and a masked IRQ just continues.

## Shared code and duplication

The 65C64 shares no processor code with the W65 processors, but both sides use the same library pieces: the signal
classes ([`CMetaTrace`], [`CMetaSignal`], [`CMetaBus16`], [`CMetaBus32`], [`STraceValue`]), the timeline, the
[`CLogicSignal`] and [`CLogicBus`] interfaces, [`CArrayOwnedPtr`] for their bus cycle and operation arrays,
[`CBreakpoints`] and [`CDataWatches`] for the instruction level simulators, and the state saving of
[`MicroprocessorLib/StateXMLFields.h`].  The 65C64 had its own copies of the signal and timeline classes until
2026-10-23, when the user had it use the W65 ones.

What is still duplicated was surveyed on 2026-10-23 and, at the user's word ("just note for now"), left alone:

| Duplicated | Where | Sharing it needs |
|---|---|---|
| Operation, data operation, bus cycle, address offset (and its array), constant offset, execute, increment program counter, internal operation, wait, done instruction, read opcode, instruction, instruction cycles, the function pointer type | the same shape on both sides, `Execute(C65C64*)` against `Execute(CW65CCore*)` | a templated or common base CPU parameter |
| Bus cycle parameter ([`C65C64BusCycleParameter`], [`CW65BusCycleParameter`]) | identical but for the forward declaration | the same |
| The pins and metas | different pins, the same pattern (`TickInstruction`, `TestTickCycle`, the tick callbacks, input transitions) | a common base |
| [`C65C64ExecuteIfLastPointerWord`] and [`C65C64ExecuteIfLastTransferWord`]; read pointer word and read transfer word; read operand word and read vector word; the vector, pointer and transfer word address offsets | copies within the 65C64 | merging each pair |
| Read and write data low and high, read program counter low and high, execute and done if 8 or 16 bit, X and Y index, the NMI and IRQ vectors | copies within the W65 side | merging each pair, as [`CW65NoteFour`] was |

## Tests

TestMicroprocessorLib ([`TestMicroprocessorLib/TestMicroprocessorLib.cpp`]) runs every processor's tests.  Failed
asserts trap, so run it with `BASELIB_BREAK=0` in the environment
([BaseLib](../baselib/index.md#stopping-on-an-error)).

| Test | Covers |
|---|---|
| [`TestMicroprocessorLib/Test65C64.cpp`] and the other `Test65C64*.cpp` | the 65C64's instructions, interrupts, multiply and divide, subcopy, bits, operands, block and vector operations, through the encoders in [`TestMicroprocessorLib/Test65C64Context.h`] |
| [`TestMicroprocessorLib/TestFast65C64.cpp`] | the fuzz against [`C65C64`], interrupts, breakpoints, devices, unaligned data, watches |
| [`TestMicroprocessorLib/TestFastW65C816.cpp`] | the single step files, the fuzz and the interrupt lockstep against [`CW65C816`], breakpoints, watches, signals |
| `TestW65C816*.cpp` | the W65C816S: the datasheet tests, loads, adds and subtracts, reset |
| [`TestMicroprocessorLib/TestSingleStep.cpp`], [`TestMicroprocessorLib/TestW65C816SingleStep.cpp`], [`TestMicroprocessorLib/TestW65C02SingleStep.cpp`] | the SingleStepTests runner and its two processors (`CSingleStepProcessor`) |
| [`TestMicroprocessorLib/TestW65C02.cpp`] | the W65C02S: reset, IRQ, NMI, BRK and RTI, WAI with I set, SOB, SYNC |
| [`TestMicroprocessorLib/TestArithmetic.cpp`] | a W65C816S arithmetic program ([`Input/TestArithmetic.bin`]) |

## Decisions

- A 65C64 simulator in the style of the W65C816S's, classes prefixed for the processor, sharing no code with the
  W65 classes; 16 bit data bus, 32 bit address bus; an instruction executes in the cycle its last word is read (the
  user, 2026-10-22).  The prefix was C65 until 2026-11-13, when the classes became `C65C64*` and their files
  `65C64*`.
- The 65C64 uses the W65 meta and signal classes after all, and its copies were deleted (the user, 2026-10-23).
- The instruction behaviour where the spreadsheet is silent: the user's rulings, listed in
  [`MicroprocessorLib/65C64Simulator.txt`].  Where the sheet and the simulator disagree, the sheet is changed (the
  user, 2026-10-23).
- The hardware register instructions are not simulated (the user, 2026-10-22).
- An instruction level 65C64 in one `.cpp`, cross-checked against the cycle level one, for the assembler's and
  compiler's tests (the user, 2026-10-24).  Then the same for the W65C816S, as the first step of the W65C816
  compiler target (the user, 2026-10-25).
- The W65C02S shares the W65C816S's machinery through [`CW65CCore`] (2026-10-22), and every shared class has the
  W65 prefix (the user, 2026-10-22).
- The W65C816S matches the datasheet (the user, 2026-10-22), and where SingleStepTests disagree with the datasheet,
  SingleStepTests.
- NMIB is edge latched in both W65C816S simulators (the user, 2026-10-25).
- The duplication between the two sides is noted, not merged (the user, 2026-10-23).

## Not done

- 65C64 hardware interrupts 0 to 2 and the interrupt vector offset register, which need the hardware registers.
- The W65C816S simulators don't model ABORTB or RDY.
- No instruction level W65C02S.
- The duplication in the table above.
- The user hasn't ruled on the zero register as a find's index.

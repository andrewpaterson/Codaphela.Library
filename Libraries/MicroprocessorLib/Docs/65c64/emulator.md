# 65C64 Emulator

The 65C64 only exists in software.  Two simulators run its programs, and both are C++ classes
in MicroprocessorLib: there is no stand alone emulator program yet.

| Class | Simulates | Speed |
|---|---|---|
| [`C65C64`] | Every bus cycle: the pins, the address and data buses and the clock | The reference |
| [`CFast65C64`] | An instruction at a time | About 50 times faster |

## The cycle level simulator

[`C65C64`] is the reference.  It models the processor as the [bus](bus.md) sees it: each
instruction is a list of bus cycles, and each cycle puts an address and the control signals on
the pins on one clock edge and takes or gives data on the next.  [`C65C64Meta`] owns the pins as
traces, so a logic level system can be built around the processor, and calls the memory back on
each edge of the clock.

It follows the structure of the library's W65C816S simulator.

## The instruction level simulator

[`CFast65C64`] does what the reference does an instruction at a time, without pins, bus cycles
or a timeline.  It decodes with the same decoder, makes the same memory accesses in the same
order, counts the same clock cycles and takes interrupts at the same instruction boundaries.

One `Step` is one instruction.  A step of a [repeating instruction](instructions/block.md)
(block, vector, `mul` or `div`) is a `Step` too.  `Run` steps until the processor stops or a
count of instructions runs out.  It has breakpoints, and the registers and the cycle count can
be read at any time.

The assembler's tests, the C++ compiler's tests and the runtime library's tests all run on it.

## Keeping them the same

A change to an instruction is made in both simulators, and tests hold them together:

- A fuzz test runs every opcode from random state on both and compares everything after
  each step.
- Lockstep tests ([`CAsm65C64Lockstep`]) run whole programs on both side by side, comparing the
  registers after every step and memory every so often.

## Memory and devices

[`CAsm65Memory`] (Assembler65Lib) is a flat memory for either simulator:

| Address | What |
|---|---|
| `0000:0000`–`001F:FFFF` | 2MB of memory (the size can be set) |
| `00FF:0000` | A storage drive's registers |
| `00FF:0100` | A console: each byte written is appended to a string |
| `00FF:0104` | The console's input: write `00FF:0104` to take the next byte; `00FF:0105` is then 1 with the byte in `00FF:0106`, or 0 when the input has ended |
| `00FF:0300` | The size of the memory, read only |
| `FFFF:F000`–`FFFF:FFFF` | The 4KB [vector](interrupts.md#vectors) page |

An access anywhere else reads 0 and is counted as a bad access, which the tests check for.

The drive copies whole sectors between a file drive and memory.  A program writes the first
sector, the address in memory and the number of sectors to its registers, and then a command (1
to read, 2 to write).  The [lazy loader](linking.md#lazy-loading) uses it.

The console's input comes from a string or a callback (`SetConsoleInput`); with neither, the input
has ended.  Taking a byte is a write, so reading the registers (as a debugger does) changes nothing.
The X65C64 Emulator gives a program its standard input, showing what the program has written
first, so a prompt appears before it waits.  Libc65's [`stdin`] reads it on both processors.

## Running a program

Assemble the source, load the output into a memory, connect a simulator and run:

```cpp
CAsm65C64Assembler	cAssembler;
CAsm65Memory		cMemory;
CFast65C64		cMPU;

cAssembler.Init();
cAssembler.Assemble(szSource);

cMemory.Init();
cMemory.LoadProgram(cAssembler.GetOutput(), 0x2000);	//Also points the reset vector at $2000.
cMemory.InitFast65C64(&cMPU);
cAssembler.Kill();

cMPU.Reset();
cMPU.Run(1000000);		//Until an stp, or a million instructions.

cMPU.GetA();			//The registers: GetB, GetX, GetY, GetS, GetV, GetW, GetP.
cMPU.NumCycles();		//The clock cycles taken.
cMemory.GetLong(0x3000);	//And the memory.

cMPU.Kill();
cMemory.Kill();
```

`SetEdgeInterrupt` and `SetLevelInterrupt` drive the [interrupt](interrupts.md) pins.

## What isn't simulated

- The [hardware register](instructions/hardware.md) instructions, and with them hardware
  interrupts 0 to 2 and the interrupt vector offset.
- User mode: the processor is always in [kernel mode](interrupts.md#kernel-mode-and-user-mode).
- The on-chip SRAM, user bank remapping, the programmable IO and the CPLD, the clock control and
  the watchdog timer (see [Bus and Signals](bus.md) and the [Memory Map](memory-map.md)).

An opcode that isn't an instruction runs as a one cycle instruction that does nothing.

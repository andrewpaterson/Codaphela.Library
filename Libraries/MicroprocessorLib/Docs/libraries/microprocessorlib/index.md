# MicroprocessorLib

MicroprocessorLib simulates processors and the circuits around them.  It has cycle level simulators of the 65C64,
the W65C816S and the W65C02S, which drive the processors' pins on every clock edge; instruction level simulators of
the 65C64 and the W65C816S, which programs run on; and a port of Logicim's logic simulation, in which a processor
is one chip among gates, latches and buses on analog traces with real propagation times.  It depends on BaseLib and
StandardLib only: circuits are StandardLib objects.

Programs run on its instruction level simulators everywhere: the assembler's, the compiler's and Libc65's tests and
the [65 suites](../../compiler/design/test-suites.md) through Assembler65Lib's test memory ([`CAsm65Memory`]), the
X65C64 Emulator, the [HCP65 emulator](../../hcp65/emulator.md) ([`CHCP65`]) and the debugger's targets
(EmulatorDebugLib).  LogiCim's circuits are made of its chips.

| Page | Covers |
|---|---|
| [The Processor Simulators](simulators.md) | how the cycle and instruction level simulators are built, keeping them the same, the W65C816S's datasheet and single step tests, what the two sides share and duplicate |
| [Logic Simulation](logic.md) | the timeline, voltages and logic families, traces and ports, circuits and chips, processors as chips |
| [65C64 Emulator](../../65c64/emulator.md) | running a 65C64 program on the simulators (the user page) |

## Its parts

| Part | Main classes | Is |
|---|---|---|
| 65C64 | [`C65C64`], [`C65C64Meta`], [`Decode65C64`], [`C65C64InstructionFactory`] | the 65C64 bus cycle by bus cycle ([The Processor Simulators](simulators.md#the-65c64)) |
| W65C816S and W65C02S | [`CW65CCore`], [`CW65C816`], [`CW65C02`], [`CW65CMeta`], [`CW65C816Meta`], [`CW65C02Meta`] | the WDC processors, sharing one engine ([The Processor Simulators](simulators.md#the-w65c816s-and-the-w65c02s)) |
| Instruction level | [`CFast65C64`], [`CFastW65C816`], [`CBreakpoints`], [`CDataWatches`] | fast simulators with breakpoints and watches ([The Processor Simulators](simulators.md#the-instruction-level-simulators)) |
| Signals | [`CLogicSignal`], [`CLogicBus`], [`CMetaTrace`], [`CMetaBus16`], [`STraceValue`] | what processors' pins drive and read, ideal or analog |
| Timeline | [`CTimeline`], [`CEvent`], [`CSimultaneousEvents`] | events in time order ([Logic Simulation](logic.md#time-and-the-timeline)) |
| Voltages | [`CVoltageConfiguration`], [`CFamilyVoltageConfiguration`], [`CFamilyVoltageConfigurationStore`] | logic families' thresholds and timings ([Logic Simulation](logic.md#voltages-and-logic-families)) |
| Traces and ports | [`CTrace`], [`CPort`], [`CLogicPort`], [`CLogicPortBus`] | analog wires and the connections to them ([Logic Simulation](logic.md#traces-and-ports)) |
| Circuits | [`CCircuitSimulation`], [`CCircuit`], [`CCircuitComponent`], [`CIntegratedCircuit`], [`CPassive`], [`CSubcircuitInstance`] | circuits and their components ([Logic Simulation](logic.md#circuits)) |
| Chips | [`CAndGate`], [`CDTypeFlipFlop`], [`CCounter`], [`CGateArray`], [`CRegister`], [`CW65C816Chip`], [`C65C64Chip`] | Logicim's chips, generic 74 series parts and the processors ([Logic Simulation](logic.md#chips)) |

The tests are TestMicroprocessorLib ([`TestMicroprocessorLib/TestMicroprocessorLib.cpp`]).  The 65C64 simulators'
rulings on what instructions do are listed in [`MicroprocessorLib/65C64Simulator.txt`].

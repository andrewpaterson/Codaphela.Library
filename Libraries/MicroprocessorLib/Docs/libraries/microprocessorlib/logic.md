# Logic Simulation

MicroprocessorLib has a C++ port of Logicim's simulation domain (Logicim is the user's Java circuit simulator,
`net.logicim.domain`): a timeline of events, voltages and logic families, analog traces and the ports on them,
chips and the circuits they make.  The processors of [The Processor Simulators](simulators.md) can be chips in
these circuits, so a program runs with real propagation times.  It was ported on 2026-10-23 at the user's request,
class by class, and checked against every Java class and method.  It is the simulation only: drawing and editing
circuits belong to LogiCim.

Each class says in its header which Logicim class it is, and where it does something different and why.  This page
is how the parts fit and what was changed.

## Time and the timeline

Time is an [`int64`] of 1/1024ths of a nanosecond, as Logicim's `#!text LongTime`, so a clock's half periods add up
exactly ([`MicroprocessorLib/LongTime.h`]: [`NanosecondsToTime`], [`FrequencyToTime`], [`AppendNanoseconds`]).

A [`CEvent`] happens at a time.  [`CTimeline`] keeps the events at each time in a [`CSimultaneousEvents`], in the
order they were added, which is the order they run in, and keeps those in BaseLib's red-black tree keyed by time
(the user asked for the tree, 2026-10-23).  Running moves time on to the earliest events and executes them; an
executing event can add events, but only later ones.

The timeline owns what is added to it: an event is killed and freed once it has executed or been removed.  An owner
that keeps pointers to its events (a chip its pending events, a port its slews) drops them in
[`CEvent::RemoveFromOwner`], called on both.  If adding fails the caller still owns the event;
[`CTimeline::AddNewEvent`] and [`CTimeline::AddNewFutureEvent`] free it for the caller instead.  An event removed
while its own time's events are executing isn't executed and is freed after them.  Where Logicim throws (an event
in the past), the timeline logs an error and returns false.  Logicim's `#!text Simulation` is only the timeline's
context ([`CTimeline::GetContext`]), which is the [`CCircuitSimulation`].

## Voltages and logic families

A voltage is a [`float32`] in volts, NaN when nothing drives it ([`NoVoltage`], [`IsVoltage`]).

A [`CVoltageConfiguration`] is a logic family at one supply voltage: the input thresholds (low at or below, high at
or above, unsettled between), the output voltages, and the hold and slew times for each kind of change, an output
holding its old voltage for the hold time and then slewing over the slew time.  A [`CFamilyVoltageConfiguration`]
holds a family at each VCC it has data for and interpolates between them; below the lowest the voltages come from
zero, above the highest they scale with VCC, and with no VCC there is no power.  A part works from its family's
lowest VCC up ([`CVoltageConfigurationSource::IsPowered`]): a chip is powered while its GND is zero and its VCC is one
its family works at ([`CIntegratedCircuit::IsPowered`]); below it nothing it drives slews.  [`CFixedVoltageConfiguration`] is
one configuration at any VCC, for tests and simple circuits; both are a [`CVoltageConfigurationSource`].

[`CFamilyVoltageConfigurationStore`] loads the families from a CSV file, Logicim's
`Documents/Propagation Times (541).csv` (a copy is the test's [`TestMicroprocessorLib/Input/PropagationTimes.csv`]).  The caller owns the
store, where Logicim has a static one; the default family is LVC.

## Traces and ports

A [`CTrace`] is an analog wire.  Its voltage is the average of what the ports on it drive strongly, NaN when nothing
does.  A port drives weakly through a resistance ([`CPort::GetDrive`]): a pull ([`CPull`], its [`CPullPort`]) or an
open drain output's pull-up ([`CLogicPort::SetPullUp`]).  Weak drives count only while nothing drives the trace
strongly, and then share by conductance, as a divider does.
A [`CPort`] joins a [`CPortHolder`] (a chip's pins or a passive) to a trace:

| Port | Is |
|---|---|
| [`CPowerInPort`] | a chip's VCC or GND, reading its trace |
| [`CPowerOutPort`] | a power supply, driving its trace with its voltage, see Supplies below |
| [`CTracePort`] | a passive connection, a wire's or a splitter's end, that neither drives nor reads |
| [`CLogicPort`] | a chip's input or output |
| [`CPullPort`] | a pull's end on the trace it pulls, driving its supply's voltage weakly |

Writing a logic port schedules a [`CSlewEvent`] after the family's hold time.  When it executes the port slews its
trace to the new voltage and schedules a [`CDriveEvent`] at the end of the slew, and every other logic port on the
trace schedules a [`CTransitionEvent`] for when the voltage crosses its input threshold.  The transition tells the
chip that an input changed.  A port keeps a copy of what it is driving ([`CPortOutput`]), because the timeline
frees the event that set it.  [`CLogicPortBus`] is a logic port per bit, least significant first.

Driving and undriving are changes the ports must see:

- An undriven trace being driven takes the new level at once, and its other ports see a transition on the next tick
  ([`CSlewEvent::Update`]).  Logicim slews it up from the low output voltage, so driving low from undriven went
  unseen and driving high read low for a moment.
- [`CPort::ConnectLive`] wires a port while the simulation runs, as an editor does: the port sees the trace's
  level, and if it drives an undriven trace the trace's other ports see the new level too.  A trace that was
  already driven leaves them alone, so connecting makes no false clock edges.  Plain `Connect` is for building a
  circuit before it runs.
- [`CPort::Disconnect`] is the reverse: the port sees undriven, and if the trace is left undriven so do its other
  ports ([`CTrace::NotifyUndriven`]).  A drive of NaN, and a slew from a chip without power, notify the same way.
- A strong driver letting go of a pulled trace leaves it at its pulls' voltage at once, and its ports are told it moved
  ([`CTrace::NotifyVoltageChanged`]); a driver taking a pulled trace slews from the pulled level.  An input held only by
  supplies and pulls is read as its chip starts ([`CIntegratedCircuit::Reset`]): no slew will tell it.
- [`CLogicPort::WriteImpedance`] drops the port's pending slews and drives after it, so the last write wins;
  Logicim lets an earlier write's slew drive the trace again.
- A chip with no power slews forever (its slew time is the largest [`int64`]); such a slew drives nothing and its
  transition isn't worked out, where Logicim would never finish working it out.

## Supplies

A supply ([`CPowerSource`], its [`CPowerOutPort`]) is switched on and off ([`CPowerSource::Switch`]).  Switched on, it
rises from where it is to its voltage over its rise time, and switched off it falls to 0V the same way: a regulator
charging the rail's capacitors and the load discharging them.  It moves in steps of 0.1V (`#!text POWER_STEP_VOLTAGE`),
each a [`CPowerStepEvent`] that tells the ports on its trace ([`CPort::TraceVoltageChanged`]): a chip's power port tells the
chip, which starts, writes again at its new VCC or stops, and a logic port tied to the rail tells its chip when it reads
another level.  Its voltage at a time is worked out from when it was last switched and the voltage it had then.  With
no rise time it switches at once, the next tick, and when the simulation starts it is on at its voltage, as Logicim's
was.  With a rise time it starts at 0V and rises, or stays off if it isn't on at start.

A supply fed by another, a regulator ([`CPowerSource::SetFeed`]), has a power in port on its feed's rail: it is on
while its feed is at its voltage plus its dropout or above, and off below it.

A component still coming up from power on, a supply ramping or a supervisor holding its power-on reset, says so
([`CCircuitComponent::IsPoweringUp`]), so an application can run the simulation flat out until nothing is
([`CCircuitSimulation::IsPoweringUp`]).

## Ideal and analog signals

The processors' pins don't hold traces: they hold a [`CLogicSignal`] for each signal and a [`CLogicBus`] for each
bus.  [`CMetaTrace`] and [`CMetaBus8`], [`CMetaBus16`] and [`CMetaBus32`] are ideal ones (a write is seen at once,
the timeline is ignored); [`CLogicPort`] and [`CLogicPortBus`] are analog.  So a processor runs the same code under
a meta processor and as a chip.  Pins that may be unconnected use the `#!text SafeWriteSignal` and
`#!text SafeReadSignal` macros ([`MicroprocessorLib/LogicSignal.h`]): an unconnected pin reads undriven.

## Circuits

A [`CCircuitSimulation`] is a circuit and the timeline it runs on, Logicim's `#!text CircuitSimulation` with its
`#!text Simulation` folded in.  It owns the [`CCircuit`], the [`CTimeline`], its [`CSubcircuitTopSimulation`] and a
[`CSubcircuitInstanceSimulation`] for each placed subcircuit.  [`CSubcircuitSimulations`] lists each circuit
simulation's subcircuit simulations without owning them.

The circuit holds its components and owns its traces.  A component ([`CCircuitComponent`]) is made in a
[`CSubcircuitSimulation`], through which it reaches the circuit and timeline:

- a chip, [`CIntegratedCircuit`], with its [`CPins`] (always VCC and GND, then its logic ports);
- a passive, [`CPassive`], which holds its own ports and does nothing while the simulation runs: [`CPowerSource`],
  [`CPull`] (a pull-up or pull-down), [`CPin`] (a circuit's input or output, a trace port per bit), [`CSplitter`] and
  [`CSubcircuitInstance`] (a subcircuit placed in a circuit, a list of trace ports per pin).

Circuits, components and pins are StandardLib objects, made with [`OMalloc`] and kept by [`Ptr`]s: the circuit
simulation is kept under the root, the circuit by the simulation, the components by the circuit and the pins by
their chip.  Components are freed in batches in no particular order, so a component's `Free` only lets go of what
it holds itself; freeing a chip takes its events off the timeline and freeing its pins disconnects their ports.
The circuit is therefore freed before its simulation, which needs the timeline to outlive it.  To take a component
out of a running simulation, disconnect it and its ports ([`CCircuitComponent::DisconnectPorts`]) first, so the
ports on its traces see them go, then remove it from the circuit.

## Chips

A chip responds to its inputs in `InputTransition`, schedules [`CTickEvent`]s for itself if it is clocked, and
keeps what it remembers in its own fields (Logicim keeps that in a separate state object).  A chip whose outputs
come from what it remembers drives them from it as soon as it starts, as a real part does from power up
([`CIntegratedCircuit::WriteOutputs`]); Logicim's only drive them when what they remember changes, so a flip-flop
fed back to itself never started.

A chip starts when its power comes and stops when it goes ([`CIntegratedCircuit::PowerChanged`]), however that
happens.  When the simulation starts only the powered chips start (the circuit resets its passives, the supplies among
them, first).  A VCC or GND wire connected or cut while it runs, or a supply switched or ramping, starts a chip (it
resets, then reads the inputs already at a level, [`CLogicPort::ReadTraceLevel`]) or stops it (its events dropped and
its outputs undriven).  When only its VCC moves, its ports write what they were told to write again at the new VCC and
read their traces at its new thresholds ([`CLogicPort::VccChanged`]).  An instrument, the oscilloscope, needs no power
([`CIntegratedCircuit::NeedsPower`]).  Logicim's power was there from the moment a supply was connected and no chip was
told it came or went: only its clock looked.

What a chip remembers as its power comes (a flip-flop's bit, a counter's count, a register) is the simulation's
power-up state ([`EPowerUpState`], [`CCircuitSimulation::SetPowerUpState`]): Zero, every bit low, as Logicim's (the
default); Random, each bit from the simulation's seed ([`CCircuitSimulation::SetPowerUpSeed`]), drawn again from it
each time the simulation starts so a run repeats; or HalfSupply, not known, the chip's outputs at VCC/2
([`CLogicPort::WriteHalf`]), which reads as neither high nor low, until it is set, reset, loaded or clocked.  An input
reads VCC/2 as unsettled, so a flip-flop fed back to itself never starts with HalfSupply.  A chip that remembers something saves it as XML with a saved simulation
([`CIntegratedCircuit::SaveXML`]).

Inputs that nothing drives are unknown.  A gate whose output an unknown input could decide stops driving, and the
unknown spreads; an input that already decides the output (a low into an AND or NAND, a high into an OR or NOR)
still drives it, and unsettled inputs, slewing between levels, are ignored ([`CAndLogic`]).  Logicim ignores
undriven inputs and keeps the output.

| Chips | Are |
|---|---|
| [`CAndGate`], [`CNandGate`], [`COrGate`], [`CNorGate`], [`CXorGate`], [`CXnorGate`], [`CBuffer`], [`CInverter`] | Logicim's gates |
| [`CConstant`], [`CProbe`], [`CClockOscillator`], [`COscilloscope`] | a fixed value, inputs read from outside the simulation, a clock, sampled traces |
| [`CDTypeFlipFlop`], [`CCounter`] | Logicim's flip-flop and counter (a 74161, optionally with its synchronous load) |
| [`CGateArray`], [`CBusBuffer`], [`CBusTransceiver`], [`CRegister`], [`CFlipFlopArray`], [`CDecoder`], [`CMultiplexer`], [`CAdder`], [`CComparator`] | generic 74 series parts, groups of bits whose pins are a [`CGroupPins`] (a 74LVC16373 is two groups of an 8 bit latch) |
| [`CResetSupervisor`] | a power-on reset supervisor, a DS1813: reset low while VCC is under its trip voltage and for its reset time after, or after a button pulls it low; its open drain reset has its own levels and pull-up ([`CResetSupervisorPins`]) |
| [`CW65C816Chip`], [`C65C64Chip`] | the processors |

### Processors as chips

[`CW65C816Chip`] and [`C65C64Chip`] own the processor and the pins it drives ([`CW65C816Pins`], [`C65C64Pins`]),
wired to the chip's [`CLogicPort`]s and [`CLogicPortBus`]es, and pass every input transition to the processor,
which acts on PHI2's edges as it does under a meta processor.  The chip's pin names are the simulator's signals,
not the 65C64 spreadsheet's pinout.  On analog traces the processors must let go of the data bus, which ideal
traces never needed ([`CW65CPins::ReleaseData`], [`C65C64Pins::ReleaseData`]): the W65C816S when PHI2 rises in a
read cycle, as the bank address was on the data pins while PHI2 was low, and the 65C64 when PHI2 falls after a
write.

## LogiCim

LogiCim (a separate repository) is the C++ version of Logicim's application.  Its LogicimLib draws and edits
circuits on SupportLib's geometry maps and makes these classes from its views: each view makes its component in a
circuit simulation, a whole 74 series part is simulated as one generic chip, and a running simulation's state (the
timeline's times, each chip's state and its pending events) is saved and loaded as XML.  Interface code belongs
there, not here: MicroprocessorLib depends only on BaseLib and StandardLib.

## Tests

| Test | Covers |
|---|---|
| [`TestMicroprocessorLib/TestTimeline.cpp`] | adding, ordering, removing during a run, ownership, a thousand event stress with the tree checked |
| [`TestMicroprocessorLib/TestVoltageConfiguration.cpp`] | configurations, families and interpolation, loading the CSV |
| [`TestMicroprocessorLib/TestLogicPort.cpp`] | slews, drives and transitions with times worked out by hand |
| [`TestMicroprocessorLib/TestPassive.cpp`] | power sources, pins, splitters, subcircuit instances and simulations |
| [`TestMicroprocessorLib/TestStandardIntegratedCircuits.cpp`] | truth tables, gates made of NANDs, a 4 bit ripple adder over all 256 sums, latches, flip-flops and counters on LVC at 3.3V, live wiring |
| [`TestMicroprocessorLib/TestGroupIntegratedCircuits.cpp`] | the generic 74 series parts |
| [`TestMicroprocessorLib/TestProcessorChips.cpp`] | the same program on a meta processor and on a chip with a test RAM: registers and memory must match (the W65C816S's fails without releasing the data bus) |

The test circuits are built on a board in [`TestMicroprocessorLib/TestCircuitBoard.h`].

## Decisions

- Port Logicim's timeline, events and simultaneous events into MicroprocessorLib, the events kept in BaseLib's
  red-black tree (the user, 2026-10-23).
- The voltage model, then a separate analog trace and port layer, with the processors' pins able to sit on either
  ideal traces or analog ports (the user's choice, 2026-10-23).  The signal classes lost their W65 prefix then:
  they belong to no processor (the user, 2026-10-23).
- A logic level (TTL or CMOS) in the propagation times file applies to its own VCC only; Logicim carried it to the
  family's later VCCs, so LVT's TTL configuration became 3.3V (2026-10-23).
- An undriven trace taking a level, low or high, is a transition (the user: "Treat undriven to low (and maybe high)
  as a transition", 2026-10-23).
- Connecting a port in a running circuit tells the ports what changed, not a firing of every connection event (the
  user, 2026-10-23, replacing `#!text FireConnectionEvents`); a driver removed so its trace goes undriven is seen
  the same way (the user asked for a solution, 2026-10-23).
- Power comes and goes: a chip starts and stops with it, and works only from its family's lowest VCC (the user: "the
  way power works in Logicim is a bit weird / broken.  Power is instantaneously applied on connection", 2026-11-26;
  `PowerPlan.txt`).
- What a chip remembers at power up is a setting of the simulation: Random from a seed, Zero, or Half supply (the
  user, 2026-11-26).
- Undriven inputs are unknown only: there is no per-family rule that a floating TTL input reads high (the user,
  2026-10-23).
- Things returned by pointer are allocated one by one ([`CArrayTemplatePtr`] and [`NewMalloc`]), never pointers
  into a by-value array, which move on the next add: a test found such a pointer dangling (2026-10-23).

## Not done

- Logicim's user interface parts (voltage colours, oscilloscope colours, the bidirectional port map, the editor's
  ports and traces, checking subcircuit simulations against the editor's views), which are LogiCim's if anywhere.
- The circuit simulation isn't saved as an object ([`CCircuitSimulation::Save`] returns false): LogiCim saves its
  state as XML.
- A W65C02S chip.

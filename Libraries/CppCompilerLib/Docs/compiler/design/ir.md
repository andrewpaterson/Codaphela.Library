# IR and Optimiser Design

How the compiler's intermediate representation, its front end and its optimiser are built, and why.  [The IR](../ir.md)
says what the IR is (types, instructions, text form, the passes in a line each) and
[Inside the Compiler](../inside.md) where it sits among the stages; this page is for changing them.  The back ends
that read the IR have their own pages: [65C64 back end](65c64.md) and [W65C816 back end](w65c816.md); the C++ front
end that drives the IR front end is [Front end](front-end.md).

The plans this page replaces, [`CppCompilerLib/IR.txt`] and [`CppCompilerLib/OptimiserPlan.txt`], are finished;
they are kept only as the record code and other plans cite by step.

## Why there is an IR

Until 2026-10-24 [`CCPPUnit`] walked the syntax tree and wrote 65C64 assembly as it went: every value ended in A,
every local had its own place in the frame and nothing saw further than one expression.  The user asked for an IR
so the compiler could optimise.  The generator kept all its C++ work (overloads, conversions, templates,
temporaries, destructors, `this` adjustments, class layout, labels) and the IR front end took over only the code
inside functions, one kind of C++ at a time, with every suite passing at each step: a function the IR front end
couldn't write went to the old generator.

The measure that justified it, the map benchmark (TestMapStringIntAndPointer on the 65C64, steps): 2.33B with the
direct generator, 1.32B after the first register allocation and inlining, 0.917B after the second round of passes,
0.898B with the text passes (2026-10-25).  The direct generator has since been removed: a function the front end
can't write is now an error that names the front end's own file and line ([`CCPP65IRFrontEnd::SetUnsupported`]),
which says which construct to support next.

## The IR's choices

The user's rulings of 2026-10-24 were: a variant of QBE, in this library; phi instructions; signed overflow wraps;
an LLVM like text; and "the IR will be a black box to me", so the rest are choices made while building it.

| Choice | Why |
|---|---|
| Fixed size instructions ([`SIRInstruction`]: an opcode, flags, a type, a result, two operands, a source) in an array per block, as QBE's | Cheap to copy, scan and insert; no allocation per instruction. |
| Operands are [`SIRReference`]s (a kind and an index into the function's temporaries or constants, or the module's symbols), not pointers | A block's array can grow or be rebuilt without breaking anything that names a value. |
| Phis kept apart from instructions, at a block's start; each block ends in an [`SIRJump`] that is not an instruction | Passes walk instructions without stepping over phis or jumps. |
| Calls are an `arg` per argument straight before the `call` | Keeps the instruction fixed size; the verifier holds args to that place. |
| SSA, made by promoting slots afterwards ([`CIRPromotionPasses`]), not while writing | The front end writes every C++ lvalue as an address, which keeps it simple; promotion also catches slots that stop escaping after inlining.  (Building SSA while writing, Braun et al. 2013, was the first plan.  [`CIRSSAUpdater`] uses Braun's method for passes that give a value several definitions.) |
| `i8` and `i16` are values, not only memory types | The W65C816 works in 16 bits and QBE only has 32 and 64 bit values. |
| `i1` only for conditions, never in memory or an argument | A comparison can be joined with the branch that reads it, and a W65C816 comparison stays out of the accumulator. |
| No aggregate types | The front end has already laid out classes and does the calling convention's work. |
| Floats stay instructions until the back end makes them soft float calls | Folding, value numbering and dead code see them; as calls they would be opaque. |
| Signed overflow wraps (the user) | No undefined behaviour to reason about in the passes.  Constant expressions are the exception: the front end marks their arithmetic (`IRF_NoSignedWrap`) so [`CIRInterpreter`] can refuse an overflow, as C++ requires. |
| No alignment on loads and stores | Neither processor faults on an unaligned access. |
| Address spaces in the type (`ptr.space`), `spacecast`, and `inbank` on `ptradd` | Kept room for near and far W65C816 pointers.  Neither target uses more than the default space: the front end declares one space of the target's pointer size ([`CIRModule::AddDefaultSpace`]), and the W65C816 went to 4 byte pointers everywhere. |
| An exception is a flag, not a table: a function is left by `ret` or `throw`, and `thrown` straight after a call reads which | No unwinder and no tables; a program without `throw` compiles to the same bytes.  Both back ends carry the flag in the carry. |
| Symbol attributes as one bit set ([`CIRModule::SetSymbolAttributes`]): restrict, local, regcall, nowrites, nothrow | One parse, one print, one copy rule.  Restrict, regcall and nothrow are carried when a function is copied into another module (`IRSA_Carried`); local must not be, and nowrites is not (see Not done). |
| Debug value markers ([`SIRDebugValue`]) kept beside a block's instructions, not among them | No pass counts or uses them, so the code is the same with and without a debugger's information. |
| The text prints and parses back exactly ([`CIRPrinter`], [`CIRParser`], floats as their bits) | A pass is tested with IR text in and the IR text expected out. |

The opcode table ([`CppCompilerLib/IROpcode.h`]) drives the printer, parser and verifier, and holds what passes ask
of an opcode: pure, commutative, may trap, signed or unsigned, its unsigned twin, a division's partner.  A pass
asks the table ([`IRDivisionPartner`], [`IRUnsignedOpcode`] ...) rather than listing opcodes itself.

The first design had two levels in the same structures, the second being machine IR after instruction selection
(target opcodes from [`IRO_NumGeneric`], [`CIRModule::SetTargetOpcodes`]), then legalisation and linear scan
allocation.  That was not how the back ends were built: each lowers the IR straight to assembly text and works on
the text ([`CIR65Code`], [`CIRW65C816Code`]).  Target opcodes are still in the table and tested, and nothing
else uses them.

## The IR front end

[`CCPP65IRFrontEnd`] writes a function body as IR after [`CCPPUnit`] has set the function up.  It is one front end
for both processors; the target shows through in a few places only:

- Every local is a `slot` and every parameter is read through its `argaddr`, where the caller pushed it, as clang
  writes allocas.  Promotion makes them temporaries.
- A result narrower than 4 bytes is an `i32` on a target that returns results widened ([`CCPPTarget::WidensResults`],
  the 65C64: the caller gets A extended by the C++ type's signedness), so the front end extends before `ret` and
  truncates after `call` ([`CCPP65IRFrontEnd::ReturnIRType`]).
- A member function's call has `thiscall` (the 65C64 passes `this` in X).
- An object used as a value is its address.  A function returning one takes a hidden pointer and returns it; the
  caller passes a temporary, or the object being initialised, which is made in place.
- Objects to destroy (a scope's locals, a full expression's temporaries, and the cleanups on the way out with an
  exception) are a stack the front end keeps while writing ([`CCPP65IRCleanups`]).
- Constant expressions are written as IR by a front end that generates no code, and run by [`CIRInterpreter`]
  ([`CCPPConstants`]).

Each statement sets the source its instructions are stamped with (below), and the IR is verified
([`CIRVerifier`]) after the front end and the optimiser are done with a function and again after inlining.  IR that
doesn't verify is a mistake of the front end or a pass, and the function is an error (or, after inlining, keeps the
code it had).

### A unit's way through

1. Each function is written, optimised ([`CIROptimiser::Optimise`]), verified and given to the back end.  Its code
   waits in the unit's spill store, with a marker where it goes in the unit's assembly.
2. When the generator has written every function, [`CCPP65IRFrontEnd::InlineUnit`] runs the interprocedural passes
   over the unit, then inlines into each function, the last written first (callees are written after their
   callers, so they are inlined into first).  A function that inlined anything is optimised and inlined into again,
   up to `IR65_INLINE_ROUNDS` (4) rounds, as optimising can make an indirect call direct.  One that changed is given
   to the back end again; one that no longer verifies keeps the code it had.
3. In the unit's turn, [`CCPP65IRFrontEnd::FinishUnit`] exports its small functions to the other units, lets the back
   end finish each function now the unit's calls are known ([`CIRBackend::FinishFunction`]), and turns the source
   markers into comments or `.line` directives.

## The optimiser

[`CIROptimiser`] runs a pipeline of passes over one function.  The passes are in groups, a class each, with a
[`CIRPassContext`] they share: the function, the target's settings, and the analyses.

| Group | Passes |
|---|---|
| [`CIRBlockPasses`] | Flow graph clean up, trivial phis, dead code, comparisons moved to their branch. |
| [`CIRMemoryPasses`] | Slot types unified, slots split, redundant loads, constant data. |
| [`CIRPromotionPasses`] | Slots to SSA. |
| [`CIRFoldingPasses`] | Constant folding and identities, common subexpressions. |
| [`CIRRangePasses`] | What a known range lets go: null tests, narrow arithmetic, narrow multiplies. |
| [`CIRDivisionPasses`] | Every way of not dividing. |
| [`CIRLoopPasses`] | Rotation, invariants, unswitching, exit values, dead loops. |
| [`CIRCounterPasses`] | Induction variables: strength reduction, narrowing, linear test replacement. |
| [`CIRAddressPasses`] | Address shapes for the W65C816's index registers. |
| [`CIRIdiomPasses`] | Loops that are [`memset`], [`memmove`], [`strlen`], [`memcmp`], [`memchr`] or [`strchr`]. |

`IR_PASSES` in [`CppCompilerLib/IRPipeline.h`] has a row per pass (name, group, one line of what it does), and
everything that lists passes is made from it: the [`EIRPass`] enum, the names, the optimiser's [`CIRPass`] for
each.  [`CIRPipeline::SetStandard`] is the one place that gives the order:

- The round passes run in order, over and over, until a round changes nothing or after `IR_OPTIMISE_ROUNDS` (16).
- Then the after passes run once, each followed by its following passes only if it changed something:
  NarrowMultiplies (then FoldConstants and RemoveDeadCode), and BiasSignedIndices (then RemoveCommonSubexpressions,
  HoistInvariants, which moves the biased base out of the loop, and RemoveDeadCode).

A target builds its pipeline ([`CCPPTarget::BuildPipeline`]) and tells the optimiser what its passes need to know
([`CCPPTarget::ConfigureOptimiser`]):

| | W65C816 | 65C64 |
|---|---|---|
| Passes | All of them. | Not DivisibilityTests, ConstantDivisions, ReduceDivisions, MergeAddressPhis, NarrowCounters, NarrowMultiplies, BiasSignedIndices: it has a divide, 32 bit registers and indexed addresses. |
| Indexed addresses ([`CIROptimiser::SetIndexedAddresses`]) | No. | Yes: a load adds a pointer and an index for nothing. |
| Divides that leave a remainder ([`CIROptimiser::SetDivideRemainders`]) | 16, 32 and 64 bit, and signed (at D + 5). | 8, 16 and 32 bit (in B). |
| Division costs ([`CIROptimiser::SetDivisionCosts`]) | Cycles, from the runtime's routines ([`CppCompilerLib/IRW65C816Costs.h`]). | None: no reciprocals. |
| Profile | Yes, when the build has one ([`CCPPTarget::UsesProfile`]). | No. |

The front end also gives the optimiser the target's `int` and [`size_t`] ([`CIROptimiser::SetLibraryInt`],
[`CIROptimiser::SetLibrarySize`]), which the idioms call the library with, and a place for warnings about
comparisons a pass found always true or false.

Every search has a limit (`IROptimiserInternal.h`: how far back, how many values, how big a loop), so a large
function stays quick to optimise.  Each pass's runs, changes and time go into the unit's
[`CCPPCompileStatistics`], and `Tests/CompilerMeasure.py --compile` tabulates them: a pass that never changes
anything shows.

### Analyses

An analysis is worked out when a pass asks for it and kept until a pass changes the function (the context's fresh
flags; a pass says it changed the function by what it returns).

| Analysis | What it gives |
|---|---|
| [`CIRDominators`] | The dominator tree (Cooper, Harvey and Kennedy), numbered so dominance is constant time; children; dominance frontiers. |
| [`CIRDefinitions`] | Each temporary's defining instruction or phi, its block, and its number of uses: the one use count. |
| [`CIRLoops`] | Natural loops, outer before inner: head, blocks, latches, the entering block and whether it is a preheader, exits, depths; makes preheaders ([`CIRLoops::Preheader`]). |
| [`CIRRanges`] | How big a value can be ([`SIRValueRange`], narrowed by branches or dominators as the question says). |
| [`CIRMostRules`] | The unsigned largest-value rules, one table shared with the 65C64 back end's [`CIRMost`]. |
| [`CIRAddresses`] | What an address is of: root and offset, aliasing, own frame. |

What several passes need is written once: [`SIREdgeCondition`] (what a branch says on one way out, with a chosen
value on the left), [`CIRPasses::MatchCounter`] (a loop counter: a phi of the head, a start from outside, itself
plus a constant from inside), [`CIRPasses::SimpleLoop`], [`CIRPasses::AddCounterPhi`], [`SIRConstantDivision`] (is
this x / c or x % c), [`CIRBlockCopier`] (the inliner, loop unswitching, loop rotation and
[`CIRFunction::CopyBody`] all copy blocks through it).  That consolidation was `OptimiserPlan.txt` (2026-11-21 and
22, SmellCheckFindings 60 to 74 and 97): every stage but the last left all 4,201 listings of the three *65 suites'
projects on both processors byte for byte the same.

### Passes worth knowing about

The rows in `IR_PASSES` and the comments above each pass's function say what each does.  Some design behind them:

- **Slots.**  UnifySlotTypes makes a slot also copied as an integer a pointer slot (a struct returned through
  memory), so SplitSlots can make an object's fields a slot each, and PromoteSlots then makes them temporaries.
- **Loads.**  RemoveRedundantLoads knows what is in memory along single predecessors and where all predecessors
  agree; different globals and different slots don't alias, and a call or store that may touch it ends what is
  known.  Restrict pointers ([`CIRModule::HasRestrict`]) let more stay known.  A load from a constant global or
  from known data ([`CIRModule::AddKnownData`]) is its value ([`CIRMemoryPasses::ConstantData`]).
- **Loops.**  RotateLoops copies a short test (`IR_ROTATE_LIMIT`, 8 instructions) to the preheader and the latch,
  so each time round is one branch.  LessLoopTests turns `a <= b` into `a < b + 1` where that can't wrap, as the
  65C64 branches on less in one instruction.  ReduceStrength turns `p + i * size` into a stepped pointer; it takes
  a counter of the target's `int` sign extended in the index ([`CIRCounterPasses::ExtendedCounter`]), and a zero
  extended `i16` counter only when [`CIRCounterPasses::CounterUnwrapped`] proves it can't wrap.  NarrowCounters
  (W65C816) counts a 32 bit counter in 16 bits when its start and bound fit.
- **Divisions.**  BoundDivisions makes a divide of a value known to be below c (or 2c) a compare and subtract.
  ConstantDivisions (W65C816) makes `x / c` a multiply by a reciprocal and a shift, checking every x of the known
  range (up to `IR_DIVISION_RANGE` values) so the reciprocal is exact, and only where the target's
  [`CIRDivisionCosts`] say the multiply costs fewer cycles than its divide routine
  ([`CIRDivisionPasses::IsReciprocalCheaper`]); `x / c` and `x % c` of one x are priced together.  With a profile,
  a hot byte divide is marked `hot` (`IRF_Hot`) and the W65C816 back end reads a table instead.  ShareDivisions
  gives `x / d` and `x % d` one divide where the target's divide leaves the remainder.
- **Addresses (W65C816).**  BiasSignedIndices makes `p + sext(x)` of an `i16` x `(p - 32768) + zext(x ^ 32768)`,
  so the back end can put the index in Y; MergeAddressPhis makes a phi of one base plus different indices the base
  plus a phi of the indices.
- **Idioms.**  A loop of the right shape is a library call; the loops of the library's own functions, and known
  counts of 16 bytes or fewer (`IR_IDIOM_MIN_BYTES`), stay loops.  The idioms are a table ([`EIRIdiom`]) with a
  32 bit count form for a target whose [`size_t`] is 16 bits.

## Inlining

[`CIRInliner`] replaces a call by a copy of the callee's blocks: the call's block is split, the arguments become
the parameters, returns jump to the rest and the result is a phi there.  It decides by instruction count, with
limits the target sets ([`CCPPTarget::GetInlineLimits`], [`SIRInlineLimits`]):

| Limit | 65C64 | W65C816 |
|---|---|---|
| Callee instructions | 24 | 64 |
| ... where the call is in a loop | 48 | 128 |
| ... where the profile says the call is hot | 96 | 192 |
| Calls inlined into one function at a time | 32 | 64 |
| Preferring size | 4 | 4 |

The 65C64's limits were measured on TestBaseLib65 with units inlining each other's functions (2026-11-13): 16.8%
fewer cycles for 18.0% more code, against 14.7% for 8.2% at 16/32/64/24 and 19.1% for 42.9% at the W65C816's
limits; past these each percent of size bought less than a seventh of a percent of cycles.

Beyond the count:

- Each constant argument whose parameter the callee compares or branches on adds `IR_INLINE_CONSTANT_BONUS` (8),
  as those fold.
- All copies of a callee into one caller must still fit the limit (a function called from many places in one
  function stays a call).  No function grows past `IR_INLINE_MAX_CALLER` (4096) instructions: a W65C816 function
  must fit in a bank.
- A function that calls itself is inlined once into a caller; variadic functions are not inlined.
- A callee that takes its parameters' addresses (an object passed by value) gets a slot in the caller laid out as
  its parameters, with the arguments stored to it.
- A callee that may throw is inlined where its call is tested: its throws jump straight to the caller's landing
  code.
- On the W65C816 a function that only passes a call on is a jump ([`CIRInliner::SetForwardersJump`]), so it only
  takes a callee no bigger than itself unless the call is hot or passes a constant the callee compares: inlining
  more would copy the layers' shared work into each.

**Indirect calls become direct.**  The front end describes each vtable it writes as known data
([`CCPP65IRFrontEnd::DescribeVtable`]) and writes implicit default constructors and destructors as IR, so a virtual
call on an object whose vtable store the optimiser sees loads a constant: the call is direct and may be inlined in
the next round.  Calls through function pointers held in objects stay indirect.

**Interprocedural passes** ([`CIRInterprocedural`]) run over the unit before inlining, when optimising and inlining
are on: [`CIRInterprocedural::SpecialiseConstantArguments`] gives a function only the unit calls a copy per constant
its parameter gets (at most 3 values, 3 rounds); [`CIRInterprocedural::RemoveDeadArguments`] removes parameters
never read; [`CIRInterprocedural::MarkNoWrites`] and, in a build with exceptions,
[`CIRInterprocedural::MarkNoThrow`] mark functions optimistically and take marks back until nothing changes
([`CIRInterprocedural::MarkOptimistically`]).  A function calling a nowrites function keeps what it knows of memory
across the call; one testing a call of a nothrow function loses the test.  They reach the unit only through
[`CIRUnitFunctions`], so they know nothing of C++ or the generated text.

**Across units** ([`CIRImports`]): each unit exports copies of its small functions (up to `IR_IMPORT_INSTRUCTIONS`,
32) with external linkage that name nothing local to it, and other units inline them, mapping symbols by name.
Units compiled at once take turns in the order they would be compiled one after another, so the result doesn't
depend on threads: a unit may look before its turn, and in its turn checks that nothing it missed was exported
since ([`CIRImports::Missed`]); if something was, it is compiled again.  A unit that arrives early is parked rather
than holding a thread.  Both targets inline across units ([`CCPPTarget::InlinesAcrossUnits`]).

## Source lines and listings

Every instruction and jump carries `uiSource`: the C++ token it was written for plus one, 0 for none.  The front end
sets [`CIRModule::muiSource`] for each statement (restoring it after a nested one), and the function's first token
before its body; it is 0 while the optimiser and inliner run, so most instructions they add have none.  The
exception is [`CIRPasses::InsertInstruction`], which gives a new instruction the source of the one it goes before:
that keeps a profile's line counts meaning the same in the next build.  Inlined instructions carry an inline
location instead (`IR_INLINE_LOCATION` set: an index into the function's [`SIRInlineLocation`]s, the inline and
the callee's source), so a debugger and the profile can tell the call from the callee.

The back ends write a marker line (`IR_SOURCE_MARKER`, `;@` then the location) where the source changes, only when
the unit is listed or has a line table ([`CIRBackend::SetSourceMarkers`]).  Their text passes keep the markers as
fields of the code's lines ([`CIRLineMarkers`]): a line taken out gives its markers to the next.  Until 2026-11-25
they were taken out before the passes and matched back afterwards by a line diff; as fields they can't be put back
on the wrong line.  [`CCPP65IRFrontEnd::SourceComments`] then makes each change of line a comment with the file,
line and text (a listing), a `.line` directive (the line table), or nothing.  A macro's tokens count as where the
outermost macro was used.  The listing's code is the code without a listing, byte for byte: that is how
OptimiserPlan's "same code" stages were proved.  [`CIRPrinter::SetSources`] shows the same lines in IR dumps.

To get one: [`CCPP65Builder::SetListingDirectory`] (a `.lst` per unit with code), `hcp65build -listings`, or
`TEST_PROJECT65_LISTINGS=dir` for the *65 suites; see [Listings, Profiling and Testing](../testing.md).

## Profile-guided optimisation

The profile is taken outside the program: [`CAsm65Profile`] runs a linked program on an emulated processor
([`CAsm65ProfiledCore`]) and counts each function's calls and each source line's entries and cycles through the
line table ([`CAsm65LineTable`]; the `.line` directives).  Nothing is added to the program, so a profiled build is
the build.  The user confirmed (2026-10-30) that emulator-level profiling is what they wanted.

Each function is fingerprinted: a hash of its IR as written, before optimising ([`CCPP65IRFrontEnd::Fingerprint`]),
written with the compile time as `.fingerprint`.  The profile keeps them, and [`CCPP65IRFrontEnd::CallCount`] only
believes a caller's line counts when the caller's fingerprint is the same; otherwise it uses the line's count over
all functions.  A call the profiled build had inlined left its line no code, so its count is the most entries of
any of the callee's entry block's lines in the caller ([`CCPP65IRFrontEnd::InlinedEntries`]); with that and
[`CIRPasses::InsertInstruction`]'s source, a build from a profile of a profiled build keeps its decisions
(TestNumber on the W65C816 over three generations: 59.00, 59.30 and 58.93G cycles, 2026-10-30).

What uses it, on the W65C816 only ([`CCPPTarget::UsesProfile`]; `TEST_PROJECT65_FRAME_PROFILE` in the suites):

- The inliner ([`CIRInliner::SetProfile`]): hottest calls first, and a hot call (its count times
  `IR_INLINE_HOT_SAVING`, 30 cycles, at least 1/`IR_INLINE_HOT_SHARE`, 1/2000, of the run) may inline up to the hot
  limit.  FatBench on the W65C816 490.0M to 484.4M cycles (-1.15%, 2026-10-30).
- ConstantDivisions' byte divide tables ([`CIRPasses::IsHot`]).  TestNumber on the W65C816 68.04G without a
  profile, 58.72G with (2026-10-30, with the other CNumber work).
- The W65C816 back end's page aligned frames: see [W65C816 back end](w65c816.md).

## Decisions

- **An IR, QBE-like, in this library, with phis, wrapping signed overflow and LLVM like text** (the user,
  2026-10-24).  The rest was left to Claude ("the IR will be a black box to me").
- **Floats stay IR instructions until the back end** (the user's "pick whichever will optimise best").
- **Callee saved pseudo registers** (the user, 2026-10-24): a 65C64 calling convention matter, see
  [65C64 back end](65c64.md).
- **Move to the IR one kind of C++ at a time, with every suite passing** (2026-10-24); the direct generator was
  removed once everything went through the IR.
- **Optimiser order, as the user set it for the first rounds** (2026-10-25): measure, then inlining and cheaper
  calls, then loops.
- **Specialised copies, not cloning on demand**: interprocedural constant propagation copies a function only the
  unit calls, for at most 3 values a parameter.
- **Inline limits per target, by measurement** (2026-11-13, numbers above).
- **A "very hot" inlining tier above the hot one** was tried and removed (2026-10-30): worse or no effect.
- **PGO: speed first, the W65C816 first, the FAT code to train on; a profile's staleness is told by fingerprints
  and compile time; mapping code to IR is Claude's call** (the user, 2026-10-30).
- **Pointer aliasing from a profile only by guarded versioning, never assumed** (the user, 2026-10-30).  Then not
  built: treating every pointer parameter as restrict saved only about 0.2% of the W65C816 suites' cycles
  (2026-10-31, an upper bound); the user chose "measure first".
- **Replacing a loop counter only tested by a test of a pointer stepped with it** was tried and taken out
  (2026-10-29): TestBaseLib65 on the W65C816 0.4% slower with the counter worked out on the way out, 0.1% without
  (CNumber's loops go round 2 or 3 times).  ReplaceLoopCounters, which swaps one integer counter for another going
  up beside it, is a different pass.
- **Full range `i16` reciprocals** were slower than the runtime's 16 bit divide (2026-10-30); since 2026-11-22 the
  cost rule decides each one.
- **Divide by constant costs** (the user, 2026-11-22: "Take it and give ConstantDivisions a cost rule for 32-bit
  reciprocals on the W65C816 first"): stage 11's switch of UpperBound to the shared rules went to master with
  [`CIRDivisionCosts`].  Without the rule the switch cost TestSupportLib65 on the W65C816 419 bytes for 2,253,142
  fewer cycles; with it, 16 kinds of 32 bit reciprocal are calls again.
- **Signed loop counters became unsigned in the libraries** (the user, 2026-10-30) so the W65C816 compares without
  `eor $8000`; that is why ReduceStrength learnt zero extended counters (2026-10-31, TestIROptimiserZeroExtendedCounters
  in `TestCppCompilerLib/TestIR.cpp`).
- **CNumber's digit indices are [`int16`]** (the user, 2026-10-30), and the compiler, not the source, was fixed for
  what followed: ExtendedCounter, BiasSignedIndices, MergeAddressPhis (the user: "do the compiler fix, not source").
  TestNumber on the W65C816 100.5G to 52.35G cycles over that day's work.
- **The OptimiserPlan rules** (the user, 2026-11-21): the code may change if every *65 program still passes on both
  processors and no suite's cycles or bytes get worse; a stage that only moves code must leave every listing the
  same; plan first, measure before and after.
- **Analyses are matchers asked of a phi, not kept lists** (OptimiserPlan stage 6): a kept list would be
  recomputed after every change, and no pass asks twice between changes.
- **The quotient of a division by a constant stays `add q, 0`, not `copy`** (stage 10): a copy is a back end's
  place of its own, which no pass removes.

## Not done

- **The suites' cycles and bytes with ConstantDivisions' cost rule were never measured** (2026-11-22: the run ran
  out of memory and wasn't restarted).  The switch without the rule is measured.  A measure now needs its own
  baseline.
- **`x / 255` of an x under 65536 as `(x + (x >> 8) + 1) >> 8`** in 16 bits (about 20 cycles and 11 bytes against
  about 140 and 63 for the multiply): another way to divide, not a cost; it would take back the 419 bytes and
  TestFontFamily's cycles.  The user's to ask for.
- **Carrying nowrites to other modules** (stage 3): sound, and one line (`IRSA_Carried`), but it changes the code;
  not asked for.
- **The signed range rules in the shared table**: [`CIRRanges::SignedRange`] with its arithmetic and division
  ranges is still a list of its own over the unsigned rules (finding 63's third list), and the unsigned rules have
  two drivers (the back end's table to a fixed point, UpperBound on demand).
- **ReduceDivisions, ReduceStrength and ReduceMultiplies** still look through every block of the function for the
  loop's (the order is the order new instructions are made in, so changing it isn't "same code").
- **A sign extended unsigned counter** is reduced by ReduceStrength without a proof that it doesn't wrap.
- **The W65C816 cycles after the zero extended counter fix** (2026-10-31) were not re-measured.
- **Pointer aliasing versioning from a profile**, and per-call costs, caller to callee edges, argument values and
  branch outcomes from a profile for inlining and layout: discussed, not built.  Interrupts aren't modelled in the
  profile, and real hardware can't be profiled.
- **CNumber ideas**: q and r of one byte from one table load; PrivateSubtract's loop keeps both indices and both
  pointers.
- **Callees with by-value struct parameters were a blocker for profile-guided inlining** (2026-10-30); they are
  inlined now (through a slot), but whether that recovered the FatBench gain wasn't measured.
- **The 65C64 runs no profile-guided pass**: [`CCPPTarget::UsesProfile`] is the W65C816's only.
- **Target opcodes** (the IR's machine level) are unused outside the tests.

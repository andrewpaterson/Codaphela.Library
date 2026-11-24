# Intermediate Representation

Between the C++ and the processors is an intermediate representation (IR).  The front end writes
it, the optimiser improves it, and each back end turns it into its processor's assembly.  It is
the same for both processors, which is why one optimiser serves both.

It is a variant of [QBE](https://c9x.me/compile/)'s, with an LLVM like text form:

- Functions are blocks of small fixed size instructions, each block ending with a jump.
- Values are in SSA form: each temporary is defined once, and phi instructions join the values
  that reach a block.
- Signed overflow wraps around.
- There are no aggregate types: the front end has already laid out classes and does the calling
  convention's work itself.

## Types

| Type | Meaning |
|---|---|
| `i1` | A condition: made by comparisons, used by `brif` and `phi`.  Never in memory. |
| `i8`, `i16`, `i32`, `i64` | Integers.  Signed or unsigned is in the operation, not the type. |
| `f32`, `f64` | IEEE floats (`long double` is `f64`). |
| `ptr`, `ptr.space` | A pointer in the module's first address space, or a named one. |

`i8` and `i16` are values in their own right because the W65C816 works in 16 bits.  Loads and
stores have no alignment: neither processor faults on an unaligned access.

## Instructions

| Group | Instructions |
|---|---|
| Integers | `add sub mul sdiv udiv srem urem and or xor shl lshr ashr neg not copy sext zext trunc` |
| Comparisons | `eq ne slt sle sgt sge ult ule ugt uge`, giving an `i1` |
| Floats | `fadd fsub fmul fdiv fneg feq fne flt fle fgt fge fordered funordered sitof uitof ftosi ftoui fext ftrunc` |
| Pointers and memory | `slot` (a frame object), `ptradd`, `ptrtoi`, `itoptr`, `spacecast`, `load`, `store` |
| Calls | `arg` (one per argument, straight before the call), `call`, `argaddr` (where the caller put an argument) |
| Jumps | `jmp`, `brif`, `ret`, `unreachable`: a block's last, and only there |

Floats stay instructions until the back end makes them soft float calls, so the optimiser can
fold `1.0f + 2.0f`, share a repeated `x * y` and remove an unused one.

A module also holds its globals, as data:

```text
global @gTable, align 4, local { i32 1, i32 2, ptr @Str3+4, zero 52 }
global @Str3, local, constant { bytes "hello\00" }
```

## Text form

The IR prints and parses back exactly, so a pass is tested with the IR text before it and the
text expected after.

```text
function i32 @_Sum__FPii(ptr %p, i32 %n)
{
entry:
	jmp loop
loop:
	%s = phi i32 [entry, 0], [body, %s2]
	%i = phi i32 [entry, 0], [body, %i2]
	%c = slt i32 %i, %n
	brif %c, body, exit
body:
	%o = shl i32 %i, 2
	%a = ptradd ptr %p, %o
	%v = load i32 %a
	%s2 = add i32 %s, %v
	%i2 = add i32 %i, 1
	jmp loop
exit:
	ret i32 %s
}
```

- Temporaries are `%name`, symbols `@name`, and blocks `name:`.  Unnamed ones print as numbers.
- The type after the opcode is the result's, except that comparisons give their operands' type,
  `store` and `arg` give the value's, and conversions are `op T v to T`.
- Floating point constants are written as their bits in hexadecimal (`f64 0x3FF0000000000000`),
  so they print and parse exactly.
- Flags follow the opcode: `load volatile i32 %a`, `ptradd inbank ptr %p, %o`.

In debug builds a verifier checks every function after every pass: each block ends with a jump,
each phi has an argument for each predecessor, each temporary is defined once before its uses,
and each operand has the type its opcode needs.

## The optimiser's passes

The optimiser runs a *pipeline* the target builds.  The passes of a round run in order, and the
rounds repeat until one changes nothing.  Analyses (dominators, definitions, loops) are worked out
when a pass asks for them and kept until a pass changes the function.

| Pass | Does |
|---|---|
| `SimplifyCFG` | Joins and removes blocks, threads jumps through blocks of phis. |
| `UnifySlotTypes`, `SplitSlots`, `PromoteSlots` | Turn stack slots whose address doesn't escape into temporaries; an object's fields become a slot each first. |
| `RemoveTrivialPhis` | Removes phis that only ever have one value. |
| `FoldConstants` | Works out constant operations and simplifies identities; puts constants on the right of commutative operations. |
| `FoldNullTests` | `p == 0` of a pointer that can't be null there. |
| `NarrowTruncations` | Does `char` and `short` arithmetic narrow, undoing C's promotions. |
| `BoundDivisions` | A divide whose dividend is known to be small becomes a compare. |
| `SignedPowerDivisions` | Signed divides by powers of two as shifts and an add. |
| `DivisibilityTests` | `x % c == 0` without a divide. |
| `ConstantDivisions` | Divides by constants as multiplies. |
| `ShareDivisions` | `x / d` then `x % d` share one divide. |
| `RemoveCommonSubexpressions` | Value numbering: a repeated computation is done once. |
| `RemoveRedundantLoads` | A load of what was just stored or loaded. |
| `MergeAddressPhis` | A phi of one base plus different indices becomes the base plus a phi of the indices. |
| `RotateLoops` | Moves a loop's test to the bottom, so each time round is one branch. |
| `LessLoopTests` | `a <= b` as `a < b + 1` where that can't wrap. |
| `ReplaceLoopCounters` | Linear test replacement: a counter only tested is replaced by another value going up beside it. |
| `NarrowCounters` | 32 bit counters that stay in 16 bits count in 16 bits. |
| `ReduceMultiplies` | A multiply of a loop's counter by an invariant becomes an add each time round. |
| `ReplaceExitValues` | A value's final value after a loop, worked out without the loop. |
| `DeleteDeadLoops` | Removes a loop whose results nothing uses. |
| `ReduceStrength` | `p + i * size` in a loop becomes a pointer stepped by `size`. |
| `SplitLoopAddresses` | `p + (a + i)` with `p` and `a` invariant becomes `(p + a) + i`, hoisted. |
| `HoistInvariants` | Moves work the loop doesn't change out of it. |
| `SinkLoads` | Moves a load down to the block that uses it. |
| `HoistCommon` | Work both ways of a branch do is done once before it. |
| `UnswitchLoops` | A loop with a branch on an invariant becomes two loops. |
| `ReplaceLoopIdioms` | Byte loops become calls of `memset`, `memmove`, `strlen`, `memcmp`, `memchr` and `strchr`. |
| `RemoveDeadCode` | Removes what nothing uses. |
| `SinkComparisons` | A comparison only a later branch reads moves next to it. |
| `NarrowMultiplies` | A multiply whose product fits 16 bits is done in 16 bits.  After the rounds. |
| `BiasSignedIndices` | A signed 16 bit index becomes an unsigned one on a moved base, so the W65C816 can index with Y.  After the rounds. |

The 65C64 leaves out `DivisibilityTests`, `ConstantDivisions`, `MergeAddressPhis`,
`NarrowCounters`, `NarrowMultiplies` and `BiasSignedIndices`: it has a divide instruction and
32 bit registers.

Besides the per function passes:

- **The inliner** (`CIRInliner`) puts small functions into their callers, within a unit and, on the
  W65C816, across units.
- **Interprocedural passes** (`CIRInterprocedural`) work over a unit's functions together.
- **Indirect calls through known data** (a vtable entry of a known class, a constant table) become
  direct calls.

Each pass's runs, changes and time are counted in the unit's statistics, so a pass that never
changes anything shows up.

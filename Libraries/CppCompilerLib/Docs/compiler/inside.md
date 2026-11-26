# Inside the Compiler

A unit (one `.cpp` file) goes through these stages.  Everything up to the back end is the same
for both processors.

| Stage | Classes | Does |
|---|---|---|
| Preprocess | [`CCPPPreprocessor`], [`CPPHeaderCache`] | Includes, macros and conditional compilation, to a token stream. |
| Tokenise and parse | [`CCPPTokeniser`], [`CCPPParser`] | C++ tokens to a syntax tree ([`CCPPSyntaxTree`]), with names resolved to scopes and symbols. |
| Declarations and types | [`CCPPUnit`], [`CCPP65TypeChecker`] | Classes laid out, templates instantiated, overloads chosen, each expression's type worked out. |
| IR front end | [`CCPP65IRFrontEnd`] | Each function body written as [IR](ir.md), already in SSA form. |
| Inline and optimise | [`CIRInliner`], [`CIROptimiser`], [`CIRInterprocedural`] | The IR made smaller and faster, the same for both processors. |
| Back end | [`CIRW65C816Backend`], [`CIR65Backend`] | IR to the processor's assembly text, with its own passes over the code. |
| Assemble | [`CAsmW65C816Assembler`], [`CAsm65C64Assembler`] | Assembly text to an object, in memory. |
| Link | [`CAsm65Linker`] | Every unit's object, Libc65 and the runtime, to the output file. |

[`CCPP65Builder`] runs these for every unit of a program, on several threads (one unit a thread),
then links them.

## Preprocessor

The preprocessor works on lines, as a C preprocessor does, and has most of C++17's: `__has_include`,
`_Pragma`, `__VA_ARGS__` with the `, ##` rule, digit separators, binary literals and hexadecimal
floats.  Raw string literals are the main gap.

Two things make compiling a whole library quick:

- **The header cache.**  A header's preprocessed tokens are kept, keyed by the macros it reads,
  so the next unit that includes it with the same macros doesn't preprocess it again.
- **Lazy headers.**  A header's declarations are indexed but only parsed when a unit first
  uses them, so a unit only parses the header code it needs.

The target's macros (`__W65C816__` or `__65C64__`, and `__LIBC65__`) come from the target; Libc65
is a system include directory.

## Parser

The parser builds a syntax tree of the whole unit, with each name looked up in the scope it is
used in.  The same parser drives the HCP65 IDE's syntax highlighting, Go to Declaration and
rename; the IDE checks a file by running the stages up to the type checker without writing any
code.

## Generator and type checker

[`CCPPUnit`] does the C++: it lays out classes, writes vtables and their thunks, instantiates
templates, chooses overloads and conversions, and decides which temporaries need destroying.  It
works the way a C++ compiler does, doing only what is used:

- A class is laid out the first time it is needed whole: an object, `sizeof`, a member or a base.
  A class used only through pointers is never laid out.
- Functions defined outside classes and not `inline` are written where they are defined.  Inline
  functions, functions defined in classes and template functions are written when they are first
  called.
- A class's vtable, and so its virtual functions, is written when an object of the class is first
  made.
- A `dynamic_cast` that has to look for its answer calls `R_DynamicCast`, a routine the compiler
  writes once into a program that casts, with the two classes' type infos; nothing from Libc65 is
  needed for it.  A cast to a base is a conversion, and `dynamic_cast<void*>` reads the vtable in
  place.
- A class whose destructor is virtual has one more virtual function, which the compiler writes: its
  deleting destructor (`_Class__opDeleting`), which destroys the object and frees it with the
  class's own `operator delete` and size.  `delete p` calls it through the vtable, so an object
  deleted through a pointer to a base is freed as its own class frees.  Its call of the global
  `operator delete` is a weak reference: a program that never deletes needn't have one.

So a header like `BaseLib/ArrayTemplate.h`, with all it includes, compiles as long as what the
program uses is supported.

[`CCPP65TypeChecker`] works out each expression's type for overload resolution and the IR front
end, and reports the errors in expressions.

### Names

C++ names become labels with their types in them, so units can declare different overloads of a
function and the linker still joins the right ones.

| C++ | Label |
|---|---|
| `int Add(int, int)` | `_Add__Fii` |
| `size_t Length(const char*)` | `_Length__FPKc` |
| `int32_t Sum(const int16_t*, uint16_t)` | `_Sum__FPKij` on the W65C816, `_Sum__FPKst` on the 65C64 |
| `main`, `extern "C"` functions, a function with no parameters | `_main`, `_name` |
| `CPoint::Move` | `_CPoint__Move` |
| A constructor, destructor, vtable | `_Class__0`, `_Class__1`, `_Class__2` |
| `CArrayTemplate<int>::Add` | `_CArrayTemplate__T_i__Add` |
| `operator[]`, `operator+=` | `_Class__opIndex`, `_Class__opAddAssign` |
| A static function or global | `_name`, local to the unit's object |

The type codes are those of the Itanium C++ ABI, one for each type however wide it is: `v` void,
`b` bool, `c` char, `a` signed char, `h` unsigned char, `w` wchar_t, `Ds` char16_t, `Di`
char32_t, `s` short, `t` unsigned short, `i` int, `j` unsigned int, `l` long, `m` unsigned long,
`x` long long, `y` unsigned long long, `f` float, `d` double, `e` long double, `P` pointer, `R`
reference, `K` const, `C` length and name for a class, `N` for an enum.  C++ names can't contain `__` or start with
a digit, so a label can't clash with one the compiler makes up: `L5` for a label in a function,
`Str0` for a string literal, `R_` and `R816_` for runtime routines.

On the W65C816 a function that takes arguments in registers has a second label with a suffix
naming where each argument word is (`_Sum__FPKij.axy`): see
[W65C816 Code](w65c816.md#register-arguments).

## IR front end

[`CCPP65IRFrontEnd`] writes each function body as IR: every local as a stack slot, every
expression's value as a temporary, objects' constructors and destructors as calls.  It writes
global constructors (`R_Initialise<n>`) and destructors (`R_Finalise<n>`) the same way.  A function
using C++ it can't write is an error that names the compiler's own source line, which shows which
construct it was.

## Optimiser and inliner

The optimiser works on one function at a time, in rounds, until a round changes nothing.  The
target chooses the passes: the W65C816 runs every one, the 65C64 leaves out seven that don't pay on
it.  [Intermediate Representation](ir.md#the-optimisers-passes) lists them.

The inliner puts small functions into their callers.  On both processors small functions are also
inlined *across units*: each unit's small functions are kept after it is compiled, and a unit that
called one before it was kept is compiled again at the end.  Functions that name a unit's own
symbols (its string literals, static globals and static functions) aren't shared.

## Back ends

Each back end turns IR into assembly text:

- [W65C816 Code](w65c816.md): places each value in the direct page frame, A, X or Y, splits
  32 and 64 bit operations into 16 bit pieces, and calls the runtime for multiply and divide.
- [65C64 Code](65c64.md): allocates registers and pseudo registers, then passes over the text for
  the flags, register widths and a last peephole.

The assembly goes straight to the processor's assembler in memory.  With a listing directory set,
it is also written out as a `.lst` file with each C++ line as a comment.

## What gets measured

[`CCPPCompileStatistics`] counts and times each unit's compile: the phases (preprocess, tokenise,
parse, declarations, types, front end, inline, optimise, back end, code passes, assemble, link),
each optimiser pass's runs and changes, and the things the compiler is being reshaped to remove.
The test suites print them with `TEST_PROJECT65_COMPILE_STATS`, and
`CommonLibraries/Tests/CompilerMeasure.py` makes tables of them.

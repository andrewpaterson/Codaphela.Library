# Front End Design

The front end is everything between a `.cpp` file and the IR: the preprocessor, the tokeniser, the parser and its
syntax tree, and the semantic layer that decides what the C++ means (classes, templates, overloads, constants, types)
for the [IR front end](ir.md) to write.  [Inside the Compiler](../inside.md) lists the stages and
[Supported C++](../language.md) says what is and isn't supported; this page is how the front end is built and why, for
someone who will change it.

The library started on 2026-10-21 as CppParserLib, a parser for the IDE's syntax highlighting, and grew into the
compiler.  The user's decisions that day still shape it: a hand-written recursive descent parser that looks names up
as it parses, a semantic pass kept apart from the parser, a syntax tree that keeps where each node came from in the
tokens, and the IDE as the first consumer.  The same code runs in a build and in the IDE's check of a file
([`CCPPSemanticPass`]), so anything the front end gets wrong shows in both.

## Stages and their errors

| Stage | Classes | Gives |
|---|---|---|
| Preprocess | [`CCPPPreprocessor`], [`CPPHeaderCache`] | [`CPreprocessedTokens`]: tokens with their file, line, column and the macro expansion they came from. |
| Tokenise | [`CCPPTokeniser`] | C++ tokens: keywords, punctuators, identifiers by ID, literals with their values. |
| Parse | [`CCPPParser`], [`CCPPHeaderIndex`] | A [`CCPPSyntaxTree`] whose names are already resolved to scopes and symbols. |
| Mean | [`CCPPUnit`] and its parts, [`CCPPBinder`], [`CCPP65TypeChecker`] | Classes laid out, templates instantiated, calls bound, each expression typed. |

Each stage keeps its own [`CCPPErrors`]: a list of "File (line, column): message [token]" lines with the file, line,
column and token of each.  The user asked for one error class shared by the stages, with an instance each, not one
list for all (2026-10-23).  The preprocessor's tokens are its own (an entry's token is -1), the tokeniser's are
preprocessed token indices, the parser's and the unit's C++ token indices.  The unit's list belongs to
[`CCPPDiagnostics`], which also decides when an error is said at all (below).  [`gcUserError`] is left for the
compiler's own sanity checks, not for errors in the source.

## The preprocessor

[`CCPPPreprocessor`] works a line at a time, as a C preprocessor does, but its macro expansion was rewritten on
2026-10-21 (the user's choice over mending the old one, which had no rescanning or hide sets and crashed on the
C standard's 6.10.3.5 examples).  It reads from a stack of sources, [`SPPSource`]: a file, a macro's replacement or an
argument, so a macro's expansion is read before the rest of the line that invoked it and rescanning crosses lines.
A macro being expanded is disabled while its source is read, and a token that named a disabled macro is marked
`PTF_NO_EXPAND` so it is never expanded later, as clang does, rather than carrying hide sets.  Placemarkers, GNU's
`, ## __VA_ARGS__`, `_Pragma` and `__LINE__` as g++ gives it are all there.  `#if` is
[`CPreprocessorExpression`], with [`intmax_t`] arithmetic and the alternative spellings (`and`, `not`, `bitand`:
[`CCPPPreprocessor::AlternativeOperator`]).

Its output keeps what the IDE needs beside the tokens: [`SMacroExpansion`] for each expansion, [`SInactiveRegion`] for
skipped groups, [`SDirectiveLine`] for directives, and the comments in [`CSourceFile::GetComments`].  The highlighter
([`CCPPHighlighter`]) is built from those side tables and the tokens.

Phase 1's trigraphs are replaced in the file before it is tokenised ([`CSourceFile::ReplaceTrigraphs`]), skipping raw
strings; a column after a trigraph on its line is two less than the source's.  `%:` and `%:%:` are `#` and `##`, spelt
as written when stringified ([`CPPHashes::mbDigraph`]).  The clause 16 sweep (2026-11-22) made the preprocessor stricter
than it had been: [`CCPPPreprocessor::CheckReplacementList`] checks a definition, a redefinition must be spelt the same,
a variadic macro's call needs an argument for its `...`, and a `##` that doesn't make one token is an error.  The
libraries and HCP65 relied on neither of the last two.

A unit's headers are mostly the same as the last unit's, so [`CPPHeaderCache`] keeps a header's preprocessed output
for every unit the builder compiles.  It is keyed by what the header saw: a recorder ([`SPPRecorder`]) notes each
name the header looks up or defines, and only a name's first lookup depends on who included it.  A header is made
whole before it is published to the cache and never changes after, so threads share it under a lock.

How it was proved, 2026-10-21: every one of the 1,573 units of the libraries preprocessed and compared token by token
with `i686-w64-mingw32-g++ -E`.  The differences left were expected ones: `__has_builtin` answering 0, g++ dropping
statements with Latin-1 bytes, and includes that match only ignoring case.  On Linux the MinGW headers stand in for
the Windows SDK's; Windows system headers are only skimmed (below), never compiled.

## The tokeniser

[`CCPPTokeniser`] turns preprocessed tokens into C++ tokens: each keyword and punctuator its own kind (digraphs,
alternative tokens, and the MSVC and GNU keywords too), identifiers given IDs, and literals parsed.  Literal reading
is shared with `#if` and the code ([`ParseNumberLiteral`], [`RawStringLength`] in `LiteralHelper`), so the three can't
disagree.  Any identifier after a number that isn't one of the language's suffixes is a user-defined literal's
suffix ([`CCTUserLiteral`]), and the semantic layer says whether there is an operator for it.

Identifiers are UTF-8 inside the compiler: a universal character name in one becomes its UTF-8 when the line
tokeniser makes the token, so both spellings are one name.  Which characters a name may have is checked once, where an
identifier is made ([`InvalidCPPNameCharacter`]); a byte of 0x80 or more is a letter to [`IsCPPNameStart`], so a
stray Latin-1 byte in code is that error.  Names aren't encoded when they become labels: the assembler takes UTF-8 in
labels.

## The parser and the syntax tree

[`CCPPParser`] is hand-written recursive descent with precedence climbing for binary operators
([`CCPPParser::ParseBinaryExpression`]).  It looks every name up as it parses, in [`CCPPScope`]s owned by the unit's
[`CCPPSymbolTable`], because C++ can't be parsed without knowing which names are types.  Where the standard's
ambiguity rules need it ([stmt.ambig], [dcl.ambig.res]) it parses tentatively ([`CCPPParser::BeginTentative`]), recording
nothing until it knows.  The first `>` of a `>>` closes a template argument list and the second is left.  Member
function bodies, and default arguments that name later members, are parsed again once their class is complete
([`CCPPParser::ParseDeferredBodies`]).

Some ambiguities need types the parser hasn't got.  `(decltype(d)::k) + 1` is a cast or an addition depending on what
`k` is: [`CCPPParser::IsCastAhead`] asks [`CCPPParser::IsDecltypeMemberCast`], which looks the member up where it can
(`d` a variable declared with a class's name, or `f()` whose declarations all return one:
[`CCPPParser::DeclaredClassMemberIsType`]) and otherwise guesses from what follows the `)`.

The tree's nodes are one class for each construct ([`CCPPSyntax`] and its `CCPPSyntax...` subclasses), each knowing its
first and last token ([`CCPPSyntax::miFirstToken`]), so the IDE can map any node back to the text.  They are allocated
in an arena ([`CCPPSyntaxMemory`]) and freed together, not made as BaseLib objects.  Types are not syntax: they are
[`SCPP65Type`]s, made unique by [`CCPP65Types`].  Every node prints itself back as source ([`CCPPSyntax::PrettyPrint`]):
the parser's acceptance test on 2026-10-21 was every library unit parsed with no errors and printed back token for
token.  The node classes were first generated by a script that was never committed; they are kept by hand now.

### Headers: skimmed, lazy and read back

A unit includes far more than it uses, so the parser avoids most of it:

- **System headers are skimmed** ([`CCPPParser::IsSkimming`]): declarations are read, function bodies skipped and parse
  errors ignored.  The bodies of templates are parsed even there ([`CCPPParser::miTemplates`]): an instance of a
  skipped body was an empty function (`std::make_exception_ptr` returned null).  A non-template inline function in a
  system header is still never written, so Libc65's headers define nothing inline that isn't a template.
- **Lazy declarations.**  [`CCPPHeaderIndex`] remembers, for every header declaration a unit has parsed, where it
  starts.  A later unit whose tokens there are the same skips it and declares its names as lazy symbols
  ([`CCPPSyntaxLazyDeclaration`]); the first lookup of one parses it ([`CCPPParser::LoadLazy`]).  So a unit parses only
  the header code it uses.
- **Syntax blocks.**  A declaration already parsed by another unit is read back as bytes rather than parsed again
  ([`CCPPSyntaxBlockWriter`], [`CCPPSyntaxBlockReader`]; the format is in [`CppCompilerLib/CPPSyntaxBlock.h`]): nodes,
  scopes and symbols by 16 bit indices, tokens relative to the declaration's first, so a block fits any tree with the
  same tokens.

Each thread that compiles units keeps one [`CCPP65UnitWorkspace`], an arena and allocators emptied for each unit, so a
unit's memory isn't allocated afresh and threads don't wait for each other's allocations.

### What the parser checks

A rule that needs only the syntax is the parser's, so it is checked in every declaration whether or not anything
uses it: [`CCPPParser::CheckDeclSpecifiers`], [`CCPPParser::CheckClassMembers`], [`CCPPParser::CheckSpecialMember`],
[`CCPPParser::CheckAnonymousUnion`], [`CCPPParser::CheckTemplateDeclaration`] and [`CCPPParser::CheckDefaulted`].  A rule
that needs types is the semantic layer's, and is checked where the thing is first used (next section).  This split
came from the clause sweeps (2026-11-20 and 21): several invalid programs were accepted only because nothing used
the class or function that was wrong.

## The semantic layer

[`CCPPUnit`] is one translation unit's meaning.  It was one class of a dozen jobs (605 functions on 2026-11-22); the
semantic layer plan has been moving each job into a part of its own that the unit holds and hands out:

| Part | Does |
|---|---|
| [`CCPPContext`] | Where the compiler is: template bindings, namespace, linkage, the function being written, whether there is a `this`, whether the operand is evaluated, what is being worked out. Entered and left, never saved and restored by hand. |
| [`CCPPDiagnostics`] | Errors and warnings, quiet code, trials, kept and deferred errors. |
| [`CCPPBinder`] | What a call, a name, a member, a conversion or a construction means, decided once for the type checker and the IR front end; owns [`CCPPOverloads`] and asks [`CCPPAccess`]. |
| [`CCPPNames`] | Which class a name or its qualifiers mean, and whether lookup in a class finds a name more than once. |
| [`CCPPDeclarators`], [`CCPPFunctionDeclarer`] | Declarations to types; functions declared, their returns deduced, their declarations checked. |
| [`CCPPClasses`] | Classes made, their bases resolved, laid out ([`CCPPClassLayout`]) when first needed whole, `final` and `override` checked. |
| [`CCPPInstantiator`] | Templates: instances, specialisations, outside definitions, explicit instantiation; deduction is [`CCPPDeduction`] and [`CCPPPatternMatch`]. |
| [`CCPPClosures`] | Lambdas: each closure ([`SCPPLambda`]), its captures, its `operator()` and conversion, generic lambdas' instances. |
| [`CCPPConstantFolder`], [`CCPPConstants`] | Constant expressions (below). |
| [`CCPPExceptions`] | Exception specifications and what an expression may throw. |
| [`CCPPEnums`], [`CCPPStrings`], [`CCPPAttributes`], [`CCPPBuiltins`], [`CCPPDesugarings`] | Enums, string literals, attributes and `[[deprecated]]`, `__builtin_`s, and what the compiler makes once to stand for what was written (a range-for's variables, a literal's call, braces put back). |
| [`CCPPGlobals`], [`CCPPClassData`], [`CCPPMangler`], [`CCPPUnitData`] | Static data, start-up and exit lists; vtables, VTTs and type infos; labels; the unit's assembly other than code. |
| [`CCPPCheck`], [`CCPPWarnings`] | The checks of a unit as written (for the IDE, and the source file's unused code); the W65C816's 16 bit `int` warnings. |

What is left of the move, and the next stage (stage 24: the parts stop being the unit's friends, each given a narrow
interface), is [`CppCompilerLib/SemanticLayerPlan.txt`].  Its source, the code review of 2026-11-21, is
[`CppCompilerLib/SmellCheckFindings.txt`], which also feeds [`CppCompilerLib/ConsolidationPlan.txt`].

### Quiet code, trials and when an error is said

Much of the semantic layer asks questions that may fail without the program being wrong: does this overload take
these arguments, is this a constant, can this template be instantiated (SFINAE).  So errors depend on where the
compiler is ([`CCPPContext::BeginQuiet`], [`CCPPDiagnostics`]):

- In **quiet** code an error is only noted: what is done there is judged by what it returns.  A declaration the
  compiler can't do yet is only an error where it is used.
- A **trial** is quiet code whose failing is its own ([`CCPPContext::EndTrial`] says if it failed).  Overload
  resolution and SFINAE are trials.  [`CCPPCompileStatistics`] counts them: their number is how the declarations
  phase's speed is measured (below).
- A **loud** part is said whatever is quiet around it: access errors, for example, are errors where they are and are
  said once for each place ([`CCPPDiagnostics::ReportOnce`]), even inside a trial, since they never change what a
  name means.
- What is worked out once and remembered keeps its errors with it ([`CCPPDiagnostics::BeginKeeping`]) and says them
  when the answer is wanted for good.  What is worked out once where errors are dropped (a class laid out, an
  instance made) **defers** them to the end of the unit ([`CCPPDiagnostics::Defer`]).

### The type checker and the IR front end

[`CCPP65TypeChecker`] works out the type of an expression, and what is wrong with it, without writing code; it walks an
expression the way [`CCPP65IRFrontEnd`] writes it, and its functions have the front end's names (`Value`, `Place`,
`Construct` ...), so a type and its code are found in the same place in both.  Both decide what a program means, and
where they disagree the IDE shows an error the build doesn't, or the other way round.  The rule since the clause
sweeps: a rule of the language goes in the binder ([`CCPPBinder`]) or a part of the unit, where both ask it, not in
one of them.  [`CCPPBinder::Bind`] is the one cascade that says what a call is.

The IDE's check ([`CCPPSemanticPass`], driving [`CCPPCheck`]) checks every function of the source files, called or
not, statements included (jumps, switches, returns); a build is the IR front end asking the checker for each
expression's type.  So a rule about declarations or copies that only the checker's statement walk asked was missed
in a build: such rules are asked from the front end's own construction paths too, or put in the binder.  The checker
gives the IDE what it needs to refactor: the type of each expression and how its place can be changed
([`CCPPUnitObserver`] is the hook; a build's observer hears nothing).

What the check is known to get wrong of the conformance cases is listed in
`Tests/TestCppCompilerLib/TestCppCompilerLib/TestCPPConformanceChecked.h` (37 cases on 2026-11-26: 20 need
constant evaluation, 6 are templates', 5 exceptions').  The semantic layer plan's next large step is to work out what an expression is (its type,
whether it is an object, an rvalue, a place) once, in the type checker, and have everything else read it.

## Declarations, and what is checked where

The front end works as a C++ compiler on a large program must: it does only what is used.

- A class is laid out the first time it is needed whole (an object, `sizeof`, a member, a base); one used only
  through pointers never is.  Laying out declares every member function, which is the costly part
  ([`CCPPFunctionDeclarer`]).
- A function defined in a class, an inline function or a template is generated when first called.
- A class written in the unit's own source file is laid out where it is defined, so its `final` and `override` are
  checked even if nothing uses it ([`CCPPClasses::CheckVirtSpecifiers`], [`CCPPCheck::IsInSourceFile`]); a class in a
  header is checked by the first unit that needs it whole.  The same goes for function bodies:
  [`CCPPCheck::CheckUncalledFunctions`] checks the source file's uncalled definitions, never a header's, so a library's
  unused inline functions cost nothing.
- A unit the builder compiles a second time (to inline across units) has already said everything it will say, so it
  skips the checks that only report ([`CCPPUnit::SetCompiledBefore`]).
- [`CCPPCheck::ReportNamedInstances`], the warning about classes that only name a template's instance, walks only the
  parts of the tree that hold a class key or a base's name ([`CCPPTokenMarks`]: for each token, how many marked
  tokens are before it, so "is one in this node" is two reads).

These are the result of the declarations speed work of 2026-11-21.  From 18 to 21 November the phase had grown from
about 6 s to 28 s, all of it two new checks: the named-instance walk over every node, and laying out every class a
header mentioned to check `override`.  Measured compiling TestSupportLib65's project for the W65C816 a unit at a time
(Release x64, `CompilerMeasure.py --compile --serial`):

| | Before | After |
|---|---|---|
| Declarations phase | 28.2 s | 6.4 s |
| Quiet trials | 6,265,360 | 1,728,988 |
| Class layouts | 246,098 | 90,472 |
| Named-instance nodes visited | 484,627,079 | 1,842,970 |
| The IDE's phases (preprocess to front end) | 70.2 s | 49.5 s |
| Whole compile | 149 s | 127 s |

The code written was the same byte for byte (4,201 listings compared), and every warning was still given, by fewer
units.  `TestCPP65Declarations` compiles a small project on both processors and asserts the layouts, nodes and quiet
trials as exact numbers, so a change that lays classes out early or walks the tree again fails a test.  The plan,
[`CppCompilerLib/DeclarationsSpeedPlan.txt`], is finished; it is kept only as the record code cites by step.

## Types

Each fundamental type is a type of its own however wide ([`SCPP65Type`] has its integer kind): `int` and `long` are
different on the 65C64 though both are 32 bits, as are `short` and `int` on the W65C816.  Until 2026-11-18 the
compiler's types were its sizes; the user asked for each to have its own identity, so they overload, specialise
templates, are caught and named apart, and labels use the Itanium codes ([Names](../inside.md#names)).  Promotions
and the usual arithmetic conversions go by rank ([`CCPP65Types::Promote`], [`CCPP65Types::Arithmetic`]).
`std::nullptr_t` is a type of its own too ([`CPP65TK_NullPointer`]), converting to any pointer and to `bool` only where
it directly initialises one or is tested ([`CCPP65TypeChecker::KeepsNullPointer`]).  [`CCPPTypeWriter`] is the one
writer of a type as C++ text, for messages, `typeid` names and the IDE.

A string literal is an array of `const` characters.  Its conversion to `char*` is still accepted, since the libraries'
g++ builds use `-Wno-write-strings`; the warning is a build switch, off by default.

## Templates, overloads and lambdas

Overloads are [`CCPPOverloads`]: candidates compared argument by argument ([`CCPPOverloads::CompareCandidates`],
[`CCPPOverloads::CompareArgument`] with [over.ics.rank]'s tie-breaks), a template's instance competing with the declared
functions and winning only where it is better ([`CCPPOverloads::FindConstructor`] does the same for constructors and
constructor templates), function templates ordered by [`CCPPOverloads::MostSpecialised`].  Deduction is
[`CCPPDeduction`]; matching a partial specialisation's or an alias template's pattern against a type is
[`CCPPPatternMatch`], which defers what can't be deduced (`typename T::Type`) until the parameters are known.  SFINAE is
an instance declared on trial.  Two-phase lookup in an instance limits the overloads to those declared before the
template ([`CCPPOverloadScopes::LimitTo`]), and a name found only in a dependent base is refused
([`CCPPBinder::InDependentBase`]).

A closure is an unnamed class made the first time its lambda is typed ([`CCPPClosures::GetLambda`]): a field for each
capture, an `operator()` that is `const` unless the lambda is `mutable`, and for a lambda that captures nothing a
conversion to a function pointer through a static invoker.  A closure in an inline function or a template is one type
in every unit: its functions and static locals are COMDAT, named for the function and the lambda's number in it.

## Constant expressions

A constant is worked out two ways, the quick one first ([`CCPPConstantFolder`]):

1. The syntax tree is folded ([`CCPPConstantFolder::Fold`]): literals, enumerators, `sizeof`, `const` integers with
   constant initialisers and the operators on them.  The common case stays fast.
2. What the tree can't give (a `constexpr` call, a member of a constant object, a float) is run
   ([`CCPPConstantFolder::RunConstant`]): [`CCPPConstants`] writes the expression as the body of a function of no
   arguments, in IR, with front ends of its own that generate no code, and [`CIRInterpreter`] runs it, writing each
   `constexpr` function it calls the same way when asked and keeping it.

The user's ruling (2026-11-15) was to run the IR rather than teach the tree folder C++14's statements one at a time:
the IR already says loops, locals, assignments and returns, and the front end already writes it.  The interpreter
keeps memory as objects and pointers as an object and an offset, so it catches a read of what was never written or a
store outside its object.  Its integer arithmetic is the optimiser's folding, so the two can't disagree.

- Signed overflow is not a constant: the front end marks signed arithmetic (`IRF_NoSignedWrap`) for the interpreter.
- The limits are [implimits]': 512 nested calls and 1,048,576 full-expressions, counted at full-expression marks
  (`IRF_FullExpression`), both options ([`CCPPCompileOptions::muiConstantDepth`],
  [`CCPPCompileOptions::muiConstantSteps`]).
- A failure says why, with the `constexpr` calls it failed in ([`CCPPConstantFolder::ConstantError`]); a definition is
  checked as [dcl.constexpr] asks when it is defined ([`CCPPConstants::CheckDefinition`]).
- A `constexpr` global is its bytes as data ([`CCPPConstants::AppendData`]), so it has its value before any dynamic
  initialisation runs.
- Floats are worked out on the host; the conformance case ConstexprFloatAgrees runs awkward values on both targets
  with Libc65's soft float linked, and they agree.

[`CppCompilerLib/ConstexprPlan.txt`] was built in full on 2026-11-16; it is kept only as the record code cites by
step.

## Classes, vtables, `typeid` and `dynamic_cast`

Classes without virtual bases are laid out as MSVC lays them out ([`CCPPClassLayout`]), because BaseLib reads and writes
its classes as raw bytes.  Virtual bases (the user, 2026-11-16) are laid out the Itanium ABI's way, offsets in the
vtable and not a pointer in the object, which is usually smaller: after the class's own part, once each, with
complete and base versions of constructors and destructors and a cut-down VTT, so a base being made or destroyed sees
its own virtual functions.  One difference from Itanium was kept: a nearly empty virtual base isn't made a primary base
sharing the vtable pointer, which would complicate construction vtables and thunks to save 4 bytes in rare classes.
Layouts were compared with `g++ -m32`'s and kept in `TestCPP65GeneratorVirtualBaseLayouts`.

Every vtable part ([`CCPPVtable`]) has two words before it: the distance from that vtable pointer to the object's start,
and the class's type info; before those, the distances to the part's virtual bases.  The vtable pointer still points at
the first function, so virtual calls don't change.  A type info starts with the type's C++ name and its kind's word
([`ECPP65TypeInfoKind`]); a class's has an entry of three words for itself and each base subobject (its type info, its
offset, and flags: reached publicly, virtual).  All of it is written by [`CCPPClassData`] as COMDAT, the same in every
unit, so a type info's address is its identity across units.  It costs 8 bytes a vtable part and a type info per
polymorphic class whether or not any `dynamic_cast` is used; the names cost the kernal 1,021 bytes (2026-11-18).

`dynamic_cast` (asked for by the user 2026-11-03, first to make BaseLib's [`Ptr::IsInstanceOfTemplateClass`] cheap) is
a conversion when it casts to a base, and `dynamic_cast<void*>` reads the distance to the top in place.  Otherwise it
calls `R_DynamicCast`, a routine written once into each unit that casts ([`CCPP65IRFrontEnd::DynamicCastRoutine`],
[`CCPPGlobals::WriteDynamicCast`]) in a section the linker keeps one of, so no library has to be linked.  It searches
the object's class's entries for the target, refusing a base reached through a private one or found twice.  The
first version wrote the search inline at every cast; the user chose the routine after seeing the sizes (2026-11-20): a
function with one cast went from 423 to 451 bytes to 59 on the W65C816, and from 188 to 202 to 34 on the 65C64.  A
failed `dynamic_cast<T&>` traps without exceptions and throws [`std::bad_cast`] with them.

A class whose destructor is virtual has one more virtual function, its deleting destructor
([`CCPP65IRFrontEnd::DeletingDestructor`], [`CCPPClass::mpcDeleting`]), so `delete` through a base frees with the right
class's `operator delete` and size (the user chose it over type info at run time, 2026-11-20).  Its call of the global
`operator delete` is a weak reference, so a program with virtual destructors and no heap still links.

## Exceptions

Exceptions were built by another session, 2026-11-18 to 20, behind a build option that is off by default
([`CCPPCompileOptions::mbExceptions`]): off, `throw` and `try` are errors and no program's bytes or cycles change.  What
the option does and costs for a program is in [Supported C++](../language.md#exceptions).

The scheme is a second way out of each call.  A function that throws returns as it always does, with the carry flag
set; the caller tests the carry after any call that may throw and branches to code that destroys what it made and
then handles the exception or returns in the same way.  Nothing walks the stack, there are no tables and nothing is
saved when a `try` is entered.  Tables were rejected because the frames are irregular (the W65C816's frameless
functions, short exits and register stubs), and `setjmp` chaining because every scope with an object to destroy would
have to register, not only every `try`.  As ordinary branches in the IR, every exceptional edge is one the optimiser
already understands: `thrown` after a call and a fifth way for a block to end, `throw` (the IR side is
[Intermediate Representation](ir.md)).  A function declared `noexcept` costs its callers nothing, and the
interprocedural pass marks functions that can't throw so their calls aren't tested.

In the front end ([`CppCompilerLib/CPP65IRFrontEndExceptions.cpp`]) what an exception must undo is a cleanup stack:
each entry (an object to destroy, the elements of an array made so far, a constructor's bases and members made so far,
memory from `new` whose constructor may throw, a handler being run, a static local's guard, a `try`'s mark) gets a
block, made only when asked for, that undoes it and jumps to the block of the entry below
([`CCPP65IRFrontEnd::Landing`]).  An entry is pushed only when its object's construction has finished: that rule had
to become true everywhere before any of it worked.  Exception specifications are [`CCPPExceptionSpec`], worked out by
[`CCPPExceptions`] (an implicit member's from its parts', [`CCPPExceptions::ImplicitExceptions`]).

The runtime is Libc65's ([`Libc65/exception.cpp`]): C functions the front end calls by name ([`__Throw`], [`__Match`],
[`__BeginCatch`], [`__EndCatch`], [`__Rethrow`], [`__Terminate`] and the rest), and `<exception>`, `<new>` and `<typeinfo>`.
Matching a handler reads the same type infos as `dynamic_cast`.  Assembly called from C++ in a program with exceptions
must be declared `noexcept`, as it leaves the carry as it falls; the build says which isn't.

[`CppCompilerLib/ExceptionsPlan.txt`] was built in full (its stage 10 being the IDE's refactorings); it is kept only
as the record code cites by step.

## C++14 compliance

The aim (the user, 2026-11-19) is a freestanding C++14 implementation first and the standard library after.  It is
measured by `Tests/TestCppCompilerLib/TestCppCompilerLib/TestCPPConformance.cpp`: groups of small programs, each
returning a number made of bits or required to be refused, run on both processors.  The clause groups were written
paragraph by paragraph from N4140, every case checked against `g++ -std=c++14 -pedantic-errors` first
(`Tests/TestCppCompilerLib/Conformance/clauses.py`), the older stage groups against the Microsoft compiler.  The way a
clause was done: measure, show the user the list worst first (wrong code, valid code refused, invalid code accepted),
fix what the user picks.  Where g++ and the standard's text disagree, g++ was followed and the case left out.

| Groups | State on 2026-11-26 |
|---|---|
| Clauses 2, 3, 4, 6, 8 to 14 and 16 | Every case passes on both processors. |
| Clause 5 | All but ConstantInactiveUnionMemberRefused on the 65C64 (the interpreter keeps bytes, not which union member is active; the user didn't want it fixed). |
| Clause 7 | All but two on both: `constexpr P p;` with a `constexpr` default constructor is refused, and a `constexpr` static member never defined is accepted (found by the semantic layer plan's stage 0, 2026-11-22). |
| Clause 15 (the Exceptions group, built with the option on) | All 180 pass. |
| Stage1 to Stage10, Works | The hand-written cases of the first C++11 and C++14 work: all pass. |

What is left (owed checks on Windows, the freestanding headers, a few small things) is in
[`CppCompilerLib/Cpp14CompliancePlan.txt`].  The record of the first compliance work, its numbered questions and how
each stage was built, [`CppCompilerLib/CppParserTodo.txt`], is kept only as the record the sources cite by number;
its rulings are below, by those numbers.

Deliberately not as the standard says (the user accepted each, 2026-11-16 unless dated):

- Access control allows a little more: a class template's instances are one class, and a friend function is known
  by its name alone.
- As MSVC: `T&` takes an object with no name, and a forwarding `T&&` deduces an array or a function as a `const`
  pointer.
- Without the exceptions option there is no exception handling at all, and `new` that fails gives null.
- `delete[]` of elements with no destructor, and `::delete` of a polymorphic object, call the `operator delete` without
  a size, as [expr.delete] 10 allows.
- A nearly empty virtual base isn't a primary base (above).
- On the W65C816 `alignof` is 1 for every type `alignas` doesn't align ([W65C816 Code](w65c816.md)).

## Decisions

- The front end (the user, 2026-10-21): replace the old raw-text parser and keep the tokeniser; C++14 (N4140); a
  hand-written recursive descent parser with name lookup as it parses, tentative parsing and precedence climbing; the
  semantic pass separate; nodes in an arena with first and last tokens, types separate from syntax; the highlighter its
  first consumer.  Windows system headers are skimmed, not compiled.
- The macro expansion core was rewritten, not mended; the project, library, header maps, directive tokenising and
  conditional stack were kept (the user, 2026-10-21).
- One error class, [`CCPPErrors`], an instance for each stage (the user, 2026-10-23).
- A function that overrides a base class's virtual function without `override` or `final` is warned about, as
  g++'s `-Wsuggest-override` does, and the warning is on by default (`mbWarnMissingOverride` in
  [`CCPPCompileOptions`]; the user, 2026-11-16).  A destructor isn't warned about.
- `__cplusplus` is `201402L` (2026-11-15).  `"%"PRId64` with no space is read as g++ reads it.
- Access control is enforced, and what it broke in BaseLib and HCP65 was fixed with it (the user, 2026-11-15).
- `constexpr` by running the IR, planned on its own (the user, 2026-11-15).
- Virtual inheritance in scope and tested thoroughly (2026-11-15), laid out Itanium's way (question 9, 2026-11-16).
- A failed `dynamic_cast<T&>` traps when there are no exceptions (the user, 2026-11-15).
- Exceptions: built, off by default, programs opt in (question 1; "Keep it off by default, programs opt in",
  2026-11-18).  The second-way-out scheme, assembly declared `noexcept`, the runtime in Libc65, `throw(...)` lists
  built, no exception across an interrupt or a kernal call, every function returning with the carry clear: the plan's
  assumed answers, accepted ("Your assumed answers are fine", 2026-11-18).  Speed-ups only past 5% in size or 2% in
  time; the user stopped them after the 65C64's stubs were shared ("Stop the speed ups here please", 2026-11-20).  The
  IDE checks a file with its build configuration's options, made by the build's own code (2026-11-20).
- The library (question 2, 2026-11-19): "I don't use the C++ Std Libs, and don't want to include the library with
  their source code in any of my projects"; Libc65 stays; "a freestanding C++14 implementation first, then the
  standard libraries afterwards".  The freestanding headers' plan (in Libc65, from the draft branch) waits for the go-ahead.
- Questions 3 to 11 (2026-11-18): "pick the answer that is most correct for the standard, I'm not worried about
  performance".  3: signed overflow isn't a constant.  4: the [implimits] limits, as options.  5: a closure in an
  inline function or template is one type in every unit (COMDAT).  6: a static local's guard has three states, and
  reaching it while it is being made traps (`R_RecursiveInit`).  7: `thread_local` is static storage on its own exit
  list, run first.  8: `typeid(x).name()` is the type's C++ name.  10: a `const` integer of the enclosing scope may
  be used in a lambda without capturing it where it isn't odr-used.  11: static objects are destroyed at exit in
  reverse order of construction, interleaved with [`atexit`] functions.
- Question 12 (2026-11-18): the linker may drop an unused global with a constructor; keeping them is a build option
  (`KeepUnusedObjects`): "I suspect it's usually fine".
- Question 13 (2026-11-18): "give each fundamental type its own identity please"; Libc65's fixed width types are `int`
  where `int` is that wide.
- Question 14 (2026-11-18): [`size_t`] is 16 bits on the W65C816, the 32 bit allocation and string functions have a
  `32` suffix, large allocations take whole banks, [`ptrdiff_t`] is `long`.
- The answers of 2026-11-20: a string literal to `char*` warns only by build switch ("A build swith"); `unsigned
  int(3)` is refused unless a build switch allows it; C conversions are a build option, not decided by a file's name;
  deleting destructors for `delete` through a base; a member function's `const` is part of its type ("Fix it now");
  constructor templates later (built in the re-plan of that day); `R_DynamicCast` as a routine ("Build the runtime
  function").
- A union's inactive member read in a constant on the 65C64 is left accepted ("no I don't want item 2", 2026-11-20).
- A sweep is measured and shown as a list before anything is fixed; the user picks what is fixed (2026-11-19).
- `override` and `final` in a header's class are checked by the unit that first needs the class whole, not by every
  unit that names it (DeclarationsSpeedPlan question 1, 2026-11-21).
- Rules go where both the type checker and the IR front end ask them (the clause sweeps, 2026-11-21): the checker and
  the front end mustn't each decide.
- Syntax-only rules are the parser's, so a class or function nothing uses is still checked (2026-11-21).

## Not done

- The semantic layer plan's stages 24 and 25 ([`CppCompilerLib/SemanticLayerPlan.txt`]), and the expression-once
  rework after them.
- The compliance plan's owed checks: a Microsoft build of the compiler and its tests, the cases run against the
  Microsoft compiler (`hostcheck.py`; everything since 2026-11-19 was checked with g++ only), and the build
  configurations dialog's three switch rows never looked at on a screen ([`CppCompilerLib/Cpp14CompliancePlan.txt`] C).
- The freestanding headers and then the standard library (the plan's D), waiting for the user's word.
- Clause 7's two cases and the semantic pass's list in `TestCPPConformanceChecked.h` (above).
- A non-template inline function in a system header is never written (bodies are skimmed).
- `__cplusplus` was to have a setting for `201703L` (2026-11-15); there is none.
- The declarations phase, after 2026-11-21's work, has small costs no one has chased (question 3 of that plan, the
  recommendation being to stop): [`CCPPFunctionDeclarer`] walking every function of a name rather than its class's
  (0.6% of a compile at most), the named-instance finder's two passes over the tokens, and the IDE check's
  [`CCPPUndeclaredTypeFinder`] walk, never measured.  Not explained: `FindMemberTemplate` asked 93,730 times where it
  was 64,822 on 14 November, and 576 quiet trials failing where 20 did.
- Exceptions: `std::throw_with_nested` doesn't tell a `final` class or one already a [`std::nested_exception`] from the
  others, `std::rethrow_if_nested` needs a polymorphic class; a call that only passes an exception on isn't a jump
  (stage 9d's rest), and `nothrow` isn't worked out across the whole program (9e).
- Small things noted from 2026-10-21 to 11-08 and never looked at again (the plan's E): `class X;` and `enum X;` leave
  a node out of the tree, an error in an uncalled template instance doesn't fail [`CCPPSemanticPass::Check`], `#define`
  with nothing after it has no position, and the parser doesn't look up `using Base<M>::x;`, names in `offsetof` or
  `goto` labels.
- Some comments are older than the code: [`CCPPAccess`]'s says the protected rule ([class.protected]) isn't checked,
  but [`CCPPAccess::CheckProtectedObject`] checks it; [`CCPPErrors`]'s names a "65C64 generator" stage that is gone.

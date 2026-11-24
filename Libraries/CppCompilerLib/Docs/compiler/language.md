# Supported C++

The compiler takes C++14 as far as the tables below go.  The preprocessor and tokeniser go
further, most of the way to C++17; the gaps are in the code generation.  How much C++ works is
measured by the project's own libraries: BaseLib, StandardLib and SupportLib compile for both
processors and their test suites run on the emulators, bit for bit against the same tests built
natively.

The preprocessor defines `__cplusplus` as `201402L`.

## Types

| Type | W65C816 | 65C64 | Notes |
|---|---|---|---|
| `char` | 1 byte | 1 byte | Signed, as MSVC. |
| `short` | 2 bytes | 2 bytes | |
| `int`, `unsigned` | **2 bytes** | 4 bytes | On the W65C816 `int` and `short` are the same size. |
| `long` | 4 bytes | 4 bytes | As on Windows. |
| `long long` | 8 bytes | 8 bytes | 8 byte aligned in classes on the 65C64, as MSVC (see [Alignment and layout](#alignment-and-layout)). |
| `float` | 4 bytes | 4 bytes | IEEE 754 single, in software. |
| `double`, `long double` | 8 bytes | 8 bytes | IEEE 754 double, in software.  `long double` is a double but its own type for overloads. |
| `bool` | 1 byte | 1 byte | |
| `wchar_t`, `char16_t` | 2 bytes | 2 bytes | Unsigned.  `L'x'` is a `wchar_t`, `u'x'` a `char16_t`. |
| `char32_t` | 4 bytes | 4 bytes | Unsigned.  `U'x'` is one. |
| Pointers, references | 4 bytes | 4 bytes | On the W65C816 a 24 bit address with a zero top byte. |
| `size_t` | **2 bytes** | 4 bytes | `unsigned int` on both, so `sizeof`'s type, `size_t` and BaseLib's `size` are one type.  On the W65C816 `memcpy`, `malloc` and the others take 16 bit counts; `memcpy32`, `malloc32` and the other 32 suffixed functions take longer ones (see [Libc65](libc65.md)). |
| `ptrdiff_t` | 4 bytes | 4 bytes | A pointer wide: `int` on the 65C64, `long` on the W65C816, so a difference of pointers into a block of more than 32KB isn't cut. |
| Pointers to data members | 4 bytes | 4 bytes | The field's offset plus one; the null one is 0. |
| Pointers to member functions | 8 bytes | 8 bytes | The function's address, then what the object's address is moved by for the function's `this`: 0 unless the pointer was converted to one of a class the function's class is further on in.  The null one is 0. |
| `std::nullptr_t` | 4 bytes | 4 bytes | `nullptr`'s type, a type of its own: it becomes any pointer or pointer to a member, a `bool` only where it is tested or directly initialises one, and nothing else. |

Each fundamental type is a type of its own, however wide: `int` and `long` are different types
on the 65C64 though both are 32 bits, as are `short` and `int` on the W65C816, and `char`,
`signed char` and `unsigned char` everywhere.  They overload, specialise templates and are
labelled apart, and promotions and the usual arithmetic conversions go by the standard's ranks
(`long + unsigned int` is `unsigned long` on the 65C64, `long` on the W65C816).  Libc65's fixed
width types are `int` where `int` is that wide: `int16_t` and `uint16_t` are `int` and
`unsigned int` on the W65C816, `int32_t` and `uint32_t` on the 65C64 (and `long` and
`unsigned long` on the W65C816).

An enum's underlying type follows the standard: `int`, then `unsigned int`, `long`,
`unsigned long` and `long long` as its values need, so on the W65C816 an enum with a value of
`0xFFFFFFFF` is 32 bits, and promotes to its underlying type: it is compared in 32 bits.

### Alignment and layout

Classes are laid out in the order MSVC lays them out: the bases, then the vtable pointer, then
the fields.  `#pragma pack` is followed, and `alignas` is kept whatever the pack is (on a class,
on a member, and on a member's or a base's class).

On the 65C64 each member is aligned to its size (8 bytes for `long long` and `double`, 4 for a
pointer), as MSVC aligns it, because BaseLib reads and writes classes as raw bytes and expects
the layout a PC gives them.  An object on its own is promised 4 bytes at most, which is what a
65C64 frame's places are multiples of: `alignof(double)` and `alignof(long long)` are 4, and so
is `alignof` of a class that holds one, though the class still has it at a multiple of 8.

The W65C816 has an 8 bit data bus, so a 16 or 32 bit value costs the same to read at an odd
address as at an even one, and nothing there is aligned past a byte unless `alignas` asks it:

- `alignof` is 1 for every type, `long`, `double`, pointers and classes too.  `alignas` still
  asks for more and gets it; `alignas(long)` is `alignof(long)`, which is 1.
- Globals, statics and locals are placed at any byte.
- In a class a member of 2 bytes or more (or a member class that holds one) is at an even
  offset, a `char` sized one anywhere, and a class with such a member is a whole number of 2
  bytes: `struct { char c; long l; }` is 6 bytes with `l` at 2 (8 bytes with `l` at 4 on the
  65C64).  The byte of padding after a `char` lets the pair be written as one 16 bit value.
  This is `#pragma pack(2)` for every class: `#pragma pack(1)` still packs to 1, and
  `#pragma pack(4)` or `(8)` change nothing.

So a class's size and its members' offsets are not the same on the two processors, or on a PC:
bytes shared with a program built by another compiler (a file's header, a device's registers, a
structure the emulator reads) should be made of members that need no padding anywhere, 4 byte
members at multiples of 4 and 2 byte ones at multiples of 2.

### A 16 bit `int`

On the W65C816 `int` is 16 bits; to get 32 bits, declare a `long` or an `int32_t`.  There is no
option to make `int` 32 bits.  Code written for a 32 bit `int` meets these traps, all silent:

- An `int` passed to an overload set with a 16 bit and a 32 bit member picks the 16 bit one.
  BaseLib's own overloads (`AssertInt`, `IntToString`, `WriteInt32` ...) have a 16 bit family
  under `__W65C816__`.
- `*(int*)` and `sizeof(int)` used for 4 byte data.
- `uint16_t` is `unsigned int`, so `u16 - 1` is 65535, not -1.
- `uint8_t << 16` and seeds, counts and sizes cast to `int`.
- Multi character literals (`'\0MAN'`) are `long`, and `NULL` passed to a variadic function
  must be pointer sized (`0L`).
- A virtual function's override that takes an `int` where the base class's takes an `int32` is
  a different function, which calls through the base class never reach.  Declare it `override`
  and the compiler says so (below).

The compiler warns about the constant cases: an *integer overflow* warning for a constant
expression whose value doesn't fit its type, and an *integer conversion overflow* warning for a
constant that an implicit conversion cuts short.

### `override` and `final`

A function declared `override` must override a virtual function of a base class, and no function
may override one declared `final`: both are errors.  A function that overrides one without being
declared `override` (or `final`) gets a warning, as G++'s `-Wsuggest-override` gives, except a
destructor.  The warning is on unless a build configuration turns it off
(`WarnMissingOverride="false"`, see [Building](building.md)).  A class declared `final` is no
base class.  A class written in a source file that has a base, or a member declared `override`
or `final`, is checked where it is defined, whether or not anything uses it.  A class in a header
is checked by the first unit that needs it whole (one that makes an object of it, calls a member
or defines one), not by every unit that includes the header; so is a class nested in another
class or in a function.  A header's class that a program names and never uses is not checked.

### BaseLib's `size` and `lsize`

BaseLib counts and measures with its own `size` and `lsize` types (`BaseLib/PrimitiveTypes.h`).
`size` is `size_t`; `lsize` is for what can pass 64KB.  A call doesn't say which function it wants:
on the W65C816 `memcpy`, `memmove`, `memset`, `memcmp`, `memchr`, `malloc`, `calloc`, `realloc`,
`fread` and `fwrite` called with an `lsize` count (or a `long`, or a constant made with `KB` or
`MB`) are Libc65's `memcpy32`, `malloc32`, `fread32` and the others, chosen by the count's type
by overloads in `PrimitiveTypes.h`; with a `size`, a `size_t` or a `sizeof` they are the C
library's own.
`Memory32.h` has the 32 suffixed names on every target for the few places that name them, and
`strlen32`, a string's whole length:

| Type | W65C816 | 65C64 | Notes |
|---|---|---|---|
| `size` | **2 bytes** | 4 bytes | `size_t` on every target: the W65C816's natural width there. |
| `lsize` | 4 bytes | 4 bytes | `uint32` on the W65C816, for counts and lengths that can pass 64KB; `size` elsewhere. |

The two are different types only on the W65C816, so only there can a function be overloaded on
both.  A host build checks the W65C816's size model with `g++ -m32 -DBASELIB_SIZE16
-Wconversion`, which warns wherever an `lsize`, or anything wider, is narrowed to a `size`.

## What is supported

| Area | Supported |
|---|---|
| Statements | Everything in C: `if`, `while`, `do`, `for`, `switch`, `break`, `continue`, `return`, `goto`.  Declarations in conditions (`if (int x = f())`, an object too, made again each time round a loop).  A `switch` over an object, converted by its one conversion function to an integer.  `asm("...")`: its text is assembled as a routine of its own, which the statement calls (it takes no operands). |
| Functions | Overloading, default arguments (a template instance's are worked out with its template arguments), recursion, variadic functions (`stdarg.h`), pointers to functions.  A function, or a member function, declared by the name of its type (`typedef int Binary(int, int); Binary Add;`). |
| Classes | Structs, classes and unions, named or not, local and anonymous; multiple inheritance with several polymorphic bases; virtual functions, pure virtuals, virtual destructors; `override` and `final`; constructors, destructors, member initialisers, default member initialisers, delegating constructors; `= default` (in the class or after it) and `= delete`; `const` and `volatile` member functions, which overload; static members; friends; nested classes; virtual base classes; a class's own `operator new` and `operator delete`. |
| Bit-fields | Laid out as MSVC lays them out: a bit-field shares a unit the size of its type, from the unit's lowest bit, and a new unit starts when the type's size changes, the bits run out, or after a zero width bit-field.  A unit is placed as a member of its type is (at an even offset at most on the W65C816).  Signed ones are sign extended when read; unnamed ones (`int : 3;`) are padding.  A bit-field has no address, but a `const` reference can bind to a copy of one.  `c ? x : y` of two bit-fields can be assigned to: it is the one picked. |
| Objects by value | Passed and returned by value; temporaries destroyed at the end of their full expression; results made in place (as C++17 requires). |
| Operators | Member and free operator functions, which compete for an operator as the standard says, and free ones for enums too; conversion functions; `operator->`, chained through the objects it returns; an object called through the pointer to a function it converts to; pointers to members (`.*` and `->*`).  A built-in operator on an object that no operator function takes is done on what the object's conversion function gives (`count + 1` for a class with `operator int`), when the class has one that fits. |
| Templates | Class, function and member templates; explicit and partial specialisation; non-type and default arguments; deduction from `T`, `T*`, `T&` and `X<..., T, N, ...>`; explicit instantiation (of a member function too); deduction through an alias template and of a template template parameter (`Get(C<T>&)`), from function and member pointer types (`R (*)(A)`, `M C::*`), from a braced list for `std::initializer_list<T>`, and of packs from a template's argument list (`F(Types<T...>, Ints<I...>)`); a function template's address, its arguments deduced from the type wanted; member pointers and `nullptr` as template arguments, and alias templates as template template arguments; names that don't depend on a template parameter found where the template is written, and a dependent base's members only through `this->` or qualified; partial specialisations of variable templates, and on function and member pointer types (`F<R(A...)>`, `F<R (C::*)(A...) const>`) and on types their parameters are only used in (`Has<T, Void<typename T::Type>>`); variadic templates (`sizeof...`, packs expanded in arguments, lists and base classes); template template parameters; alias templates; member class templates, constructor templates and variable templates that are static members; a class template's member classes, member class templates and member variable templates, each instance its own, defined in the class template or outside it (`template<class T> struct Outer<T>::Inner { };`), and their members defined outside too; SFINAE; a function template that is declared, called, and defined after. |
| Names | Namespaces (named, unnamed, inline), `using` declarations and directives, argument dependent lookup, `using Base::Base;` (inherited constructors), type aliases (`using Int = int;`). |
| C++11 and C++14 | `nullptr`, `enum class`, `static_assert`, `override`, `final`; `auto`, `decltype` (as a name's first qualifier too, `decltype(d)::T`), `decltype(auto)` and return types written after the function; lambdas, generic ones too (`auto...` as well, which can expand the pack of the function they are in beside their own), which capture by copy (arrays and parameter packs as well) or by reference, have default arguments, and convert to a pointer to a function when they capture nothing; `[[deprecated]]`, with what it says, on a function, a variable, a member, a class, an enumeration, an enumerator and a typedef: a warning where it is used; range-based `for`; rvalue references, move constructors and move assignment; `constexpr` functions and variables, worked out when the program is compiled; `typeid` (with `<typeinfo>`); `thread_local`; wide, Unicode and raw string literals; brace initialisation, with the braces of nested aggregates left out (`Nested n = { 1, 2, 3 };`). |
| Casts | C style, `static_cast`, `reinterpret_cast`, `const_cast`, functional casts, and `dynamic_cast<T*>`. |
| Memory | `new`, `new[]`, `delete`, `delete[]` and placement `new(address)`, over Libc65's `operator new`.  A class's own `operator new` and `delete`, and an `operator delete` that takes the size, which is given it (the global one where the size is known: for an object with a virtual destructor its own class's, whatever the pointer's; not for `delete[]` of elements with no destructor, which keeps no count). |
| Constants | Constant expressions with enumerators, `const` and `constexpr` variables (locals and their members too), casts, `sizeof` and type traits, calls of `constexpr` functions and literals with a `constexpr` literal operator (`int a[3_k]`), an object with a `constexpr` conversion to an integer, comparisons and differences of addresses; floating point constants worked out as the processor would.  A value that doesn't fit the type it is converted to as a constant is an error: `Byte<300>` for an `unsigned char` parameter, an enumerator past its enum's fixed type. |

### Extensions

| Extension | Meaning |
|---|---|
| `__builtin_clz`, `__builtin_ctz`, `__builtin_popcount` | And their `l` and `ll` forms. |
| `__builtin_expect`, `__builtin_unreachable` | |
| `__builtin_trap` | Stops the processor (`stp`). |
| `__builtin_alloca` | Memory from the stack, freed when the function returns. |
| `__builtin_memcpy` | A constant count of up to 64 bytes is copied in place. |
| `__builtin_huge_val`, `__builtin_inf`, `__builtin_nan` | Floating point constants. |
| `__builtin_bcd_add`, `__builtin_bcd_subtract` | Packed BCD arithmetic in the W65C816's decimal mode. |
| `__restrict` | On a global pointer or a pointer parameter: what it points at isn't reached any other way.  Lets loads leave loops. |
| `__attribute__((used))` | Keeps a global the program never names, so its constructor runs. |
| `__func__`, `__FUNCTION__`, `__PRETTY_FUNCTION__`, `__FUNCSIG__`, `__FUNCDNAME__` | String literals. |
| `#pragma pack` | As MSVC.  On the W65C816 it can only lower a member's alignment, which is 2 at most (see [Alignment and layout](#alignment-and-layout)). |

### Predefined macros

| Macro | Defined |
|---|---|
| `__W65C816__` | Compiling for the W65C816. |
| `__65C64__` | Compiling for the 65C64. |
| `__LIBC65__` | Either: the program is built against Libc65. |
| `__W65C816_ASSEMBLY__` | The W65C816 build uses the hand written assembly units beside their C++ (`stringW65C816.asm`, `softfloatW65C816.asm` ...). |

## What isn't supported yet

Most of these give an error that names the construct.  A function that uses C++ the back end
can't write gives *"This function uses C++ the W65C816 compiler doesn't support yet"* with the
compiler's own file and line, which says which construct it was.

| Not supported | Notes |
|---|---|
| Exceptions without the build switch | `try`, `catch` and `throw` are compiled only for a program built with `Exceptions="true"` (see [Exceptions](#exceptions)); without it they are errors. |
| `(decltype(f())::name) + 1` | Read as a cast, where `decltype`'s operand isn't the name of a variable or a parameter declared with a class's name: the parser can't tell what the member is.  `(decltype(d)::name) + 1` for such a `d` is the member's value. |
| A member template whose parameters are its class's pack and then its own | `template<class... B> static int Mix(A... a, B... b)` in `template<class... A> struct Pair`: the call isn't matched. |
| A partial specialisation, outside the class, of a class template's member variable template | The template itself, defined in the class or outside it, is each instance's own. |
| A union's inactive member in a constant | Reading the member that wasn't the last given a value is refused only where the two differ in size. |
| The C++ standard library | Libc65 has the C library and `<new>`, `<exception>` and `<typeinfo>` only. |

A member's name that two base classes each declare is ambiguous wherever it is used
(`c.v`, `c.F()`, `C::Type`), unless one of them hides the other's or the name says whose
(`c.A::v`).  Default arguments are a template's first declaration's: a later declaration of a
function template, a definition outside the class of a template's member, and an explicit
specialisation can add none.

What a declaration alone shows to be wrong is refused wherever it is written.  The classes
defined in the unit's own `.cpp` file are laid out, and the functions defined there that nothing
calls are checked as if they were written, so what is wrong in them is refused too.  In a header
the checks that need a class laid out, or a function's body, are made where it is first used, so
a library's unused inline functions cost nothing.

Overload resolution ranks each argument's conversion as the standard does (exact match,
promotion, conversion, user defined conversion, ellipsis), with its tie-breaks: a nearer base, a
class pointer before `void*`, less `const` and `volatile`, `&&` for what is going, a braced list's
`std::initializer_list`.  A function template's instance competes with the other functions, and of
two templates the more specialised is chosen; where none is best the call is ambiguous.

## Exceptions

`throw`, `try` and `catch` are compiled for a program built with `Exceptions="true"` (or
`-exceptions`; see [Building](building.md)).  The option is off by default: without it they are
errors, and not a byte of any program pays for them.  With it, all of C++14's exceptions are there:
handlers by value, by reference and `(...)`, `throw;`, function-try-blocks, `noexcept` and
`throw(...)` lists with `std::unexpected`, what is half made undone (a constructor's members and
bases, an array's elements, `new`'s memory, a static local's guard), and what the language throws:
`std::bad_alloc` from `new`, `std::bad_array_new_length`, `std::bad_cast` and `std::bad_typeid`.
Libc65's `<exception>` has `std::exception_ptr` and `std::nested_exception`.

A function that throws returns as it always does, with the carry flag set, and its caller tests the
flag after the call.  So nothing walks the stack and no tables are written, a throw is fast, and the
debugger steps through it; the cost is on the normal path, a test and a branch after each call that
may throw, and the code that undoes what a function made, which only an exception runs.  A function
declared `noexcept` costs its callers nothing.

What turning the option on costs, in the Lib65 test suites (the same programs built with it on
against off, and the kernal as a one-off):

| Programs | Processor | Bytes | Cycles |
|---|---|---|---|
| BaseLib's tests | 65C64 | +3.9% | +2.3% |
| BaseLib's tests | W65C816 | +0.6% | +1.1% |
| StandardLib's tests | 65C64 | +5.9% | +2.4% |
| StandardLib's tests | W65C816 | +1.0% | +1.7% |
| SupportLib's tests | 65C64 | +6.1% | +1.9% |
| SupportLib's tests | W65C816 | +1.0% | +0.4% |
| The kernal (`OS.SYS`) | W65C816 | +2.8% | |

What a program built with exceptions must do:

- Declare `noexcept` every function written in assembly that C++ calls: assembly leaves the carry
  as it falls, which a tested call would take for an exception.  The build says which ones aren't
  (*"f is assembly and is called by C++ that tests for an exception: declare it noexcept"*).
  Libc65's, BaseLib's and the kernal's are.
- Not let an exception leave an interrupt handler or a kernal call: nothing catches it there, and it
  is `std::terminate`.

Not there yet: `std::throw_with_nested` takes a class and doesn't tell a `final` one, or one that is
already a `std::nested_exception`, from the others; `std::rethrow_if_nested` takes a polymorphic
class.

## Floating point

Neither processor has floating point instructions, so `float` and `double` are done by Libc65's
soft float routines, hand written in assembly for each processor.  Rounding is to nearest, ties
to even, with subnormals, infinities and NaNs; NaNs behave as SSE's do, so results match a
Windows build bit for bit.  [Libc65](libc65.md#floating-point) has the details.

A build can refuse floating point altogether (`NoFloats`): any `float` or `double` is then an
error.  The HCP65 kernal is built this way.

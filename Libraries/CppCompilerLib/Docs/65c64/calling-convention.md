# Calling Convention

The processor has no calling convention of its own: [`jsr`](instructions/jump.md) pushes a
return address and [`ret`](instructions/return.md) pulls it.  This page is the convention the
C++ compiler's 65C64 back end uses, which the runtime library (Libc65) and hand written assembly
that is called from C++ follow too.

## Registers

| Register | Use | Across a call |
|---|---|---|
| A | Expression values, the first register argument, and the result of a function | May change |
| B | The right operand of an operator, and the second register argument | May change |
| X | Addresses, and `this` in a call to a member function | May change |
| Y | Scratch, and the third register argument | May change |
| V | The frame pointer | Kept |
| W | Scratch, and the step count of `mul` and `div` | May change |
| S | The stack pointer | Kept |

A, B, X and Y are 32 bits wide at every call and return.  Code that changes a width sets it
back.  Values of 8 or 16 bits are read and written with [`cpyx`](instructions/cpyx.md), which
extends them into the 32 bit register.

## W

W is a register like any other: compiled code keeps values in it, and a call may change it.
Don't count on it being 0 at a call or a return.

`mul` and `div` count their steps in W from 0, so compiled code sets it to 0 just before one
(and leaves that out when W is known to be 0 already):

```asm
	cpy W, 0
	mul
```

## Pseudo registers

The first 256 bytes of memory, `0000:0000` to `0000:00FF`, are the *pseudo registers*: 32 bit
places that the compiler uses like extra registers, as a 6502 program uses the zero page.  They
are reached with one word [offset addressing](operands.md#address-offset): slower than a
register, but quicker to reach than the frame.

| Pseudo registers | Across a call |
|---|---|
| `[$00]` to `[$7F]` | May change |
| `[$80]` to `[$FF]` | Kept: a function that uses one pushes it first and pulls it before it returns |

The runtime routines use no pseudo registers.

## Arguments and results

- A function that isn't variadic takes its first arguments in registers: see
  [Register arguments](#register-arguments).  The others are pushed.
- Arguments are pushed on the stack, the last one first, so the first argument is at the lowest
  address.
- Each argument is pushed at its own size: a `char` is one byte and a `short` two.  They
  aren't aligned.  An argument is converted to its parameter's type before it's pushed.
- In a call to a member function, `this` is passed in X.
- The caller removes the arguments after the call, with `add S, bytes`.
- The result is returned in A: all 64 bits of it for a `long long`.
- An object is returned through memory.  The caller passes the address of a temporary to
  construct it in.
- The extra arguments of a variadic function are pushed like the others, after the default
  argument promotions (`char`, `short` and `bool` become `int`).

```asm
	.org $2000
	psh 3:32		; the second argument
	psh 2:32		; the first argument
	jsr Add			; Add(2, 3)
	add S, 8		; remove the arguments
	stp

Add:
	cpy A, [5 + S]		; the first argument, above the return address
	cpy B, [9 + S]		; the second
	add A, B
	ret			; the result is in A
```

The stack is post-decrement, so S points at the byte below the last value pushed.  On entry
the return address is at `[1 + S]` and the first argument at `[5 + S]`.

`Add` is called at its plain label with everything pushed, as assembly calls it.  C++ calls a
function like it with its arguments in registers, at the label below.

### Register arguments

A function that isn't variadic, and has no parameter that is an object passed by value, takes
its first arguments in registers, whether it has C++ or C linkage:

- Going through the arguments in order (a hidden result pointer first), each of 32 bits or less
  takes the next of A, B and Y.  A `long long` or `double` is pushed, and a later, smaller
  argument may still take a register.  `this` is in X as before.
- The function's code is at its label plus a **suffix** naming where each argument is: a dot,
  then `a`, `b` or `y` for a register or `s` for a pushed argument, without the pushed ones at
  the end.  `_Add__Fii.ab` takes two `int`s in A and B.  A caller and a definition that
  disagree don't link.
- The plain label is an entry for callers that push everything (function pointers, vtables,
  thunks and assembly): it loads the registers from the stack and goes on to the body.

Every caller works out the same rule from the parameter types alone.  Hand written assembly
that C++ calls defines the suffixed label too.

## Frames

A function with local variables keeps the caller's V on the stack and points V at its own
frame:

```asm
	.org $2000
Function:
	psh V
	sub S, 16		; the frame: 16 bytes of locals
	cpy V, S
	cpy A, [1 + 16 + 8 + V]	; the first argument
	cpy [1 + V], A		; the first local
	add S, 16
	pul V
	ret
```

| Place | What |
|---|---|
| `[1 + n + V]` | The local n bytes into the frame |
| `[1 + frame + V]` | The caller's V |
| `[1 + frame + 4 + V]` | The return address |
| `[1 + frame + 8 + n + V]` | The argument n bytes after the first |

The frame is the locals, each aligned to its size (4 bytes at most), rounded up to 4 bytes.
Places from V don't move when values are pushed, which places from S do.

A function with nothing in its frame doesn't set V up.  It reads its arguments from S, as `Add`
does above.

## Names

C++ names are labels with an underscore in front: `main` is `_main` and a global `gTable` is
`_gTable`.

| C++ | Label |
|---|---|
| A function | `_name__F` and its parameters' types: `Add(int, int)` is `_Add__Fii` |
| A member | `_Class__name` |
| A constructor | `_Class__0` |
| A destructor | `_Class__1` |
| A vtable | `_Class__2` |

The runtime's own routines start with `R_`.

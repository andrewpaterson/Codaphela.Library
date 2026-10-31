; IEEE 754 single and double precision for the W65C816.  The C++ back end calls these for float
; and double arithmetic, comparisons and conversions; they are softfloat.cpp's functions, which
; stays as their reference (the W65C816 build of it is empty).  The results are the same bit for
; bit: rounding to nearest, ties to even, subnormals, infinities, and NaNs as SSE has them (an
; invalid operation gives the default NaN $FFC00000 or $FFF8000000000000, a NaN operand comes back
; quietened, the first if both are NaNs, and an integer conversion that is out of range or a NaN
; gives the most negative integer).  The code follows softfloat.cpp's steps where it can, with
; its 64 bit integers as four words in the direct page: a value is unpacked to a sign, a class,
; the exponent of its significand's lowest bit and the significand.  Results are rounded by
; RoundPack, or by RoundDoubleAt and RoundFloat31 when the top bit's place is known.  Multiply and
; divide use Garth Wilson's tables (MathTbls): byte products from MathMult and the reciprocals of
; 16 bit numbers from MathInvert, read through 24 bit pointers to a byte's row of products.
;
; The calling convention is the compiler's (IR816Backend.h): arguments are pushed last first (32
; bits as two words, 64 as four), A and X are 16 bits wide on the way in and out, a 32 bit result
; is in A (low) and X (high), an int (16 bits) in A, and a 64 bit result is written through a
; hidden pointer the caller pushes after the arguments (the first parameter).  D, the data bank and
; the widths are kept; A, X and Y change.  Each function makes at least Locals bytes of room on
; the stack and points D at them, on a page so each direct page access is a cycle faster (up to
; 255 bytes more stack):
;	D + 1 ...			the locals below
;	SavedS + 1			the caller's D
;	SavedS + 3			the return address
;	D + ArgOff ...		the arguments (the hidden pointer first)

; The two operands, unpacked: Sign ($8000 or 0), Class, Exponent (the power of two of the
; significand's lowest bit, signed) and the 64 bit significand.  Side B is 14 bytes after side A,
; so the routines given X = 0 or SideB work on either.
SignA		= 1
ClassA		= 3
ExpA		= 5
SigA		= 7
SignB		= 15
ClassB		= 17
ExpB		= 19
SigB		= 21
SideB		= 14
Product		= 29		; 128 bits: a product; a divide's remainder window
Result		= 45		; 64 bits: the bits returned
RawA		= 53		; the operands' bits as given (RawB is SideB after RawA)
Fraction	= 61		; the format: 23 or 52 fraction bits
RawB		= 67
SqrtT		= 75		; 64 bits: the square root's T (the root << 2)
SqrtRest	= 83		; 64 bits: the square root's remainder
Count		= 107		; a shift's count; a divide's q; the square root's next radicand word
Sticky		= 109		; not 0 when a shift right lost a 1 bit
TopBit		= 111
Biased		= 113
Shift		= 115
Temp		= 117
Temp2		= 119
IndexI		= 121
Subtract	= 125		; not 0 for a subtraction; ToInteger's signed flag
Return		= 127		; how the result is returned: 0 32 bits, 1 64 bits, 2 16 bits
Bytes		= 129		; ToInteger's width in bits
SavedS		= 131		; S on the way in (after phd)
ArgOff		= 133		; the arguments' offset from D (they are 6 bytes above SavedS)
ResultPtr	= 135		; the hidden result pointer, copied to the direct page to write through
Ptr1		= 138		; divide: MathMult + r1 * 512, the row of products of the reciprocal's middle byte
BPtr		= 142		; multiply and divide: MathMult + b * 512 for each byte b of SigB (7 24 bit pointers)
Locals		= 170

SoftZero	= 0
SoftFinite	= 1
SoftInfinity	= 2
SoftNaN		= 3


		.section text.SoftFloat816
		.relax
		.global _SoftFloatAdd, _SoftFloatSubtract, _SoftFloatMultiply, _SoftFloatDivide, _SoftFloatCompare
		.global _SoftFloatFromInt, _SoftFloatFromUnsigned, _SoftFloatFromLongLong, _SoftFloatFromUnsignedLongLong
		.global _SoftFloatToInt, _SoftFloatToUnsigned, _SoftFloatToLongLong, _SoftFloatToUnsignedLongLong
		.global _SoftFloatToDouble, _SoftFloatSqrt, _SoftDoubleToFloat
		.global _SoftDoubleAdd, _SoftDoubleSubtract, _SoftDoubleMultiply, _SoftDoubleDivide, _SoftDoubleCompare
		.global _SoftDoubleFromInt, _SoftDoubleFromUnsigned, _SoftDoubleFromLongLong, _SoftDoubleFromUnsignedLongLong
		.global _SoftDoubleToInt, _SoftDoubleToUnsigned, _SoftDoubleToLongLong, _SoftDoubleToUnsignedLongLong
		.global _SoftDoubleSqrt


;-----------------------------------------------------------------------------------------------
; The functions.  Each makes its frame, says the format, how it returns and where its arguments
; are, then goes to the operation.

; uint32_t SoftFloatAdd(uint32_t uiA, uint32_t uiB)
_SoftFloatAdd:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		stz [D + Subtract]
		jsr SetFloat32
		jsr LoadFloats
		jmp AddOp

; uint32_t SoftFloatSubtract(uint32_t uiA, uint32_t uiB)
_SoftFloatSubtract:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		lda 1
		sta [D + Subtract]
		jsr SetFloat32
		jsr LoadFloats
		jmp AddOp

; uint32_t SoftFloatMultiply(uint32_t uiA, uint32_t uiB)
_SoftFloatMultiply:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetFloat32
		jsr LoadFloats
		jmp MultiplyOp

; uint32_t SoftFloatDivide(uint32_t uiA, uint32_t uiB)
_SoftFloatDivide:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetFloat32
		jsr LoadFloats
		jmp DivideOp

; int SoftFloatCompare(uint32_t uiA, uint32_t uiB)
_SoftFloatCompare:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetFloat32
		lda 2
		sta [D + Return]
		jsr LoadFloats
		jmp CompareOp

; uint32_t SoftFloatSqrt(uint32_t ui)
; float sqrtf(float x) too.
_SoftFloatSqrt:
_sqrtf:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetFloat32
		ldx 0
		jsr LoadFloatA
		jmp SqrtOp

; uint32_t SoftFloatFromInt(int32_t i)
_SoftFloatFromInt:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetFloat32
		ldx 0
		jmp FromInt32Op

; uint32_t SoftFloatFromUnsigned(uint32_t ui)
_SoftFloatFromUnsigned:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetFloat32
		ldx 0
		jmp FromUnsigned32Op

; uint32_t SoftFloatFromLongLong(int64_t lli)
_SoftFloatFromLongLong:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetFloat32
		ldx 0
		jmp FromInt64Op

; uint32_t SoftFloatFromUnsignedLongLong(uint64_t ulli)
_SoftFloatFromUnsignedLongLong:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetFloat32
		ldx 0
		jmp FromUnsigned64Op

; int32_t SoftFloatToInt(uint32_t ui)
_SoftFloatToInt:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetFloat32
		ldx 0
		jsr LoadFloatA
		lda 32
		ldx 1
		jmp ToIntegerOp

; uint32_t SoftFloatToUnsigned(uint32_t ui)
_SoftFloatToUnsigned:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetFloat32
		ldx 0
		jsr LoadFloatA
		lda 32
		ldx 0
		jmp ToIntegerOp

; int64_t SoftFloatToLongLong(uint32_t ui)
_SoftFloatToLongLong:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetFloat64
		ldx 4
		jsr LoadFloatA
		jsr SetFormatFloat
		lda 64
		ldx 1
		jmp ToIntegerOp

; uint64_t SoftFloatToUnsignedLongLong(uint32_t ui)
_SoftFloatToUnsignedLongLong:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetFloat64
		ldx 4
		jsr LoadFloatA
		jsr SetFormatFloat
		lda 64
		ldx 0
		jmp ToIntegerOp

; uint64_t SoftFloatToDouble(uint32_t ui)
_SoftFloatToDouble:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetFloat64
		ldx 4
		jsr LoadFloatA
		jsr SetFormatFloat
		jmp FloatToDoubleOp

; uint32_t SoftDoubleToFloat(uint64_t ulli)
_SoftDoubleToFloat:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetDouble32
		ldx 0
		jsr LoadDoubleA
		jmp DoubleToFloatOp

; uint64_t SoftDoubleAdd(uint64_t ulliA, uint64_t ulliB)
_SoftDoubleAdd:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		stz [D + Subtract]
		jsr SetDouble64
		ldx 4
		jsr LoadDoubles
		jmp AddOp

; uint64_t SoftDoubleSubtract(uint64_t ulliA, uint64_t ulliB)
_SoftDoubleSubtract:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		lda 1
		sta [D + Subtract]
		jsr SetDouble64
		ldx 4
		jsr LoadDoubles
		jmp AddOp

; uint64_t SoftDoubleMultiply(uint64_t ulliA, uint64_t ulliB)
_SoftDoubleMultiply:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetDouble64
		ldx 4
		jsr LoadDoubles
		jmp MultiplyOp

; uint64_t SoftDoubleDivide(uint64_t ulliA, uint64_t ulliB)
_SoftDoubleDivide:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetDouble64
		ldx 4
		jsr LoadDoubles
		jmp DivideOp

; int SoftDoubleCompare(uint64_t ulliA, uint64_t ulliB)
_SoftDoubleCompare:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetDouble64
		lda 2
		sta [D + Return]
		ldx 0
		jsr LoadDoubles
		jmp CompareOp

; uint64_t SoftDoubleSqrt(uint64_t ulli)
; double sqrt(double x), long double sqrtl(long double x) too.
_SoftDoubleSqrt:
_sqrt:
_sqrtl:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetDouble64
		ldx 4
		jsr LoadDoubleA
		jmp SqrtOp

; uint64_t SoftDoubleFromInt(int32_t i)
_SoftDoubleFromInt:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetDouble64
		ldx 4
		jmp FromInt32Op

; uint64_t SoftDoubleFromUnsigned(uint32_t ui)
_SoftDoubleFromUnsigned:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetDouble64
		ldx 4
		jmp FromUnsigned32Op

; uint64_t SoftDoubleFromLongLong(int64_t lli)
_SoftDoubleFromLongLong:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetDouble64
		ldx 4
		jmp FromInt64Op

; uint64_t SoftDoubleFromUnsignedLongLong(uint64_t ulli)
_SoftDoubleFromUnsignedLongLong:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetDouble64
		ldx 4
		jmp FromUnsigned64Op

; int32_t SoftDoubleToInt(uint64_t ulli)
_SoftDoubleToInt:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetDouble32
		ldx 0
		jsr LoadDoubleA
		lda 32
		ldx 1
		jmp ToIntegerOp

; uint32_t SoftDoubleToUnsigned(uint64_t ulli)
_SoftDoubleToUnsigned:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetDouble32
		ldx 0
		jsr LoadDoubleA
		lda 32
		ldx 0
		jmp ToIntegerOp

; int64_t SoftDoubleToLongLong(uint64_t ulli)
_SoftDoubleToLongLong:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetDouble64
		ldx 4
		jsr LoadDoubleA
		lda 64
		ldx 1
		jmp ToIntegerOp

; uint64_t SoftDoubleToUnsignedLongLong(uint64_t ulli)
_SoftDoubleToUnsignedLongLong:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetDouble64
		ldx 4
		jsr LoadDoubleA
		lda 64
		ldx 0
		jmp ToIntegerOp


;-----------------------------------------------------------------------------------------------
; The format and how the result is returned.

; Float, returned in A and X.
SetFloat32:
		stz [D + Return]
SetFormatFloat:
		lda 23
		sta [D + Fraction]
		rts

; Double, returned through the hidden pointer.
SetDouble64:
		lda 1
		sta [D + Return]
SetFormatDouble:
		lda 52
		sta [D + Fraction]
		rts

; Double, returned in A and X (a float or a 32 bit integer from a double).
SetDouble32:
		stz [D + Return]
		bra SetFormatDouble

; Float, returned through the hidden pointer (a double or a 64 bit integer from a float).
SetFloat64:
		lda 1
		sta [D + Return]
		bra SetFormatFloat


;-----------------------------------------------------------------------------------------------
; The arguments into RawA and RawB, X bytes after the hidden pointer's place.  A float is its
; two words only: nothing reads a float's Raw + 4 and + 6.

LoadFloats:
		ldx [D + ArgOff]
		lda [D + 4 + X]
		sta [D + RawB]
		lda [D + 6 + X]
		sta [D + RawB + 2]
		bra LoadFloatAt
LoadFloatA:
		jsr ArgIndex
LoadFloatAt:
		lda [D + X]
		sta [D + RawA]
		lda [D + 2 + X]
		sta [D + RawA + 2]
		rts

LoadDoubles:
		jsr ArgIndex
		lda [D + 8 + X]
		sta [D + RawB]
		lda [D + 10 + X]
		sta [D + RawB + 2]
		lda [D + 12 + X]
		sta [D + RawB + 4]
		lda [D + 14 + X]
		sta [D + RawB + 6]
		bra LoadDoubleAt
LoadDoubleA:
		jsr ArgIndex
LoadDoubleAt:
		lda [D + X]
		sta [D + RawA]
		lda [D + 2 + X]
		sta [D + RawA + 2]
		lda [D + 4 + X]
		sta [D + RawA + 4]
		lda [D + 6 + X]
		sta [D + RawA + 6]
		rts

; X = X + ArgOff: the argument X bytes after the hidden pointer's place, as an offset from D.
ArgIndex:
		txa
		clc
		adc [D + ArgOff]
		tax
		rts

; Keeps S (in X, after phd) and where the arguments are.  D is on a page now.
Framed:
		stx [D + SavedS]
		tdc
		sta [D + Temp]
		txa
		sec
		sbc [D + Temp]
		clc
		adc 6
		sta [D + ArgOff]
		rts


;-----------------------------------------------------------------------------------------------
; Returning the result: Result's low 32 bits in A and X, its 64 bits through the hidden pointer,
; or its low 16 bits in A.

ReturnResult:
		lda [D + Return]
		beq Return32
		cmp 1
		beq Return64
		ldy [D + Result]
		lda [D + SavedS]
		tcs
		tya
		pld
		rtl

Return32:
		ldx [D + Result + 2]
		ldy [D + Result]
		lda [D + SavedS]
		tcs
		tya
		pld
		rtl

Return64:
		ldx [D + ArgOff]
		lda [D + X]
		sta [D + ResultPtr]
		lda [D + 2 + X]
		sta [D + ResultPtr + 2]
		ldy 0
		lda [D + Result]
		sta [[D + ResultPtr]:24 + Y]
		ldy 2
		lda [D + Result + 2]
		sta [[D + ResultPtr]:24 + Y]
		ldy 4
		lda [D + Result + 4]
		sta [[D + ResultPtr]:24 + Y]
		ldy 6
		lda [D + Result + 6]
		sta [[D + ResultPtr]:24 + Y]
		lda [D + SavedS]
		tcs
		pld
		rtl


;-----------------------------------------------------------------------------------------------
; Special results, in Result.

; Result = RawA (X = 0) or RawB (X = SideB).
ResultRaw:
		lda [D + RawA + X]
		sta [D + Result]
		lda [D + RawA + 2 + X]
		sta [D + Result + 2]
		lda [D + RawA + 4 + X]
		sta [D + Result + 4]
		lda [D + RawA + 6 + X]
		sta [D + Result + 6]
		rts

; The quiet NaN of the first NaN operand.
PropagateNaN:
		ldx 0
		lda [D + ClassA]
		cmp SoftNaN
		beq PropagateA
		ldx SideB
PropagateA:
		jsr ResultRaw
Quieten:
		lda [D + Fraction]
		cmp 23
		beq QuietenFloat
		lda [D + Result + 6]
		ora $0008
		sta [D + Result + 6]
		rts
QuietenFloat:
		lda [D + Result + 2]
		ora $0040
		sta [D + Result + 2]
		rts

; The default NaN: sign set, quiet.
DefaultNaN:
		jsr ClearResult
		lda [D + Fraction]
		cmp 23
		beq DefaultNaNFloat
		lda $FFF8
		sta [D + Result + 6]
		rts
DefaultNaNFloat:
		lda $FFC0
		sta [D + Result + 2]
		rts

; An infinity with SignA's sign.
Infinity:
		jsr ClearResult
		lda [D + Fraction]
		cmp 23
		beq InfinityFloat
		lda $7FF0
		ora [D + SignA]
		sta [D + Result + 6]
		rts
InfinityFloat:
		lda $7F80
		ora [D + SignA]
		sta [D + Result + 2]
		rts

; A zero with SignA's sign.
SignedZero:
		jsr ClearResult
PutSign:
		lda [D + Fraction]
		cmp 23
		beq PutSignFloat
		lda [D + Result + 6]
		ora [D + SignA]
		sta [D + Result + 6]
		rts
PutSignFloat:
		lda [D + Result + 2]
		ora [D + SignA]
		sta [D + Result + 2]
		rts

ClearResult:
		stz [D + Result]
		stz [D + Result + 2]
		stz [D + Result + 4]
		stz [D + Result + 6]
		rts


;-----------------------------------------------------------------------------------------------
; Unpack: RawA (X = 0) or RawB (X = SideB) into its side's Sign, Class, Exponent and
; significand, as softfloat.cpp's Unpack.

Unpack:
		lda [D + Fraction]
		cmp 23
		beq UnpackFloat
		lda [D + RawA + X]
		sta [D + SigA + X]
		lda [D + RawA + 2 + X]
		sta [D + SigA + 2 + X]
		lda [D + RawA + 4 + X]
		sta [D + SigA + 4 + X]
		; The top word (kept in Y): sign, exponent, the fraction's top four bits.
		lda [D + RawA + 6 + X]
		tay
		and $8000
		sta [D + SignA + X]
		tya
		and $7FF0
		beq UnpackDoubleSmall
		cmp $7FF0
		beq UnpackDoubleTop
		lsr A
		lsr A
		lsr A
		lsr A
		sec
		sbc 1075
		sta [D + ExpA + X]
		tya
		and $000F
		ora $0010
		sta [D + SigA + 6 + X]
UnpackFinite:
		lda SoftFinite
		sta [D + ClassA + X]
		rts
UnpackDoubleSmall:
		stz [D + ExpA + X]
		tya
		and $000F
		sta [D + SigA + 6 + X]
		jsr UnpackFractionZero
		beq UnpackZero
		lda -1074
		sta [D + ExpA + X]
		bra UnpackFinite
UnpackDoubleTop:
		lda 2047
		sta [D + ExpA + X]
		tya
		and $000F
		sta [D + SigA + 6 + X]
		bra UnpackTop

UnpackFloat:
		; Two words: the significand's top two are 0.
		lda [D + RawA + X]
		sta [D + SigA + X]
		stz [D + SigA + 4 + X]
		stz [D + SigA + 6 + X]
		lda [D + RawA + 2 + X]
		and $8000
		sta [D + SignA + X]
		lda [D + RawA + 2 + X]
		and $7F80
		asl A
		xba
		and $00FF
		sta [D + ExpA + X]
		lda [D + RawA + 2 + X]
		and $007F
		sta [D + SigA + 2 + X]
		lda [D + ExpA + X]
		beq UnpackFloatSmall
		cmp 255
		beq UnpackTop
		lda [D + SigA + 2 + X]
		ora $0080
		sta [D + SigA + 2 + X]
		lda [D + ExpA + X]
		sec
		sbc 150
		sta [D + ExpA + X]
		bra UnpackFinite
UnpackFloatSmall:
		jsr UnpackFractionZero
		beq UnpackZero
		lda -149
		sta [D + ExpA + X]
		bra UnpackFinite

UnpackZero:
		lda SoftZero
		sta [D + ClassA + X]
		rts

UnpackTop:
		jsr UnpackFractionZero
		beq UnpackInfinity
		lda SoftNaN
		sta [D + ClassA + X]
		rts
UnpackInfinity:
		lda SoftInfinity
		sta [D + ClassA + X]
		rts

; Z set if the significand (the fraction, unpacked so far) is 0.
UnpackFractionZero:
		lda [D + SigA + X]
		ora [D + SigA + 2 + X]
		ora [D + SigA + 4 + X]
		ora [D + SigA + 6 + X]
		rts


;-----------------------------------------------------------------------------------------------
; The 64 bit value at D + X.

; Shifts it left A bits (0 to 63): whole words, then a byte (a word read a byte along is the
; bytes moved 8 bits), then up to 7 bits.
ShiftLeft:
		sta [D + Count]
ShiftLeftWords:
		lda [D + Count]
		cmp 16
		bcc ShiftLeftByte
		lda [D + 4 + X]
		sta [D + 6 + X]
		lda [D + 2 + X]
		sta [D + 4 + X]
		lda [D + X]
		sta [D + 2 + X]
		stz [D + X]
		lda [D + Count]
		sec
		sbc 16
		sta [D + Count]
		bra ShiftLeftWords
ShiftLeftByte:
		cmp 8
		bcc ShiftLeftBits
		lda [D + 5 + X]
		sta [D + 6 + X]
		lda [D + 3 + X]
		sta [D + 4 + X]
		lda [D + 1 + X]
		sta [D + 2 + X]
		lda [D + X]
		xba
		and $FF00
		sta [D + X]
		lda [D + Count]
		and 7
ShiftLeftBits:
		tay
		beq ShiftLeftDone
ShiftLeftLoop:
		asl [D + X]
		rol [D + 2 + X]
		rol [D + 4 + X]
		rol [D + 6 + X]
		dey
		bne ShiftLeftLoop
ShiftLeftDone:
		rts

; Shifts it right A bits (any count; 64 or more leaves 0), Sticky not 0 if a 1 bit went.
ShiftRight:
		stz [D + Sticky]
		cmp 64
		bcc ShiftRightSome
		lda [D + X]
		ora [D + 2 + X]
		ora [D + 4 + X]
		ora [D + 6 + X]
		sta [D + Sticky]
		stz [D + X]
		stz [D + 2 + X]
		stz [D + 4 + X]
		stz [D + 6 + X]
		rts
ShiftRightSome:
		sta [D + Count]
ShiftRightWords:
		lda [D + Count]
		cmp 16
		bcc ShiftRightByte
		lda [D + X]
		ora [D + Sticky]
		sta [D + Sticky]
		lda [D + 2 + X]
		sta [D + X]
		lda [D + 4 + X]
		sta [D + 2 + X]
		lda [D + 6 + X]
		sta [D + 4 + X]
		stz [D + 6 + X]
		lda [D + Count]
		sec
		sbc 16
		sta [D + Count]
		bra ShiftRightWords
ShiftRightByte:
		cmp 8
		bcc ShiftRightBits
		lda [D + X]
		and $00FF
		ora [D + Sticky]
		sta [D + Sticky]
		lda [D + 1 + X]
		sta [D + X]
		lda [D + 3 + X]
		sta [D + 2 + X]
		lda [D + 5 + X]
		sta [D + 4 + X]
		lda [D + 7 + X]
		and $00FF
		sta [D + 6 + X]
		lda [D + Count]
		and 7
ShiftRightBits:
		tay
		beq ShiftRightDone
ShiftRightLoop:
		lsr [D + 6 + X]
		ror [D + 4 + X]
		ror [D + 2 + X]
		ror [D + X]
		bcc ShiftRightNext
		lda 1
		tsb [D + Sticky]
ShiftRightNext:
		dey
		bne ShiftRightLoop
ShiftRightDone:
		rts

; Shifts it right A bits, any bit shifted out kept in the lowest bit (softfloat.cpp's
; ShiftRightJam).
ShiftRightJam:
		jsr ShiftRight
		lda [D + Sticky]
		beq ShiftRightJamDone
		lda [D + X]
		ora 1
		sta [D + X]
ShiftRightJamDone:
		rts

; A = the index of its highest 1 bit (it isn't 0).
HighestBit:
		ldy 48
		lda [D + 6 + X]
		bne HighestBitWord
		ldy 32
		lda [D + 4 + X]
		bne HighestBitWord
		ldy 16
		lda [D + 2 + X]
		bne HighestBitWord
		ldy 0
		lda [D + X]
HighestBitWord:
		sty [D + Temp2]
		ldy 15
		cmp $0100
		bcs HighestBitLoop
		xba
		ldy 7
HighestBitLoop:
		asl A
		bcs HighestBitFound
		dey
		bra HighestBitLoop
HighestBitFound:
		tya
		clc
		adc [D + Temp2]
		rts

; Z set if it is 0.
IsZero:
		lda [D + X]
		ora [D + 2 + X]
		ora [D + 4 + X]
		ora [D + 6 + X]
		rts


;-----------------------------------------------------------------------------------------------
; RoundPack: SignA, ExpA and SigA rounded to the format in Result, as softfloat.cpp's RoundPack.

RoundPack:
		ldx SigA
		jsr IsZero
		bne RoundPackNotZero
		jmp SignedZero
RoundPackNotZero:
		jsr HighestBit
		sta [D + TopBit]
		clc
		adc [D + ExpA]
		ldy [D + Fraction]
		cpy 23
		beq RoundPackBiasFloat
		clc
		adc 1023
		sta [D + Biased]
		sec
		sbc 2047
		bra RoundPackBiased
RoundPackBiasFloat:
		clc
		adc 127
		sta [D + Biased]
		sec
		sbc 255
RoundPackBiased:
		bvs RoundPackOverflowed
		bpl RoundPackInfinity
		bra RoundPackInRange
RoundPackOverflowed:
		bmi RoundPackInfinity
RoundPackInRange:
		lda [D + TopBit]
		sec
		sbc [D + Fraction]
		sta [D + Shift]
		lda [D + Biased]
		dec A
		bpl RoundPackNormal
		lda 1
		sec
		sbc [D + Biased]
		clc
		adc [D + Shift]
		sta [D + Shift]
		lda 1
		sta [D + Biased]
RoundPackNormal:
		lda [D + Shift]
		beq RoundPackKept
		bmi RoundPackLeft
		cmp 64
		bcc RoundPackRight
		beq RoundPack64
		ldx SigA
		jsr ZeroValue
		bra RoundPackKept
RoundPack64:
		; Only rounds up to the smallest subnormal when more than half of it.
		lda [D + SigA + 6]
		cmp $8000
		bcc RoundPack64Zero
		bne RoundPack64One
		lda [D + SigA]
		ora [D + SigA + 2]
		ora [D + SigA + 4]
		beq RoundPack64Zero
RoundPack64One:
		ldx SigA
		jsr ZeroValue
		inc [D + SigA]
		bra RoundPackKept
RoundPack64Zero:
		ldx SigA
		jsr ZeroValue
		bra RoundPackKept
RoundPackLeft:
		eor $FFFF
		inc A
		ldx SigA
		jsr ShiftLeft
		bra RoundPackKept
RoundPackRight:
		; Down to the round bit, then the round bit out: up when it is set and the rest isn't 0 or
		; the kept bits are odd.
		dec A
		ldx SigA
		jsr ShiftRight
		lda [D + SigA]
		and 1
		sta [D + Temp]
		lsr [D + SigA + 6]
		ror [D + SigA + 4]
		ror [D + SigA + 2]
		ror [D + SigA]
		lda [D + Temp]
		beq RoundPackKept
		lda [D + Sticky]
		bne RoundPackUp
		lda [D + SigA]
		and 1
		beq RoundPackKept
RoundPackUp:
		inc [D + SigA]
		bne RoundPackKept
		inc [D + SigA + 2]
		bne RoundPackKept
		inc [D + SigA + 4]
		bne RoundPackKept
		inc [D + SigA + 6]
RoundPackKept:
		; The significand's top bit adds to the exponent, so a carry out of rounding is right.
		lda [D + SigA]
		sta [D + Result]
		lda [D + SigA + 2]
		sta [D + Result + 2]
		lda [D + SigA + 4]
		sta [D + Result + 4]
		lda [D + SigA + 6]
		sta [D + Result + 6]
		lda [D + Fraction]
		cmp 23
		beq RoundPackPackFloat
		lda [D + Biased]
		dec A
		asl A
		asl A
		asl A
		asl A
		clc
		adc [D + Result + 6]
		sta [D + Result + 6]
		cmp $7FF0
		bcs RoundPackInfinity
		jmp PutSign
RoundPackPackFloat:
		lda [D + Biased]
		dec A
		xba
		and $FF00
		lsr A
		clc
		adc [D + Result + 2]
		sta [D + Result + 2]
		cmp $7F80
		bcs RoundPackInfinity
		jmp PutSign
RoundPackInfinity:
		jmp Infinity

; Z set when RawA (X = 0) or RawB (X = SideB) has no fraction bits: a finite one is a power of two.
IsRawPower:
		lda [D + Fraction]
		cmp 23
		beq IsRawPowerFloat
		lda [D + RawA + 6 + X]
		and $000F
		ora [D + RawA + X]
		ora [D + RawA + 2 + X]
		ora [D + RawA + 4 + X]
		rts
IsRawPowerFloat:
		lda [D + RawA + 2 + X]
		and $007F
		ora [D + RawA + X]
		rts

; SignA (X = 0) or SignB (X = SideB) from the raw bits, the carry set when they are a NaN.
RawSignNaN:
		lda [D + Fraction]
		cmp 23
		beq RawSignNaNFloat
		lda [D + RawA + 6 + X]
		and $8000
		sta [D + SignA + X]
		lda [D + RawA + 6 + X]
		and $7FFF
		cmp $7FF0
		bcc RawSignNaNDone
		bne RawSignNaNDone
		lda [D + RawA + 4 + X]
		ora [D + RawA + 2 + X]
		ora [D + RawA + X]
		cmp 1
RawSignNaNDone:
		rts
RawSignNaNFloat:
		lda [D + RawA + 2 + X]
		and $8000
		sta [D + SignA + X]
		lda [D + RawA + 2 + X]
		and $7FFF
		cmp $7F80
		bcc RawSignNaNDone
		bne RawSignNaNDone
		lda [D + RawA + X]
		cmp 1
		rts

; The 64 bit value at D + X = 0.
ZeroValue:
		stz [D + X]
		stz [D + 2 + X]
		stz [D + 4 + X]
		stz [D + 6 + X]
		rts


;-----------------------------------------------------------------------------------------------
; The float rounding: as RoundPack, for a significand of up to 48 bits (SigA's low three words),
; with the format's numbers as constants.

; RoundPackFloat for a float, else RoundPack, then the result returned.
RoundPackAndReturn:
		lda [D + Fraction]
		cmp 23
		bne RoundPackAndReturnDouble
		jsr RoundPackFloat
		jmp ReturnResult
RoundPackAndReturnDouble:
		jsr RoundPack
		jmp ReturnResult

RoundPackFloat:
		lda [D + SigA]
		ora [D + SigA + 2]
		ora [D + SigA + 4]
		bne RoundPackFloatNotZero
		jmp SignedZero
RoundPackFloatNotZero:
		ldy 32
		lda [D + SigA + 4]
		bne RoundPackFloatTop
		ldy 16
		lda [D + SigA + 2]
		bne RoundPackFloatTop
		ldy 0
		lda [D + SigA]
RoundPackFloatTop:
		jsr HighestBitWord
		sta [D + TopBit]
		clc
		adc [D + ExpA]
		clc
		adc 127
		sta [D + Biased]
		sec
		sbc 255
		bvs RoundPackFloatOverflowed
		bpl RoundPackFloatInfinity
		bra RoundPackFloatInRange
RoundPackFloatOverflowed:
		bmi RoundPackFloatInfinity
RoundPackFloatInRange:
		lda [D + TopBit]
		sec
		sbc 23
		sta [D + Shift]
		lda [D + Biased]
		dec A
		bpl RoundPackFloatNormal
		lda 1
		sec
		sbc [D + Biased]
		clc
		adc [D + Shift]
		sta [D + Shift]
		lda 1
		sta [D + Biased]
RoundPackFloatNormal:
		lda [D + Shift]
		beq RoundPackFloatKept
		bmi RoundPackFloatLeft
		cmp 49
		bcc RoundPackFloatRight
		; Below half the smallest subnormal: 0.
		stz [D + SigA]
		stz [D + SigA + 2]
		bra RoundPackFloatKept
RoundPackFloatLeft:
		; Up to 23 bits: the significand fits two words.
		eor $FFFF
		inc A
		cmp 16
		bcc RoundPackFloatLeftByte
		ldx [D + SigA]
		stx [D + SigA + 2]
		stz [D + SigA]
		sec
		sbc 16
RoundPackFloatLeftByte:
		cmp 8
		bcc RoundPackFloatLeftBits
		tax
		lda [D + SigA + 1]
		sta [D + SigA + 2]
		lda [D + SigA]
		xba
		and $FF00
		sta [D + SigA]
		txa
		and 7
RoundPackFloatLeftBits:
		tay
		beq RoundPackFloatKept
RoundPackFloatLeftLoop:
		asl [D + SigA]
		rol [D + SigA + 2]
		dey
		bne RoundPackFloatLeftLoop
		bra RoundPackFloatKept
RoundPackFloatRight:
		; Down to the round bit, then the round bit out: up when it is set and the rest isn't 0 or
		; the kept bits are odd.
		dec A
		jsr ShiftRight48
		lda [D + SigA]
		and 1
		sta [D + Temp]
		lsr [D + SigA + 4]
		ror [D + SigA + 2]
		ror [D + SigA]
		lda [D + Temp]
		beq RoundPackFloatKept
		lda [D + Sticky]
		bne RoundPackFloatUp
		lda [D + SigA]
		and 1
		beq RoundPackFloatKept
RoundPackFloatUp:
		inc [D + SigA]
		bne RoundPackFloatKept
		inc [D + SigA + 2]
RoundPackFloatKept:
		; The significand's top bit adds to the exponent, so a carry out of rounding is right.
		lda [D + SigA]
		sta [D + Result]
		lda [D + Biased]
		dec A
		xba
		and $FF00
		lsr A
		clc
		adc [D + SigA + 2]
		cmp $7F80
		bcs RoundPackFloatInfinity
		ora [D + SignA]
		sta [D + Result + 2]
		rts
RoundPackFloatInfinity:
		jmp Infinity

; SigA's low 48 bits shifted right A bits (any count), Sticky not 0 if a 1 bit went.
ShiftRight48:
		stz [D + Sticky]
		cmp 48
		bcc ShiftRight48Some
		lda [D + SigA]
		ora [D + SigA + 2]
		ora [D + SigA + 4]
		sta [D + Sticky]
		stz [D + SigA]
		stz [D + SigA + 2]
		stz [D + SigA + 4]
		rts
ShiftRight48Some:
		sta [D + Count]
ShiftRight48Words:
		lda [D + Count]
		cmp 16
		bcc ShiftRight48Byte
		lda [D + SigA]
		ora [D + Sticky]
		sta [D + Sticky]
		lda [D + SigA + 2]
		sta [D + SigA]
		lda [D + SigA + 4]
		sta [D + SigA + 2]
		stz [D + SigA + 4]
		lda [D + Count]
		sec
		sbc 16
		sta [D + Count]
		bra ShiftRight48Words
ShiftRight48Byte:
		cmp 8
		bcc ShiftRight48Bits
		lda [D + SigA]
		and $00FF
		ora [D + Sticky]
		sta [D + Sticky]
		lda [D + SigA + 1]
		sta [D + SigA]
		lda [D + SigA + 3]
		sta [D + SigA + 2]
		lda [D + SigA + 5]
		and $00FF
		sta [D + SigA + 4]
		lda [D + Count]
		and 7
ShiftRight48Bits:
		tay
		beq ShiftRight48Done
ShiftRight48Loop:
		lsr [D + SigA + 4]
		ror [D + SigA + 2]
		ror [D + SigA]
		bcc ShiftRight48Next
		lda 1
		tsb [D + Sticky]
ShiftRight48Next:
		dey
		bne ShiftRight48Loop
ShiftRight48Done:
		rts

; The 32 bit value at D + X shifted right A bits (any count), any bit shifted out kept in the
; lowest bit.
ShiftRightJam32:
		stz [D + Sticky]
		cmp 32
		bcc ShiftRightJam32Some
		lda [D + X]
		ora [D + 2 + X]
		sta [D + Sticky]
		stz [D + X]
		stz [D + 2 + X]
		bra ShiftRightJam32Jam
ShiftRightJam32Some:
		cmp 16
		bcc ShiftRightJam32Byte
		ldy [D + X]
		sty [D + Sticky]
		ldy [D + 2 + X]
		sty [D + X]
		stz [D + 2 + X]
		sec
		sbc 16
ShiftRightJam32Byte:
		cmp 8
		bcc ShiftRightJam32Bits
		tay
		lda [D + X]
		and $00FF
		ora [D + Sticky]
		sta [D + Sticky]
		lda [D + 1 + X]
		sta [D + X]
		lda [D + 3 + X]
		and $00FF
		sta [D + 2 + X]
		tya
		and 7
ShiftRightJam32Bits:
		tay
		beq ShiftRightJam32Jam
ShiftRightJam32Loop:
		lsr [D + 2 + X]
		ror [D + X]
		bcc ShiftRightJam32Next
		lda 1
		tsb [D + Sticky]
ShiftRightJam32Next:
		dey
		bne ShiftRightJam32Loop
ShiftRightJam32Jam:
		lda [D + Sticky]
		beq ShiftRightJam32Done
		lda [D + X]
		ora 1
		sta [D + X]
ShiftRightJam32Done:
		rts

; Floats added and subtracted in two words, their top bits at 31 rather than softfloat.cpp's 61
; (a carry out goes down a bit): the 7 bits below the 24, the lowest sticky, round the same.  A
; difference only goes up more than a bit when no bits were lost aligning them.
AddFiniteFloat:
		; Both up a byte, the top bits at 31: a carry out of the sum goes into SigA + 4.
		lda [D + SigA + 1]
		sta [D + SigA + 2]
		lda [D + SigA]
		xba
		and $FF00
		sta [D + SigA]
		lda [D + SigB + 1]
		sta [D + SigB + 2]
		lda [D + SigB]
		xba
		and $FF00
		sta [D + SigB]
		lda [D + ExpA]
		sec
		sbc 8
		sta [D + ExpA]
		lda [D + ExpB]
		sec
		sbc 8
		sta [D + ExpB]

		; The larger in A.
		lda [D + ExpA]
		sec
		sbc [D + ExpB]
		beq AddFloatSameExponent
		bvc AddFloatExponentSign
		eor $8000
AddFloatExponentSign:
		bpl AddFloatOrdered
		bra AddFloatSwap
AddFloatSameExponent:
		lda [D + SigA + 2]
		cmp [D + SigB + 2]
		bne AddFloatCompared
		lda [D + SigA]
		cmp [D + SigB]
AddFloatCompared:
		bcs AddFloatOrdered
AddFloatSwap:
		jsr SwapFloatSides
AddFloatOrdered:
		lda [D + ExpA]
		sec
		sbc [D + ExpB]
		ldx SigB
		jsr ShiftRightJam32

		lda [D + SignA]
		cmp [D + SignB]
		bne AddFloatDifferentSigns
		clc
		lda [D + SigA]
		adc [D + SigB]
		sta [D + SigA]
		lda [D + SigA + 2]
		adc [D + SigB + 2]
		sta [D + SigA + 2]
		bcc AddFloatSumFits
		; A carry out: down a bit, the bit lost sticky in the lowest.
		ror [D + SigA + 2]
		ror [D + SigA]
		bcc AddFloatSumHalved
		lda [D + SigA]
		ora 1
		sta [D + SigA]
AddFloatSumHalved:
		inc [D + ExpA]
AddFloatSumFits:
		jmp RoundFloat31
AddFloatDifferentSigns:
		lda [D + SigA + 2]
		cmp [D + SigB + 2]
		bne AddFloatUnequal
		lda [D + SigA]
		cmp [D + SigB]
		bne AddFloatUnequal
		stz [D + SignA]
		jsr SignedZero
		jmp ReturnResult
AddFloatUnequal:
		bcs AddFloatASubtractB
		sec
		lda [D + SigB]
		sbc [D + SigA]
		sta [D + SigA]
		lda [D + SigB + 2]
		sbc [D + SigA + 2]
		sta [D + SigA + 2]
		lda [D + SignB]
		sta [D + SignA]
		jmp NormaliseRoundFloat31
AddFloatASubtractB:
		sec
		lda [D + SigA]
		sbc [D + SigB]
		sta [D + SigA]
		lda [D + SigA + 2]
		sbc [D + SigB + 2]
		sta [D + SigA + 2]
		jmp NormaliseRoundFloat31


;-----------------------------------------------------------------------------------------------
; A float's rounding when SigA (two words, 32 bits) has its top bit at 31, any sticky bits in its
; lowest 7: the 24 kept are its top three bytes (read a byte along, no shifting), bit 7 is the
; round bit.  Rounded to nearest, ties to even, and packed with the biased exponent less one
; added to the top word (so a carry out of rounding goes into the exponent, and past the largest
; gives infinity).  A result that isn't a normal float goes to RoundPackFloat.  Returns.
RoundFloat31:
		lda [D + ExpA]
		clc
		adc 157
		cmp 254
		bcs RoundFloat31Range
		sta [D + Biased]
		lda [D + SigA + 1]
		sta [D + Result]
		lda [D + SigA + 3]
		and $00FF
		sta [D + Result + 2]
		lda [D + SigA]
		bit $0080
		beq RoundFloat31Pack
		and $017F
		beq RoundFloat31Pack
		inc [D + Result]
		bne RoundFloat31Pack
		inc [D + Result + 2]
RoundFloat31Pack:
		lda [D + Biased]
		xba
		lsr A
		clc
		adc [D + Result + 2]
		ora [D + SignA]
		sta [D + Result + 2]
		jmp ReturnResult
RoundFloat31Range:
		stz [D + SigA + 4]
		jsr RoundPackFloat
		jmp ReturnResult

; SigA (two words, not 0) shifted left until its top bit is 31, ExpA less the bits.  Bits that
; were sticky in the lowest go up with it: callers shift those by under 7.
NormaliseFloat31:
		lda [D + SigA + 2]
		bne NormaliseFloat31Word
		lda [D + SigA]
		sta [D + SigA + 2]
		stz [D + SigA]
		lda [D + ExpA]
		sec
		sbc 16
		sta [D + ExpA]
		lda [D + SigA + 2]
NormaliseFloat31Word:
		cmp $0100
		bcs NormaliseFloat31Byte
		lda [D + SigA + 1]
		sta [D + SigA + 2]
		lda [D + SigA]
		xba
		and $FF00
		sta [D + SigA]
		lda [D + ExpA]
		sec
		sbc 8
		sta [D + ExpA]
NormaliseFloat31Byte:
		lda [D + SigA + 2]
		bmi NormaliseFloat31Done
NormaliseFloat31Loop:
		dec [D + ExpA]
		asl [D + SigA]
		rol [D + SigA + 2]
		bpl NormaliseFloat31Loop
NormaliseFloat31Done:
		rts

; NormaliseFloat31 then RoundFloat31.
NormaliseRoundFloat31:
		jsr NormaliseFloat31
		jmp RoundFloat31


; A double's rounding when the caller knows where SigA's top bit is (A = P, 54 to 63) and has a
; Sticky (not 0 when a 1 bit was lost below SigA): the bits below the 53 kept are the round bit
; and sticky ones, read with a mask, SigA is shifted down without looking at them, rounded to
; nearest, ties to even, and packed with the biased exponent less one added to its top word (so
; a carry out of rounding goes into the exponent, and past the largest gives infinity).  A
; result that isn't a normal double (an exponent out of 1 to 2046) goes to RoundPack, the sticky
; bits in SigA's lowest (below the round bit).  Returns.
RoundDoubleAt:
		sta [D + TopBit]
		clc
		adc [D + ExpA]
		clc
		adc 1023
		sta [D + Biased]
		dec A
		cmp 2046
		bcs RoundDoubleRange
		lda [D + TopBit]
		sec
		sbc 53
		asl A
		tax
		lda [RoundMasks + X]:24
		and [D + SigA]
		ora [D + Sticky]
		sta [D + Sticky]
		lda [RoundBits + X]:24
		and [D + SigA]
		sta [D + Temp]
		txa
		lsr A
		inc A
		cmp 8
		bcc RoundDoubleBits
		tax
		lda [D + SigA + 1]
		sta [D + SigA]
		lda [D + SigA + 3]
		sta [D + SigA + 2]
		lda [D + SigA + 5]
		sta [D + SigA + 4]
		lda [D + SigA + 7]
		and $00FF
		sta [D + SigA + 6]
		txa
		sec
		sbc 8
RoundDoubleBits:
		tay
		beq RoundDoubleShifted
RoundDoubleLoop:
		lsr [D + SigA + 6]
		ror [D + SigA + 4]
		ror [D + SigA + 2]
		ror [D + SigA]
		dey
		bne RoundDoubleLoop
RoundDoubleShifted:
		lda [D + Temp]
		beq RoundDoublePack
		lda [D + Sticky]
		bne RoundDoubleUp
		lda [D + SigA]
		and 1
		beq RoundDoublePack
RoundDoubleUp:
		inc [D + SigA]
		bne RoundDoublePack
		inc [D + SigA + 2]
		bne RoundDoublePack
		inc [D + SigA + 4]
		bne RoundDoublePack
		inc [D + SigA + 6]
RoundDoublePack:
		lda [D + SigA]
		sta [D + Result]
		lda [D + SigA + 2]
		sta [D + Result + 2]
		lda [D + SigA + 4]
		sta [D + Result + 4]
		lda [D + Biased]
		dec A
		asl A
		asl A
		asl A
		asl A
		clc
		adc [D + SigA + 6]
		ora [D + SignA]
		sta [D + Result + 6]
		jmp ReturnResult
RoundDoubleRange:
		lda [D + Sticky]
		beq RoundDoubleGeneral
		lda [D + SigA]
		ora 1
		sta [D + SigA]
RoundDoubleGeneral:
		jsr RoundPack
		jmp ReturnResult

; (1 << r) - 1 and 1 << r for r = 0 to 10: the bits below a round bit r, and it.
RoundMasks:
		.word $0000, $0001, $0003, $0007, $000F, $001F, $003F, $007F, $00FF, $01FF, $03FF
RoundBits:
		.word $0001, $0002, $0004, $0008, $0010, $0020, $0040, $0080, $0100, $0200, $0400


;-----------------------------------------------------------------------------------------------
; Add and subtract, as softfloat.cpp's Add.

AddOp:
		ldx 0
		jsr Unpack
		ldx SideB
		jsr Unpack
		lda [D + ClassA]
		cmp SoftNaN
		beq AddNaN
		lda [D + ClassB]
		cmp SoftNaN
		bne AddNotNaN
AddNaN:
		jsr PropagateNaN
		jmp ReturnResult
AddNotNaN:
		lda [D + Subtract]
		beq AddSigned
		lda [D + SignB]
		eor $8000
		sta [D + SignB]
		lda [D + Fraction]
		cmp 23
		beq AddFlipFloat
		lda [D + RawB + 6]
		eor $8000
		sta [D + RawB + 6]
		bra AddSigned
AddFlipFloat:
		lda [D + RawB + 2]
		eor $8000
		sta [D + RawB + 2]
AddSigned:
		lda [D + ClassA]
		cmp SoftInfinity
		bne AddANotInfinity
		lda [D + ClassB]
		cmp SoftInfinity
		bne AddReturnA
		lda [D + SignA]
		cmp [D + SignB]
		beq AddReturnA
		jsr DefaultNaN
		jmp ReturnResult
AddANotInfinity:
		lda [D + ClassB]
		cmp SoftInfinity
		beq AddReturnB
		lda [D + ClassA]
		bne AddANotZero
		lda [D + ClassB]
		bne AddReturnB
		; -0 + -0 is -0, any other sum of zeros is +0.
		lda [D + SignA]
		and [D + SignB]
		sta [D + SignA]
		jsr SignedZero
		jmp ReturnResult
AddANotZero:
		lda [D + ClassB]
		beq AddReturnA
		bra AddFinite
AddReturnA:
		ldx 0
		jsr ResultRaw
		jmp ReturnResult
AddReturnB:
		ldx SideB
		jsr ResultRaw
		jmp ReturnResult

AddFinite:
		lda [D + Fraction]
		cmp 23
		bne AddFiniteDouble
		jmp AddFiniteFloat
AddFiniteDouble:
		; The top bits at 61, leaving room for a carry (and RoundPack a byte step down).
		lda 61
		sec
		sbc [D + Fraction]
		sta [D + Shift]
		ldx SigA
		jsr ShiftLeft
		lda [D + Shift]
		ldx SigB
		jsr ShiftLeft
		lda [D + ExpA]
		sec
		sbc [D + Shift]
		sta [D + ExpA]
		lda [D + ExpB]
		sec
		sbc [D + Shift]
		sta [D + ExpB]

		; The larger in A.
		lda [D + ExpA]
		sec
		sbc [D + ExpB]
		beq AddSameExponent
		bvc AddExponentSign
		eor $8000
AddExponentSign:
		bpl AddOrdered
		bra AddSwap
AddSameExponent:
		jsr CompareSignificands
		bcs AddOrdered
AddSwap:
		jsr SwapAddSides
AddOrdered:
		lda [D + ExpA]
		sec
		sbc [D + ExpB]
		ldx SigB
		jsr ShiftRightJam

		lda [D + SignA]
		cmp [D + SignB]
		bne AddDifferentSigns
		clc
		lda [D + SigA]
		adc [D + SigB]
		sta [D + SigA]
		lda [D + SigA + 2]
		adc [D + SigB + 2]
		sta [D + SigA + 2]
		lda [D + SigA + 4]
		adc [D + SigB + 4]
		sta [D + SigA + 4]
		lda [D + SigA + 6]
		adc [D + SigB + 6]
		sta [D + SigA + 6]
		jsr RoundPack
		jmp ReturnResult
AddDifferentSigns:
		jsr CompareSignificands
		bne AddUnequal
		stz [D + SignA]
		jsr SignedZero
		jmp ReturnResult
AddUnequal:
		bcs AddASubtractB
		; B - A, with B's sign.
		sec
		lda [D + SigB]
		sbc [D + SigA]
		sta [D + SigA]
		lda [D + SigB + 2]
		sbc [D + SigA + 2]
		sta [D + SigA + 2]
		lda [D + SigB + 4]
		sbc [D + SigA + 4]
		sta [D + SigA + 4]
		lda [D + SigB + 6]
		sbc [D + SigA + 6]
		sta [D + SigA + 6]
		lda [D + SignB]
		sta [D + SignA]
		jsr RoundPack
		jmp ReturnResult
AddASubtractB:
		sec
		lda [D + SigA]
		sbc [D + SigB]
		sta [D + SigA]
		lda [D + SigA + 2]
		sbc [D + SigB + 2]
		sta [D + SigA + 2]
		lda [D + SigA + 4]
		sbc [D + SigB + 4]
		sta [D + SigA + 4]
		lda [D + SigA + 6]
		sbc [D + SigB + 6]
		sta [D + SigA + 6]
		jsr RoundPack
		jmp ReturnResult

; SigA against SigB, unsigned: carry clear if A < B, Z set if equal.
CompareSignificands:
		lda [D + SigA + 6]
		cmp [D + SigB + 6]
		bne CompareSignificandsDone
		lda [D + SigA + 4]
		cmp [D + SigB + 4]
		bne CompareSignificandsDone
		lda [D + SigA + 2]
		cmp [D + SigB + 2]
		bne CompareSignificandsDone
		lda [D + SigA]
		cmp [D + SigB]
CompareSignificandsDone:
		rts

; Swaps side A's Sign, Exponent and two word significand with side B's (AddFiniteFloat's).
SwapFloatSides:
		lda [D + SignA]
		ldx [D + SignB]
		sta [D + SignB]
		stx [D + SignA]
		lda [D + ExpA]
		ldx [D + ExpB]
		sta [D + ExpB]
		stx [D + ExpA]
		lda [D + SigA]
		ldx [D + SigB]
		sta [D + SigB]
		stx [D + SigA]
		lda [D + SigA + 2]
		ldx [D + SigB + 2]
		sta [D + SigB + 2]
		stx [D + SigA + 2]
		rts

; Swaps side A's Sign, Exponent and significand with side B's (not the Class: Add is past it).
SwapAddSides:
		lda [D + SignA]
		ldx [D + SignB]
		sta [D + SignB]
		stx [D + SignA]
		lda [D + ExpA]
		ldx [D + ExpB]
		sta [D + ExpB]
		stx [D + ExpA]
		lda [D + SigA]
		ldx [D + SigB]
		sta [D + SigB]
		stx [D + SigA]
		lda [D + SigA + 2]
		ldx [D + SigB + 2]
		sta [D + SigB + 2]
		stx [D + SigA + 2]
		lda [D + SigA + 4]
		ldx [D + SigB + 4]
		sta [D + SigB + 4]
		stx [D + SigA + 4]
		lda [D + SigA + 6]
		ldx [D + SigB + 6]
		sta [D + SigB + 6]
		stx [D + SigA + 6]
		rts

;-----------------------------------------------------------------------------------------------
; Multiply, as softfloat.cpp's Multiply.

MultiplyOp:
		ldx 0
		jsr Unpack
		ldx SideB
		jsr Unpack
		lda [D + ClassA]
		cmp SoftNaN
		beq MultiplyNaN
		lda [D + ClassB]
		cmp SoftNaN
		bne MultiplyNotNaN
MultiplyNaN:
		jsr PropagateNaN
		jmp ReturnResult
MultiplyNotNaN:
		lda [D + SignA]
		eor [D + SignB]
		sta [D + SignA]
		lda [D + ClassA]
		cmp SoftInfinity
		beq MultiplyInfinity
		lda [D + ClassB]
		cmp SoftInfinity
		bne MultiplyFiniteOrZero
MultiplyInfinity:
		lda [D + ClassA]
		beq MultiplyInvalid
		lda [D + ClassB]
		beq MultiplyInvalid
		jsr Infinity
		jmp ReturnResult
MultiplyInvalid:
		jsr DefaultNaN
		jmp ReturnResult
MultiplyFiniteOrZero:
		lda [D + ClassA]
		beq MultiplyZero
		lda [D + ClassB]
		bne MultiplyFinite
MultiplyZero:
		jsr SignedZero
		jmp ReturnResult

MultiplyFinite:
		; A power of two (only its hidden bit) times the other is the other's significand,
		; exactly, at the exponents' sum and a power of two's Fraction.
		ldx SideB
		jsr IsRawPower
		beq MultiplyPower
		ldx 0
		jsr IsRawPower
		bne MultiplyGeneral
		lda [D + SigB]
		sta [D + SigA]
		lda [D + SigB + 2]
		sta [D + SigA + 2]
		lda [D + SigB + 4]
		sta [D + SigA + 4]
		lda [D + SigB + 6]
		sta [D + SigA + 6]
MultiplyPower:
		lda [D + ExpA]
		clc
		adc [D + ExpB]
		clc
		adc [D + Fraction]
		sta [D + ExpA]
		jmp RoundPackAndReturn
MultiplyGeneral:
		jsr MultiplySignificands
		lda [D + ExpA]
		clc
		adc [D + ExpB]
		sta [D + ExpA]
		lda [D + Fraction]
		cmp 23
		bne MultiplyDoubleProduct
		; A float's 48 bit product: with its top bit at 47 or 46 (two normal significands) its
		; bytes 2 to 5 (up a bit for 46) are a RoundFloat31 significand, bytes 0 and 1 sticky.
		lda [D + Product + 4]
		bmi MultiplyFloatTop47
		bit $4000
		beq MultiplyFloatAny
		asl [D + Product]
		rol [D + Product + 2]
		rol [D + Product + 4]
		dec [D + ExpA]
MultiplyFloatTop47:
		lda [D + ExpA]
		clc
		adc 16
		sta [D + ExpA]
		lda [D + Product + 4]
		sta [D + SigA + 2]
		lda [D + Product]
		beq MultiplyFloatExact
		lda 1
MultiplyFloatExact:
		ora [D + Product + 2]
		sta [D + SigA]
		jmp RoundFloat31
MultiplyFloatAny:
		lda [D + Product]
		sta [D + SigA]
		lda [D + Product + 2]
		sta [D + SigA + 2]
		lda [D + Product + 4]
		sta [D + SigA + 4]
		jsr RoundPackFloat
		jmp ReturnResult
MultiplyDoubleProduct:

		; The top 64 bits, the rest sticky in the lowest.
		ldx Product + 8
		jsr IsZero
		bne MultiplyHigh
		lda [D + Product]
		sta [D + SigA]
		lda [D + Product + 2]
		sta [D + SigA + 2]
		lda [D + Product + 4]
		sta [D + SigA + 4]
		lda [D + Product + 6]
		sta [D + SigA + 6]
		jsr RoundPack
		jmp ReturnResult
MultiplyHigh:
		; Usually (two normal significands, none of SigB shifted off) the top byte is 13: the
		; window is bytes 6 to 13 and 0 to 5 are sticky.
		lda [D + Product + 13]
		and $00FF
		beq MultiplyHighSearch
		lda [D + ExpA]
		clc
		adc 48
		sta [D + ExpA]
		lda [D + Product + 6]
		sta [D + SigA]
		lda [D + Product + 8]
		sta [D + SigA + 2]
		lda [D + Product + 10]
		sta [D + SigA + 4]
		lda [D + Product + 12]
		sta [D + SigA + 6]
		lda [D + Product]
		ora [D + Product + 2]
		ora [D + Product + 4]
		sta [D + Sticky]
		jmp MultiplyStickyDone
MultiplyHighSearch:
		; The eight bytes ending with the highest that isn't 0 (at least 57 bits below the top,
		; plenty for RoundPack), the bytes below them sticky in the lowest bit.  softfloat.cpp
		; shifts the top 64 bits down instead: the rounding is the same.
		ldx 15
MultiplyTopByte:
		lda [D + Product + X]
		and $00FF
		bne MultiplyTopFound
		dex
		bra MultiplyTopByte
MultiplyTopFound:
		txa
		sec
		sbc 7
		sta [D + Temp]
		asl A
		asl A
		asl A
		clc
		adc [D + ExpA]
		sta [D + ExpA]
		ldx [D + Temp]
		lda [D + Product + X]
		sta [D + SigA]
		lda [D + Product + 2 + X]
		sta [D + SigA + 2]
		lda [D + Product + 4 + X]
		sta [D + SigA + 4]
		lda [D + Product + 6 + X]
		sta [D + SigA + 6]
		stz [D + Sticky]
		ldx 0
MultiplyStickyLoop:
		cpx [D + Temp]
		bcs MultiplyStickyDone
		lda [D + Product + X]
		inx
		cpx [D + Temp]
		bne MultiplyStickyWord
		and $00FF
MultiplyStickyWord:
		ora [D + Sticky]
		sta [D + Sticky]
		inx
		bra MultiplyStickyLoop
MultiplyStickyDone:
		lda [D + SigA + 6]
		ldy 48
		jsr HighestBitWord
		jmp RoundDoubleAt

; Product = SigA * SigB, a row for each byte a of SigA that isn't 0: the row's byte products
; (Garth Wilson's table: the word at MathMult + ((a << 8) | b) * 2 is a * b) are read through
; BPtr + 4 * j = MathMult + b * 512 for SigB's byte j (made once, its bank one up for a b of 128
; or more) with Y = a * 2, and added straight into Product, the even bytes' products as one run
; of words and the odd bytes' as another a byte up.  SigB's low bytes that are 0 are shifted off first (and SigA's, swapping
; them, when it has more), so a row is only as many products as SigB has bytes left: a double
; from a float has four.  ExpA gets the bits shifted off (ExpB is added to it after).
MultiplySignificands:
		lda [D + Fraction]
		cmp 23
		bne MultiplyDouble
		jmp MultiplyFloat
MultiplyDouble:
		ldx SigB
		jsr LowZeroBytes
		sta [D + Temp]
		ldx SigA
		jsr LowZeroBytes
		cmp [D + Temp]
		bcc MultiplyBShorter
		beq MultiplyBShorter
		sta [D + Temp]
		lda [D + SigA]
		ldx [D + SigB]
		sta [D + SigB]
		stx [D + SigA]
		lda [D + SigA + 2]
		ldx [D + SigB + 2]
		sta [D + SigB + 2]
		stx [D + SigA + 2]
		lda [D + SigA + 4]
		ldx [D + SigB + 4]
		sta [D + SigB + 4]
		stx [D + SigA + 4]
		lda [D + SigA + 6]
		ldx [D + SigB + 6]
		sta [D + SigB + 6]
		stx [D + SigA + 6]
MultiplyBShorter:
		lda [D + Temp]
		beq MultiplyBTrimmed
		asl A
		asl A
		asl A
		pha
		clc
		adc [D + ExpA]
		sta [D + ExpA]
		pla
		ldx SigB
		jsr ShiftRight
MultiplyBTrimmed:
		lda [D + SigB + 0]
		xba
		and $FF00
		asl A
		sta [D + BPtr + 0]
		lda MathMult >> 16
		adc 0
		sta [D + BPtr + 0 + 2]
		lda [D + SigB + 1]
		xba
		and $FF00
		asl A
		sta [D + BPtr + 4]
		lda MathMult >> 16
		adc 0
		sta [D + BPtr + 4 + 2]
		lda [D + SigB + 2]
		xba
		and $FF00
		asl A
		sta [D + BPtr + 8]
		lda MathMult >> 16
		adc 0
		sta [D + BPtr + 8 + 2]
		lda [D + SigB + 3]
		xba
		and $FF00
		asl A
		sta [D + BPtr + 12]
		lda MathMult >> 16
		adc 0
		sta [D + BPtr + 12 + 2]
		lda [D + SigB + 4]
		xba
		and $FF00
		asl A
		sta [D + BPtr + 16]
		lda MathMult >> 16
		adc 0
		sta [D + BPtr + 16 + 2]
		lda [D + SigB + 5]
		xba
		and $FF00
		asl A
		sta [D + BPtr + 20]
		lda MathMult >> 16
		adc 0
		sta [D + BPtr + 20 + 2]
		lda [D + SigB + 6]
		xba
		and $FF00
		asl A
		sta [D + BPtr + 24]
		lda MathMult >> 16
		adc 0
		sta [D + BPtr + 24 + 2]
		ldx Product
		jsr ZeroValue
		ldx Product + 8
		jsr ZeroValue
		; The rows for SigB's 7 - Temp bytes; seven with both normal leaves the lowest products out.
		lda [D + Temp]
		bne MultiplyRowsAny
		lda [D + SigA + 6]
		and [D + SigB + 6]
		and $0010
		beq MultiplyRowsAny
		jmp MultiplyRows7Truncated
MultiplyRowsAny:
		lda 6
		sec
		sbc [D + Temp]
		asl A
		tax
		jmp [MultiplyRowsTable + X]

MultiplyRowsTable:
		.word MultiplyRows1 & $FFFF, MultiplyRows2 & $FFFF, MultiplyRows3 & $FFFF, MultiplyRows4 & $FFFF, MultiplyRows5 & $FFFF, MultiplyRows6 & $FFFF, MultiplyRows7 & $FFFF

; The rows for a SigB of 1 to 7 bytes (made by a script: each is the same, for fewer bytes).  The
; usual seven (a double) and four (a double from a float) are unrolled, their offsets constants.
MultiplyRows1:
		ldx 0
MultiplyRows1Loop:
		lda [D + SigA + X]
		and $00FF
		beq MultiplyRows1Next
		asl A
		tay
		clc
		lda [D + Product + 0 + X]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 0 + X]
		bcc MultiplyRows1EvenDone
		inc [D + Product + 2 + X]
		bne MultiplyRows1EvenDone
		lda 4
		jsr MultiplyRipple
MultiplyRows1EvenDone:
MultiplyRows1Next:
		inx
		cpx 7
		bcc MultiplyRows1Loop
		rts

MultiplyRows2:
		ldx 0
MultiplyRows2Loop:
		lda [D + SigA + X]
		and $00FF
		beq MultiplyRows2Next
		asl A
		tay
		clc
		lda [D + Product + 0 + X]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 0 + X]
		bcc MultiplyRows2EvenDone
		inc [D + Product + 2 + X]
		bne MultiplyRows2EvenDone
		lda 4
		jsr MultiplyRipple
MultiplyRows2EvenDone:
		clc
		lda [D + Product + 1 + X]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 1 + X]
		bcc MultiplyRows2OddDone
		inc [D + Product + 3 + X]
		bne MultiplyRows2OddDone
		lda 5
		jsr MultiplyRipple
MultiplyRows2OddDone:
MultiplyRows2Next:
		inx
		cpx 7
		bcc MultiplyRows2Loop
		rts

MultiplyRows3:
		ldx 0
MultiplyRows3Loop:
		lda [D + SigA + X]
		and $00FF
		beq MultiplyRows3Next
		asl A
		tay
		clc
		lda [D + Product + 0 + X]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 0 + X]
		lda [D + Product + 2 + X]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 2 + X]
		bcc MultiplyRows3EvenDone
		inc [D + Product + 4 + X]
		bne MultiplyRows3EvenDone
		lda 6
		jsr MultiplyRipple
MultiplyRows3EvenDone:
		clc
		lda [D + Product + 1 + X]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 1 + X]
		bcc MultiplyRows3OddDone
		inc [D + Product + 3 + X]
		bne MultiplyRows3OddDone
		lda 5
		jsr MultiplyRipple
MultiplyRows3OddDone:
MultiplyRows3Next:
		inx
		cpx 7
		bcc MultiplyRows3Loop
		rts

MultiplyRows4:
		lda [D + SigA + 0]
		and $00FF
		beq MultiplyRows4Row0Done
		asl A
		tay
		clc
		lda [D + Product + 0]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 0]
		lda [D + Product + 2]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 2]
		bcc MultiplyRows4Row0Even
		inc [D + Product + 4]
		bne MultiplyRows4Row0Even
		ldx 6
		jsr MultiplyRippleAt
MultiplyRows4Row0Even:
		clc
		lda [D + Product + 1]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 1]
		lda [D + Product + 3]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 3]
		bcc MultiplyRows4Row0Odd
		inc [D + Product + 5]
		bne MultiplyRows4Row0Odd
		ldx 7
		jsr MultiplyRippleAt
MultiplyRows4Row0Odd:
MultiplyRows4Row0Done:
		lda [D + SigA + 1]
		and $00FF
		beq MultiplyRows4Row1Done
		asl A
		tay
		clc
		lda [D + Product + 1]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 1]
		lda [D + Product + 3]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 3]
		bcc MultiplyRows4Row1Even
		inc [D + Product + 5]
		bne MultiplyRows4Row1Even
		ldx 7
		jsr MultiplyRippleAt
MultiplyRows4Row1Even:
		clc
		lda [D + Product + 2]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 2]
		lda [D + Product + 4]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 4]
		bcc MultiplyRows4Row1Odd
		inc [D + Product + 6]
		bne MultiplyRows4Row1Odd
		ldx 8
		jsr MultiplyRippleAt
MultiplyRows4Row1Odd:
MultiplyRows4Row1Done:
		lda [D + SigA + 2]
		and $00FF
		beq MultiplyRows4Row2Done
		asl A
		tay
		clc
		lda [D + Product + 2]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 2]
		lda [D + Product + 4]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 4]
		bcc MultiplyRows4Row2Even
		inc [D + Product + 6]
		bne MultiplyRows4Row2Even
		ldx 8
		jsr MultiplyRippleAt
MultiplyRows4Row2Even:
		clc
		lda [D + Product + 3]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 3]
		lda [D + Product + 5]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 5]
		bcc MultiplyRows4Row2Odd
		inc [D + Product + 7]
		bne MultiplyRows4Row2Odd
		ldx 9
		jsr MultiplyRippleAt
MultiplyRows4Row2Odd:
MultiplyRows4Row2Done:
		lda [D + SigA + 3]
		and $00FF
		beq MultiplyRows4Row3Done
		asl A
		tay
		clc
		lda [D + Product + 3]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 3]
		lda [D + Product + 5]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 5]
		bcc MultiplyRows4Row3Even
		inc [D + Product + 7]
		bne MultiplyRows4Row3Even
		ldx 9
		jsr MultiplyRippleAt
MultiplyRows4Row3Even:
		clc
		lda [D + Product + 4]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 4]
		lda [D + Product + 6]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 6]
		bcc MultiplyRows4Row3Odd
		inc [D + Product + 8]
		bne MultiplyRows4Row3Odd
		ldx 10
		jsr MultiplyRippleAt
MultiplyRows4Row3Odd:
MultiplyRows4Row3Done:
		lda [D + SigA + 4]
		and $00FF
		beq MultiplyRows4Row4Done
		asl A
		tay
		clc
		lda [D + Product + 4]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 4]
		lda [D + Product + 6]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 6]
		bcc MultiplyRows4Row4Even
		inc [D + Product + 8]
		bne MultiplyRows4Row4Even
		ldx 10
		jsr MultiplyRippleAt
MultiplyRows4Row4Even:
		clc
		lda [D + Product + 5]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 5]
		lda [D + Product + 7]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 7]
		bcc MultiplyRows4Row4Odd
		inc [D + Product + 9]
		bne MultiplyRows4Row4Odd
		ldx 11
		jsr MultiplyRippleAt
MultiplyRows4Row4Odd:
MultiplyRows4Row4Done:
		lda [D + SigA + 5]
		and $00FF
		beq MultiplyRows4Row5Done
		asl A
		tay
		clc
		lda [D + Product + 5]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 5]
		lda [D + Product + 7]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 7]
		bcc MultiplyRows4Row5Even
		inc [D + Product + 9]
		bne MultiplyRows4Row5Even
		ldx 11
		jsr MultiplyRippleAt
MultiplyRows4Row5Even:
		clc
		lda [D + Product + 6]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 6]
		lda [D + Product + 8]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 8]
		bcc MultiplyRows4Row5Odd
		inc [D + Product + 10]
		bne MultiplyRows4Row5Odd
		ldx 12
		jsr MultiplyRippleAt
MultiplyRows4Row5Odd:
MultiplyRows4Row5Done:
		lda [D + SigA + 6]
		and $00FF
		beq MultiplyRows4Row6Done
		asl A
		tay
		clc
		lda [D + Product + 6]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 6]
		lda [D + Product + 8]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 8]
		bcc MultiplyRows4Row6Even
		inc [D + Product + 10]
		bne MultiplyRows4Row6Even
		ldx 12
		jsr MultiplyRippleAt
MultiplyRows4Row6Even:
		clc
		lda [D + Product + 7]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 7]
		lda [D + Product + 9]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 9]
		bcc MultiplyRows4Row6Odd
		inc [D + Product + 11]
		bne MultiplyRows4Row6Odd
		ldx 13
		jsr MultiplyRippleAt
MultiplyRows4Row6Odd:
MultiplyRows4Row6Done:
		rts

MultiplyRows5:
		ldx 0
MultiplyRows5Loop:
		lda [D + SigA + X]
		and $00FF
		beq MultiplyRows5Next
		asl A
		tay
		clc
		lda [D + Product + 0 + X]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 0 + X]
		lda [D + Product + 2 + X]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 2 + X]
		lda [D + Product + 4 + X]
		adc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 4 + X]
		bcc MultiplyRows5EvenDone
		inc [D + Product + 6 + X]
		bne MultiplyRows5EvenDone
		lda 8
		jsr MultiplyRipple
MultiplyRows5EvenDone:
		clc
		lda [D + Product + 1 + X]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 1 + X]
		lda [D + Product + 3 + X]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 3 + X]
		bcc MultiplyRows5OddDone
		inc [D + Product + 5 + X]
		bne MultiplyRows5OddDone
		lda 7
		jsr MultiplyRipple
MultiplyRows5OddDone:
MultiplyRows5Next:
		inx
		cpx 7
		bcc MultiplyRows5Loop
		rts

MultiplyRows6:
		ldx 0
MultiplyRows6Loop:
		lda [D + SigA + X]
		and $00FF
		beq MultiplyRows6Next
		asl A
		tay
		clc
		lda [D + Product + 0 + X]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 0 + X]
		lda [D + Product + 2 + X]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 2 + X]
		lda [D + Product + 4 + X]
		adc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 4 + X]
		bcc MultiplyRows6EvenDone
		inc [D + Product + 6 + X]
		bne MultiplyRows6EvenDone
		lda 8
		jsr MultiplyRipple
MultiplyRows6EvenDone:
		clc
		lda [D + Product + 1 + X]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 1 + X]
		lda [D + Product + 3 + X]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 3 + X]
		lda [D + Product + 5 + X]
		adc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 5 + X]
		bcc MultiplyRows6OddDone
		inc [D + Product + 7 + X]
		bne MultiplyRows6OddDone
		lda 9
		jsr MultiplyRipple
MultiplyRows6OddDone:
MultiplyRows6Next:
		inx
		cpx 7
		bcc MultiplyRows6Loop
		rts

MultiplyRows7:
		lda [D + SigA + 0]
		and $00FF
		beq MultiplyRows7Row0Done
		asl A
		tay
		clc
		lda [D + Product + 0]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 0]
		lda [D + Product + 2]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 2]
		lda [D + Product + 4]
		adc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 4]
		lda [D + Product + 6]
		adc [[D + BPtr + 24]:24 + Y]
		sta [D + Product + 6]
		bcc MultiplyRows7Row0Even
		inc [D + Product + 8]
		bne MultiplyRows7Row0Even
		ldx 10
		jsr MultiplyRippleAt
MultiplyRows7Row0Even:
		clc
		lda [D + Product + 1]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 1]
		lda [D + Product + 3]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 3]
		lda [D + Product + 5]
		adc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 5]
		bcc MultiplyRows7Row0Odd
		inc [D + Product + 7]
		bne MultiplyRows7Row0Odd
		ldx 9
		jsr MultiplyRippleAt
MultiplyRows7Row0Odd:
MultiplyRows7Row0Done:
		lda [D + SigA + 1]
		and $00FF
		beq MultiplyRows7Row1Done
		asl A
		tay
		clc
		lda [D + Product + 1]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 1]
		lda [D + Product + 3]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 3]
		lda [D + Product + 5]
		adc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 5]
		lda [D + Product + 7]
		adc [[D + BPtr + 24]:24 + Y]
		sta [D + Product + 7]
		bcc MultiplyRows7Row1Even
		inc [D + Product + 9]
		bne MultiplyRows7Row1Even
		ldx 11
		jsr MultiplyRippleAt
MultiplyRows7Row1Even:
		clc
		lda [D + Product + 2]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 2]
		lda [D + Product + 4]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 4]
		lda [D + Product + 6]
		adc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 6]
		bcc MultiplyRows7Row1Odd
		inc [D + Product + 8]
		bne MultiplyRows7Row1Odd
		ldx 10
		jsr MultiplyRippleAt
MultiplyRows7Row1Odd:
MultiplyRows7Row1Done:
		lda [D + SigA + 2]
		and $00FF
		beq MultiplyRows7Row2Done
		asl A
		tay
		clc
		lda [D + Product + 2]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 2]
		lda [D + Product + 4]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 4]
		lda [D + Product + 6]
		adc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 6]
		lda [D + Product + 8]
		adc [[D + BPtr + 24]:24 + Y]
		sta [D + Product + 8]
		bcc MultiplyRows7Row2Even
		inc [D + Product + 10]
		bne MultiplyRows7Row2Even
		ldx 12
		jsr MultiplyRippleAt
MultiplyRows7Row2Even:
		clc
		lda [D + Product + 3]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 3]
		lda [D + Product + 5]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 5]
		lda [D + Product + 7]
		adc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 7]
		bcc MultiplyRows7Row2Odd
		inc [D + Product + 9]
		bne MultiplyRows7Row2Odd
		ldx 11
		jsr MultiplyRippleAt
MultiplyRows7Row2Odd:
MultiplyRows7Row2Done:
		lda [D + SigA + 3]
		and $00FF
		beq MultiplyRows7Row3Done
		asl A
		tay
		clc
		lda [D + Product + 3]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 3]
		lda [D + Product + 5]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 5]
		lda [D + Product + 7]
		adc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 7]
		lda [D + Product + 9]
		adc [[D + BPtr + 24]:24 + Y]
		sta [D + Product + 9]
		bcc MultiplyRows7Row3Even
		inc [D + Product + 11]
		bne MultiplyRows7Row3Even
		ldx 13
		jsr MultiplyRippleAt
MultiplyRows7Row3Even:
		clc
		lda [D + Product + 4]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 4]
		lda [D + Product + 6]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 6]
		lda [D + Product + 8]
		adc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 8]
		bcc MultiplyRows7Row3Odd
		inc [D + Product + 10]
		bne MultiplyRows7Row3Odd
		ldx 12
		jsr MultiplyRippleAt
MultiplyRows7Row3Odd:
MultiplyRows7Row3Done:
		lda [D + SigA + 4]
		and $00FF
		beq MultiplyRows7Row4Done
		asl A
		tay
		clc
		lda [D + Product + 4]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 4]
		lda [D + Product + 6]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 6]
		lda [D + Product + 8]
		adc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 8]
		lda [D + Product + 10]
		adc [[D + BPtr + 24]:24 + Y]
		sta [D + Product + 10]
		bcc MultiplyRows7Row4Even
		inc [D + Product + 12]
		bne MultiplyRows7Row4Even
		ldx 14
		jsr MultiplyRippleAt
MultiplyRows7Row4Even:
		clc
		lda [D + Product + 5]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 5]
		lda [D + Product + 7]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 7]
		lda [D + Product + 9]
		adc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 9]
		bcc MultiplyRows7Row4Odd
		inc [D + Product + 11]
		bne MultiplyRows7Row4Odd
		ldx 13
		jsr MultiplyRippleAt
MultiplyRows7Row4Odd:
MultiplyRows7Row4Done:
		lda [D + SigA + 5]
		and $00FF
		beq MultiplyRows7Row5Done
		asl A
		tay
		clc
		lda [D + Product + 5]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 5]
		lda [D + Product + 7]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 7]
		lda [D + Product + 9]
		adc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 9]
		lda [D + Product + 11]
		adc [[D + BPtr + 24]:24 + Y]
		sta [D + Product + 11]
		bcc MultiplyRows7Row5Even
		inc [D + Product + 13]
		bne MultiplyRows7Row5Even
		ldx 15
		jsr MultiplyRippleAt
MultiplyRows7Row5Even:
		clc
		lda [D + Product + 6]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 6]
		lda [D + Product + 8]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 8]
		lda [D + Product + 10]
		adc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 10]
		bcc MultiplyRows7Row5Odd
		inc [D + Product + 12]
		bne MultiplyRows7Row5Odd
		ldx 14
		jsr MultiplyRippleAt
MultiplyRows7Row5Odd:
MultiplyRows7Row5Done:
		lda [D + SigA + 6]
		and $00FF
		beq MultiplyRows7Row6Done
		asl A
		tay
		clc
		lda [D + Product + 6]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 6]
		lda [D + Product + 8]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 8]
		lda [D + Product + 10]
		adc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 10]
		lda [D + Product + 12]
		adc [[D + BPtr + 24]:24 + Y]
		sta [D + Product + 12]
		bcc MultiplyRows7Row6Even
		inc [D + Product + 14]
		bne MultiplyRows7Row6Even
		ldx 16
		jsr MultiplyRippleAt
MultiplyRows7Row6Even:
		clc
		lda [D + Product + 7]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 7]
		lda [D + Product + 9]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 9]
		lda [D + Product + 11]
		adc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 11]
		bcc MultiplyRows7Row6Odd
		inc [D + Product + 13]
		bne MultiplyRows7Row6Odd
		ldx 15
		jsr MultiplyRippleAt
MultiplyRows7Row6Odd:
MultiplyRows7Row6Done:
		rts

; Seven bytes each, both normal: the products of columns 4 and up only (those below add up to
; under 2^42).  The kept bits and the round bit (51 or 52) are those of the whole product unless
; a carry from below could reach them, through bits 42 to 49 all 1s: then the low columns are
; added too.  Otherwise the sticky bits aren't 0 (neither significand has a byte of 0 at the
; bottom, so the product has under 16 trailing 0s), and Product's lowest byte (0 here) is made
; 1 to say so.
MultiplyRows7Truncated:
		lda [D + SigA + 0]
		and $00FF
		beq MultiplyRows7TRow0Done
		asl A
		tay
		clc
		lda [D + Product + 4]
		adc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 4]
		lda [D + Product + 6]
		adc [[D + BPtr + 24]:24 + Y]
		sta [D + Product + 6]
		bcc MultiplyRows7TRow0Even
		inc [D + Product + 8]
		bne MultiplyRows7TRow0Even
		ldx 10
		jsr MultiplyRippleAt
MultiplyRows7TRow0Even:
		clc
		lda [D + Product + 5]
		adc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 5]
		bcc MultiplyRows7TRow0Odd
		inc [D + Product + 7]
		bne MultiplyRows7TRow0Odd
		ldx 9
		jsr MultiplyRippleAt
MultiplyRows7TRow0Odd:
MultiplyRows7TRow0Done:
		lda [D + SigA + 1]
		and $00FF
		beq MultiplyRows7TRow1Done
		asl A
		tay
		clc
		lda [D + Product + 5]
		adc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 5]
		lda [D + Product + 7]
		adc [[D + BPtr + 24]:24 + Y]
		sta [D + Product + 7]
		bcc MultiplyRows7TRow1Even
		inc [D + Product + 9]
		bne MultiplyRows7TRow1Even
		ldx 11
		jsr MultiplyRippleAt
MultiplyRows7TRow1Even:
		clc
		lda [D + Product + 4]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 4]
		lda [D + Product + 6]
		adc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 6]
		bcc MultiplyRows7TRow1Odd
		inc [D + Product + 8]
		bne MultiplyRows7TRow1Odd
		ldx 10
		jsr MultiplyRippleAt
MultiplyRows7TRow1Odd:
MultiplyRows7TRow1Done:
		lda [D + SigA + 2]
		and $00FF
		beq MultiplyRows7TRow2Done
		asl A
		tay
		clc
		lda [D + Product + 4]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 4]
		lda [D + Product + 6]
		adc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 6]
		lda [D + Product + 8]
		adc [[D + BPtr + 24]:24 + Y]
		sta [D + Product + 8]
		bcc MultiplyRows7TRow2Even
		inc [D + Product + 10]
		bne MultiplyRows7TRow2Even
		ldx 12
		jsr MultiplyRippleAt
MultiplyRows7TRow2Even:
		clc
		lda [D + Product + 5]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 5]
		lda [D + Product + 7]
		adc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 7]
		bcc MultiplyRows7TRow2Odd
		inc [D + Product + 9]
		bne MultiplyRows7TRow2Odd
		ldx 11
		jsr MultiplyRippleAt
MultiplyRows7TRow2Odd:
MultiplyRows7TRow2Done:
		lda [D + SigA + 3]
		and $00FF
		beq MultiplyRows7TRow3Done
		asl A
		tay
		clc
		lda [D + Product + 5]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 5]
		lda [D + Product + 7]
		adc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 7]
		lda [D + Product + 9]
		adc [[D + BPtr + 24]:24 + Y]
		sta [D + Product + 9]
		bcc MultiplyRows7TRow3Even
		inc [D + Product + 11]
		bne MultiplyRows7TRow3Even
		ldx 13
		jsr MultiplyRippleAt
MultiplyRows7TRow3Even:
		clc
		lda [D + Product + 4]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 4]
		lda [D + Product + 6]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 6]
		lda [D + Product + 8]
		adc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 8]
		bcc MultiplyRows7TRow3Odd
		inc [D + Product + 10]
		bne MultiplyRows7TRow3Odd
		ldx 12
		jsr MultiplyRippleAt
MultiplyRows7TRow3Odd:
MultiplyRows7TRow3Done:
		lda [D + SigA + 4]
		and $00FF
		beq MultiplyRows7TRow4Done
		asl A
		tay
		clc
		lda [D + Product + 4]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 4]
		lda [D + Product + 6]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 6]
		lda [D + Product + 8]
		adc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 8]
		lda [D + Product + 10]
		adc [[D + BPtr + 24]:24 + Y]
		sta [D + Product + 10]
		bcc MultiplyRows7TRow4Even
		inc [D + Product + 12]
		bne MultiplyRows7TRow4Even
		ldx 14
		jsr MultiplyRippleAt
MultiplyRows7TRow4Even:
		clc
		lda [D + Product + 5]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 5]
		lda [D + Product + 7]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 7]
		lda [D + Product + 9]
		adc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 9]
		bcc MultiplyRows7TRow4Odd
		inc [D + Product + 11]
		bne MultiplyRows7TRow4Odd
		ldx 13
		jsr MultiplyRippleAt
MultiplyRows7TRow4Odd:
MultiplyRows7TRow4Done:
		lda [D + SigA + 5]
		and $00FF
		beq MultiplyRows7TRow5Done
		asl A
		tay
		clc
		lda [D + Product + 5]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 5]
		lda [D + Product + 7]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 7]
		lda [D + Product + 9]
		adc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 9]
		lda [D + Product + 11]
		adc [[D + BPtr + 24]:24 + Y]
		sta [D + Product + 11]
		bcc MultiplyRows7TRow5Even
		inc [D + Product + 13]
		bne MultiplyRows7TRow5Even
		ldx 15
		jsr MultiplyRippleAt
MultiplyRows7TRow5Even:
		clc
		lda [D + Product + 6]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 6]
		lda [D + Product + 8]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 8]
		lda [D + Product + 10]
		adc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 10]
		bcc MultiplyRows7TRow5Odd
		inc [D + Product + 12]
		bne MultiplyRows7TRow5Odd
		ldx 14
		jsr MultiplyRippleAt
MultiplyRows7TRow5Odd:
MultiplyRows7TRow5Done:
		lda [D + SigA + 6]
		and $00FF
		beq MultiplyRows7TRow6Done
		asl A
		tay
		clc
		lda [D + Product + 6]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 6]
		lda [D + Product + 8]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 8]
		lda [D + Product + 10]
		adc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 10]
		lda [D + Product + 12]
		adc [[D + BPtr + 24]:24 + Y]
		sta [D + Product + 12]
		bcc MultiplyRows7TRow6Even
		inc [D + Product + 14]
		bne MultiplyRows7TRow6Even
		ldx 16
		jsr MultiplyRippleAt
MultiplyRows7TRow6Even:
		clc
		lda [D + Product + 7]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 7]
		lda [D + Product + 9]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 9]
		lda [D + Product + 11]
		adc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 11]
		bcc MultiplyRows7TRow6Odd
		inc [D + Product + 13]
		bne MultiplyRows7TRow6Odd
		ldx 15
		jsr MultiplyRippleAt
MultiplyRows7TRow6Odd:
MultiplyRows7TRow6Done:
		lda [D + Product + 5]
		and $03FC
		cmp $03FC
		beq MultiplyLowColumns
		lda 1
		sta [D + Product]
		rts

MultiplyLowColumns:
		lda [D + SigA + 0]
		and $00FF
		beq MultiplyLowColumnsRow0Done
		asl A
		tay
		clc
		lda [D + Product + 0]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 0]
		lda [D + Product + 2]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 2]
		bcc MultiplyLowColumnsRow0Even
		inc [D + Product + 4]
		bne MultiplyLowColumnsRow0Even
		ldx 6
		jsr MultiplyRippleAt
MultiplyLowColumnsRow0Even:
		clc
		lda [D + Product + 1]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 1]
		lda [D + Product + 3]
		adc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 3]
		bcc MultiplyLowColumnsRow0Odd
		inc [D + Product + 5]
		bne MultiplyLowColumnsRow0Odd
		ldx 7
		jsr MultiplyRippleAt
MultiplyLowColumnsRow0Odd:
MultiplyLowColumnsRow0Done:
		lda [D + SigA + 1]
		and $00FF
		beq MultiplyLowColumnsRow1Done
		asl A
		tay
		clc
		lda [D + Product + 1]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 1]
		lda [D + Product + 3]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 3]
		bcc MultiplyLowColumnsRow1Even
		inc [D + Product + 5]
		bne MultiplyLowColumnsRow1Even
		ldx 7
		jsr MultiplyRippleAt
MultiplyLowColumnsRow1Even:
		clc
		lda [D + Product + 2]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 2]
		bcc MultiplyLowColumnsRow1Odd
		inc [D + Product + 4]
		bne MultiplyLowColumnsRow1Odd
		ldx 6
		jsr MultiplyRippleAt
MultiplyLowColumnsRow1Odd:
MultiplyLowColumnsRow1Done:
		lda [D + SigA + 2]
		and $00FF
		beq MultiplyLowColumnsRow2Done
		asl A
		tay
		clc
		lda [D + Product + 2]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 2]
		bcc MultiplyLowColumnsRow2Even
		inc [D + Product + 4]
		bne MultiplyLowColumnsRow2Even
		ldx 6
		jsr MultiplyRippleAt
MultiplyLowColumnsRow2Even:
		clc
		lda [D + Product + 3]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 3]
		bcc MultiplyLowColumnsRow2Odd
		inc [D + Product + 5]
		bne MultiplyLowColumnsRow2Odd
		ldx 7
		jsr MultiplyRippleAt
MultiplyLowColumnsRow2Odd:
MultiplyLowColumnsRow2Done:
		lda [D + SigA + 3]
		and $00FF
		beq MultiplyLowColumnsRow3Done
		asl A
		tay
		clc
		lda [D + Product + 3]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 3]
		bcc MultiplyLowColumnsRow3Even
		inc [D + Product + 5]
		bne MultiplyLowColumnsRow3Even
		ldx 7
		jsr MultiplyRippleAt
MultiplyLowColumnsRow3Even:
MultiplyLowColumnsRow3Done:
		rts

; A carry out of an unrolled row's run of words, on past a word that wrapped: X is that next
; word's offset in Product.
MultiplyRippleAt:
		inc [D + Product + X]
		bne MultiplyRippleAtDone
		inx
		inx
		bra MultiplyRippleAt
MultiplyRippleAtDone:
		rts

; A carry out of a row's run of words, on past a word that wrapped: A is that next word's
; offset from the row's (X, which is kept).
MultiplyRipple:
		stx [D + IndexI]
		clc
		adc [D + IndexI]
		tax
MultiplyRippleLoop:
		inc [D + Product + X]
		bne MultiplyRippleDone
		inx
		inx
		bra MultiplyRippleLoop
MultiplyRippleDone:
		ldx [D + IndexI]
		rts

; A = the number of bytes at D + X, from the lowest, that are 0 (it isn't 0, so up to 7).
LowZeroBytes:
		ldy 0
		lda [D + X]
		bne LowZeroBytesWord
		ldy 2
		lda [D + 2 + X]
		bne LowZeroBytesWord
		ldy 4
		lda [D + 4 + X]
		bne LowZeroBytesWord
		ldy 6
		lda [D + 6 + X]
LowZeroBytesWord:
		and $00FF
		bne LowZeroBytesDone
		iny
LowZeroBytesDone:
		tya
		rts

; A float's Product: 24 bit significands, so nine byte products and a 48 bit product, three
; unrolled rows as MultiplySignificands' (through BPtr, Y = a * 2).
MultiplyFloat:
		ldx Product
		jsr ZeroValue
		lda [D + SigB + 0]
		xba
		and $FF00
		asl A
		sta [D + BPtr + 0]
		lda MathMult >> 16
		adc 0
		sta [D + BPtr + 0 + 2]
		lda [D + SigB + 1]
		xba
		and $FF00
		asl A
		sta [D + BPtr + 4]
		lda MathMult >> 16
		adc 0
		sta [D + BPtr + 4 + 2]
		lda [D + SigB + 2]
		xba
		and $FF00
		asl A
		sta [D + BPtr + 8]
		lda MathMult >> 16
		adc 0
		sta [D + BPtr + 8 + 2]
		lda [D + SigA + 0]
		and $00FF
		beq MultiplyFloatRow0Done
		asl A
		tay
		clc
		lda [D + Product + 0]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 0]
		lda [D + Product + 2]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 2]
		bcc MultiplyFloatRow0Even
		inc [D + Product + 4]
MultiplyFloatRow0Even:
		clc
		lda [D + Product + 1]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 1]
		bcc MultiplyFloatRow0Odd
		inc [D + Product + 3]
MultiplyFloatRow0Odd:
MultiplyFloatRow0Done:
		lda [D + SigA + 1]
		and $00FF
		beq MultiplyFloatRow1Done
		asl A
		tay
		clc
		lda [D + Product + 1]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 1]
		lda [D + Product + 3]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 3]
		bcc MultiplyFloatRow1Even
		inc [D + Product + 5]
MultiplyFloatRow1Even:
		clc
		lda [D + Product + 2]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 2]
		bcc MultiplyFloatRow1Odd
		inc [D + Product + 4]
MultiplyFloatRow1Odd:
MultiplyFloatRow1Done:
		lda [D + SigA + 2]
		and $00FF
		beq MultiplyFloatRow2Done
		asl A
		tay
		clc
		lda [D + Product + 2]
		adc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 2]
		lda [D + Product + 4]
		adc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 4]
		bcc MultiplyFloatRow2Even
		inc [D + Product + 6]
MultiplyFloatRow2Even:
		clc
		lda [D + Product + 3]
		adc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 3]
		bcc MultiplyFloatRow2Odd
		inc [D + Product + 5]
MultiplyFloatRow2Odd:
MultiplyFloatRow2Done:
		rts

;-----------------------------------------------------------------------------------------------
; Divide, as softfloat.cpp's Divide.

DivideOp:
		ldx 0
		jsr Unpack
		ldx SideB
		jsr Unpack
		lda [D + ClassA]
		cmp SoftNaN
		beq DivideNaN
		lda [D + ClassB]
		cmp SoftNaN
		bne DivideNotNaN
DivideNaN:
		jsr PropagateNaN
		jmp ReturnResult
DivideNotNaN:
		lda [D + SignA]
		eor [D + SignB]
		sta [D + SignA]
		lda [D + ClassA]
		cmp SoftInfinity
		bne DivideANotInfinity
		lda [D + ClassB]
		cmp SoftInfinity
		beq DivideInvalid
DivideInfinity:
		jsr Infinity
		jmp ReturnResult
DivideInvalid:
		jsr DefaultNaN
		jmp ReturnResult
DivideANotInfinity:
		lda [D + ClassB]
		cmp SoftInfinity
		beq DivideZero
		lda [D + ClassB]
		bne DivideBNotZero
		lda [D + ClassA]
		beq DivideInvalid
		bra DivideInfinity
DivideBNotZero:
		lda [D + ClassA]
		bne DivideFinite
DivideZero:
		jsr SignedZero
		jmp ReturnResult

DivideFinite:
		; Over a power of two (only its hidden bit) is A's significand, exactly, at the exponents'
		; difference less the power of two's Fraction.
		ldx SideB
		jsr IsRawPower
		bne DivideGeneral
		lda [D + ExpA]
		sec
		sbc [D + ExpB]
		sec
		sbc [D + Fraction]
		sta [D + ExpA]
		jmp RoundPackAndReturn
DivideGeneral:
		lda [D + Fraction]
		cmp 23
		beq DivideFloat
		jmp DivideDouble
DivideFloat:
		; As DivideDouble, four steps: A up to its top bit at 30 (a byte up and a bit down for a
		; normal one), B with its top bit at 23 (where a normal one has it), B's top 16 bits its
		; bytes 1 and 2.  The window is four bytes (and a sign byte), from Product + 4 down to + 1,
		; and Q = A * 2^24 / B, its top bit 31 or 30.
		lda [D + SigA + 2]
		and $0080
		beq DivideFloatNormaliseA
		lda [D + SigA + 1]
		sta [D + SigA + 2]
		lda [D + SigA]
		xba
		and $FF00
		sta [D + SigA]
		lsr [D + SigA + 2]
		ror [D + SigA]
		lda 7
		bra DivideFloatNormalisedA
DivideFloatNormaliseA:
		lda 30
		ldx SigA
		jsr NormaliseTo
		lda [D + Shift]
DivideFloatNormalisedA:
		eor $FFFF
		sec
		adc [D + ExpA]
		sta [D + ExpA]
		lda [D + SigB + 2]
		and $0080
		bne DivideFloatNormalisedB
		lda 23
		ldx SigB
		jsr NormaliseTo
		lda [D + ExpB]
		sec
		sbc [D + Shift]
		sta [D + ExpB]
DivideFloatNormalisedB:
		lda [D + SigB + 0]
		xba
		and $FF00
		asl A
		sta [D + BPtr + 0]
		lda MathMult >> 16
		adc 0
		sta [D + BPtr + 0 + 2]
		lda [D + SigB + 1]
		xba
		and $FF00
		asl A
		sta [D + BPtr + 4]
		lda MathMult >> 16
		adc 0
		sta [D + BPtr + 4 + 2]
		lda [D + SigB + 2]
		xba
		and $FF00
		asl A
		sta [D + BPtr + 8]
		lda MathMult >> 16
		adc 0
		sta [D + BPtr + 8 + 2]
		lda [D + SigB + 1]
		cmp $8000
		bne DivideFloatInvert
		lda $FFFF
		bra DivideFloatInverted
DivideFloatInvert:
		asl A
		asl A
		tax
		bcs DivideFloatInvertHigh
		lda [MathInvert + X]:24
		bra DivideFloatInverted
DivideFloatInvertHigh:
		lda [MathInvert + $10000 + X]:24
DivideFloatInverted:
		and $FF00
		asl A
		sta [D + Ptr1]
		lda MathMult >> 16
		adc 0
		sta [D + Ptr1 + 2]
		lda [D + SigA]
		sta [D + Product + 4]
		lda [D + SigA + 2]
		sta [D + Product + 6]
		stz [D + Product]
		stz [D + Product + 2]
		stz [D + Product + 8]
		stz [D + SigA]
		stz [D + SigA + 2]
		stz [D + SigA + 4]
		; Step 1: the window at Product + 4.
		lda [D + Product + 6]
		sta [D + Temp]
		xba
		and $00FF
		asl A
		tay
		lda [D + Temp]
		and $00FF
		clc
		adc 104
		adc [[D + Ptr1]:24 + Y]
		xba
		and $00FF
		sta [D + Temp2]
		tya
		lsr A
		clc
		adc [D + Temp2]
		cmp $0100
		bcc DivideFloatEstimated1
		lda $00FF
DivideFloatEstimated1:
		sta [D + Count]
		asl A
		tay
		sec
		lda [D + Product + 4]
		sbc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 4]
		lda [D + Product + 6]
		sbc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 6]
		lda [D + Product + 8]
		sbc 0
		sta [D + Product + 8]
		sec
		lda [D + Product + 5]
		sbc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 5]
		lda [D + Product + 7]
		sbc 0
		sta [D + Product + 7]
		bpl DivideFloatCheck1
		ldx 4
		jsr DivideFloatAddBack
		bra DivideFloatDigit1
DivideFloatCheck1:
		lda [D + Product + 6]
		cmp [D + SigB + 2]
		bne DivideFloatChecked1
		lda [D + Product + 4]
		cmp [D + SigB]
DivideFloatChecked1:
		bcc DivideFloatDigit1
		ldx 4
		jsr DivideFloatTakeB
DivideFloatDigit1:
		lda [D + SigA + 3]
		and $FF00
		ora [D + Count]
		sta [D + SigA + 3]
		; Step 2: the window at Product + 3.
		lda [D + Product + 5]
		sta [D + Temp]
		xba
		and $00FF
		asl A
		tay
		lda [D + Temp]
		and $00FF
		clc
		adc 104
		adc [[D + Ptr1]:24 + Y]
		xba
		and $00FF
		sta [D + Temp2]
		tya
		lsr A
		clc
		adc [D + Temp2]
		cmp $0100
		bcc DivideFloatEstimated2
		lda $00FF
DivideFloatEstimated2:
		sta [D + Count]
		asl A
		tay
		sec
		lda [D + Product + 3]
		sbc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 3]
		lda [D + Product + 5]
		sbc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 5]
		lda [D + Product + 7]
		sbc 0
		sta [D + Product + 7]
		sec
		lda [D + Product + 4]
		sbc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 4]
		lda [D + Product + 6]
		sbc 0
		sta [D + Product + 6]
		bpl DivideFloatCheck2
		ldx 3
		jsr DivideFloatAddBack
		bra DivideFloatDigit2
DivideFloatCheck2:
		lda [D + Product + 5]
		cmp [D + SigB + 2]
		bne DivideFloatChecked2
		lda [D + Product + 3]
		cmp [D + SigB]
DivideFloatChecked2:
		bcc DivideFloatDigit2
		ldx 3
		jsr DivideFloatTakeB
DivideFloatDigit2:
		lda [D + SigA + 2]
		and $FF00
		ora [D + Count]
		sta [D + SigA + 2]
		; Step 3: the window at Product + 2.
		lda [D + Product + 4]
		sta [D + Temp]
		xba
		and $00FF
		asl A
		tay
		lda [D + Temp]
		and $00FF
		clc
		adc 104
		adc [[D + Ptr1]:24 + Y]
		xba
		and $00FF
		sta [D + Temp2]
		tya
		lsr A
		clc
		adc [D + Temp2]
		cmp $0100
		bcc DivideFloatEstimated3
		lda $00FF
DivideFloatEstimated3:
		sta [D + Count]
		asl A
		tay
		sec
		lda [D + Product + 2]
		sbc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 2]
		lda [D + Product + 4]
		sbc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 4]
		lda [D + Product + 6]
		sbc 0
		sta [D + Product + 6]
		sec
		lda [D + Product + 3]
		sbc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 3]
		lda [D + Product + 5]
		sbc 0
		sta [D + Product + 5]
		bpl DivideFloatCheck3
		ldx 2
		jsr DivideFloatAddBack
		bra DivideFloatDigit3
DivideFloatCheck3:
		lda [D + Product + 4]
		cmp [D + SigB + 2]
		bne DivideFloatChecked3
		lda [D + Product + 2]
		cmp [D + SigB]
DivideFloatChecked3:
		bcc DivideFloatDigit3
		ldx 2
		jsr DivideFloatTakeB
DivideFloatDigit3:
		lda [D + SigA + 1]
		and $FF00
		ora [D + Count]
		sta [D + SigA + 1]
		; Step 4: the window at Product + 1.
		lda [D + Product + 3]
		sta [D + Temp]
		xba
		and $00FF
		asl A
		tay
		lda [D + Temp]
		and $00FF
		clc
		adc 104
		adc [[D + Ptr1]:24 + Y]
		xba
		and $00FF
		sta [D + Temp2]
		tya
		lsr A
		clc
		adc [D + Temp2]
		cmp $0100
		bcc DivideFloatEstimated4
		lda $00FF
DivideFloatEstimated4:
		sta [D + Count]
		asl A
		tay
		sec
		lda [D + Product + 1]
		sbc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 1]
		lda [D + Product + 3]
		sbc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 3]
		lda [D + Product + 5]
		sbc 0
		sta [D + Product + 5]
		sec
		lda [D + Product + 2]
		sbc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 2]
		lda [D + Product + 4]
		sbc 0
		sta [D + Product + 4]
		bpl DivideFloatCheck4
		ldx 1
		jsr DivideFloatAddBack
		bra DivideFloatDigit4
DivideFloatCheck4:
		lda [D + Product + 3]
		cmp [D + SigB + 2]
		bne DivideFloatChecked4
		lda [D + Product + 1]
		cmp [D + SigB]
DivideFloatChecked4:
		bcc DivideFloatDigit4
		ldx 1
		jsr DivideFloatTakeB
DivideFloatDigit4:
		lda [D + SigA + 0]
		and $FF00
		ora [D + Count]
		sta [D + SigA + 0]
		; Q (top bit 31 or 30, up a bit for 30) at ExpA - ExpB - 24, R (Product + 1 to + 3)
		; sticky in the lowest bit.
		lda [D + ExpA]
		sec
		sbc [D + ExpB]
		sec
		sbc 24
		sta [D + ExpA]
		lda [D + SigA + 2]
		bmi DivideFloatTop31
		asl [D + SigA]
		rol [D + SigA + 2]
		dec [D + ExpA]
DivideFloatTop31:
		lda [D + Product + 1]
		ora [D + Product + 3]
		beq DivideFloatExact
		lda [D + SigA]
		ora 1
		sta [D + SigA]
DivideFloatExact:
		jmp RoundFloat31

; As DivideAddBack and DivideTakeB, for DivideFloat's four byte window.
DivideFloatAddBack:
		clc
		lda [D + Product + X]
		adc [D + SigB]
		sta [D + Product + X]
		lda [D + Product + 2 + X]
		adc [D + SigB + 2]
		sta [D + Product + 2 + X]
		lda [D + Product + 4 + X]
		adc 0
		sta [D + Product + 4 + X]
		dec [D + Count]
		lda [D + Product + 3 + X]
		bmi DivideFloatAddBack
		rts
DivideFloatTakeB:
		lda [D + Product + X]
		sbc [D + SigB]
		sta [D + Product + X]
		lda [D + Product + 2 + X]
		sbc [D + SigB + 2]
		sta [D + Product + 2 + X]
		inc [D + Count]
		lda [D + Product + 2 + X]
		cmp [D + SigB + 2]
		bne DivideFloatTakeBChecked
		lda [D + Product + X]
		cmp [D + SigB]
DivideFloatTakeBChecked:
		bcs DivideFloatTakeB
		rts

;-----------------------------------------------------------------------------------------------
; A double's divide: long division a byte at a time (radix 256).  A goes up to its top bit at 62
; and B to 55 (seven bytes, its top 16 bits its bytes 5 and 6).  Each of seven steps takes q * B
; from the remainder R * 256 (A itself the first time), q = R * 256 / B estimated from its top
; bytes t2 and t1 and the reciprocal of B's top 16 bits, 1:r1:r0 = round(2^32 / those bits), from
; Garth Wilson's table (MathInvert, a 128KB slice of INVERT):
;	q = t2 + ((t1 + t2 * r1 + 104) >> 8)
; 104 stands for the terms left out (t1 * r1 and t2 * r0 over 256, and the next byte).  Modelled
; on 140000 random steps it is right 80% of the time, otherwise one over (11%) or one under (9%):
; a negative remainder gets B back and q one less, one of B or more gives B and q one more.  One
; product is cheaper than the two corrections it saves most of (the full estimate is right
; 99.86% of the time but costs about 85 cycles a step more).  t2 * r1 is read through Ptr1 (made
; once), q * B through BPtr with Y = q * 2, as MultiplySignificands' rows are, taken straight off
; R.  The first q (64 to 255) is the quotient's integer part, so Q = A * 2^48 / B has its top
; bit at 55 or 54.
; R is a window that moves a byte down Product each step: R * 256 is Product + 7 down to + 1 (the
; steps are unrolled), with the byte above it the sign of a remainder that went below 0 (its
; word's other byte isn't used).  The quotient's bytes go into SigA, top down, as the window
; moves, and RoundDoubleAt rounds Q at ExpA - ExpB - 48, R sticky.
DivideDouble:
		; A normal significand (top bit 52) goes up a byte and two bits, or three bits, without
		; looking for its top.
		lda [D + SigA + 6]
		and $0010
		beq DivideNormaliseA
		lda [D + SigA + 5]
		sta [D + SigA + 6]
		lda [D + SigA + 3]
		sta [D + SigA + 4]
		lda [D + SigA + 1]
		sta [D + SigA + 2]
		lda [D + SigA]
		xba
		and $FF00
		asl A
		sta [D + SigA]
		rol [D + SigA + 2]
		rol [D + SigA + 4]
		rol [D + SigA + 6]
		asl [D + SigA]
		rol [D + SigA + 2]
		rol [D + SigA + 4]
		rol [D + SigA + 6]
		lda 10
		bra DivideNormalisedA
DivideNormaliseA:
		lda 62
		ldx SigA
		jsr NormaliseTo
		lda [D + Shift]
DivideNormalisedA:
		eor $FFFF
		sec
		adc [D + ExpA]
		sta [D + ExpA]
		lda [D + SigB + 6]
		and $0010
		beq DivideNormaliseB
		ldy 3
DivideShiftB:
		asl [D + SigB]
		rol [D + SigB + 2]
		rol [D + SigB + 4]
		rol [D + SigB + 6]
		dey
		bne DivideShiftB
		lda 3
		bra DivideNormalisedB
DivideNormaliseB:
		lda 55
		ldx SigB
		jsr NormaliseTo
		lda [D + Shift]
DivideNormalisedB:
		eor $FFFF
		sec
		adc [D + ExpB]
		sta [D + ExpB]
		lda [D + SigB + 0]
		xba
		and $FF00
		asl A
		sta [D + BPtr + 0]
		lda MathMult >> 16
		adc 0
		sta [D + BPtr + 0 + 2]
		lda [D + SigB + 1]
		xba
		and $FF00
		asl A
		sta [D + BPtr + 4]
		lda MathMult >> 16
		adc 0
		sta [D + BPtr + 4 + 2]
		lda [D + SigB + 2]
		xba
		and $FF00
		asl A
		sta [D + BPtr + 8]
		lda MathMult >> 16
		adc 0
		sta [D + BPtr + 8 + 2]
		lda [D + SigB + 3]
		xba
		and $FF00
		asl A
		sta [D + BPtr + 12]
		lda MathMult >> 16
		adc 0
		sta [D + BPtr + 12 + 2]
		lda [D + SigB + 4]
		xba
		and $FF00
		asl A
		sta [D + BPtr + 16]
		lda MathMult >> 16
		adc 0
		sta [D + BPtr + 16 + 2]
		lda [D + SigB + 5]
		xba
		and $FF00
		asl A
		sta [D + BPtr + 20]
		lda MathMult >> 16
		adc 0
		sta [D + BPtr + 20 + 2]
		lda [D + SigB + 6]
		xba
		and $FF00
		asl A
		sta [D + BPtr + 24]
		lda MathMult >> 16
		adc 0
		sta [D + BPtr + 24 + 2]

		; 1:r1:r0 for B's top 16 bits d: the long at MathInvert + (d - $8000) * 4, bit 14 of d
		; picking the bank.  d = $8000's 2^17 is taken as $1FFFF.
		lda [D + SigB + 5]
		cmp $8000
		bne DivideInvert
		lda $FFFF
		bra DivideInverted
DivideInvert:
		asl A
		asl A
		tax
		bcs DivideInvertHigh
		lda [MathInvert + X]:24
		bra DivideInverted
DivideInvertHigh:
		lda [MathInvert + $10000 + X]:24
DivideInverted:
		sta [D + Temp]
		and $FF00
		asl A
		sta [D + Ptr1]
		lda MathMult >> 16
		adc 0
		sta [D + Ptr1 + 2]

		; A into Product + 7 to + 14 (the first window), the rest 0.
		lda [D + SigA]
		sta [D + Product + 7]
		lda [D + SigA + 2]
		sta [D + Product + 9]
		lda [D + SigA + 4]
		sta [D + Product + 11]
		lda [D + SigA + 6]
		sta [D + Product + 13]
		stz [D + Product]
		stz [D + Product + 2]
		stz [D + Product + 4]
		stz [D + Product + 5]
		stz [D + Product + 15]
		stz [D + SigA]
		stz [D + SigA + 2]
		stz [D + SigA + 4]
		stz [D + SigA + 6]

		; The steps, unrolled: the window at Product + 7 down to + 1.
		; q = t2 + ((t1 + t2 * r1 + 104) >> 8).
		lda [D + Product + 13]
		sta [D + Temp]
		xba
		and $00FF
		asl A
		tay
		lda [D + Temp]
		and $00FF
		clc
		adc 104
		adc [[D + Ptr1]:24 + Y]
		xba
		and $00FF
		sta [D + Temp2]
		tya
		lsr A
		clc
		adc [D + Temp2]
		cmp $0100
		bcc DivideEstimated1
		lda $00FF
DivideEstimated1:
		sta [D + Count]
		; R -= q * B.
		asl A
		tay
		sec
		lda [D + Product + 7]
		sbc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 7]
		lda [D + Product + 9]
		sbc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 9]
		lda [D + Product + 11]
		sbc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 11]
		lda [D + Product + 13]
		sbc [[D + BPtr + 24]:24 + Y]
		sta [D + Product + 13]
		lda [D + Product + 15]
		sbc 0
		sta [D + Product + 15]
		sec
		lda [D + Product + 8]
		sbc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 8]
		lda [D + Product + 10]
		sbc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 10]
		lda [D + Product + 12]
		sbc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 12]
		lda [D + Product + 14]
		sbc 0
		sta [D + Product + 14]
		bpl DivideCheck1
		ldx 7
		jsr DivideAddBack
		bra DivideDigit1
DivideCheck1:
		lda [D + Product + 13]
		cmp [D + SigB + 6]
		bne DivideChecked1
		lda [D + Product + 11]
		cmp [D + SigB + 4]
		bne DivideChecked1
		lda [D + Product + 9]
		cmp [D + SigB + 2]
		bne DivideChecked1
		lda [D + Product + 7]
		cmp [D + SigB]
DivideChecked1:
		bcc DivideDigit1
		ldx 7
		jsr DivideTakeB
DivideDigit1:
		lda [D + SigA + 6]
		and $FF00
		ora [D + Count]
		sta [D + SigA + 6]
DivideStep2:
		; q = t2 + ((t1 + t2 * r1 + 104) >> 8).
		lda [D + Product + 12]
		sta [D + Temp]
		xba
		and $00FF
		asl A
		tay
		lda [D + Temp]
		and $00FF
		clc
		adc 104
		adc [[D + Ptr1]:24 + Y]
		xba
		and $00FF
		sta [D + Temp2]
		tya
		lsr A
		clc
		adc [D + Temp2]
		cmp $0100
		bcc DivideEstimated2
		lda $00FF
DivideEstimated2:
		sta [D + Count]
		; R -= q * B.
		asl A
		tay
		sec
		lda [D + Product + 6]
		sbc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 6]
		lda [D + Product + 8]
		sbc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 8]
		lda [D + Product + 10]
		sbc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 10]
		lda [D + Product + 12]
		sbc [[D + BPtr + 24]:24 + Y]
		sta [D + Product + 12]
		lda [D + Product + 14]
		sbc 0
		sta [D + Product + 14]
		sec
		lda [D + Product + 7]
		sbc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 7]
		lda [D + Product + 9]
		sbc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 9]
		lda [D + Product + 11]
		sbc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 11]
		lda [D + Product + 13]
		sbc 0
		sta [D + Product + 13]
		bpl DivideCheck2
		ldx 6
		jsr DivideAddBack
		bra DivideDigit2
DivideCheck2:
		lda [D + Product + 12]
		cmp [D + SigB + 6]
		bne DivideChecked2
		lda [D + Product + 10]
		cmp [D + SigB + 4]
		bne DivideChecked2
		lda [D + Product + 8]
		cmp [D + SigB + 2]
		bne DivideChecked2
		lda [D + Product + 6]
		cmp [D + SigB]
DivideChecked2:
		bcc DivideDigit2
		ldx 6
		jsr DivideTakeB
DivideDigit2:
		lda [D + SigA + 5]
		and $FF00
		ora [D + Count]
		sta [D + SigA + 5]
DivideStep3:
		; q = t2 + ((t1 + t2 * r1 + 104) >> 8).
		lda [D + Product + 11]
		sta [D + Temp]
		xba
		and $00FF
		asl A
		tay
		lda [D + Temp]
		and $00FF
		clc
		adc 104
		adc [[D + Ptr1]:24 + Y]
		xba
		and $00FF
		sta [D + Temp2]
		tya
		lsr A
		clc
		adc [D + Temp2]
		cmp $0100
		bcc DivideEstimated3
		lda $00FF
DivideEstimated3:
		sta [D + Count]
		; R -= q * B.
		asl A
		tay
		sec
		lda [D + Product + 5]
		sbc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 5]
		lda [D + Product + 7]
		sbc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 7]
		lda [D + Product + 9]
		sbc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 9]
		lda [D + Product + 11]
		sbc [[D + BPtr + 24]:24 + Y]
		sta [D + Product + 11]
		lda [D + Product + 13]
		sbc 0
		sta [D + Product + 13]
		sec
		lda [D + Product + 6]
		sbc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 6]
		lda [D + Product + 8]
		sbc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 8]
		lda [D + Product + 10]
		sbc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 10]
		lda [D + Product + 12]
		sbc 0
		sta [D + Product + 12]
		bpl DivideCheck3
		ldx 5
		jsr DivideAddBack
		bra DivideDigit3
DivideCheck3:
		lda [D + Product + 11]
		cmp [D + SigB + 6]
		bne DivideChecked3
		lda [D + Product + 9]
		cmp [D + SigB + 4]
		bne DivideChecked3
		lda [D + Product + 7]
		cmp [D + SigB + 2]
		bne DivideChecked3
		lda [D + Product + 5]
		cmp [D + SigB]
DivideChecked3:
		bcc DivideDigit3
		ldx 5
		jsr DivideTakeB
DivideDigit3:
		lda [D + SigA + 4]
		and $FF00
		ora [D + Count]
		sta [D + SigA + 4]
DivideStep4:
		; q = t2 + ((t1 + t2 * r1 + 104) >> 8).
		lda [D + Product + 10]
		sta [D + Temp]
		xba
		and $00FF
		asl A
		tay
		lda [D + Temp]
		and $00FF
		clc
		adc 104
		adc [[D + Ptr1]:24 + Y]
		xba
		and $00FF
		sta [D + Temp2]
		tya
		lsr A
		clc
		adc [D + Temp2]
		cmp $0100
		bcc DivideEstimated4
		lda $00FF
DivideEstimated4:
		sta [D + Count]
		; R -= q * B.
		asl A
		tay
		sec
		lda [D + Product + 4]
		sbc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 4]
		lda [D + Product + 6]
		sbc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 6]
		lda [D + Product + 8]
		sbc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 8]
		lda [D + Product + 10]
		sbc [[D + BPtr + 24]:24 + Y]
		sta [D + Product + 10]
		lda [D + Product + 12]
		sbc 0
		sta [D + Product + 12]
		sec
		lda [D + Product + 5]
		sbc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 5]
		lda [D + Product + 7]
		sbc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 7]
		lda [D + Product + 9]
		sbc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 9]
		lda [D + Product + 11]
		sbc 0
		sta [D + Product + 11]
		bpl DivideCheck4
		ldx 4
		jsr DivideAddBack
		bra DivideDigit4
DivideCheck4:
		lda [D + Product + 10]
		cmp [D + SigB + 6]
		bne DivideChecked4
		lda [D + Product + 8]
		cmp [D + SigB + 4]
		bne DivideChecked4
		lda [D + Product + 6]
		cmp [D + SigB + 2]
		bne DivideChecked4
		lda [D + Product + 4]
		cmp [D + SigB]
DivideChecked4:
		bcc DivideDigit4
		ldx 4
		jsr DivideTakeB
DivideDigit4:
		lda [D + SigA + 3]
		and $FF00
		ora [D + Count]
		sta [D + SigA + 3]
DivideStep5:
		; q = t2 + ((t1 + t2 * r1 + 104) >> 8).
		lda [D + Product + 9]
		sta [D + Temp]
		xba
		and $00FF
		asl A
		tay
		lda [D + Temp]
		and $00FF
		clc
		adc 104
		adc [[D + Ptr1]:24 + Y]
		xba
		and $00FF
		sta [D + Temp2]
		tya
		lsr A
		clc
		adc [D + Temp2]
		cmp $0100
		bcc DivideEstimated5
		lda $00FF
DivideEstimated5:
		sta [D + Count]
		; R -= q * B.
		asl A
		tay
		sec
		lda [D + Product + 3]
		sbc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 3]
		lda [D + Product + 5]
		sbc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 5]
		lda [D + Product + 7]
		sbc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 7]
		lda [D + Product + 9]
		sbc [[D + BPtr + 24]:24 + Y]
		sta [D + Product + 9]
		lda [D + Product + 11]
		sbc 0
		sta [D + Product + 11]
		sec
		lda [D + Product + 4]
		sbc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 4]
		lda [D + Product + 6]
		sbc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 6]
		lda [D + Product + 8]
		sbc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 8]
		lda [D + Product + 10]
		sbc 0
		sta [D + Product + 10]
		bpl DivideCheck5
		ldx 3
		jsr DivideAddBack
		bra DivideDigit5
DivideCheck5:
		lda [D + Product + 9]
		cmp [D + SigB + 6]
		bne DivideChecked5
		lda [D + Product + 7]
		cmp [D + SigB + 4]
		bne DivideChecked5
		lda [D + Product + 5]
		cmp [D + SigB + 2]
		bne DivideChecked5
		lda [D + Product + 3]
		cmp [D + SigB]
DivideChecked5:
		bcc DivideDigit5
		ldx 3
		jsr DivideTakeB
DivideDigit5:
		lda [D + SigA + 2]
		and $FF00
		ora [D + Count]
		sta [D + SigA + 2]
DivideStep6:
		; q = t2 + ((t1 + t2 * r1 + 104) >> 8).
		lda [D + Product + 8]
		sta [D + Temp]
		xba
		and $00FF
		asl A
		tay
		lda [D + Temp]
		and $00FF
		clc
		adc 104
		adc [[D + Ptr1]:24 + Y]
		xba
		and $00FF
		sta [D + Temp2]
		tya
		lsr A
		clc
		adc [D + Temp2]
		cmp $0100
		bcc DivideEstimated6
		lda $00FF
DivideEstimated6:
		sta [D + Count]
		; R -= q * B.
		asl A
		tay
		sec
		lda [D + Product + 2]
		sbc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 2]
		lda [D + Product + 4]
		sbc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 4]
		lda [D + Product + 6]
		sbc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 6]
		lda [D + Product + 8]
		sbc [[D + BPtr + 24]:24 + Y]
		sta [D + Product + 8]
		lda [D + Product + 10]
		sbc 0
		sta [D + Product + 10]
		sec
		lda [D + Product + 3]
		sbc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 3]
		lda [D + Product + 5]
		sbc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 5]
		lda [D + Product + 7]
		sbc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 7]
		lda [D + Product + 9]
		sbc 0
		sta [D + Product + 9]
		bpl DivideCheck6
		ldx 2
		jsr DivideAddBack
		bra DivideDigit6
DivideCheck6:
		lda [D + Product + 8]
		cmp [D + SigB + 6]
		bne DivideChecked6
		lda [D + Product + 6]
		cmp [D + SigB + 4]
		bne DivideChecked6
		lda [D + Product + 4]
		cmp [D + SigB + 2]
		bne DivideChecked6
		lda [D + Product + 2]
		cmp [D + SigB]
DivideChecked6:
		bcc DivideDigit6
		ldx 2
		jsr DivideTakeB
DivideDigit6:
		lda [D + SigA + 1]
		and $FF00
		ora [D + Count]
		sta [D + SigA + 1]
DivideStep7:
		; q = t2 + ((t1 + t2 * r1 + 104) >> 8).
		lda [D + Product + 7]
		sta [D + Temp]
		xba
		and $00FF
		asl A
		tay
		lda [D + Temp]
		and $00FF
		clc
		adc 104
		adc [[D + Ptr1]:24 + Y]
		xba
		and $00FF
		sta [D + Temp2]
		tya
		lsr A
		clc
		adc [D + Temp2]
		cmp $0100
		bcc DivideEstimated7
		lda $00FF
DivideEstimated7:
		sta [D + Count]
		; R -= q * B.
		asl A
		tay
		sec
		lda [D + Product + 1]
		sbc [[D + BPtr + 0]:24 + Y]
		sta [D + Product + 1]
		lda [D + Product + 3]
		sbc [[D + BPtr + 8]:24 + Y]
		sta [D + Product + 3]
		lda [D + Product + 5]
		sbc [[D + BPtr + 16]:24 + Y]
		sta [D + Product + 5]
		lda [D + Product + 7]
		sbc [[D + BPtr + 24]:24 + Y]
		sta [D + Product + 7]
		lda [D + Product + 9]
		sbc 0
		sta [D + Product + 9]
		sec
		lda [D + Product + 2]
		sbc [[D + BPtr + 4]:24 + Y]
		sta [D + Product + 2]
		lda [D + Product + 4]
		sbc [[D + BPtr + 12]:24 + Y]
		sta [D + Product + 4]
		lda [D + Product + 6]
		sbc [[D + BPtr + 20]:24 + Y]
		sta [D + Product + 6]
		lda [D + Product + 8]
		sbc 0
		sta [D + Product + 8]
		bpl DivideCheck7
		ldx 1
		jsr DivideAddBack
		bra DivideDigit7
DivideCheck7:
		lda [D + Product + 7]
		cmp [D + SigB + 6]
		bne DivideChecked7
		lda [D + Product + 5]
		cmp [D + SigB + 4]
		bne DivideChecked7
		lda [D + Product + 3]
		cmp [D + SigB + 2]
		bne DivideChecked7
		lda [D + Product + 1]
		cmp [D + SigB]
DivideChecked7:
		bcc DivideDigit7
		ldx 1
		jsr DivideTakeB
DivideDigit7:
		lda [D + SigA + 0]
		and $FF00
		ora [D + Count]
		sta [D + SigA + 0]
		jmp DivideDoubleDone

; R (the window at Product + X) is below 0: B back and q one less until it isn't.
DivideAddBack:
		clc
		lda [D + Product + X]
		adc [D + SigB]
		sta [D + Product + X]
		lda [D + Product + 2 + X]
		adc [D + SigB + 2]
		sta [D + Product + 2 + X]
		lda [D + Product + 4 + X]
		adc [D + SigB + 4]
		sta [D + Product + 4 + X]
		lda [D + Product + 6 + X]
		adc [D + SigB + 6]
		sta [D + Product + 6 + X]
		lda [D + Product + 8 + X]
		adc 0
		sta [D + Product + 8 + X]
		dec [D + Count]
		lda [D + Product + 7 + X]
		bmi DivideAddBack
		rts

; R (the window at Product + X) is B or more, the carry set: B off and q one more until it
; isn't.
DivideTakeB:
		lda [D + Product + X]
		sbc [D + SigB]
		sta [D + Product + X]
		lda [D + Product + 2 + X]
		sbc [D + SigB + 2]
		sta [D + Product + 2 + X]
		lda [D + Product + 4 + X]
		sbc [D + SigB + 4]
		sta [D + Product + 4 + X]
		lda [D + Product + 6 + X]
		sbc [D + SigB + 6]
		sta [D + Product + 6 + X]
		inc [D + Count]
		lda [D + Product + 6 + X]
		cmp [D + SigB + 6]
		bne DivideTakeBChecked
		lda [D + Product + 4 + X]
		cmp [D + SigB + 4]
		bne DivideTakeBChecked
		lda [D + Product + 2 + X]
		cmp [D + SigB + 2]
		bne DivideTakeBChecked
		lda [D + Product + X]
		cmp [D + SigB]
DivideTakeBChecked:
		bcs DivideTakeB
		rts

DivideDoubleDone:
		; R (Product + 1 to + 7) is sticky; Q's top bit is 55 or 54, at ExpA - ExpB - 48.
		lda [D + Product + 1]
		ora [D + Product + 3]
		ora [D + Product + 5]
		ora [D + Product + 7]
		sta [D + Sticky]
		lda [D + ExpA]
		sec
		sbc [D + ExpB]
		sec
		sbc 48
		sta [D + ExpA]
		lda [D + SigA + 6]
		cmp $0080
		lda 55
		bcs DivideDoubleTop
		dec A
DivideDoubleTop:
		jmp RoundDoubleAt

; The 64 bit value at D + X shifted left to put its top bit at A; Shift = the bits.
NormaliseTo:
		sta [D + Shift]
		jsr HighestBit
		eor $FFFF
		sec
		adc [D + Shift]
		sta [D + Shift]
		jmp ShiftLeft

; The 64 bit value at D + X shifted left to put its top bit at 62; Shift = the bits.
Normalise62:
		jsr HighestBit
		eor $FFFF
		sec
		adc 62
		sta [D + Shift]
		jmp ShiftLeft


;-----------------------------------------------------------------------------------------------
; Compare, as softfloat.cpp's Compare: -1, 0 or 1 as A is less than, equal to or greater than
; B, and 2 if either is a NaN.

CompareOp:
		; From the bits as given, without unpacking: NaNs, then two zeros, then the signs and the
		; magnitudes.
		ldx 0
		jsr RawSignNaN
		bcs CompareUnordered
		ldx SideB
		jsr RawSignNaN
		bcs CompareUnordered
		lda [D + Fraction]
		cmp 23
		beq CompareFloatZeros
		lda [D + RawA + 6]
		ora [D + RawB + 6]
		and $7FFF
		ora [D + RawA + 4]
		ora [D + RawB + 4]
		ora [D + RawA + 2]
		ora [D + RawB + 2]
		ora [D + RawA]
		ora [D + RawB]
		beq CompareEqual
		bra CompareSigns
CompareFloatZeros:
		lda [D + RawA + 2]
		ora [D + RawB + 2]
		and $7FFF
		ora [D + RawA]
		ora [D + RawB]
		beq CompareEqual
CompareSigns:
		lda [D + SignA]
		cmp [D + SignB]
		beq CompareSameSign
		lda [D + SignA]
		bne CompareLess
		bra CompareGreater
CompareSameSign:
		; The bits without the sign order the magnitudes.
		lda [D + Fraction]
		cmp 23
		beq CompareFloatMagnitude
		lda [D + RawA + 6]
		and $7FFF
		sta [D + Temp]
		lda [D + RawB + 6]
		and $7FFF
		cmp [D + Temp]
		bne CompareMagnitude
		lda [D + RawA + 4]
		cmp [D + RawB + 4]
		bne CompareMagnitudeAB
		lda [D + RawA + 2]
		cmp [D + RawB + 2]
		bne CompareMagnitudeAB
		lda [D + RawA]
		cmp [D + RawB]
		bne CompareMagnitudeAB
		bra CompareEqual
CompareFloatMagnitude:
		lda [D + RawA + 2]
		and $7FFF
		sta [D + Temp]
		lda [D + RawB + 2]
		and $7FFF
		cmp [D + Temp]
		bne CompareMagnitude
		lda [D + RawA]
		cmp [D + RawB]
		bne CompareMagnitudeAB
CompareEqual:
		stz [D + Result]
		jmp ReturnResult
CompareMagnitude:
		; B's magnitude against A's (carry set if B >= A): A < B when the carry is clear... reversed.
		bcc CompareALarger
		bra CompareASmaller
CompareMagnitudeAB:
		bcc CompareASmaller
CompareALarger:
		lda [D + SignA]
		bne CompareLess
		bra CompareGreater
CompareASmaller:
		lda [D + SignA]
		bne CompareGreater
CompareLess:
		lda $FFFF
		sta [D + Result]
		jmp ReturnResult
CompareGreater:
		lda 1
		sta [D + Result]
		jmp ReturnResult
CompareUnordered:
		lda 2
		sta [D + Result]
		jmp ReturnResult


;-----------------------------------------------------------------------------------------------
; From an integer (X = its offset in the arguments): its magnitude rounded, as softfloat.cpp's
; FromInteger.

FromInt32Op:
		jsr ArgIndex
		stz [D + SignA]
		lda [D + X]
		sta [D + SigA]
		lda [D + 2 + X]
		sta [D + SigA + 2]
		bpl FromInteger32
		lda $8000
		sta [D + SignA]
		sec
		lda 0
		sbc [D + SigA]
		sta [D + SigA]
		lda 0
		sbc [D + SigA + 2]
		sta [D + SigA + 2]
		bra FromInteger32

FromUnsigned32Op:
		jsr ArgIndex
		stz [D + SignA]
		lda [D + X]
		sta [D + SigA]
		lda [D + 2 + X]
		sta [D + SigA + 2]
FromInteger32:
		stz [D + SigA + 4]
		stz [D + SigA + 6]
		stz [D + ExpA]
		lda [D + SigA]
		ora [D + SigA + 2]
		beq FromInteger32Zero
		lda [D + Fraction]
		cmp 23
		bne FromInteger32Double
		jmp NormaliseRoundFloat31
FromInteger32Zero:
		jsr SignedZero
		jmp ReturnResult
FromInteger32Double:
		; Exact: up to its top bit at 31, then the top of SigA (top bit 63) for RoundDoubleAt.
		jsr NormaliseFloat31
		lda [D + SigA]
		sta [D + SigA + 4]
		lda [D + SigA + 2]
		sta [D + SigA + 6]
		stz [D + SigA]
		stz [D + SigA + 2]
		stz [D + Sticky]
		lda [D + ExpA]
		sec
		sbc 32
		sta [D + ExpA]
		lda 63
		jmp RoundDoubleAt

FromInt64Op:
		stz [D + SignA]
		jsr FromLoad64
		lda [D + SigA + 6]
		bpl FromInteger
		lda $8000
		sta [D + SignA]
		sec
		lda 0
		sbc [D + SigA]
		sta [D + SigA]
		lda 0
		sbc [D + SigA + 2]
		sta [D + SigA + 2]
		lda 0
		sbc [D + SigA + 4]
		sta [D + SigA + 4]
		lda 0
		sbc [D + SigA + 6]
		sta [D + SigA + 6]
		bra FromInteger

FromUnsigned64Op:
		stz [D + SignA]
		jsr FromLoad64
FromInteger:
		stz [D + ExpA]
		jsr RoundPack
		jmp ReturnResult

FromLoad64:
		jsr ArgIndex
		lda [D + X]
		sta [D + SigA]
		lda [D + 2 + X]
		sta [D + SigA + 2]
		lda [D + 4 + X]
		sta [D + SigA + 4]
		lda [D + 6 + X]
		sta [D + SigA + 6]
		rts


;-----------------------------------------------------------------------------------------------
; To an integer of A bits (32 or 64), signed when X is 1: truncated, as softfloat.cpp's
; ToInteger.  Out of range or a NaN is the most negative integer of the width.

ToIntegerOp:
		sta [D + Bytes]
		stx [D + Subtract]
		ldx 0
		jsr Unpack
		lda [D + ClassA]
		bne ToIntegerNotZero
ToIntegerZero:
		jsr ClearResult
		jmp ReturnResult
ToIntegerNotZero:
		cmp SoftFinite
		beq ToIntegerFinite
ToIntegerIndefinite:
		jsr ClearResult
		lda [D + Bytes]
		cmp 32
		beq ToIntegerIndefinite32
		lda $8000
		sta [D + Result + 6]
		jmp ReturnResult
ToIntegerIndefinite32:
		lda $8000
		sta [D + Result + 2]
		jmp ReturnResult

ToIntegerFinite:
		; 32 bits from a float of under 2^24, or a double under 2^31: the result's bits are the
		; 48 a byte along SigA shifted down under 8 bits (above SigA is side B's, which a
		; conversion doesn't use: 0 for a double).  A float of 2^24 or more, a larger double and
		; 64 bits go the long way.
		lda [D + ExpA]
		bpl ToIntegerLong
		lda [D + Bytes]
		cmp 32
		bne ToIntegerLong
		lda [D + ExpA]
		eor $FFFF
		inc A
		sta [D + Shift]
		ldx SigA
		ldy [D + Fraction]
		cpy 23
		beq ToInteger32Window
		cmp 22
		bcc ToIntegerLong
		stz [D + SigA + 8]
		stz [D + SigA + 10]
		cmp 53
		bcs ToInteger32Zero
		bra ToInteger32Read
ToInteger32Window:
		cmp 24
		bcs ToInteger32Zero
ToInteger32Read:
		lsr A
		lsr A
		lsr A
		sta [D + Temp]
		txa
		clc
		adc [D + Temp]
		tax
		lda [D + X]
		sta [D + Result]
		lda [D + 2 + X]
		sta [D + Result + 2]
		lda [D + 4 + X]
		sta [D + Result + 4]
		lda [D + Shift]
		and 7
		beq ToInteger32Shifted
		tay
ToInteger32Loop:
		lsr [D + Result + 4]
		ror [D + Result + 2]
		ror [D + Result]
		dey
		bne ToInteger32Loop
ToInteger32Shifted:
		lda [D + SignA]
		beq ToInteger32Done
		lda [D + Subtract]
		beq ToInteger32Unsigned
		sec
		lda 0
		sbc [D + Result]
		sta [D + Result]
		lda 0
		sbc [D + Result + 2]
		sta [D + Result + 2]
ToInteger32Done:
		jmp ReturnResult
ToInteger32Unsigned:
		; Below 0: 0 if it truncates to 0, otherwise the indefinite integer.
		lda [D + Result]
		ora [D + Result + 2]
		bne ToInteger32Indefinite
ToInteger32Zero:
		jmp ToIntegerZero
ToInteger32Indefinite:
		jmp ToIntegerIndefinite
ToIntegerLong:
		lda [D + ExpA]
		bmi ToIntegerDown
		ldx SigA
		jsr HighestBit
		clc
		adc [D + ExpA]
		cmp [D + Bytes]
		bcs ToIntegerIndefinite
		lda [D + ExpA]
		ldx SigA
		jsr ShiftLeft
		bra ToIntegerShifted
ToIntegerDown:
		eor $FFFF
		inc A
		cmp 64
		bcs ToIntegerZero
		ldx SigA
		jsr ShiftRight
ToIntegerShifted:
		lda [D + Subtract]
		beq ToIntegerUnsigned

		; Up to 2^(bits-1) - 1, or 2^(bits-1) when negative.
		lda [D + Bytes]
		cmp 32
		bne ToIntegerSigned64
		lda [D + SigA + 4]
		ora [D + SigA + 6]
		bne ToIntegerIndefinite
		lda [D + SigA + 2]
		ldx [D + SigA]
		bra ToIntegerSignedTop
ToIntegerSigned64:
		lda [D + SigA]
		ora [D + SigA + 2]
		ora [D + SigA + 4]
		tax
		lda [D + SigA + 6]
ToIntegerSignedTop:
		; A = the top word, X not 0 if any lower bit is set.
		cmp $8000
		bcc ToIntegerSignedFits
		bne ToIntegerIndefinite
		cpx 0
		bne ToIntegerIndefinite
		lda [D + SignA]
		beq ToIntegerIndefinite
ToIntegerSignedFits:
		lda [D + SignA]
		beq ToIntegerPositive
		sec
		lda 0
		sbc [D + SigA]
		sta [D + SigA]
		lda 0
		sbc [D + SigA + 2]
		sta [D + SigA + 2]
		lda 0
		sbc [D + SigA + 4]
		sta [D + SigA + 4]
		lda 0
		sbc [D + SigA + 6]
		sta [D + SigA + 6]
ToIntegerPositive:
		lda [D + SigA]
		sta [D + Result]
		lda [D + SigA + 2]
		sta [D + Result + 2]
		lda [D + SigA + 4]
		sta [D + Result + 4]
		lda [D + SigA + 6]
		sta [D + Result + 6]
		jmp ReturnResult

ToIntegerUnsigned:
		lda [D + SignA]
		beq ToIntegerUnsignedPositive
		ldx SigA
		jsr IsZero
		bne ToIntegerToIndefinite
		jmp ToIntegerZero
ToIntegerUnsignedPositive:
		lda [D + Bytes]
		cmp 32
		bne ToIntegerPositive
		; Too large: only a double from 2^32 to 2^53 gets here (shifted down), as unsigned.
		lda [D + SigA + 4]
		ora [D + SigA + 6]
		beq ToIntegerPositive
ToIntegerToIndefinite:
		jmp ToIntegerIndefinite


;-----------------------------------------------------------------------------------------------
; Float to double and double to float, as softfloat.cpp's Change.  NaNs keep the top of their
; payload and are quietened.

FloatToDoubleOp:
		; A normal float's bits move: the fraction up 29 and the exponent biased 896 more.
		lda [D + RawA + 2]
		and $7F80
		beq FloatToDoubleUnpack
		cmp $7F80
		beq FloatToDoubleUnpack
		lsr A
		lsr A
		lsr A
		clc
		adc $3800
		sta [D + Temp2]
		lda [D + RawA + 2]
		and $007F
		sta [D + Temp]
		lda [D + RawA]
		lsr [D + Temp]
		ror A
		lsr [D + Temp]
		ror A
		lsr [D + Temp]
		ror A
		sta [D + Result + 4]
		lda [D + RawA + 2]
		and $8000
		ora [D + Temp]
		ora [D + Temp2]
		sta [D + Result + 6]
		lda [D + RawA]
		xba
		and $FF00
		asl A
		asl A
		asl A
		asl A
		asl A
		sta [D + Result + 2]
		stz [D + Result]
		jmp ReturnResult
FloatToDoubleUnpack:
		ldx 0
		jsr Unpack
		jsr SetFormatDouble
		lda [D + ClassA]
		cmp SoftNaN
		beq FloatToDoubleNaN
		jmp ChangeClass
FloatToDoubleNaN:
		; The fraction (unpacked) up 29 bits.
		lda 29
		ldx SigA
		jsr ShiftLeft
		jmp ChangeNaN

DoubleToFloatOp:
		; A double that is a normal float: its bits from the fraction's top 23 down (the exponent
		; biased 896 less), rounded to nearest, ties to even, by what is below them.  A carry out
		; of rounding goes into the exponent (to infinity from the largest).
		lda [D + RawA + 6]
		and $7FF0
		cmp $3810
		bcc DoubleToFloatUnpack
		cmp $47F0
		bcs DoubleToFloatUnpack
		lda [D + RawA + 2]
		sta [D + Result]
		lda [D + RawA + 4]
		sta [D + Result + 2]
		lda [D + RawA + 6]
		and $7FFF
		sta [D + Result + 4]
		ldy 3
DoubleToFloatShift:
		asl [D + Result]
		rol [D + Result + 2]
		rol [D + Result + 4]
		dey
		bne DoubleToFloatShift
		lda [D + Result]
		bpl DoubleToFloatRounded
		and $7FFF
		ora [D + RawA]
		bne DoubleToFloatUp
		lda [D + Result + 2]
		and 1
		beq DoubleToFloatRounded
DoubleToFloatUp:
		inc [D + Result + 2]
		bne DoubleToFloatRounded
		inc [D + Result + 4]
DoubleToFloatRounded:
		lda [D + Result + 2]
		sta [D + Result]
		lda [D + RawA + 6]
		and $8000
		sta [D + Temp]
		lda [D + Result + 4]
		sec
		sbc $C000
		ora [D + Temp]
		sta [D + Result + 2]
		jmp ReturnResult
DoubleToFloatUnpack:
		ldx 0
		jsr Unpack
		jsr SetFormatFloat
		lda [D + ClassA]
		cmp SoftNaN
		beq DoubleToFloatNaN
		jmp ChangeClass
DoubleToFloatNaN:
		lda 29
		ldx SigA
		jsr ShiftRight
ChangeNaN:
		jsr Infinity
		lda [D + SigA]
		ora [D + Result]
		sta [D + Result]
		lda [D + SigA + 2]
		ora [D + Result + 2]
		sta [D + Result + 2]
		lda [D + SigA + 4]
		ora [D + Result + 4]
		sta [D + Result + 4]
		lda [D + SigA + 6]
		ora [D + Result + 6]
		sta [D + Result + 6]
		jsr Quieten
		jmp ReturnResult

; The other classes, the format now the one changed to.
ChangeClass:
		lda [D + ClassA]
		bne ChangeNotZero
		jsr SignedZero
		jmp ReturnResult
ChangeNotZero:
		cmp SoftInfinity
		bne ChangeFinite
		jsr Infinity
		jmp ReturnResult
ChangeFinite:
		jsr RoundPack
		jmp ReturnResult


;-----------------------------------------------------------------------------------------------
; The square root, correctly rounded, as softfloat.cpp's SquareRoot (its classes and NaNs; the
; root a bit at a time, but with the remainder no wider than it has to be).  The square root of
; -0 is -0; of anything else below zero, the default NaN.

SqrtOp:
		ldx 0
		jsr Unpack
		lda [D + ClassA]
		cmp SoftNaN
		bne SqrtNotNaN
		ldx 0
		jsr ResultRaw
		jsr Quieten
		jmp ReturnResult
SqrtNotNaN:
		lda [D + ClassA]
		beq SqrtReturnA
		lda [D + SignA]
		beq SqrtNotNegative
		jsr DefaultNaN
		jmp ReturnResult
SqrtNotNegative:
		lda [D + ClassA]
		cmp SoftInfinity
		bne SqrtFinite
SqrtReturnA:
		ldx 0
		jsr ResultRaw
		jmp ReturnResult

SqrtFinite:
		; The significand from 2^61 to 2^63 with an even exponent.
		ldx SigA
		jsr Normalise62
		lda [D + ExpA]
		sec
		sbc [D + Shift]
		sta [D + ExpA]
		and 1
		beq SqrtEven
		lsr [D + SigA + 6]
		ror [D + SigA + 4]
		ror [D + SigA + 2]
		ror [D + SigA]
		inc [D + ExpA]
SqrtEven:
		; A bit of the root for each two bits of the radicand, the significand times 2^(2N - 64)
		; (N steps): its bits come in from the top, a word at a time (SqrtLoadWord), then 0s.
		; SqrtT keeps T = root << 2, so the trial (root << 2) | 1 is T + 1, reached when the
		; remainder is above T.  After k steps the root is under 2^k and the remainder under
		; 2^(k + 3), so the steps are done in one word, then two, then four as they grow.
		lda 6
		sta [D + Count]
		stz [D + Temp2]
		ldx SqrtT
		jsr ZeroValue
		ldx SqrtRest
		jsr ZeroValue
		lda 13
		sta [D + IndexI]
		jsr SqrtStep1
		lda [D + Fraction]
		cmp 23
		bne SqrtDouble
		jmp SqrtFloat
SqrtDouble:
		; 55 steps give a root of at least 54 bits: 53 and the round bit.  The remainder is
		; under 2^31 after 28.
		lda 15
		sta [D + IndexI]
		jsr SqrtStep2
		lda 27
		sta [D + IndexI]
		jsr SqrtStep4
SqrtDoubleDone:
		; (root << 1) | (remainder != 0) = (T >> 1) | (remainder != 0), at (exponent - 46) / 2 - 1.
		lsr [D + SqrtT + 6]
		ror [D + SqrtT + 4]
		ror [D + SqrtT + 2]
		ror [D + SqrtT]
		ldx SqrtRest
		jsr IsZero
		beq SqrtDoubleExact
		lda 1
SqrtDoubleExact:
		ora [D + SqrtT]
		sta [D + SigA]
		lda [D + SqrtT + 2]
		sta [D + SigA + 2]
		lda [D + SqrtT + 4]
		sta [D + SigA + 4]
		lda [D + SqrtT + 6]
		sta [D + SigA + 6]
		lda [D + ExpA]
		sec
		sbc 46
		cmp $8000
		ror A
		dec A
		sta [D + ExpA]
		stz [D + SignA]
		jsr RoundPack
		jmp ReturnResult

SqrtFloat:
		; 26 steps give a root of at least 25 bits: 24 and the round bit.  The remainder is
		; under 2^29.
		lda 13
		sta [D + IndexI]
		jsr SqrtStep2
SqrtFloatDone:
		; As the double's, at (exponent + 12) / 2 - 1.
		lsr [D + SqrtT + 2]
		ror [D + SqrtT]
		lda [D + SqrtRest]
		ora [D + SqrtRest + 2]
		beq SqrtFloatExact
		lda 1
SqrtFloatExact:
		ora [D + SqrtT]
		sta [D + SigA]
		lda [D + SqrtT + 2]
		sta [D + SigA + 2]
		stz [D + SigA + 4]
		lda [D + ExpA]
		clc
		adc 12
		cmp $8000
		ror A
		dec A
		sta [D + ExpA]
		stz [D + SignA]
		jmp NormaliseRoundFloat31

; Temp = the radicand's next word (SigA + Count, then 0s), Temp2 = its 8 steps.
SqrtLoadWord:
		lda 8
		sta [D + Temp2]
		lda [D + Count]
		bmi SqrtLoadZero
		tax
		lda [D + SigA + X]
		dex
		dex
		stx [D + Count]
		sta [D + Temp]
		rts
SqrtLoadZero:
		stz [D + Temp]
		rts

; IndexI steps, the remainder and T in one, two or four words.
SqrtStep1:
		lda [D + Temp2]
		bne SqrtStep1Loaded
		jsr SqrtLoadWord
SqrtStep1Loaded:
		asl [D + Temp]
		rol [D + SqrtRest]
		asl [D + Temp]
		rol [D + SqrtRest]
		lda [D + SqrtRest]
		cmp [D + SqrtT]
		beq SqrtStep1ZeroBit
		bcc SqrtStep1ZeroBit
		; SqrtRest -= T + 1, T = (T << 1) + 4.
		clc
		lda [D + SqrtRest]
		sbc [D + SqrtT]
		sta [D + SqrtRest]
		lda [D + SqrtT]
		asl A
		ora 4
		sta [D + SqrtT]
		bra SqrtStep1ShiftT
SqrtStep1ZeroBit:
		asl [D + SqrtT]
SqrtStep1ShiftT:
		dec [D + Temp2]
		dec [D + IndexI]
		bne SqrtStep1
		rts

SqrtStep2:
		lda [D + Temp2]
		bne SqrtStep2Loaded
		jsr SqrtLoadWord
SqrtStep2Loaded:
		asl [D + Temp]
		rol [D + SqrtRest]
		rol [D + SqrtRest + 2]
		asl [D + Temp]
		rol [D + SqrtRest]
		rol [D + SqrtRest + 2]
		lda [D + SqrtRest + 2]
		cmp [D + SqrtT + 2]
		bne SqrtStep2Compared
		lda [D + SqrtRest]
		cmp [D + SqrtT]
		beq SqrtStep2ZeroBit
SqrtStep2Compared:
		bcc SqrtStep2ZeroBit
		; SqrtRest -= T + 1, T = (T << 1) + 4.
		clc
		lda [D + SqrtRest]
		sbc [D + SqrtT]
		sta [D + SqrtRest]
		lda [D + SqrtRest + 2]
		sbc [D + SqrtT + 2]
		sta [D + SqrtRest + 2]
		lda [D + SqrtT]
		asl A
		ora 4
		sta [D + SqrtT]
		bra SqrtStep2ShiftT
SqrtStep2ZeroBit:
		asl [D + SqrtT]
SqrtStep2ShiftT:
		rol [D + SqrtT + 2]
		dec [D + Temp2]
		dec [D + IndexI]
		bne SqrtStep2
		rts

SqrtStep4:
		lda [D + Temp2]
		bne SqrtStep4Loaded
		jsr SqrtLoadWord
SqrtStep4Loaded:
		asl [D + Temp]
		rol [D + SqrtRest]
		rol [D + SqrtRest + 2]
		rol [D + SqrtRest + 4]
		rol [D + SqrtRest + 6]
		asl [D + Temp]
		rol [D + SqrtRest]
		rol [D + SqrtRest + 2]
		rol [D + SqrtRest + 4]
		rol [D + SqrtRest + 6]
		lda [D + SqrtRest + 6]
		cmp [D + SqrtT + 6]
		bne SqrtStep4Compared
		lda [D + SqrtRest + 4]
		cmp [D + SqrtT + 4]
		bne SqrtStep4Compared
		lda [D + SqrtRest + 2]
		cmp [D + SqrtT + 2]
		bne SqrtStep4Compared
		lda [D + SqrtRest]
		cmp [D + SqrtT]
		beq SqrtStep4ZeroBit
SqrtStep4Compared:
		bcc SqrtStep4ZeroBit
		; SqrtRest -= T + 1, T = (T << 1) + 4.
		clc
		lda [D + SqrtRest]
		sbc [D + SqrtT]
		sta [D + SqrtRest]
		lda [D + SqrtRest + 2]
		sbc [D + SqrtT + 2]
		sta [D + SqrtRest + 2]
		lda [D + SqrtRest + 4]
		sbc [D + SqrtT + 4]
		sta [D + SqrtRest + 4]
		lda [D + SqrtRest + 6]
		sbc [D + SqrtT + 6]
		sta [D + SqrtRest + 6]
		lda [D + SqrtT]
		asl A
		ora 4
		sta [D + SqrtT]
		bra SqrtStep4ShiftT
SqrtStep4ZeroBit:
		asl [D + SqrtT]
SqrtStep4ShiftT:
		rol [D + SqrtT + 2]
		rol [D + SqrtT + 4]
		rol [D + SqrtT + 6]
		dec [D + Temp2]
		dec [D + IndexI]
		bne SqrtStep4
		rts


;-----------------------------------------------------------------------------------------------
; <math.h>'s functions that only take bits apart: math.cpp has them for the 65C64 and leaves them
; to this file on the W65C816 (fabs, sqrt, trunc, floor, ceil, round and their float and long
; double forms, frexp, ldexp, modf and fmod).  The results are math.cpp's, bit for bit (its float
; forms worked in double: a float NaN comes back quiet).
;
; trunc, floor, ceil and round clear x's bits below the binary point (k = 1075 - the biased
; exponent of a double, 150 - a float's), then floor, ceil or round adds one in the lowest bit
; left when it has to.  They run on a frame of their own (D on S, ten bytes below the caller's D):
;	D + 1 mode (0 trunc, 1 floor, 2 ceil, 3 round), + 3 k, + 5 the cleared bits' OR, + 7 the bit
;	below the lowest kept (or the exponent bits for x under 1), + 9 a word
;	D + 11 the caller's D, + 13 the return address
;	a double's: D + 16 the result pointer, D + 20 x; a float's: D + 16 x

		.global _fabs, _fabsl, _fabsf, _sqrt, _sqrtl, _sqrtf
		.global _trunc, _truncl, _truncf, _floor, _floorl, _floorf, _ceil, _ceill, _ceilf, _round, _roundl, _roundf
		.global _frexp, _ldexp, _modf, _fmod, _fmodl, _fmodf

; double trunc(double x), long double truncl(long double x)
_trunc:
_truncl:
		.width (A, X), 16
		ldy 0
		jmp RoundingDouble

; double floor(double x), long double floorl(long double x)
_floor:
_floorl:
		.width (A, X), 16
		ldy 1
		jmp RoundingDouble

; double ceil(double x), long double ceill(long double x)
_ceil:
_ceill:
		.width (A, X), 16
		ldy 2
		jmp RoundingDouble

; double round(double x), long double roundl(long double x): halves away from 0
_round:
_roundl:
		.width (A, X), 16
		ldy 3
		jmp RoundingDouble

; float truncf(float x)
_truncf:
		.width (A, X), 16
		ldy 0
		jmp RoundingFloat

; float floorf(float x)
_floorf:
		.width (A, X), 16
		ldy 1
		jmp RoundingFloat

; float ceilf(float x)
_ceilf:
		.width (A, X), 16
		ldy 2
		jmp RoundingFloat

; float roundf(float x)
_roundf:
		.width (A, X), 16
		ldy 3
		jmp RoundingFloat

RoundingDouble:
		phd
		tsc
		sec
		sbc 10
		tcs
		tcd
		sty [D + 1]
		stz [D + 5]
		stz [D + 7]
		lda [D + 26]
		and $7FF0
		cmp $4330
		bcc RoundingDoubleFraction
		jmp RoundingDoubleDone
RoundingDoubleFraction:
		cmp $3FF0
		bcs RoundingDoubleInteger
		jmp RoundingDoubleSmall
RoundingDoubleInteger:
		lsr A
		lsr A
		lsr A
		lsr A
		eor $FFFF
		sec
		adc 1075
		sta [D + 3]
		; Half (for round): bit k - 1.
		dec A
		jsr RoundingDoubleBit
		sta [D + 7]
		; The k bits below the binary point cleared, Frac their OR.
		ldx 0
		ldy [D + 3]
RoundingDoubleWords:
		cpy 16
		bcc RoundingDoublePart
		lda [D + 20 + X]
		ora [D + 5]
		sta [D + 5]
		stz [D + 20 + X]
		inx
		inx
		tya
		sec
		sbc 16
		tay
		bra RoundingDoubleWords
RoundingDoublePart:
		tya
		beq RoundingDoubleCleared
		asl A
		phx
		tax
		lda [LowBitMasks + X]:24
		plx
		sta [D + 9]
		and [D + 20 + X]
		ora [D + 5]
		sta [D + 5]
		lda [D + 9]
		eor $FFFF
		and [D + 20 + X]
		sta [D + 20 + X]
RoundingDoubleCleared:
		; floor: below 0 with a fraction, ceil: above 0 with one, round: Half; one up a unit in
		; the last place left (a carry goes into the exponent).
		lda [D + 1]
		beq RoundingDoubleDone
		cmp 3
		beq RoundingDoubleRound
		cmp 1
		beq RoundingDoubleFloor
		lda [D + 26]
		bmi RoundingDoubleDone
		lda [D + 5]
		beq RoundingDoubleDone
		bra RoundingDoubleUp
RoundingDoubleFloor:
		lda [D + 26]
		bpl RoundingDoubleDone
		lda [D + 5]
		beq RoundingDoubleDone
		bra RoundingDoubleUp
RoundingDoubleRound:
		lda [D + 7]
		beq RoundingDoubleDone
RoundingDoubleUp:
		lda [D + 3]
		and 15
		asl A
		tax
		lda [BitWords + X]:24
		sta [D + 9]
		lda [D + 3]
		lsr A
		lsr A
		lsr A
		lsr A
		asl A
		tax
		clc
		lda [D + 20 + X]
		adc [D + 9]
		sta [D + 20 + X]
		bcc RoundingDoubleDone
		; The carry on up (the top word, with the exponent, never wraps).
RoundingDoubleCarry:
		inx
		inx
		inc [D + 20 + X]
		beq RoundingDoubleCarry
		jmp RoundingDoubleDone
RoundingDoubleSmall:
		; Under 1: 0 with its sign, or 1 with it for floor (below 0), ceil (above 0) and round
		; (a half or more).
		sta [D + 7]
		lda [D + 26]
		and $8000
		sta [D + 9]
		lda [D + 26]
		and $7FFF
		ora [D + 20]
		ora [D + 22]
		ora [D + 24]
		sta [D + 5]
		stz [D + 20]
		stz [D + 22]
		stz [D + 24]
		lda [D + 1]
		beq RoundingDoubleZero
		cmp 3
		beq RoundingDoubleSmallRound
		cmp 1
		beq RoundingDoubleSmallFloor
		lda [D + 9]
		bne RoundingDoubleZero
		lda [D + 5]
		beq RoundingDoubleZero
		bra RoundingDoubleOne
RoundingDoubleSmallFloor:
		lda [D + 9]
		beq RoundingDoubleZero
		lda [D + 5]
		beq RoundingDoubleZero
		bra RoundingDoubleOne
RoundingDoubleSmallRound:
		lda [D + 7]
		cmp $3FE0
		bne RoundingDoubleZero
RoundingDoubleOne:
		lda $3FF0
		ora [D + 9]
		sta [D + 26]
		bra RoundingDoubleDone
RoundingDoubleZero:
		lda [D + 9]
		sta [D + 26]
RoundingDoubleDone:
		ldy 0
		lda [D + 20]
		sta [[D + 16]:24 + Y]
		ldy 2
		lda [D + 22]
		sta [[D + 16]:24 + Y]
		ldy 4
		lda [D + 24]
		sta [[D + 16]:24 + Y]
		ldy 6
		lda [D + 26]
		sta [[D + 16]:24 + Y]
		tsc
		clc
		adc 10
		tcs
		pld
		rtl

; A = the word of x holding bit A, with only that bit.
RoundingDoubleBit:
		pha
		and 15
		asl A
		tax
		lda [BitWords + X]:24
		sta [D + 9]
		pla
		lsr A
		lsr A
		lsr A
		lsr A
		asl A
		tax
		lda [D + 20 + X]
		and [D + 9]
		rts

RoundingFloat:
		phd
		tsc
		sec
		sbc 10
		tcs
		tcd
		sty [D + 1]
		stz [D + 5]
		stz [D + 7]
		lda [D + 18]
		and $7F80
		cmp $4B00
		bcc RoundingFloatFraction
		; An integer, infinity or NaN; a NaN is quietened (as the float to double conversion did).
		cmp $7F80
		bne RoundingFloatJump
		lda [D + 18]
		and $007F
		ora [D + 16]
		beq RoundingFloatJump
		lda [D + 18]
		ora $0040
		sta [D + 18]
RoundingFloatJump:
		jmp RoundingFloatDone
RoundingFloatFraction:
		cmp $3F80
		bcs RoundingFloatInteger
		jmp RoundingFloatSmall
RoundingFloatInteger:
		asl A
		xba
		and $00FF
		eor $FFFF
		sec
		adc 150
		sta [D + 3]
		; Half (for round): bit k - 1.
		dec A
		jsr RoundingFloatBit
		sta [D + 7]
		; The k bits below the binary point cleared, Frac their OR.
		ldx 0
		ldy [D + 3]
RoundingFloatWords:
		cpy 16
		bcc RoundingFloatPart
		lda [D + 16 + X]
		ora [D + 5]
		sta [D + 5]
		stz [D + 16 + X]
		inx
		inx
		tya
		sec
		sbc 16
		tay
		bra RoundingFloatWords
RoundingFloatPart:
		tya
		beq RoundingFloatCleared
		asl A
		phx
		tax
		lda [LowBitMasks + X]:24
		plx
		sta [D + 9]
		and [D + 16 + X]
		ora [D + 5]
		sta [D + 5]
		lda [D + 9]
		eor $FFFF
		and [D + 16 + X]
		sta [D + 16 + X]
RoundingFloatCleared:
		; floor: below 0 with a fraction, ceil: above 0 with one, round: Half; one up a unit in
		; the last place left (a carry goes into the exponent).
		lda [D + 1]
		beq RoundingFloatDone
		cmp 3
		beq RoundingFloatRound
		cmp 1
		beq RoundingFloatFloor
		lda [D + 18]
		bmi RoundingFloatDone
		lda [D + 5]
		beq RoundingFloatDone
		bra RoundingFloatUp
RoundingFloatFloor:
		lda [D + 18]
		bpl RoundingFloatDone
		lda [D + 5]
		beq RoundingFloatDone
		bra RoundingFloatUp
RoundingFloatRound:
		lda [D + 7]
		beq RoundingFloatDone
RoundingFloatUp:
		lda [D + 3]
		and 15
		asl A
		tax
		lda [BitWords + X]:24
		sta [D + 9]
		lda [D + 3]
		lsr A
		lsr A
		lsr A
		lsr A
		asl A
		tax
		clc
		lda [D + 16 + X]
		adc [D + 9]
		sta [D + 16 + X]
		bcc RoundingFloatDone
		; The carry on up (the top word, with the exponent, never wraps).
RoundingFloatCarry:
		inx
		inx
		inc [D + 16 + X]
		beq RoundingFloatCarry
		jmp RoundingFloatDone
RoundingFloatSmall:
		; Under 1: 0 with its sign, or 1 with it for floor (below 0), ceil (above 0) and round
		; (a half or more).
		sta [D + 7]
		lda [D + 18]
		and $8000
		sta [D + 9]
		lda [D + 18]
		and $7FFF
		ora [D + 16]
		sta [D + 5]
		stz [D + 16]
		lda [D + 1]
		beq RoundingFloatZero
		cmp 3
		beq RoundingFloatSmallRound
		cmp 1
		beq RoundingFloatSmallFloor
		lda [D + 9]
		bne RoundingFloatZero
		lda [D + 5]
		beq RoundingFloatZero
		bra RoundingFloatOne
RoundingFloatSmallFloor:
		lda [D + 9]
		beq RoundingFloatZero
		lda [D + 5]
		beq RoundingFloatZero
		bra RoundingFloatOne
RoundingFloatSmallRound:
		lda [D + 7]
		cmp $3F00
		bne RoundingFloatZero
RoundingFloatOne:
		lda $3F80
		ora [D + 9]
		sta [D + 18]
		bra RoundingFloatDone
RoundingFloatZero:
		lda [D + 9]
		sta [D + 18]
RoundingFloatDone:
		ldy [D + 16]
		ldx [D + 18]
		tsc
		clc
		adc 10
		tcs
		tya
		pld
		rtl

; A = the word of x holding bit A, with only that bit.
RoundingFloatBit:
		pha
		and 15
		asl A
		tax
		lda [BitWords + X]:24
		sta [D + 9]
		pla
		lsr A
		lsr A
		lsr A
		lsr A
		asl A
		tax
		lda [D + 16 + X]
		and [D + 9]
		rts

; 1 << b and (1 << b) - 1 for b = 0 to 15.
BitWords:
		.word $0001, $0002, $0004, $0008, $0010, $0020, $0040, $0080, $0100, $0200, $0400, $0800, $1000, $2000, $4000, $8000
LowBitMasks:
		.word $0000, $0001, $0003, $0007, $000F, $001F, $003F, $007F, $00FF, $01FF, $03FF, $07FF, $0FFF, $1FFF, $3FFF, $7FFF

; double fabs(double x), long double fabsl(long double x): D on S, D + 6 the result pointer, D + 10
; x.
_fabs:
_fabsl:
		.width (A, X), 16
		phd
		tsc
		tcd
		ldy 0
		lda [D + 10]
		sta [[D + 6]:24 + Y]
		ldy 2
		lda [D + 12]
		sta [[D + 6]:24 + Y]
		ldy 4
		lda [D + 14]
		sta [[D + 6]:24 + Y]
		ldy 6
		lda [D + 16]
		and $7FFF
		sta [[D + 6]:24 + Y]
		pld
		rtl

; float fabsf(float x)
_fabsf:
		.width (A, X), 16
		lda [S + 6]
		and $7FFF
		tax
		lda [S + 4]
		rtl

; double frexp(double x, int* piExponent): the significand from 1/2 to 1 and the power of two (0
; and x itself for 0, infinity and NaN).  A subnormal x is made normal first.  Frame as the
; rounding functions': D + 16 the result pointer, D + 20 x, D + 28 piExponent; D + 1 the shift.
_frexp:
		.width (A, X), 16
		phd
		tsc
		sec
		sbc 10
		tcs
		tcd
		lda [D + 26]
		and $7FF0
		beq FrexpSmall
		cmp $7FF0
		beq FrexpSame
		lsr A
		lsr A
		lsr A
		lsr A
		sec
		sbc 1022
FrexpExponent:
		sta [[D + 28]:24]
		lda [D + 26]
		and $800F
		ora $3FE0
		sta [D + 26]
		bra FrexpDone
FrexpSmall:
		lda [D + 26]
		and $000F
		ora [D + 24]
		ora [D + 22]
		ora [D + 20]
		beq FrexpSame
		; Up until bit 52 is set, the sign kept aside.
		lda [D + 26]
		and $8000
		sta [D + 3]
		lda [D + 26]
		and $000F
		sta [D + 26]
		lda -1021
		sta [D + 1]
FrexpShift:
		dec [D + 1]
		asl [D + 20]
		rol [D + 22]
		rol [D + 24]
		rol [D + 26]
		lda [D + 26]
		and $0010
		beq FrexpShift
		lda [D + 26]
		and $000F
		ora [D + 3]
		sta [D + 26]
		lda [D + 1]
		bra FrexpExponent
FrexpSame:
		lda 0
		sta [[D + 28]:24]
FrexpDone:
		ldy 0
		lda [D + 20]
		sta [[D + 16]:24 + Y]
		ldy 2
		lda [D + 22]
		sta [[D + 16]:24 + Y]
		ldy 4
		lda [D + 24]
		sta [[D + 16]:24 + Y]
		ldy 6
		lda [D + 26]
		sta [[D + 16]:24 + Y]
		tsc
		clc
		adc 10
		tcs
		pld
		rtl


; The rest run on the soft float frame.

; double ldexp(double x, int iExponent)
_ldexp:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetDouble64
		ldx 4
		jsr LoadDoubleA
		ldx 12
		jsr ArgIndex
		lda [D + X]
		sta [D + IndexI]
		jmp LdexpOp

; double modf(double x, double* pdInteger)
_modf:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetDouble64
		ldx 4
		jsr LoadDoubleA
		ldx 12
		jsr ArgIndex
		lda [D + X]
		sta [D + Product]
		lda [D + 2 + X]
		sta [D + Product + 2]
		jmp ModfOp

; double fmod(double x, double y), long double fmodl(long double x, long double y)
_fmod:
_fmodl:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetDouble64
		ldx 4
		jsr LoadDoubles
		jmp FmodOp

; float fmodf(float x, float y)
_fmodf:
		.width (A, X), 16
		phd
		tsc
		tax
		sec
		sbc Locals
		and $FF00
		tcs
		tcd
		jsr Framed
		jsr SetFloat32
		jsr LoadFloats
		jmp FmodOp

; x * 2^n as math.cpp has it: a NaN quietened, 0 and infinity as they are, and otherwise one
; rounding, except that below 2^-1022 it steps (x * 2^-1022 rounded, again until n is -1022 or
; more, then the rest), each step rounded.  n (IndexI) over 2200 is 2200: any finite x is
; infinity by then.
LdexpOp:
		ldx 0
		jsr Unpack
		lda [D + ClassA]
		cmp SoftFinite
		beq LdexpFinite
		ldx 0
		jsr ResultRaw
		lda [D + ClassA]
		cmp SoftNaN
		bne LdexpReturn
		jsr Quieten
LdexpReturn:
		jmp ReturnResult
LdexpFinite:
		lda [D + IndexI]
		bpl LdexpLast
		clc
		adc 1022
		bpl LdexpLast
		sta [D + IndexI]
		lda [D + ExpA]
		sec
		sbc 1022
		sta [D + ExpA]
		jsr RoundPack
		; The step's result unpacked again; a 0 stays 0.
		lda [D + Result]
		sta [D + RawA]
		lda [D + Result + 2]
		sta [D + RawA + 2]
		lda [D + Result + 4]
		sta [D + RawA + 4]
		lda [D + Result + 6]
		sta [D + RawA + 6]
		ldx 0
		jsr Unpack
		lda [D + ClassA]
		beq LdexpReturn
		bra LdexpFinite
LdexpLast:
		lda [D + IndexI]
		bmi LdexpAdd
		cmp 2200
		bcc LdexpAdd
		lda 2200
LdexpAdd:
		clc
		adc [D + ExpA]
		sta [D + ExpA]
		jsr RoundPack
		jmp ReturnResult

; modf: *pdInteger (Product) = trunc(x) (an infinity or NaN as it is), and x less it, exactly: 0
; with x's sign when there is no fraction (or x is infinite), a NaN quietened.
ModfOp:
		; trunc(x) into RawB.
		lda [D + RawA]
		sta [D + RawB]
		lda [D + RawA + 2]
		sta [D + RawB + 2]
		lda [D + RawA + 4]
		sta [D + RawB + 4]
		lda [D + RawA + 6]
		sta [D + RawB + 6]
		and $7FF0
		cmp $4330
		bcs ModfTruncated
		cmp $3FF0
		bcs ModfClear
		stz [D + RawB]
		stz [D + RawB + 2]
		stz [D + RawB + 4]
		lda [D + RawB + 6]
		and $8000
		sta [D + RawB + 6]
		bra ModfTruncated
ModfClear:
		; k = 1075 - E bits cleared, a word at a time then a mask.
		lsr A
		lsr A
		lsr A
		lsr A
		eor $FFFF
		sec
		adc 1075
		tay
		ldx 0
ModfWords:
		cpy 16
		bcc ModfPart
		stz [D + RawB + X]
		inx
		inx
		tya
		sec
		sbc 16
		tay
		bra ModfWords
ModfPart:
		tya
		beq ModfTruncated
		asl A
		phx
		tax
		lda [LowBitMasks + X]:24
		plx
		eor $FFFF
		and [D + RawB + X]
		sta [D + RawB + X]
ModfTruncated:
		ldy 0
		lda [D + RawB]
		sta [[D + Product]:24 + Y]
		ldy 2
		lda [D + RawB + 2]
		sta [[D + Product]:24 + Y]
		ldy 4
		lda [D + RawB + 4]
		sta [[D + Product]:24 + Y]
		ldy 6
		lda [D + RawB + 6]
		sta [[D + Product]:24 + Y]
		; NaN: x - x's truncation is x quietened.  Infinite, or nothing cleared: 0 with x's sign.
		lda [D + RawA + 6]
		and $7FF0
		cmp $7FF0
		bne ModfFinite
		lda [D + RawA + 6]
		and $000F
		ora [D + RawA + 4]
		ora [D + RawA + 2]
		ora [D + RawA]
		beq ModfZero
		ldx 0
		jsr ResultRaw
		jsr Quieten
		jmp ReturnResult
ModfFinite:
		lda [D + RawA]
		cmp [D + RawB]
		bne ModfSubtract
		lda [D + RawA + 2]
		cmp [D + RawB + 2]
		bne ModfSubtract
		lda [D + RawA + 4]
		cmp [D + RawB + 4]
		bne ModfSubtract
		lda [D + RawA + 6]
		cmp [D + RawB + 6]
		bne ModfSubtract
ModfZero:
		jsr ClearResult
		lda [D + RawA + 6]
		and $8000
		sta [D + Result + 6]
		jmp ReturnResult
ModfSubtract:
		lda 1
		sta [D + Subtract]
		jmp AddOp

; The remainder of x / y with x's sign, exactly, as math.cpp's fmod: its (x - x) / (y - y + 0 * x)
; for a 0 y or a NaN or infinite x is x quietened for a NaN x, the default NaN for an infinite x,
; y quietened for a NaN y and the default NaN for a 0 y; an infinite y, or x smaller than y, is x.
; Otherwise both go up to their top bit at 62 and the remainder is taken a bit at a time, x's
; extra exponent's worth.
FmodOp:
		ldx 0
		jsr Unpack
		ldx SideB
		jsr Unpack
		lda [D + ClassA]
		cmp SoftNaN
		bne FmodANotNaN
		ldx 0
FmodQuietened:
		jsr ResultRaw
		jsr Quieten
		jmp ReturnResult
FmodANotNaN:
		cmp SoftInfinity
		beq FmodInvalid
		lda [D + ClassB]
		cmp SoftNaN
		bne FmodBNotNaN
		ldx SideB
		bra FmodQuietened
FmodBNotNaN:
		cmp SoftZero
		bne FmodBNotZero
FmodInvalid:
		jsr DefaultNaN
		jmp ReturnResult
FmodBNotZero:
		cmp SoftInfinity
		beq FmodReturnA
		lda [D + ClassA]
		beq FmodReturnA
		lda 62
		ldx SigA
		jsr NormaliseTo
		lda [D + ExpA]
		sec
		sbc [D + Shift]
		sta [D + ExpA]
		lda 62
		ldx SigB
		jsr NormaliseTo
		lda [D + ExpB]
		sec
		sbc [D + Shift]
		sta [D + ExpB]
		; x smaller than y is x.
		lda [D + ExpA]
		sec
		sbc [D + ExpB]
		bmi FmodReturnA
		sta [D + IndexI]
		bne FmodLoop
		jsr CompareSignificands
		bcc FmodReturnA
FmodLoop:
		jsr CompareSignificands
		bcc FmodNext
		lda [D + SigA]
		sbc [D + SigB]
		sta [D + SigA]
		lda [D + SigA + 2]
		sbc [D + SigB + 2]
		sta [D + SigA + 2]
		lda [D + SigA + 4]
		sbc [D + SigB + 4]
		sta [D + SigA + 4]
		lda [D + SigA + 6]
		sbc [D + SigB + 6]
		sta [D + SigA + 6]
FmodNext:
		lda [D + IndexI]
		beq FmodDone
		dec A
		sta [D + IndexI]
		asl [D + SigA]
		rol [D + SigA + 2]
		rol [D + SigA + 4]
		rol [D + SigA + 6]
		bra FmodLoop
FmodDone:
		; The remainder at y's exponent, exact; 0 has x's sign.
		lda [D + ExpB]
		sta [D + ExpA]
		jsr RoundPack
		jmp ReturnResult
FmodReturnA:
		ldx 0
		jsr ResultRaw
		jmp ReturnResult

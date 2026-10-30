; IEEE 754 single and double precision for the W65C816.  The C++ back end calls these for float
; and double arithmetic, comparisons and conversions; they are softfloat.cpp's functions, which
; stays as their reference (the W65C816 build of it is empty).  The results are the same bit for
; bit: rounding to nearest, ties to even, subnormals, infinities, and NaNs as SSE has them (an
; invalid operation gives the default NaN $FFC00000 or $FFF8000000000000, a NaN operand comes back
; quietened, the first if both are NaNs, and an integer conversion that is out of range or a NaN
; gives the most negative integer).  The code follows softfloat.cpp's steps, with its 64 bit
; integers as four words in the direct page: a value is unpacked to a sign, a class, the exponent
; of its significand's lowest bit and the significand, and results are rounded by RoundPack.
; Products come from Garth Wilson's byte multiplication table (MathTbls/MathMult.asm), as the
; runtime's R816_Mul16 does.
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
Product		= 29		; 128 bits: a product, and the square root's radicand shifted
Result		= 45		; 64 bits: the bits returned
RawA		= 53		; the operands' bits as given (RawB is SideB after RawA)
Fraction	= 61		; the format: 23 or 52 fraction bits
Bias		= 63		; 127 or 1023
MaxExp		= 65		; 255 or 2047
RawB		= 67
Quotient	= 75		; 64 bits: the quotient, and the square root's root
Remainder	= 83		; 64 bits: the square root's remainder
ByteB2		= 75		; multiply: SigB's bytes doubled, 7 words (over Quotient and Remainder)
B7			= 67		; multiply: SigB << 7 (over RawB, not needed by then)
RowEven		= 91		; multiply: a row's even byte products, 4 words
RowOdd		= 100		; multiply: its odd byte products, 3 words, with a 0 byte either side
RowBase		= 119		; multiply: the row byte (less 128) << 9 (Temp2, free while the rows are made)
Count		= 107		; a shift's count
Sticky		= 109		; not 0 when a shift right lost a 1 bit
TopBit		= 111
Biased		= 113
Shift		= 115
Temp		= 117
Temp2		= 119
IndexI		= 121
IndexJ		= 123
Subtract	= 125		; not 0 for a subtraction; ToInteger's signed flag
Return		= 127		; how the result is returned: 0 32 bits, 1 64 bits, 2 16 bits
Bytes		= 129		; the significand's bytes (3 or 7); ToInteger's width in bits
SavedS		= 131		; S on the way in (after phd)
ArgOff		= 133		; the arguments' offset from D (they are 6 bytes above SavedS)
ResultPtr	= 135		; the hidden result pointer, copied to the direct page to write through
Locals		= 138

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
_SoftFloatSqrt:
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
_SoftDoubleSqrt:
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
		lda 127
		sta [D + Bias]
		lda 255
		sta [D + MaxExp]
		lda 3
		sta [D + Bytes]
		rts

; Double, returned through the hidden pointer.
SetDouble64:
		lda 1
		sta [D + Return]
SetFormatDouble:
		lda 52
		sta [D + Fraction]
		lda 1023
		sta [D + Bias]
		lda 2047
		sta [D + MaxExp]
		lda 7
		sta [D + Bytes]
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
; The arguments into RawA and RawB, X bytes after the hidden pointer's place.

LoadFloats:
		ldx [D + ArgOff]
		lda [D + 4 + X]
		sta [D + RawB]
		lda [D + 6 + X]
		sta [D + RawB + 2]
		stz [D + RawB + 4]
		stz [D + RawB + 6]
		bra LoadFloatAt
LoadFloatA:
		jsr ArgIndex
LoadFloatAt:
		lda [D + X]
		sta [D + RawA]
		lda [D + 2 + X]
		sta [D + RawA + 2]
		stz [D + RawA + 4]
		stz [D + RawA + 6]
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
		lda [D + RawA + X]
		sta [D + SigA + X]
		lda [D + RawA + 2 + X]
		sta [D + SigA + 2 + X]
		lda [D + RawA + 4 + X]
		sta [D + SigA + 4 + X]
		lda [D + RawA + 6 + X]
		sta [D + SigA + 6 + X]
		lda [D + Fraction]
		cmp 23
		beq UnpackFloat

		lda [D + SigA + 6 + X]
		and $8000
		sta [D + SignA + X]
		lda [D + SigA + 6 + X]
		and $7FF0
		lsr A
		lsr A
		lsr A
		lsr A
		sta [D + ExpA + X]
		lda [D + SigA + 6 + X]
		and $000F
		sta [D + SigA + 6 + X]
		lda [D + ExpA + X]
		beq UnpackDoubleSmall
		cmp 2047
		beq UnpackTop
		lda [D + SigA + 6 + X]
		ora $0010
		sta [D + SigA + 6 + X]
		lda [D + ExpA + X]
		sec
		sbc 1075
		sta [D + ExpA + X]
UnpackFinite:
		lda SoftFinite
		sta [D + ClassA + X]
		rts
UnpackDoubleSmall:
		jsr UnpackFractionZero
		beq UnpackZero
		lda -1074
		sta [D + ExpA + X]
		bra UnpackFinite

UnpackFloat:
		lda [D + SigA + 2 + X]
		and $8000
		sta [D + SignA + X]
		lda [D + SigA + 2 + X]
		and $7F80
		asl A
		xba
		and $00FF
		sta [D + ExpA + X]
		lda [D + SigA + 2 + X]
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
		clc
		adc [D + Bias]
		sta [D + Biased]
		sec
		sbc [D + MaxExp]
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
		beq RoundPackFloat
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
RoundPackFloat:
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

; The 64 bit value at D + X = 0.
ZeroValue:
		stz [D + X]
		stz [D + 2 + X]
		stz [D + 4 + X]
		stz [D + 6 + X]
		rts


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
		; The top bits at 61, leaving room for a carry.
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
		jsr SwapSides
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

; Swaps side A's Sign, Class, Exponent and significand with side B's.
SwapSides:
		ldx 0
SwapSidesLoop:
		lda [D + SignA + X]
		pha
		lda [D + SignB + X]
		sta [D + SignA + X]
		pla
		sta [D + SignB + X]
		inx
		inx
		cpx SideB
		bcc SwapSidesLoop
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
		jsr MultiplySignificands
		lda [D + ExpA]
		clc
		adc [D + ExpB]
		sta [D + ExpA]

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
		lda [D + Sticky]
		beq MultiplyRound
		lda [D + SigA]
		ora 1
		sta [D + SigA]
MultiplyRound:
		jsr RoundPack
		jmp ReturnResult

; Product = SigA * SigB, from the Bytes low bytes of each, a row at a time: for each byte a of
; SigA, the row a * SigB is put together from byte products (Garth Wilson's table, the word at
; MathMult + ((a << 8) | b) * 2), the even bytes' in RowEven and the odd bytes' in RowOdd so
; none overlap, then added up and into Product a byte further up for each row.  The table is
; read in its low bank only: a byte of 128 or more is a - 128, and 128 * SigB (SigB << 7, made
; once) is added to its row.
MultiplySignificands:
		lda [D + Fraction]
		cmp 23
		bne MultiplyDouble
		jmp MultiplyFloat
MultiplyDouble:
		ldx Product
		jsr ZeroValue
		ldx Product + 8
		jsr ZeroValue
		stz [D + RowOdd - 1]
		stz [D + RowOdd + 5]
		ldx RowEven
		jsr ZeroValue

		; SigB's bytes doubled (the table index's low bits; a float's top four are 0).
		lda [D + SigB]
		and $00FF
		asl A
		sta [D + ByteB2]
		lda [D + SigB + 1]
		and $00FF
		asl A
		sta [D + ByteB2 + 2]
		lda [D + SigB + 2]
		and $00FF
		asl A
		sta [D + ByteB2 + 4]
		lda [D + SigB + 3]
		and $00FF
		asl A
		sta [D + ByteB2 + 6]
		lda [D + SigB + 4]
		and $00FF
		asl A
		sta [D + ByteB2 + 8]
		lda [D + SigB + 5]
		and $00FF
		asl A
		sta [D + ByteB2 + 10]
		lda [D + SigB + 6]
		and $00FF
		asl A
		sta [D + ByteB2 + 12]

		; SigB << 7: up a byte, then down a bit.
		lda [D + SigB + 5]
		sta [D + B7 + 6]
		lda [D + SigB + 3]
		sta [D + B7 + 4]
		lda [D + SigB + 1]
		sta [D + B7 + 2]
		lda [D + SigB]
		xba
		and $FF00
		sta [D + B7]
		lsr [D + B7 + 6]
		ror [D + B7 + 4]
		ror [D + B7 + 2]
		ror [D + B7]

		stz [D + IndexI]
MultiplyRow:
		ldx [D + IndexI]
		lda [D + SigA + X]
		and $00FF
		beq MultiplyNextRow
		sta [D + Temp]
		and $007F
		xba
		asl A
		sta [D + RowBase]
		lda [D + Bytes]
		cmp 7
		beq MultiplyRow7
		jmp MultiplyRow3
MultiplyRow7:
		lda [D + ByteB2 + 12]
		ora [D + RowBase]
		tax
		lda [MathMult + X]:24
		sta [D + RowEven + 6]
		lda [D + ByteB2 + 10]
		ora [D + RowBase]
		tax
		lda [MathMult + X]:24
		sta [D + RowOdd + 4]
		lda [D + ByteB2 + 8]
		ora [D + RowBase]
		tax
		lda [MathMult + X]:24
		sta [D + RowEven + 4]
		lda [D + ByteB2 + 6]
		ora [D + RowBase]
		tax
		lda [MathMult + X]:24
		sta [D + RowOdd + 2]
MultiplyRow3:
		lda [D + ByteB2 + 4]
		ora [D + RowBase]
		tax
		lda [MathMult + X]:24
		sta [D + RowEven + 2]
		lda [D + ByteB2 + 2]
		ora [D + RowBase]
		tax
		lda [MathMult + X]:24
		sta [D + RowOdd]
		lda [D + ByteB2]
		ora [D + RowBase]
		tax
		lda [MathMult + X]:24
		sta [D + RowEven]

		; The row: the even products plus the odd ones a byte up (a word read a byte before
		; RowOdd), plus SigB << 7 for a byte of 128 or more.
		clc
		lda [D + RowEven]
		adc [D + RowOdd - 1]
		sta [D + RowEven]
		lda [D + RowEven + 2]
		adc [D + RowOdd + 1]
		sta [D + RowEven + 2]
		lda [D + RowEven + 4]
		adc [D + RowOdd + 3]
		sta [D + RowEven + 4]
		lda [D + RowEven + 6]
		adc [D + RowOdd + 5]
		sta [D + RowEven + 6]
		lda [D + Temp]
		cmp $0080
		bcc MultiplyAddRow
		clc
		lda [D + RowEven]
		adc [D + B7]
		sta [D + RowEven]
		lda [D + RowEven + 2]
		adc [D + B7 + 2]
		sta [D + RowEven + 2]
		lda [D + RowEven + 4]
		adc [D + B7 + 4]
		sta [D + RowEven + 4]
		lda [D + RowEven + 6]
		adc [D + B7 + 6]
		sta [D + RowEven + 6]

MultiplyAddRow:
		; Into Product, IndexI bytes up.
		ldx [D + IndexI]
		clc
		lda [D + RowEven]
		adc [D + Product + X]
		sta [D + Product + X]
		lda [D + RowEven + 2]
		adc [D + Product + 2 + X]
		sta [D + Product + 2 + X]
		lda [D + RowEven + 4]
		adc [D + Product + 4 + X]
		sta [D + Product + 4 + X]
		lda [D + RowEven + 6]
		adc [D + Product + 6 + X]
		sta [D + Product + 6 + X]
		bcc MultiplyNextRow
MultiplyRowCarry:
		inc [D + Product + 8 + X]
		bne MultiplyNextRow
		inx
		inx
		bra MultiplyRowCarry
MultiplyNextRow:
		lda [D + IndexI]
		inc A
		sta [D + IndexI]
		cmp [D + Bytes]
		bcs MultiplyRowsDone
		jmp MultiplyRow
MultiplyRowsDone:
		rts

; A float's Product: 24 bit significands, so each row a * SigB is 32 bits (the products of b0
; and b2 side by side, b1's a byte up) and the product 48.  As MultiplySignificands otherwise.
MultiplyFloat:
		ldx Product
		jsr ZeroValue
		ldx Product + 8
		jsr ZeroValue
		lda [D + SigB]
		and $00FF
		asl A
		sta [D + ByteB2]
		lda [D + SigB + 1]
		and $00FF
		asl A
		sta [D + ByteB2 + 2]
		lda [D + SigB + 2]
		and $00FF
		asl A
		sta [D + ByteB2 + 4]
		; SigB << 7 (31 bits).
		lda [D + SigB + 1]
		sta [D + B7 + 2]
		lda [D + SigB]
		xba
		and $FF00
		sta [D + B7]
		lsr [D + B7 + 2]
		ror [D + B7]

		stz [D + IndexI]
MultiplyFloatRow:
		ldx [D + IndexI]
		lda [D + SigA + X]
		and $00FF
		beq MultiplyFloatNextRow
		sta [D + Temp]
		and $007F
		xba
		asl A
		sta [D + RowBase]
		ora [D + ByteB2]
		tax
		lda [MathMult + X]:24
		sta [D + RowEven]
		lda [D + ByteB2 + 4]
		ora [D + RowBase]
		tax
		lda [MathMult + X]:24
		sta [D + RowEven + 2]
		lda [D + ByteB2 + 2]
		ora [D + RowBase]
		tax
		lda [MathMult + X]:24
		sta [D + RowOdd]
		; The row: b1's product a byte up.
		xba
		and $FF00
		clc
		adc [D + RowEven]
		sta [D + RowEven]
		lda [D + RowOdd]
		xba
		and $00FF
		adc [D + RowEven + 2]
		sta [D + RowEven + 2]
		lda [D + Temp]
		cmp $0080
		bcc MultiplyFloatAddRow
		clc
		lda [D + RowEven]
		adc [D + B7]
		sta [D + RowEven]
		lda [D + RowEven + 2]
		adc [D + B7 + 2]
		sta [D + RowEven + 2]
MultiplyFloatAddRow:
		ldx [D + IndexI]
		clc
		lda [D + RowEven]
		adc [D + Product + X]
		sta [D + Product + X]
		lda [D + RowEven + 2]
		adc [D + Product + 2 + X]
		sta [D + Product + 2 + X]
		bcc MultiplyFloatNextRow
		inc [D + Product + 4 + X]
MultiplyFloatNextRow:
		lda [D + IndexI]
		inc A
		sta [D + IndexI]
		cmp 3
		bcc MultiplyFloatRow
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
		; Both with their top bit at 62 (a double) or 30 (a float, so it fits two words), so A / B
		; is from 1/2 to 2.
		lda [D + Fraction]
		cmp 23
		beq DivideTop30
		lda 62
		bra DivideTop
DivideTop30:
		lda 30
DivideTop:
		sta [D + TopBit]
		ldx SigA
		jsr NormaliseTo
		lda [D + ExpA]
		sec
		sbc [D + Shift]
		sta [D + ExpA]
		lda [D + TopBit]
		ldx SigB
		jsr NormaliseTo
		lda [D + ExpB]
		sec
		sbc [D + Shift]
		sta [D + ExpB]

		; Long division, a bit at a time, Fraction + 3 bits.  Each bit comes out of the compare
		; (or the subtract) in the carry and is rotated into the quotient's low word; the words are
		; moved up after each 16, the first lot being the bits over a multiple of 16.
		ldx Quotient
		jsr ZeroValue
		lda [D + Fraction]
		clc
		adc 3
		sta [D + IndexJ]
		sta [D + IndexI]
		and 15
		bne DivideFirstWord
		lda 16
DivideFirstWord:
		sta [D + Temp]
		tay
		lda [D + Fraction]
		cmp 23
		bne DivideLoop64
		jmp DivideLoop32

DivideLoop64:
		lda [D + SigA + 6]
		cmp [D + SigB + 6]
		bne DivideDecided64
		lda [D + SigA + 4]
		cmp [D + SigB + 4]
		bne DivideDecided64
		lda [D + SigA + 2]
		cmp [D + SigB + 2]
		bne DivideDecided64
		lda [D + SigA]
		cmp [D + SigB]
DivideDecided64:
		bcc DivideBit64
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
DivideBit64:
		rol [D + Quotient]
		asl [D + SigA]
		rol [D + SigA + 2]
		rol [D + SigA + 4]
		rol [D + SigA + 6]
		dey
		bne DivideLoop64
		jsr DivideNextWord
		bne DivideLoop64
		jmp DivideDone

DivideLoop32:
		lda [D + SigA + 2]
		cmp [D + SigB + 2]
		bne DivideDecided32
		lda [D + SigA]
		cmp [D + SigB]
DivideDecided32:
		bcc DivideBit32
		lda [D + SigA]
		sbc [D + SigB]
		sta [D + SigA]
		lda [D + SigA + 2]
		sbc [D + SigB + 2]
		sta [D + SigA + 2]
DivideBit32:
		rol [D + Quotient]
		asl [D + SigA]
		rol [D + SigA + 2]
		dey
		bne DivideLoop32
		jsr DivideNextWord
		bne DivideLoop32

DivideDone:
		; The quotient with the rest sticky below it.
		ldx SigA
		jsr IsZero
		beq DivideNoRest
		lda 1
DivideNoRest:
		sta [D + Temp]
		lda [D + Quotient]
		asl A
		ora [D + Temp]
		sta [D + SigA]
		lda [D + Quotient + 2]
		rol A
		sta [D + SigA + 2]
		lda [D + Quotient + 4]
		rol A
		sta [D + SigA + 4]
		lda [D + Quotient + 6]
		rol A
		sta [D + SigA + 6]
		lda [D + ExpA]
		sec
		sbc [D + ExpB]
		sec
		sbc [D + IndexJ]
		sta [D + ExpA]
		jsr RoundPack
		jmp ReturnResult

; After a word of quotient bits: Z set when all are done, otherwise the quotient's words moved
; up and Y = 16 for the next.
DivideNextWord:
		lda [D + IndexI]
		sec
		sbc [D + Temp]
		sta [D + IndexI]
		beq DivideNextWordDone
		lda [D + Quotient + 4]
		sta [D + Quotient + 6]
		lda [D + Quotient + 2]
		sta [D + Quotient + 4]
		lda [D + Quotient]
		sta [D + Quotient + 2]
		stz [D + Quotient]
		lda 16
		sta [D + Temp]
		tay
DivideNextWordDone:
		rts

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
		ldx 0
		jsr Unpack
		ldx SideB
		jsr Unpack
		lda [D + ClassA]
		cmp SoftNaN
		beq CompareUnordered
		lda [D + ClassB]
		cmp SoftNaN
		beq CompareUnordered
		lda [D + ClassA]
		ora [D + ClassB]
		beq CompareEqual
		lda [D + SignA]
		cmp [D + SignB]
		beq CompareSameSign
		lda [D + SignA]
		bne CompareLess
		bra CompareGreater
CompareSameSign:
		; The bits without the sign order the magnitudes.
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
		lda [D + Fraction]
		cmp 23
		beq CompareFloatMagnitude
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
		bra FromInteger

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
; The square root, correctly rounded, as softfloat.cpp's SquareRoot.  The square root of -0 is
; -0; of anything else below zero, the default NaN.

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
		; The radicand is the significand times 2^48 (112 bits); in Product it is up 16 more, so its
		; top two bits are always Product's.
		ldx Product
		jsr ZeroValue
		lda [D + SigA]
		sta [D + Product + 8]
		lda [D + SigA + 2]
		sta [D + Product + 10]
		lda [D + SigA + 4]
		sta [D + Product + 12]
		lda [D + SigA + 6]
		sta [D + Product + 14]
		ldx Quotient
		jsr ZeroValue
		ldx Remainder
		jsr ZeroValue

		; A bit of the root for each two bits of the radicand, from the top.
		lda 56
		sta [D + IndexI]
SqrtLoop:
		ldy 2
SqrtTwoBits:
		asl [D + Product]
		rol [D + Product + 2]
		rol [D + Product + 4]
		rol [D + Product + 6]
		rol [D + Product + 8]
		rol [D + Product + 10]
		rol [D + Product + 12]
		rol [D + Product + 14]
		rol [D + Remainder]
		rol [D + Remainder + 2]
		rol [D + Remainder + 4]
		rol [D + Remainder + 6]
		dey
		bne SqrtTwoBits

		; The trial (root << 2) | 1 in SigB.
		lda [D + Quotient]
		asl A
		sta [D + SigB]
		lda [D + Quotient + 2]
		rol A
		sta [D + SigB + 2]
		lda [D + Quotient + 4]
		rol A
		sta [D + SigB + 4]
		lda [D + Quotient + 6]
		rol A
		sta [D + SigB + 6]
		asl [D + SigB]
		rol [D + SigB + 2]
		rol [D + SigB + 4]
		rol [D + SigB + 6]
		lda [D + SigB]
		ora 1
		sta [D + SigB]

		; The root shifted up, its new bit 1 when the remainder reaches the trial.
		asl [D + Quotient]
		rol [D + Quotient + 2]
		rol [D + Quotient + 4]
		rol [D + Quotient + 6]
		lda [D + Remainder + 6]
		cmp [D + SigB + 6]
		bne SqrtCompared
		lda [D + Remainder + 4]
		cmp [D + SigB + 4]
		bne SqrtCompared
		lda [D + Remainder + 2]
		cmp [D + SigB + 2]
		bne SqrtCompared
		lda [D + Remainder]
		cmp [D + SigB]
SqrtCompared:
		bcc SqrtNext
		sec
		lda [D + Remainder]
		sbc [D + SigB]
		sta [D + Remainder]
		lda [D + Remainder + 2]
		sbc [D + SigB + 2]
		sta [D + Remainder + 2]
		lda [D + Remainder + 4]
		sbc [D + SigB + 4]
		sta [D + Remainder + 4]
		lda [D + Remainder + 6]
		sbc [D + SigB + 6]
		sta [D + Remainder + 6]
		inc [D + Quotient]
SqrtNext:
		dec [D + IndexI]
		bne SqrtLoop

		; (root << 1) | (remainder != 0), at (exponent - 48) / 2 - 1.
		ldx Remainder
		jsr IsZero
		beq SqrtExact
		lda 1
SqrtExact:
		sta [D + Temp]
		lda [D + Quotient]
		asl A
		ora [D + Temp]
		sta [D + SigA]
		lda [D + Quotient + 2]
		rol A
		sta [D + SigA + 2]
		lda [D + Quotient + 4]
		rol A
		sta [D + SigA + 4]
		lda [D + Quotient + 6]
		rol A
		sta [D + SigA + 6]
		lda [D + ExpA]
		sec
		sbc 48
		cmp $8000
		ror A
		dec A
		sta [D + ExpA]
		stz [D + SignA]
		jsr RoundPack
		jmp ReturnResult

; IEEE 754 single and double precision for the 65C64, written for speed.  The 65C64
; generator calls these for float and double arithmetic, comparisons and conversions;
; they are softfloat.cpp's functions, which stays as their reference (the 65C64 build of
; it is empty).  The results are the same bit for bit: rounding to nearest, ties to even,
; subnormals, infinities, and NaNs as SSE has them (an invalid operation gives the
; default NaN $FFC00000 or $FFF8000000000000, a NaN operand comes back quietened, the
; first if both are NaNs, and an integer conversion that is out of range or a NaN gives
; the most negative integer).  The double routines follow the floats'.
;
; The calling convention is the generator's: the arguments are on the stack (the first
; at [5 + S] above the return address, the second after it), the result is in A (all 8
; bytes of it for a 64 bit result), A, B, X and Y are 32 bits wide on the way in and
; out, and V, W and S are kept.  A, B, X, Y and the status flags (f and g too) change.
; The pseudo registers are not used.
;
; Significands are worked on in 64 bit registers.  A product is one mul (the 24 bit
; significands make 48 bits), a quotient one div (a 64 bit dividend gives 40 quotient
; bits and the remainder), and a sum is exact because the smaller significand is only
; shifted out of the low zero bits of the larger.  A result that may need rounding goes
; through RoundPack (the hot routines have their own copy of its usual path).
;
; For speed: a branch to a label 2 to 8 bytes on is one word (one cycle), and further
; it is two words (two cycles) taken or not, so the rare cases (special values, results
; out of range) are reached by a short branch over a bra to them.  W is 0 in generated
; code, so mul and div only keep it out of line when it is not.
;
; Cycles per call on the simulator for normal operands from 2^-20 to 2^20, with
; softfloat.cpp's compiled code in brackets: add 80 (2140), subtract 84 (2230),
; multiply 64 (2180), divide 85 (4930), compare 25 (1145), sqrt 134 (16980), from int
; 39 (590), to int 19 (770), to double 21 (1005), double to float 46 (1110).  Doubles:
; add 98 (2135), subtract 104 (2225), multiply 134 (2360), divide 206 (8245), compare 36
; (1140), sqrt 218 (16990), from int 28 (510), from long long 42 (545), to int 24 (750).


; Every routine that rounds (add, subtract, multiply, divide, the conversions from
; integers and doubles, and sqrt) with RoundPack, Unpack, TwoNaNs and the special
; results they share.  They are one section so that they reach them with short
; branches, and so that no routine jumps into another section (a lazily loaded section
; is only entered by a call, with A, B, X and Y 32 bits wide).
		.section text.SoftFloatArithmetic
		.global _SoftFloatAdd, _SoftFloatSubtract, _SoftFloatMultiply, _SoftFloatDivide, _SoftFloatSqrt
		.global _SoftFloatFromInt, _SoftFloatFromUnsigned, _SoftFloatFromLongLong, _SoftFloatFromUnsignedLongLong
		.global _SoftDoubleToFloat

; uint32_t SoftFloatMultiply(uint32_t uiA, uint32_t uiB)
_SoftFloatMultiply:
		cpy A, [5 + S]
		cpy B, [9 + S]
		cpy X, A
		xor X, B
		bist f, (X, 31)
		cpy X, 0
		scb (A, 23), (X, 0), 8
		cpy Y, 0
		scb (B, 23), (Y, 0), 8
		sub X, 1
		cmp X, 254
		brac c, NotMultiplySpecial1
		bra MultiplySpecial

NotMultiplySpecial1:
		sub Y, 1
		cmp Y, 254
		brac c, NotMultiplySpecial2
		bra MultiplySpecial

NotMultiplySpecial2:
		add X, Y
		shl A, 8
		stab (A, 31), 1
		shl B, 8
		stab (B, 31), 1
		shr B, 8

; A is a's significand shifted up 8, B is b's significand and X the sum of the exponents
; less 2.  W is 0 in generated code, so the general case (W kept in the top of X) is out
; of line.  The product goes on into RoundPack.
MultiplySignificands:
		tst W, W
		bras z, MultiplyW0
		bra MultiplyKeepW

MultiplyW0:
		wth Y, 8
		mul						; from 2^54 to 2^56
		cpy W, 0
		sub X, 125

; Rounds and packs a finite result and returns from the routine that jumped here.
;   Y	the significand, 64 bits wide, with its top bit at 55 or 54
;   X	the biased exponent of the result less 1 (the significand's top bit adds it back)
;		if the top bit is at 55
;   f	the sign
; A and B are 32 bits wide.  Y is 32 bits wide again on return.
RoundPack:
		bist c, (Y, 55)
		bras c, RoundTop
		shl Y, 1
		sub X, 1

RoundTop:
		cmp X, 254
		brac c, NotRoundOutside1
		bra RoundOutside

NotRoundOutside1:

; Nearest, ties to even: add a half, and if that leaves the low half zero it was a tie
; and the bottom bit is cleared.  Rounding up to 2^24 carries into the exponent.
RoundNearest:
		add Y, $80000000
		scp (Y, 4), (A, 0), 4
		wth Y, 4
		tst Y, Y
		brac z, RoundPacked
		and A, -2

RoundPacked:
		shl X, 23
		add A, X
		stab (A, 31), f
		ret

; X is negative (a subnormal result, or zero) or at least 254 (too large).
RoundOutside:
		tst X, X
		brac n, RoundOverflow
		neg X, X				; the extra shift right of a subnormal
		cmp X, 57
		brac c, RoundShift
		cpy Y, 1				; all shifted out: only a sticky bit is left
		bra RoundSubnormal

RoundShift:
		wth A, 8
		cpy A, Y
		shr Y, X
		neg X, X
		add X, 64
		shl A, X				; the bits shifted out, at the top
		tst A, A
		wth A, 4
		bras z, RoundSubnormal
		or Y, 1

RoundSubnormal:
		cpy X, 0
		bra RoundNearest

RoundOverflow:
		wth Y, 4
		cpy A, $7F800000
		stab (A, 31), f
		ret


; A float in A that is finite and not zero becomes its significand in A (the top bit at
; 23) and its exponent in X (below 1 for a subnormal).  Only A and X change.
Unpack:
		cpy X, 0
		scb (A, 23), (X, 0), 8
		and A, $7FFFFF
		tst X, X
		bras z, UnpackSubnormal
		stab (A, 23), 1
		ret

UnpackSubnormal:
		cpy X, 22
		cep c
		fsbi A, X, -1			; X = the top bit
		neg X, X
		add X, 23
		shl A, X
		neg X, X
		add X, 1
		ret


; Called (jsr) from the special cases of a routine with two float arguments.  If either
; is a NaN it returns from the routine with the first NaN, quietened.  Otherwise it
; returns with only X changed.
TwoNaNs:
		cpy X, [9 + S]
		shl X, 1
		cmp X, $FF000001
		brac c, TwoNaNsSecond
		cpy A, [9 + S]
		bra TwoNaNsQuiet

TwoNaNsSecond:
		cpy X, [13 + S]
		shl X, 1
		cmp X, $FF000001
		brac c, TwoNaNsNone
		cpy A, [13 + S]

TwoNaNsQuiet:
		or A, $400000
		add S, 4

TwoNaNsNone:
		ret


; Results that are jumped to: they return from the routine.
DefaultNaN:
		cpy A, $FFC00000
		ret

SignedInfinity:
		cpy A, $7F800000
		stab (A, 31), f
		ret

SignedZero:
		cpy A, 0
		stab (A, 31), f
		ret


; uint32_t SoftFloatAdd(uint32_t uiA, uint32_t uiB) and SoftFloatSubtract.  Subtract
; adds b with its sign changed; a NaN b comes back from the stack, unchanged.
_SoftFloatSubtract:
		cpy A, [5 + S]
		cpy B, [9 + S]
		xor B, $80000000
		bra AddSigned

_SoftFloatAdd:
		cpy A, [5 + S]
		cpy B, [9 + S]

; The larger magnitude goes in A.
AddSigned:
		cpy X, A
		shl X, 1
		cpy Y, B
		shl Y, 1
		cmp X, Y
		bras c, AddOrdered
		xor A, B
		xor B, A
		xor A, B
		xor X, Y
		xor Y, X
		xor X, Y

AddOrdered:
		bist f, (A, 31)			; the result's sign (unless it cancels)
		shr X, 24				; the exponents
		shr Y, 24
		cmp X, 255
		brac z, NotAddLargerSpecial1
		bra AddLargerSpecial

NotAddLargerSpecial1:
		tst Y, Y
		brac z, NotAddSmallerSpecial1
		bra AddSmallerSpecial

NotAddSmallerSpecial1:
		sub Y, X
		neg Y, Y				; how far apart they are
		wtth (A, B), 8
		bist c, (B, 31)
		and A, $7FFFFF
		stab (A, 23), 1
		and B, $7FFFFF
		stab (B, 23), 1

; A and B are the significands (64 bits wide), X the larger's exponent, Y the
; difference of the exponents, f the larger's sign and c the smaller's.
AddSignificands:
		braso (f, c), AddDifferent

; The larger with its top bit at 54 and the smaller lined up with it.  Its low 31 bits
; are zero so nothing is lost below 32 apart, and further than that the smaller only
; decides the rounding, which a single sticky bit does the same.
		shl A, 31
		shl B, 31
		cmp Y, 32
		brac c, AddSameShift
		cpy B, 1
		bra AddSame

AddSameShift:
		shr B, Y

AddSame:
		add A, B
		wth Y, 8
		cpy Y, A
		wtth (A, B), 4
		bist c, (Y, 55)
		bras c, AddSameTop
		shl Y, 1
		sub X, 1

AddSameTop:
		cmp X, 254
		brac c, NotRoundOutside2
		bra RoundOutside

NotRoundOutside2:
		add Y, $80000000
		scp (Y, 4), (A, 0), 4
		wth Y, 4
		tst Y, Y
		brac z, AddSamePacked
		and A, -2

AddSamePacked:
		shl X, 23
		add A, X
		stab (A, 31), f
		ret

; The signs are different: the larger's top bit at 55.  Two or more apart, the
; difference only loses its top bit if anything.
AddDifferent:
		shl A, 32
		shl B, 32
		cmp Y, 2
		bras c, NotAddClose1
		bra AddClose

NotAddClose1:
		cmp Y, 33
		brac c, AddDifferentShift
		cpy B, 1
		bra AddDifferentSubtract

AddDifferentShift:
		shr B, Y

AddDifferentSubtract:
		sub A, B
		wth Y, 8
		cpy Y, A
		wtth (A, B), 4
		sub X, 1
		bist c, (Y, 55)
		bras c, AddDifferentTop
		shl Y, 1
		sub X, 1

AddDifferentTop:
		cmp X, 254
		brac c, NotRoundOutside3
		bra RoundOutside

NotRoundOutside3:
		add Y, $80000000
		scp (Y, 4), (A, 0), 4
		wth Y, 4
		tst Y, Y
		brac z, AddDifferentPacked
		and A, -2

AddDifferentPacked:
		shl X, 23
		add A, X
		stab (A, 31), f
		ret

; Within one of each other the difference is exact but may lose any number of bits (or
; all of them): its top bit is found and moved to 55.
AddClose:
		shr B, Y
		sub A, B
		bras z, AddCancelled
		wth Y, 8
		cpy Y, A
		wtth (A, B), 4
		cpy B, 55
		cep c
		fsbi Y, B, -1
		neg B, B
		add B, 55
		shl Y, B
		sub X, B
		sub X, 1
		cmp X, 254
		brac c, NotRoundOutside4
		bra RoundOutside

NotRoundOutside4:
		add Y, $80000000
		scp (Y, 4), (A, 0), 4
		wth Y, 4
		tst Y, Y
		brac z, AddClosePacked
		and A, -2

AddClosePacked:
		shl X, 23
		add A, X
		stab (A, 31), f
		ret

AddCancelled:
		wtth (A, B), 4
		cpy A, 0
		ret

; The smaller is zero or subnormal (X is the larger's exponent).
AddSmallerSpecial:
		bist g, (B, 31)
		cpy Y, B
		shl Y, 1
		bras z, AddSmallerZero
		tst X, X
		bras z, AddSubnormals
		cpy Y, X
		sub Y, 1
		wtth (A, B), 8
		stat c, g
		and A, $7FFFFF
		stab (A, 23), 1
		and B, $7FFFFF
		bra AddSignificands

; Adding zero gives the other, and zeros add to -0 only when both are -0.
AddSmallerZero:
		cpy X, A
		shl X, 1
		brac z, AddReturnA
		cpy A, 0
		stat c, g
		brasa (f, c), AddNegativeZero

AddReturnA:
		ret

AddNegativeZero:
		cpy A, $80000000
		ret

; Two subnormals add exactly, carrying into the exponent if they must.
AddSubnormals:
		shl A, 1
		shr A, 1
		shl B, 1
		shr B, 1
		stat c, f
		braso (g, c), AddSubnormalsDifferent
		add A, B
		stab (A, 31), f
		ret

AddSubnormalsDifferent:
		sub A, B
		bras z, AddReturnA
		stab (A, 31), f
		ret

; The larger is infinite or a NaN.
AddLargerSpecial:
		bist g, (B, 31)
		jsr TwoNaNs
		cmp Y, 255
		brac z, AddReturnA
		stat c, f
		braso (g, c), DefaultNaN
		ret


; Either is zero, subnormal, infinite or a NaN.
MultiplySpecial:
		jsr TwoNaNs
		cpy A, [5 + S]
		cpy B, [9 + S]
		cpy X, A
		shl X, 1
		cpy Y, B
		shl Y, 1
		cmp X, $FF000000
		bras z, MultiplyInfinite
		cmp Y, $FF000000
		bras z, MultiplyInfiniteB
		tst X, X
		bras z, SignedZero
		tst Y, Y
		bras z, SignedZero
		jsr Unpack
		psh (A, X)
		cpy A, B
		jsr Unpack
		cpy B, A
		cpy Y, X
		pul (A, X)
		shl A, 8
		add X, Y
		sub X, 2
		bra MultiplySignificands

MultiplyInfinite:
		tst Y, Y
		bras z, DefaultNaN		; infinity times zero
		bra SignedInfinity

MultiplyInfiniteB:
		tst X, X
		bras z, DefaultNaN
		bra SignedInfinity

MultiplyKeepW:
		scp (W, 0), (X, 4), 4
		cpy W, 0
		wth Y, 8
		mul
		scp (X, 4), (W, 0), 4
		sub X, 125
		bra RoundPack


; uint32_t SoftFloatDivide(uint32_t uiA, uint32_t uiB)
_SoftFloatDivide:
		cpy A, [5 + S]
		cpy B, [9 + S]
		cpy X, A
		xor X, B
		bist f, (X, 31)
		cpy X, 0
		scb (A, 23), (X, 0), 8
		cpy Y, 0
		scb (B, 23), (Y, 0), 8
		sub X, 1
		cmp X, 254
		brac c, NotDivideSpecial1
		bra DivideSpecial

NotDivideSpecial1:
		sub Y, 1
		cmp Y, 254
		brac c, NotDivideSpecial2
		bra DivideSpecial

NotDivideSpecial2:
		sub X, Y
		wth Y, 8
		cpy Y, 0
		scb (A, 0), (Y, 40), 23
		stab (Y, 63), 1			; a's significand shifted up 40
		cpy A, 0
		scb (B, 0), (A, 0), 23
		stab (A, 23), 1			; b's significand

; Y (64 bits wide) is a's significand shifted up 40, A is b's significand and X the
; difference of the exponents.  The quotient has 40 or 41 bits.  W is 0 in generated
; code; otherwise it is kept in the top of X.
DivideSignificands:
		cpy B, 0
		tst W, W
		bras z, DivideW0
		bra DivideKeepW

DivideW0:
		div
		cpy W, 0

DivideQuotient:
		shl Y, 15
		tst B, B
		bras z, DivideExact
		or Y, 1					; the remainder is the sticky bit

DivideExact:
		add X, 126
		bist c, (Y, 55)
		bras c, DivideTop
		shl Y, 1
		sub X, 1

DivideTop:
		cmp X, 254
		brac c, NotRoundOutside5
		bra RoundOutside

NotRoundOutside5:
		add Y, $80000000
		scp (Y, 4), (A, 0), 4
		wth Y, 4
		tst Y, Y
		brac z, DividePacked
		and A, -2

DividePacked:
		shl X, 23
		add A, X
		stab (A, 31), f
		ret

; Either is zero, subnormal, infinite or a NaN.
DivideSpecial:
		jsr TwoNaNs
		cpy A, [5 + S]
		cpy B, [9 + S]
		cpy X, A
		shl X, 1
		cpy Y, B
		shl Y, 1
		cmp X, $FF000000
		bras z, DivideInfinite
		cmp Y, $FF000000
		bras z, SignedZero		; over infinity
		tst Y, Y
		bras z, DivideByZero
		tst X, X
		bras z, SignedZero
		jsr Unpack
		psh (A, X)
		cpy A, B
		jsr Unpack				; A = b's significand
		pul (B, Y)				; B = a's significand, Y = a's exponent
		sub Y, X
		cpy X, Y
		wth Y, 8
		cpy Y, B
		shl Y, 40
		bra DivideSignificands

DivideInfinite:
		cmp Y, $FF000000
		bras z, DefaultNaN		; infinity over infinity
		bra SignedInfinity

DivideByZero:
		tst X, X
		bras z, DefaultNaN		; zero over zero
		bra SignedInfinity

DivideKeepW:
		scp (W, 0), (X, 4), 4
		cpy W, 0
		div
		scp (X, 4), (W, 0), 4
		bra DivideQuotient


; uint32_t SoftFloatFromInt(int32_t i), SoftFloatFromUnsigned(uint32_t ui),
; SoftFloatFromLongLong(int64_t lli) and SoftFloatFromUnsignedLongLong(uint64_t ulli).
_SoftFloatFromInt:
		cpy A, [5 + S]
		bras z, FromDone
		abs A, A
		stat f, n
		bra FromMagnitude32

_SoftFloatFromUnsigned:
		cpy A, [5 + S]
		bras z, FromDone
		cep f

; A is a magnitude of up to 32 bits, not zero, and f the sign.  Below 2^24 it is exact.
FromMagnitude32:
		cpy B, 31
		cep c
		fsbi A, B, -1			; B = the top bit
		cpy X, B
		add X, 126
		cmp B, 24
		brac c, NotFromRound321
		bra FromRound32

NotFromRound321:

; A's top bit is at B (below 24), X is the exponent less 1.
FromExact:
		neg B, B
		add B, 23
		shl A, B
		shl X, 23
		add A, X
		stab (A, 31), f

FromDone:
		ret

FromRound32:
		wth Y, 8
		cpy Y, A
		neg B, B
		add B, 55
		shl Y, B
		bra RoundTop

_SoftFloatFromLongLong:
		wth Y, 8
		cpy Y, [5 + S]
		bras z, FromZeroWide
		abs Y, Y
		stat f, n
		bra FromMagnitude

_SoftFloatFromUnsignedLongLong:
		wth Y, 8
		cpy Y, [5 + S]
		bras z, FromZeroWide
		cep f

; Y (64 bits wide) is the magnitude, not zero, and f the sign.  Its top bit is moved
; to 55, keeping a sticky bit if that is a shift right.
FromMagnitude:
		cpy B, 63
		cep c
		fsbi Y, B, -1
		cpy X, B
		add X, 126
		cmp B, 24
		brac c, FromSmall
		cmp B, 56
		brac c, FromShiftLeft
		sub B, 55
		wth A, 8
		cpy A, Y
		shr Y, B
		neg B, B
		add B, 64
		shl A, B
		tst A, A
		wth A, 4
		bras z, FromRound
		or Y, 1

FromRound:
		bra RoundTop

FromShiftLeft:
		neg B, B
		add B, 55
		shl Y, B
		bra RoundTop

FromSmall:
		cpy A, Y
		wth Y, 4
		bra FromExact

FromZeroWide:
		wth Y, 4

FromZero:
		cpy A, 0
		ret


; uint32_t SoftDoubleToFloat(uint64_t ulli).  NaNs keep the top of their payload and
; are quietened; a double subnormal is far below the smallest float.
_SoftDoubleToFloat:
		wth Y, 8
		cpy Y, [5 + S]
		bist f, (Y, 63)
		cpy X, 0
		scb (Y, 52), (X, 0), 11
		sub X, 1
		cmp X, 2046
		brac c, NotDoubleToFloatSpecial1
		bra DoubleToFloatSpecial

NotDoubleToFloatSpecial1:
		shl Y, 12
		shr Y, 9
		stab (Y, 55), 1			; the significand with its top bit at 55
		sub X, 896				; 1023 - 127, and RoundPack's 1
		bra RoundTop

DoubleToFloatSpecial:
		tst X, X
		bras n, DoubleToFloatZero
		shl Y, 12
		bras z, DoubleToFloatInfinity
		shr Y, 41
		cpy A, Y
		or A, $7FC00000
		bra DoubleToFloatSign

DoubleToFloatInfinity:
		cpy A, $7F800000
		bra DoubleToFloatSign

DoubleToFloatZero:
		cpy A, 0

DoubleToFloatSign:
		wth Y, 4
		stab (A, 31), f
		ret


; uint32_t SoftFloatSqrt(uint32_t ui), correctly rounded.  The root of -0 is -0 and of
; anything else below zero the default NaN.  The significand shifted up 25 or 26 (so the
; power of two left is even) is M, from 2^48 to 2^50, and its integer square root r
; (25 bits) comes from a table and two Newton steps (a div each), corrected down with
; one mul.  r and whether r * r is M round as any other result.
_SoftFloatSqrt:
		cpy A, [5 + S]
		cpy X, A
		sub X, $800000
		cmp X, $7F000000
		brac c, NotSqrtSpecial1		; not a positive normal
		bra SqrtSpecial

NotSqrtSpecial1:
		cpy X, 0
		scb (A, 23), (X, 0), 8
		and A, $7FFFFF
		stab (A, 23), 1

; A is the significand (the top bit at 23) and X the exponent.  M is the significand
; shifted up 25 or 26; the significand shifted up 0 or 1 is kept in the top half of X
; to make M again, and W in the top half of A (divide and multiply only use their low
; halves).
SqrtSignificand:
		cep f
		bist c, (X, 0)
		bras c, SqrtOdd
		shl A, 1
		add X, 124				; (exponent - 26 + 150) / 2 is RoundPack's
		bra SqrtRoot

SqrtOdd:
		add X, 125

SqrtRoot:
		shr X, 1
		scp (A, 0), (X, 4), 4
		scp (W, 0), (A, 4), 4
		wth Y, 8
		cpy Y, A
		shl Y, 25				; M
		cpy B, A
		shr B, 16
		and B, -4				; 4 (M >> 43)
		cpy A, [SqrtTable - 128 + B]
		cpy B, 0
		cpy W, 0
		div
		add Y, A
		shr Y, 1
		cpy A, Y
		cpy Y, 0
		scp (X, 4), (Y, 0), 4
		shl Y, 25
		cpy B, 0
		cpy W, 0
		div
		add Y, A
		shr Y, 1
		cpy A, Y				; r, or r + 1
		cpy B, A
		cpy W, 0
		mul
		scp (A, 4), (W, 0), 4
		wth B, 8
		cpy B, 0
		scp (X, 4), (B, 0), 4
		shl B, 25

SqrtCorrect:
		cmp B, Y
		bras c, SqrtCorrected
		sub Y, A				; (r - 1)^2 = r^2 - 2r + 1
		sub Y, A
		add Y, 1
		sub A, 1
		bra SqrtCorrect

SqrtCorrected:
		sub B, Y				; the remainder
		cpy Y, A
		shl Y, 31				; the top bit at 55
		tst B, B
		bras z, SqrtExact
		or Y, 1

SqrtExact:
		wth B, 4
		bra RoundTop

SqrtSpecial:
		cpy X, A
		shl X, 1
		bras z, SqrtReturn		; -0 and +0
		cmp X, $FF000001
		bras c, SqrtNaN
		tst A, A
		bras n, DefaultNaN
		cmp X, $FF000000
		bras z, SqrtReturn		; +infinity
		jsr Unpack
		bra SqrtSignificand

SqrtNaN:
		or A, $400000

SqrtReturn:
		ret

; int SoftFloatCompare(uint32_t uiA, uint32_t uiB): -1, 0 or 1 as a is less than, equal
; to or greater than b, and 2 if either is a NaN.  Comparing the magnitudes first says
; which can be a NaN (the larger) and whose sign decides; equal magnitudes are equal
; values unless their signs differ (and they are not zeros).
		.section text._SoftFloatCompare
		.global _SoftFloatCompare
_SoftFloatCompare:
		cpy A, [5 + S]
		cpy B, [9 + S]
		cpy X, A
		shl X, 1
		cpy Y, B
		shl Y, 1
		cmp X, Y
		brac c, CompareSmaller
		brac z, NotCompareSame1
		bra CompareSame

NotCompareSame1:
		cmp X, $FF000001
		brac c, NotCompareNaN1
		bra CompareNaN

NotCompareNaN1:
		bist c, (A, 31)			; |a| > |b|: 1 if a is positive
		cpy A, 1
		brac c, CompareDone
		cpy A, -1

CompareDone:
		ret

CompareSmaller:
		cmp Y, $FF000001
		brac c, NotCompareNaN2
		bra CompareNaN

NotCompareNaN2:
		bist c, (B, 31)			; |a| < |b|: -1 if b is positive
		cpy A, -1
		brac c, CompareDone
		cpy A, 1
		ret

CompareSame:
		cmp X, $FF000001
		brac c, NotCompareNaN3
		bra CompareNaN

NotCompareNaN3:
		tst X, X
		bras z, CompareEqual	; -0 and +0
		xor B, A
		brac n, CompareEqual	; the same sign
		bist c, (A, 31)
		cpy A, 1
		brac c, CompareDone
		cpy A, -1
		ret

CompareEqual:
		cpy A, 0
		ret

CompareNaN:
		cpy A, 2
		ret


; int32_t SoftFloatToInt(uint32_t ui), uint32_t SoftFloatToUnsigned(uint32_t ui),
; int64_t SoftFloatToLongLong(uint32_t ui) and uint64_t SoftFloatToUnsignedLongLong(uint32_t ui),
; towards zero.  The significand at the top of the register is shifted down to the
; units.
		.section text._SoftFloatToInt
		.global _SoftFloatToInt, _SoftFloatToUnsigned, _SoftFloatToLongLong, _SoftFloatToUnsignedLongLong
_SoftFloatToInt:
		cpy A, [5 + S]
		cpy X, 0
		scb (A, 23), (X, 0), 8
		sub X, 127				; the power of two of the top bit
		cmp X, 31
		brac c, NotToIntOutside1
		bra ToIntOutside

NotToIntOutside1:
		cpy B, A
		shl A, 8
		stab (A, 31), 1
		neg X, X
		add X, 31
		shr A, X
		tst B, B
		brac n, ToIntDone
		neg A, A

ToIntDone:
		ret

; Below 1 (X is negative), or too large, infinite or a NaN.  -2^31 is the indefinite
; integer too.
ToIntOutside:
		tst X, X
		bras n, ToZero

ToIndefinite:
		cpy A, $80000000
		ret

ToZero:
		cpy A, 0
		ret

_SoftFloatToUnsigned:
		cpy A, [5 + S]
		cpy X, 0
		scb (A, 23), (X, 0), 8
		sub X, 127
		bras n, ToZero			; below 1, -1 to 0 included
		tst A, A
		bras n, ToIndefinite
		cmp X, 32
		bras c, ToIndefinite
		shl A, 8
		stab (A, 31), 1
		neg X, X
		add X, 31
		shr A, X
		ret

_SoftFloatToLongLong:
		cpy A, [5 + S]
		bist f, (A, 31)
		cpy X, 0
		scb (A, 23), (X, 0), 8
		sub X, 127
		cmp X, 63
		brac c, NotToLongOutside1
		bra ToLongOutside

NotToLongOutside1:
		wth Y, 8
		cpy Y, 0
		scb (A, 0), (Y, 40), 23
		stab (Y, 63), 1
		neg X, X
		add X, 63
		shr Y, X
		wth A, 8
		cpy A, Y
		brac f, ToLongDone
		neg A, A

ToLongDone:
		wtth (A, Y), 4
		ret

ToLongOutside:
		tst X, X
		bras n, ToLongZero

ToLongIndefinite:
		wth A, 8
		cpy A, 0
		stab (A, 63), 1
		wth A, 4
		ret

ToLongZero:
		wth A, 8
		cpy A, 0
		wth A, 4
		ret

_SoftFloatToUnsignedLongLong:
		cpy A, [5 + S]
		cpy X, 0
		scb (A, 23), (X, 0), 8
		sub X, 127
		bras n, ToLongZero
		tst A, A
		bras n, ToLongIndefinite
		cmp X, 64
		bras c, ToLongIndefinite
		wth Y, 8
		cpy Y, 0
		scb (A, 0), (Y, 40), 23
		stab (Y, 63), 1
		neg X, X
		add X, 63
		shr Y, X
		wth A, 8
		cpy A, Y
		wtth (A, Y), 4
		ret


; uint64_t SoftFloatToDouble(uint32_t ui), exact.  NaNs keep the top of their payload
; and are quietened.
		.section text._SoftFloatToDouble
		.global _SoftFloatToDouble
_SoftFloatToDouble:
		cpy A, [5 + S]
		cpy X, 0
		scb (A, 23), (X, 0), 8
		sub X, 1
		cmp X, 254
		brac c, NotToDoubleSpecial1
		bra ToDoubleSpecial

NotToDoubleSpecial1:
		add X, 897				; 1023 - 127, and the 1 taken off

; The sign, exponent and fraction of A (32 bits wide) move to their places in a double,
; then X (11 bits) is written over the exponent.
ToDoublePack:
		wthx A, 8
		shl A, 29
		scb (A, 60), (A, 63), 1
		scb (X, 0), (A, 52), 11
		wth A, 4
		ret

ToDoubleSpecial:
		tst X, X
		bras n, ToDoubleSmall
		cpy X, $7FF
		cpy Y, A
		shl Y, 9
		bras z, ToDoublePack	; an infinity
		wthx A, 8
		shl A, 29
		scb (A, 60), (A, 63), 1
		scb (X, 0), (A, 52), 11
		stab (A, 51), 1			; a NaN, quietened
		wth A, 4
		ret

; Zero, or a subnormal: its top bit becomes the implicit one.
ToDoubleSmall:
		bist f, (A, 31)
		shl A, 9
		bras z, ToDoubleZero
		cpy X, 31
		cep c
		fsbi A, X, -1
		wthx A, 8
		cpy Y, 52
		sub Y, X
		shl A, Y
		add X, 865				; the top bit at 9 is 2^-149
		scb (X, 0), (A, 52), 11
		stab (A, 63), f
		wth A, 4
		ret

ToDoubleZero:
		wth A, 8
		cpy A, 0
		stab (A, 63), f
		wth A, 4
		ret


; Double precision.  Significands are 53 bits, so a product is four 32 bit muls (the
; 106 bit product's top 64 bits and whether the rest is zero), a quotient five divs of
; 11 bits each (long division in base 2^11), and a sum keeps a sticky bit for what
; shifts out of the smaller (there are only 9 or 10 spare low bits).  DRoundPack rounds
; at bit 10.  The arguments are 8 bytes each: the first at [5 + S], the second at
; [13 + S].  A double result is all 8 bytes of A.
		.section text.SoftDoubleArithmetic
		.global _SoftDoubleAdd, _SoftDoubleSubtract, _SoftDoubleMultiply, _SoftDoubleDivide, _SoftDoubleSqrt
		.global _SoftDoubleFromInt, _SoftDoubleFromUnsigned, _SoftDoubleFromLongLong, _SoftDoubleFromUnsignedLongLong

; uint64_t SoftDoubleMultiply(uint64_t ulliA, uint64_t ulliB)
_SoftDoubleMultiply:
		wtth (A, B), 8
		wtth (X, Y), 8
		cpy A, [5 + S]
		cpy B, [13 + S]
		cpy Y, A
		xor Y, B
		bist f, (Y, 63)
		cpy X, 0
		scb (A, 52), (X, 0), 11
		cpy Y, 0
		scb (B, 52), (Y, 0), 11
		wtth (X, Y), 4
		sub X, 1
		cmp X, 2046
		brac c, NotDMultiplySpecial1
		bra DMultiplySpecial

NotDMultiplySpecial1:
		sub Y, 1
		cmp Y, 2046
		brac c, NotDMultiplySpecial2
		bra DMultiplySpecial

NotDMultiplySpecial2:
		add X, Y
		sub X, 1021
		shl A, 12
		shr A, 12
		stab (A, 52), 1
		shl B, 12
		shr B, 12
		stab (B, 52), 1

; A and B (64 bits wide) are the significands, and [5 + S] has a's low half.  X is the
; exponent for DRoundPack.  With a = aH 2^32 + aL and
; b = bH 2^32 + bL the product is aH bH 2^64 + T 2^32 + low(aL bL), T = aH bL + aL bH +
; (aL bL >> 32), and its top 64 bits are (aH bH << 21) + (T >> 11).  W and X wait on
; the stack (a's low half is then at [13 + S]); g says aL bL's low half is zero.
DMultiplySignificands:
		psh (W, X)
		wtth (A, B), 4
		wtth (X, Y), 8
		cpy W, 0
		mul						; aL bL
		wth Y, 4
		tst Y, Y
		stat g, z
		wth Y, 8
		cpy X, 0
		scp (Y, 4), (X, 0), 4
		scp (A, 4), (A, 0), 4	; aH
		cpy W, 0
		mul						; aH bL
		add X, Y
		scp (B, 4), (B, 0), 4	; bH
		cpy A, [13 + S]			; aL
		cpy W, 0
		mul						; aL bH
		add X, Y				; T
		scp (A, 4), (A, 0), 4	; aH
		cpy W, 0
		mul						; aH bH
		shl Y, 21
		cpy B, X
		and B, $7FF
		shr X, 11
		add Y, X				; from 2^61 to 2^63
		tst B, B
		brac z, DMultiplySticky
		brac g, DMultiplySticky
		bra DMultiplyRound

DMultiplySticky:
		or Y, 1

DMultiplyRound:
		wth X, 4
		pul (W, X)

; Rounds and packs a finite double and returns from the routine that jumped here.
;   Y	the significand, 64 bits wide, with its top bit at 62 or 61
;   X	the biased exponent less 1 if the top bit is at 62
;   f	the sign
; A and B are 32 bits wide.  The result is in A, and A, B, X and Y are 32 bits wide.
DRoundPack:
		bist c, (Y, 62)
		bras c, DRoundTop
		shl Y, 1
		sub X, 1

DRoundTop:
		cmp X, 2046
		brac c, DRoundNearest
		bra DRoundOutside

DRoundNearest:
		add Y, 512
		tst Y, $3FF
		brac z, DRoundShift
		stab (Y, 10), 0			; a tie: to even

DRoundShift:
		shr Y, 10
		wth A, 8
		cpy A, X
		shl A, 52
		add A, Y
		stab (A, 63), f
		wtth (A, B), 4
		wtth (X, Y), 4
		ret

; X is negative (a subnormal result, or zero) or at least 2046 (too large).
DRoundOutside:
		tst X, X
		brac n, DRoundOverflow
		neg X, X
		cmp X, 64
		brac c, DRoundSubnormalShift
		cpy Y, 1
		bra DRoundSubnormal

DRoundSubnormalShift:
		wth A, 8
		cpy A, Y
		shr Y, X
		neg X, X
		add X, 64
		shl A, X
		tst A, A
		bras z, DRoundSubnormal
		or Y, 1

DRoundSubnormal:
		cpy X, 0
		bra DRoundNearest

DRoundOverflow:
		wth A, 8
		cpy A, 0
		stab (A, 52, 11, +1), 1
		stab (A, 63), f
		wtth (A, B), 4
		wtth (X, Y), 4
		ret

; Either is zero, subnormal, infinite or a NaN.  The significands of subnormals are
; made and a's is written over a for the product to read.
DMultiplySpecial:
		jsr DTwoNaNs
		wtth (A, B), 8
		wtth (X, Y), 8
		cpy A, [5 + S]
		cpy B, [13 + S]
		cpy X, A
		shl X, 1
		cpy Y, B
		shl Y, 1
		cmp X, $FFE0000000000000
		bras z, DMultiplyInfinite
		cmp Y, $FFE0000000000000
		bras z, DMultiplyInfiniteB
		tst X, X
		bras z, DSignedZero
		tst Y, Y
		bras z, DSignedZero
		jsr DUnpack
		cpy [5 + S], A
		wth Y, 4
		cpy Y, X
		cpy A, B
		jsr DUnpack
		cpy B, A
		cpy A, [5 + S]
		add X, Y
		sub X, 1023
		bra DMultiplySignificands

DMultiplyInfinite:
		tst Y, Y
		bras z, DDefaultNaN		; infinity times zero
		bra DSignedInfinity

DMultiplyInfiniteB:
		tst X, X
		bras z, DDefaultNaN
		bra DSignedInfinity


; A double in A (64 bits wide) that is finite and not zero becomes its significand in A
; (the top bit at 52) and its exponent in X (32 bits wide, below 1 for a subnormal).
; Only A and X change.
DUnpack:
		wth X, 4
		cpy X, 0
		scb (A, 52), (X, 0), 11
		shl A, 12
		shr A, 12
		tst X, X
		bras z, DUnpackSubnormal
		stab (A, 52), 1
		ret

DUnpackSubnormal:
		cpy X, 51
		cep c
		fsbi A, X, -1			; X = the top bit
		neg X, X
		add X, 52
		shl A, X
		neg X, X
		add X, 1
		ret


; Called (jsr) from the special cases of a routine with two double arguments.  If either
; is a NaN it returns from the routine with the first NaN, quietened.  Otherwise it
; returns with only X changed (and 64 bits wide).
DTwoNaNs:
		wth X, 8
		cpy X, [9 + S]
		shl X, 1
		cmp X, $FFE0000000000001
		brac c, DTwoNaNsSecond
		wth A, 8
		cpy A, [9 + S]
		bra DTwoNaNsQuiet

DTwoNaNsSecond:
		cpy X, [17 + S]
		shl X, 1
		cmp X, $FFE0000000000001
		brac c, DTwoNaNsNone
		wth A, 8
		cpy A, [17 + S]

DTwoNaNsQuiet:
		stab (A, 51), 1
		add S, 4
		wtth (A, B), 4
		wtth (X, Y), 4

DTwoNaNsNone:
		ret


; Results that are jumped to: they return from the routine.
DDefaultNaN:
		wth A, 8
		cpy A, 0
		stab (A, 51, 13, +1), 1

DWidths:
		wtth (A, B), 4
		wtth (X, Y), 4
		ret

DSignedInfinity:
		wth A, 8
		cpy A, 0
		stab (A, 52, 11, +1), 1
		stab (A, 63), f
		bra DWidths

DSignedZero:
		wth A, 8
		cpy A, 0
		stab (A, 63), f
		bra DWidths


; uint64_t SoftDoubleAdd(uint64_t ulliA, uint64_t ulliB) and SoftDoubleSubtract.  As the
; float add, with the larger's significand shifted up 9 (the same signs) or 10.
_SoftDoubleSubtract:
		wtth (A, B), 8
		wtth (X, Y), 8
		cpy A, [5 + S]
		cpy B, [13 + S]
		xor B, $8000000000000000
		bra DAddSigned

_SoftDoubleAdd:
		wtth (A, B), 8
		wtth (X, Y), 8
		cpy A, [5 + S]
		cpy B, [13 + S]

; The larger magnitude goes in A.
DAddSigned:
		cpy X, A
		shl X, 1
		cpy Y, B
		shl Y, 1
		cmp X, Y
		bras c, DAddOrdered
		xor A, B
		xor B, A
		xor A, B
		xor X, Y
		xor Y, X
		xor X, Y

DAddOrdered:
		bist f, (A, 63)			; the result's sign (unless it cancels)
		shr X, 53				; the exponents
		shr Y, 53
		wtth (X, Y), 4
		cmp X, 2047
		brac z, NotDAddLargerSpecial
		bra DAddLargerSpecial

NotDAddLargerSpecial:
		tst Y, Y
		brac z, NotDAddSmallerSpecial
		bra DAddSmallerSpecial

NotDAddSmallerSpecial:
		sub Y, X
		neg Y, Y				; how far apart they are
		bist c, (B, 63)
		braso (f, c), DAddDifferentRaw

; The same signs: the significands made with the top bit at 61.
		shl A, 12
		shr A, 3
		stab (A, 61), 1
		shl B, 12
		shr B, 3
		stab (B, 61), 1
		bra DAddSameAlign

; A and B are the significands (64 bits wide), X the larger's exponent, Y the
; difference of the exponents, f the larger's sign and g the smaller's.
DAddSignificands:
		stat c, g
		braso (f, c), DAddDifferent

; The same signs: the larger's top bit at 61.  Up to 9 apart nothing is shifted out.
		shl A, 9
		shl B, 9

DAddSameAlign:
		cmp Y, 10
		brac c, DAddSameExact
		cmp Y, 64
		brac c, DAddSameJam
		cpy B, 1
		bra DAddSame

DAddSameJam:
; B >> Y, keeping what is shifted out as its lowest bit: B less (B >> Y) << Y is what is
; lost.  X is the spare register meanwhile, with the exponent in the top half of Y.
		scp (X, 0), (Y, 4), 4
		wth X, 8
		cpy X, B
		shr X, Y
		shl X, Y
		sub B, X
		shr X, Y
		tst B, B
		bras z, DAddSameJamExact
		or X, 1

DAddSameJamExact:
		cpy B, X
		wth X, 4
		scp (Y, 4), (X, 0), 4
		bra DAddSame

DAddSameExact:
		shr B, Y

DAddSame:
		add A, B
		wth Y, 8
		cpy Y, A
		wtth (A, B), 4
		bist c, (Y, 62)
		bras c, DAddSameTop
		shl Y, 1
		sub X, 1

DAddSameTop:
		cmp X, 2046
		brac c, DAddSameNearest
		bra DRoundOutside

DAddSameNearest:
		add Y, 512
		tst Y, $3FF
		brac z, DAddSameShift
		stab (Y, 10), 0

DAddSameShift:
		shr Y, 10
		wth A, 8
		cpy A, X
		shl A, 52
		add A, Y
		stab (A, 63), f
		wtth (A, B), 4
		wtth (X, Y), 4
		ret

; The signs are different: the significands made with the top bit at 62.
DAddDifferentRaw:
		shl A, 12
		shr A, 2
		stab (A, 62), 1
		shl B, 12
		shr B, 2
		stab (B, 62), 1
		bra DAddDifferentAlign

; The larger's top bit at 62.  Two or more apart, the difference only loses its top bit
; if anything.
DAddDifferent:
		shl A, 10
		shl B, 10

DAddDifferentAlign:
		cmp Y, 2
		brac c, DAddClose
		cmp Y, 11
		brac c, DAddDifferentExact
		cmp Y, 64
		brac c, DAddDifferentJam
		cpy B, 1
		bra DAddDifferentSubtract

DAddDifferentJam:
; B >> Y, keeping what is shifted out as its lowest bit: B less (B >> Y) << Y is what is
; lost.  X is the spare register meanwhile, with the exponent in the top half of Y.
		scp (X, 0), (Y, 4), 4
		wth X, 8
		cpy X, B
		shr X, Y
		shl X, Y
		sub B, X
		shr X, Y
		tst B, B
		bras z, DAddDifferentJamExact
		or X, 1

DAddDifferentJamExact:
		cpy B, X
		wth X, 4
		scp (Y, 4), (X, 0), 4
		bra DAddDifferentSubtract

DAddDifferentExact:
		shr B, Y

DAddDifferentSubtract:
		sub A, B
		wth Y, 8
		cpy Y, A
		wtth (A, B), 4
		sub X, 1
		bist c, (Y, 62)
		bras c, DAddDifferentTop
		shl Y, 1
		sub X, 1

DAddDifferentTop:
		cmp X, 2046
		brac c, DAddDifferentNearest
		bra DRoundOutside

DAddDifferentNearest:
		add Y, 512
		tst Y, $3FF
		brac z, DAddDifferentShift
		stab (Y, 10), 0

DAddDifferentShift:
		shr Y, 10
		wth A, 8
		cpy A, X
		shl A, 52
		add A, Y
		stab (A, 63), f
		wtth (A, B), 4
		wtth (X, Y), 4
		ret

; Within one of each other the difference is exact but may lose any number of bits (or
; all of them): its top bit is found and moved to 62.
DAddClose:
		shr B, Y
		sub A, B
		bras z, DAddCancelled
		wth Y, 8
		cpy Y, A
		wtth (A, B), 4
		cpy B, 62
		cep c
		fsbi Y, B, -1
		neg B, B
		add B, 62
		shl Y, B
		sub X, B
		sub X, 1
		bra DRoundTop

DAddCancelled:
		cpy A, 0
		bra DAddReturn

; The smaller is zero or subnormal (X is the larger's exponent).
DAddSmallerSpecial:
		bist g, (B, 63)
		wth Y, 8
		cpy Y, B
		shl Y, 1
		wth Y, 4
		bras z, DAddSmallerZero
		tst X, X
		bras z, DAddSubnormals
		cpy Y, X
		sub Y, 1
		shl A, 12
		shr A, 12
		stab (A, 52), 1
		shl B, 12
		shr B, 12
		bra DAddSignificands

; Adding zero gives the other, and zeros add to -0 only when both are -0.
DAddSmallerZero:
		wth X, 8
		cpy X, A
		shl X, 1
		brac z, DAddReturn
		cpy A, 0
		stat c, g
		brasa (f, c), DAddNegativeZero
		bra DAddReturn

DAddNegativeZero:
		stab (A, 63), 1

DAddReturn:
		wtth (A, B), 4
		wtth (X, Y), 4
		ret

; Two subnormals add exactly, carrying into the exponent if they must.
DAddSubnormals:
		shl A, 1
		shr A, 1
		shl B, 1
		shr B, 1
		stat c, g
		braso (f, c), DAddSubnormalsDifferent
		add A, B
		stab (A, 63), f
		bra DAddReturn

DAddSubnormalsDifferent:
		sub A, B
		bras z, DAddReturn
		stab (A, 63), f
		bra DAddReturn

; The larger is infinite or a NaN.
DAddLargerSpecial:
		bist g, (B, 63)
		jsr DTwoNaNs
		cmp Y, 2047
		brac z, DAddReturn
		stat c, f
		braso (g, c), DDefaultNaN
		bra DAddReturn

; uint64_t SoftDoubleDivide(uint64_t ulliA, uint64_t ulliB)
_SoftDoubleDivide:
		wtth (A, B), 8
		wtth (X, Y), 8
		cpy A, [5 + S]
		cpy B, [13 + S]
		cpy Y, A
		xor Y, B
		bist f, (Y, 63)
		cpy X, 0
		scb (A, 52), (X, 0), 11
		cpy Y, 0
		scb (B, 52), (Y, 0), 11
		wtth (X, Y), 4
		sub X, 1
		cmp X, 2046
		brac c, NotDDivideSpecial1
		bra DDivideSpecial

NotDDivideSpecial1:
		sub Y, 1
		cmp Y, 2046
		brac c, NotDDivideSpecial2
		bra DDivideSpecial

NotDDivideSpecial2:
		sub X, Y
		wth Y, 8
		cpy Y, A
		shl Y, 12
		shr Y, 12
		stab (Y, 52), 1			; a's significand
		cpy A, B
		shl A, 12
		shr A, 12
		stab (A, 52), 1			; b's significand

; Y is a's significand and A b's (64 bits wide), X the difference of the exponents.
; Each div takes the remainder shifted up 11 and gives 11 more bits of the quotient
; (12 the first time): five give floor(a 2^55 / b), from 2^54 to 2^56, put together in X
; shifted up 7 for DRoundPack, and the last remainder is the sticky bit.  W and the
; exponent wait on the stack.
DDivideSignificands:
		psh (W, X)
		wth X, 8
		cpy X, 0
		shl Y, 11
		cpy B, 0
		cpy W, 0
		div
		scb (Y, 0), (X, 51), 12
		cpy Y, B
		shl Y, 11
		cpy B, 0
		cpy W, 0
		div
		scb (Y, 0), (X, 40), 11
		cpy Y, B
		shl Y, 11
		cpy B, 0
		cpy W, 0
		div
		scb (Y, 0), (X, 29), 11
		cpy Y, B
		shl Y, 11
		cpy B, 0
		cpy W, 0
		div
		scb (Y, 0), (X, 18), 11
		cpy Y, B
		shl Y, 11
		cpy B, 0
		cpy W, 0
		div
		scb (Y, 0), (X, 7), 11
		tst B, B
		bras z, DDivideExact
		or X, 1

DDivideExact:
		cpy Y, X
		wth X, 4
		pul (W, X)
		add X, 1022
		wtth (A, B), 4
		bist c, (Y, 62)
		bras c, DDivideTop
		shl Y, 1
		sub X, 1

DDivideTop:
		cmp X, 2046
		brac c, DDivideNearest
		bra DRoundOutside

DDivideNearest:
		add Y, 512
		tst Y, $3FF
		brac z, DDivideShift
		stab (Y, 10), 0

DDivideShift:
		shr Y, 10
		wth A, 8
		cpy A, X
		shl A, 52
		add A, Y
		stab (A, 63), f
		wtth (A, B), 4
		wtth (X, Y), 4
		ret

; Either is zero, subnormal, infinite or a NaN.
DDivideSpecial:
		jsr DTwoNaNs
		wtth (A, B), 8
		wtth (X, Y), 8
		cpy A, [5 + S]
		cpy B, [13 + S]
		cpy X, A
		shl X, 1
		cpy Y, B
		shl Y, 1
		cmp X, $FFE0000000000000
		bras z, DDivideInfinite
		cmp Y, $FFE0000000000000
		bras z, DSignedZero		; over infinity
		tst Y, Y
		bras z, DDivideByZero
		tst X, X
		bras z, DSignedZero
		jsr DUnpack
		cpy [5 + S], A
		wth Y, 4
		cpy Y, X
		cpy A, B
		jsr DUnpack				; A = b's significand
		sub Y, X
		cpy X, Y
		wth Y, 8
		cpy Y, [5 + S]
		bra DDivideSignificands

DDivideInfinite:
		cmp Y, $FFE0000000000000
		bras z, DDefaultNaN		; infinity over infinity
		bra DSignedInfinity

DDivideByZero:
		tst X, X
		bras z, DDefaultNaN		; zero over zero
		bra DSignedInfinity


; uint64_t SoftDoubleFromInt(int32_t i), SoftDoubleFromUnsigned(uint32_t ui) (exact),
; SoftDoubleFromLongLong(int64_t lli) and SoftDoubleFromUnsignedLongLong(uint64_t ulli).
_SoftDoubleFromInt:
		cpy A, [5 + S]
		bras z, DFromZero
		abs A, A
		stat f, n
		bra DFromMagnitude32

_SoftDoubleFromUnsigned:
		cpy A, [5 + S]
		bras z, DFromZero
		cep f

DFromMagnitude32:
		cpy B, 31
		cep c
		fsbi A, B, -1			; B = the top bit
		wthx A, 8

; A (64 bits wide) is the magnitude with its top bit at B (52 or below), f the sign.
DFromExact:
		cpy X, B
		add X, 1022
		neg B, B
		add B, 52
		shl A, B
		wthx X, 8
		shl X, 52
		add A, X
		stab (A, 63), f
		wtth (A, X), 4
		ret

DFromZeroWide:
		wth Y, 4

DFromZero:
		wth A, 8
		cpy A, 0
		wth A, 4
		ret

_SoftDoubleFromLongLong:
		wth Y, 8
		cpy Y, [5 + S]
		bras z, DFromZeroWide
		abs Y, Y
		stat f, n
		bra DFromMagnitude

_SoftDoubleFromUnsignedLongLong:
		wth Y, 8
		cpy Y, [5 + S]
		bras z, DFromZeroWide
		cep f

; Y (64 bits wide) is the magnitude.  Up to 53 bits it is exact; longer, its top bit is
; moved to 62 (or 63 shifted down one with a sticky bit) and rounded.
DFromMagnitude:
		cpy B, 63
		cep c
		fsbi Y, B, -1
		cmp B, 53
		brac c, DFromSmall
		cpy X, B
		add X, 1022
		cmp B, 63
		brac c, DFromLeft
		shr Y, 1
		brac c, DFromRound
		or Y, 1
		bra DFromRound

DFromLeft:
		neg B, B
		add B, 62
		shl Y, B

DFromRound:
		bra DRoundTop

DFromSmall:
		wth A, 8
		cpy A, Y
		wth Y, 4
		bra DFromExact


; uint64_t SoftDoubleSqrt(uint64_t ulli), correctly rounded.  The significand shifted
; up 56 or 57 (so the power of two left is even) is M, from 2^108 to 2^110; its top 64
; bits MH (M's low 46 bits are zero) have an integer root r1 of 32 bits: the float
; sqrt's seed table and three Newton steps (a div each), corrected down with one mul.
; With D = MH - r1^2, one more Newton step from r1 2^23 gives x = r1 2^23 + d where d,
; p = divmod(D 2^22, r1): x is the root of M or one more, and M - x^2 = p 2^24 - d^2
; says which, and whether it is exact.  MH, then p 2^24, is kept in X, and W and the
; exponent on the stack.
_SoftDoubleSqrt:
		wtth (A, B), 8
		wtth (X, Y), 8
		cpy A, [5 + S]
		cpy X, A
		sub X, $10000000000000
		cmp X, $7FE0000000000000
		brac c, NotDSqrtSpecial
		bra DSqrtSpecial		; not a positive normal

NotDSqrtSpecial:
		wth X, 4
		cpy X, 0
		scb (A, 52), (X, 0), 11
		shl A, 12
		shr A, 12
		stab (A, 52), 1

; A (64 bits wide) is the significand (the top bit at 52) and X the exponent.
DSqrtSignificand:
		cep f
		cpy Y, A
		bist c, (X, 0)
		brac c, DSqrtEven
		shl Y, 10
		add X, 1021				; (exponent - 56 + 1077) / 2 is DRoundPack's
		bra DSqrtRoot

DSqrtEven:
		shl Y, 11
		add X, 1020

DSqrtRoot:
		shr X, 1
		psh (W, X)				; W and the exponent wait on the stack
		wth X, 8
		cpy X, Y				; MH
		cpy B, Y
		shr B, 55
		and B, -4				; 4 (MH >> 57)
		wth A, 4
		cpy A, [SqrtTable - 128 + B]
		wthx A, 8
		shl A, 7
		cpy B, 0
		cpy W, 0
		div
		add Y, A
		shr Y, 1
		cpy A, Y
		cpy Y, X
		cpy B, 0
		cpy W, 0
		div
		add Y, A
		shr Y, 1
		cpy A, Y
		cpy Y, X
		cpy B, 0
		cpy W, 0
		div
		add Y, A
		shr Y, 1
		cpy A, Y				; r1 or r1 + 1, at most 2^32
		cpy B, A
		shr B, 32
		bras z, DSqrtFits
		cpy A, $FFFFFFFF

DSqrtFits:
		wtth (A, B), 4
		cpy B, A
		cpy W, 0
		mul
		wth B, 8
		cpy B, X
		cmp B, Y
		bras c, DSqrtRemainder
		sub Y, A				; (r - 1)^2 = r^2 - 2r + 1
		sub Y, A
		add Y, 1
		sub A, 1

DSqrtRemainder:
		sub B, Y				; D
		cpy Y, B
		shl Y, 22
		cpy B, 0
		cpy W, 0
		div						; Y = d, B = p
		shl B, 24
		cpy X, B				; p 2^24
		psh A					; r1
		cpy A, Y
		wth B, 4
		cpy B, Y
		cpy W, 0
		mul						; Y = d^2, B = d
		pul A
		wthx A, 8
		shl A, 23
		add A, B				; x
		cmp X, Y
		bras c, DSqrtNotLess
		sub A, 1				; x was one too large
		bra DSqrtInexact

DSqrtNotLess:
		bras z, DSqrtExact

DSqrtInexact:
		cpy Y, A
		shl Y, 8
		or Y, 1
		bra DSqrtDone

DSqrtExact:
		cpy Y, A
		shl Y, 8				; the top bit at 62

DSqrtDone:
		wth X, 4
		pul (W, X)
		wtth (A, B), 4
		bra DRoundTop

DSqrtSpecial:
		cpy X, A
		shl X, 1
		bras z, DSqrtReturn		; -0 and +0
		cmp X, $FFE0000000000001
		bras c, DSqrtNaN
		tst A, A
		bras n, DDefaultNaN
		cmp X, $FFE0000000000000
		bras z, DSqrtReturn		; +infinity
		jsr DUnpack
		bra DSqrtSignificand

DSqrtNaN:
		stab (A, 51), 1

DSqrtReturn:
		wtth (A, B), 4
		wtth (X, Y), 4
		ret


; int SoftDoubleCompare(uint64_t ulliA, uint64_t ulliB), as SoftFloatCompare.
		.section text._SoftDoubleCompare
		.global _SoftDoubleCompare
_SoftDoubleCompare:
		wtth (A, B), 8
		wtth (X, Y), 8
		cpy A, [5 + S]
		cpy B, [13 + S]
		cpy X, A
		shl X, 1
		cpy Y, B
		shl Y, 1
		cmp X, Y
		brac c, DCompareSmaller
		brac z, NotDCompareSame
		bra DCompareSame

NotDCompareSame:
		cmp X, $FFE0000000000001
		brac c, NotDCompareNaN
		bra DCompareNaN

NotDCompareNaN:
		bist c, (A, 63)			; |a| > |b|: 1 if a is positive
		cpy A, 1
		brac c, DCompareDone
		cpy A, -1

DCompareDone:
		wtth (A, B), 4
		wtth (X, Y), 4
		ret

DCompareSmaller:
		cmp Y, $FFE0000000000001
		bras c, DCompareNaN
		bist c, (B, 63)			; |a| < |b|: -1 if b is positive
		cpy A, -1
		brac c, DCompareDone
		cpy A, 1
		bra DCompareDone

DCompareSame:
		cmp X, $FFE0000000000001
		bras c, DCompareNaN
		tst X, X
		bras z, DCompareEqual	; -0 and +0
		xor B, A
		brac n, DCompareEqual	; the same sign
		bist c, (A, 63)
		cpy A, 1
		brac c, DCompareDone
		cpy A, -1
		bra DCompareDone

DCompareEqual:
		cpy A, 0
		bra DCompareDone

DCompareNaN:
		cpy A, 2
		bra DCompareDone


; int32_t SoftDoubleToInt(uint64_t ulli), uint32_t SoftDoubleToUnsigned(uint64_t ulli),
; int64_t SoftDoubleToLongLong(uint64_t ulli) and uint64_t SoftDoubleToUnsignedLongLong(uint64_t ulli),
; towards zero.  The significand at the top of A is shifted down to the units.
		.section text._SoftDoubleToInt
		.global _SoftDoubleToInt, _SoftDoubleToUnsigned, _SoftDoubleToLongLong, _SoftDoubleToUnsignedLongLong
_SoftDoubleToInt:
		wth A, 8
		cpy A, [5 + S]
		cpy X, 0
		scb (A, 52), (X, 0), 11
		sub X, 1023				; the power of two of the top bit
		cmp X, 31
		brac c, NotDToIntOutside
		bra DToIntOutside

NotDToIntOutside:
		bist f, (A, 63)
		shl A, 11
		stab (A, 63), 1
		neg X, X
		add X, 63
		shr A, X
		brac f, DToIntDone
		neg A, A

DToIntDone:
		wth A, 4
		ret

; Below 1 (X is negative), or too large, infinite or a NaN.  -2^31 is the indefinite
; integer too.
DToIntOutside:
		tst X, X
		bras n, DToZero

DToIndefinite:
		cpy A, $80000000
		wth A, 4
		ret

DToZero:
		cpy A, 0
		wth A, 4
		ret

_SoftDoubleToUnsigned:
		wth A, 8
		cpy A, [5 + S]
		cpy X, 0
		scb (A, 52), (X, 0), 11
		sub X, 1023
		bras n, DToZero			; below 1, -1 to 0 included
		tst A, A
		bras n, DToIndefinite
		cmp X, 32
		bras c, DToIndefinite
		shl A, 11
		stab (A, 63), 1
		neg X, X
		add X, 63
		shr A, X
		wth A, 4
		ret

_SoftDoubleToLongLong:
		wth A, 8
		cpy A, [5 + S]
		cpy X, 0
		scb (A, 52), (X, 0), 11
		sub X, 1023
		cmp X, 63
		brac c, NotDToLongOutside
		bra DToLongOutside

NotDToLongOutside:
		bist f, (A, 63)
		shl A, 11
		stab (A, 63), 1
		neg X, X
		add X, 63
		shr A, X
		brac f, DToLongDone
		neg A, A

DToLongDone:
		wth A, 4
		ret

DToLongOutside:
		tst X, X
		bras n, DToZero

DToLongIndefinite:
		cpy A, 0
		stab (A, 63), 1
		wth A, 4
		ret

_SoftDoubleToUnsignedLongLong:
		wth A, 8
		cpy A, [5 + S]
		cpy X, 0
		scb (A, 52), (X, 0), 11
		sub X, 1023
		bras n, DToZero
		tst A, A
		bras n, DToLongIndefinite
		cmp X, 64
		bras c, DToLongIndefinite
		shl A, 11
		stab (A, 63), 1
		neg X, X
		add X, 63
		shr A, X
		wth A, 4
		ret


; ceil(sqrt((i + 1) * 2^43)) for i from 32 to 127: at least the root of an M whose top
; bits are i.
		.section rodata.SoftFloatSqrtTable
		.align 4
SqrtTable:
		.long $103F820, $107E0F7, $10BBB31, $10F876D, $1134640, $116F834
		.long $11A9DC9, $11E377A, $121C5B8, $12548EC, $128C17C, $12C2FC6
		.long $12F9423, $132EEE8, $1364064, $13988E2, $13CC8AA, $1400000
		.long $1432F25, $1465656, $14975CE, $14C8DC3, $14F9E6C, $152A7FB
		.long $155AAA1, $158A68B, $15B9BE6, $15E8ADE, $1617399, $1645641
		.long $16732F9, $16A09E7, $16CDB2C, $16FA6EB, $1726D42, $1752E51
		.long $177EA36, $17AA10E, $17D52F3, $1800000, $182A851, $1854BFC
		.long $187EB1A, $18A85C3, $18D1C0C, $18FAE0D, $1923BD8, $194C584
		.long $1974B24, $199CCCA, $19C4A8A, $19EC475, $1A13A9D, $1A3AD13
		.long $1A61BE7, $1A8872A, $1AAEEEA, $1AD5337, $1AFB420, $1B211B2
		.long $1B46BFD, $1B6C30C, $1B916EE, $1BB67AF, $1BDB55C, $1C00000
		.long $1C247A9, $1C48C61, $1C6CE33, $1C90D2A, $1CB4952, $1CD82B5
		.long $1CFB95C, $1D1ED53, $1D41EA1, $1D64D52, $1D8796F, $1DAA2FF
		.long $1DCCA0E, $1DEEEA2, $1E110C4, $1E3307D, $1E54DD5, $1E768D4
		.long $1E98181, $1EB97E5, $1EDAC06, $1EFBDEC, $1F1CD9D, $1F3DB22
		.long $1F5E680, $1F7EFBF, $1F9F6E5, $1FBFBF8, $1FDFEFF, $2000000

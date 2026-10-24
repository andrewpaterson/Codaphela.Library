//IEEE 754 single and double precision for the 65C64, in C++ for the 65C64 generator, which calls these for
//float and double arithmetic, comparisons and conversions.  Values are passed as their bits.  Rounding is to
//nearest, ties to even; subnormals, infinities and NaNs are handled.  NaNs behave as SSE's do: an invalid
//operation gives the default NaN (sign set, quiet), a NaN operand is returned quietened (the first if both
//are NaNs), and a conversion to an integer that is out of range or a NaN gives the most negative integer.
//The code only uses integers so it also compiles for a host, to be checked against its hardware.
//On the 65C64 every function here is softfloat.asm's, which gives the same results much faster, so this file
//compiles to nothing there; it is kept as softfloat.asm's reference (TestLibc65 compiles it natively).
#include <stdint.h>
#ifndef __65C64__


#define SOFT_ZERO		0
#define SOFT_FINITE		1
#define SOFT_INFINITY	2
#define SOFT_NAN		3


//A format: the bits of the fraction, the exponent bias and the largest (all ones) exponent.
#define FLOAT_FRACTION		23
#define FLOAT_BIAS			127
#define FLOAT_MAX_EXPONENT	255
#define DOUBLE_FRACTION		52
#define DOUBLE_BIAS			1023
#define DOUBLE_MAX_EXPONENT	2047


static uint64_t SignBit(int iFraction)
{
	return (uint64_t)1 << (iFraction == FLOAT_FRACTION ? 31 : 63);
}


static uint64_t FractionMask(int iFraction)
{
	return ((uint64_t)1 << iFraction) - 1;
}


static int MaxExponent(int iFraction)
{
	return (iFraction == FLOAT_FRACTION) ? FLOAT_MAX_EXPONENT : DOUBLE_MAX_EXPONENT;
}


static int Bias(int iFraction)
{
	return (iFraction == FLOAT_FRACTION) ? FLOAT_BIAS : DOUBLE_BIAS;
}


static uint64_t Infinity(int iFraction, bool bNegative)
{
	return (bNegative ? SignBit(iFraction) : 0) | ((uint64_t)MaxExponent(iFraction) << iFraction);
}


static uint64_t DefaultNaN(int iFraction)
{
	return SignBit(iFraction) | ((uint64_t)MaxExponent(iFraction) << iFraction) | ((uint64_t)1 << (iFraction - 1));
}


static uint64_t Quiet(uint64_t ulliBits, int iFraction)
{
	return ulliBits | ((uint64_t)1 << (iFraction - 1));
}


//Splits a value into its sign and, for a finite value, a significand and the power of two of its lowest bit.
static int Unpack(uint64_t ulliBits, int iFraction, bool* pbNegative, int* piExponent, uint64_t* pulliSignificand)
{
	int			iBiased;
	uint64_t	ulliFraction;

	*pbNegative = (ulliBits & SignBit(iFraction)) != 0;
	iBiased = (int)((ulliBits >> iFraction) & (uint64_t)MaxExponent(iFraction));
	ulliFraction = ulliBits & FractionMask(iFraction);
	if (iBiased == MaxExponent(iFraction))
	{
		return (ulliFraction == 0) ? SOFT_INFINITY : SOFT_NAN;
	}
	if (iBiased == 0)
	{
		if (ulliFraction == 0)
		{
			return SOFT_ZERO;
		}
		*piExponent = 1 - Bias(iFraction) - iFraction;
		*pulliSignificand = ulliFraction;
		return SOFT_FINITE;
	}
	*piExponent = iBiased - Bias(iFraction) - iFraction;
	*pulliSignificand = ulliFraction | ((uint64_t)1 << iFraction);
	return SOFT_FINITE;
}


static int HighestBit(uint64_t ulli)
{
	return 63 - __builtin_clzll(ulli);
}


//Rounds ulliSignificand * 2^iExponent to the format: to nearest, ties to even.  Too large is an infinity and
//too small a subnormal or zero.
static uint64_t RoundPack(bool bNegative, int iExponent, uint64_t ulliSignificand, int iFraction)
{
	int			iTop;
	int			iBiased;
	int			iShift;
	uint64_t	ulliKept;
	uint64_t	ulliRest;
	uint64_t	ulliHalf;
	uint64_t	ulliBits;
	uint64_t	ulliSign;

	ulliSign = bNegative ? SignBit(iFraction) : 0;
	if (ulliSignificand == 0)
	{
		return ulliSign;
	}

	//The biased exponent of the top bit, and the bits below the format's precision.
	iTop = HighestBit(ulliSignificand);
	iBiased = iExponent + iTop + Bias(iFraction);
	if (iBiased >= MaxExponent(iFraction))
	{
		return Infinity(iFraction, bNegative);
	}
	iShift = iTop - iFraction;
	if (iBiased < 1)
	{
		iShift += 1 - iBiased;
		iBiased = 1;
	}

	if (iShift <= 0)
	{
		ulliKept = ulliSignificand << -iShift;
	}
	else if (iShift >= 64)
	{
		//Only rounds up to the smallest subnormal when more than half of it.
		ulliKept = ((iShift == 64) && (ulliSignificand > ((uint64_t)1 << 63))) ? 1 : 0;
	}
	else
	{
		ulliKept = ulliSignificand >> iShift;
		ulliRest = ulliSignificand & (((uint64_t)1 << iShift) - 1);
		ulliHalf = (uint64_t)1 << (iShift - 1);
		if ((ulliRest > ulliHalf) || ((ulliRest == ulliHalf) && ((ulliKept & 1) != 0)))
		{
			ulliKept++;
		}
	}

	//The significand's top bit adds to the exponent, so carries out of rounding and subnormals that round up
	//to the smallest normal come out right.
	ulliBits = ((uint64_t)(iBiased - 1) << iFraction) + ulliKept;
	if (ulliBits >= ((uint64_t)MaxExponent(iFraction) << iFraction))
	{
		return Infinity(iFraction, bNegative);
	}
	return ulliSign | ulliBits;
}


//Shifts right, keeping any bits shifted out as the lowest bit (sticky).
static uint64_t ShiftRightJam(uint64_t ulli, int iShift)
{
	if (iShift <= 0)
	{
		return ulli;
	}
	if (iShift >= 63)
	{
		return (ulli != 0) ? 1 : 0;
	}
	return (ulli >> iShift) | (((ulli & (((uint64_t)1 << iShift) - 1)) != 0) ? 1 : 0);
}


//The NaN result of an operation with a NaN operand.
static uint64_t PropagateNaN(uint64_t ulliA, int iClassA, uint64_t ulliB, int iFraction)
{
	return Quiet((iClassA == SOFT_NAN) ? ulliA : ulliB, iFraction);
}


static uint64_t Add(uint64_t ulliA, uint64_t ulliB, bool bSubtract, int iFraction)
{
	bool		bNegativeA;
	bool		bNegativeB;
	int			iExponentA;
	int			iExponentB;
	uint64_t	ulliA2;
	uint64_t	ulliB2;
	int			iClassA;
	int			iClassB;
	int			iShift;
	uint64_t	ulliSwap;
	int			iSwap;
	bool		bSwap;

	iExponentA = 0;
	iExponentB = 0;
	ulliA2 = 0;
	ulliB2 = 0;
	iClassA = Unpack(ulliA, iFraction, &bNegativeA, &iExponentA, &ulliA2);
	iClassB = Unpack(ulliB, iFraction, &bNegativeB, &iExponentB, &ulliB2);
	if ((iClassA == SOFT_NAN) || (iClassB == SOFT_NAN))
	{
		return PropagateNaN(ulliA, iClassA, ulliB, iFraction);
	}
	if (bSubtract)
	{
		bNegativeB = !bNegativeB;
		ulliB ^= SignBit(iFraction);
	}
	if (iClassA == SOFT_INFINITY)
	{
		if ((iClassB == SOFT_INFINITY) && (bNegativeA != bNegativeB))
		{
			return DefaultNaN(iFraction);
		}
		return ulliA;
	}
	if (iClassB == SOFT_INFINITY)
	{
		return ulliB;
	}
	if (iClassA == SOFT_ZERO)
	{
		if (iClassB == SOFT_ZERO)
		{
			//-0 + -0 is -0, any other sum of zeros is +0.
			return (bNegativeA && bNegativeB) ? SignBit(iFraction) : 0;
		}
		return ulliB;
	}
	if (iClassB == SOFT_ZERO)
	{
		return ulliA;
	}

	//The top bits at 61, leaving room for a carry, and the smaller lined up with the larger, sticky.
	iShift = 61 - iFraction;
	ulliA2 <<= iShift;
	ulliB2 <<= iShift;
	iExponentA -= iShift;
	iExponentB -= iShift;
	if ((iExponentA < iExponentB) || ((iExponentA == iExponentB) && (ulliA2 < ulliB2)))
	{
		ulliSwap = ulliA2;
		ulliA2 = ulliB2;
		ulliB2 = ulliSwap;
		iSwap = iExponentA;
		iExponentA = iExponentB;
		iExponentB = iSwap;
		bSwap = bNegativeA;
		bNegativeA = bNegativeB;
		bNegativeB = bSwap;
	}
	ulliB2 = ShiftRightJam(ulliB2, iExponentA - iExponentB);

	if (bNegativeA == bNegativeB)
	{
		return RoundPack(bNegativeA, iExponentA, ulliA2 + ulliB2, iFraction);
	}
	if (ulliA2 == ulliB2)
	{
		return 0;
	}
	if (ulliA2 < ulliB2)
	{
		return RoundPack(bNegativeB, iExponentA, ulliB2 - ulliA2, iFraction);
	}
	return RoundPack(bNegativeA, iExponentA, ulliA2 - ulliB2, iFraction);
}


static uint64_t Multiply(uint64_t ulliA, uint64_t ulliB, int iFraction)
{
	bool		bNegativeA;
	bool		bNegativeB;
	bool		bNegative;
	int			iExponentA;
	int			iExponentB;
	uint64_t	ulliA2;
	uint64_t	ulliB2;
	int			iClassA;
	int			iClassB;
	uint64_t	ulliLowA;
	uint64_t	ulliHighA;
	uint64_t	ulliLowB;
	uint64_t	ulliHighB;
	uint64_t	ulliLow;
	uint64_t	ulliMiddle1;
	uint64_t	ulliMiddle2;
	uint64_t	ulliHigh;
	uint64_t	ulliCarry;
	int			iBits;

	iExponentA = 0;
	iExponentB = 0;
	ulliA2 = 0;
	ulliB2 = 0;
	iClassA = Unpack(ulliA, iFraction, &bNegativeA, &iExponentA, &ulliA2);
	iClassB = Unpack(ulliB, iFraction, &bNegativeB, &iExponentB, &ulliB2);
	if ((iClassA == SOFT_NAN) || (iClassB == SOFT_NAN))
	{
		return PropagateNaN(ulliA, iClassA, ulliB, iFraction);
	}
	bNegative = bNegativeA != bNegativeB;
	if ((iClassA == SOFT_INFINITY) || (iClassB == SOFT_INFINITY))
	{
		if ((iClassA == SOFT_ZERO) || (iClassB == SOFT_ZERO))
		{
			return DefaultNaN(iFraction);
		}
		return Infinity(iFraction, bNegative);
	}
	if ((iClassA == SOFT_ZERO) || (iClassB == SOFT_ZERO))
	{
		return bNegative ? SignBit(iFraction) : 0;
	}

	//The 128 bit product from 32 bit halves.
	ulliLowA = ulliA2 & 0xFFFFFFFF;
	ulliHighA = ulliA2 >> 32;
	ulliLowB = ulliB2 & 0xFFFFFFFF;
	ulliHighB = ulliB2 >> 32;
	ulliLow = ulliLowA * ulliLowB;
	ulliMiddle1 = ulliLowA * ulliHighB;
	ulliMiddle2 = ulliHighA * ulliLowB;
	ulliHigh = ulliHighA * ulliHighB;
	ulliCarry = (ulliLow >> 32) + (ulliMiddle1 & 0xFFFFFFFF) + (ulliMiddle2 & 0xFFFFFFFF);
	ulliLow = (ulliLow & 0xFFFFFFFF) | (ulliCarry << 32);
	ulliHigh += (ulliMiddle1 >> 32) + (ulliMiddle2 >> 32) + (ulliCarry >> 32);

	//The top 64 bits, the rest sticky in the lowest.
	if (ulliHigh == 0)
	{
		return RoundPack(bNegative, iExponentA + iExponentB, ulliLow, iFraction);
	}
	iBits = HighestBit(ulliHigh) + 1;
	if (iBits == 64)
	{
		return RoundPack(bNegative, iExponentA + iExponentB + 64, ulliHigh | ((ulliLow != 0) ? 1 : 0), iFraction);
	}
	ulliHigh = (ulliHigh << (64 - iBits)) | (ulliLow >> iBits);
	if ((ulliLow << (64 - iBits)) != 0)
	{
		ulliHigh |= 1;
	}
	return RoundPack(bNegative, iExponentA + iExponentB + iBits, ulliHigh, iFraction);
}


static uint64_t Divide(uint64_t ulliA, uint64_t ulliB, int iFraction)
{
	bool		bNegativeA;
	bool		bNegativeB;
	bool		bNegative;
	int			iExponentA;
	int			iExponentB;
	uint64_t	ulliA2;
	uint64_t	ulliB2;
	int			iClassA;
	int			iClassB;
	uint64_t	ulliQuotient;
	int			iShift;
	int			i;
	int			iBits;

	iExponentA = 0;
	iExponentB = 0;
	ulliA2 = 0;
	ulliB2 = 0;
	iClassA = Unpack(ulliA, iFraction, &bNegativeA, &iExponentA, &ulliA2);
	iClassB = Unpack(ulliB, iFraction, &bNegativeB, &iExponentB, &ulliB2);
	if ((iClassA == SOFT_NAN) || (iClassB == SOFT_NAN))
	{
		return PropagateNaN(ulliA, iClassA, ulliB, iFraction);
	}
	bNegative = bNegativeA != bNegativeB;
	if (iClassA == SOFT_INFINITY)
	{
		return (iClassB == SOFT_INFINITY) ? DefaultNaN(iFraction) : Infinity(iFraction, bNegative);
	}
	if (iClassB == SOFT_INFINITY)
	{
		return bNegative ? SignBit(iFraction) : 0;
	}
	if (iClassB == SOFT_ZERO)
	{
		return (iClassA == SOFT_ZERO) ? DefaultNaN(iFraction) : Infinity(iFraction, bNegative);
	}
	if (iClassA == SOFT_ZERO)
	{
		return bNegative ? SignBit(iFraction) : 0;
	}

	//Both with their top bit at 62, so A / B is from 1/2 to 2.  Long division, a bit at a time.
	iShift = 62 - HighestBit(ulliA2);
	ulliA2 <<= iShift;
	iExponentA -= iShift;
	iShift = 62 - HighestBit(ulliB2);
	ulliB2 <<= iShift;
	iExponentB -= iShift;

	iBits = iFraction + 3;
	ulliQuotient = 0;
	for (i = 0; i < iBits; i++)
	{
		ulliQuotient <<= 1;
		if (ulliA2 >= ulliB2)
		{
			ulliA2 -= ulliB2;
			ulliQuotient |= 1;
		}
		ulliA2 <<= 1;
	}
	ulliQuotient = (ulliQuotient << 1) | ((ulliA2 != 0) ? 1 : 0);
	return RoundPack(bNegative, iExponentA - iExponentB - iBits, ulliQuotient, iFraction);
}


//-1, 0 or 1 as A is less than, equal to or greater than B, and 2 if either is a NaN.  The zeros are equal.
static int Compare(uint64_t ulliA, uint64_t ulliB, int iFraction)
{
	bool		bNegativeA;
	bool		bNegativeB;
	int			iExponent;
	uint64_t	ulliSignificand;
	int			iClassA;
	int			iClassB;
	uint64_t	ulliMagnitudeA;
	uint64_t	ulliMagnitudeB;

	iClassA = Unpack(ulliA, iFraction, &bNegativeA, &iExponent, &ulliSignificand);
	iClassB = Unpack(ulliB, iFraction, &bNegativeB, &iExponent, &ulliSignificand);
	if ((iClassA == SOFT_NAN) || (iClassB == SOFT_NAN))
	{
		return 2;
	}
	if ((iClassA == SOFT_ZERO) && (iClassB == SOFT_ZERO))
	{
		return 0;
	}

	//The bits without the sign order the magnitudes.
	ulliMagnitudeA = ulliA & ~SignBit(iFraction);
	ulliMagnitudeB = ulliB & ~SignBit(iFraction);
	if (bNegativeA != bNegativeB)
	{
		return bNegativeA ? -1 : 1;
	}
	if (ulliMagnitudeA == ulliMagnitudeB)
	{
		return 0;
	}
	if ((ulliMagnitudeA < ulliMagnitudeB) != bNegativeA)
	{
		return -1;
	}
	return 1;
}


static uint64_t FromInteger(bool bNegative, uint64_t ulliMagnitude, int iFraction)
{
	return RoundPack(bNegative, 0, ulliMagnitude, iFraction);
}


//Truncates towards zero.  Out of range or a NaN is the most negative integer of iBits bits; bSigned says
//which range.
static uint64_t ToInteger(uint64_t ulliBits, int iFraction, int iBits, bool bSigned)
{
	bool		bNegative;
	int			iExponent;
	uint64_t	ulliSignificand;
	int			iClass;
	uint64_t	ulliResult;
	uint64_t	ulliIndefinite;
	uint64_t	ulliLimit;

	iExponent = 0;
	ulliSignificand = 0;
	ulliIndefinite = (uint64_t)1 << (iBits - 1);
	iClass = Unpack(ulliBits, iFraction, &bNegative, &iExponent, &ulliSignificand);
	if (iClass == SOFT_ZERO)
	{
		return 0;
	}
	if (iClass != SOFT_FINITE)
	{
		return ulliIndefinite;
	}

	if (iExponent >= 0)
	{
		if (HighestBit(ulliSignificand) + iExponent >= iBits)
		{
			return ulliIndefinite;
		}
		ulliResult = ulliSignificand << iExponent;
	}
	else if (-iExponent >= 64)
	{
		return 0;
	}
	else
	{
		ulliResult = ulliSignificand >> -iExponent;
	}

	if (bSigned)
	{
		//Up to 2^(bits-1) - 1, or 2^(bits-1) when negative.
		ulliLimit = ulliIndefinite - (bNegative ? 0 : 1);
		if (ulliResult > ulliLimit)
		{
			return ulliIndefinite;
		}
		return bNegative ? (0 - ulliResult) : ulliResult;
	}
	if (bNegative)
	{
		return (ulliResult == 0) ? 0 : ulliIndefinite;
	}
	if ((iBits < 64) && ((ulliResult >> iBits) != 0))
	{
		//Too large: only a double from 2^32 to 2^53 gets here (shifted down), as unsigned.
		return ulliIndefinite;
	}
	return ulliResult;
}


//To a format with iToFraction bits.  NaNs keep the top of their payload and are quietened.
static uint64_t Change(uint64_t ulliBits, int iFromFraction, int iToFraction)
{
	bool		bNegative;
	int			iExponent;
	uint64_t	ulliSignificand;
	int			iClass;
	uint64_t	ulliPayload;

	iExponent = 0;
	ulliSignificand = 0;
	iClass = Unpack(ulliBits, iFromFraction, &bNegative, &iExponent, &ulliSignificand);
	switch (iClass)
	{
	case SOFT_ZERO:
		return bNegative ? SignBit(iToFraction) : 0;
	case SOFT_INFINITY:
		return Infinity(iToFraction, bNegative);
	case SOFT_NAN:
		ulliPayload = ulliBits & FractionMask(iFromFraction);
		if (iToFraction > iFromFraction)
		{
			ulliPayload <<= iToFraction - iFromFraction;
		}
		else
		{
			ulliPayload >>= iFromFraction - iToFraction;
		}
		return Quiet(Infinity(iToFraction, bNegative) | ulliPayload, iToFraction);
	default:
		return RoundPack(bNegative, iExponent, ulliSignificand, iToFraction);
	}
}


//The square root, correctly rounded.  The square root of -0 is -0; of anything else below zero, the default NaN.
static uint64_t SquareRoot(uint64_t ulliBits, int iFraction)
{
	bool		bNegative;
	int			iExponent;
	uint64_t	ulliSignificand;
	int			iClass;
	uint64_t	ulliHigh;
	uint64_t	ulliLow;
	uint64_t	ulliRoot;
	uint64_t	ulliRemainder;
	uint64_t	ulliTrial;
	int			i;

	iExponent = 0;
	ulliSignificand = 0;
	iClass = Unpack(ulliBits, iFraction, &bNegative, &iExponent, &ulliSignificand);
	if (iClass == SOFT_NAN)
	{
		return Quiet(ulliBits, iFraction);
	}
	if (iClass == SOFT_ZERO)
	{
		return ulliBits;
	}
	if (bNegative)
	{
		return DefaultNaN(iFraction);
	}
	if (iClass == SOFT_INFINITY)
	{
		return ulliBits;
	}

	//The significand from 2^61 to 2^63 with an even exponent, then times 2^48: a 112 bit radicand whose root has
	//56 bits, more than a double needs.
	i = 62 - HighestBit(ulliSignificand);
	ulliSignificand <<= i;
	iExponent -= i;
	if ((iExponent & 1) != 0)
	{
		ulliSignificand >>= 1;
		iExponent++;
	}
	ulliHigh = ulliSignificand >> 16;
	ulliLow = ulliSignificand << 48;

	//A bit of the root for each two bits of the radicand, from the top.
	ulliRoot = 0;
	ulliRemainder = 0;
	for (i = 0; i < 56; i++)
	{
		ulliRemainder = (ulliRemainder << 2) | (ulliHigh >> 46);
		ulliHigh = ((ulliHigh << 2) | (ulliLow >> 62)) & (((uint64_t)1 << 48) - 1);
		ulliLow <<= 2;
		ulliTrial = (ulliRoot << 2) | 1;
		if (ulliRemainder >= ulliTrial)
		{
			ulliRemainder -= ulliTrial;
			ulliRoot = (ulliRoot << 1) | 1;
		}
		else
		{
			ulliRoot <<= 1;
		}
	}
	return RoundPack(false, (iExponent - 48) / 2 - 1, (ulliRoot << 1) | ((ulliRemainder != 0) ? 1 : 0), iFraction);
}


//C linkage: softfloat.asm defines the same names on the 65C64.
extern "C"
{
uint32_t SoftFloatAdd(uint32_t uiA, uint32_t uiB)						{ return (uint32_t)Add(uiA, uiB, false, FLOAT_FRACTION); }
uint32_t SoftFloatSubtract(uint32_t uiA, uint32_t uiB)					{ return (uint32_t)Add(uiA, uiB, true, FLOAT_FRACTION); }
uint32_t SoftFloatMultiply(uint32_t uiA, uint32_t uiB)					{ return (uint32_t)Multiply(uiA, uiB, FLOAT_FRACTION); }
uint32_t SoftFloatDivide(uint32_t uiA, uint32_t uiB)					{ return (uint32_t)Divide(uiA, uiB, FLOAT_FRACTION); }
int SoftFloatCompare(uint32_t uiA, uint32_t uiB)						{ return Compare(uiA, uiB, FLOAT_FRACTION); }
uint32_t SoftFloatFromInt(int32_t i)									{ return (uint32_t)FromInteger(i < 0, (i < 0) ? (0 - (uint64_t)(int64_t)i) : (uint64_t)i, FLOAT_FRACTION); }
uint32_t SoftFloatFromUnsigned(uint32_t ui)								{ return (uint32_t)FromInteger(false, ui, FLOAT_FRACTION); }
uint32_t SoftFloatFromLongLong(int64_t lli)								{ return (uint32_t)FromInteger(lli < 0, (lli < 0) ? (0 - (uint64_t)lli) : (uint64_t)lli, FLOAT_FRACTION); }
uint32_t SoftFloatFromUnsignedLongLong(uint64_t ulli)					{ return (uint32_t)FromInteger(false, ulli, FLOAT_FRACTION); }
int32_t SoftFloatToInt(uint32_t ui)										{ return (int32_t)ToInteger(ui, FLOAT_FRACTION, 32, true); }
uint32_t SoftFloatToUnsigned(uint32_t ui)								{ return (uint32_t)ToInteger(ui, FLOAT_FRACTION, 32, false); }
int64_t SoftFloatToLongLong(uint32_t ui)								{ return (int64_t)ToInteger(ui, FLOAT_FRACTION, 64, true); }
uint64_t SoftFloatToUnsignedLongLong(uint32_t ui)						{ return ToInteger(ui, FLOAT_FRACTION, 64, false); }
uint64_t SoftFloatToDouble(uint32_t ui)									{ return Change(ui, FLOAT_FRACTION, DOUBLE_FRACTION); }
uint32_t SoftFloatSqrt(uint32_t ui)										{ return (uint32_t)SquareRoot(ui, FLOAT_FRACTION); }
uint32_t SoftDoubleToFloat(uint64_t ulli)								{ return (uint32_t)Change(ulli, DOUBLE_FRACTION, FLOAT_FRACTION); }

uint64_t SoftDoubleAdd(uint64_t ulliA, uint64_t ulliB)					{ return Add(ulliA, ulliB, false, DOUBLE_FRACTION); }
uint64_t SoftDoubleSubtract(uint64_t ulliA, uint64_t ulliB)				{ return Add(ulliA, ulliB, true, DOUBLE_FRACTION); }
uint64_t SoftDoubleMultiply(uint64_t ulliA, uint64_t ulliB)				{ return Multiply(ulliA, ulliB, DOUBLE_FRACTION); }
uint64_t SoftDoubleDivide(uint64_t ulliA, uint64_t ulliB)				{ return Divide(ulliA, ulliB, DOUBLE_FRACTION); }
int SoftDoubleCompare(uint64_t ulliA, uint64_t ulliB)					{ return Compare(ulliA, ulliB, DOUBLE_FRACTION); }
uint64_t SoftDoubleFromInt(int32_t i)									{ return FromInteger(i < 0, (i < 0) ? (0 - (uint64_t)(int64_t)i) : (uint64_t)i, DOUBLE_FRACTION); }
uint64_t SoftDoubleFromUnsigned(uint32_t ui)							{ return FromInteger(false, ui, DOUBLE_FRACTION); }
uint64_t SoftDoubleFromLongLong(int64_t lli)							{ return FromInteger(lli < 0, (lli < 0) ? (0 - (uint64_t)lli) : (uint64_t)lli, DOUBLE_FRACTION); }
uint64_t SoftDoubleFromUnsignedLongLong(uint64_t ulli)					{ return FromInteger(false, ulli, DOUBLE_FRACTION); }
int32_t SoftDoubleToInt(uint64_t ulli)									{ return (int32_t)ToInteger(ulli, DOUBLE_FRACTION, 32, true); }
uint32_t SoftDoubleToUnsigned(uint64_t ulli)							{ return (uint32_t)ToInteger(ulli, DOUBLE_FRACTION, 32, false); }
int64_t SoftDoubleToLongLong(uint64_t ulli)								{ return (int64_t)ToInteger(ulli, DOUBLE_FRACTION, 64, true); }
uint64_t SoftDoubleToUnsignedLongLong(uint64_t ulli)					{ return ToInteger(ulli, DOUBLE_FRACTION, 64, false); }
uint64_t SoftDoubleSqrt(uint64_t ulli)									{ return SquareRoot(ulli, DOUBLE_FRACTION); }
}
#endif // __65C64__

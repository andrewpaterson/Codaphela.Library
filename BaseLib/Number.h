/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __NUMBER_H__
#define __NUMBER_H__
#include "Chars.h"


#define DEFAULT_WHOLE_NUMBERS 26
#define DEFAULT_DECIMALS 16

#define NUMBER_FLAGS_LAST_DIGIT_MASK	0x00
#define NUMBER_FLAGS_FIRST_DIGIT_MASK	0x00
#define NUMBER_FLAGS_NEGATIVE			0x01
#define NUMBER_FLAGS_NAN				0x02
#define NUMBER_FLAGS_OVERFLOW			0x04
#define NUMBER_FLAGS_UNDERFLOW			0x08
#define NUMBER_FLAGS_DIVISION_BY_ZERO	0x10
#define NUMBER_FIRST_DIGIT_SHIFT		8

#define NUMBER_INFINITE_STRING "inf"
#define NUMBER_NOT_A_NUMBER "nan"


class CNumberDetail
{
protected:
	int16		mcMaxWholeNumbers;
	int16		mcMaxDecimals;
	int16		mcFirstDigit;
	int16		mcLastDigit;
	int16		mcFlags;
};


//The digits are packed BCD, four to a word (the least significant in the low nibble), the least significant word
//first.  The whole numbers and the decimals are each rounded up to whole words, so the decimal point is on a word
//boundary and the same digit of two numbers is in the same nibble of a word.
#define NUMBER_WORD_DIGITS(p) (((p) + 3) & ~3)
#define NUMBER_WORDS(p, q) ((NUMBER_WORD_DIGITS(p) + NUMBER_WORD_DIGITS(q)) >> 2)
#define NUMBER_SIZE(p, q) ((size_t)NUMBER_WORDS(p, q) * sizeof(uint16) + sizeof(CNumberDetail))
#define UNumber(p, q) union { CNumber c; char a[NUMBER_SIZE(p, q)];	}


class CNumberControl;
class CNumber : public CNumberDetail
{
public:

	uint16		mauiDigits[NUMBER_WORDS(DEFAULT_WHOLE_NUMBERS, DEFAULT_DECIMALS)];
											// ..WWWWWWWWWWWWWWWWWWWWWWWWWW.DDDDDDDDDDDDDDDD
											//
											// 4444444444333333333322222222221111111111
											// 32109876543210987654321098765432109876543210 <- Nibble Position (DigitPosition)
											//
											//   ++++++++++++++++++++++++++ ----------------
											//   22222221111111111000000000 0000000001111111 <- Digit Access (Get/Set)
											//   65432109876543210987654321 1234567890123456
	//Initialisation
	CNumber*	Init(void);
	CNumber*	Init(int32 iNumber);
	CNumber*	Init(float32 ff);
	CNumber*	Init(const char* szNumber, size iLen = 0);
	CNumber*	Init(CNumber* pcNumber);
	CNumber*	Zero(void);
	CNumber*	One(bool bPositive = true);
	CNumber*	Two(bool bPositive = true);
	CNumber*	Half(bool bPositive = true);
	CNumber*	Pi(bool bPositive = true);
	CNumber*	E(bool bPositive = true);
	CNumber*	Digit(char cValue);
	CNumber*	Digit(int16 iDigit, char cValue);
	CNumber* 	Copy(CNumber* pcNumber);
	//Two
	//Half
	//All Initialisation functions must be prefixed with Init.
	//Then things like Zero, One, E etc... and operate on an existing number.

	//Initialisation(cMaxWholeNumbers, cMaxDecimals)
	CNumber*	Init(int16 cMaxWholeNumbers, int16 cMaxDecimals);
	CNumber*	Init(int32 iNumber, int16 cMaxWholeNumbers, int16 cMaxDecimals);
#ifdef __W65C816__
	//An int is 16 bits on the W65C816: ints match these exactly (the 32 bit ones would all need a conversion, so none
	//would be chosen).
	CNumber*	Init(int16 iNumber);
	CNumber*	Init(int16 iNumber, int16 cMaxWholeNumbers, int16 cMaxDecimals);
#endif // __W65C816__
	CNumber*	Init(float32 ff, int16 cMaxWholeNumbers, int16 cMaxDecimals);
	CNumber*	Init(const char* szNumber, int16 cMaxWholeNumbers, int16 cMaxDecimals, size iLen = 0);
	CNumber*	Init(CNumber* pcNumber, int16 cMaxWholeNumbers, int16 cMaxDecimals);
	CNumber*	Zero(int16 cMaxWholeNumbers, int16 cMaxDecimals);
	CNumber*	One(int16 cMaxWholeNumbers, int16 cMaxDecimals, bool bPositive = true);
	CNumber*	Two(int16 cMaxWholeNumbers, int16 cMaxDecimals, bool bPositive = true);
	CNumber*	Half(int16 cMaxWholeNumbers, int16 cMaxDecimals, bool bPositive = true);
	CNumber*	Pi(int16 cMaxWholeNumbers, int16 cMaxDecimals, bool bPositive = true);
	CNumber*	E(int16 cMaxWholeNumbers, int16 cMaxDecimals, bool bPositive = true);
	CNumber*	BinaryOne(int16 iBinaryExponent, int16 cMaxWholeNumbers, int16 cMaxDecimals);
	CNumber*	Digit(char cValue, int16 cMaxWholeNumbers, int16 cMaxDecimals);
	CNumber*	Digit(int16 iDigit, char cValue, int16 cMaxWholeNumbers, int16 cMaxDecimals);
	CNumber* 	Copy(CNumber* pcNumber, int16 cMaxWholeNumbers, int16 cMaxDecimals);

	//Initialisation Errors
	CNumber*	NotANumber(void);
	CNumber*	Overflow(int16 iSign);
	CNumber*	DivisionByZero(void);

	//Miscellaneous
	void		SetFirstNonZerotDigit(int16 iDigit);  //Valid: 26 -> 1, not 0, -1 -> -16.
	void		SetLastNonZeroDigit(int16 iDigit);  //Inclusive of this digit.  First always >= Last.
	void		SetSign(int16 iSign);
	int16		GetSign(void);
	void		SetFlag(int16 iFlag);
	void		SetPositive(bool bPositive);
	void		ClearFlag(int16 iFlags);
	int16		GetMaxWholeNumbers(void);
	int16		GetMaxDecimals(void);
	int16		GetFirstNonZeroDigit(void);  //GetFirstNonZeroDigit
	int16		GetLastNonZeroDigit(void);  //GetLastNonZeroDigit
	int16		GetDecimals(void);
	int16		GetWholeNumbers(void);
	size		GetDigitsBetween(void);
	bool		IsNAN(void);
	bool		IsUnderflow(void);
	bool		IsOverflow(void);
	bool		IsNegative(void);
	bool		IsPositive(void);
	bool		IsPositiveOrZero(void);
	bool		IsClean(void);
	bool		IsDivisionByZero(void);
	bool		IsError(void);
	void		AddBinaryOne(int16 iBinaryExponent);
	void		SetDigit(int16 iDigit, char cValue);
	void		SetDigitUnsafe(int16 iDigit, char cValue);
	void		Clean(void);
	void		CleanRight(void);
	void		CleanLeft(void);
	int16		DigitPosition(int16 iDigit);
	int16		PositionDigit(int16 iPosition);
	int16		PrivateFindNonZero(bool bDown);
	char		GetNibble(int16 iPosition);
	void		SetNibble(int16 iPosition, char cValue);
	int16		NumWords(void);
	int16		MinimumPosition(void);
	void		PrivateZeroPositions(int16 iHigh, int16 iLow);
	void		PrivateShiftNibbles(int16 iShift, int16 iFirst, int16 iLast);
	int16		PrivateCompareDigits(CNumber* pcOther, int16 iFirst, int16 iOtherFirst, size uiDigits);
	int16		OffsetDigit(int16 iCurrentDigit, int16 iOffset);
	size		GetDigitsBetween(int16 iFirst, int16 iLast);
	void		RoundSignificant(int16 iSignificantDigits);
	void		PrivateZeroDigits(int16 iFirst, int16 iLast);
	void		PrivateZeroEnds(void);
	bool		IsInteger(void);

	//Logical
	bool		Equals(CNumber* pcNumber);
	bool		GreaterThan(CNumber* pcNumber);
	bool		LessThan(CNumber* pcNumber);
	bool		GreaterThanOrEquals(CNumber* pcNumber);
	bool		LessThanOrEquals(CNumber* pcNumber);
	bool		IsZero(void);
	bool		IsOdd(void);
	bool		IsEven(void);

	//Private Math
	void		PrivateAdd(CNumber* pcNumber);
	void		PrivateSubtract(CNumber* pcNumber);
	uint16		PrivateAddWords(CNumber* pcNumber, int16 iFirst, int16 iLast, bool bSubtract);
	uint16		PrivateRipple(int16 iWord, uint16 uiCarry, bool bSubtract);
	int16		PrivateLowestLast(CNumber* pcNumber);
	bool		PrivateAboveMaximum(void);
	void		PrivateFindEnds(void);
	void		PrivateNoDigits(void);
	bool		PrivateAddPlaced(uint16* pauiSource, int16 iSourceWords, int16 iShift, uint16* pauiPlaced);
	bool		PrivateGreaterThan(CNumber* pcNumber);
	bool		PrivateGreaterThanOrEquals(CNumber* pcNumber);
	char		PrivateDivide(CNumber* pcDivisor, bool* pbExact);
	void		PrivateUnderflow(CNumber* pcNumber);
	bool		PrivateNAN(CNumber* pcNumber);
	bool		PrivateOverflow(CNumber* pcNumber);
	bool		PrivateDivisionByZero(CNumber* pcNumber);
	bool		PrivateError(CNumber* pcNumber);
	void		PrivateIntegerRoot(CNumber* pcRoot);
	void		PrivateIntegerExponent(CNumber* pcExponent);  //The number returned is not this.
	void		PrivateIntegerExponent(int16 iExponent);
	void		PrivateIntegerFactorial(int16 iN, int16 cMaxWholeNumbers, int16 cMaxDecimals);
	void		PrivateInit(void);
	void		PrivateInit(size cWholeNumbers, size cMaxDecimals);
	void		PrivateCopy(CNumber* pcNumber, int16 cMaxWholeNumbers, int16 cMaxDecimals);
	bool		PrivateEquals(CNumber* pcNumber, int16 iDecimals);

	//Mathematical
	CNumber*	Add(CNumber* pcNumber);
	CNumber*	Subtract(CNumber* pcNumber);
	CNumber*	Multiply(CNumber* pcNumber);
	CNumber*	Divide(CNumber* pcNumber);
	CNumber*	Inverse(void);
	CNumber*	Negate(void);
	CNumber*	Absolute(void);
	CNumber*	Decrement(void);
	CNumber*	Increment(void);
	CNumber*	Power(CNumber* pcExponentIn);  //this^pcExponent
	CNumber*	Root(CNumber* pcRoot);  // pcRoot'th of this.
	CNumber*	Antilog(CNumber* pcBase);  //pcBase^this
	CNumber*	Exponential(void);  //e^this
	CNumber*	Logarithm(CNumber* pcBase);  //log_pcBase(this)
	CNumber*	NaturalLogarithm(void);
	CNumber*	CommonLogarithm(void);
	CNumber*	BinaryLogarithm(void);
	CNumber*	SquareRoot(void);
	CNumber*	CubeRoot(void);
	CNumber*	Squared(void);
	CNumber*	Cubed(void);
	CNumber*	InverseSquared(void);
	CNumber*	InverseCubed(void);
	CNumber*	Modulus(CNumber* pcNumber);
	CNumber*	ArithmeticGeometricMean(CNumber* pcOther);
	CNumber*	LogicalNot(void);
	CNumber*	LogicalOr(CNumber* pcRight);
	CNumber*	LogicalAnd(CNumber* pcRight);

	CNumber*	Sine(void);
	CNumber*	Cosine(void);
	CNumber*	Tangent(void);
	CNumber*	ArcSine(void);
	CNumber*	ArcCosine(void);
	CNumber*	ArcTangent(void);
	CNumber*	HyperbolicSine(void);
	CNumber*	HyperbolicCosine(void);
	CNumber*	HyperbolicTangent(void);
	CNumber*	DegreesToRadians(void);
	CNumber*	RadiansToDegrees(void);

	CNumber*	Shift(int16 iOffset);
	CNumber*	Truncate(int16 iDigit);
	CNumber*	TruncateHigh(int16 iDigit);
	CNumber*	Integer(void);
	CNumber*	Fractional(void);
	CNumber*	Ceiling(void);
	CNumber*	Floor(void);

	//Conversion
	int32  		IntValue(void);
	float32		FloatValue(void);
	float64		DoubleValue(void);
	uint64		ULongValue(void);
	uint64		DecimalValue(uint16* puiNumDecimals);
	char		GetDigitUnsafe(int16 iDigit);  //zero returns '.' or '\0'.
	char		SafeGetDigit(int16 iDigit);
	void		Print(CChars* pcChars);
	bool		PrintFloating(CChars* pcChars);
	void		Dump(void);
	void		Dump(bool bNewLine);

	size		ByteSize(void);
};


#endif // __NUMBER_H__


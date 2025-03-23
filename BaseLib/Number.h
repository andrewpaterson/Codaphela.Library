/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#define DEFAULT_DIGITS (DEFAULT_WHOLE_NUMBERS + DEFAULT_DECIMALS)

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


#define NUMBER_SIZE(p, q) (p + q + sizeof(CNumberDetail))
#define UNumber(p, q) union { CNumber c; char a[NUMBER_SIZE(p, q)];	}


class CNumberControl;
class CNumber : public CNumberDetail
{
CONSTRUCTABLE(CNumber);
public:

	char		mcDigits[DEFAULT_DIGITS];		// WWWWWWWWWWWWWWWWWWWWWWWWWW.DDDDDDDDDDDDDDDD
												//
												// 00000000001111111111222222 2222333333333344 <- Array Position
												// 01234567890123456789012345 6789012345678901
												//
												// ++++++++++++++++++++++++++ ----------------
												// 2222222111111111100000000000000000001111111 <- Digit Access (Get/Set)
												// 6543210987654321098765432101234567890123456
	//Initialisation
	CNumber*	Init(void);
	CNumber*	Init(int32 iNumber);
	CNumber*	Init(float32 ff);
	CNumber*	Init(char* szNumber, size iLen = 0);
	CNumber*	Init(CNumber* pcNumber);
	CNumber*	Zero(void);
	CNumber*	One(void);
	CNumber*	Pi(void);
	CNumber*	E(void);
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
	CNumber*	Init(float32 ff, int16 cMaxWholeNumbers, int16 cMaxDecimals);
	CNumber*	Init(const char* szNumber, int16 cMaxWholeNumbers, int16 cMaxDecimals, size iLen = 0);
	CNumber*	Init(CNumber* pcNumber, int16 cMaxWholeNumbers, int16 cMaxDecimals);
	CNumber*	Zero(int16 cMaxWholeNumbers, int16 cMaxDecimals);
	CNumber*	One(int16 cMaxWholeNumbers, int16 cMaxDecimals);
	CNumber*	Pi(int16 cMaxWholeNumbers, int16 cMaxDecimals);
	CNumber*	E(int16 cMaxWholeNumbers, int16 cMaxDecimals);
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
	char*		DigitToArray(int16 iDigit);
	int16		OffsetDigit(int16 iCurrentDigit, int16 iOffset);
	size		GetDigitsBetween(int16 iFirst, int16 iLast);
	void		RoundSignificant(int16 iSignificantDigits);
	void		PrivateZeroDigits(int16 iFirst, int16 iLast);
	void		PrivateZeroEnds(void);
	bool		IsInteger(void);
	bool		DigitsEqual(CNumber* pcTest, int16 iFirstDigit, int16 iLastDigit);

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
	size		PrivateGetIndex(int16 iDigit);

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
	char		GetDigitUnsafe(int16 iDigit);  //zero returns '.' or '\0'.
	char		SafeGetDigit(int16 iDigit);
	void		Print(CChars* pcChars);
	bool		PrintFloating(CChars* pcChars);
	void		Dump(void);
	void		Dump(bool bNewLine);
};


#endif // __NUMBER_H__


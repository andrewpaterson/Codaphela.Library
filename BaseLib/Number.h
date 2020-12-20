/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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

#define NUMBER_SIZE(p, q) (p + q + 10)
#define UNumber(p, q) union { CNumber c; char a[NUMBER_SIZE(p, q)];	}


class CNumberControl;
class CNumber
{
public:
	short		mcMaxWholeNumbers;
	short		mcMaxDecimals;
	short		mcFirstDigit;
	short		mcLastDigit;
	short		mcFlags;

	char		mcDigits[DEFAULT_DIGITS];		// WWWWWWWWWWWWWWWWWWWWWWWWWW.DDDDDDDDDDDDDDDD
												//
												// 01234567891111111111222222 2222333333333344 <- Array Position
												//			 0123456789012345 6789012345678901
												//
												// ++++++++++++++++++++++++++ ----------------
												// 2222222111111111198765432101234567891111111 <- Digit Access (Get/Set)
												// 65432109876543210                   0123456
	//Initialisation
	CNumber*	Init(void);
	CNumber*	Init(int i);
	CNumber*	Init(float ff);
	CNumber*	Init(char* szNumber, int iLen = 0);
	CNumber*	Init(CNumber* pcNumber);
	CNumber*	Zero(void);
	CNumber*	One(void);
	CNumber*	Pi(void);
	CNumber*	E(void);
	CNumber*	Digit(char cValue);
	CNumber*	Digit(int iDigit, char cValue);
	CNumber* 	Copy(CNumber* pcNumber);
	//Two
	//Half
	//All Initialisation functions must be prefixed with Init.
	//Then things like Zero, One, E etc... and operate on an existing number.

	//Initialisation(cMaxWholeNumbers, cMaxDecimals)
	CNumber*	Init(short cMaxWholeNumbers, short cMaxDecimals);
	CNumber*	Init(int i, short cMaxWholeNumbers, short cMaxDecimals);
	CNumber*	Init(float ff, short cMaxWholeNumbers, short cMaxDecimals);
	CNumber*	Init(const char* szNumber, short cMaxWholeNumbers, short cMaxDecimals, int iLen = 0);
	CNumber*	Init(CNumber* pcNumber, short cMaxWholeNumbers, short cMaxDecimals);
	CNumber*	Zero(short cMaxWholeNumbers, short cMaxDecimals);
	CNumber*	One(short cMaxWholeNumbers, short cMaxDecimals);
	CNumber*	Pi(short cMaxWholeNumbers, short cMaxDecimals);
	CNumber*	E(short cMaxWholeNumbers, short cMaxDecimals);
	CNumber*	BinaryOne(int iBinaryExponent, short cMaxWholeNumbers, short cMaxDecimals);
	CNumber*	Digit(char cValue, short cMaxWholeNumbers, short cMaxDecimals);
	CNumber*	Digit(int iDigit, char cValue, short cMaxWholeNumbers, short cMaxDecimals);
	CNumber* 	Copy(CNumber* pcNumber, short cMaxWholeNumbers, short cMaxDecimals);

	//Initialisation Errors
	CNumber*	NotANumber(void);
	CNumber*	Overflow(int iSign);
	CNumber*	DivisionByZero(void);

	//Miscellaneous
	void		SetFirstNonZerotDigit(short iDigit);  //Valid: 26 -> 1, not 0, -1 -> -16.
	void		SetLastNonZeroDigit(short iDigit);  //Inclusive of this digit.  First always >= Last.
	void		SetSign(int iSign);
	int			GetSign(void);
	void		SetFlag(short iFlag);
	void		ClearFlag(short iFlags);
	short		GetFirstNonZeroDigit(void);  //GetFirstNonZeroDigit
	short		GetLastNonZeroDigit(void);  //GetLastNonZeroDigit
	int			GetDecimals(void);
	int			GetWholeNumbers(void);
	int			GetDigitsBetween(void);
	BOOL		IsNAN(void);
	BOOL		IsUnderflow(void);
	BOOL		IsOverflow(void);
	BOOL		IsNegative(void);
	BOOL		IsClean(void);
	BOOL		IsDivisionByZero(void);
	BOOL		IsError(void);
	void		AddBinaryOne(int iBinaryExponent);
	void		SetDigit(int iDigit, char cValue);
	void		SetDigitUnsafe(int iDigit, char cValue);
	void		Clean(void);
	void		CleanRight(void);
	void		CleanLeft(void);
	char*		DigitToArray(int iDigit);
	int			OffsetDigit(int iCurrentDigit, int iOffset);
	int			GetDigitsBetween(int iFirst, int iLast);
	void		RoundSignificant(int iSignificantDigits);
	void		PrivateZeroDigits(int iFirst, int iLast);
	void		PrivateZeroEnds(void);
	BOOL		IsInteger(void);
	BOOL		DigitsEqual(CNumber* pcTest, int iFirstDigit, int iLastDigit);

	//Logical
	BOOL		Equals(CNumber* pcNumber);
	BOOL		GreaterThan(CNumber* pcNumber);
	BOOL		LessThan(CNumber* pcNumber);
	BOOL		GreaterThanOrEquals(CNumber* pcNumber);
	BOOL		LessThanOrEquals(CNumber* pcNumber);
	BOOL		IsZero(void);
	BOOL		IsOdd(void);
	BOOL		IsEven(void);

	//Private Math
	void		PrivateAdd(CNumber* pcNumber);
	void		PrivateSubtract(CNumber* pcNumber);
	BOOL		PrivateGreaterThan(CNumber* pcNumber);
	BOOL		PrivateGreaterThanOrEquals(CNumber* pcNumber);
	char		PrivateDivide(CNumber* pcDivisor, BOOL* pbExact);
	void		PrivateUnderflow(CNumber* pcNumber);
	BOOL		PrivateNAN(CNumber* pcNumber);
	BOOL		PrivateOverflow(CNumber* pcNumber);
	BOOL		PrivateDivisionByZero(CNumber* pcNumber);
	BOOL		PrivateError(CNumber* pcNumber);
	void		PrivateIntegerRoot(CNumber* pcRoot);
	void		PrivateIntegerExponent(CNumber* pcExponent);  //The number returned is not this.
	void		PrivateIntegerExponent(int iExponent);
	void		PrivateIntegerFactorial(int iN, short cMaxWholeNumbers, short cMaxDecimals);
	void		PrivateInit(void);
	void		PrivateInit(short cWholeNumbers, short cMaxDecimals);
	void		PrivateCopy(CNumber* pcNumber, short cMaxWholeNumbers, short cMaxDecimals);
	BOOL		PrivateEquals(CNumber* pcNumber, short iDecimals);
	int			PrivateGetIndex(int iDigit);

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

	CNumber*	Shift(int iOffset);
	CNumber*	Truncate(int iDigit);
	CNumber*	TruncateHigh(int iDigit);
	CNumber*	Integer(void);
	CNumber*	Fractional(void);

	//Conversion
	int  		IntValue(void);
	float		FloatValue(void);
	char		GetDigit(int iDigit);  //zero returns '.' or '\0'.
	void		ToString(CChars* pcChars);
	void		Dump(BOOL bNewLine = TRUE);
};


#endif // __NUMBER_H__


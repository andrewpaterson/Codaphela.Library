#include "BaseLib/FixedPoint.h"
#include "BaseLib/IntegerHelper.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFixedPointDivide(void)
{
	AssertLong(4, DivideRound(7, 2));
	AssertLong(-4, DivideRound(-7, 2));
	AssertLong(-4, DivideRound(7, -2));
	AssertLong(4, DivideRound(-7, -2));
	AssertLong(2, DivideRound(7, 3));
	AssertLong(3, DivideRound(8, 3));
	AssertLong(-3, DivideRound(-8, 3));
	AssertLong(0, DivideRound(0, 5));
	AssertLong(0, DivideRound(1, 3));

	AssertLong(3, DivideFloor(7, 2));
	AssertLong(-4, DivideFloor(-7, 2));
	AssertLong(-4, DivideFloor(7, -2));
	AssertLong(3, DivideFloor(-7, -2));
	AssertLong(-3, DivideFloor(-6, 2));
	AssertLong(0, DivideFloor(0, 2));

	AssertLong(4, DivideCeiling(7, 2));
	AssertLong(-3, DivideCeiling(-7, 2));
	AssertLong(-3, DivideCeiling(7, -2));
	AssertLong(4, DivideCeiling(-7, -2));
	AssertLong(3, DivideCeiling(6, 2));
	AssertLong(0, DivideCeiling(0, 2));

	AssertLong(5000000000LL, DivideRound(10000000000LL, 2));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFixedPointConversion(void)
{
	AssertInt((int32)256, FixedFromInt(1, 8));
	AssertInt((int32)-768, FixedFromInt(-3, 8));
	AssertInt(65536, FixedFromInt(1, 16));

	AssertInt((int32)384, FixedFromFloat(1.5f, 8));
	AssertInt((int32)-384, FixedFromFloat(-1.5f, 8));
	AssertInt((int32)26, FixedFromFloat(0.1f, 8));
	AssertInt((int32)1, FixedFromFloat(0.5f, 0));
	AssertInt((int32)0, FixedFromFloat(-0.5f, 0));
	AssertInt((int32)3, FixedFromFloat(3.0f, 0));

	AssertInt((int32)128, FloatToIntScaled(0.5f, 255));
	AssertInt((int32)255, FloatToIntScaled(1.0f, 255));
	AssertInt((int32)0, FloatToIntScaled(0.0f, 255));
	AssertInt((int32)77, FloatToIntScaled(0.3f, 255));
	AssertInt((int32)-77, FloatToIntScaled(-0.3f, 255));

	AssertFloat(1.5f, FixedToFloat(384, 8), 6);
	AssertFloat(-0.25f, FixedToFloat(-64, 8), 6);

	AssertInt((int32)96, FixedConvert(384, 8, 6));
	AssertInt((int32)1536, FixedConvert(384, 8, 10));
	AssertInt((int32)2, FixedConvert(384, 8, 0));
	AssertInt((int32)-2, FixedConvert(-384, 8, 0));
	AssertInt((int32)1, FixedConvert(383, 8, 0));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFixedPointRounding(void)
{
	AssertInt((int32)1, FixedFloor(384, 8));
	AssertInt((int32)-2, FixedFloor(-384, 8));
	AssertInt((int32)2, FixedFloor(512, 8));
	AssertInt((int32)-2, FixedFloor(-512, 8));

	AssertInt((int32)2, FixedCeiling(384, 8));
	AssertInt((int32)-1, FixedCeiling(-384, 8));
	AssertInt((int32)2, FixedCeiling(512, 8));
	AssertInt((int32)1, FixedCeiling(1, 8));

	AssertInt((int32)2, FixedRound(384, 8));
	AssertInt((int32)-2, FixedRound(-384, 8));
	AssertInt((int32)1, FixedRound(383, 8));
	AssertInt((int32)-1, FixedRound(-383, 8));

	AssertInt((int32)128, FixedFraction(384, 8));
	AssertInt((int32)192, FixedFraction(-320, 8));
	AssertInt((int32)0, FixedFraction(-512, 8));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFixedPointArithmetic(void)
{
	AssertInt((int32)576, FixedMultiply(384, 384, 8));
	AssertInt((int32)-576, FixedMultiply(-384, 384, 8));
	AssertInt((int32)1, FixedMultiply(1, 128, 8));
	AssertInt((int32)-1, FixedMultiply(-1, 128, 8));
	AssertInt(0x7FFF0000, FixedMultiply(0x7FFF0000, 0x10000, 16));

	AssertInt((int32)256, FixedDivide(384, 384, 8));
	AssertInt((int32)85, FixedDivide(256, 768, 8));
	AssertInt((int32)-171, FixedDivide(-512, 768, 8));
	AssertInt(32768, FixedDivide(1, 2, 16));

	AssertInt(2000000000, FixedMultiplyDivide(2000000000, 300000, 300000));
	AssertInt((int32)67, FixedMultiplyDivide(100, 2, 3));

	AssertInt((int32)512, FixedSquareRoot(1024, 8));
	AssertInt((int32)362, FixedSquareRoot(512, 8));
	AssertInt((int32)0, FixedSquareRoot(0, 8));
	AssertInt((int32)16, FixedSquareRoot(1, 8));
	AssertInt((int32)-1, FixedSquareRoot(-1, 8));
}


//////////////////////////////////////////////////////////////////////////
//
// SquareRoot used to divide by zero for 0 and 1.
//
//////////////////////////////////////////////////////////////////////////
void TestFixedPointSquareRoot(void)
{
	AssertLong(0, SquareRoot((int64)0));
	AssertLong(1, SquareRoot((int64)1));
	AssertLong(1, SquareRoot((int64)3));
	AssertLong(2, SquareRoot((int64)4));
	AssertLong(3037000499LL, SquareRoot(0x7FFFFFFFFFFFFFFFLL));
	AssertInt((int32)0, SquareRoot((int32)0));
	AssertInt((int32)1, SquareRoot((int32)1));
	AssertInt(46340, SquareRoot((int32)0x7FFFFFFF));
	AssertShort((int16)0, SquareRoot((int16)0));
	AssertShort((int16)1, SquareRoot((int16)1));
	AssertShort((int16)3, SquareRoot((int16)15));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFixedPointSFixed(void)
{
	typedef SFixed<int16, int32, 14>	SUnit;
	typedef SFixed<int32, int64, 4>		SSubpixel;
	typedef SFixed<int32, int64, 16>	SAxis;
	typedef SFixed<int32, int64, 0>		SWhole;
	typedef SFixed<int64, int64, 4>		SWideSubpixel;

	SSubpixel	s;
	SAxis		sAxis;
	SUnit		sUnit;

	AssertSize(2, sizeof(SUnit));
	AssertSize(4, sizeof(SSubpixel));

	s = SSubpixel::FromInt(3);
	AssertInt((int32)48, s.miRaw);
	s = SSubpixel::FromInt(-3);
	AssertInt((int32)-48, s.miRaw);
	s = SSubpixel::FromFloat(1.5f);
	AssertInt((int32)24, s.miRaw);
	AssertFloat(1.5f, s.ToFloat(), 6);
	AssertDouble(1.5, s.ToDouble(), 10);

	//As FixedFloor, FixedCeiling, FixedRound and FixedFraction for every value either side of zero.
	for (int32 i = -40; i <= 40; i++)
	{
		s = SSubpixel::FromRaw(i);
		AssertInt(FixedFloor(i, 4), s.Floor());
		AssertInt(FixedCeiling(i, 4), s.Ceiling());
		AssertInt(FixedRound(i, 4), s.Round());
		AssertInt(FixedFraction(i, 4), s.Fraction().miRaw);
		AssertInt(FixedConvert(i, 4, 2), (s.To<SFixed<int32, int64, 2> >().miRaw));
		AssertInt(FixedConvert(i, 4, 16), s.To<SAxis>().miRaw);
		AssertInt((int32)i >> 2, (s.ToFloor<SFixed<int32, int64, 2> >().miRaw));
	}

	AssertInt((int32)32, (SSubpixel::FromRaw(20) + SSubpixel::FromRaw(12)).miRaw);
	AssertInt((int32)8, (SSubpixel::FromRaw(20) - SSubpixel::FromRaw(12)).miRaw);
	AssertInt((int32)-20, (-SSubpixel::FromRaw(20)).miRaw);
	s = SSubpixel::FromRaw(20);
	s += SSubpixel::FromRaw(12);
	AssertInt((int32)32, s.miRaw);
	s -= SSubpixel::FromRaw(2);
	AssertInt((int32)30, s.miRaw);

	AssertTrue(SSubpixel::FromRaw(1) < SSubpixel::FromRaw(2));
	AssertTrue(SSubpixel::FromRaw(2) <= SSubpixel::FromRaw(2));
	AssertTrue(SSubpixel::FromRaw(3) > SSubpixel::FromRaw(2));
	AssertTrue(SSubpixel::FromRaw(2) >= SSubpixel::FromRaw(2));
	AssertTrue(SSubpixel::FromRaw(2) == SSubpixel::FromRaw(2));
	AssertTrue(SSubpixel::FromRaw(1) != SSubpixel::FromRaw(2));

	//A multiply widens to N: int16 to int32 and int32 to int64, with both formats' fractions.
	sUnit = SUnit::FromRaw(8192);
	AssertSize(4, sizeof(sUnit * sUnit));
	AssertInt((int32)134217728, (SUnit::FromRaw(16384) * sUnit).miRaw);

	sAxis = SAxis::FromRaw(32768);
	AssertSize(8, sizeof(SSubpixel::FromInt(3) * sAxis));
	AssertLong(48LL * 32768LL, (SSubpixel::FromInt(3) * sAxis).miRaw);

	//A value too big for int32 is kept.
	AssertLong(1152921504606846976LL, (SAxis::FromRaw((int32)1 << 30) * SAxis::FromRaw((int32)1 << 30)).miRaw);

	//To narrows back: half of 3 is 1.5.
	AssertInt((int32)24, (SSubpixel::FromInt(3) * sAxis).To<SSubpixel>().miRaw);

	//Dividing takes the divisor's fractions away, rounding as it says.
	AssertLong(3, (SWhole::FromRaw(7).DivideFloor(SWhole::FromRaw(2))).miRaw);
	AssertLong(4, (SWhole::FromRaw(7).DivideRound(SWhole::FromRaw(2))).miRaw);
	AssertLong(-4, (SWhole::FromRaw(-7).DivideFloor(SWhole::FromRaw(2))).miRaw);
	AssertLong(5, (SWideSubpixel::FromRaw(10).DivideRound(SWhole::FromRaw(2))).miRaw);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFixedPoint(void)
{
	BeginTests();

	TestFixedPointDivide();
	TestFixedPointConversion();
	TestFixedPointRounding();
	TestFixedPointArithmetic();
	TestFixedPointSquareRoot();
	TestFixedPointSFixed();

	TestStatistics();
}

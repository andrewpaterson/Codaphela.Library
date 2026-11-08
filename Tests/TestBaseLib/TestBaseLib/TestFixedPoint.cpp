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


typedef SFixed<int32, int64, 8>		SFixed8;
typedef SFixed<int32, int64, 16>	SFixed16;
typedef SFixed<int32, int64, 0>		SFixed0;
typedef SFixed<int64, int64, 16>	SWideFixed16;
typedef SFixed<int64, int64, 32>	SWideFixed32;
typedef SFixed<int32, int64, 6>		SFixed6;
typedef SFixed<int32, int64, 10>	SFixed10;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFixedPointConversion(void)
{
	AssertInt((int32)256, SFixed8::FromInt(1).miRaw);
	AssertInt((int32)-768, SFixed8::FromInt(-3).miRaw);
	AssertInt((int32)65536, SFixed16::FromInt(1).miRaw);

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

	AssertFloat(1.5f, SFixed8::FromRaw(384).ToFloat(), 6);
	AssertFloat(-0.25f, SFixed8::FromRaw(-64).ToFloat(), 6);

	AssertInt((int32)96, SFixed8::FromRaw(384).To<SFixed6>().miRaw);
	AssertInt((int32)1536, SFixed8::FromRaw(384).To<SFixed10>().miRaw);
	AssertInt((int32)2, SFixed8::FromRaw(384).To<SFixed0>().miRaw);
	AssertInt((int32)-2, SFixed8::FromRaw(-384).To<SFixed0>().miRaw);
	AssertInt((int32)1, SFixed8::FromRaw(383).To<SFixed0>().miRaw);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFixedPointRounding(void)
{
	AssertInt((int32)1, SFixed8::FromRaw(384).Floor());
	AssertInt((int32)-2, SFixed8::FromRaw(-384).Floor());
	AssertInt((int32)2, SFixed8::FromRaw(512).Floor());
	AssertInt((int32)-2, SFixed8::FromRaw(-512).Floor());

	AssertInt((int32)2, SFixed8::FromRaw(384).Ceiling());
	AssertInt((int32)-1, SFixed8::FromRaw(-384).Ceiling());
	AssertInt((int32)2, SFixed8::FromRaw(512).Ceiling());
	AssertInt((int32)1, SFixed8::FromRaw(1).Ceiling());

	AssertInt((int32)2, SFixed8::FromRaw(384).Round());
	AssertInt((int32)-2, SFixed8::FromRaw(-384).Round());
	AssertInt((int32)1, SFixed8::FromRaw(383).Round());
	AssertInt((int32)-1, SFixed8::FromRaw(-383).Round());

	AssertInt((int32)128, SFixed8::FromRaw(384).Fraction().miRaw);
	AssertInt((int32)192, SFixed8::FromRaw(-320).Fraction().miRaw);
	AssertInt((int32)0, SFixed8::FromRaw(-512).Fraction().miRaw);
}


//////////////////////////////////////////////////////////////////////////
//
// A multiply then To rounds to the nearest, and a value made wider first keeps its fractions through a divide.
//
//////////////////////////////////////////////////////////////////////////
void TestFixedPointArithmetic(void)
{
	AssertInt((int32)576, (SFixed8::FromRaw(384) * SFixed8::FromRaw(384)).To<SFixed8>().miRaw);
	AssertInt((int32)-576, (SFixed8::FromRaw(-384) * SFixed8::FromRaw(384)).To<SFixed8>().miRaw);
	AssertInt((int32)1, (SFixed8::FromRaw(1) * SFixed8::FromRaw(128)).To<SFixed8>().miRaw);
	AssertInt((int32)-1, (SFixed8::FromRaw(-1) * SFixed8::FromRaw(128)).To<SFixed8>().miRaw);
	AssertInt((int32)0x7FFF0000, (SFixed16::FromRaw(0x7FFF0000) * SFixed16::FromRaw(0x10000)).To<SFixed16>().miRaw);

	AssertLong(256, SFixed8::FromRaw(384).To<SWideFixed16>().DivideRound(SFixed8::FromRaw(384)).miRaw);
	AssertLong(85, SFixed8::FromRaw(256).To<SWideFixed16>().DivideRound(SFixed8::FromRaw(768)).miRaw);
	AssertLong(-171, SFixed8::FromRaw(-512).To<SWideFixed16>().DivideRound(SFixed8::FromRaw(768)).miRaw);
	AssertLong(32768, SFixed16::FromRaw(1).To<SWideFixed32>().DivideRound(SFixed16::FromRaw(2)).miRaw);

	AssertLong(2000000000, (SFixed0::FromRaw(2000000000) * SFixed0::FromRaw(300000)).DivideRound(SFixed0::FromRaw(300000)).miRaw);
	AssertLong(67, (SFixed0::FromRaw(100) * SFixed0::FromRaw(2)).DivideRound(SFixed0::FromRaw(3)).miRaw);
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

	//Every value either side of zero.
	for (int32 i = -40; i <= 40; i++)
	{
		s = SSubpixel::FromRaw(i);
		AssertInt((int32)DivideFloor(i, 16), s.Floor());
		AssertInt((int32)DivideCeiling(i, 16), s.Ceiling());
		AssertInt((int32)DivideRound(i, 16), s.Round());
		AssertInt(i - (int32)DivideFloor(i, 16) * 16, s.Fraction().miRaw);
		AssertInt((int32)DivideRound(i, 4), (s.To<SFixed<int32, int64, 2> >().miRaw));
		AssertInt(i * 4096, s.To<SAxis>().miRaw);
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

#include "BaseLib/TypeConverter.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTypeConverterFloat64ScaleTruncateToInt8(void)
{
	int8	i8;
	float64	f64;

	f64 = -2;		gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(-128, i8);

	f64 = -1.5;		gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(-128, i8);

	f64 = -1;		gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(-128, i8);

	f64 = -0.999;	gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(-127, i8);

	f64 = -0.9922;	gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(-127, i8);

	f64 = -0.9921;	gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(-126, i8);

	f64 = -0.99;	gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(-126, i8);

	f64 = -0.5;		gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(-64, i8);

	f64 = -0.01;	gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(-1, i8);

	f64 = -0.001;	gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(0, i8);

	f64 = -0.0;		gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(0, i8);


	f64 = 2;		gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(127, i8);

	f64 = 1.5;		gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(127, i8);

	f64 = -1;		gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(-128, i8);

	f64 = 0.999;	gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(126, i8);

	f64 = 0.9922;	gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(126, i8);

	f64 = 0.9921;	gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(125, i8);

	f64 = 0.9842;	gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(124, i8);

	f64 = 0.5;		gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(63, i8);

	f64 = 0.01;		gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(1, i8);

	f64 = 0.001;	gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(0, i8);

	f64 = 0.0;		gcTypeConverter.Do(PT_int8, &i8, PT_float64, &f64);
	AssertInt(0, i8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTypeConverterFloat32ScaleTruncateToInt8(void)
{
	int8	i8;
	float32	f32;

	f32 = -2;		gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(-128, i8);

	f32 = -1.5;		gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(-128, i8);

	f32 = -1;		gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(-128, i8);

	f32 = -0.999;	gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(-127, i8);

	f32 = -0.9922;	gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(-127, i8);

	f32 = -0.9921;	gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(-126, i8);

	f32 = -0.99;	gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(-126, i8);

	f32 = -0.5;		gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(-64, i8);

	f32 = -0.75;	gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(-96, i8);

	f32 = -0.01;	gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(-1, i8);

	f32 = -0.001;	gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(0, i8);

	f32 = -0.0;		gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(0, i8);

	f32 = 0.75;		gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(95, i8);

	f32 = 2;		gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(127, i8);

	f32 = 1.5;		gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(127, i8);

	f32 = -1;		gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(-128, i8);

	f32 = 0.999;	gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(126, i8);

	f32 = 0.9922;	gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(126, i8);

	f32 = 0.9921;	gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(125, i8);

	f32 = 0.9842;	gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(124, i8);

	f32 = 0.5;		gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(63, i8);

	f32 = 0.01;		gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(1, i8);

	f32 = 0.001;	gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(0, i8);

	f32 = 0.0;		gcTypeConverter.Do(PT_int8, &i8, PT_float32, &f32);
	AssertInt(0, i8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTypeConverterUInt16ScaleTruncateToUInt8(void)
{
	uint8	ui8;
	uint16	ui16;

	ui16 = 0;		gcTypeConverter.Do(PT_uint8, &ui8, PT_uint16, &ui16);
	AssertInt(0, ui8);

	ui16 = 0xfeff;	gcTypeConverter.Do(PT_uint8, &ui8, PT_uint16, &ui16);
	AssertInt(0xfe, ui8);

	ui16 = 0xfe00;	gcTypeConverter.Do(PT_uint8, &ui8, PT_uint16, &ui16);
	AssertInt(0xfe, ui8);

	ui16 = 0x100;	gcTypeConverter.Do(PT_uint8, &ui8, PT_uint16, &ui16);
	AssertInt(0x1, ui8);

	ui16 = 0x0ff;	gcTypeConverter.Do(PT_uint8, &ui8, PT_uint16, &ui16);
	AssertInt(0x0, ui8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTypeConverterUInt8ScaleTruncateToUInt8(void)
{
	uint8	ui8Dest;
	uint16	ui8Src;

	ui8Src = 0;		gcTypeConverter.Do(PT_uint8, &ui8Dest, PT_uint8, &ui8Src);
	AssertInt(0, ui8Dest);

	ui8Src = 0xfe;	gcTypeConverter.Do(PT_uint8, &ui8Dest, PT_uint8, &ui8Src);
	AssertInt(0xfe, ui8Dest);

	ui8Src = 0xff;	gcTypeConverter.Do(PT_uint8, &ui8Dest, PT_uint8, &ui8Src);
	AssertInt(0xff, ui8Dest);

	ui8Src = 0x1;	gcTypeConverter.Do(PT_uint8, &ui8Dest, PT_uint8, &ui8Src);
	AssertInt(0x1, ui8Dest);

	ui8Src = 0x7f;	gcTypeConverter.Do(PT_uint8, &ui8Dest, PT_uint8, &ui8Src);
	AssertInt(0x7f, ui8Dest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTypeConverterUInt16ScaleTruncateToInt8(void)
{
	int8	i8;
	uint16	ui16;

	ui16 = 0;		gcTypeConverter.Do(PT_int8, &i8, PT_uint16, &ui16);
	AssertInt(0, i8);

	ui16 = 0xffff;	gcTypeConverter.Do(PT_int8, &i8, PT_uint16, &ui16);
	AssertInt(0x7f, i8);

	ui16 = 0xff00;	gcTypeConverter.Do(PT_int8, &i8, PT_uint16, &ui16);
	AssertInt(0x7f, i8);

	ui16 = 0xfeff;	gcTypeConverter.Do(PT_int8, &i8, PT_uint16, &ui16);
	AssertInt(0x7f, i8);

	ui16 = 0xfe00;	gcTypeConverter.Do(PT_int8, &i8, PT_uint16, &ui16);
	AssertInt(0x7f, i8);

	ui16 = 0xfdff;	gcTypeConverter.Do(PT_int8, &i8, PT_uint16, &ui16);
	AssertInt(0x7e, i8);

	ui16 = 0xfc00;	gcTypeConverter.Do(PT_int8, &i8, PT_uint16, &ui16);
	AssertInt(0x7e, i8);

	ui16 = 0x200;	gcTypeConverter.Do(PT_int8, &i8, PT_uint16, &ui16);
	AssertInt(0x1, i8);

	ui16 = 0x1ff;	gcTypeConverter.Do(PT_int8, &i8, PT_uint16, &ui16);
	AssertInt(0x0, i8);

	ui16 = 0x100;	gcTypeConverter.Do(PT_int8, &i8, PT_uint16, &ui16);
	AssertInt(0x0, i8);

	ui16 = 0x0ff;	gcTypeConverter.Do(PT_int8, &i8, PT_uint16, &ui16);
	AssertInt(0x0, i8);

	ui16 = 0x0;	gcTypeConverter.Do(PT_int8, &i8, PT_uint16, &ui16);
	AssertInt(0x0, i8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTypeConverterInt16ScaleTruncateToUInt8(void)
{
	uint8	ui8;
	int16	i16;

	i16 = 0;		gcTypeConverter.Do(PT_uint8, &ui8, PT_int16, &i16);
	AssertInt(0, ui8);

	i16 = 0x7fff;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int16, &i16);
	AssertInt(0xff, ui8);

	i16 = 0x7f80;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int16, &i16);
	AssertInt(0xff, ui8);

	i16 = 0x7f7f;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int16, &i16);
	AssertInt(0xfe, ui8);

	i16 = 0x7f00;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int16, &i16);
	AssertInt(0xfe, ui8);

	i16 = -0x8000;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int16, &i16);
	AssertInt(0x0, ui8);

	i16 = -0x7fff;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int16, &i16);
	AssertInt(0x0, ui8);

	i16 = -0x7f00;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int16, &i16);
	AssertInt(0x0, ui8);

	i16 = -0x7eff;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int16, &i16);
	AssertInt(0x0, ui8);

	i16 = 0x200;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int16, &i16);
	AssertInt(4, ui8);

	i16 = 0x1ff;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int16, &i16);
	AssertInt(3, ui8);

	i16 = 0x101;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int16, &i16);
	AssertInt(2, ui8);

	i16 = 0x100;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int16, &i16);
	AssertInt(2, ui8);

	i16 = 0x0ff;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int16, &i16);
	AssertInt(1, ui8);

	i16 = 0x080;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int16, &i16);
	AssertInt(0x1, ui8);

	i16 = 0x07f;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int16, &i16);
	AssertInt(0x0, ui8);

	i16 = 0x0;		gcTypeConverter.Do(PT_uint8, &ui8, PT_int16, &i16);
	AssertInt(0x0, ui8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTypeConverterInt8ScaleTruncateToUInt8(void)
{
	uint8	ui8;
	int8	i8;

	i8 = 0;		gcTypeConverter.Do(PT_uint8, &ui8, PT_int8, &i8);
	AssertInt(0, ui8);

	i8 = 0x7f;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int8, &i8);
	AssertInt(0xff, ui8);

	i8 = 0x7e;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int8, &i8);
	AssertInt(0xfd, ui8);

	i8 = 0x7d;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int8, &i8);
	AssertInt(0xfb, ui8);

	i8 = 0x40;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int8, &i8);
	AssertInt(0x80, ui8);

	i8 = 0x41;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int8, &i8);
	AssertInt(0x83, ui8);

	i8 = -0x80;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int8, &i8);
	AssertInt(0x0, ui8);

	i8 = -0x7f;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int8, &i8);
	AssertInt(0x0, ui8);

	i8 = -0x40;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int8, &i8);
	AssertInt(0x0, ui8);

	i8 = -0x1;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int8, &i8);
	AssertInt(0x0, ui8);

	i8 = 0x2;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int8, &i8);
	AssertInt(4, ui8);

	i8 = 0x3;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int8, &i8);
	AssertInt(6, ui8);

	i8 = 0x2;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int8, &i8);
	AssertInt(4, ui8);

	i8 = 0x1;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int8, &i8);
	AssertInt(2, ui8);

	i8 = 0x0;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int8, &i8);
	AssertInt(0, ui8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTypeConverterInt64ScaleTruncateToUInt8(void)
{
	uint8	ui8;
	int64	i64;

	i64 = 0;			gcTypeConverter.Do(PT_uint8, &ui8, PT_int64, &i64);
	AssertInt(0, ui8);

	i64 = 0x7fffffffffffffffLL;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int64, &i64);
	AssertInt(0xff, ui8);

	i64 = 0x7f80000000000000LL;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int64, &i64);
	AssertInt(0xff, ui8);

	i64 = 0x7f7fffffffffffffLL;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int64, &i64);
	AssertInt(0xfe, ui8);

	i64 = 0x7f00000000000000LL;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int64, &i64);
	AssertInt(0xfe, ui8);

	i64 = -0x800000000000000LL;gcTypeConverter.Do(PT_uint8, &ui8, PT_int64, &i64);
	AssertInt(0x0, ui8);

	i64 =-0x7fffffffffffffffLL;gcTypeConverter.Do(PT_uint8, &ui8, PT_int64, &i64);
	AssertInt(0x0, ui8);

	i64 =-0x7f00000000000000LL;gcTypeConverter.Do(PT_uint8, &ui8, PT_int64, &i64);
	AssertInt(0x0, ui8);

	i64 =-0x7effffffffffffffLL;gcTypeConverter.Do(PT_uint8, &ui8, PT_int64, &i64);
	AssertInt(0x0, ui8);

	i64 = 0x200000000000000LL;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int64, &i64);
	AssertInt(4, ui8);

	i64 = 0x1ffffffffffffffLL;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int64, &i64);
	AssertInt(3, ui8);

	i64 = 0x101000000000000LL;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int64, &i64);
	AssertInt(2, ui8);

	i64 = 0x100000000000000LL;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int64, &i64);
	AssertInt(2, ui8);

	i64 = 0x0ffffffffffffffLL;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int64, &i64);
	AssertInt(1, ui8);

	i64 = 0x080000000000000LL;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int64, &i64);
	AssertInt(0x1, ui8);

	i64 = 0x07fffffffffffffLL;	gcTypeConverter.Do(PT_uint8, &ui8, PT_int64, &i64);
	AssertInt(0x0, ui8);

	i64 = 0x000000000000000LL;		gcTypeConverter.Do(PT_uint8, &ui8, PT_int64, &i64);
	AssertInt(0x0, ui8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTypeConverterInt16ScaleTruncateToInt8(void)
{
	int8	i8;
	int16	i16;

	i16 = 0;		gcTypeConverter.Do(PT_int8, &i8, PT_int16, &i16);
	AssertInt(0, i8);

	i16 = -0x8000;	gcTypeConverter.Do(PT_int8, &i8, PT_int16, &i16);
	AssertInt(-0x80, i8);
	AssertInt(-128, i8);

	i16 = -0x7fff;	gcTypeConverter.Do(PT_int8, &i8, PT_int16, &i16);
	AssertInt(-0x7f, i8);

	i16 = -0x7f00;	gcTypeConverter.Do(PT_int8, &i8, PT_int16, &i16);
	AssertInt(-0x7f, i8);

	i16 = -0x7eff;	gcTypeConverter.Do(PT_int8, &i8, PT_int16, &i16);
	AssertInt(-0x7e, i8);

	i16 = -0x0200;	gcTypeConverter.Do(PT_int8, &i8, PT_int16, &i16);
	AssertInt(-2, i8);

	i16 = -0x01ff;	gcTypeConverter.Do(PT_int8, &i8, PT_int16, &i16);
	AssertInt(-1, i8);

	i16 = 0x200;	gcTypeConverter.Do(PT_int8, &i8, PT_int16, &i16);
	AssertInt(2, i8);

	i16 = 0x1ff;	gcTypeConverter.Do(PT_int8, &i8, PT_int16, &i16);
	AssertInt(1, i8);

	i16 = 0x100;	gcTypeConverter.Do(PT_int8, &i8, PT_int16, &i16);
	AssertInt(1, i8);

	i16 = 0x0ff;	gcTypeConverter.Do(PT_int8, &i8, PT_int16, &i16);
	AssertInt(0, i8);

	i16 = 0x1;	gcTypeConverter.Do(PT_int8, &i8, PT_int16, &i16);
	AssertInt(0, i8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTypeConverterInt8ScaleTruncateToInt8(void)
{
	int8	i8Dest;
	int8	i8Src;

	i8Src = 0;		gcTypeConverter.Do(PT_int8, &i8Dest, PT_int8, &i8Src);
	AssertInt(0, i8Dest);

	i8Src = -0x80;	gcTypeConverter.Do(PT_int8, &i8Dest, PT_int8, &i8Src);
	AssertInt(-0x80, i8Dest);
	AssertInt(-128, i8Dest);

	i8Src = -0x7f;	gcTypeConverter.Do(PT_int8, &i8Dest, PT_int8, &i8Src);
	AssertInt(-0x7f, i8Dest);

	i8Src = -0x7e;	gcTypeConverter.Do(PT_int8, &i8Dest, PT_int8, &i8Src);
	AssertInt(-0x7e, i8Dest);

	i8Src = -0x02;	gcTypeConverter.Do(PT_int8, &i8Dest, PT_int8, &i8Src);
	AssertInt(-2, i8Dest);

	i8Src = -0x01;	gcTypeConverter.Do(PT_int8, &i8Dest, PT_int8, &i8Src);
	AssertInt(-1, i8Dest);

	i8Src = 0x2;	gcTypeConverter.Do(PT_int8, &i8Dest, PT_int8, &i8Src);
	AssertInt(2, i8Dest);

	i8Src = 0x1;	gcTypeConverter.Do(PT_int8, &i8Dest, PT_int8, &i8Src);
	AssertInt(1, i8Dest);

	i8Src = 0x0;	gcTypeConverter.Do(PT_int8, &i8Dest, PT_int8, &i8Src);
	AssertInt(0, i8Dest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTypeConverterUInt8ScaleTruncateToInt8(void)
{
	int8	i8;
	uint8	ui8;

	ui8 = 0xff;	gcTypeConverter.Do(PT_int8, &i8, PT_uint8, &ui8);
	AssertInt(0x7f, i8);

	ui8 = 0xfe;	gcTypeConverter.Do(PT_int8, &i8, PT_uint8, &ui8);
	AssertInt(0x7f, i8);

	ui8 = 0xfd;	gcTypeConverter.Do(PT_int8, &i8, PT_uint8, &ui8);
	AssertInt(0x7e, i8);

	ui8 = 0x4;	gcTypeConverter.Do(PT_int8, &i8, PT_uint8, &ui8);
	AssertInt(2, i8);

	ui8 = 0x3;	gcTypeConverter.Do(PT_int8, &i8, PT_uint8, &ui8);
	AssertInt(1, i8);

	ui8 = 0x2;	gcTypeConverter.Do(PT_int8, &i8, PT_uint8, &ui8);
	AssertInt(1, i8);

	ui8 = 0x1;	gcTypeConverter.Do(PT_int8, &i8, PT_uint8, &ui8);
	AssertInt(0, i8);

	ui8 = 0x0;	gcTypeConverter.Do(PT_int8, &i8, PT_uint8, &ui8);
	AssertInt(0, i8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTypeConverter(void)
{
	BeginTests();
	TypeConverterInit();

	TestTypeConverterUInt16ScaleTruncateToUInt8();
	TestTypeConverterUInt8ScaleTruncateToUInt8();

	TestTypeConverterUInt16ScaleTruncateToInt8();
	TestTypeConverterUInt8ScaleTruncateToInt8();

	TestTypeConverterInt16ScaleTruncateToInt8();
	TestTypeConverterInt8ScaleTruncateToInt8();

	TestTypeConverterInt64ScaleTruncateToUInt8();
	TestTypeConverterInt16ScaleTruncateToUInt8();
	TestTypeConverterInt8ScaleTruncateToUInt8();

	TestTypeConverterFloat32ScaleTruncateToInt8();
	TestTypeConverterFloat64ScaleTruncateToInt8();
	
	TypeConverterKill();
	TestStatistics();
}


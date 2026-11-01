#include "BaseLib/GlobalMemory.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Unknowns.h"
#include "StandardLib/Channels.h"
#include "StandardLib/SingleChannelAccessor.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSingleChannelAccessorCastSigned(void)
{
	CChannels					cChannels;
	uint8						auiData[16];
	int16						i16;
	int8						i8;
	float32						f32;
	int32						i32;
	CSingleChannelAccessor		cSingle;

	i16 = -3;
	i8 = -5;
	f32 = -1.5f;
	i32 = -70000;
	memcpy(&auiData[0], &i16, 2);
	memcpy(&auiData[2], &i8, 1);
	auiData[3] = 0;
	memcpy(&auiData[4], &f32, 4);
	memcpy(&auiData[8], &i32, 4);

	cChannels.Init();
	cChannels.BeginChange();
	cChannels.SetData(auiData);
	cChannels.AddChannel(0, PT_int16);
	cChannels.AddChannel(1, PT_int8);
	cChannels.AddChannel(2, PT_uint8);
	cChannels.AddChannel(3, PT_float32);
	cChannels.AddChannel(4, PT_int32);
	cChannels.SetSize(1);
	cChannels.EndChange();
	cSingle.Init(&cChannels);

	AssertFloat(-3.0f, cSingle.GetCastToFloat(0), 1);
	AssertFloat(-5.0f, cSingle.GetCastToFloat(1), 1);
	AssertFloat(0.0f, cSingle.GetCastToFloat(2), 1);
	AssertFloat(-1.5f, cSingle.GetCastToFloat(3), 1);
	AssertFloat(-70000.0f, cSingle.GetCastToFloat(4), 1);

	cChannels.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSingleChannelAccessor(void)
{
	BeginTests();

	MemoryInit();
	TypesInit();
	TypeConverterInit();
	DataIOInit();
	ObjectsInit();

	CChannels					cChannels;
	uint8						cData[9] = "ti@YA,\n#";
	CSingleChannelAccessor		cSingle;
	char						c;
	uint16						s;

	cChannels.Init();
	cChannels.BeginChange();
	cChannels.SetData(cData);
	cChannels.AddChannel(16, 15, 14, 13, PT_nybble);
	cChannels.AddChannel(12, 11, PT_uint8);
	cChannels.AddChannel(10, PT_uint16);
	cChannels.AddChannel(9, 8, 7, 6, PT_bit);
	cChannels.AddChannel(5, 4, PT_crumb);
	cChannels.AddChannel(3, PT_uint8);
	cChannels.SetSize(1);
	cChannels.EndChange();
	cSingle.Init(&cChannels);
	AssertFloat(0.27f, cSingle.GetConvertToFloat(16), 2);
	AssertFloat(0.47f, cSingle.GetConvertToFloat(15), 2);
	AssertFloat(0.60f, cSingle.GetConvertToFloat(14), 2);
	AssertFloat(0.40f, cSingle.GetConvertToFloat(13), 2);
	cSingle.GetNative(16, &c); AssertChar(4, c);
	cSingle.GetNative(15, &c); AssertChar(7, c);
	cSingle.GetNative(14, &c); AssertChar(9, c);
	cSingle.GetNative(13, &c); AssertChar(6, c);

	AssertFloat(0.25f, cSingle.GetConvertToFloat(12), 2);
	AssertFloat(0.35f, cSingle.GetConvertToFloat(11), 2);
	cSingle.GetNative(12, &c); AssertChar('@', c);
	cSingle.GetNative(11, &c); AssertChar('Y', c);

	cSingle.GetNative(10, &s); AssertShortHex((uint16)0x2c41, s);

	AssertFloat(0.0f, cSingle.GetConvertToFloat(9), 1);
	AssertFloat(1.0f, cSingle.GetConvertToFloat(8), 1);
	AssertFloat(0.0f, cSingle.GetConvertToFloat(7), 1);
	AssertFloat(1.0f, cSingle.GetConvertToFloat(6), 1);

	AssertFloat(0.14f, cSingle.GetConvertToFloat(3), 2);
	cSingle.GetNative(3, &c); AssertChar('#', c);

	AssertFloat(4.0f, cSingle.GetCastToFloat(16), 1);
	AssertFloat(9.0f, cSingle.GetCastToFloat(14), 1);
	AssertFloat(64.0f, cSingle.GetCastToFloat(12), 1);
	AssertFloat(89.0f, cSingle.GetCastToFloat(11), 1);
	AssertFloat(11329.0f, cSingle.GetCastToFloat(10), 1);
	AssertFloat(1.0f, cSingle.GetCastToFloat(8), 1);
	AssertFloat(0.0f, cSingle.GetCastToFloat(7), 1);
	AssertFloat(35.0f, cSingle.GetCastToFloat(3), 1);
	AssertFloat(0.0f, cSingle.GetCastToFloat(99), 1);

	cChannels.Kill();

	TestSingleChannelAccessorCastSigned();


	ObjectsKill();
	DataIOKill();
	TypeConverterKill();
	TypesKill();
	MemoryKill();

	TestStatistics();
}


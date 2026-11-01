#include "BaseLib/FloatHelper.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloatHelperTruncateFloat(void)
{
	AssertFloat(0.1234130859375, TruncateFloat(0.12345f, -13), 13);
	AssertFloat(0.1232910156250, TruncateFloat(0.12345f, -12), 13);
	AssertFloat(0.1230468750000, TruncateFloat(0.12345f, -11), 13);
	AssertFloat(0.1230468750000, TruncateFloat(0.12345f, -10), 13);
	AssertFloat(0.1230468750000, TruncateFloat(0.12345f, -9), 13);
	AssertFloat(0.1210937500000, TruncateFloat(0.12345f, -8), 13);
	AssertFloat(0.1171875000000, TruncateFloat(0.12345f, -7), 13);
	AssertFloat(0.1093750000000, TruncateFloat(0.12345f, -6), 13);
	AssertFloat(0.0937500000000, TruncateFloat(0.12345f, -5), 13);
	AssertFloat(0.0625000000000, TruncateFloat(0.12345f, -4), 13);
	AssertFloat(0.0000000000000, TruncateFloat(0.12345f, -3), 13);
	AssertFloat(0.0000000000000, TruncateFloat(0.12345f, -2), 13);
	AssertFloat(0.0000000000000, TruncateFloat(0.12345f, -1), 13);

	AssertFloat(3274.4062500000, TruncateFloat(3274.437f, -5), 13);
	AssertFloat(3274.3750000000, TruncateFloat(3274.437f, -4), 13);
	AssertFloat(3274.3750000000, TruncateFloat(3274.437f, -3), 13);
	AssertFloat(3274.2500000000, TruncateFloat(3274.437f, -2), 13);
	AssertFloat(3274.0000000000, TruncateFloat(3274.437f, -1), 13);
	AssertFloat(3274.0000000000, TruncateFloat(3274.437f, 0), 13);
	AssertFloat(3274.0000000000, TruncateFloat(3274.437f, 1), 13);
	AssertFloat(3272.0000000000, TruncateFloat(3274.437f, 2), 13);
	AssertFloat(3272.0000000000, TruncateFloat(3274.437f, 3), 13);
	AssertFloat(3264.0000000000, TruncateFloat(3274.437f, 4), 13);
	AssertFloat(3264.0000000000, TruncateFloat(3274.437f, 5), 13);
	AssertFloat(3264.0000000000, TruncateFloat(3274.437f, 6), 13);
	AssertFloat(3200.0000000000, TruncateFloat(3274.437f, 7), 13);
	AssertFloat(3072.0000000000, TruncateFloat(3274.437f, 8), 13);
	AssertFloat(3072.0000000000, TruncateFloat(3274.437f, 9), 13);
	AssertFloat(3072.0000000000, TruncateFloat(3274.437f, 10), 13);
	AssertFloat(2048.0000000000, TruncateFloat(3274.437f, 11), 13);
	AssertFloat(0000.0000000000, TruncateFloat(3274.437f, 12), 13);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloatHelperRoundFloat(void)
{
	AssertFloat(0.1234130859375, RoundFloat(0.12345f, -13), 13);
	AssertFloat(0.1235351562500, RoundFloat(0.12345f, -12), 13);
	AssertFloat(0.1235351562500, RoundFloat(0.12345f, -11), 13);
	AssertFloat(0.1230468750000, RoundFloat(0.12345f, -10), 13);
	AssertFloat(0.1230468750000, RoundFloat(0.12345f, -9), 13);
	AssertFloat(0.1250000000000, RoundFloat(0.12345f, -8), 13);
	AssertFloat(0.1250000000000, RoundFloat(0.12345f, -7), 13);
	AssertFloat(0.1250000000000, RoundFloat(0.12345f, -6), 13);
	AssertFloat(0.1250000000000, RoundFloat(0.12345f, -5), 13);
	AssertFloat(0.1250000000000, RoundFloat(0.12345f, -4), 13);
	AssertFloat(0.1250000000000, RoundFloat(0.12345f, -3), 13);
	AssertFloat(0.0000000000000, RoundFloat(0.12345f, -2), 13);
	AssertFloat(0.0000000000000, RoundFloat(0.12345f, -1), 13);

	AssertFloat(3274.4375000000, RoundFloat(3274.437f, -5), 13);
	AssertFloat(3274.4375000000, RoundFloat(3274.437f, -4), 13);
	AssertFloat(3274.3750000000, RoundFloat(3274.437f, -3), 13);
	AssertFloat(3274.5000000000, RoundFloat(3274.437f, -2), 13);
	AssertFloat(3274.5000000000, RoundFloat(3274.437f, -1), 13);
	AssertFloat(3274.0000000000, RoundFloat(3274.437f, 0), 13);
	AssertFloat(3274.0000000000, RoundFloat(3274.437f, 1), 13);
	AssertFloat(3276.0000000000, RoundFloat(3274.437f, 2), 13);
	AssertFloat(3272.0000000000, RoundFloat(3274.437f, 3), 13);
	AssertFloat(3280.0000000000, RoundFloat(3274.437f, 4), 13);
	AssertFloat(3264.0000000000, RoundFloat(3274.437f, 5), 13);
	AssertFloat(3264.0000000000, RoundFloat(3274.437f, 6), 13);
	AssertFloat(3328.0000000000, RoundFloat(3274.437f, 7), 13);
	AssertFloat(3328.0000000000, RoundFloat(3274.437f, 8), 13);
	AssertFloat(3072.0000000000, RoundFloat(3274.437f, 9), 13);
	AssertFloat(3072.0000000000, RoundFloat(3274.437f, 10), 13);
	AssertFloat(4096.0000000000, RoundFloat(3274.437f, 11), 13);
	AssertFloat(4096.0000000000, RoundFloat(3274.437f, 12), 13);
	AssertFloat(0000.0000000000, RoundFloat(3274.437f, 13), 13);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloatHelperTruncateDouble(void)
{
	AssertDouble(0.1234130859375, TruncateFloat(0.12345, -13LL), 13);
	AssertDouble(0.1232910156250, TruncateFloat(0.12345, -12LL), 13);
	AssertDouble(0.1230468750000, TruncateFloat(0.12345, -11LL), 13);
	AssertDouble(0.1230468750000, TruncateFloat(0.12345, -10LL), 13);
	AssertDouble(0.1230468750000, TruncateFloat(0.12345, -9LL), 13);
	AssertDouble(0.1210937500000, TruncateFloat(0.12345, -8LL), 13);
	AssertDouble(0.1171875000000, TruncateFloat(0.12345, -7LL), 13);
	AssertDouble(0.1093750000000, TruncateFloat(0.12345, -6LL), 13);
	AssertDouble(0.0937500000000, TruncateFloat(0.12345, -5LL), 13);
	AssertDouble(0.0625000000000, TruncateFloat(0.12345, -4LL), 13);
	AssertDouble(0.0000000000000, TruncateFloat(0.12345, -3LL), 13);
	AssertDouble(0.0000000000000, TruncateFloat(0.12345, -2LL), 13);
	AssertDouble(0.0000000000000, TruncateFloat(0.12345, -1LL), 13);

	AssertDouble(3274.4062500000, TruncateFloat(3274.437, -5LL), 13);
	AssertDouble(3274.3750000000, TruncateFloat(3274.437, -4LL), 13);
	AssertDouble(3274.3750000000, TruncateFloat(3274.437, -3LL), 13);
	AssertDouble(3274.2500000000, TruncateFloat(3274.437, -2LL), 13);
	AssertDouble(3274.0000000000, TruncateFloat(3274.437, -1LL), 13);
	AssertDouble(3274.0000000000, TruncateFloat(3274.437, 0LL), 13);
	AssertDouble(3274.0000000000, TruncateFloat(3274.437, 1LL), 13);
	AssertDouble(3272.0000000000, TruncateFloat(3274.437, 2LL), 13);
	AssertDouble(3272.0000000000, TruncateFloat(3274.437, 3LL), 13);
	AssertDouble(3264.0000000000, TruncateFloat(3274.437, 4LL), 13);
	AssertDouble(3264.0000000000, TruncateFloat(3274.437, 5LL), 13);
	AssertDouble(3264.0000000000, TruncateFloat(3274.437, 6LL), 13);
	AssertDouble(3200.0000000000, TruncateFloat(3274.437, 7LL), 13);
	AssertDouble(3072.0000000000, TruncateFloat(3274.437, 8LL), 13);
	AssertDouble(3072.0000000000, TruncateFloat(3274.437, 9LL), 13);
	AssertDouble(3072.0000000000, TruncateFloat(3274.437, 10LL), 13);
	AssertDouble(2048.0000000000, TruncateFloat(3274.437, 11LL), 13);
	AssertDouble(0000.0000000000, TruncateFloat(3274.437, 12LL), 13);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloatHelperRoundDouble(void)
{
	AssertDouble(0.1234130859375, RoundFloat(0.12345, -13LL), 13);
	AssertDouble(0.1235351562500, RoundFloat(0.12345, -12LL), 13);
	AssertDouble(0.1235351562500, RoundFloat(0.12345, -11LL), 13);
	AssertDouble(0.1230468750000, RoundFloat(0.12345, -10LL), 13);
	AssertDouble(0.1230468750000, RoundFloat(0.12345, -9LL), 13);
	AssertDouble(0.1250000000000, RoundFloat(0.12345, -8LL), 13);
	AssertDouble(0.1250000000000, RoundFloat(0.12345, -7LL), 13);
	AssertDouble(0.1250000000000, RoundFloat(0.12345, -6LL), 13);
	AssertDouble(0.1250000000000, RoundFloat(0.12345, -5LL), 13);
	AssertDouble(0.1250000000000, RoundFloat(0.12345, -4LL), 13);
	AssertDouble(0.1250000000000, RoundFloat(0.12345, -3LL), 13);
	AssertDouble(0.0000000000000, RoundFloat(0.12345, -2LL), 13);
	AssertDouble(0.0000000000000, RoundFloat(0.12345, -1LL), 13);

	AssertDouble(3274.4375000000, RoundFloat(3274.437, -5LL), 13);
	AssertDouble(3274.4375000000, RoundFloat(3274.437, -4LL), 13);
	AssertDouble(3274.3750000000, RoundFloat(3274.437, -3LL), 13);
	AssertDouble(3274.5000000000, RoundFloat(3274.437, -2LL), 13);
	AssertDouble(3274.5000000000, RoundFloat(3274.437, -1LL), 13);
	AssertDouble(3274.0000000000, RoundFloat(3274.437, 0LL), 13);
	AssertDouble(3274.0000000000, RoundFloat(3274.437, 1LL), 13);
	AssertDouble(3276.0000000000, RoundFloat(3274.437, 2LL), 13);
	AssertDouble(3272.0000000000, RoundFloat(3274.437, 3LL), 13);
	AssertDouble(3280.0000000000, RoundFloat(3274.437, 4LL), 13);
	AssertDouble(3264.0000000000, RoundFloat(3274.437, 5LL), 13);
	AssertDouble(3264.0000000000, RoundFloat(3274.437, 6LL), 13);
	AssertDouble(3328.0000000000, RoundFloat(3274.437, 7LL), 13);
	AssertDouble(3328.0000000000, RoundFloat(3274.437, 8LL), 13);
	AssertDouble(3072.0000000000, RoundFloat(3274.437, 9LL), 13);
	AssertDouble(3072.0000000000, RoundFloat(3274.437, 10LL), 13);
	AssertDouble(4096.0000000000, RoundFloat(3274.437, 11LL), 13);
	AssertDouble(4096.0000000000, RoundFloat(3274.437, 12LL), 13);
	AssertDouble(0000.0000000000, RoundFloat(3274.437, 13LL), 13);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTruncateFloat(void)
{
	AssertFloat(1.0f, TruncateFloat(1.25f, 0), 7);
	AssertFloat(13.0f, TruncateFloat(13.5493f, 0), 7);
	AssertFloat(8.0f, TruncateFloat(13.5493f, 3), 7);  //mask 2^3 = 8 = %1000  value 13 = %1011
	AssertFloat(13.53125f, TruncateFloat(13.5493f, -5), 7);
	AssertFloat(0.0f, TruncateFloat(1.0f, 1), 7);
	AssertFloat(-1.0f, TruncateFloat(-1.25f, 0), 7);
	AssertFloat(-13.0f, TruncateFloat(-13.5493f, 0), 7);
	AssertFloat(-8.0f, TruncateFloat(-13.5493f, 3), 7);
	AssertFloat(-13.53125f, TruncateFloat(-13.5493f, -5), 7);
	AssertFloat(0.8125f, TruncateFloat(0.824f, -5), 7);
	AssertFloat(0.8125f, TruncateFloat(0.817f, -5), 7);
	AssertFloat(280.39874f, TruncateFloat(280.39874f, -16), 7);
	AssertFloat(80.39874f, TruncateFloat(80.39874f, -16), 7);
	AssertFloat(1000000.0f, TruncateFloat(1000000.0f, -16), 7);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRoundFloat(void)
{
	AssertFloat(1.0f, RoundFloat(0.75f, 0), 7);
	AssertFloat(1.0f, RoundFloat(1.0f, 0), 7);
	AssertFloat(1.0f, RoundFloat(0.5f, 0), 7);
	AssertFloat(1.0f, RoundFloat(1.49f, 0), 7);
	AssertFloat(0.0f, RoundFloat(0.25f, 0), 7);
	AssertFloat(-1.0f, RoundFloat(-0.75f, 0), 7);
	AssertFloat(-1.0f, RoundFloat(-1.0f, 0), 7);
	AssertFloat(-1.0f, RoundFloat(-0.5f, 0), 7);
	AssertFloat(0.0f, RoundFloat(-0.25f, 0), 7);
	AssertFloat(2.0f, RoundFloat(2.9f, 1), 7);
	AssertFloat(2.0f, RoundFloat(1.0f, 1), 7);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloatHelper(void)
{
	TestFloatHelperTruncateFloat();
	TestFloatHelperRoundFloat();
	TestFloatHelperTruncateDouble();
	TestFloatHelperRoundDouble();
	TestTruncateFloat();
	TestRoundFloat();
}


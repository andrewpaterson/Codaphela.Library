#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "InputLib/InputDataFormat.h"
#include "InputLib/InputSourceValue.h"
#include "InputLib/InputSourceChannel.h"
#include "TestLib/Assert.h"


#define INPUT_CHANNEL_INT8		0
#define INPUT_CHANNEL_INT16		1
#define INPUT_CHANNEL_UINT8		2
#define INPUT_CHANNEL_UINT32	3
#define INPUT_CHANNEL_FLOAT32	4
#define INPUT_CHANNEL_NYBBLE	5


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetInputSourceChannelData(CInputDataFormat* pcFormat, uint8* puiData, size iChannel, void* pvValue, size iByteSize)
{
	CChannel*	pcChannel;

	pcChannel = pcFormat->Get(iChannel);
	memcpy(&puiData[pcChannel->miByteOffset], pvValue, iByteSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CompareInputSourceChannel(CInputSourceValue* pcValue, size iChannel, EInputSourceChannelComparator eCompare, M tTest, void* pvData)
{
	CInputSouceChannel*	pcSourceChannel;
	char				acTest[8];

	memset(acTest, 0, 8);
	memcpy(acTest, &tTest, sizeof(M));
	pcSourceChannel = pcValue->AddChannel(iChannel, eCompare, acTest);
	return pcSourceChannel->Compare(pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestInputSourceChannelCompare(void)
{
	CInputDataFormat	cFormat;
	CInputSourceValue	cValue;
	uint8				auiData[64];
	int8				i8;
	int16				i16;
	uint8				ui8;
	uint32				ui32;
	float32				f32;
	CChannel*			pcNybble;

	cFormat.Init();
	cFormat.Add(PT_int8, (char*)"Int8");
	cFormat.Add(PT_int16, (char*)"Int16");
	cFormat.Add(PT_uint8, (char*)"UInt8");
	cFormat.Add(PT_uint32, (char*)"UInt32");
	cFormat.Add(PT_float32, (char*)"Float32");
	cFormat.Add(PT_nybble, (char*)"Nybble");
	cFormat.Done();

	cValue.Init(NULL, &cFormat, (size)0);

	memset(auiData, 0, 64);
	i8 = -5;		SetInputSourceChannelData(&cFormat, auiData, INPUT_CHANNEL_INT8, &i8, 1);
	i16 = -3;		SetInputSourceChannelData(&cFormat, auiData, INPUT_CHANNEL_INT16, &i16, 2);
	ui8 = 200;		SetInputSourceChannelData(&cFormat, auiData, INPUT_CHANNEL_UINT8, &ui8, 1);
	ui32 = 70000;	SetInputSourceChannelData(&cFormat, auiData, INPUT_CHANNEL_UINT32, &ui32, 4);
	f32 = 1.5f;		SetInputSourceChannelData(&cFormat, auiData, INPUT_CHANNEL_FLOAT32, &f32, 4);
	pcNybble = cFormat.Get(INPUT_CHANNEL_NYBBLE);
	auiData[pcNybble->miBitOffset / 8] |= (uint8)(9 << (pcNybble->miBitOffset % 8));

	//int8 has no entries in the old operator tables so only worked after the rewrite.
	AssertTrue(CompareInputSourceChannel<int8>(&cValue, INPUT_CHANNEL_INT8, ISCC_Equals, -5, auiData));
	AssertTrue(CompareInputSourceChannel<int8>(&cValue, INPUT_CHANNEL_INT8, ISCC_LessThan, -4, auiData));
	AssertFalse(CompareInputSourceChannel<int8>(&cValue, INPUT_CHANNEL_INT8, ISCC_LessThan, -6, auiData));
	AssertTrue(CompareInputSourceChannel<int8>(&cValue, INPUT_CHANNEL_INT8, ISCC_NotEquals, 0, auiData));

	AssertTrue(CompareInputSourceChannel<int16>(&cValue, INPUT_CHANNEL_INT16, ISCC_LessThanEquals, -3, auiData));
	AssertTrue(CompareInputSourceChannel<int16>(&cValue, INPUT_CHANNEL_INT16, ISCC_GreaterThanEquals, -3, auiData));
	AssertTrue(CompareInputSourceChannel<int16>(&cValue, INPUT_CHANNEL_INT16, ISCC_GreaterThan, -4, auiData));
	AssertFalse(CompareInputSourceChannel<int16>(&cValue, INPUT_CHANNEL_INT16, ISCC_GreaterThan, -3, auiData));

	//200 is negative if treated as signed.
	AssertTrue(CompareInputSourceChannel<uint8>(&cValue, INPUT_CHANNEL_UINT8, ISCC_GreaterThan, 100, auiData));
	AssertTrue(CompareInputSourceChannel<uint8>(&cValue, INPUT_CHANNEL_UINT8, ISCC_Equals, 200, auiData));

	AssertTrue(CompareInputSourceChannel<uint32>(&cValue, INPUT_CHANNEL_UINT32, ISCC_Equals, 70000, auiData));
	AssertFalse(CompareInputSourceChannel<uint32>(&cValue, INPUT_CHANNEL_UINT32, ISCC_LessThanEquals, 69999, auiData));

	AssertTrue(CompareInputSourceChannel<float32>(&cValue, INPUT_CHANNEL_FLOAT32, ISCC_GreaterThan, 1.0f, auiData));
	AssertFalse(CompareInputSourceChannel<float32>(&cValue, INPUT_CHANNEL_FLOAT32, ISCC_NotEquals, 1.5f, auiData));

	//Bit packed channels are compared as uint8.
	AssertTrue(CompareInputSourceChannel<uint8>(&cValue, INPUT_CHANNEL_NYBBLE, ISCC_Equals, 9, auiData));
	AssertTrue(CompareInputSourceChannel<uint8>(&cValue, INPUT_CHANNEL_NYBBLE, ISCC_LessThan, 10, auiData));

	AssertTrue(CompareInputSourceChannel<uint8>(&cValue, INPUT_CHANNEL_UINT8, ISCC_AlwaysValid, 0, auiData));
	AssertFalse(CompareInputSourceChannel<uint8>(&cValue, INPUT_CHANNEL_UINT8, ISCC_Unknown, 200, auiData));

	cValue.Kill();
	cFormat.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestInputSourceChannel(void)
{
	BeginTests();
	ObjectsInit();

	TestInputSourceChannelCompare();

	ObjectsKill();
	TestStatistics();
}


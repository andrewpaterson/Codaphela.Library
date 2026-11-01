#include "BaseLib/GlobalMemory.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Unknowns.h"
#include "StandardLib/Channels.h"
#include "StandardLib/ChannelsAccessor.h"
#include "StandardLib/ChannelsAccessorCreator.h"
#include "TestLib/Assert.h"

#define CHANNEL_NAME_JACK	101
#define CHANNEL_NAME_BOB	7
#define CHANNEL_NAME_ALICE	3


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChannelsAccessorContiguous(void)
{
	CChannels*					pcChannels;
	CChannelsAccessor*			pcAccessor;
	CChannelsAccessorCreator	cCreator;
	void*						pvData;
	int32						iData;
	uint16						sData;

	pcChannels = UMalloc(CChannels);
	pcChannels->Init();
	pcChannels->BeginChange();
	pcChannels->SetSize(2);
	pcChannels->AddChannel(CHANNEL_NAME_BOB, PT_int32);
	pcChannels->EndChange();

	cCreator.Init(pcChannels);
	cCreator.AddAccess(CHANNEL_NAME_BOB, PT_Undefined);
	pcAccessor = cCreator.CreateAndKill();
	AssertString("CChannelsAccessorContiguous", pcAccessor->ClassName());
	pvData = pcAccessor->Get(0);
	AssertPointer(pcChannels->GetData(), pvData);
	iData = 784;
	pcAccessor->Set(1, &iData);
	pvData = pcAccessor->Get(1);
	AssertInt((int32)784, *(int32*)pvData);
	pcAccessor->Kill();

	pcChannels->BeginChange();
	pcChannels->AddChannel(CHANNEL_NAME_ALICE, PT_int16);
	pcChannels->EndChange();

	cCreator.Init(pcChannels);
	cCreator.AddAccess(CHANNEL_NAME_BOB, PT_Undefined);
	pcAccessor = cCreator.CreateAndKill();
	AssertString("CChannelsAccessorContiguous", pcAccessor->ClassName());
	pvData = pcAccessor->Get(1);
	AssertInt((int32)784, *(int32*)pvData);
	pcAccessor->Kill();

	cCreator.Init(pcChannels);
	cCreator.AddAccess(CHANNEL_NAME_ALICE, PT_Undefined);
	pcAccessor = cCreator.CreateAndKill();
	AssertString("CChannelsAccessorContiguous", pcAccessor->ClassName());
	pvData = pcAccessor->Get(0);
	AssertPointer(RemapSinglePointer(pcChannels->GetData(), 4), pvData);
	sData = 602;
	pcAccessor->Set(1, &sData);
	pvData = pcAccessor->Get(1);
	AssertShort((uint16)602, *(uint16*)pvData);
	pcAccessor->Kill();

	cCreator.Init(pcChannels);
	cCreator.AddAccess(CHANNEL_NAME_BOB, CHANNEL_NAME_ALICE, PT_Undefined);
	pcAccessor = cCreator.CreateAndKill();
	AssertString("CChannelsAccessorContiguous", pcAccessor->ClassName());
	AssertSize(6, pcAccessor->GetByteSize());
	pvData = pcAccessor->Get(1);
	AssertInt((int32)784, *(int32*)pvData);
	pvData = RemapSinglePointer(pvData, 4);
	AssertInt(602, *(uint16*)pvData);
	pcAccessor->Kill();

	pcChannels->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChannelsAccessorByteAligned(void)
{
	CChannels*					pcChannels;
	CChannelsAccessor*			pcAccessor;
	CChannelsAccessorCreator	cCreator;
	uint16*						psData;
	uint16						sData[3];
	int32						iData;

	pcChannels = UMalloc(CChannels);
	pcChannels->Init();
	pcChannels->BeginChange();
	pcChannels->SetSize(2);
	pcChannels->AddChannel(CHANNEL_NAME_JACK, PT_int16);
	pcChannels->AddChannel(CHANNEL_NAME_BOB, PT_int32);
	pcChannels->AddChannel(CHANNEL_NAME_ALICE, PT_int16);
	pcChannels->EndChange();

	cCreator.Init(pcChannels);
	cCreator.AddAccess(CHANNEL_NAME_JACK, PT_Undefined);
	cCreator.AddAccess(CHANNEL_NAME_ALICE, PT_Undefined);
	pcAccessor = cCreator.CreateAndKill();
	AssertString("CChannelsAccessorByteAligned", pcAccessor->ClassName());
	sData[0] = 784;
	sData[1] = 602;
	pcAccessor->Set(1, &sData);
	AssertShort((uint16)784, *(uint16*)RemapSinglePointer(pcChannels->GetData(), 8 + 0));
	AssertShort((uint16)602, *(uint16*)RemapSinglePointer(pcChannels->GetData(), 8 + 6));
	psData = (uint16*)pcAccessor->Get(1);
	AssertShort((uint16)784, psData[0]);
	AssertShort((uint16)602, psData[1]);
	pcAccessor->Kill();

	cCreator.Init(pcChannels);
	cCreator.AddAccess(CHANNEL_NAME_BOB, PT_Undefined);
	pcAccessor = cCreator.CreateAndKill();
	iData = 999999;
	pcAccessor->Set(1, &iData);
	pcAccessor->Kill();

	cCreator.Init(pcChannels);
	cCreator.AddAccess(CHANNEL_NAME_ALICE, PT_Undefined);
	cCreator.AddAccess(CHANNEL_NAME_BOB, PT_Undefined);
	pcAccessor = cCreator.CreateAndKill();
	AssertString("CChannelsAccessorByteAligned", pcAccessor->ClassName());
	psData = (uint16*)pcAccessor->Get(1);
	AssertShort((uint16)602, psData[0]);
	AssertInt(999999, *(int32*)&psData[1]);
	sData[0] = 22172;
	sData[1] = 18432;
	sData[2] = 30812;
	pcAccessor->Set(0, sData);
	AssertInt(2019313664, *(int32*)RemapSinglePointer(pcChannels->GetData(), 2));
	AssertShort((uint16)22172, *(uint16*)RemapSinglePointer(pcChannels->GetData(), 6));
	pcAccessor->Kill();

	pcChannels->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChannelsAccessorTypeConvert(void)
{
	CChannels*					pcChannels;
	CChannelsAccessor*			pcAccessor;
	CChannelsAccessorCreator	cCreator;
	float*						pfData;
	float						afData[3];

	pcChannels = UMalloc(CChannels);
	pcChannels->Init();
	pcChannels->BeginChange();
	pcChannels->SetSize(2);
	pcChannels->AddChannel(CHANNEL_NAME_JACK, PT_uint16);
	pcChannels->AddChannel(CHANNEL_NAME_BOB, PT_uint32);
	pcChannels->AddChannel(CHANNEL_NAME_ALICE, PT_uint16);
	pcChannels->EndChange();
	pcChannels->Black();

	cCreator.Init(pcChannels);
	cCreator.AddAccess(CHANNEL_NAME_JACK, PT_float32);
	cCreator.AddAccess(CHANNEL_NAME_ALICE, PT_float32);
	pcAccessor = cCreator.CreateAndKill();
	AssertString("CChannelsAccessorTypeConverter", pcAccessor->ClassName());
	afData[0] = 1.0f;
	afData[1] = 0.34f;
	pcAccessor->Set(1, afData);
	AssertShortHex((uint16)0xffff, *(uint16*)RemapSinglePointer(pcChannels->GetData(), 8 + 0));
	AssertShortHex((uint16)0x5709, *(uint16*)RemapSinglePointer(pcChannels->GetData(), 8 + 6));
	pfData = (float*)pcAccessor->Get(1);
	AssertFloat(1.0f, pfData[0], 2);
	AssertFloat(0.34f, pfData[1], 2);
	pcAccessor->Kill();

	cCreator.Init(pcChannels);
	cCreator.AddAccess(CHANNEL_NAME_BOB, PT_float32);
	pcAccessor = cCreator.CreateAndKill();
	AssertString("CChannelsAccessorTypeConverter", pcAccessor->ClassName());
	afData[0] = 0.8f;
	pcAccessor->Set(1, afData);
	AssertIntHex((uint32)0xcccccd00, *(uint32*)RemapSinglePointer(pcChannels->GetData(), 8 + 2));
	pcAccessor->Kill();

	cCreator.Init(pcChannels);
	cCreator.AddAccess(CHANNEL_NAME_ALICE, PT_float32);
	cCreator.AddAccess(CHANNEL_NAME_BOB, PT_float32);
	pcAccessor = cCreator.CreateAndKill();
	AssertString("CChannelsAccessorTypeConverter", pcAccessor->ClassName());
	pfData = (float*)pcAccessor->Get(1);
	AssertFloat(0.34f, pfData[0], 2);
	AssertFloat(0.80f, pfData[1], 2);
	pcAccessor->Kill();

	pcChannels->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChannelsAccessorChannelBitty(void)
{
	CChannels*					pcChannels;
	CChannelsAccessor*			pcAccessor;
	CChannelsAccessorCreator	cCreator;
	uint8*				pucData;
	uint8				aucData[3];
	float*						pfData;

	pcChannels = UMalloc(CChannels);
	pcChannels->Init();
	pcChannels->BeginChange();
	pcChannels->SetSize(1);
	pcChannels->AddChannel(CHANNEL_NAME_JACK, PT_bit);
	pcChannels->AddChannel(CHANNEL_NAME_BOB, PT_crumb);
	pcChannels->AddChannel(CHANNEL_NAME_ALICE, PT_bit);
	pcChannels->EndChange();
	pcChannels->Black();
	AssertSize(1, pcChannels->GetByteSize());
	AssertSize(1, pcChannels->GetSize());

	cCreator.Init(pcChannels);
	cCreator.AddAccess(CHANNEL_NAME_JACK, PT_uint8);
	cCreator.AddAccess(CHANNEL_NAME_BOB, PT_uint8);
	cCreator.AddAccess(CHANNEL_NAME_ALICE, PT_uint8);
	pcAccessor = cCreator.CreateAndKill();
	AssertString("CChannelsAccessorChannelBitty", pcAccessor->ClassName());
	aucData[0] = 0xff;
	aucData[1] = 0x55;
	aucData[2] = 0xff;
	pcAccessor->Set(0, aucData);
	AssertChar(0xb, *pcChannels->GetData());  //Or maybe 0xd.  I'm not sure of the ordering
	pucData = (uint8*)pcAccessor->Get(0);
	AssertChar((char)0xff, pucData[0]);
	AssertChar((char)0x55, pucData[1]);
	AssertChar((char)0xff, pucData[2]);
	pcAccessor->Kill();

	pcChannels->BeginChange();
	pcChannels->SetSize(2);
	pcChannels->EndChange();
	AssertSize(1, pcChannels->GetByteSize());
	AssertSize(2, pcChannels->GetSize());

	cCreator.Init(pcChannels);
	cCreator.AddAccess(CHANNEL_NAME_JACK, PT_uint8);
	cCreator.AddAccess(CHANNEL_NAME_BOB, PT_uint8);
	cCreator.AddAccess(CHANNEL_NAME_ALICE, PT_uint8);
	pcAccessor = cCreator.CreateAndKill();
	AssertString("CChannelsAccessorChannelBitty", pcAccessor->ClassName());
	pucData = (uint8*)pcAccessor->Get(0);
	AssertChar((char)0xff, pucData[0]);
	AssertChar((char)0x55, pucData[1]);
	AssertChar((char)0xff, pucData[2]);

	aucData[0] = 0x00;
	aucData[1] = 0xaa;
	aucData[2] = 0xff;
	pcAccessor->Set(1, aucData);
	AssertChar(0xc, ((uint8)*pcChannels->GetData()) >> 4);  //Or maybe 0xd.  I'm not sure of the ordering
	pucData = (uint8*)pcAccessor->Get(1);
	AssertChar((char)0x00, pucData[0]);
	AssertChar((char)0xaa, pucData[1]);
	AssertChar((char)0xff, pucData[2]);
	pcAccessor->Kill();

	cCreator.Init(pcChannels);
	cCreator.AddAccess(CHANNEL_NAME_ALICE, PT_float32);
	cCreator.AddAccess(CHANNEL_NAME_BOB, PT_float32);
	pcAccessor = cCreator.CreateAndKill();
	AssertString("CChannelsAccessorChannelBitty", pcAccessor->ClassName());
	pfData = (float*)pcAccessor->Get(0);
	AssertFloat(1.0f, pfData[0], 2);
	AssertFloat(0.33f, pfData[1], 2);
	pfData = (float*)pcAccessor->Get(1);
	AssertFloat(1.0f, pfData[0], 2);
	AssertFloat(0.67f, pfData[1], 2);
	pcAccessor->Kill();

	pcChannels->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChannelsAccessorAccessBitty(void)
{
	CChannels*					pcChannels;
	CChannelsAccessor*			pcAccessor;
	CChannelsAccessorCreator	cCreator;
	uint8*				pucData;
	uint8				aucData[2];

	pcChannels = UMalloc(CChannels);
	pcChannels->Init();
	pcChannels->BeginChange();
	pcChannels->SetSize(1);
	pcChannels->AddChannel(CHANNEL_NAME_JACK, PT_uint16);
	pcChannels->AddChannel(CHANNEL_NAME_BOB, PT_uint16);
	pcChannels->AddChannel(CHANNEL_NAME_ALICE, PT_uint16);
	pcChannels->EndChange();
	pcChannels->Black();

	cCreator.Init(pcChannels);
	cCreator.AddAccess(CHANNEL_NAME_JACK, PT_sixbits);
	cCreator.AddAccess(CHANNEL_NAME_BOB, PT_sixbits);
	cCreator.AddAccess(CHANNEL_NAME_ALICE, PT_nybble);
	pcAccessor = cCreator.CreateAndKill();
	AssertString("CChannelsAccessorAccessBitty", pcAccessor->ClassName());

	//XX10110010100011
	aucData[0] = 0xa3;
	aucData[1] = 0x2c;
	pcAccessor->Set(0, aucData);
	AssertShortHex((uint16)0x8e38, *(uint16*)RemapSinglePointer(pcChannels->GetData(), 0 + 0));
	AssertShortHex((uint16)0xcb2c, *(uint16*)RemapSinglePointer(pcChannels->GetData(), 0 + 2));
	AssertShortHex((uint16)0x2222, *(uint16*)RemapSinglePointer(pcChannels->GetData(), 0 + 4));
	pucData = (uint8*)pcAccessor->Get(0);
	AssertShortHex((uint16)0x2ca3, *(uint16*)pucData);
	pcAccessor->Kill();

	pcChannels->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChannelsAccessorWorstCase(void)
{
	CChannels*					pcChannels;
	CChannelsAccessor*			pcAccessor;
	CChannelsAccessorCreator	cCreator;
	uint8*						pucData;
	uint8						aucData[5];

	pcChannels = UMalloc(CChannels);
	pcChannels->Init();
	pcChannels->BeginChange();
	pcChannels->SetSize(2);
	pcChannels->AddChannel(CHANNEL_NAME_JACK, PT_bit);
	pcChannels->AddChannel(CHANNEL_NAME_BOB, PT_uint32);
	pcChannels->AddChannel(CHANNEL_NAME_ALICE, PT_uint8);
	pcChannels->EndChange();
	pcChannels->Black();
	AssertSize(2, pcChannels->GetSize());
	AssertSize(11, pcChannels->GetByteSize());
	AssertSize(41, pcChannels->GetBitStride());
	AssertSize(CHANNEL_NON_ALIGNED_BYTES, pcChannels->GetByteStride());
	AssertBool(false, pcChannels->IsOnlyBasicTypes());

	memset(aucData, 0, 5);

	cCreator.Init(pcChannels);
	cCreator.AddAccess(CHANNEL_NAME_JACK, PT_uint32);
	cCreator.AddAccess(CHANNEL_NAME_BOB, PT_bit);
	cCreator.AddAccess(CHANNEL_NAME_ALICE, PT_nybble);
	pcAccessor = cCreator.CreateAndKill();
	AssertString("CChannelsAccessorWorstCase", pcAccessor->ClassName());
	AssertSize(-1, pcAccessor->GetByteSize());
	AssertSize(37, pcAccessor->GetBitSize());
	AssertSize(5, pcAccessor->GetBufferSize());
	aucData[0] = 0xff;
	aucData[1] = 0xff;
	aucData[2] = 0xff;
	aucData[3] = 0xff;
	aucData[4] = 0xff;
	pcAccessor->Set(0, aucData);
	AssertChar((uint8)0xff, pcChannels->GetData()[0]);
	AssertChar((uint8)0xff, pcChannels->GetData()[1]);
	AssertChar((uint8)0xff, pcChannels->GetData()[2]);
	AssertChar((uint8)0xff, pcChannels->GetData()[3]);
	AssertChar((uint8)0xff, pcChannels->GetData()[4]);
	AssertChar((uint8)0x01, pcChannels->GetData()[5]);  //01 because the next 'pixel' is all zeros.  The 1 is the high bit of the char in ALICE.
	pucData = (uint8*)pcAccessor->Get(0);
	AssertChar((uint8)0xff, pucData[0]);
	AssertChar((uint8)0xff, pucData[1]);
	AssertChar((uint8)0xff, pucData[2]);
	AssertChar((uint8)0xff, pucData[3]);
	AssertChar((uint8)0x1f, pucData[4]);

	aucData[0] = 0xff;
	aucData[1] = 0xff;
	aucData[2] = 0xff;
	aucData[3] = 0xff;
	aucData[4] = 0xfe;
	pcAccessor->Set(1, aucData);
	AssertChar((uint8)0x03, pcChannels->GetData()[5]); //01 from 'pixel' 0 and 02 from the bit in JACK in 'pixel' 1.
	AssertChar((uint8)0x00, pcChannels->GetData()[6]);
	AssertChar((uint8)0x00, pcChannels->GetData()[7]);
	AssertChar((uint8)0x00, pcChannels->GetData()[8]);
	AssertChar((uint8)0xfc, pcChannels->GetData()[9]);  //fc it the six low bits of the char
	AssertChar((uint8)0x03, pcChannels->GetData()[10] & 0x03);  //03 is the two high bits.
	pucData = (uint8*)pcAccessor->Get(1);
	AssertChar((uint8)0xff, pucData[0]);
	AssertChar((uint8)0xff, pucData[1]);
	AssertChar((uint8)0xff, pucData[2]);
	AssertChar((uint8)0xff, pucData[3]);
	AssertChar((uint8)0x1e, pucData[4]);
	
	pcAccessor->Kill();

	pcChannels->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChannelsAccessor(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	TypeConverterInit();
	DataIOInit();
	ObjectsInit();

	TestChannelsAccessorContiguous();
	TestChannelsAccessorByteAligned();
	TestChannelsAccessorTypeConvert();
	TestChannelsAccessorChannelBitty();
	TestChannelsAccessorAccessBitty();
	TestChannelsAccessorWorstCase();

	ObjectsKill();
	DataIOKill();
	TypeConverterKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}


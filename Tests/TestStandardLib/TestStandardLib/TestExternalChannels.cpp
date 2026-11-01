#include "BaseLib/GlobalMemory.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "StandardLib/Channels.h"
#include "StandardLib/ChannelsAccessorCreator.h"
#include "StandardLib/Unknowns.h"
#include "StandardLib/ChannelsCopier.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestExternalChannelsSubBytePositions(void)
{
	CChannelsAccessor*			pcChannel0;
	CChannelsAccessor*			pcChannel1;
	CChannelsAccessorCreator	cCreator;
	CChannels					cSourceChannels;
	uint8						uc[11] = {0x29, 0x33, 0xBE, 0x84, 0xE1, 0x6C, 0xD6, 0xAE, 0x52, 0xE8, 0x01};
	uint8*						pucData;
//   MSb                                                                                           LSb
// X 1 11101000 0 10100101 0 10111011 0 10110011 0 11001110 0 00110000 1 00101111 1 00011001 1 00101001
// ^         ^          ^          ^          ^          ^          ^          ^          ^          ^
// 8 8 77777777 7 76666666 6 66555555 5 55544444 4 44443333 3 33333222 2 22222211 1 11111110 0 00000000
// 1 0 98765432 1 09876543 2 10987654 3 21098765 4 32109876 5 43210987 6 54321098 7 65432109 8 76543210

	cSourceChannels.Init();
	cSourceChannels.BeginChange();
	cSourceChannels.AddChannel(0, PT_uint8);
	cSourceChannels.AddChannel(1, PT_bit);
	cSourceChannels.SetData((char*)uc);
	cSourceChannels.SetSize(9);
	cSourceChannels.EndChange();

	cCreator.Init(&cSourceChannels);
	cCreator.AddAccess((size)0, PT_uint8);
	pcChannel0 = cCreator.CreateAndKill();
	AssertString("CChannelsAccessorChannelBitty", pcChannel0->ClassName());

	cCreator.Init(&cSourceChannels);
	cCreator.AddAccess(1, PT_uint8);
	pcChannel1 = cCreator.CreateAndKill();
	AssertString("CChannelsAccessorChannelBitty", pcChannel1->ClassName());

	pucData = (uint8*)pcChannel0->Get(0);
	AssertInt(0x29, *pucData);

	
	pucData = (uint8*)pcChannel1->Get(0);
	AssertInt(0xFF, *pucData);  //LSb of the right hand 3 in 0x33 scaled up to a byte from a bit.

	pucData = (uint8*)pcChannel0->Get(1);
	AssertInt(0x19, *pucData);
	pucData = (uint8*)pcChannel1->Get(1);
	AssertInt(0xFF, *pucData);

	pucData = (uint8*)pcChannel0->Get(2);
	AssertInt(0x2F, *pucData);
	pucData = (uint8*)pcChannel1->Get(2);
	AssertInt(0xFF, *pucData);

	pucData = (uint8*)pcChannel0->Get(3);
	AssertInt(0x30, *pucData);
	
	pucData = (uint8*)pcChannel1->Get(3);
	AssertInt(0x00, *pucData);

	pucData = (uint8*)pcChannel0->Get(4);
	AssertInt(0xCE, *pucData);
	pucData = (uint8*)pcChannel1->Get(4);
	AssertInt(0x00, *pucData);

	pucData = (uint8*)pcChannel0->Get(5);
	AssertInt(0xB3, *pucData);
	pucData = (uint8*)pcChannel1->Get(5);
	AssertInt(0x00, *pucData);

	pucData = (uint8*)pcChannel0->Get(6);
	AssertInt(0xBB, *pucData);
	pucData = (uint8*)pcChannel1->Get(6);
	AssertInt(0x00, *pucData);

	pucData = (uint8*)pcChannel0->Get(7);
	AssertInt(0xA5, *pucData);
	pucData = (uint8*)pcChannel1->Get(7);
	AssertInt(0x00, *pucData);

	pucData = (uint8*)pcChannel0->Get(8);
	AssertInt(0xE8, *pucData);
	pucData = (uint8*)pcChannel1->Get(8);
	AssertInt(0xFF, *pucData);

	cSourceChannels.Kill();
	pcChannel0->Kill();
	pcChannel1->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestContiguousExternalChannels(void)
{
	CChannels			cChannels;
	CChannels			cExternalChannel;
	int32				vSourse;
	int32				iExpected0;
	int32				iExpected1;
	int32				iExpected2;
	int32				iExpected3;

	//Rows      3 2 1 0
	vSourse = 0xFF00aa55;

	cExternalChannel.Init();
	cExternalChannel.BeginChange();
	cExternalChannel.AddChannel(3, PT_nybble);
	cExternalChannel.AddChannel(2, PT_bit);
	cExternalChannel.AddChannel(1, PT_bit);
	cExternalChannel.AddChannel(0, PT_crumb);
	cExternalChannel.SetData((char*)&vSourse);
	cExternalChannel.SetSize(4);
	cExternalChannel.EndChange();
	AssertSize(4, cExternalChannel.GetSize());

	cChannels.Init();
	cChannels.BeginChange();
	cChannels.AddChannel(3, PT_uint8);
	cChannels.AddChannel(2, PT_uint8);
	cChannels.AddChannel(1, PT_uint8);
	cChannels.AddChannel(0, PT_uint8);
	cChannels.SetSize(4);
	cChannels.EndChange();
	AssertSize(4, cChannels.GetSize());

	CChannelsCopier cCopy;

	cCopy.Init(&cExternalChannel, &cChannels);

	cCopy.Copy(0, 0, 4);

	iExpected0 = 0x5500ff55;
	iExpected1 = 0xaaff00aa;
	iExpected2 = 0x00000000;
	iExpected3 = 0xFFffffff;
	AssertIntHex((uint32)iExpected0, ((uint32*)cChannels.GetData())[0]);
	AssertIntHex((uint32)iExpected1, ((uint32*)cChannels.GetData())[1]);
	AssertIntHex((uint32)iExpected2, ((uint32*)cChannels.GetData())[2]);
	AssertIntHex((uint32)iExpected3, ((uint32*)cChannels.GetData())[3]);

	cCopy.Kill();
	cChannels.Kill();
	cExternalChannel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestExternalChannels(void)
{
	BeginTests();

	MemoryInit();
	TypesInit();
	TypeConverterInit();
	DataIOInit();
	ObjectsInit();

	TestExternalChannelsSubBytePositions();
	TestContiguousExternalChannels();

	ObjectsKill();
	DataIOKill();
	TypesKill();
	TypeConverterKill();
	MemoryKill();

	TestStatistics();
}


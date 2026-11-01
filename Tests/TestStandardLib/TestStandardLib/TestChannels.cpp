#include "BaseLib/GlobalMemory.h"
#include "BaseLib/Logger.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Unknowns.h"
#include "StandardLib/Channels.h"
#include "StandardLib/ChannelsCopier.h"
#include "StandardLib/ChannelsAccessorCreator.h"
#include "StandardLib/ExternalObjectSerialiser.h"
#include "StandardLib/MultiFileObjectWriter.h"
#include "StandardLib/ExternalObjectDeserialiser.h"
#include "StandardLib/FileObjectReader.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChannelsStuff(void)
{
	ObjectsInit();

	CChannels					cChannels;
	CChannelsAccessor*			pcAccessor;
	CChannelsAccessor*			pcAccessor2;
	CChannelsAccessor*			pcAccessor3;
	char						c;

	cChannels.Init();

	cChannels.BeginChange();
	cChannels.SetSize(4);
	cChannels.AddChannel(0, PT_uint8);
	cChannels.EndChange();
	AssertSize(4*1, cChannels.GetByteSize());

	pcAccessor = CChannelsAccessorCreator::CreateSingleChannelAccessor(&cChannels, 0, PT_uint8);
	pcAccessor->Set(0, &(c = 'A'));
	pcAccessor->Set(1, &(c = 'a'));
	pcAccessor->Set(2, &(c = 'A'));
	pcAccessor->Set(3, &(c = 0));
	AssertString("AaA", (char*)cChannels.GetData());
	pcAccessor->Kill();

	cChannels.BeginChange();
	cChannels.AddChannel(1, 2, PT_uint8);
	cChannels.EndChange();
	AssertSize(4*3, cChannels.GetByteSize());

	pcAccessor = CChannelsAccessorCreator::CreateSingleChannelAccessor(&cChannels, 1, PT_uint8);
	pcAccessor->Set(0, &(c = 'B'));
	pcAccessor->Set(1, &(c = 'b'));
	pcAccessor->Set(2, &(c = 'B'));
	pcAccessor->Set(3, &(c = 'X'));
	pcAccessor->Kill();

	pcAccessor = CChannelsAccessorCreator::CreateSingleChannelAccessor(&cChannels, 2, PT_uint8);
	pcAccessor->Set(0, &(c = 'C'));
	pcAccessor->Set(1, &(c = 'c'));
	pcAccessor->Set(2, &(c = 'C'));
	pcAccessor->Set(3, &(c = 'Z'));
	pcAccessor->Kill();

	AssertString("ABCabcABC", (char*)cChannels.GetData());

	cChannels.BeginChange();
	cChannels.AddChannel(3, PT_uint8);
	cChannels.EndChange();
	AssertSize(4*4, cChannels.GetByteSize());

	pcAccessor = CChannelsAccessorCreator::CreateSingleChannelAccessor(&cChannels, 0, PT_uint8);

	pcAccessor->Set(3, &(c = 'V'));
	pcAccessor->Kill();

	pcAccessor = CChannelsAccessorCreator::CreateSingleChannelAccessor(&cChannels, 3, PT_uint8);
	pcAccessor->Set(0, &(c = 'D'));
	pcAccessor->Set(1, &(c = 'd'));
	pcAccessor->Set(2, &(c = 'D'));
	pcAccessor->Set(3, &(c = (char)0));
	pcAccessor->Kill();

	AssertString("ABCDabcdABCDVXZ", (char*)cChannels.GetData());

	cChannels.Kill();

	char*	pvUserData;

	pvUserData = (char*)malloc(4*3 + 1);
	pvUserData[0] = 'Q'; pvUserData[1] =  'A'; pvUserData[2] =  'Z';
	pvUserData[3] = 'W'; pvUserData[4] =  'S'; pvUserData[5] =  'X';
	pvUserData[6] = 'E'; pvUserData[7] =  'D'; pvUserData[8] =  'C';
	pvUserData[9] = 'R'; pvUserData[10] = 'F'; pvUserData[11] = 'V';
	pvUserData[12] = 0;

	cChannels.Init();
	cChannels.BeginChange();
	cChannels.AddChannel(100, 110, 120, PT_uint8);
	cChannels.SetSize(4);
	cChannels.SetData(pvUserData);
	cChannels.EndChange();

	pcAccessor = CChannelsAccessorCreator::CreateSingleChannelAccessor(&cChannels, 110, PT_uint8);
	pcAccessor2 = CChannelsAccessorCreator::CreateSingleChannelAccessor(&cChannels, 120, PT_uint8);
	pcAccessor3 = CChannelsAccessorCreator::CreateSingleChannelAccessor(&cChannels, 100, PT_uint8);
	AssertNotNull(pcAccessor);
	AssertNotNull(pcAccessor2);
	AssertSize(4, cChannels.GetSize());
	AssertSize(4*3, cChannels.GetByteSize());
	AssertChar('S', *((char*)pcAccessor->Get(1)));
	AssertChar('V', *((char*)pcAccessor2->Get(3)));
	AssertChar('R', *((char*)pcAccessor3->Get(3)));
	pcAccessor->Kill();
	pcAccessor2->Kill();
	pcAccessor3->Kill();

	cChannels.BeginChange();
	cChannels.SetData(NULL);
	cChannels.EndChange();

	cChannels.Kill();
	free(pvUserData);

	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChannelsRemoveBeforeSize(void)
{
	ObjectsInit();

	CChannels					cChannels;
	CChannelsAccessor*			pcAccessor;
	char						c;

	cChannels.Init();

	cChannels.BeginChange();
	cChannels.AddChannel(0, 1, 2, PT_uint8);
	cChannels.EndChange();
	AssertSize(3, cChannels.GetNumChannels());
	AssertSize(24, cChannels.GetBitStride());

	cChannels.BeginChange();
	AssertTrue(cChannels.RemoveChannel(1));
	cChannels.EndChange();
	AssertSize(2, cChannels.GetNumChannels());
	AssertTrue(cChannels.HasChannel(0));
	AssertFalse(cChannels.HasChannel(1));
	AssertTrue(cChannels.HasChannel(2));
	AssertSize(16, cChannels.GetBitStride());

	cChannels.BeginChange();
	cChannels.SetSize(3);
	cChannels.EndChange();
	AssertSize(3*2, cChannels.GetByteSize());

	pcAccessor = CChannelsAccessorCreator::CreateSingleChannelAccessor(&cChannels, 0, PT_uint8);
	pcAccessor->Set(0, &(c = 'A'));
	pcAccessor->Set(1, &(c = 'a'));
	pcAccessor->Set(2, &(c = 'A'));
	pcAccessor->Kill();

	pcAccessor = CChannelsAccessorCreator::CreateSingleChannelAccessor(&cChannels, 2, PT_uint8);
	pcAccessor->Set(0, &(c = 'C'));
	pcAccessor->Set(1, &(c = 'c'));
	pcAccessor->Set(2, &(c = 'C'));
	pcAccessor->Kill();

	AssertMemory("ACacAC", cChannels.GetData(), 6);

	cChannels.Kill();

	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChannelsAddDuplicate(void)
{
	ObjectsInit();

	CChannels	cChannels;
	SLogConfig	sLogConfig;

	cChannels.Init();
	cChannels.BeginChange();
	cChannels.SetSize(2);
	cChannels.AddChannel(0, PT_uint8);
	cChannels.EndChange();
	AssertSize(2, cChannels.GetByteSize());

	//A change that adds a channel which already exists adds none of its channels.
	cChannels.BeginChange();
	cChannels.AddChannel(1, PT_uint8);
	cChannels.AddChannel(0, PT_uint8);
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cChannels.EndChange());
	gcLogger.SetConfig(&sLogConfig);
	AssertSize(1, cChannels.GetNumChannels());
	AssertFalse(cChannels.HasChannel(1));
	AssertSize(8, cChannels.GetBitStride());
	AssertSize(2, cChannels.GetByteSize());

	//Nor does one that adds the same channel twice.
	cChannels.BeginChange();
	cChannels.AddChannel(2, PT_uint8);
	cChannels.AddChannel(2, PT_uint8);
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cChannels.EndChange());
	gcLogger.SetConfig(&sLogConfig);
	AssertSize(1, cChannels.GetNumChannels());
	AssertSize(2, cChannels.GetByteSize());

	//Removing a channel and adding it back in one change is allowed.
	cChannels.BeginChange();
	AssertTrue(cChannels.RemoveChannel(0));
	cChannels.AddChannel(0, PT_uint16);
	AssertTrue(cChannels.EndChange());
	AssertSize(1, cChannels.GetNumChannels());
	AssertSize(16, cChannels.GetBitStride());

	cChannels.BeginChange();
	cChannels.AddChannel(1, PT_uint8);
	AssertTrue(cChannels.EndChange());
	AssertSize(2, cChannels.GetNumChannels());
	AssertSize(2 * 3, cChannels.GetByteSize());

	cChannels.Kill();

	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChannelsLoad(void)
{
	ObjectsInit();

	{
		Ptr<CChannels>				pcChannels;
		CChannelsAccessor*			pcAccessorR;
		CChannelsAccessor*			pcAccessorG;
		CChannelsAccessor*			pcAccessorB;
		int							r;
		int							g;
		int							b;
		int							iCount;
		int							x;
		int							y;
		CExternalObjectSerialiser	cSerialiser;
		CMultiFileObjectWriter		cWriter;
		CFileUtil					cFileUtil;

		pcChannels = ONMalloc<CChannels>("File");
		pcChannels.GetIndex();


		pcChannels->BeginChange();
		pcChannels->SetSize(8 * 32);
		pcChannels->AddChannel(0, PT_uint8);
		pcChannels->AddChannel(1, PT_uint8);
		pcChannels->AddChannel(2, PT_uint8);
		pcChannels->EndChange();
		AssertSize(8 * 32 * 3, pcChannels->GetByteSize());

		pcAccessorR = CChannelsAccessorCreator::CreateSingleChannelAccessor(&pcChannels, 0);
		pcAccessorG = CChannelsAccessorCreator::CreateSingleChannelAccessor(&pcChannels, 1);
		pcAccessorB = CChannelsAccessorCreator::CreateSingleChannelAccessor(&pcChannels, 2);

		x = 0;
		y = 0;
		iCount = 0;
		for (r = 0; r < 8; r++)
		{
			for (g = 0; g < 8; g++)
			{
				for (b = 0; b < 4; b++)
				{
					pcAccessorR->Set(x + (lsize)y * 8, &r);
					pcAccessorG->Set(x + (lsize)y * 8, &g);
					pcAccessorB->Set(x + (lsize)y * 8, &b);

					x++;
					if (x >= 8)
					{
						x = 0;
						y++;
					}

					iCount++;
				}
			}
		}

		AssertInt(256, iCount);

		pcAccessorR->Kill();
		pcAccessorG->Kill();
		pcAccessorB->Kill();

		AssertTrue(cFileUtil.RemoveDir("Output" _FS_ "Channels"));
		AssertTrue(cFileUtil.TouchDir("Output" _FS_ "Channels"));

		cWriter.Init("Output" _FS_ "Channels" _FS_, "");
		cSerialiser.Init(&cWriter);
		AssertTrue(cSerialiser.Write(&pcChannels));
		cSerialiser.Kill();
		cWriter.Kill();

		pcChannels = NULL;
	}

	ObjectsFlush();
	ObjectsKill();

	ObjectsInit();

	{
		Ptr<CChannels>				pcChannels;
		CChannelsAccessor*			pcAccessorR;
		CChannelsAccessor*			pcAccessorG;
		CChannelsAccessor*			pcAccessorB;
		int							r;
		int							g;
		int							b;
		int							iCount;
		int							x;
		int							y;
		CMultiFileObjectWriter		cWriter;
		CChannel*					pcChannel;

		gcObjects.AddConstructor<CChannels>();

		CFileObjectReader				cReader;
		CExternalObjectDeserialiser		cDeserialiser;
		CFileBasic						cFileBasic;
		CDiskFile*						pcDiskFile;
		uint8							ar;
		uint8							ag;
		uint8							ab;

		pcDiskFile = DiskFile("Output" _FS_ "Channels" _FS_ "File.DRG");
		cFileBasic.Init(pcDiskFile);

		cReader.Init(&cFileBasic);
		cDeserialiser.Init(&cReader, false, &gcObjects);
		pcChannels = cDeserialiser.Read("File");
		cDeserialiser.Kill();
		cReader.Kill();

		cFileBasic.Kill();


		AssertSize(8 * 32 * 3, pcChannels->GetByteSize());
		AssertSize(3, pcChannels->GetByteStride());
		AssertSize(3 * 8, pcChannels->GetBitStride());
		AssertSize(3, pcChannels->GetNumChannels());

		pcChannel = pcChannels->GetChannel(0);
		AssertSize(1, pcChannel->miByteSize);
		AssertSize(0, pcChannel->miByteOffset);
		AssertSize(8, pcChannel->miBitSize);
		AssertSize(0, pcChannel->miBitOffset);
		AssertSize(PT_uint8, pcChannel->eType);
		AssertSize(0, pcChannel->iChannel);
		AssertBool(false, pcChannel->bReverse);

		pcChannel = pcChannels->GetChannel(1);
		AssertSize(1, pcChannel->miByteSize);
		AssertSize(1, pcChannel->miByteOffset);
		AssertSize(8, pcChannel->miBitSize);
		AssertSize(8, pcChannel->miBitOffset);
		AssertSize(PT_uint8, pcChannel->eType);
		AssertSize(1, pcChannel->iChannel);
		AssertBool(false, pcChannel->bReverse);

		pcChannel = pcChannels->GetChannel(2);
		AssertSize(1, pcChannel->miByteSize);
		AssertSize(2, pcChannel->miByteOffset);
		AssertSize(8, pcChannel->miBitSize);
		AssertSize(16, pcChannel->miBitOffset);
		AssertSize(PT_uint8, pcChannel->eType);
		AssertSize(2, pcChannel->iChannel);
		AssertBool(false, pcChannel->bReverse);

		AssertBool(true, pcChannels->IsOnlyBasicTypes());

		pcAccessorR = CChannelsAccessorCreator::CreateSingleChannelAccessor(&pcChannels, 0);
		pcAccessorG = CChannelsAccessorCreator::CreateSingleChannelAccessor(&pcChannels, 1);
		pcAccessorB = CChannelsAccessorCreator::CreateSingleChannelAccessor(&pcChannels, 2);

		iCount = 0;

		x = 0;
		y = 0;
		iCount = 0;
		for (r = 0; r < 8; r++)
		{
			for (g = 0; g < 8; g++)
			{
				for (b = 0; b < 4; b++)
				{
					ar = *((uint8*)pcAccessorR->Get(x + (lsize)y * 8));
					ag = *((uint8*)pcAccessorG->Get(x + (lsize)y * 8));
					ab = *((uint8*)pcAccessorB->Get(x + (lsize)y * 8));

					if ((ar != (uint8)r) ||
						(ag != (uint8)g) ||
						(ab != (uint8)b))
					{
						AssertChar(r, (uint8)ar);
						AssertChar(g, (uint8)ag);
						AssertChar(b, (uint8)ab);
					}

					x++;
					if (x >= 8)
					{
						x = 0;
						y++;
					}

					iCount++;
				}
			}
		}
		AssertInt(256, iCount);

		pcAccessorR->Kill();
		pcAccessorG->Kill();
		pcAccessorB->Kill();

		pcChannels = NULL;
	}

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChannelsCopier16bitTo8bit(void)
{
	ObjectsInit();

	Ptr<CChannels>		pcSource;
	CChannelsAccessor*	pcAccessor;
	uint16				ui16;
	uint16*				pui16;
	Ptr<CChannels>		pcDest;
	uint8*				pui8;

	pcSource = ONMalloc<CChannels>("Source");

	pcSource->BeginChange();
	pcSource->SetSize(4);
	pcSource->AddChannel(0, PT_uint16);
	pcSource->EndChange();
	AssertSize(4 * 2, pcSource->GetByteSize());

	pcAccessor = CChannelsAccessorCreator::CreateSingleChannelAccessor(&pcSource, 0);
	ui16 = 0xfeff; pcAccessor->Set(0, &ui16);
	ui16 = 0xcab3; pcAccessor->Set(1, &ui16);
	ui16 = 0x0000; pcAccessor->Set(2, &ui16);
	ui16 = 0x2100; pcAccessor->Set(3, &ui16);
	pcAccessor->Kill();

	pui16 = (uint16*)pcSource->GetData();
	AssertShortHex((int16)0xfeff, pui16[0]);
	AssertShortHex((int16)0xcab3, pui16[1]);
	AssertShortHex((uint16)0x0000, pui16[2]);
	AssertShortHex((uint16)0x2100, pui16[3]);

	pcDest = ONMalloc<CChannels>("Dest");

	pcDest->BeginChange();
	pcDest->SetSize(4);
	pcDest->AddChannel(0, PT_uint8);
	pcDest->EndChange();
	AssertSize(4 * 1, pcDest->GetByteSize());

	CChannelsCopier	cCopier;

	cCopier.Init(&pcSource, &pcDest);
	cCopier.Copy(0, 0, 4);
	cCopier.Kill();

	pui8 = (uint8*)pcDest->GetData();
	AssertChar((int8)0xfe, pui8[0]);
	AssertChar((int8)0xca, pui8[1]);
	AssertChar(0x00, pui8[2]);
	AssertChar(0x21, pui8[3]);

	pcSource = NULL;
	pcDest = NULL;

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChannelsCopier24bitTo8bit(void)
{
	ObjectsInit();

	Ptr<CChannels>		pcSource;
	CChannelsAccessor*	pcAccessor0;
	CChannelsAccessor*	pcAccessor1;
	CChannelsAccessor*	pcAccessor2;
	uint8				ui8;
	uint8*				pui8;
	Ptr<CChannels>		pcDest;

	pcSource = ONMalloc<CChannels>("Source");

	pcSource->BeginChange();
	pcSource->SetSize(4);
	pcSource->AddChannel(0, PT_uint8);
	pcSource->AddChannel(1, PT_uint8);
	pcSource->AddChannel(2, PT_uint8);
	pcSource->EndChange();
	AssertSize(4 * 3, pcSource->GetByteSize());

	pcAccessor0 = CChannelsAccessorCreator::CreateSingleChannelAccessor(&pcSource, 0);
	ui8 = 0xfe; pcAccessor0->Set(0, &ui8);
	ui8 = 0xca; pcAccessor0->Set(1, &ui8);
	ui8 = 0x00; pcAccessor0->Set(2, &ui8);
	ui8 = 0x21; pcAccessor0->Set(3, &ui8);
	pcAccessor0->Kill();

	pcAccessor1 = CChannelsAccessorCreator::CreateSingleChannelAccessor(&pcSource, 1);
	ui8 = 0xff; pcAccessor1->Set(0, &ui8);
	ui8 = 0x80; pcAccessor1->Set(1, &ui8);
	ui8 = 0x7f; pcAccessor1->Set(2, &ui8);
	ui8 = 0x40; pcAccessor1->Set(3, &ui8);
	pcAccessor1->Kill();

	pcAccessor2 = CChannelsAccessorCreator::CreateSingleChannelAccessor(&pcSource, 2);
	ui8 = 0x03; pcAccessor2->Set(0, &ui8);
	ui8 = 0x02; pcAccessor2->Set(1, &ui8);
	ui8 = 0x01; pcAccessor2->Set(2, &ui8);
	ui8 = 0x20; pcAccessor2->Set(3, &ui8);
	pcAccessor2->Kill();

	pcDest = ONMalloc<CChannels>("Dest");

	pcDest->BeginChange();
	pcDest->SetSize(4);
	pcDest->AddChannel(2, PT_crumb);
	pcDest->AddChannel(1, PT_tribble);
	pcDest->AddChannel(0, PT_tribble);
	pcDest->EndChange();
	AssertSize(4 * 1, pcDest->GetByteSize());

	CChannelsCopier	cCopier;

	cCopier.Init(&pcSource, &pcDest);
	cCopier.Copy(0, 0, 4);
	cCopier.Kill();

	pui8 = (uint8*)pcDest->GetData();
	AssertChar((int8)0xfc, pui8[0]);
	AssertChar((int8)0xac, pui8[1]);
	AssertChar(0x0c, pui8[2]);
	AssertChar(0x04, pui8[3]);

	pcSource = NULL;
	pcDest = NULL;

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChannels(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	TypeConverterInit();
	DataIOInit();

	TestChannelsStuff();
	TestChannelsRemoveBeforeSize();
	TestChannelsAddDuplicate();
	TestChannelsLoad();
	TestChannelsCopier16bitTo8bit();
	TestChannelsCopier24bitTo8bit();

	DataIOKill();
	TypeConverterKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}


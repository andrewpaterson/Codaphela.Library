#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/BufferedFile.h"
#include "BaseLib/ArrayVoidPtr.h"
#include "BaseLib/ArrayBit.h"
#include "StandardLib/Unknowns.h"
#include "StandardLib/MapUnknownUnknown.h"
#include "TestLib/Assert.h"
#include "UnknownTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapUnknownUnknownAdd(void)
{
	MemoryInit();
	UnknownsInit();

	CMapUnknownUnknown		cMap;
	CTestNamedUnknown*		pcKey1;
	CTestUnknownJobbie*		pcValue1;
	CTestNamedUnknown*		pcKey2;
	CTestUnknownJobbie*		pcValue2;

	gcUnknowns.AddConstructor<CTestUnknown>();
	gcUnknowns.AddConstructor<CTestNamedUnknown>();
	gcUnknowns.AddConstructor<CTestUnknownJobbie>();
	gcUnknowns.AddConstructor<CTestIterableUnknown>();
	AssertSize(0, gcUnknowns.NumElements());

	cMap.Init();

	pcKey1 = gcUnknowns.Add<CTestNamedUnknown>();
	pcValue1 = gcUnknowns.Add<CTestUnknownJobbie>();
	pcKey2 = gcUnknowns.Add<CTestNamedUnknown>();
	pcValue2 = gcUnknowns.Add<CTestUnknownJobbie>();
	pcKey1->Init("Hello");
	pcValue1->Init(4, "Snurk");
	pcKey2->Init("World");
	pcValue2->Init(67, "krust");
	AssertSize(0, cMap.NumElements());

	cMap.Put(pcKey1, pcValue1);
	AssertSize(1, cMap.NumElements());

	cMap.Put(pcKey2, pcValue2);
	AssertSize(2, cMap.NumElements());

	cMap.Kill();

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapUnknownUnknownIterate(void)
{
	MemoryInit();
	UnknownsInit();

	CMapUnknownUnknown		cMap;
	CTestNamedUnknown*		pcKeyA;
	CTestNamedUnknown*		pcKeyB;
	CTestNamedUnknown*		pcKeyC;
	CTestNamedUnknown*		pcKey1;
	CTestUnknownJobbie*		pcValue1;
	CTestNamedUnknown*		pcKey2;
	CTestUnknownJobbie*		pcValue2;
	CTestNamedUnknown*		pcKey3;
	CTestUnknownJobbie*		pcValue3;
	SMapIterator			sIter;
	CUnknown*				pcKey;
	CUnknown*				pcValue;
	bool					bExists;
	CArrayVoidPtr			aPtrs;

	gcUnknowns.AddConstructor<CTestUnknown>();
	gcUnknowns.AddConstructor<CTestNamedUnknown>();
	gcUnknowns.AddConstructor<CTestUnknownJobbie>();
	gcUnknowns.AddConstructor<CTestIterableUnknown>();
	AssertSize(0, gcUnknowns.NumElements());

	cMap.Init();

	pcKey1 = NULL;
	pcKey2 = NULL;
	pcKey3 = NULL;

	pcKeyA = gcUnknowns.Add<CTestNamedUnknown>();
	pcKeyB = gcUnknowns.Add<CTestNamedUnknown>();
	pcKeyC = gcUnknowns.Add<CTestNamedUnknown>();

	aPtrs.Init();
	aPtrs.Add(pcKeyA);
	aPtrs.Add(pcKeyB);
	aPtrs.Add(pcKeyC);
	aPtrs.TimSort();
	pcKey1 = (CTestNamedUnknown*)aPtrs.GetPtr(0);
	pcKey2 = (CTestNamedUnknown*)aPtrs.GetPtr(1);
	pcKey3 = (CTestNamedUnknown*)aPtrs.GetPtr(2);
	aPtrs.Kill();

	pcKeyA = NULL;
	pcKeyB = NULL;
	pcKeyC = NULL;

	pcValue1 = gcUnknowns.Add<CTestUnknownJobbie>();
	pcValue2 = gcUnknowns.Add<CTestUnknownJobbie>();
	pcValue3 = gcUnknowns.Add<CTestUnknownJobbie>();
	pcKey1->Init("Hello");
	pcValue1->Init(4, "Snurk");
	pcKey2->Init("World");
	pcValue2->Init(67, "krust");
	pcKey3->Init("Aardvark");
	pcValue3->Init(67, "Berry");
	AssertSize(0, cMap.NumElements());

	cMap.Put(pcKey1, pcValue1);
	cMap.Put(pcKey2, pcValue2);
	cMap.Put(pcKey3, pcValue3);
	AssertSize(3, cMap.NumElements());

	bExists = cMap.StartIteration(&sIter, &pcKey, &pcValue);
	AssertTrue(bExists);
	AssertPointer(pcKey1, pcKey);
	AssertPointer(pcValue1, pcValue);
	bExists = cMap.Iterate(&sIter, &pcKey, &pcValue);
	AssertTrue(bExists);
	AssertPointer(pcKey2, pcKey);
	AssertPointer(pcValue2, pcValue);
	bExists = cMap.Iterate(&sIter, &pcKey, &pcValue);
	AssertTrue(bExists);
	AssertPointer(pcKey3, pcKey);
	AssertPointer(pcValue3, pcValue);
	bExists = cMap.Iterate(&sIter, &pcKey, &pcValue);
	AssertFalse(bExists);
	AssertSize(3, cMap.GetHoldingSize());
	AssertSize(0, cMap.GetSortedSize());

	cMap.Pack();
	AssertSize(3, cMap.NumElements());
	AssertSize(3, cMap.GetSortedSize());
	AssertSize(0, cMap.GetHoldingSize());

	bExists = cMap.StartIteration(&sIter, &pcKey, &pcValue);
	AssertTrue(bExists);
	AssertPointer(pcKey1, pcKey);
	AssertPointer(pcValue1, pcValue);
	bExists = cMap.Iterate(&sIter, &pcKey, &pcValue);
	AssertTrue(bExists);
	AssertPointer(pcKey2, pcKey);
	AssertPointer(pcValue2, pcValue);
	bExists = cMap.Iterate(&sIter, &pcKey, &pcValue);
	AssertTrue(bExists);
	AssertPointer(pcKey3, pcKey);
	AssertPointer(pcValue3, pcValue);
	bExists = cMap.Iterate(&sIter, &pcKey, &pcValue);
	AssertFalse(bExists);

	cMap.Kill();

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapUnknownUnknownGet(void)
{
	MemoryInit();
	UnknownsInit();

	CMapUnknownUnknown		cMap;
	CTestNamedUnknown*		pcKey;
	CTestUnknownJobbie*		pcValue;
	CArrayVoidPtr			aPtrs;
	size					ui;
	CUnknown*				pcTestKey;
	CUnknown*				pcTestValue;
	bool					bValid;

	gcUnknowns.AddConstructor<CTestUnknown>();
	gcUnknowns.AddConstructor<CTestNamedUnknown>();
	gcUnknowns.AddConstructor<CTestUnknownJobbie>();
	gcUnknowns.AddConstructor<CTestIterableUnknown>();
	AssertSize(0, gcUnknowns.NumElements());

	aPtrs.Init();
	cMap.Init();

	for (ui = 0; ui < 12000; ui++)
	{
		pcKey = gcUnknowns.Add<CTestNamedUnknown>();
		pcValue = gcUnknowns.Add<CTestUnknownJobbie>();
		pcKey->Init(SizeToString(ui));
		pcValue->Init(((int32)ui + 1) * 7, SizeToString(ui));
		aPtrs.Add(pcKey);
		cMap.Put(pcKey, pcValue);
	}
	AssertSize(12000, cMap.NumElements());
	AssertSize(11264, cMap.GetSortedSize());
	AssertSize(736, cMap.GetHoldingSize());

	bValid = false;
	for (ui = 0; ui < 12000; ui++)
	{
		pcTestKey = (CUnknown*)aPtrs.GetPtr(ui);
		pcTestValue = cMap.Get(pcTestKey);

		bValid = StringCompare("CTestNamedUnknown", pcTestKey->ClassName()) == 0;
		bValid &= StringCompare("CTestUnknownJobbie", pcTestValue->ClassName()) == 0;
		
		pcKey = (CTestNamedUnknown*)pcTestKey;
		pcValue = (CTestUnknownJobbie*)pcTestValue;
		bValid &= StringCompare(SizeToString(ui), pcKey->mszName.Text()) == 0;
		bValid &= ((int32)ui + 1) * 7 == pcValue->miANumber;
		bValid &= StringCompare(SizeToString(ui), pcValue->mszText.Text()) == 0;

		if (!bValid)
		{
			AssertTrue(bValid);
			break;
		}
	}
	AssertTrue(bValid);

	aPtrs.TimSort();
	aPtrs.Kill();


	cMap.Pack();
	AssertSize(12000, cMap.NumElements());
	AssertSize(12000, cMap.GetSortedSize());
	AssertSize(0, cMap.GetHoldingSize());

	cMap.Kill();

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapUnknownUnknownLoad(void)
{
	MemoryInit();
	UnknownsInit();

	CMapUnknownUnknown		cMap;
	CFileUtil				cFileUtil;
	CTestNamedUnknown*		pcKey;
	CTestUnknownJobbie*		pcValue;
	size					ui;
	CUnknown*				pcTestKey;
	CUnknown*				pcTestValue;
	CChunkFile				cFile;
	bool					bResult;
	SMapIterator			sIter;
	bool					bExists;
	bool					bValid;
	CArrayBit				aBits;
	CArrayVoidPtr			aPtrs;

	gcUnknowns.AddConstructor<CTestUnknown>();
	gcUnknowns.AddConstructor<CTestNamedUnknown>();
	gcUnknowns.AddConstructor<CTestUnknownJobbie>();
	gcUnknowns.AddConstructor<CTestIterableUnknown>();
	AssertSize(0, gcUnknowns.NumElements());

	cFileUtil.RemoveDir("Output" _FS_ "MapUnknownUnknown");
	cFileUtil.MakeDir("Output" _FS_ "MapUnknownUnknown");

	cMap.Init();

	for (ui = 0; ui < 12000; ui++)
	{
		pcKey = gcUnknowns.Add<CTestNamedUnknown>();
		pcValue = gcUnknowns.Add<CTestUnknownJobbie>();
		pcKey->Init(SizeToString(ui));
		pcValue->Init(((int32)ui + 1) * 7, SizeToString(ui));
		cMap.Put(pcKey, pcValue);
	}
	AssertSize(12000, cMap.NumElements());
	AssertSize(24000, gcUnknowns.NumElements());

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "MapUnknownUnknown" _FS_ "Map.dat")));
	bResult = cFile.WriteOpen();
	AssertTrue(bResult);

	bResult = cMap.Save(&cFile);
	AssertTrue(bResult);

	bResult = cFile.WriteClose();
	AssertTrue(bResult);

	cMap.Sort();

	cFile.Kill();
	cMap.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "MapUnknownUnknown" _FS_ "Map.dat")));
	bResult = cFile.ReadOpen();
	AssertTrue(bResult);

	bResult = cMap.Load(&cFile);
	AssertTrue(bResult);

	bResult = cFile.ReadClose();
	AssertTrue(bResult);
	cFile.Kill();
	AssertSize(12000, cMap.NumElements());
	AssertSize(24000, gcUnknowns.NumElements());

	aPtrs.Init();
	aBits.Init();
	aBits.GrowTo(12000);
	aBits.Zero();
	ui = 0;
	bValid = false;
	bExists = cMap.StartIteration(&sIter, &pcTestKey, &pcTestValue);
	while (bExists)
	{
		bValid = StringCompare("CTestNamedUnknown", pcTestKey->ClassName()) == 0;
		bValid &= StringCompare("CTestUnknownJobbie", pcTestValue->ClassName()) == 0;

		pcKey = (CTestNamedUnknown*)pcTestKey;
		pcValue = (CTestUnknownJobbie*)pcTestValue;
		if (!bValid)
		{
			AssertTrue(bValid);
			break;
		}

		aPtrs.Add(pcKey);
		aBits.Set(pcValue->miANumber / 7 - 1, true);
		bExists = cMap.Iterate(&sIter, &pcTestKey, &pcTestValue);
		ui++;
	}
	AssertTrue(bValid);
	AssertSize(12000, ui);
	aPtrs.TimSort();

	bValid = true;
	for (ui = 0; ui < 12000; ui++)
	{
		bValid &= aBits.Get(ui);
		if (!bValid)
		{
			AssertTrue(bValid);
			break;
		}
	}
	AssertTrue(bValid);

	bValid = false;
	for (ui = 0; ui < 12000; ui++)
	{
		pcKey = (CTestNamedUnknown*)aPtrs.GetPtr(ui);
		pcValue = (CTestUnknownJobbie*)cMap.Get(pcKey);

		if (pcValue)
		{
			bValid = StringCompare(pcKey->mszName.Text(), pcValue->mszText.Text()) == 0;
			if (!bValid)
			{
				AssertTrue(bValid);
				break;
			}
		}
		else
		{
			AssertNotNull(pcValue);
			break;
		}
	}

	aBits.Kill();
	aPtrs.Kill();

	cMap.Kill();

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
void TestMapUnknownUnknownLoadNull(void)
{
	MemoryInit();
	UnknownsInit();

	CMapUnknownUnknown		cMap;
	CFileUtil				cFileUtil;
	CTestNamedUnknown*		pcKey;
	CTestUnknownJobbie*		pcValue;
	CUnknown*				pcTestKey;
	CUnknown*				pcTestValue;
	CChunkFile				cFile;
	bool					bResult;
	SMapIterator			sIter;
	bool					bExists;
	size					uiNulls;
	size					uiValues;

	gcUnknowns.AddConstructor<CTestNamedUnknown>();
	gcUnknowns.AddConstructor<CTestUnknownJobbie>();
	AssertSize(0, gcUnknowns.NumElements());

	cFileUtil.RemoveDir("Output" _FS_ "MapUnknownUnknownNull");
	cFileUtil.MakeDir("Output" _FS_ "MapUnknownUnknownNull");

	cMap.Init();

	pcKey = gcUnknowns.Add<CTestNamedUnknown>();
	pcKey->Init("Something");
	pcValue = gcUnknowns.Add<CTestUnknownJobbie>();
	pcValue->Init(7, "Something");
	AssertTrue(cMap.Put(pcKey, pcValue));

	pcKey = gcUnknowns.Add<CTestNamedUnknown>();
	pcKey->Init("Nothing");
	AssertTrue(cMap.Put(pcKey, NULL));
	AssertSize(2, cMap.NumElements());
	AssertSize(3, gcUnknowns.NumElements());

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "MapUnknownUnknownNull" _FS_ "Map.dat")));
	bResult = cFile.WriteOpen();
	AssertTrue(bResult);

	bResult = cMap.Save(&cFile);
	AssertTrue(bResult);

	bResult = cFile.WriteClose();
	AssertTrue(bResult);

	cFile.Kill();
	cMap.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "MapUnknownUnknownNull" _FS_ "Map.dat")));
	bResult = cFile.ReadOpen();
	AssertTrue(bResult);

	bResult = cMap.Load(&cFile);
	AssertTrue(bResult);

	bResult = cFile.ReadClose();
	AssertTrue(bResult);
	cFile.Kill();
	AssertSize(2, cMap.NumElements());
	AssertSize(3, gcUnknowns.NumElements());

	uiNulls = 0;
	uiValues = 0;
	bExists = cMap.StartIteration(&sIter, &pcTestKey, &pcTestValue);
	while (bExists)
	{
		pcKey = (CTestNamedUnknown*)pcTestKey;
		if (pcTestValue == NULL)
		{
			AssertString("Nothing", pcKey->mszName.Text());
			uiNulls++;
		}
		else
		{
			pcValue = (CTestUnknownJobbie*)pcTestValue;
			AssertString("Something", pcKey->mszName.Text());
			AssertInt((int32)7, pcValue->miANumber);
			AssertString("Something", pcValue->mszText.Text());
			uiValues++;
		}
		bExists = cMap.Iterate(&sIter, &pcTestKey, &pcTestValue);
	}
	AssertSize(1, uiNulls);
	AssertSize(1, uiValues);

	cMap.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	cFileUtil.RemoveDir("Output" _FS_ "MapUnknownUnknownNull");

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
void TestMapUnknownUnknownReplaceValue(void)
{
	MemoryInit();
	UnknownsInit();

	CMapUnknownUnknown		cMap;
	CTestNamedUnknown*		pcKey;
	CTestUnknownJobbie*		pcValue1;
	CTestUnknownJobbie*		pcValue2;

	gcUnknowns.AddConstructor<CTestNamedUnknown>();
	gcUnknowns.AddConstructor<CTestUnknownJobbie>();
	AssertSize(0, gcUnknowns.NumElements());

	cMap.Init();

	pcKey = gcUnknowns.Add<CTestNamedUnknown>();
	pcKey->Init("Key");
	pcValue1 = gcUnknowns.Add<CTestUnknownJobbie>();
	pcValue1->Init(1, "One");
	pcValue2 = gcUnknowns.Add<CTestUnknownJobbie>();
	pcValue2->Init(2, "Two");

	AssertTrue(cMap.Put(pcKey, pcValue1));
	AssertSize(3, gcUnknowns.NumElements());

	//The old value is killed, the key is kept.
	AssertTrue(cMap.Put(pcKey, pcValue2));
	AssertSize(1, cMap.NumElements());
	AssertSize(2, gcUnknowns.NumElements());
	AssertPointer(pcValue2, cMap.Get(pcKey));
	AssertString("Key", pcKey->mszName.Text());

	cMap.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
void TestMapUnknownUnknownReplaceSameValue(void)
{
	MemoryInit();
	UnknownsInit();

	CMapUnknownUnknown		cMap;
	CTestNamedUnknown*		pcKey;
	CTestUnknownJobbie*		pcValue;

	gcUnknowns.AddConstructor<CTestNamedUnknown>();
	gcUnknowns.AddConstructor<CTestUnknownJobbie>();
	AssertSize(0, gcUnknowns.NumElements());

	cMap.Init();

	pcKey = gcUnknowns.Add<CTestNamedUnknown>();
	pcKey->Init("Key");
	pcValue = gcUnknowns.Add<CTestUnknownJobbie>();
	pcValue->Init(1, "One");

	AssertTrue(cMap.Put(pcKey, pcValue));
	AssertTrue(cMap.Put(pcKey, pcValue));
	AssertSize(1, cMap.NumElements());
	AssertSize(2, gcUnknowns.NumElements());
	AssertPointer(pcValue, cMap.Get(pcKey));
	AssertInt((int32)1, pcValue->miANumber);
	AssertString("Key", pcKey->mszName.Text());

	cMap.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
void TestMapUnknownUnknownNoOverwrite(void)
{
	MemoryInit();
	UnknownsInit();

	CMapUnknownUnknown		cMap;
	CTestNamedUnknown*		pcKey;
	CTestUnknownJobbie*		pcValue1;
	CTestUnknownJobbie*		pcValue2;

	gcUnknowns.AddConstructor<CTestNamedUnknown>();
	gcUnknowns.AddConstructor<CTestUnknownJobbie>();
	AssertSize(0, gcUnknowns.NumElements());

	cMap.Init(true, false);
	AssertFalse(cMap.IsOverwriteExisting());

	pcKey = gcUnknowns.Add<CTestNamedUnknown>();
	pcKey->Init("Key");
	pcValue1 = gcUnknowns.Add<CTestUnknownJobbie>();
	pcValue1->Init(1, "One");
	pcValue2 = gcUnknowns.Add<CTestUnknownJobbie>();
	pcValue2->Init(2, "Two");

	AssertTrue(cMap.Put(pcKey, pcValue1));
	AssertFalse(cMap.Put(pcKey, pcValue2));
	AssertSize(1, cMap.NumElements());
	AssertSize(3, gcUnknowns.NumElements());
	AssertPointer(pcValue1, cMap.Get(pcKey));

	//pcValue2 was never added so the map doesn't own it.
	pcValue2->Kill();
	cMap.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
void TestMapUnknownUnknown(void)
{
	BeginTests();

	TestMapUnknownUnknownAdd();
	TestMapUnknownUnknownIterate();
	TestMapUnknownUnknownGet();
	TestMapUnknownUnknownLoad();
	TestMapUnknownUnknownLoadNull();
	TestMapUnknownUnknownReplaceValue();
	TestMapUnknownUnknownReplaceSameValue();
	TestMapUnknownUnknownNoOverwrite();

	TestStatistics();
}


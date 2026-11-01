#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/BufferedFile.h"
#include "BaseLib/ArrayVoidPtr.h"
#include "BaseLib/ArrayBit.h"
#include "BaseLib/DataOrderers.h"
#include "StandardLib/Unknowns.h"
#include "StandardLib/IndexUnknown.h"
#include "StandardLib/IndexUnknown.h"
#include "TestLib/Assert.h"
#include "UnknownTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexUnknownAdd(void)
{
	MemoryInit();
	UnknownsInit();

	CIndexUnknown			cIndex;
	CTestUnknownJobbie*		pcValue1;
	CTestUnknownJobbie*		pcValue2;

	gcUnknowns.AddConstructor<CTestUnknown>();
	gcUnknowns.AddConstructor<CTestNamedUnknown>();
	gcUnknowns.AddConstructor<CTestUnknownJobbie>();
	gcUnknowns.AddConstructor<CTestIterableUnknown>();
	AssertSize(0, gcUnknowns.NumElements());

	cIndex.Init();

	pcValue1 = gcUnknowns.Add<CTestUnknownJobbie>();
	pcValue2 = gcUnknowns.Add<CTestUnknownJobbie>();
	pcValue1->Init(4, "Snurk");
	pcValue2->Init(67, "krust");
	AssertSize(0, cIndex.NumElements());

	cIndex.Put("Hello", pcValue1);
	AssertSize(1, cIndex.NumElements());

	cIndex.Put("World", pcValue2);
	AssertSize(2, cIndex.NumElements());

	cIndex.Kill();

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexUnknownIterate(void)
{
	MemoryInit();
	UnknownsInit();

	CIndexUnknown				cIndex;
	CTestUnknownJobbie*			pcValue1;
	CTestUnknownJobbie*			pcValue2;
	CTestUnknownJobbie*			pcValue3;
	SIndexTreeMemoryIterator	sIter;
	CUnknown*					pcValue;
	bool						bExists;
	size						uiKeyLength;
	char						szKey[20];

	gcUnknowns.AddConstructor<CTestUnknown>();
	gcUnknowns.AddConstructor<CTestNamedUnknown>();
	gcUnknowns.AddConstructor<CTestUnknownJobbie>();
	gcUnknowns.AddConstructor<CTestIterableUnknown>();
	AssertSize(0, gcUnknowns.NumElements());

	cIndex.Init();

	pcValue1 = gcUnknowns.Add<CTestUnknownJobbie>();
	pcValue2 = gcUnknowns.Add<CTestUnknownJobbie>();
	pcValue3 = gcUnknowns.Add<CTestUnknownJobbie>();
	pcValue1->Init(4, "Snurk");
	pcValue2->Init(67, "krust");
	pcValue3->Init(67, "Berry");
	AssertSize(0, cIndex.NumElements());

	cIndex.Put("SDFHLGJADH", pcValue1);
	cIndex.Put("SDFLKJF", pcValue2);
	cIndex.Put("SDBJHDFGKASFSDGF", pcValue3);
	AssertSize(3, cIndex.NumElements());

	memset(szKey, 0, 20);
	bExists = cIndex.StartIteration(&sIter, (uint8*)szKey, &uiKeyLength, 20, &pcValue);
	AssertTrue(bExists);
	AssertString("SDBJHDFGKASFSDGF", szKey);
	AssertPointer(pcValue3, pcValue);
	memset(szKey, 0, 20);
	bExists = cIndex.Iterate(&sIter, (uint8*)szKey, &uiKeyLength, 20, &pcValue);
	AssertTrue(bExists);
	AssertString("SDFHLGJADH", szKey);
	AssertPointer(pcValue1, pcValue);
	memset(szKey, 0, 20);
	bExists = cIndex.Iterate(&sIter, (uint8*)szKey, &uiKeyLength, 20, &pcValue);
	AssertTrue(bExists);
	AssertString("SDFLKJF", szKey);
	AssertPointer(pcValue2, pcValue);
	memset(szKey, 0, 20);
	bExists = cIndex.Iterate(&sIter, (uint8*)szKey, &uiKeyLength, 20, &pcValue);
	sIter.Kill();
	AssertFalse(bExists);

	AssertSize(3, cIndex.NumElements());

	cIndex.Kill();

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIndexUnknownGet(void)
{
	MemoryInit();
	UnknownsInit();

	CIndexUnknown			cIndex;
	CTestUnknownJobbie*		pcValue;
	size					ui;
	CUnknown*				pcTestValue;
	bool					bValid;
	char*					szKey;

	gcUnknowns.AddConstructor<CTestUnknown>();
	gcUnknowns.AddConstructor<CTestNamedUnknown>();
	gcUnknowns.AddConstructor<CTestUnknownJobbie>();
	gcUnknowns.AddConstructor<CTestIterableUnknown>();
	AssertSize(0, gcUnknowns.NumElements());

	cIndex.Init();

	for (ui = 0; ui < 12000; ui++)
	{
		pcValue = gcUnknowns.Add<CTestUnknownJobbie>();
		pcValue->Init(((int32)ui + 1) * 7, SizeToString(ui));
		szKey = SizeToString(ui);
		cIndex.Put(szKey, pcValue);
	}
	AssertSize(12000, cIndex.NumElements());

	bValid = false;
	for (ui = 0; ui < 12000; ui++)
	{
		szKey = SizeToString(ui);
		pcTestValue = cIndex.Get(szKey);

		bValid = StringCompare("CTestUnknownJobbie", pcTestValue->ClassName()) == 0;
		
		pcValue = (CTestUnknownJobbie*)pcTestValue;
		bValid &= ((int32)ui + 1) * 7 == pcValue->miANumber;
		bValid &= StringCompare(SizeToString(ui), pcValue->mszText.Text()) == 0;

		if (!bValid)
		{
			AssertTrue(bValid);
			break;
		}
	}
	AssertTrue(bValid);


	AssertSize(12000, cIndex.NumElements());

	cIndex.Kill();

	UnknownsKill();
	MemoryKill();
}


////////////////////////////////////////////////////////////////////////////
////
////
////////////////////////////////////////////////////////////////////////////
void TestIndexUnknownLoad(void)
{
	MemoryInit();
	UnknownsInit();
	DataOrderersInit();

	CIndexUnknown				cIndex;
	CIndexUnknown				cIndexIn;
	CFileUtil					cFileUtil;
	CTestUnknownJobbie*			pcValue1;
	CUnknown*					pcValue;
	size						ui;
	CChunkFile					cFile;
	bool						bResult;
	SIndexTreeMemoryIterator	sIter;
	bool						bExists;
	bool						bValid;
	char*						szKey1;
	size						uiKeyLength;
	char						szKey[20];

	gcUnknowns.AddConstructor<CTestUnknown>();
	gcUnknowns.AddConstructor<CTestNamedUnknown>();
	gcUnknowns.AddConstructor<CTestUnknownJobbie>();
	gcUnknowns.AddConstructor<CTestIterableUnknown>();
	AssertSize(0, gcUnknowns.NumElements());

	cFileUtil.RemoveDir("Output" _FS_ "IndexUnknown");
	cFileUtil.MakeDir("Output" _FS_ "IndexUnknown");

	cIndex.Init();

	for (ui = 0; ui < 12000; ui++)
	{
		pcValue1 = gcUnknowns.Add<CTestUnknownJobbie>();
		pcValue1->Init(((int32)ui + 1) * 7, SizeToString(ui));
		szKey1 = SizeToString(ui);
		cIndex.Put(szKey1, pcValue1);
	}
	AssertSize(12000, cIndex.NumElements());
	AssertSize(12000, gcUnknowns.NumElements());

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "IndexUnknown" _FS_ "Map.dat")));
	bResult = cFile.WriteOpen();
	AssertTrue(bResult);

	bResult = cIndex.Save(&cFile);
	AssertTrue(bResult);

	bResult = cFile.WriteClose();
	AssertTrue(bResult);

	cFile.Kill();
	cIndex.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "IndexUnknown" _FS_ "Map.dat")));
	bResult = cFile.ReadOpen();
	AssertTrue(bResult);

	bResult = cIndexIn.Load(&cFile);
	AssertTrue(bResult);

	bResult = cFile.ReadClose();
	AssertTrue(bResult);
	cFile.Kill();
	AssertSize(12000, cIndexIn.NumElements());
	AssertSize(12000, gcUnknowns.NumElements());

	ui = 0;
	bValid = false;
	bExists = cIndexIn.StartIteration(&sIter, (uint8*)szKey, &uiKeyLength, 20, &pcValue);
	while (bExists)
	{
		bValid = StringCompare("CTestUnknownJobbie", pcValue->ClassName()) == 0;
		if (!bValid)
		{
			AssertTrue(bValid);
			break;
		}

		bExists = cIndexIn.Iterate(&sIter, (uint8*)szKey, &uiKeyLength, 20, &pcValue);
		ui++;
	}
	sIter.Kill();
	AssertTrue(bValid);
	AssertSize(12000, ui);

	cIndexIn.Kill();

	DataOrderersKill();
	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
void TestIndexUnknownRemoveValue(void)
{
	MemoryInit();
	UnknownsInit();

	CIndexUnknown			cIndex;
	CTestUnknownJobbie*		pcA;
	CTestUnknownJobbie*		pcB;
	CTestUnknownJobbie*		pcC;
	CTestUnknownJobbie*		pcD;

	gcUnknowns.AddConstructor<CTestUnknownJobbie>();
	AssertSize(0, gcUnknowns.NumElements());

	cIndex.Init();

	pcA = gcUnknowns.Add<CTestUnknownJobbie>();
	pcB = gcUnknowns.Add<CTestUnknownJobbie>();
	pcC = gcUnknowns.Add<CTestUnknownJobbie>();
	pcD = gcUnknowns.Add<CTestUnknownJobbie>();
	pcA->Init(1, "A");
	pcB->Init(2, "B");
	pcC->Init(3, "C");
	pcD->Init(4, "D");
	cIndex.Put("A", pcA);
	cIndex.Put("B", pcB);
	cIndex.Put("C", pcC);
	cIndex.Put("D", pcD);
	AssertSize(4, gcUnknowns.NumElements());

	//Middle.
	AssertSize(1, cIndex.Remove(pcB));
	AssertSize(3, cIndex.NumElements());
	AssertSize(3, gcUnknowns.NumElements());
	AssertPointer(pcA, cIndex.Get("A"));
	AssertNull(cIndex.Get("B"));
	AssertPointer(pcC, cIndex.Get("C"));
	AssertPointer(pcD, cIndex.Get("D"));

	//Last.
	AssertSize(1, cIndex.Remove(pcD));
	AssertSize(2, cIndex.NumElements());
	AssertPointer(pcA, cIndex.Get("A"));
	AssertPointer(pcC, cIndex.Get("C"));
	AssertNull(cIndex.Get("D"));

	//First.
	AssertSize(1, cIndex.Remove(pcA));
	AssertSize(1, cIndex.NumElements());
	AssertNull(cIndex.Get("A"));
	AssertPointer(pcC, cIndex.Get("C"));
	AssertSize(1, gcUnknowns.NumElements());

	cIndex.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
void TestIndexUnknownRemoveDuplicateValue(void)
{
	MemoryInit();
	UnknownsInit();

	CIndexUnknown			cIndex;
	CTestUnknownJobbie*		pcA;
	CTestUnknownJobbie*		pcB;

	gcUnknowns.AddConstructor<CTestUnknownJobbie>();

	//The same value under two keys must not be killed twice, so the index doesn't kill its elements.
	cIndex.Init(false);

	pcA = gcUnknowns.Add<CTestUnknownJobbie>();
	pcB = gcUnknowns.Add<CTestUnknownJobbie>();
	pcA->Init(1, "A");
	pcB->Init(2, "B");
	cIndex.Put("A", pcA);
	cIndex.Put("B", pcB);
	cIndex.Put("C", pcA);
	cIndex.Put("D", pcB);
	cIndex.Put("E", pcA);

	AssertSize(2, cIndex.Remove(pcB));
	AssertSize(3, cIndex.NumElements());
	AssertPointer(pcA, cIndex.Get("A"));
	AssertNull(cIndex.Get("B"));
	AssertPointer(pcA, cIndex.Get("C"));
	AssertNull(cIndex.Get("D"));
	AssertPointer(pcA, cIndex.Get("E"));

	AssertSize(3, cIndex.Remove(pcA));
	AssertSize(0, cIndex.NumElements());
	AssertSize(0, cIndex.Remove(pcA));

	cIndex.Kill();
	pcA->Kill();
	pcB->Kill();
	AssertSize(0, gcUnknowns.NumElements());

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
void TestIndexUnknownLoadNull(void)
{
	MemoryInit();
	UnknownsInit();
	DataOrderersInit();

	CIndexUnknown				cIndex;
	CIndexUnknown				cIndexIn;
	CFileUtil					cFileUtil;
	CTestUnknownJobbie*			pcValue;
	CChunkFile					cFile;
	bool						bResult;

	gcUnknowns.AddConstructor<CTestUnknownJobbie>();
	AssertSize(0, gcUnknowns.NumElements());

	cFileUtil.RemoveDir("Output" _FS_ "IndexUnknownNull");
	cFileUtil.MakeDir("Output" _FS_ "IndexUnknownNull");

	cIndex.Init();

	pcValue = gcUnknowns.Add<CTestUnknownJobbie>();
	pcValue->Init(7, "Hello");
	AssertTrue(cIndex.Put("Hello", pcValue));
	AssertTrue(cIndex.Put("Nothing", (CUnknown*)NULL));
	pcValue = gcUnknowns.Add<CTestUnknownJobbie>();
	pcValue->Init(19, "World");
	AssertTrue(cIndex.Put("World", pcValue));
	AssertSize(3, cIndex.NumElements());
	AssertSize(2, gcUnknowns.NumElements());

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "IndexUnknownNull" _FS_ "Index.dat")));
	bResult = cFile.WriteOpen();
	AssertTrue(bResult);

	bResult = cIndex.Save(&cFile);
	AssertTrue(bResult);

	bResult = cFile.WriteClose();
	AssertTrue(bResult);

	cFile.Kill();
	cIndex.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "IndexUnknownNull" _FS_ "Index.dat")));
	bResult = cFile.ReadOpen();
	AssertTrue(bResult);

	bResult = cIndexIn.Load(&cFile);
	AssertTrue(bResult);

	bResult = cFile.ReadClose();
	AssertTrue(bResult);
	cFile.Kill();
	AssertSize(3, cIndexIn.NumElements());
	AssertSize(2, gcUnknowns.NumElements());

	pcValue = (CTestUnknownJobbie*)cIndexIn.Get("Hello");
	AssertInt((int32)7, pcValue->miANumber);
	AssertNull(cIndexIn.Get("Nothing"));
	pcValue = (CTestUnknownJobbie*)cIndexIn.Get("World");
	AssertInt((int32)19, pcValue->miANumber);

	AssertTrue(cIndexIn.Remove("Nothing"));
	AssertSize(2, cIndexIn.NumElements());

	cIndexIn.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	cFileUtil.RemoveDir("Output" _FS_ "IndexUnknownNull");

	DataOrderersKill();
	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
void TestIndexUnknownReplaceValues(void)
{
	MemoryInit();
	UnknownsInit();

	CIndexUnknown				cMap;
	CIndexUnknown				cNoOverwrite;
	CTestUnknownJobbie*		pcValue1;
	CTestUnknownJobbie*		pcValue2;

	gcUnknowns.AddConstructor<CTestUnknownJobbie>();
	AssertSize(0, gcUnknowns.NumElements());

	cMap.Init();
	pcValue1 = gcUnknowns.Add<CTestUnknownJobbie>();
	pcValue1->Init(1, "One");
	pcValue2 = gcUnknowns.Add<CTestUnknownJobbie>();
	pcValue2->Init(2, "Two");

	//Putting the same value again keeps it.
	AssertTrue(cMap.Put("Key", pcValue1));
	AssertTrue(cMap.Put("Key", pcValue1));
	AssertSize(1, cMap.NumElements());
	AssertSize(2, gcUnknowns.NumElements());
	AssertPointer(pcValue1, cMap.Get("Key"));
	AssertInt((int32)1, pcValue1->miANumber);

	//A new value replaces the old one, which is killed.
	AssertTrue(cMap.Put("Key", pcValue2));
	AssertSize(1, cMap.NumElements());
	AssertSize(1, gcUnknowns.NumElements());
	AssertPointer(pcValue2, cMap.Get("Key"));

	cMap.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	//Without overwriting the first value stays.
	cNoOverwrite.Init(true, false);
	pcValue1 = gcUnknowns.Add<CTestUnknownJobbie>();
	pcValue1->Init(1, "One");
	pcValue2 = gcUnknowns.Add<CTestUnknownJobbie>();
	pcValue2->Init(2, "Two");
	AssertTrue(cNoOverwrite.Put("Key", pcValue1));
	AssertFalse(cNoOverwrite.Put("Key", pcValue2));
	AssertSize(1, cNoOverwrite.NumElements());
	AssertPointer(pcValue1, cNoOverwrite.Get("Key"));
	AssertSize(2, gcUnknowns.NumElements());

	pcValue2->Kill();
	cNoOverwrite.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
void TestIndexUnknown(void)
{
	BeginTests();

	TestIndexUnknownAdd();
	TestIndexUnknownIterate();
	TestIndexUnknownGet();
	TestIndexUnknownLoad();
	TestIndexUnknownRemoveValue();
	TestIndexUnknownRemoveDuplicateValue();
	TestIndexUnknownLoadNull();
	TestIndexUnknownReplaceValues();

	TestStatistics();
}


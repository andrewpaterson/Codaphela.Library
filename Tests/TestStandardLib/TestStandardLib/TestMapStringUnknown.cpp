#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/BufferedFile.h"
#include "StandardLib/Unknowns.h"
#include "StandardLib/MapStringUnknown.h"
#include "TestLib/Assert.h"
#include "UnknownTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapStringUnknownPut(void)
{
	MemoryInit();
	UnknownsInit();

	CMapStringUnknown	cMap;
	CTestUnknownJobbie*	pcTest;

	cMap.Init();

	AssertSize(0, cMap.NumElements());

	pcTest = cMap.Put<CTestUnknownJobbie>("Hello");
	pcTest->Init(7, "1st");
	AssertSize(1, cMap.NumElements());

	pcTest = cMap.Put<CTestUnknownJobbie>("World");
	pcTest->Init(19, "A pony");
	AssertSize(2, cMap.NumElements());

	pcTest = cMap.Put<CTestUnknownJobbie>("Aardvark");
	pcTest->Init(4, "Restore");
	AssertSize(3, cMap.NumElements());

	pcTest = cMap.Put<CTestUnknownJobbie>(NULL);
	AssertNull(pcTest);
	AssertSize(3, cMap.NumElements());

	cMap.Kill();

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapStringUnknownGet(void)
{
	MemoryInit();
	UnknownsInit();

	CMapStringUnknown	cMap;
	CTestUnknownJobbie*	pcTest;
	CTestUnknownJobbie*	pcA1;
	CTestUnknownJobbie*	pcA2;
	CTestUnknownJobbie*	pcA3;

	cMap.Init();

	pcTest = cMap.Put<CTestUnknownJobbie>("Hello");
	pcTest->Init(7, "1st");
	pcTest = cMap.Put<CTestUnknownJobbie>("World");
	pcTest->Init(19, "A pony");
	pcTest = cMap.Put<CTestUnknownJobbie>("Aardvark");
	pcTest->Init(4, "Restore");
	AssertSize(3, cMap.NumElements());
	AssertSize(3, gcUnknowns.NumElements());

	pcTest = (CTestUnknownJobbie*)cMap.Get("World");
	AssertInt((int32)19, pcTest->miANumber);
	AssertString("A pony", pcTest->mszText.Text());

	pcA1 = UMalloc(CTestUnknownJobbie);
	pcA1->Init(999, "Grand");
	cMap.Put("Thesaurus", pcA1);
	AssertSize(4, cMap.NumElements());
	AssertSize(4, gcUnknowns.NumElements());
	pcA2 = UMalloc(CTestUnknownJobbie);
	pcA2->Init(17, "Replaced!");
	cMap.Put("World", pcA2);
	AssertSize(4, cMap.NumElements());
	AssertSize(4, gcUnknowns.NumElements());
	pcA3 = UMalloc(CTestUnknownJobbie);
	pcA3->Init(32, "Another");
	cMap.Put("Jump", pcA3);
	AssertSize(5, cMap.NumElements());
	AssertSize(5, gcUnknowns.NumElements());

	pcTest = (CTestUnknownJobbie*)cMap.Get("World");
	AssertPointer(pcA2, pcTest);
	AssertString("Replaced!", pcTest->mszText.Text());

	pcTest = (CTestUnknownJobbie*)cMap.Get("world");
	AssertPointer(NULL, pcTest);
	pcTest = (CTestUnknownJobbie*)cMap.Get(NULL);
	AssertPointer(NULL, pcTest);
	pcTest = (CTestUnknownJobbie*)cMap.Get("");
	AssertPointer(NULL, pcTest);

	pcTest = cMap.Put<CTestUnknownJobbie>("");
	AssertNull(pcTest);

	cMap.Kill();

	AssertSize(0, gcUnknowns.NumElements());

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapStringUnknownIterate(void)
{
	MemoryInit();
	UnknownsInit();

	CMapStringUnknown		cMap;
	CTestUnknownJobbie*		pcTest1;
	CTestUnknownJobbie*		pcTest2;
	CTestUnknownJobbie*		pcTest3;
	SMapIterator			sIter;
	char*					pszKey;
	CUnknown*				pcValue;
	bool					bResult;

	cMap.Init();

	AssertSize(0, cMap.NumElements());

	pcTest1 = cMap.Put<CTestUnknownJobbie>("World");
	pcTest1->Init(19, "A pony");
	pcTest2 = cMap.Put<CTestUnknownJobbie>("Hello");
	pcTest2->Init(7, "1st");
	pcTest3 = cMap.Put<CTestUnknownJobbie>("Aardvark");
	pcTest3->Init(4, "Restore");
	AssertSize(3, cMap.NumElements());

	bResult = cMap.StartIteration(&sIter, &pszKey, &pcValue);
	AssertTrue(bResult);
	AssertString("Aardvark", pszKey);
	AssertPointer(pcTest3, pcValue);
	bResult = cMap.Iterate(&sIter, &pszKey, &pcValue);
	AssertTrue(bResult);
	AssertString("Hello", pszKey);
	AssertPointer(pcTest2, pcValue);
	bResult = cMap.Iterate(&sIter, &pszKey, &pcValue);
	AssertTrue(bResult);
	AssertString("World", pszKey);
	AssertPointer(pcTest1, pcValue);
	bResult = cMap.Iterate(&sIter, &pszKey, &pcValue);
	AssertFalse(bResult);

	cMap.Kill();

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapStringUnknownNoOverwrite(void)
{
	MemoryInit();
	UnknownsInit();

	CMapStringUnknown	cMap;
	CTestUnknownJobbie*	pcTest;
	SLogConfig			sLogConfig;

	cMap.Init(true, false);

	AssertSize(0, gcUnknowns.NumElements());

	pcTest = cMap.Put<CTestUnknownJobbie>("One");
	pcTest->Init(2, "");
	AssertNotNull(pcTest);
	AssertSize(1, gcUnknowns.NumElements());

	sLogConfig = gcLogger.SetSilent();
	pcTest = cMap.Put<CTestUnknownJobbie>("One");
	AssertNull(pcTest);
	AssertSize(1, gcUnknowns.NumElements());
	gcLogger.SetConfig(&sLogConfig);

	cMap.Kill();

	AssertSize(0, gcUnknowns.NumElements());

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapStringUnknownLoad(void)
{
	MemoryInit();
	UnknownsInit();

	CMapStringUnknown		cMap;
	CTestUnknownJobbie*		pcTest;
	CChunkFile				cFile;
	CFileUtil				cFileUtil;
	bool					bResult;

	gcUnknowns.AddConstructor<CTestUnknownJobbie>();
	AssertSize(0, gcUnknowns.NumElements());

	cFileUtil.RemoveDir("Output" _FS_ "MapStringUnknown");
	cFileUtil.MakeDir("Output" _FS_ "MapStringUnknown");

	cMap.Init();

	pcTest = cMap.Put<CTestUnknownJobbie>("Hello");
	pcTest->Init(7, "1st");
	pcTest = cMap.Put<CTestUnknownJobbie>("World");
	pcTest->Init(19, "A pony");
	pcTest = cMap.Put<CTestUnknownJobbie>("Aardvark");
	pcTest->Init(4, "Restore");
	pcTest = cMap.Put<CTestUnknownJobbie>("Yurk");
	pcTest->Init(8, "Yurk");

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "MapStringUnknown" _FS_ "Map.dat")));
	bResult = cFile.WriteOpen();
	AssertTrue(bResult);

	bResult = cMap.Save(&cFile);
	AssertTrue(bResult);

	bResult = cFile.WriteClose();
	AssertTrue(bResult);
	AssertSize(4, gcUnknowns.NumElements());

	cFile.Kill();
	cMap.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "MapStringUnknown" _FS_ "Map.dat")));
	bResult = cFile.ReadOpen();
	AssertTrue(bResult);

	bResult = cMap.Load(&cFile);
	AssertTrue(bResult);

	bResult = cFile.ReadClose();
	AssertTrue(bResult);

	cFile.Kill();
	AssertSize(4, cMap.NumElements());
	AssertSize(4, gcUnknowns.NumElements());

	pcTest = (CTestUnknownJobbie*)cMap.Get("Hello");
	AssertInt((int32)7, pcTest->miANumber);
	AssertString("1st", pcTest->mszText.Text());

	pcTest = (CTestUnknownJobbie*)cMap.Get("World");
	AssertInt((int32)19, pcTest->miANumber);
	AssertString("A pony", pcTest->mszText.Text());

	pcTest = (CTestUnknownJobbie*)cMap.Get("Aardvark");
	AssertInt((int32)4, pcTest->miANumber);
	AssertString("Restore", pcTest->mszText.Text());

	pcTest = (CTestUnknownJobbie*)cMap.Get("Yurk");
	AssertInt((int32)8, pcTest->miANumber);
	AssertString("Yurk", pcTest->mszText.Text());
	AssertSize(4, gcUnknowns.NumElements());

	cMap.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	cFileUtil.RemoveDir("Output" _FS_ "MapStringUnknown");

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapStringUnknownLoadNull(void)
{
	MemoryInit();
	UnknownsInit();

	CMapStringUnknown		cMap;
	CTestUnknownJobbie*		pcTest;
	CChunkFile				cFile;
	CFileUtil				cFileUtil;
	bool					bResult;

	gcUnknowns.AddConstructor<CTestUnknownJobbie>();
	AssertSize(0, gcUnknowns.NumElements());

	cFileUtil.RemoveDir("Output" _FS_ "MapStringUnknownNull");
	cFileUtil.MakeDir("Output" _FS_ "MapStringUnknownNull");

	cMap.Init();

	pcTest = cMap.Put<CTestUnknownJobbie>("Hello");
	pcTest->Init(7, "1st");
	AssertTrue(cMap.Put("Nothing", (CUnknown*)NULL));
	pcTest = cMap.Put<CTestUnknownJobbie>("World");
	pcTest->Init(19, "A pony");
	AssertSize(3, cMap.NumElements());
	AssertSize(2, gcUnknowns.NumElements());

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "MapStringUnknownNull" _FS_ "Map.dat")));
	bResult = cFile.WriteOpen();
	AssertTrue(bResult);

	bResult = cMap.Save(&cFile);
	AssertTrue(bResult);

	bResult = cFile.WriteClose();
	AssertTrue(bResult);

	cFile.Kill();
	cMap.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "MapStringUnknownNull" _FS_ "Map.dat")));
	bResult = cFile.ReadOpen();
	AssertTrue(bResult);

	bResult = cMap.Load(&cFile);
	AssertTrue(bResult);

	bResult = cFile.ReadClose();
	AssertTrue(bResult);

	cFile.Kill();
	AssertSize(3, cMap.NumElements());
	AssertSize(2, gcUnknowns.NumElements());

	pcTest = (CTestUnknownJobbie*)cMap.Get("Hello");
	AssertInt((int32)7, pcTest->miANumber);
	AssertString("1st", pcTest->mszText.Text());

	AssertNull(cMap.Get("Nothing"));

	pcTest = (CTestUnknownJobbie*)cMap.Get("World");
	AssertInt((int32)19, pcTest->miANumber);
	AssertString("A pony", pcTest->mszText.Text());

	cMap.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	cFileUtil.RemoveDir("Output" _FS_ "MapStringUnknownNull");

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapStringUnknownReplaceValues(void)
{
	MemoryInit();
	UnknownsInit();

	CMapStringUnknown				cMap;
	CMapStringUnknown				cNoOverwrite;
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
//
//
//////////////////////////////////////////////////////////////////////////
void TestMapStringUnknown(void)
{
	BeginTests();

	TestMapStringUnknownPut();
	TestMapStringUnknownGet();
	TestMapStringUnknownIterate();
	TestMapStringUnknownNoOverwrite();
	TestMapStringUnknownLoad();
	TestMapStringUnknownLoadNull();
	TestMapStringUnknownReplaceValues();

	TestStatistics();
}


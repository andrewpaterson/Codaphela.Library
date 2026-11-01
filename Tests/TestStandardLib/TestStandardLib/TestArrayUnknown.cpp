#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/BufferedFile.h"
#include "TestLib/Assert.h"
#include "StandardLib/Unknowns.h"
#include "StandardLib/ArrayUnknown.h"
#include "UnknownTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayAllocation(void)
{
	MemoryInit();
	UnknownsInit();

	CArrayUnknown							cArray;
	CTestNamedUnknown*						pcTest1;
	CTestNamedUnknown*						pcTest2;
	CTestNamedUnknown*						pcTest3;
	size									iNum;
	CTestNamedUnknown*						pcTest;
	SIteratorTemplate<CTestNamedUnknown>	sIter;

	cArray.Init();

	pcTest1 = cArray.Add<CTestNamedUnknown>();
	pcTest1->Init("The One");

	pcTest2 = cArray.Add<CTestNamedUnknown>();
	pcTest2->Init("Two");

	pcTest3 = UMalloc(CTestNamedUnknown);
	pcTest3->Init("There can be only three");
	cArray.Add(pcTest3);
	
	iNum = cArray.NumElements();
	AssertSize(3, iNum);
	pcTest = (CTestNamedUnknown*)cArray.Get(0);
	AssertString("The One", pcTest->GetName());
	pcTest = (CTestNamedUnknown*)cArray.Get(1);
	AssertString("Two", pcTest->GetName());
	pcTest = (CTestNamedUnknown*)cArray.Get(2);
	AssertString("There can be only three", pcTest->GetName());

	cArray.Remove(1);

	iNum = cArray.NumElements();
	AssertSize(2, iNum);
	pcTest = (CTestNamedUnknown*)cArray.Get(0);
	AssertString("The One", pcTest->GetName());
	pcTest = (CTestNamedUnknown*)cArray.Get(1);
	AssertString("There can be only three", pcTest->GetName());

	pcTest = gcUnknowns.StartIteration(&sIter);
	AssertString("The One", pcTest->GetName());
	pcTest = gcUnknowns.Iterate(&sIter);
	AssertString("There can be only three", pcTest->GetName());
	
	cArray.Kill();

	pcTest = gcUnknowns.StartIteration(&sIter);
	AssertNull(pcTest);

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRemoval(void)
{
	MemoryInit();
	UnknownsInit();

	CArrayUnknown							cArray;
	CTestNamedUnknown*						pcTest1;
	CTestNamedUnknown*						pcTest2;
	CTestNamedUnknown*						pcTest3;
	size									iNum;
	CTestNamedUnknown*						pcTest;
	SIteratorTemplate<CTestNamedUnknown>	sIter;
	SSetIterator							sSetIter;
	bool									bExists;

	cArray.Init();

	pcTest1 = cArray.Add<CTestNamedUnknown>();
	pcTest1->Init("The One");

	pcTest2 = cArray.Add<CTestNamedUnknown>();
	pcTest2->Init("Two");

	pcTest3 = UMalloc(CTestNamedUnknown);
	pcTest3->Init("There can be only three");
	cArray.Add(pcTest3);

	iNum = cArray.NumElements();
	AssertSize(3, iNum);

	cArray.StartIteration(&sSetIter, NULL);
	cArray.RemoveDuringIteration(&sSetIter);
	cArray.Iterate(&sSetIter, NULL);
	cArray.RemoveDuringIteration(&sSetIter);
	cArray.Iterate(&sSetIter, NULL);
	cArray.RemoveDuringIteration(&sSetIter);
	cArray.Iterate(&sSetIter, NULL);

	iNum = cArray.NumElements();
	AssertSize(0, iNum);

	cArray.RemoveDuringIteration(&sSetIter);

	pcTest = gcUnknowns.StartIteration(&sIter);
	AssertNull(pcTest);

	cArray.Kill();

	cArray.Init();

	pcTest1 = cArray.Add<CTestNamedUnknown>();
	pcTest1->Init("The One");

	pcTest2 = cArray.Add<CTestNamedUnknown>();
	pcTest2->Init("Two");

	pcTest3 = UMalloc(CTestNamedUnknown);
	pcTest3->Init("There can be only three");
	cArray.Add(pcTest3);

	iNum = cArray.NumElements();
	AssertSize(3, iNum);

	bExists = cArray.StartIteration(&sSetIter, (CUnknown**)&pcTest);
	AssertTrue(bExists);
	AssertString("The One", pcTest->GetName());

	cArray.RemoveDuringIteration(&sSetIter);
	bExists = cArray.Iterate(&sSetIter, (CUnknown**)&pcTest);
	AssertTrue(bExists);
	AssertString("Two", pcTest->GetName());

	cArray.RemoveDuringIteration(&sSetIter);
	iNum = cArray.NumElements();
	AssertSize(1, iNum);
	bExists = cArray.Iterate(&sSetIter, (CUnknown**)&pcTest);
	AssertTrue(bExists);
	AssertString("There can be only three", pcTest->GetName());

	cArray.RemoveDuringIteration(&sSetIter);
	iNum = cArray.NumElements();
	AssertSize(0, iNum);
	bExists = cArray.Iterate(&sSetIter, NULL);
	AssertFalse(bExists);

	cArray.RemoveDuringIteration(&sSetIter);
	cArray.Kill();

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIteration(void)
{
	MemoryInit();
	UnknownsInit();

	CArrayUnknown			cArray;
	CTestNamedUnknown*		pcTest;
	SSetIterator			sSetIter;
	bool					bExists;

	cArray.Init();

	pcTest = cArray.Add<CTestNamedUnknown>();
	pcTest->Init("A");
	pcTest = cArray.Add<CTestNamedUnknown>();
	pcTest->Init("B");
	pcTest = cArray.Add<CTestNamedUnknown>();
	pcTest->Init("C");
	pcTest = cArray.Add<CTestNamedUnknown>();
	pcTest->Init("D");

	bExists = cArray.StartIteration(&sSetIter, (CUnknown**)&pcTest);
	AssertTrue(bExists);
	AssertString("A", pcTest->GetName());
	AssertSize(0, sSetIter.iIndex);
	bExists = cArray.Iterate(&sSetIter, (CUnknown**)&pcTest);
	AssertTrue(bExists);
	AssertString("B", pcTest->GetName());
	AssertSize(1, sSetIter.iIndex);
	bExists = cArray.Iterate(&sSetIter, (CUnknown**)&pcTest);
	AssertTrue(bExists);
	AssertString("C", pcTest->GetName());
	AssertSize(2, sSetIter.iIndex);
	bExists = cArray.Iterate(&sSetIter, (CUnknown**)&pcTest);
	AssertTrue(bExists);
	AssertString("D", pcTest->GetName());
	AssertSize(3, sSetIter.iIndex);
	bExists = cArray.Iterate(&sSetIter, (CUnknown**)&pcTest);
	AssertFalse(bExists);
	AssertNull(pcTest);
	AssertSize(4, sSetIter.iIndex);
	bExists = cArray.Iterate(&sSetIter, (CUnknown**)&pcTest);
	AssertFalse(bExists);
	AssertNull(pcTest);
	AssertSize(4, sSetIter.iIndex);

	
	cArray.Kill();

	UnknownsKill();
	MemoryKill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayUnknownLoad(void)
{
	MemoryInit();
	UnknownsInit();

	CArrayUnknown		cArray;
	CChunkFile			cFile;
	CFileUtil			cFileUtil;
	CTestUnknown*		pcTest;
	CTestUnknownJobbie*	pcTestJobbie;
	bool				bResult;
	CUnknown*			pcUnknown;

	AssertSize(0, gcConstructors.NumConstructors());
	AssertSize(0, gcUnknowns.NumElements());
	gcUnknowns.AddConstructor<CTestUnknownJobbie>();
	gcUnknowns.AddConstructor<CTestUnknown>();
	AssertSize(0, gcUnknowns.NumElements());
	AssertSize(2, gcConstructors.NumConstructors());

	cFileUtil.RemoveDir("Output" _FS_ "ArrayUnknown");
	cFileUtil.TouchDir("Output" _FS_ "ArrayUnknown");

	cArray.Init();
	pcTest = cArray.Add<CTestUnknown>();
	pcTest->Init(3);
	pcTestJobbie = cArray.Add<CTestUnknownJobbie>();
	pcTestJobbie->Init(2, "Two");
	pcTestJobbie = cArray.Add<CTestUnknownJobbie>();
	pcTestJobbie->Init(1, "Onety");
	pcTest = cArray.Add<CTestUnknown>();
	pcTest->Init(0);

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "ArrayUnknown" _FS_ "Array.dat")));
	bResult = cFile.WriteOpen();
	AssertTrue(bResult);

	bResult = cArray.Save(&cFile);
	AssertTrue(bResult);

	bResult = cFile.WriteClose();
	AssertTrue(bResult);

	cFile.Kill();
	cArray.Kill();

	AssertSize(0, gcUnknowns.NumElements());

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "ArrayUnknown" _FS_ "Array.dat")));
	bResult = cFile.ReadOpen();
	AssertTrue(bResult);

	bResult = cArray.Load(&cFile);
	AssertTrue(bResult);

	bResult = cFile.ReadClose();
	AssertTrue(bResult);

	cFile.Kill();

	AssertSize(4, cArray.NumElements());
	AssertSize(4, gcUnknowns.NumElements());

	pcUnknown = cArray.Get(0);
	AssertString("CTestUnknown", pcUnknown->ClassName());
	AssertInt((int32)3, ((CTestUnknown*)pcUnknown)->miCount);

	pcUnknown = cArray.Get(1);
	AssertString("CTestUnknownJobbie", pcUnknown->ClassName());
	AssertInt((int32)2, ((CTestUnknownJobbie*)pcUnknown)->miANumber);
	AssertString("Two", ((CTestUnknownJobbie*)pcUnknown)->mszText.Text());

	pcUnknown = cArray.Get(2);
	AssertString("CTestUnknownJobbie", pcUnknown->ClassName());
	AssertInt((int32)1, ((CTestUnknownJobbie*)pcUnknown)->miANumber);
	AssertString("Onety", ((CTestUnknownJobbie*)pcUnknown)->mszText.Text());

	pcUnknown = cArray.Get(3);
	AssertString("CTestUnknown", pcUnknown->ClassName());
	AssertInt((int32)0, ((CTestUnknown*)pcUnknown)->miCount);

	cArray.Kill();

	AssertSize(0, gcUnknowns.NumElements());

	cFileUtil.RemoveDir("Output" _FS_ "ArrayUnknown");

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayUnknownLoadNull(void)
{
	MemoryInit();
	UnknownsInit();

	CArrayUnknown		cArray;
	CChunkFile			cFile;
	CFileUtil			cFileUtil;
	CTestUnknown*		pcTest;
	CTestUnknownJobbie*	pcTestJobbie;
	CUnknown*			pcUnknown;

	gcUnknowns.AddConstructor<CTestUnknownJobbie>();
	gcUnknowns.AddConstructor<CTestUnknown>();

	cFileUtil.RemoveDir("Output" _FS_ "ArrayUnknownNull");
	cFileUtil.TouchDir("Output" _FS_ "ArrayUnknownNull");

	cArray.Init();
	pcTest = cArray.Add<CTestUnknown>();
	pcTest->Init(1);
	AssertTrue(cArray.Add((CUnknown*)NULL));
	pcTestJobbie = cArray.Add<CTestUnknownJobbie>();
	pcTestJobbie->Init(2, "Two");
	AssertTrue(cArray.Insert(0, (CUnknown*)NULL));
	AssertSize(4, cArray.NumElements());
	AssertSize(2, gcUnknowns.NumElements());

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "ArrayUnknownNull" _FS_ "Array.dat")));
	AssertTrue(cFile.WriteOpen());
	AssertTrue(cArray.Save(&cFile));
	AssertTrue(cFile.WriteClose());
	cFile.Kill();
	cArray.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "ArrayUnknownNull" _FS_ "Array.dat")));
	AssertTrue(cFile.ReadOpen());
	AssertTrue(cArray.Load(&cFile));
	AssertTrue(cFile.ReadClose());
	cFile.Kill();

	AssertSize(4, cArray.NumElements());
	AssertSize(2, gcUnknowns.NumElements());
	AssertNull(cArray.Get(0));
	pcUnknown = cArray.Get(1);
	AssertString("CTestUnknown", pcUnknown->ClassName());
	AssertInt((int32)1, ((CTestUnknown*)pcUnknown)->miCount);
	AssertNull(cArray.Get(2));
	pcUnknown = cArray.Get(3);
	AssertString("CTestUnknownJobbie", pcUnknown->ClassName());
	AssertInt((int32)2, ((CTestUnknownJobbie*)pcUnknown)->miANumber);

	cArray.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	cFileUtil.RemoveDir("Output" _FS_ "ArrayUnknownNull");

	UnknownsKill();
	MemoryKill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayUnknown(void)
{
	BeginTests();

	TestArrayAllocation();
	TestIteration();
	TestRemoval();
	TestArrayUnknownLoad();
	TestArrayUnknownLoadNull();

	TestStatistics();
}


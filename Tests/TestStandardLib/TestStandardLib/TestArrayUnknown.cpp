#include "BaseLib/GlobalMemory.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/BufferedFile.h"
#include "TestLib/Assert.h"
#include "StandardLib/Unknowns.h"
#include "StandardLib/ArrayUnknown.h"
#include "StandardLib/SetUnknown.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/Logger.h"
#include "UnknownTestClasses.h"


void StartCapturingLog(CMemoryFile* pcMemory, SLogConfig* psLogConfig);  //TestObjectFreeOrdering.cpp
void StopCapturingLog(CMemoryFile* pcMemory, SLogConfig* psLogConfig, char* szLogged, size uiSize);


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
// The array kills its elements, so the one Set replaces is killed.
//
//////////////////////////////////////////////////////////////////////////
void TestArrayUnknownSetKillsReplaced(void)
{
	MemoryInit();
	UnknownsInit();

	CArrayUnknown			cArray;
	CTestNamedUnknown*		pcOne;
	CTestNamedUnknown*		pcTwo;

	cArray.Init();
	pcOne = cArray.Add<CTestNamedUnknown>();
	pcOne->Init("One");
	pcTwo = UMalloc(CTestNamedUnknown);
	pcTwo->Init("Two");
	AssertSize(2, gcUnknowns.NumElements());

	AssertTrue(cArray.Set(0, pcTwo));
	AssertSize(1, gcUnknowns.NumElements());
	AssertSize(1, cArray.NumElements());
	AssertPointer(pcTwo, cArray.Get(0));

	cArray.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
// An array kept sorted that doesn't preserve order is still sorted, and
// still finds every element, after one is removed.
//
//////////////////////////////////////////////////////////////////////////
void TestArrayUnknownRemoveKeepsSorted(void)
{
	MemoryInit();
	UnknownsInit();

	CArrayCommonUnknown	cArray;
	CTestUnknown*		apcTests[5];
	size				i;

	cArray.Init(ARRAY_COMMON_KILL_ELEMENT | ARRAY_COMMON_SORT_POINTERS);
	for (i = 0; i < 5; i++)
	{
		apcTests[i] = UMalloc(CTestUnknown);
		apcTests[i]->Init((int)i);
		AssertTrue(cArray.Add(apcTests[i]));
	}
	AssertTrue(cArray.IsSorted());

	AssertTrue(cArray.Remove(cArray.Find(apcTests[0]), true));
	AssertSize(4, cArray.NumElements());
	AssertTrue(cArray.CalculateIsSorted());
	for (i = 1; i < 5; i++)
	{
		AssertTrue(cArray.Contains(apcTests[i]));
	}

	cArray.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Removing during iteration from the middle of an array goes on to the
// element after the one removed.
//
//////////////////////////////////////////////////////////////////////////
void TestArrayUnknownRemoveDuringIterationMiddle(void)
{
	MemoryInit();
	UnknownsInit();

	CArrayUnknown	cArray;
	CTestUnknown*	pcTest;
	SSetIterator	sIter;
	CUnknown*		pcUnknown;
	bool			bExists;
	size			i;

	cArray.Init();
	for (i = 0; i < 4; i++)
	{
		pcTest = cArray.Add<CTestUnknown>();
		pcTest->Init((int)i);
	}

	bExists = cArray.StartIteration(&sIter, &pcUnknown);
	AssertTrue(bExists);
	AssertInt((int32)0, ((CTestUnknown*)pcUnknown)->miCount);
	bExists = cArray.Iterate(&sIter, &pcUnknown);
	AssertTrue(bExists);
	AssertInt((int32)1, ((CTestUnknown*)pcUnknown)->miCount);

	AssertTrue(cArray.RemoveDuringIteration(&sIter));
	bExists = cArray.Iterate(&sIter, &pcUnknown);
	AssertTrue(bExists);
	AssertInt((int32)2, ((CTestUnknown*)pcUnknown)->miCount);
	bExists = cArray.Iterate(&sIter, &pcUnknown);
	AssertTrue(bExists);
	AssertInt((int32)3, ((CTestUnknown*)pcUnknown)->miCount);
	bExists = cArray.Iterate(&sIter, &pcUnknown);
	AssertFalse(bExists);
	AssertSize(3, cArray.NumElements());

	cArray.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
// The header's count of elements that aren't NULL is checked against its
// count of elements, and replaced by what was read.
//
//////////////////////////////////////////////////////////////////////////
void TestArrayUnknownLoadBadNonNullCount(void)
{
	MemoryInit();
	UnknownsInit();

	CArrayUnknown		cArray;
	CChunkFile			cFile;
	CFileUtil			cFileUtil;
	CMemoryFile			cLog;
	SLogConfig			sLogConfig;
	char				szLogged[512];

	cFileUtil.RemoveDir("Output" _FS_ "ArrayUnknownCount");
	cFileUtil.TouchDir("Output" _FS_ "ArrayUnknownCount");

	//Two elements that aren't NULL in an array of one.
	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "ArrayUnknownCount" _FS_ "TooMany.dat")));
	AssertTrue(cFile.WriteOpen());
	AssertTrue(cFile.WriteSize((size)1));
	AssertTrue(cFile.WriteSize((size)2));
	AssertTrue(cFile.WriteSize((size)1));
	AssertTrue(cFile.WriteInt16((uint16)0));
	AssertTrue(gcUnknowns.SaveUnknown(&cFile, NULL));
	AssertTrue(cFile.WriteClose());
	cFile.Kill();

	cArray._Init();
	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "ArrayUnknownCount" _FS_ "TooMany.dat")));
	AssertTrue(cFile.ReadOpen());
	StartCapturingLog(&cLog, &sLogConfig);
	AssertFalse(cArray.Load(&cFile));
	StopCapturingLog(&cLog, &sLogConfig, szLogged, sizeof(szLogged));
	AssertNotNull(strstr(szLogged, "The array has [2] elements that aren't NULL but only [1] elements."));
	AssertTrue(cFile.ReadClose());
	cFile.Kill();
	cArray.Kill();

	//Two that aren't NULL, but both elements are.
	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "ArrayUnknownCount" _FS_ "AllNull.dat")));
	AssertTrue(cFile.WriteOpen());
	AssertTrue(cFile.WriteSize((size)2));
	AssertTrue(cFile.WriteSize((size)2));
	AssertTrue(cFile.WriteSize((size)1));
	AssertTrue(cFile.WriteInt16((uint16)0));
	AssertTrue(gcUnknowns.SaveUnknown(&cFile, NULL));
	AssertTrue(gcUnknowns.SaveUnknown(&cFile, NULL));
	AssertTrue(cFile.WriteClose());
	cFile.Kill();

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "ArrayUnknownCount" _FS_ "AllNull.dat")));
	AssertTrue(cFile.ReadOpen());
	AssertTrue(cArray.Load(&cFile));
	AssertTrue(cFile.ReadClose());
	cFile.Kill();
	AssertSize(2, cArray.UnsafeNumElements());
	AssertSize(0, cArray.UnsafeNonNullElements());
	cArray.Kill();

	cFileUtil.RemoveDir("Output" _FS_ "ArrayUnknownCount");

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
// An array that ignores NULLs is cleaned when enough of it is NULL, even
// when all of it is.
//
//////////////////////////////////////////////////////////////////////////
void TestArrayUnknownCleanAllNull(void)
{
	MemoryInit();
	UnknownsInit();

	CSetUnknown		cSet;
	CTestUnknown*	pcTest;
	size			i;

	cSet.Init(true, false, true, false, 1, false);
	for (i = 0; i < 64; i++)
	{
		pcTest = cSet.Add<CTestUnknown>();
		pcTest->Init();
	}
	for (i = 0; i < 64; i++)
	{
		AssertTrue(cSet.CArrayCommonUnknown::Remove(i, false));  //Leaves a NULL; doesn't clean.
	}
	AssertSize(64, cSet.UnsafeNumElements());
	AssertSize(0, cSet.UnsafeNonNullElements());

	pcTest = cSet.Add<CTestUnknown>();
	pcTest->Init();
	AssertSize(1, cSet.UnsafeNumElements());
	AssertSize(1, cSet.UnsafeNonNullElements());

	cSet.Kill();
	AssertSize(0, gcUnknowns.NumElements());

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
	TestArrayUnknownSetKillsReplaced();
	TestArrayUnknownRemoveKeepsSorted();
	TestArrayUnknownRemoveDuringIterationMiddle();
	TestArrayUnknownLoadBadNonNullCount();
	TestArrayUnknownCleanAllNull();

	TestStatistics();
}


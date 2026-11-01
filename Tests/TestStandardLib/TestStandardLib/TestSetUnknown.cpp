#include "BaseLib/GlobalMemory.h"
#include "TestLib/Assert.h"
#include "BaseLib/BufferedFile.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/Unknowns.h"
#include "StandardLib/SetUnknown.h"
#include "StandardLib/ArrayUnknown.h"
#include "UnknownTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetUnknownRemoveDuringIteration1(void)
{
	MemoryInit();
	UnknownsInit();

	CSetUnknown								cSet;
	CTestNamedUnknown*						pcTest1;
	CTestNamedUnknown*						pcTest2;
	CTestNamedUnknown*						pcTest3;
	size									iNum;
	CTestNamedUnknown*						pcTest;
	SIteratorTemplate<CTestNamedUnknown>	sIter;
	SSetIterator							sSetIter;

	cSet.Init(false);

	pcTest1 = cSet.Add<CTestNamedUnknown>();
	pcTest1->Init("The One");

	pcTest2 = cSet.Add<CTestNamedUnknown>();
	pcTest2->Init("Two");

	pcTest3 = UMalloc(CTestNamedUnknown);
	pcTest3->Init("There can be only three");
	cSet.Add(pcTest3);

	iNum = cSet.NumElements();
	AssertSize(3, iNum);
	cSet.StartIteration(&sSetIter, (CUnknown**)&pcTest);
	AssertString("The One", pcTest->GetName());
	cSet.Iterate(&sSetIter, (CUnknown**)&pcTest);
	AssertString("Two", pcTest->GetName());
	cSet.Iterate(&sSetIter, (CUnknown**)&pcTest);
	AssertString("There can be only three", pcTest->GetName());

	cSet.Remove(pcTest2);

	iNum = cSet.NumElements();
	AssertSize(2, iNum);
	cSet.StartIteration(&sSetIter, (CUnknown**)&pcTest);
	AssertString("The One", pcTest->GetName());
	cSet.Iterate(&sSetIter, (CUnknown**)&pcTest);
	AssertNotNull(pcTest);
	AssertString("There can be only three", pcTest->GetName());

	pcTest = gcUnknowns.StartIteration(&sIter);
	AssertString("The One", pcTest->GetName());
	pcTest = gcUnknowns.Iterate(&sIter);
	AssertString("There can be only three", pcTest->GetName());

	cSet.StartIteration(&sSetIter, (CUnknown**)&pcTest);
	cSet.RemoveDuringIteration(&sSetIter);
	cSet.Iterate(&sSetIter, (CUnknown**)&pcTest);
	cSet.RemoveDuringIteration(&sSetIter);

	iNum = cSet.NumElements();
	AssertSize(0, iNum);

	cSet.RemoveDuringIteration(&sSetIter);

	cSet.Kill();

	pcTest = gcUnknowns.StartIteration(&sIter);
	AssertNull(pcTest);

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetUnknownRemoveDuringIteration2(void)
{
	MemoryInit();
	UnknownsInit();

	CSetUnknown								cSet;
	CTestNamedUnknown*						pcTest1;
	CTestNamedUnknown*						pcTest2;
	CTestNamedUnknown*						pcTest3;
	size									iNum;
	CTestNamedUnknown*						pcTest;
	SIteratorTemplate<CTestNamedUnknown>	sIter;
	SSetIterator							sSetIter;
	bool									bExists;

	cSet.Init(false);

	pcTest1 = cSet.Add<CTestNamedUnknown>();
	pcTest1->Init("The One");

	pcTest2 = cSet.Add<CTestNamedUnknown>();
	pcTest2->Init("Two");

	pcTest3 = UMalloc(CTestNamedUnknown);
	pcTest3->Init("There can be only three");
	cSet.Add(pcTest3);

	iNum = cSet.NumElements();
	AssertSize(3, iNum);

	cSet.StartIteration(&sSetIter, NULL);
	cSet.RemoveDuringIteration(&sSetIter);
	cSet.Iterate(&sSetIter, NULL);
	cSet.RemoveDuringIteration(&sSetIter);
	cSet.Iterate(&sSetIter, NULL);
	cSet.RemoveDuringIteration(&sSetIter);

	iNum = cSet.NumElements();
	AssertSize(0, iNum);
	cSet.Iterate(&sSetIter, NULL);

	cSet.RemoveDuringIteration(&sSetIter);

	pcTest = gcUnknowns.StartIteration(&sIter);
	AssertNull(pcTest);

	cSet.Kill();

	cSet.Init(false);

	pcTest1 = cSet.Add<CTestNamedUnknown>();
	pcTest1->Init("The One");

	pcTest2 = cSet.Add<CTestNamedUnknown>();
	pcTest2->Init("Two");

	pcTest3 = UMalloc(CTestNamedUnknown);
	pcTest3->Init("There can be only three");
	cSet.Add(pcTest3);

	iNum = cSet.NumElements();
	AssertSize(3, iNum);

	bExists = cSet.StartIteration(&sSetIter, (CUnknown**)&pcTest);
	AssertString("The One", pcTest->GetName());

	cSet.RemoveDuringIteration(&sSetIter);
	bExists = cSet.Iterate(&sSetIter, (CUnknown**)&pcTest);
	AssertString("Two", pcTest->GetName());

	cSet.RemoveDuringIteration(&sSetIter);
	iNum = cSet.NumElements();
	AssertSize(1, iNum);
	bExists = cSet.Iterate(&sSetIter, (CUnknown**)&pcTest);
	AssertNotNull(pcTest);
	AssertString("There can be only three", pcTest->GetName());

	cSet.RemoveDuringIteration(&sSetIter);
	iNum = cSet.NumElements();
	AssertSize(0, iNum);
	bExists = cSet.Iterate(&sSetIter, (CUnknown**)&pcTest);
	AssertFalse(bExists);

	cSet.RemoveDuringIteration(&sSetIter);
	cSet.Kill();

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetUnknownCleanNulls(void)
{
	MemoryInit();
	UnknownsInit();

	CSetUnknown			cSet;
	CTestNamedUnknown*	apUnknowns[32];
	CChars				sz;
	int					i;
	SSetIterator		sSetIter;
	bool				bResult;

	cSet.Init(4, false);

	for (int i = 0; i < 32; i++)
	{
		apUnknowns[i] = cSet.Add<CTestNamedUnknown>();
		sz.Init();
		sz.Append(i);
		apUnknowns[i]->Init(sz.Text());
		sz.Kill();
	}

	cSet.SetChunkSize(4);

	AssertSize(32, cSet.NumElements());
	AssertSize(32, cSet.UnsafeNumElements());

	cSet.Remove(apUnknowns[0]);
	cSet.Remove(apUnknowns[2]);
	cSet.Remove(apUnknowns[3]);
	cSet.Remove(apUnknowns[7]);

	cSet.Remove(apUnknowns[17]);
	cSet.Remove(apUnknowns[18]);
	cSet.Remove(apUnknowns[19]);
	cSet.Remove(apUnknowns[23]);

	cSet.Remove(apUnknowns[26]);
	cSet.Remove(apUnknowns[27]);
	cSet.Remove(apUnknowns[28]);

	AssertSize(21, cSet.NumElements());
	AssertSize(32, cSet.UnsafeNumElements());
	
	AssertPointer(NULL,          cSet.UnsafeGet(0));
	AssertPointer(apUnknowns[1], cSet.UnsafeGet(1));
	AssertPointer(NULL,          cSet.UnsafeGet(2));
	AssertPointer(NULL,          cSet.UnsafeGet(3));
	AssertPointer(apUnknowns[4], cSet.UnsafeGet(4));
	AssertPointer(apUnknowns[5], cSet.UnsafeGet(5));
	AssertPointer(apUnknowns[6], cSet.UnsafeGet(6));
	AssertPointer(NULL,          cSet.UnsafeGet(7));
	AssertPointer(apUnknowns[8], cSet.UnsafeGet(8));
	AssertPointer(apUnknowns[9], cSet.UnsafeGet(9));
	AssertPointer(apUnknowns[10], cSet.UnsafeGet(10));
	AssertPointer(apUnknowns[11], cSet.UnsafeGet(11));
	AssertPointer(apUnknowns[12], cSet.UnsafeGet(12));
	AssertPointer(apUnknowns[13], cSet.UnsafeGet(13));
	AssertPointer(apUnknowns[14], cSet.UnsafeGet(14));
	AssertPointer(apUnknowns[15], cSet.UnsafeGet(15));
	AssertPointer(apUnknowns[16], cSet.UnsafeGet(16));
	AssertPointer(NULL,           cSet.UnsafeGet(17));
	AssertPointer(NULL,           cSet.UnsafeGet(18));
	AssertPointer(NULL,           cSet.UnsafeGet(19));
	AssertPointer(apUnknowns[20], cSet.UnsafeGet(20));
	AssertPointer(apUnknowns[21], cSet.UnsafeGet(21));
	AssertPointer(apUnknowns[22], cSet.UnsafeGet(22));
	AssertPointer(NULL,           cSet.UnsafeGet(23));
	AssertPointer(apUnknowns[24], cSet.UnsafeGet(24));
	AssertPointer(apUnknowns[25], cSet.UnsafeGet(25));
	AssertPointer(NULL,           cSet.UnsafeGet(26));
	AssertPointer(NULL,           cSet.UnsafeGet(27));
	AssertPointer(NULL,           cSet.UnsafeGet(28));
	AssertPointer(apUnknowns[29], cSet.UnsafeGet(29));
	AssertPointer(apUnknowns[30], cSet.UnsafeGet(30));
	AssertPointer(apUnknowns[31], cSet.UnsafeGet(31));

	//CleanNullsIfNecessary happens here.
	cSet.Remove(apUnknowns[29]);
	AssertSize(20, cSet.NumElements());
	AssertSize(20, cSet.UnsafeNumElements());

	AssertPointer(apUnknowns[1], cSet.UnsafeGet(0));
	AssertPointer(apUnknowns[4], cSet.UnsafeGet(1));
	AssertPointer(apUnknowns[5], cSet.UnsafeGet(2));
	AssertPointer(apUnknowns[6], cSet.UnsafeGet(3));
	AssertPointer(apUnknowns[8], cSet.UnsafeGet(4));
	AssertPointer(apUnknowns[9], cSet.UnsafeGet(5));
	AssertPointer(apUnknowns[10], cSet.UnsafeGet(6));
	AssertPointer(apUnknowns[11], cSet.UnsafeGet(7));
	AssertPointer(apUnknowns[12], cSet.UnsafeGet(8));
	AssertPointer(apUnknowns[13], cSet.UnsafeGet(9));
	AssertPointer(apUnknowns[14], cSet.UnsafeGet(10));
	AssertPointer(apUnknowns[15], cSet.UnsafeGet(11));
	AssertPointer(apUnknowns[16], cSet.UnsafeGet(12));
	AssertPointer(apUnknowns[20], cSet.UnsafeGet(13));
	AssertPointer(apUnknowns[21], cSet.UnsafeGet(14));
	AssertPointer(apUnknowns[22], cSet.UnsafeGet(15));
	AssertPointer(apUnknowns[24], cSet.UnsafeGet(16));
	AssertPointer(apUnknowns[25], cSet.UnsafeGet(17));
	AssertPointer(apUnknowns[30], cSet.UnsafeGet(18));
	AssertPointer(apUnknowns[31], cSet.UnsafeGet(19));

	cSet.StartIteration(&sSetIter, NULL);
	for (i = 0; i < 19; i++)
	{
		cSet.RemoveDuringIteration(&sSetIter);
		cSet.Iterate(&sSetIter, NULL);
	}

	AssertSize(1, cSet.NumElements());

	bResult = cSet.Remove(apUnknowns[31]);
	AssertTrue(bResult);

	AssertSize(0, cSet.NumElements());

	cSet.Kill();

	UnknownsKill();
	MemoryKill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetUnknownSorted(size uiNumElements)
{
	MemoryInit();
	UnknownsInit();

	CSetUnknown								cSet;
	size									iNum;
	CTestNamedUnknown*						pcTest;
	bool									bExists;
	CArrayUnknownPtr						apTests;

	cSet.Init(true, true, false, true, 1, true);  //You need some way of disallowing null.
	apTests.Init();

	for (iNum = 0; iNum < uiNumElements; iNum++)
	{
		pcTest = cSet.Add<CTestNamedUnknown>();
		pcTest->Init(SizeToString(iNum));
		apTests.Add(pcTest);
	}

	iNum = cSet.NumElements();
	AssertSize(uiNumElements, iNum);

	AssertTrue(cSet.IsSorted());
	AssertTrue(cSet.CalculateIsSorted());

	bExists = true;
	for (iNum = 0; iNum < uiNumElements; iNum++)
	{
		pcTest = *((CTestNamedUnknown**)apTests.Get(iNum));
		bExists = cSet.Contains(pcTest);
		if (!bExists)
		{
			AssertTrue(bExists);
		}
	}
	AssertTrue(bExists);

	for (iNum = 0; iNum < uiNumElements; iNum++)
	{
		if (iNum % 2 == 1)
		{
			pcTest = *((CTestNamedUnknown**)apTests.Get(iNum));
			cSet.Remove(pcTest);
		}
	}

	iNum = cSet.NumElements();
	AssertSize(uiNumElements / 2, iNum);

	AssertTrue(cSet.IsSorted());
	AssertTrue(cSet.CalculateIsSorted());

	bExists = true;
	for (iNum = 0; iNum < uiNumElements; iNum++)
	{
		pcTest = *((CTestNamedUnknown**)apTests.Get(iNum));
		bExists = cSet.Contains(pcTest);
		if (iNum % 2 == 0)
		{
			if (!bExists)
			{
				AssertTrue(bExists);
			}
		}
		else
		{
			if (bExists)
			{
				AssertFalse(bExists);
			}
		}
	}

	apTests.Kill();

	cSet.Kill();

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetUnknownLoadAfterRemove(void)
{
	MemoryInit();
	UnknownsInit();

	CSetUnknown			cSet;
	CChunkFile			cFile;
	CFileUtil			cFileUtil;
	CTestUnknown*		pcTest;
	CTestUnknown*		pcRemoved;
	SSetIterator		sIter;
	CUnknown*			pcUnknown;
	int					iSum;
	size				uiCount;
	bool				bExists;

	gcUnknowns.AddConstructor<CTestUnknown>();

	cFileUtil.RemoveDir("Output" _FS_ "SetUnknownRemoved");
	cFileUtil.TouchDir("Output" _FS_ "SetUnknownRemoved");

	//Removing from a set leaves a NULL in its place.  Saving must not count it as an element.
	cSet.Init(false);
	pcTest = cSet.Add<CTestUnknown>();
	pcTest->Init(1);
	pcRemoved = cSet.Add<CTestUnknown>();
	pcRemoved->Init(10);
	pcTest = cSet.Add<CTestUnknown>();
	pcTest->Init(100);
	AssertSize(1, cSet.Remove(pcRemoved));
	AssertSize(2, cSet.NumElements());
	AssertSize(2, gcUnknowns.NumElements());

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "SetUnknownRemoved" _FS_ "Set.dat")));
	AssertTrue(cFile.WriteOpen());
	AssertTrue(cSet.Save(&cFile));
	AssertTrue(cFile.WriteClose());
	cFile.Kill();
	cSet.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "SetUnknownRemoved" _FS_ "Set.dat")));
	AssertTrue(cFile.ReadOpen());
	AssertTrue(cSet.Load(&cFile));
	AssertTrue(cFile.ReadClose());
	cFile.Kill();

	AssertSize(2, cSet.NumElements());
	AssertSize(2, gcUnknowns.NumElements());
	iSum = 0;
	uiCount = 0;
	bExists = cSet.StartIteration(&sIter, &pcUnknown);
	while (bExists)
	{
		iSum += ((CTestUnknown*)pcUnknown)->miCount;
		uiCount++;
		bExists = cSet.Iterate(&sIter, &pcUnknown);
	}
	AssertSize(2, uiCount);
	AssertInt(101, iSum);

	cSet.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	cFileUtil.RemoveDir("Output" _FS_ "SetUnknownRemoved");

	UnknownsKill();
	MemoryKill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetUnknownAddAllWithNulls(void)
{
	MemoryInit();
	UnknownsInit();

	CSetUnknown			cSet;
	CArrayUnknown		cArray;
	CTestUnknown*		pcTest;
	SSetIterator		sIter;
	CUnknown*			pcUnknown;
	size				uiCount;
	bool				bExists;

	//The array keeps its NULL as an element; the set ignores NULLs, so it must not count it.
	cArray.Init();
	cArray.KillElements(false);
	pcTest = cArray.Add<CTestUnknown>();
	pcTest->Init(1);
	AssertTrue(cArray.Add((CUnknown*)NULL));
	pcTest = cArray.Add<CTestUnknown>();
	pcTest->Init(2);
	AssertSize(3, cArray.NumElements());

	cSet.Init(false);
	AssertTrue(cSet.AddAll(&cArray));
	AssertSize(2, cSet.NumElements());

	uiCount = 0;
	bExists = cSet.StartIteration(&sIter, &pcUnknown);
	while (bExists)
	{
		uiCount++;
		bExists = cSet.Iterate(&sIter, &pcUnknown);
	}
	AssertSize(2, uiCount);

	cArray.Kill();
	cSet.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetUnknown(void)
{
	BeginTests();

	TestSetUnknownRemoveDuringIteration1();
	TestSetUnknownRemoveDuringIteration2();
	TestSetUnknownCleanNulls();
	TestSetUnknownLoadAfterRemove();
	TestSetUnknownAddAllWithNulls();
	TestSetUnknownSorted(4);
	TestSetUnknownSorted(10000);

	TestStatistics();
}


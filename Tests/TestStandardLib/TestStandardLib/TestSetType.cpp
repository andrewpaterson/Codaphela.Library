#include "BaseLib/GlobalMemory.h"
#include "TestLib/Assert.h"
#include "StandardLib/Unknowns.h"
#include "StandardLib/SetType.h"
#include "UnknownTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetTypeInstantiation(void)
{
	MemoryInit();
	UnknownsInit();

	CSetType<CTestUnknown>	cSet;
	CTestUnknown*			pcTest1;
	CTestUnknown*			pcTest2;
	CTestUnknown*			pcTest;
	size					iSize;
	const char*				szName;
	SSetIterator			sIter;
	int						iCount;
	bool					bExists;

	cSet.Init(false);

	pcTest1 = cSet.Add();
	pcTest1->Init();
	pcTest2 = UMalloc(CTestUnknown);
	pcTest2->Init();
	cSet.Add(pcTest2);

	iCount = 0;
	bExists = cSet.StartIteration(&sIter, &pcTest);
	while (bExists)
	{
		iCount++;
		bExists = cSet.Iterate(&sIter, &pcTest);
	}
	AssertInt(2, iCount);
	AssertSize(2, cSet.NumElements());

	//This test is not correct.  The order is not guaranteed.
	//However it necessary to make the template compile.
	AssertTrue(cSet.Contains(pcTest1));
	AssertTrue(cSet.Contains(pcTest2));

	bExists = cSet.StartIteration(&sIter, &pcTest);
	AssertPointer(pcTest1, pcTest);
	bExists = cSet.Iterate(&sIter, &pcTest);
	AssertPointer(pcTest2, pcTest);

	bExists = cSet.StartIteration(&sIter, &pcTest);
	cSet.RemoveDuringIteration(&sIter);
	bExists = cSet.Iterate(&sIter, &pcTest);
	AssertPointer(pcTest2, pcTest);

	bExists = cSet.Iterate(&sIter, &pcTest);
	AssertFalse(bExists);

	AssertSize(1, cSet.NumElements());

	cSet.Remove(pcTest2);
	AssertSize(0, cSet.NumElements());

	iSize = cSet.ClassSize();
#ifdef BUILD_64
	AssertSize(104, iSize);
#elif defined(SIZE_16)
	AssertSize(44, iSize);
#else
	AssertSize(52, iSize);
#endif // BUILD_64

	szName = cSet.ClassName();
	AssertString("CSetType", szName);

	cSet.Kill();

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetTypeAddAll(void)
{
	MemoryInit();
	UnknownsInit();

	CSetType<CTestNamedUnknown>	cSet;
	int							i;
	CTestNamedUnknown*			pcTest;
	CSetType<CTestNamedUnknown>	cDest;
	CChars						sz;
	CTestNamedUnknown*			pcTest25;

	cSet.Init(false);
	pcTest25 = NULL;
	for (i = 0; i < 50; i++)
	{
		pcTest = UMalloc(CTestNamedUnknown);

		sz.Init(":");
		sz.Append(i);
		pcTest->Init(sz.Text());
		sz.Kill();
		cSet.Add(pcTest);
		if (i == 25)
		{
			pcTest25 = pcTest;
		}
	}

	cSet.Remove(pcTest25);

	cDest.Init(false);
	cDest.AddAll(&cSet);
	cSet.KillElements(false);
	cSet.Kill();

	AssertSize(49, cDest.NumElements());

	for (i = 0; i < 25; i++)
	{
		pcTest = (CTestNamedUnknown*)cDest.UnsafeGet(i);
		sz.Init(":");
		sz.Append(i);
		AssertString(sz.Text(), pcTest->mszName.Text());
		sz.Kill();
	}

	pcTest = (CTestNamedUnknown*)cDest.UnsafeGet(i);
	AssertNull(pcTest);

	for (i = 26; i < 50; i++)
	{
		pcTest = (CTestNamedUnknown*)cDest.UnsafeGet(i);
		sz.Init(":");
		sz.Append(i);
		AssertString(sz.Text(), pcTest->mszName.Text());
		sz.Kill();
	}

	cDest.Kill();

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSetType(void)
{
	BeginTests();

	TestSetTypeInstantiation();
	TestSetTypeAddAll();

	TestStatistics();
}


#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/BufferedFile.h"
#include "BaseLib/ChunkFile.h"
#include "TestLib/Assert.h"
#include "StandardLib/Unknowns.h"
#include "StandardLib/SetType.h"
#include "UnknownTestClasses.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/Logger.h"


void StartCapturingLog(CMemoryFile* pcMemory, SLogConfig* psLogConfig);  //TestObjectFreeOrdering.cpp
void StopCapturingLog(CMemoryFile* pcMemory, SLogConfig* psLogConfig, char* szLogged, size uiSize);


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
	AssertSize(96, iSize);
#elif defined(SIZE_16)
	AssertSize(40, iSize);
#else
	AssertSize(48, iSize);
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
	CMemoryFile					cLog;
	SLogConfig					sLogConfig;
	char						szLogged[512];

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

	//Both kill their elements: refused until the source gives them up.
	StartCapturingLog(&cLog, &sLogConfig);
	AssertFalse(cDest.AddAll(&cSet));
	StopCapturingLog(&cLog, &sLogConfig, szLogged, sizeof(szLogged));
	AssertNotNull(strstr(szLogged, "Both arrays kill their elements."));
	AssertSize(0, cDest.NumElements());

	cSet.KillElements(false);
	AssertTrue(cDest.AddAll(&cSet));
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
// A set leaves a hole where an element was removed.  It is saved and
// loaded as a hole.
//
//////////////////////////////////////////////////////////////////////////
void TestSetTypeSaveWithHole(void)
{
	MemoryInit();
	UnknownsInit();

	CSetType<CTestUnknown>	cSet;
	CTestUnknown*			pcTest;
	CTestUnknown*			pcTwo;
	CChunkFile				cFile;
	CFileUtil				cFileUtil;
	SSetIterator			sIter;
	bool					bExists;

	cFileUtil.RemoveDir("Output" _FS_ "SetTypeHole");
	cFileUtil.TouchDir("Output" _FS_ "SetTypeHole");

	cSet.Init(false);
	cSet.Add()->Init(1);
	pcTwo = cSet.Add();
	pcTwo->Init(2);
	cSet.Add()->Init(3);
	AssertSize(1, cSet.Remove(pcTwo));
	AssertSize(2, cSet.NumElements());

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "SetTypeHole" _FS_ "Set.dat")));
	AssertTrue(cFile.WriteOpen());
	AssertTrue(cSet.Save(&cFile));
	AssertTrue(cFile.WriteClose());
	cFile.Kill();
	cSet.Kill();
	AssertSize(0, gcUnknowns.NumElements());

	cFile.Init(BufferedFile(DiskFile("Output" _FS_ "SetTypeHole" _FS_ "Set.dat")));
	AssertTrue(cFile.ReadOpen());
	AssertTrue(cSet.Load(&cFile));
	AssertTrue(cFile.ReadClose());
	cFile.Kill();

	AssertSize(2, cSet.NumElements());
	AssertSize(2, gcUnknowns.NumElements());
	bExists = cSet.StartIteration(&sIter, &pcTest);
	AssertTrue(bExists);
	AssertInt((int32)1, pcTest->miCount);
	bExists = cSet.Iterate(&sIter, &pcTest);
	AssertTrue(bExists);
	AssertInt((int32)3, pcTest->miCount);
	bExists = cSet.Iterate(&sIter, &pcTest);
	AssertFalse(bExists);

	cSet.Kill();
	AssertSize(0, gcUnknowns.NumElements());
	cFileUtil.RemoveDir("Output" _FS_ "SetTypeHole");

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Removing by value says how many were removed: none for NULL (a set
// ignores NULLs) or for an element it doesn't have.
//
//////////////////////////////////////////////////////////////////////////
void TestSetTypeRemoveCounts(void)
{
	MemoryInit();
	UnknownsInit();

	CSetType<CTestUnknown>	cSet;
	CTestUnknown*			pcIn;
	CTestUnknown*			pcOut;

	cSet.Init(false);
	pcIn = cSet.Add();
	pcIn->Init(1);
	pcOut = UMalloc(CTestUnknown);
	pcOut->Init(2);

	AssertSize(0, cSet.Remove((CTestUnknown*)NULL));
	AssertSize(0, cSet.Remove(pcOut));
	AssertSize(1, cSet.Remove(pcIn));
	AssertSize(0, cSet.NumElements());

	pcOut->Kill();
	cSet.Kill();

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Adding says whether the element went in, removing while iterating says
// whether it came out, and removing by value counts every copy.
//
//////////////////////////////////////////////////////////////////////////
void TestSetTypeResults(bool bSortPointers)
{
	MemoryInit();
	UnknownsInit();

	CSetType<CTestUnknown>	cSet;
	CTestUnknown*			pcOne;
	CTestUnknown*			pcTwo;
	CTestUnknown*			pcFound;
	SSetIterator			sIter;

	cSet.Init(bSortPointers);
	cSet.KillElements(false);
	pcOne = UMalloc(CTestUnknown);
	pcOne->Init(1);
	pcTwo = UMalloc(CTestUnknown);
	pcTwo->Init(2);

	AssertTrue(cSet.Add(pcOne));
	AssertFalse(cSet.Add((CTestUnknown*)NULL));
	AssertSize(1, cSet.NumElements());

	AssertTrue(cSet.Add(pcTwo));
	AssertTrue(cSet.Add(pcOne));
	AssertTrue(cSet.Add(pcOne));
	AssertSize(4, cSet.NumElements());
	AssertSize(3, cSet.Remove(pcOne));
	AssertSize(1, cSet.NumElements());
	AssertFalse(cSet.Contains(pcOne));
	AssertTrue(cSet.Contains(pcTwo));

	AssertTrue(cSet.StartIteration(&sIter, &pcFound));
	AssertPointer(pcTwo, pcFound);
	AssertTrue(cSet.RemoveDuringIteration(&sIter));
	AssertFalse(cSet.RemoveDuringIteration(&sIter));
	AssertSize(0, cSet.NumElements());

	pcOne->Kill();
	pcTwo->Kill();
	cSet.Kill();
	AssertSize(0, gcUnknowns.NumElements());

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
	TestSetTypeSaveWithHole();
	TestSetTypeRemoveCounts();
	TestSetTypeResults(false);
	TestSetTypeResults(true);

	TestStatistics();
}


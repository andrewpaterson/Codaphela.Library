#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "StandardLib/Objects.h"
#include "TestLib/Assert.h"
#include "ObjectTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsCompilation(void)
{
	if (false)  //This is intentional.  This code must only be compiled (testing template compilation) not run.
	{
		CObjects			cObjects;
		Ptr<CTestObject>	cTestPtr;

		cTestPtr = cObjects.Malloc<CTestObject>();
		cTestPtr = cObjects.Get<CTestObject>("Invalid Name");
		cTestPtr = cObjects.Get<CTestObject>(INVALID_O_INDEX);
	}
	AssertTrue(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectFreeing(void)
{
	Ptr<CTestObject>			pTest1;
	Ptr<CTestObject>			pTest2;
	STestObjectFreedNotifier	sFreedNotifier1;
	STestObjectFreedNotifier	sFreedNotifier2;
	CCodabase*					pcDatabase;
	CSequence*					pcSequence;
	char						szDirectory[] = "Output" _FS_ "TestObjectFreeing";
	CFileUtil					cFileUtil;
			
	cFileUtil.RemoveDir(szDirectory);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);

	pTest1 = OMalloc<CTestObject>(&sFreedNotifier1);
	pTest2 = OMalloc<CTestObject>(&sFreedNotifier2);

	AssertNotNull(&pTest1);
	AssertNotNull(&pTest2);

	AssertLong(2, gcObjects.NumMemoryIndexes());
	AssertLong(0, pcDatabase->NumIndicesCached());

	pTest1->Kill();
	AssertLong(1, gcObjects.NumMemoryIndexes());
	AssertLong(0, pcDatabase->NumIndicesCached());
	AssertTrue(sFreedNotifier1.bFreed);

	pTest2->Kill();
	AssertLong(0, gcObjects.NumMemoryIndexes());
	AssertLong(0, pcDatabase->NumIndicesCached());
	AssertTrue(sFreedNotifier2.bFreed);

	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsSimple(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestObjectsCompilation();
	TestObjectFreeing();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}


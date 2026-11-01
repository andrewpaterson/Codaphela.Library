#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "BaseLib/ValueIndexedDataConfig.h"
#include "BaseLib/ValueNamedIndexesConfig.h"
#include "BaseLib/NamedIndexedHeader.h"
#include "StandardLib/Objects.h"
#include "TestLib/Assert.h"
#include "ObjectTestClasses.h"
#include "ChunkFileObjectWriterTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsEvictVsKillUserEvictAfterSave(EIndexWriteThrough eWriteThrough)
{
	CFileUtil					cFileUtil;
	char						szDirectory[] = "Output" _FS_ "ObjectsUserFree";
	CCodabase*					pcDatabase;
	CSequence*					pcSequence;
	Ptr<CTestObject>			pTest1a;
	Ptr<CTestObject>			pTest1b;
	STestObjectFreedNotifier	sFreeNotifier1a;
	STestObjectFreedNotifier	sFreeNotifier1b;
	Ptr<CRoot>					pRoot;

	cFileUtil.RemoveDir(szDirectory);
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, eWriteThrough, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);

	pTest1a = OMalloc<CTestObject>(&sFreeNotifier1a);
	pRoot = ORoot();
	pRoot->Add(pTest1a);
	pTest1b = OMalloc<CTestObject>(&sFreeNotifier1b);
	pTest1a->mpTest = pTest1b;
	AssertLong(eWriteThrough == IWT_No ? 0LL : 3LL , pcDatabase->NumIndices());
	AssertLong(4, gcObjects.NumMemoryIndexes());

	AssertInt((int32)0x07070707, pTest1a->mi);

	pTest1a->Flush();

	AssertLong(eWriteThrough == IWT_No ? 1LL : 4LL, pcDatabase->NumIndices());
	AssertLong(4, gcObjects.NumMemoryIndexes());

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsEvictVsKill(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestObjectsEvictVsKillUserEvictAfterSave(IWT_No);
	TestObjectsEvictVsKillUserEvictAfterSave(IWT_Yes);

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}


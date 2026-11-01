#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "StandardLib/Objects.h"
#include "StandardLib/Pointer.h"
#include "StandardLib/Root.h"
#include "StandardLib/ArrayObject.h"
#include "TestLib/Assert.h"
#include "NamedObjectTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedObjectAddConstructors(void)
{
	gcObjects.AddConstructor<CTestNamedObject>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedObjectSize(void)
{
#ifdef BUILD_64
	AssertSize(344, sizeof(CObject));
#elif defined(SIZE_16)
	AssertSize(160, sizeof(CObject));
#else
	AssertSize(184, sizeof(CObject));
#endif // BUILD_64
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedObjectName(void)
{
	CFileUtil				cFileUtil;
	Ptr<CTestNamedObject>	pNamed1;
	Ptr<CTestNamedObject>	pNamed2;
	Ptr<CTestNamedObject>	pNamed3;
	Ptr<CTestNamedObject>	pResult1;
	Ptr<CTestNamedObject>	pResult2;
	CPointer				pResult3;
	Ptr<CRoot>				pRoot;
	Ptr<CSetObject>			pSet;
	CSetObject*				pcSet;
	OIndex					oiSet;
	OIndex					oiNamed3;
	CCodabase*				pcDatabase;
	CSequence*				pcSequence;
	char					szDirectory[] = "Output" _FS_ "NamedObject";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.TouchDir(szDirectory);


	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	TestNamedObjectAddConstructors(); //This is only important if an object is being loaded.

	pNamed1 = ONMalloc<CTestNamedObject>("Frank", 1);

	pNamed2 = ONMalloc<CTestNamedObject>("Zappa", 2);
	pNamed2->mpNamedTest1 = pNamed1;

	pNamed3 = OMalloc<CTestNamedObject>(3);
	pNamed3->mpNamedTest1 = pNamed1;
	pNamed3->mpNamedTest2 = pNamed2;
	oiNamed3 = pNamed3->GetIndex();
	AssertLong(3, oiNamed3);

	pRoot = ORoot();
	pRoot->Add(pNamed3);
	
	AssertLong(3, gcObjects.NumMemoryNames());
	AssertLong(5, gcObjects.NumMemoryIndexes());

	pResult1 = gcObjects.Get("Frank");
	AssertNotNull(pResult1.Object());
	AssertLong(1, pResult1.GetIndex());

	pResult2 = gcObjects.Get("Zappa");
	AssertNotNull(pResult2.Object());

	pResult3 = gcObjects.Get("");
	AssertNull(pResult3.Object());

	pcSet = pRoot->TestGetSet();
	oiSet = pcSet->GetIndex();

	AssertTrue(pRoot.IsDirty());
	AssertTrue(pcSet->IsDirty());

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcSequence);
	SafeKill(pcDatabase);
	ObjectsKill(false);

	AssertNull(pNamed1.Object());
	AssertNull(pNamed2.Object());
	AssertNull(pNamed3.Object());
	AssertNull(pRoot.Object());
	AssertNull(pSet.Object());


	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_Yes, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);

	AssertLong(3, pcDatabase->NumNames());
	AssertLong(5, pcDatabase->NumIndices());
	AssertLong(0, gcObjects.NumMemoryNames());
	AssertLong(0, gcObjects.NumMemoryIndexes());

	pSet = gcObjects.Get(oiSet);
	AssertLong(3, gcObjects.NumMemoryIndexes());
	AssertString("CSetObject", pSet.ClassName());

	AssertTrue(pSet.IsNotNull());
	AssertFalse(pSet.IsHollow());
	AssertInt(1, pSet.GetDistToRoot());

	AssertSize(1, pSet->NumElements());
	pResult3 = pSet->UnsafeGet(0);
	AssertTrue(pResult3.IsNotNull());
	AssertLong(oiNamed3, pResult3.GetIndex());
	AssertTrue(pResult3.IsHollow());
	AssertInt(UNATTACHED_DIST_TO_ROOT, pResult3.GetDistToRoot());

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedOjectKill(void)
{
	Ptr<CTestNamedObject>	pNamedObject;

	ObjectsInit();

	pNamedObject = ONMalloc<CTestNamedObject>("This is my Name", 7);


	//Trying to test that FreeIdentifiers is called and that mon is freed.
	pNamedObject->Kill();

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedOjectReuse(EIndexWriteThrough eWriteThrough)
{
	CFileUtil				cFileUtil;
	Ptr<CTestNamedObject>	pNamed1;
	Ptr<CTestNamedObject>	pNamed2;
	Ptr<CTestNamedObject>	pNamed3;
	Ptr<CRoot>				pRoot;
	CCodabase*				pcDatabase;
	CSequence*				pcSequence;
	char					szDirectory[] = "Output" _FS_ "NamedObjectReuse";
	SLogConfig				sLogConfig;

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.TouchDir(szDirectory);


	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, eWriteThrough, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);

	pNamed1 = ONMalloc<CTestNamedObject>("Sister Two", 1);
	AssertLong(1, pNamed1.GetIndex());

	pNamed2 = ONMalloc<CTestNamedObject>("Office Block", 2);
	pNamed2->mpNamedTest1 = pNamed1;

	pRoot = ORoot();
	pRoot->Add(pNamed1);

	AssertLong(3, gcObjects.NumMemoryNames());
	AssertLong(4, gcObjects.NumMemoryIndexes());

	AssertLong(3LL, pRoot.GetIndex());
	AssertLong(1LL, pNamed1.GetIndex());
	AssertLong(2LL, pNamed2.GetIndex());


	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcSequence);
	SafeKill(pcDatabase);
	ObjectsKill(false);

	AssertNull(pNamed1.Object());
	AssertNull(pNamed2.Object());
	AssertNull(pRoot.Object());


	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, eWriteThrough, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);

	pNamed1 = gcObjects.Get("Sister Two");
	AssertLong(1, pNamed1.GetIndex());

	sLogConfig = gcLogger.SetSilent();
	pNamed3 = ONMalloc<CTestNamedObject>("Sister Two", 3);
	gcLogger.SetConfig(&sLogConfig);

	AssertTrue(pNamed3.IsNull());
	AssertLong(1, pNamed1.GetIndex());

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcSequence);
	SafeKill(pcDatabase);
	ObjectsKill(false);


	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNamedObject(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestNamedObjectSize();
	TestNamedObjectName();
	TestNamedOjectKill();
	TestNamedOjectReuse(IWT_No);
	TestNamedOjectReuse(IWT_Yes);

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}


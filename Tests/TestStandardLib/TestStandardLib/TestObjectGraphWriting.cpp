#include "BaseLib/GlobalMemory.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/LogToMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "StandardLib/Objects.h"
#include "StandardLib/String.h"
#include "TestLib/Assert.h"
#include "NamedObjectTestClasses.h"
#include "ObjectTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectGraphWritingAddConstructors(void)
{
	gcObjects.AddConstructor<CTestObject>();
	gcObjects.AddConstructor<CTestTriPointerObject>();
	gcObjects.AddConstructor<CTestSaveableObject1>();
	gcObjects.AddConstructor<CTestSaveableObject2>();
	gcObjects.AddConstructor<CTestNamedObject>();
	gcObjects.AddConstructor<CTestNamedObjectSmall>();
	gcObjects.AddConstructor<CTestNamedObjectWithEmbedded>();
	gcObjects.AddConstructor<CTestObjectWithFields>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectGraphWritingStackIsNotWritten(void)
{
	Ptr<CTestSaveableObject2>	pObj0;
	Ptr<CTestSaveableObject1>	pObj1;
	Ptr<CTestSaveableObject2>	pObj2;
	Ptr<CTestSaveableObject1>	pObj3;
	Ptr<CRoot>					pRoot;
	OIndex						oi0;
	OIndex						oi1;
	OIndex						oi2;
	OIndex						oi3;
	CCodabase*					pcDatabase;
	CSequence*					pcSequence;
	CFileUtil					cFileUtil;
	char						szDirectory[] = "Output" _FS_ "TestObjectGraphWriting" _FS_ "Database1";

	cFileUtil.RemoveDir(szDirectory);
	MemoryInit();
	TypesInit();
	DataIOInit();

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	TestObjectGraphWritingAddConstructors();

	pRoot = ORoot();
	pObj0 = OMalloc<CTestSaveableObject2>("Field 0");
	pObj1 = OMalloc<CTestSaveableObject1>();
	pObj2 = OMalloc<CTestSaveableObject2>("Field 2");
	pObj3 = OMalloc<CTestSaveableObject1>();

	oi0 = pObj0.GetIndex();
	oi1 = pObj1.GetIndex();
	oi2 = pObj2.GetIndex();
	oi3 = pObj3.GetIndex();

	pRoot->Add(pObj1);

	pObj1->mpObject = pObj2;
	pObj0->mp1 = pObj1;
	pObj0->mp2 = pObj3;

//
//    pObj2
//     |
//     |
//     |
//    pObj1     pObj3 
//     |  \      /
//     |   \    / 
//     |    \  /
//     |    pObj0  
//    ...      .
//   pRoot      .
//

	AssertTrue(pObj0.IsDirty());
	AssertTrue(pObj1.IsDirty());
	AssertTrue(pObj2.IsDirty());
	AssertTrue(pObj3.IsDirty());

	ObjectsFlush();
	AssertTrue(pObj0.IsDirty());
	AssertFalse(pObj1.IsDirty());
	AssertFalse(pObj2.IsDirty());
	AssertTrue(pObj3.IsDirty());


	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	pObj0.Kill();
	pObj1.Kill();
	pObj2.Kill();
	pObj3.Kill();
	pRoot = NULL;

	ObjectsKill();

	AssertNull(&pObj0);
	AssertNull(&pObj1);
	AssertNull(&pObj2);
	AssertNull(&pObj3);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);

	pRoot = ORoot();
	pObj0 = gcObjects.Get(oi0);
	pObj1 = gcObjects.Get(oi1);
	pObj2 = gcObjects.Get(oi2);
	pObj3 = gcObjects.Get(oi3);

	AssertNull(&pObj0);
	AssertNotNull(&pObj1);
	AssertNotNull(&pObj2);
	AssertNull(&pObj3);

	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	pObj0 = NULL;
	pObj1 = NULL;
	pObj2 = NULL;
	pObj3 = NULL;
	pRoot = NULL;
	
	ObjectsKill();

	DataIOKill();
	TypesKill();
	MemoryKill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectGraphWriting(void)
{
	BeginTests();

	TestObjectGraphWritingStackIsNotWritten();

	TestStatistics();
}


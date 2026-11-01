#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ExternalObjectSerialiser.h"
#include "StandardLib/MultiFileObjectWriter.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"
#include "ObjectTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectGraphSerialiserReachability(void)
{
	CFileUtil					cFileUtil;
	Ptr<CTestSaveableObject1>	pTest1a;
	Ptr<CTestSaveableObject1>	pTest1b;
	Ptr<CTestSaveableObject1>	pTest1c;
	Ptr<CTestSaveableObject2>	pTest2a;
	Ptr<CTestSaveableObject2>	pTest2b;
	CExternalObjectSerialiser	cGraphSerialiser;
	CMultiFileObjectWriter		cSimpleWriter;
	char						szDirectory[] = "Output" _FS_ "GraphWriter" _FS_ "Data";
	bool						bResult;

	cFileUtil.RemoveDir(szDirectory);
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	pTest1a = ONMalloc<CTestSaveableObject1>("Test1/OneA");
	pTest1a->mszString.Append("I'm a real string now.");
	pTest1a->miInt = 19;

	pTest1b = ONMalloc<CTestSaveableObject1>("Test1/OneB");
	pTest1b->mszString.Append("More Testing!");
	pTest1b->miInt = 2;

	pTest1c = ONMalloc<CTestSaveableObject1>("Test1/Miscellaneous/OneC");
	pTest1c->mszString.Append("Ur-Quan");
	pTest1c->miInt = -1;

	pTest2a = ONMalloc<CTestSaveableObject2>("Test2/TwoAye", "Hello");

	pTest2b = ONMalloc<CTestSaveableObject2>("Test2/TowBee", "World");

	pTest2a->mp1 = pTest1a;
	pTest2a->mp2 = pTest2b;
	pTest1a->mpObject = pTest1b;
	pTest2b->mp1 = pTest1c;
	pTest2b->mp2 = pTest1b;

#ifdef BUILD_64
	AssertLayoutSize(424, sizeof(CTestSaveableObject1));  //If these sizes change the input test files will be wrong if the serialisation has changed.
#elif defined(__W65C816__)
	AssertLayoutSize(192, sizeof(CTestSaveableObject1));
#else
	AssertLayoutSize(224, sizeof(CTestSaveableObject1));  //If these sizes change the input test files will be wrong if the serialisation has changed.
#endif // BUILD_64
#ifdef BUILD_64
	AssertLayoutSize(432, sizeof(CTestSaveableObject2));
#elif defined(__W65C816__)
	AssertLayoutSize(200, sizeof(CTestSaveableObject2));
#else
	AssertLayoutSize(232, sizeof(CTestSaveableObject2));
#endif // BUILD_64

	cSimpleWriter.Init(szDirectory, "");
	cGraphSerialiser.Init(&cSimpleWriter);
	bResult = cGraphSerialiser.Write(&pTest2a);
	AssertTrue(bResult);
	cGraphSerialiser.Kill();
	cSimpleWriter.Kill();

	AssertTrue(pTest1a->mbSaved);
	AssertTrue(pTest1b->mbSaved);
	AssertTrue(pTest1c->mbSaved);
	AssertTrue(pTest2a->mbSaved);
	AssertTrue(pTest2b->mbSaved);

	AssertFile("Input" _FS_ "OneA.DRG", "Output" _FS_ "GraphWriter" _FS_ "Data" _FS_ "Test1" _FS_ "OneA.DRG");
	AssertFile("Input" _FS_ "OneB.DRG", "Output" _FS_ "GraphWriter" _FS_ "Data" _FS_ "Test1" _FS_ "OneB.DRG");
	AssertFile("Input" _FS_ "OneC.DRG", "Output" _FS_ "GraphWriter" _FS_ "Data" _FS_ "Test1" _FS_ "Miscellaneous" _FS_ "OneC.DRG");
	AssertFile("Input" _FS_ "TwoAye.DRG", "Output" _FS_ "GraphWriter" _FS_ "Data" _FS_ "Test2" _FS_ "TwoAye.DRG");
	AssertFile("Input" _FS_ "TowBee.DRG", "Output" _FS_ "GraphWriter" _FS_ "Data" _FS_ "Test2" _FS_ "TowBee.DRG");

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectGraphSerialiser(void)
{
	MemoryInit();
	TypesInit();
	DataIOInit();
	ObjectsInit();
	BeginTests();

	TestObjectGraphSerialiserReachability();

	TestStatistics();
	ObjectsKill();
	DataIOKill();
	TypesKill();
	MemoryKill();
}


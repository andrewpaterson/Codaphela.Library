#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/PointerRemapper.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ExternalObjectSerialiser.h"
#include "StandardLib/MultiFileObjectWriter.h"
#include "StandardLib/ObjectWriter.h"
#include "StandardLib/DependentWriteObjects.h"
#include "StandardLib/SerialisedObject.h"
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
	AssertLayoutSize(376, sizeof(CTestSaveableObject1));  //If these sizes change the input test files will be wrong if the serialisation has changed.
#elif defined(__W65C816__)
	AssertLayoutSize(176, sizeof(CTestSaveableObject1));
#else
	AssertLayoutSize(208, sizeof(CTestSaveableObject1));  //If these sizes change the input test files will be wrong if the serialisation has changed.
#endif // BUILD_64
#ifdef BUILD_64
	AssertLayoutSize(384, sizeof(CTestSaveableObject2));
#elif defined(__W65C816__)
	AssertLayoutSize(184, sizeof(CTestSaveableObject2));
#else
	AssertLayoutSize(216, sizeof(CTestSaveableObject2));
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
// Each object written to one stream starts with its own length.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectGraphSerialiserTwoInOneStream(void)
{
	Ptr<CTestSaveableObject2>	pFirst;
	Ptr<CTestSaveableObject2>	pSecond;
	CObjectWriter				cWriter;
	CMemoryFile					cMemory;
	lsize						uiSecondStart;
	lsize						uiEnd;
	uint32						uiLength;

	pFirst = ONMalloc<CTestSaveableObject2>("First", "One");
	pSecond = ONMalloc<CTestSaveableObject2>("Second", "Two and more");

	cMemory.Init();
	cMemory.Open(EFM_ReadWrite_Create);
	cWriter.Init(&cMemory, NULL);
	AssertTrue(cWriter.Write(&pFirst));
	uiSecondStart = cMemory.GetBufferSize();
	AssertTrue(cWriter.Write(&pSecond));
	uiEnd = cMemory.GetBufferSize();

	memcpy(&uiLength, cMemory.GetBufferPointer(), sizeof(uint32));
	AssertLong(uiSecondStart, uiLength);
	memcpy(&uiLength, RemapSinglePointer(cMemory.GetBufferPointer(), (ptrdiff)uiSecondStart), sizeof(uint32));
	AssertLong(uiEnd - uiSecondStart, uiLength);

	cMemory.Close();
	cWriter.Kill();
	cMemory.Kill();

	pFirst->Kill();
	pSecond->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Marking an object written that isn't a dependent marks nothing.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectGraphSerialiserMarkNotDependent(void)
{
	Ptr<CTestSaveableObject2>	pDependent;
	Ptr<CTestSaveableObject2>	pOther;
	CDependentWriteObjects		cDependents;
	SLogConfig					sLogConfig;
	bool						bResult;

	pDependent = ONMalloc<CTestSaveableObject2>("Dependent", "One");
	pOther = ONMalloc<CTestSaveableObject2>("Other", "Two");

	cDependents.Init();
	cDependents.Add(&pDependent);

	sLogConfig = gcLogger.SetSilent();
	bResult = cDependents.Mark(&pOther);
	gcLogger.SetConfig(&sLogConfig);
	AssertFalse(bResult);
	AssertPointer(pDependent.Object(), cDependents.GetUnwritten());

	AssertTrue(cDependents.Mark(&pDependent));
	AssertNull(cDependents.GetUnwritten());
	cDependents.Kill();

	pDependent->Kill();
	pOther->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Dependents come out in the order they were added, each until it's
// marked.  One marked out of turn is skipped, and adding one twice adds
// it once.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectGraphSerialiserDependentOrder(void)
{
	Ptr<CTestSaveableObject2>	pA;
	Ptr<CTestSaveableObject2>	pB;
	Ptr<CTestSaveableObject2>	pC;
	Ptr<CTestSaveableObject2>	pD;
	CDependentWriteObjects		cDependents;

	pA = ONMalloc<CTestSaveableObject2>("A", "One");
	pB = ONMalloc<CTestSaveableObject2>("B", "Two");
	pC = ONMalloc<CTestSaveableObject2>("C", "Three");
	pD = ONMalloc<CTestSaveableObject2>("D", "Four");

	cDependents.Init();
	AssertNull(cDependents.GetUnwritten());
	cDependents.Add(&pC);
	cDependents.Add(&pA);
	cDependents.Add(&pB);
	cDependents.Add(&pC);

	AssertPointer(pC.Object(), cDependents.GetUnwritten());
	AssertPointer(pC.Object(), cDependents.GetUnwritten());
	AssertTrue(cDependents.Mark(&pA));
	AssertPointer(pC.Object(), cDependents.GetUnwritten());
	AssertTrue(cDependents.Mark(&pC));
	AssertPointer(pB.Object(), cDependents.GetUnwritten());

	cDependents.Add(&pD);
	cDependents.Add(&pA);
	AssertTrue(cDependents.Mark(&pB));
	AssertPointer(pD.Object(), cDependents.GetUnwritten());
	AssertTrue(cDependents.Mark(&pD));
	AssertNull(cDependents.GetUnwritten());
	cDependents.Kill();

	pA->Kill();
	pB->Kill();
	pC->Kill();
	pD->Kill();
}


#if !defined(BASELIB_SIZE16) && !defined(__W65C816__)
//////////////////////////////////////////////////////////////////////////
//
// An object too big for a fat hollow's uint16 size.
//
//////////////////////////////////////////////////////////////////////////
class CTestTooBigToBeFat : public CObject
{
CONSTRUCTABLE(CTestTooBigToBeFat);
DESTRUCTABLE(CTestTooBigToBeFat);
public:
	uint8	mauiData[70000];

	void	Init(void) { PreInit(); PostInit(); }
	void	Class(void) override {}
	void	Free(void) override {}
	bool	Save(CObjectWriter* pcFile) override { return true; }
	bool	Load(CObjectReader* pcFile) override { return true; }
};


//////////////////////////////////////////////////////////////////////////
//
// A pointer to it can't be written as a fat hollow (its size wouldn't
// fit), only as an ordinary one.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectGraphSerialiserTooBigToBeFat(void)
{
	Ptr<CTestTooBigToBeFat>	pBig;
	CObjectWriter			cWriter;
	CMemoryFile				cMemory;
	SLogConfig				sLogConfig;
	bool					bResult;

	pBig = OMalloc<CTestTooBigToBeFat>();

	cMemory.Init();
	cMemory.Open(EFM_ReadWrite_Create);
	cWriter.Init(&cMemory, NULL);
	sLogConfig = gcLogger.SetSilent();
	bResult = cWriter.WriteDependent(pBig.Object(), true);
	gcLogger.SetConfig(&sLogConfig);
	AssertFalse(bResult);
	AssertTrue(cWriter.WriteDependent(pBig.Object(), false));
	cMemory.Close();
	cWriter.Kill();
	cMemory.Kill();

	pBig->Kill();
}
#endif // !BASELIB_SIZE16 && !__W65C816__


//////////////////////////////////////////////////////////////////////////
//
// A serialised object's length is a uint32: a longer one is refused.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectGraphSerialiserSerialisedLength(void)
{
	CSerialisedObject	cSerialised;

	AssertTrue(cSerialised.SetLength(100));
	AssertLSize(100, cSerialised.GetLength());
#ifdef BUILD_64
	SLogConfig			sLogConfig;

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cSerialised.SetLength((size)MAX_UINT32 + 1));
	gcLogger.SetConfig(&sLogConfig);
	AssertLSize(100, cSerialised.GetLength());
#endif // BUILD_64
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
	TestObjectGraphSerialiserTwoInOneStream();
	TestObjectGraphSerialiserMarkNotDependent();
	TestObjectGraphSerialiserDependentOrder();
#if !defined(BASELIB_SIZE16) && !defined(__W65C816__)
	TestObjectGraphSerialiserTooBigToBeFat();
#endif // !BASELIB_SIZE16 && !__W65C816__
	TestObjectGraphSerialiserSerialisedLength();

	TestStatistics();
	ObjectsKill();
	DataIOKill();
	TypesKill();
	MemoryKill();
}


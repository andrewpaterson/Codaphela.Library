#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "StandardLib/Objects.h"
#include "StandardLib/PointerContainer.h"
#include "StandardLib/Array.h"
#include "StandardLib/MapObject.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/FileBasic.h"
#include "TestLib/Assert.h"
#include "TestBaseObject.h"
#include "ObjectTestSetup.h"
#include "EmbeddedObjectTestClasses.h"
#include "ObjectTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetupEmbeddedObjectConstructors(void)
{
	gcObjects.AddConstructor<CEmbeddedTest>();
	gcObjects.AddConstructor<CEmbeddedContainer>();
	gcObjects.AddConstructor<CEmbeddedComplex>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedFlags(void)
{
	ObjectsInit();

	CEmbeddedComplex	cComplex;

	AssertFalse(cComplex.HasClass());
	AssertFalse(cComplex.ma.HasClass());
	AssertFalse(cComplex.mcSimple.HasClass());
	AssertFalse(cComplex.mcContainer.HasClass());
	AssertFalse(cComplex.mcContainer.mcOne.HasClass());
	AssertFalse(cComplex.mcContainer.mcTwo.HasClass());
	AssertFalse(cComplex.IsInitialised());
	AssertFalse(cComplex.ma.IsInitialised());
	AssertFalse(cComplex.mcSimple.IsInitialised());
	AssertFalse(cComplex.mcContainer.IsInitialised());
	AssertFalse(cComplex.mcContainer.mcOne.IsInitialised());
	AssertFalse(cComplex.mcContainer.mcTwo.IsInitialised());

	cComplex.Init();

	AssertTrue(cComplex.HasClass());
	AssertTrue(cComplex.ma.HasClass());
	AssertTrue(cComplex.mcSimple.HasClass());
	AssertTrue(cComplex.mcContainer.HasClass());
	AssertTrue(cComplex.mcContainer.mcOne.HasClass());
	AssertTrue(cComplex.mcContainer.mcTwo.HasClass());
	AssertTrue(cComplex.IsInitialised());
	AssertTrue(cComplex.ma.IsInitialised());
	AssertTrue(cComplex.mcSimple.IsInitialised());
	AssertTrue(cComplex.mcContainer.IsInitialised());
	AssertTrue(cComplex.mcContainer.mcOne.IsInitialised());
	AssertTrue(cComplex.mcContainer.mcTwo.IsInitialised());

	cComplex.Kill();

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// An embedded object's distance to root and flags (but for constructing
// and classing) are its embedding container's: setting them on either sets
// them for both, however deeply it is embedded.
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedSharesContainerState(void)
{
	ObjectsInit();
	{
		Ptr<CEmbeddedComplex>	pComplex;
		CEmbeddedTest*			pcDeep;

		pComplex = OMalloc<CEmbeddedComplex>();
		ORoot()->Add(pComplex);
		pcDeep = &pComplex->mcContainer.mcOne;
		AssertInt(pComplex->GetDistToRoot(), pcDeep->GetDistToRoot());
		AssertInt(pComplex->GetDistToRoot(), pComplex->mcSimple.GetDistToRoot());
		AssertTrue(pComplex->GetDistToRoot() > ROOT_DIST_TO_ROOT);

		pComplex->SetDirty(false);
		AssertFalse(pComplex->IsDirty());
		AssertFalse(pcDeep->IsDirty());
		pcDeep->SetDirty(true);
		AssertTrue(pComplex->IsDirty());
		AssertTrue(pComplex->mcSimple.IsDirty());
		pComplex->SetDirty(false);
		AssertFalse(pcDeep->IsDirty());

		AssertTrue(pcDeep->HasClass());
		AssertTrue(pcDeep->IsInitialised());
		AssertInt(pComplex->GetFlags() & OBJECT_FLAGS_CONTAINER, pcDeep->GetFlags() & OBJECT_FLAGS_CONTAINER);

		ORoot()->Remove(pComplex);
		AssertInt(UNATTACHED_DIST_TO_ROOT, pcDeep->GetDistToRoot());
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedObjectAddDistToRoot(void)
{
	ObjectsInit();

	Ptr<CRoot>				pRoot = ORoot();
	Ptr<CGameWorld>			pWorld = OMalloc<CGameWorld>();

	pRoot->Add(pWorld);

	Ptr<CClusterMissile>	pMissile = ONMalloc<CClusterMissile>("Frank", pWorld);
	Ptr<CHarrier>			pHarrier = OMalloc<CHarrier>(pWorld);
	
	pWorld->AddPlayer(pHarrier);
	AssertInt(3, pHarrier->GetDistToRoot());

	pRoot->Add(pMissile);

	AssertInt(2, pMissile->GetDistToRoot());
	AssertInt(2, pMissile->mcMissile1.GetDistToRoot());
	AssertInt(2, pMissile->mcMissile2.GetDistToRoot());

	pMissile->mcMissile1.SetTarget(pHarrier);

	AssertInt(3, pHarrier->GetDistToRoot());

	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedObjectRemoveDistToRoot(void)
{
	ObjectsInit();

	Ptr<CRoot>				pRoot = ORoot();
	Ptr<CGameWorld>			pWorld = OMalloc<CGameWorld>();

	pRoot->Add(pWorld);

	Ptr<CClusterMissile>	pMissile = ONMalloc<CClusterMissile>("Frank", pWorld);
	Ptr<CHarrier>			pHarrier = OMalloc<CHarrier>(pWorld);
	Ptr<CMissile>			pHolder = OMalloc<CMissile>(pWorld);
	
	pWorld->AddTickable(pHolder);
	AssertInt(4, pHolder->GetDistToRoot());

	pHolder->SetTarget(pHarrier);
	AssertInt(5, pHarrier->GetDistToRoot());

	Ptr<CClusterLauncher>	pLauncher = OMalloc<CClusterLauncher>();

	pRoot->Add(pLauncher);
	AssertInt(2, pLauncher->GetDistToRoot());

	pLauncher->mpMissile = pMissile;

	AssertInt(3, pMissile->GetDistToRoot());
	AssertInt(3, pMissile->mcMissile1.GetDistToRoot());
	AssertInt(3, pMissile->mcMissile2.GetDistToRoot());
	AssertInt(5, pHarrier->GetDistToRoot());
	AssertSize(1, pHarrier->NumHeapFroms());

	pMissile->mcMissile1.SetTarget(pHarrier);
	AssertInt(4, pHarrier->GetDistToRoot());
	AssertSize(2, pHarrier->NumHeapFroms());

	pWorld->RemoveTickable(pHolder);
	pHolder = NULL;

	AssertInt(4, pHarrier->GetDistToRoot());
	AssertSize(1, pHarrier->NumHeapFroms());

	pRoot->Add(pHarrier);
	AssertInt(2, pHarrier->GetDistToRoot());

	pRoot->Remove(pHarrier);
	AssertInt(4, pHarrier->GetDistToRoot());

	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedObjectKill(void)
{
	char	szClusterMissileState[64];
	char	szMissile1State[64];

	ObjectsInit();

	Ptr<CRoot> pRoot = ORoot();
	Ptr<CGameWorld> pWorld = OMalloc<CGameWorld>();

	pRoot->Add(pWorld);

	Ptr<CClusterMissile> pClusterMissile = ONMalloc<CClusterMissile>("Anna", pWorld);
	pWorld = NULL;

	Ptr<CPointerContainer> pPointerPointer = OMalloc<CPointerContainer>(&pClusterMissile->mcMissile1);
	pRoot->Add(pPointerPointer);

	Ptr<CPointerContainer> pPointerPointer2 = OMalloc<CPointerContainer>(&pClusterMissile);
	pRoot->Add(pPointerPointer2);

	//The number of allocated object shouldn't change until all the froms are removed
	//both from the embedded object and the 'normal' object.
	AssertLong(7, gcObjects.GetMemory()->NumIndexed());
	AssertLong(7, gcUnknowns.NumElements());

	pClusterMissile->SetKillString(szClusterMissileState);
	pClusterMissile->mcMissile1.SetKillString(szMissile1State);

	AssertSize(2, pClusterMissile->NumHeapFroms());
	AssertSize(1, pClusterMissile->CEmbeddedObject::NumHeapFroms());
	AssertSize(1, pClusterMissile->mcMissile1.NumHeapFroms());
	strcpy(szClusterMissileState, "Alive");
	strcpy(szMissile1State, "Alive");

	pPointerPointer->Clear();

	AssertSize(1, pClusterMissile->NumHeapFroms());
	AssertSize(1, pClusterMissile->CEmbeddedObject::NumHeapFroms());
	AssertSize(0, pClusterMissile->mcMissile1.NumHeapFroms());
	AssertString("Alive", szClusterMissileState);
	AssertString("Alive", szMissile1State);

	pPointerPointer->mp = &pClusterMissile->mcMissile1;

	AssertSize(2, pClusterMissile->NumHeapFroms());
	AssertSize(1, pClusterMissile->CEmbeddedObject::NumHeapFroms());
	AssertSize(1, pClusterMissile->mcMissile1.NumHeapFroms());

	pPointerPointer2->Clear();

	AssertSize(1, pClusterMissile->NumHeapFroms());
	AssertSize(0, pClusterMissile->CEmbeddedObject::NumHeapFroms());
	AssertSize(1, pClusterMissile->mcMissile1.NumHeapFroms());
	AssertString("Alive", szClusterMissileState);
	AssertString("Alive", szMissile1State);

	pClusterMissile = NULL;

	//Make sure nothing has been de-allocated.
	AssertLong(7, gcObjects.GetMemory()->NumIndexed());
	AssertLong(7, gcUnknowns.NumElements());

	pPointerPointer->Clear();

	AssertString("Killed", szClusterMissileState);
	AssertString("Killed", szMissile1State);

	AssertLong(6, gcObjects.GetMemory()->NumIndexed());
	AssertLong(6, gcUnknowns.NumElements());

	ObjectsFlush();
	gcObjects.EvictInMemory();

	AssertLong(0, gcObjects.GetMemory()->NumIndexed());
	AssertLong(0, gcUnknowns.NumElements());

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedObjectContainerDehollowfication(void)
{
	bool			bResult;
	CFileUtil		cFileUtil;
	const char*		szClassName;
	OIndex			oiComplex;
	CCodabase*		pcDatabase;
	CSequence*		pcSequence;
	size			uiEmbeddedTestSize;
	size			uiEmbeddedContainerSize;
	char			szDirectory[] = "Output" _FS_ "EmbeddedObject";

	cFileUtil.RemoveDir(szDirectory);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	SetupEmbeddedObjectConstructors();

	Ptr<CRoot> pRoot = ORoot();
	Ptr<CEmbeddedComplex> pComplex = OMalloc<CEmbeddedComplex>();
	oiComplex = pComplex->GetIndex();

	pRoot->Add(pComplex);

	bResult = ObjectsFlush();
	AssertTrue(bResult);
	bResult = gcObjects.EvictInMemory();
	AssertTrue(bResult);

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);

	uiEmbeddedTestSize = OBJECT_SIZE + sizeof(int32) + sizeof(float) + POINTER_SIZE;
	AssertLayoutSize(sizeof(CEmbeddedTest), uiEmbeddedTestSize);
	uiEmbeddedContainerSize = OBJECT_SIZE + 4 + uiEmbeddedTestSize + sizeof(int32) + uiEmbeddedTestSize + sizeof(float) + POINTER_SIZE + 12  /* random padding I assume */;
	AssertLayoutSize(sizeof(CEmbeddedContainer), uiEmbeddedContainerSize);
	AssertLayoutSize(sizeof(CEmbeddedComplex), OBJECT_SIZE + uiEmbeddedTestSize + sizeof(int32) * 2 + sizeof(CArray<>) + uiEmbeddedContainerSize + POINTER_SIZE);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	SetupEmbeddedObjectConstructors();

	pRoot = gcObjects.Root();
	AssertTrue(pRoot.IsNotNull());
	pComplex = pRoot->GetFirst<CEmbeddedComplex>();
	AssertTrue(pComplex.IsHollow());
	
	szClassName = pComplex->ClassName();
	AssertString("CEmbeddedComplex", szClassName);
	AssertLong(oiComplex, pComplex.GetIndex());

	AssertInt((int32)1, pComplex->mai[0]);
	AssertInt((int32)2, pComplex->mai[1]);
	
	AssertLong(INVALID_O_INDEX, pComplex->mcSimple.GetIndex());
	AssertInt((int32)85, pComplex->mcSimple.miAmANumber);
	AssertFloat(58.0f, pComplex->mcSimple.mfSoAmI, 0);

	AssertLong(INVALID_O_INDEX, pComplex->mcContainer.GetIndex());
	AssertInt((int32)73, pComplex->mcContainer.mi);
	AssertFloat(666.0f, pComplex->mcContainer.mf, 0);
	AssertString("And", pComplex->mcContainer.msz);

	AssertLong(INVALID_O_INDEX, pComplex->mcContainer.mcOne.GetIndex());
	AssertInt((int32)85, pComplex->mcContainer.mcOne.miAmANumber);
	AssertFloat(58.0f, pComplex->mcContainer.mcOne.mfSoAmI, 0);

	AssertLong(INVALID_O_INDEX, pComplex->mcContainer.mcTwo.GetIndex());
	AssertInt((int32)85, pComplex->mcContainer.mcTwo.miAmANumber);
	AssertFloat(58.0f, pComplex->mcContainer.mcTwo.mfSoAmI, 0);

	AssertLong(3, gcObjects.NumMemoryIndexes());

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcSequence);
	SafeKill(pcDatabase);
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedObjectPointTo(void)
{
	bool		bResult;
	CFileUtil	cFileUtil;
	const char*	szClassName;
	CCodabase*	pcDatabase;
	CSequence*	pcSequence;
	char		szDirectory[] = "Output" _FS_ "EmbeddedObject";

	cFileUtil.RemoveDir(szDirectory);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	SetupEmbeddedObjectConstructors();

	Ptr<CRoot> pRoot = ORoot();
	Ptr<CEmbeddedComplex> pComplex = OMalloc<CEmbeddedComplex>();

	Ptr<CEmbeddedContainer> pContainer = &pComplex->mcContainer;
	pRoot->Add(pContainer);

	bResult = ObjectsFlush();
	AssertTrue(bResult);
	bResult = gcObjects.EvictInMemory();
	AssertTrue(bResult);

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcSequence);
	SafeKill(pcDatabase);
	ObjectsKill(false);

	AssertNull(&pContainer);


	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	SetupEmbeddedObjectConstructors();

	pRoot = gcObjects.Root();
	AssertTrue(pRoot.IsNotNull());
	pContainer = pRoot->GetFirst<CEmbeddedContainer>();
	AssertTrue(pContainer.IsHollow());
	AssertSize(0, pContainer.Object()->NumEmbedded());
	AssertLong(-1, pContainer.GetIndex());

	szClassName = pContainer->ClassName();
	AssertString("CEmbeddedContainer", szClassName);

	pComplex = pContainer->GetEmbeddingContainer();
	//Kinda feels like this test just stopped...

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
void TestEmbeddedGetEmbeddedIndex(void)
{
	ObjectsInit();

	CEmbeddedComplex	cComplex;

	cComplex.Init();

	AssertSize(0, cComplex.TestGetNumEmbeddedFromFlags());
	AssertSize(6, cComplex.NumEmbedded());
	AssertSize(6, cComplex.TestGetNumEmbeddedFromFlags());

	AssertSize(1, cComplex.mcSimple.NumEmbedded());
	AssertSize(1, cComplex.ma.NumEmbedded());
	AssertSize(3, cComplex.mcContainer.NumEmbedded());
	AssertSize(1, cComplex.mcContainer.mcOne.NumEmbedded());
	AssertSize(1, cComplex.mcContainer.mcOne.NumEmbedded());

	AssertSize(0, cComplex.GetEmbeddedIndex(&cComplex));
	AssertSize(1, cComplex.GetEmbeddedIndex(&cComplex.mcSimple));
	AssertSize(2, cComplex.GetEmbeddedIndex(&cComplex.ma));
	AssertSize(3, cComplex.GetEmbeddedIndex(&cComplex.mcContainer));
	AssertSize(4, cComplex.GetEmbeddedIndex(&cComplex.mcContainer.mcOne));
	AssertSize(5, cComplex.GetEmbeddedIndex(&cComplex.mcContainer.mcTwo));
	AssertSize(-1, cComplex.GetEmbeddedIndex(NULL));

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedGetEmbeddedObject(void)
{
	ObjectsInit();

	CEmbeddedComplex	cComplex;
	CEmbeddedObject*	pcEmbedded;

	cComplex.Init();

	pcEmbedded = cComplex.GetEmbeddedObject(0);
	AssertPointer(&cComplex, pcEmbedded);
	pcEmbedded = cComplex.GetEmbeddedObject(1);
	AssertPointer(&cComplex.mcSimple, pcEmbedded);
	pcEmbedded = cComplex.GetEmbeddedObject(2);
	AssertPointer(&cComplex.ma, pcEmbedded);
	pcEmbedded = cComplex.GetEmbeddedObject(3);
	AssertPointer(&cComplex.mcContainer, pcEmbedded);
	pcEmbedded = cComplex.GetEmbeddedObject(4);
	AssertPointer(&cComplex.mcContainer.mcOne, pcEmbedded);
	pcEmbedded = cComplex.GetEmbeddedObject(5);
	AssertPointer(&cComplex.mcContainer.mcTwo, pcEmbedded);
	pcEmbedded = cComplex.GetEmbeddedObject(6);
	AssertNull(pcEmbedded);

	pcEmbedded = cComplex.mcSimple.GetEmbeddedObject(0);
	AssertPointer(&cComplex.mcSimple, pcEmbedded);
	pcEmbedded = cComplex.mcSimple.GetEmbeddedObject(1);
	AssertNull(pcEmbedded);

	pcEmbedded = cComplex.ma.GetEmbeddedObject(0);
	AssertPointer(&cComplex.ma, pcEmbedded);
	pcEmbedded = cComplex.ma.GetEmbeddedObject(1);
	AssertNull(pcEmbedded);

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedObjectClass(void)
{
	ObjectsInit();

	CEmbeddedComplex	cComplex;
	CPointer*			pcPtr0;
	CPointer*			pcPtr1;
	CPointer*			pcPtr2;
	CPointer*			pcPtr3;
	CPointer*			pcPtr4;
	CPointer*			pcPtr5;

	//Class not yet called.  Always call Class.
	AssertSize(0, cComplex.GetNumFieldPointerTos());
	AssertSize(1, cComplex.NumEmbedded());

	cComplex.ClearFlagNumEmbedded();
	cComplex.Init();
	AssertSize(5, cComplex.GetNumFieldPointerTos());
	AssertSize(6, cComplex.NumEmbedded());

	pcPtr0 = cComplex.GetFieldPointerTo(0);
	pcPtr1 = cComplex.GetFieldPointerTo(1);
	pcPtr2 = cComplex.GetFieldPointerTo(2);
	pcPtr3 = cComplex.GetFieldPointerTo(3);
	pcPtr4 = cComplex.GetFieldPointerTo(4);
	pcPtr5 = cComplex.GetFieldPointerTo(5);

	AssertPointer(cComplex.mpTest.This(), pcPtr0);
	AssertPointer(cComplex.mcSimple.mpTest.This(), pcPtr1);
	AssertPointer(cComplex.mcContainer.mpTest.This(), pcPtr2);
	AssertPointer(cComplex.mcContainer.mcOne.mpTest.This(), pcPtr3);
	AssertPointer(cComplex.mcContainer.mcTwo.mpTest.This(), pcPtr4);
	AssertNull(pcPtr5);

	AssertSize(0, cComplex.GetFieldPointerToIndex(pcPtr0));
	AssertSize(1, cComplex.GetFieldPointerToIndex(pcPtr1));
	AssertSize(2, cComplex.GetFieldPointerToIndex(pcPtr2));
	AssertSize(3, cComplex.GetFieldPointerToIndex(pcPtr3));
	AssertSize(4, cComplex.GetFieldPointerToIndex(pcPtr4));
	AssertSize(-1, cComplex.GetFieldPointerToIndex(NULL));

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedOjectIsAllocatedInObjects()
{
	ObjectsInit();

	Ptr<CClusterMissile> pClusterMissile = ONMalloc<CClusterMissile>("Gerbil", Null());
	AssertTrue(pClusterMissile->IsAllocatedInObjects());
	AssertTrue(pClusterMissile->mcMissile1.IsAllocatedInObjects());
	AssertTrue(pClusterMissile->mcMissile2.IsAllocatedInObjects());
	pClusterMissile = NULL;

	CClusterMissile cClusterMissile;
	cClusterMissile.Init(NULL);
	AssertFalse(cClusterMissile.IsAllocatedInObjects());
	AssertFalse(cClusterMissile.mcMissile1.IsAllocatedInObjects());
	AssertFalse(cClusterMissile.mcMissile2.IsAllocatedInObjects());

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedObjectKillSanity(void)
{
	ObjectsInit();

	{
		Ptr<CEmbeddedContainer>		pContainer;
		Ptr<CEmbeddedTest>			pFieldOne;
		Ptr<CEmbeddedTest>			pFieldTwo;

		pContainer = OMalloc<CEmbeddedContainer>();

		pFieldOne = pContainer->GetOne();
		pFieldTwo = pContainer->GetTwo();

		AssertPointer(&pContainer->mcOne, &pFieldOne);
		AssertPointer(&pContainer->mcTwo, &pFieldTwo);

		pContainer->Kill();

		AssertNull(&pFieldOne);
		AssertNull(&pFieldTwo);
	}
	AssertSize(0, gcObjects.NumMemoryIndexes());

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// A stack pointer to an embedded object prints the embedded object (with
// no index of its own).
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedStackPointerPrint(void)
{
	ObjectsInit();

	{
		Ptr<CEmbeddedContainer>		pContainer;
		Ptr<CEmbeddedTest>			pFieldOne;
		CChars						sz;

		pContainer = OMalloc<CEmbeddedContainer>();
		pFieldOne = pContainer->GetOne();

		sz.Init();
		gcObjects.PrintStackPointers(&sz, false);
		AssertString("\
0: Pointer [CEmbeddedContainer:1 Ru Su (CC, D, CA, CI, CC)]\n\
1: Pointer [CEmbeddedTest:-1 Ru Su (CC, D, CA, CI, CC)]\n", sz.Text());
		sz.Kill();

		pContainer->Kill();
	}
	AssertSize(0, gcObjects.NumMemoryIndexes());

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// The graph shows the objects embedded in each object, and appends to
// the string it is given.
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedPrintGraph(void)
{
	ObjectsInit();

	{
		Ptr<CEmbeddedContainer>		pContainer;
		CChars						sz;
		char*						pszEmbedded;

		pContainer = OMalloc<CEmbeddedContainer>();
		ORoot()->Add(pContainer);

		sz.Init("Before\n");
		gcObjects.PrintGraph(&sz, false);
		AssertTrue(sz.StartsWith("Before\n-------------------------- Graph ---"));
		pszEmbedded = strstr(sz.Text(), "]:(CEmbeddedTest(");
		AssertNotNull(pszEmbedded);
		AssertNotNull(strstr(pszEmbedded + 1, "]:(CEmbeddedTest("));
		sz.Kill();
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedObjectKillFreesEmbedded(void)
{
	STestObjectFreedNotifier	sNotifierContainer;
	STestObjectFreedNotifier	sNotifierOne;
	STestObjectFreedNotifier	sNotifierTwo;
	STestObjectFreedNotifier	sNotifierThree;
	STestObjectFreedNotifier	sNotifierA;
	STestObjectFreedNotifier	sNotifierB;
	STestObjectFreedNotifier	sNotifierC;

	ObjectsInit();

	{
		CEmbeddedContainerWithNotifier		cContainer;
		Ptr<CEmbeddedTestWithNotifier>		pFieldOne;
		Ptr<CEmbeddedTestWithNotifier>		pFieldTwo;
		Ptr<CEmbeddedTestWithNotifier>		pFieldThree;

		Ptr<CTestObject>					pTestA;
		Ptr<CTestObject>					pTestB;
		Ptr<CTestObject>					pTestC;


		CTestObject							cTestD;
		CTestObject							cTestE;
		CTestObject							cTestF;

		pTestA = ONMalloc<CTestObject>("Aye", &sNotifierA);
		pTestB = ONMalloc<CTestObject>("Bee", &sNotifierB);
		pTestC = ONMalloc<CTestObject>("Sea", &sNotifierC);

		cContainer.Init(&sNotifierContainer,
			NULL,
			pTestA,
			pTestB,
			pTestC,
			&sNotifierOne,
			&sNotifierTwo,
			&sNotifierThree);

		pFieldOne = cContainer.GetOne();
		pFieldTwo = cContainer.GetTwo();
		pFieldThree = cContainer.GetThree();

		cTestD.Init(pFieldOne, NULL);
		cTestE.Init(pFieldTwo, NULL);
		cTestF.Init(pFieldThree, NULL);

		AssertPointer(&cContainer.mcOne, &pFieldOne);
		AssertPointer(&cContainer.mcTwo, &pFieldTwo);
		AssertPointer(&cContainer.mcThree, &pFieldThree);

		AssertFalse(sNotifierContainer.bFreed);
		AssertFalse(sNotifierOne.bFreed);
		AssertFalse(sNotifierTwo.bFreed);
		AssertFalse(sNotifierThree.bFreed);
		AssertFalse(sNotifierA.bFreed);
		AssertFalse(sNotifierB.bFreed);
		AssertFalse(sNotifierC.bFreed);

		cContainer.Kill();

		AssertTrue(sNotifierContainer.bFreed);
		AssertTrue(sNotifierOne.bFreed);
		AssertTrue(sNotifierTwo.bFreed);
		AssertTrue(sNotifierThree.bFreed);
		AssertFalse(sNotifierA.bFreed);
		AssertFalse(sNotifierB.bFreed);
		AssertFalse(sNotifierC.bFreed);

		AssertNull(&pFieldOne);
		AssertNull(&pFieldTwo);
		AssertNull(&pFieldThree);
		AssertNull(&cTestD.mpObject);
		AssertNull(&cTestE.mpObject);
		AssertNull(&cTestF.mpObject);
	}
	AssertSize(0, gcObjects.NumMemoryIndexes());

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedObjectPointedToNoRemove(void)
{
	ObjectsInit();

	STestObjectFreedNotifier	sNotifierA;
	STestObjectFreedNotifier	sNotifierD;
	{
		Ptr<CTestObject>			pTestA;
		CTestObject					cTestD;

		pTestA = ONMalloc<CTestObject>("Aye", &sNotifierA);
		pTestA->mi = 0xA;

		cTestD.Init(&sNotifierD);
		cTestD.mi = 0xD;
		pTestA->mpObject = &cTestD;

		AssertSize(1, gcObjects.NumMemoryIndexes());
		AssertPointer(&cTestD, pTestA->mpObject.Object());
		AssertFalse(sNotifierA.bFreed);
		AssertFalse(sNotifierD.bFreed);

		pTestA = NULL;

		AssertSize(0, gcObjects.NumMemoryIndexes());
		AssertTrue(sNotifierA.bFreed);
		AssertFalse(sNotifierD.bFreed);
	}
	AssertTrue(sNotifierA.bFreed);
	AssertTrue(sNotifierD.bFreed);

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// An object embedded in an embedded object, with a collection.
//
//////////////////////////////////////////////////////////////////////////
class CTestNestedCollectionInner : public CObject
{
CONSTRUCTABLE(CTestNestedCollectionInner);
DESTRUCTABLE(CTestNestedCollectionInner);
public:
	CArray<CTestObject>	maObjects;

	void Init(void) { PreInit(); maObjects.Init(); PostInit(); }
	void Class(void) override { M_Embedded(maObjects); }
	void Free(void) override {}
	bool Save(CObjectWriter* pcFile) override { return false; }
	bool Load(CObjectReader* pcFile) override { return false; }
};


class CTestNestedCollectionOuter : public CObject
{
CONSTRUCTABLE(CTestNestedCollectionOuter);
DESTRUCTABLE(CTestNestedCollectionOuter);
public:
	CTestNestedCollectionInner	mcInner;

	void Init(void) { PreInit(); mcInner.Init(); PostInit(); }
	void Class(void) override { M_Embedded(mcInner); }
	void Free(void) override {}
	bool Save(CObjectWriter* pcFile) override { return false; }
	bool Load(CObjectReader* pcFile) override { return false; }
};


//////////////////////////////////////////////////////////////////////////
//
// What only a collection two embeddings deep holds is reached from the
// stack through the object they are embedded in.  It used to be freed:
// only the object's own embedded objects were given its distance to the
// stack, not theirs (CInput's data formats were freed this way).
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedNestedCollectionHeldFromStack(void)
{
	STestObjectFreedNotifier	sNotifier;

	ObjectsInit();
	{
		Ptr<CTestNestedCollectionOuter>	pOuter;

		pOuter = OMalloc<CTestNestedCollectionOuter>();
		{
			Ptr<CTestObject>	pObject;

			pObject = OMalloc<CTestObject>(&sNotifier);
			pOuter->mcInner.maObjects.Add(pObject);
		}
		AssertFalse(sNotifier.bFreed);
		AssertSize(1, pOuter->mcInner.maObjects.NumElements());
		AssertLong(2, gcObjects.NumMemoryIndexes());
		gcObjects.ValidateObjectsConsistency();

		pOuter = NULL;
		AssertTrue(sNotifier.bFreed);
		AssertLong(0, gcObjects.NumMemoryIndexes());
	}
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// An object with a pointer of its own and an embedded object holding a
// collection.
//
//////////////////////////////////////////////////////////////////////////
class CTestEnumeratedOuter : public CObject
{
CONSTRUCTABLE(CTestEnumeratedOuter);
DESTRUCTABLE(CTestEnumeratedOuter);
public:
	Ptr<CTestObject>			mpOwn;
	CTestNestedCollectionInner	mcInner;

	void Init(void) { PreInit(); mcInner.Init(); PostInit(); }
	void Class(void) override { M_Pointer(mpOwn); M_Embedded(mcInner); }
	void Free(void) override {}
	bool Save(CObjectWriter* pcFile) override { return false; }
	bool Load(CObjectReader* pcFile) override { return false; }
};


//////////////////////////////////////////////////////////////////////////
//
// Whether apcTos is exactly the objects given, in order.
//
//////////////////////////////////////////////////////////////////////////
void AssertPointerTos(CArrayEmbeddedObjectPtr* papcTos, CEmbeddedObject* pc1 = NULL, CEmbeddedObject* pc2 = NULL, CEmbeddedObject* pc3 = NULL)
{
	CEmbeddedObject*	apcExpected[3];
	size				uiExpected;
	size				i;

	apcExpected[0] = pc1;
	apcExpected[1] = pc2;
	apcExpected[2] = pc3;
	for (uiExpected = 0; (uiExpected < 3) && (apcExpected[uiExpected] != NULL); uiExpected++)
	{
	}
	AssertSize(uiExpected, papcTos->NumElements());
	for (i = 0; (i < uiExpected) && (i < papcTos->NumElements()); i++)
	{
		AssertPointer(apcExpected[i], papcTos->GetPtr(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// GetPointerTos lists an object's pointer tos and its embedded objects'
// (however deep), BaseGetPointerTos only its own, NULLs never.  A map
// lists each key then its value.  ContainsPointerTo looks at an object's
// own pointers only.  The distance walks and validation all use these.
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedPointerTosListing(void)
{
	ObjectsInit();
	{
		Ptr<CTestEnumeratedOuter>	pOuter;
		Ptr<CTestObject>			pOwn;
		Ptr<CTestObject>			pA;
		Ptr<CTestObject>			pB;
		Ptr<CTestObject>			pNull;
		Ptr<CMapObject>				pMap;
		CArrayEmbeddedObjectPtr		apcTos;

		pOuter = OMalloc<CTestEnumeratedOuter>();
		pOwn = OMalloc<CTestObject>();
		pA = OMalloc<CTestObject>();
		pB = OMalloc<CTestObject>();
		pOuter->mpOwn = pOwn;
		pOuter->mcInner.maObjects.Add(pA);
		pOuter->mcInner.maObjects.Add(pNull);
		pOuter->mcInner.maObjects.Add(pB);

		apcTos.Init();
		pOuter->GetPointerTos(&apcTos);
		AssertPointerTos(&apcTos, &pOwn, &pA, &pB);
		apcTos.Kill();

		apcTos.Init();
		pOuter->BaseGetPointerTos(&apcTos);
		AssertPointerTos(&apcTos, &pOwn);
		apcTos.Kill();

		apcTos.Init();
		pOuter->mcInner.GetPointerTos(&apcTos);
		AssertPointerTos(&apcTos, &pA, &pB);
		apcTos.Kill();

		apcTos.Init();
		pOuter->mcInner.BaseGetPointerTos(&apcTos);
		AssertPointerTos(&apcTos);
		apcTos.Kill();

		apcTos.Init();
		pOuter->mcInner.maObjects.BaseGetPointerTos(&apcTos);
		AssertPointerTos(&apcTos, &pA, &pB);
		apcTos.Kill();

		AssertTrue(pOuter->ContainsPointerTo(&pOwn));
		AssertFalse(pOuter->ContainsPointerTo(&pA));
		AssertFalse(pOuter->mcInner.ContainsPointerTo(&pA));
		AssertTrue(pOuter->mcInner.maObjects.ContainsPointerTo(&pA));
		AssertFalse(pOuter->mcInner.maObjects.ContainsPointerTo(&pOwn));

		pMap = OMalloc<CMapObject>();
		pMap->Put(pA, pNull);
		pMap->Put(pB, pOwn);
		apcTos.Init();
		pMap->GetPointerTos(&apcTos);
		AssertSize(3, apcTos.NumElements());
		if (apcTos.GetPtr(0) == &pA)
		{
			AssertPointerTos(&apcTos, &pA, &pB, &pOwn);
		}
		else
		{
			AssertPointerTos(&apcTos, &pB, &pOwn, &pA);
		}
		apcTos.Kill();

		pMap = NULL;
		pOuter = NULL;
		pOwn = NULL;
		pA = NULL;
		pB = NULL;
		AssertLong(0, gcObjects.NumMemoryIndexes());
	}
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// What validating every object logs (truncated to uiSize).
//
//////////////////////////////////////////////////////////////////////////
void ValidateObjectsCapturingErrors(char* szErrors, size uiSize)
{
	CMemoryFile		cMemory;
	CFileBasic		cBasicMemory;
	SLogConfig		sLogConfig;

	memset(szErrors, 0, uiSize);
	sLogConfig = gcLogger.SetSilent();
	cMemory.Init();
	gcLogger.AddOutput(&cMemory);
	gcObjects.ValidateObjectsConsistency();
	gcLogger.RemoveOutput(&cMemory);
	gcLogger.SetConfig(&sLogConfig);

	cMemory.Open(EFM_Read);
	cBasicMemory.Init(&cMemory);
	if (cBasicMemory.Size() > 0)
	{
		cBasicMemory.ReadStringChars(szErrors, (size)((cBasicMemory.Size() < (filePos)uiSize - 1) ? cBasicMemory.Size() : (filePos)uiSize - 1));
	}
	cMemory.Close();
	cMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer two embeddings deep whose object has no from back to it is
// reported, once (each embedded object checks only its own pointers).
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedNestedPointerValidated(void)
{
	char	szErrors[1024];
	char*	pcError;

	ObjectsInit();
	{
		Ptr<CTestNestedCollectionOuter>	pOuter;
		Ptr<CTestObject>				pObject;

		pOuter = OMalloc<CTestNestedCollectionOuter>();
		pObject = OMalloc<CTestObject>();
		pOuter->mcInner.maObjects.Add(pObject);

		ValidateObjectsCapturingErrors(szErrors, sizeof(szErrors));
		AssertString("", szErrors);

		AssertTrue(pObject->TestRemoveHeapFrom(&pOuter->mcInner.maObjects));
		ValidateObjectsCapturingErrors(szErrors, sizeof(szErrors));
		pcError = strstr(szErrors, "does not have a from pointing to");
		AssertNotNull(pcError);
		AssertNull(strstr(pcError + 1, "does not have a from pointing to"));

		pObject->AddHeapFrom(&pOuter->mcInner.maObjects, false);
		ValidateObjectsCapturingErrors(szErrors, sizeof(szErrors));
		AssertString("", szErrors);

		pOuter = NULL;
		pObject = NULL;
		AssertLong(0, gcObjects.NumMemoryIndexes());
	}
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// The heap from iteration walks an object's own froms, then each embedded
// object's, in order.
//
//////////////////////////////////////////////////////////////////////////
void TestEmbeddedHeapFromIteration(void)
{
	ObjectsInit();
	{
		Ptr<CTestEmbeddedStrings>	pStrings;
		Ptr<CPointerContainer>		p1;
		Ptr<CPointerContainer>		p2;
		Ptr<CPointerContainer>		p3;
		CBaseObject*				pcStrings;
		SHeapFromIterator			sIter;
		CArrayBaseObjectPtrE32		apcFroms;

		pStrings = OMalloc<CTestEmbeddedStrings>();
		pcStrings = &pStrings;
		AssertNull(pcStrings->StartHeapFromIteration(&sIter));

		p1 = OMalloc<CPointerContainer>();
		p2 = OMalloc<CPointerContainer>();
		p3 = OMalloc<CPointerContainer>();
		p1->mp = pStrings;
		p2->mp = &pStrings->mString3;
		p3->mp = &pStrings->mString1;

		AssertPointer(&p1, pcStrings->StartHeapFromIteration(&sIter));
		AssertPointer(&p3, pcStrings->IterateHeapFrom(&sIter));
		AssertPointer(&p2, pcStrings->IterateHeapFrom(&sIter));
		AssertNull(pcStrings->IterateHeapFrom(&sIter));

		apcFroms.Init();
		pcStrings->GetHeapFroms(&apcFroms);
		AssertSize(3, apcFroms.NumElements());
		AssertPointer(&p1, apcFroms.GetPtr(0));
		AssertPointer(&p3, apcFroms.GetPtr(1));
		AssertPointer(&p2, apcFroms.GetPtr(2));
		apcFroms.Kill();
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEmbedded(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestEmbeddedHeapFromIteration();
	TestEmbeddedFlags();
	TestEmbeddedSharesContainerState();
	TestEmbeddedOjectIsAllocatedInObjects();
	TestEmbeddedObjectAddDistToRoot();
	TestEmbeddedObjectRemoveDistToRoot();
	TestEmbeddedObjectKill();
	TestEmbeddedObjectContainerDehollowfication();
	TestEmbeddedGetEmbeddedIndex();
	TestEmbeddedObjectClass();
	TestEmbeddedGetEmbeddedObject();
	TestEmbeddedObjectPointTo();
	TestEmbeddedObjectKillSanity();
	TestEmbeddedStackPointerPrint();
	TestEmbeddedPrintGraph();
	TestEmbeddedObjectKillFreesEmbedded();
	TestEmbeddedObjectPointedToNoRemove();
	TestEmbeddedNestedCollectionHeldFromStack();
	TestEmbeddedPointerTosListing();
	TestEmbeddedNestedPointerValidated();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}


#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "StandardLib/Objects.h"
#include "TestLib/Assert.h"
#include "NamedObjectTestClasses.h"
#include "ObjectTestSetup.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestKillSelfPointer1(void)
{
	ObjectsInit();

	Ptr<CRoot>				pRoot;
	Ptr<CTestNamedObject>	pObject;
	bool					bResult;
	CBaseObject*			pvObject;

	pRoot = ORoot();

	pObject = OMalloc<CTestNamedObject>(1);
	pObject->mpNamedTest1 = pObject;

	pRoot->Add(pObject);
	AssertSize(1, pRoot->NumObjects());

	AssertLong(3, gcObjects.NumMemoryIndexes());

	pvObject = pObject.BaseObject();
	pObject = NULL;

	//pObject should be destroyed here and not cause a stack overflow.
	bResult = pRoot->Remove(pvObject);
	AssertTrue(bResult);
	AssertSize(0, pRoot->NumObjects());

	AssertLong(2, gcObjects.NumMemoryIndexes());

	pRoot = NULL;

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestKillSelfPointer2(void)
{
	ObjectsInit();

	Ptr<CRoot>				pRoot;
	Ptr<CTestNamedObject>	pObject;
	bool					bResult;

	pRoot = ORoot();

	pObject = OMalloc<CTestNamedObject>(1);
	pObject->mpNamedTest1 = pObject;

	pRoot->Add(pObject);
	AssertSize(1, pRoot->NumObjects());

	AssertLong(3, gcObjects.NumMemoryIndexes());

	//pObject should not be destroyed here and not cause a stack overflow.
	bResult = pRoot->Remove(pObject);
	AssertTrue(bResult);
	AssertSize(0, pRoot->NumObjects());
	AssertLong(3, gcObjects.NumMemoryIndexes());

	//If there were cyclic pointers then the object cannot tell it should be freed when a stack pointer is removed.
	pObject = NULL;
	AssertLong(2, gcObjects.NumMemoryIndexes());

	pRoot = NULL;
	AssertLong(2, gcObjects.NumMemoryIndexes());

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestKillLongCyclicSelfPointer(void)
{
	ObjectsInit();

	Ptr<CRoot>				pRoot;
	Ptr<CTestNamedObject>	pObject1;
	Ptr<CTestNamedObject>	pObject2;
	Ptr<CTestNamedObject>	pObjectBase;
	bool					bResult;

	pRoot = ORoot();

	pObjectBase = OMalloc<CTestNamedObject>(0);
	pObject1 = OMalloc<CTestNamedObject>(1);
	pObjectBase->mpNamedTest1 = pObject1;
	pObject1->mpNamedTest1 = pObjectBase;

	pRoot->Add(pObjectBase);

	//pObject should be destroyed here and not cause a stack overflow.
	bResult = pRoot->Remove(pObjectBase);
	AssertTrue(bResult);

	pRoot = NULL;
	pObject1 = NULL;
	pObjectBase = NULL;
	AssertLong(2, gcObjects.NumMemoryIndexes());

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestKillBestPractice(void)
{
	CCodabase*	pcDatabase;
	CSequence*	pcSequence;
	CFileUtil	cFileUtil;

	cFileUtil.RemoveDir("Output" _FS_ "TestKillBestPractice");
	cFileUtil.TouchDir("Output" _FS_ "TestKillBestPractice");

	pcSequence = CSequenceFactory::Create("Output" _FS_ "TestKillBestPractice", DS_No);
	pcDatabase = CCodabaseFactory::Create("Output" _FS_ "TestKillBestPractice", IWT_Yes, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);

	//Generally an object will be killed if all pointers to it a removed.
	//Sometimes we'd rather not try and remove all the pointers we just want the object to die.
	//In the example below if a missile hits a jet then both objects should be removed;
	//regardless of whether anything else points to them.
	//Those objects that did point to them will be updated to point to NULL.

	Ptr<CRoot>			pRoot;
	Ptr<CGameWorld>		pWorld;

	pRoot = ORoot();

	AssertLong(1, pcDatabase->NumNames());
	pWorld = OMalloc<CGameWorld>();
	gcObjects.ValidateObjectsConsistency();

	pRoot->Add(pWorld);
	gcObjects.ValidateObjectsConsistency();

	Ptr<CHarrier> pHarrier = ONMalloc<CHarrier>("Harrier", pWorld);

	Ptr<CJeep> pJeep = ONMalloc<CJeep>("Jeep", pWorld);

	pWorld->AddPlayer(pHarrier);
	pWorld->AddPlayer(pJeep);

	SStateOnKill	sHarrierBefore;
	SStateOnKill	sHarrierAfter;
	SStateOnKill	sJeepBefore;
	SStateOnKill	sJeepAfter;

	pHarrier->SetKillHook(&sHarrierBefore, &sHarrierAfter);
	pJeep->SetKillHook(&sJeepBefore, &sJeepAfter);

	SStateOnKill	sGooseBefore;
	SStateOnKill	sGooseAfter;
	SStateOnKill	sMaverickBefore;
	SStateOnKill	sMaverickAfter;

	Ptr<CRedJet>	pRedJetGoose = ONMalloc<CRedJet>("Goose", pWorld);
	Ptr<CRedJet>	pRedJetMaverick = ONMalloc<CRedJet>("Maverick", pWorld);

	pRedJetGoose->SetKillHook(&sGooseBefore, &sGooseAfter);
	pRedJetMaverick->SetKillHook(&sMaverickBefore, &sMaverickAfter);

	Ptr<CMissile> pMissile1 = pHarrier->FireMissile(pRedJetGoose);
	Ptr<CMissile> pMissile2 = pHarrier->FireMissile(pRedJetGoose);
	Ptr<CMissile> pMissile3 = pHarrier->FireMissile(pRedJetMaverick);

	AssertString("Kill not called", sMaverickBefore.cPicture.mszPretenedImAPicture);
	AssertString("Kill not called", sMaverickAfter.cPicture.mszPretenedImAPicture);
	AssertLong(12, gcObjects.NumMemoryIndexes());
	AssertLong(5, gcObjects.NumMemoryNames());
	AssertLong(10, pcDatabase->NumIndices());
	AssertLong(5, pcDatabase->NumNames());
	AssertInt(2, pWorld.GetDistToRoot());
	AssertInt(3, pHarrier.GetDistToRoot());
	AssertInt(3, pJeep.GetDistToRoot());
	AssertInt(4, pRedJetGoose.GetDistToRoot());
	AssertInt(4, pRedJetMaverick.GetDistToRoot());
	AssertInt(4, pMissile1.GetDistToRoot());
	AssertInt(4, pMissile2.GetDistToRoot());
	AssertInt(4, pMissile3.GetDistToRoot());
	AssertSize(2, pRedJetMaverick->NumHeapFroms());
	AssertSize(3, pHarrier->GetMissiles()->NumElements());

	OIndex oiMissile3 = pMissile3->GetIndex();
	AssertLong(12LL, oiMissile3);

	pMissile3->Kill();  //<-- This is what is being tested -----------------------------------.
	pMissile3 = gcObjects.TestGetFromMemory(oiMissile3);
	AssertTrue(pMissile3.IsNull());

	AssertString("Kill not called", sMaverickBefore.cPicture.mszPretenedImAPicture);
	AssertString("Kill not called", sMaverickAfter.cPicture.mszPretenedImAPicture);
	AssertLong(11, gcObjects.NumMemoryIndexes());
	AssertLong(5, gcObjects.NumMemoryNames());
	AssertLong(11, pcDatabase->NumIndices());
	AssertLong(5, pcDatabase->NumNames());
	AssertInt(2, pWorld.GetDistToRoot());
	AssertInt(3, pHarrier.GetDistToRoot());
	AssertInt(3, pJeep.GetDistToRoot());
	AssertInt(4, pRedJetGoose.GetDistToRoot());
	AssertInt(4, pRedJetMaverick.GetDistToRoot());
	AssertInt(4, pMissile1.GetDistToRoot());
	AssertInt(4, pMissile2.GetDistToRoot());
	AssertSize(1, pRedJetMaverick->NumHeapFroms());
	AssertSize(2, pHarrier->GetMissiles()->NumElements());

	AssertString("Kill not called", sGooseBefore.cPicture.mszPretenedImAPicture);
	AssertString("Kill not called", sGooseAfter.cPicture.mszPretenedImAPicture);

	pRedJetGoose->Kill();
	AssertString("012345678901234", sGooseBefore.cPicture.mszPretenedImAPicture);
	AssertString("Alas I am Dead!", sGooseAfter.cPicture.mszPretenedImAPicture);
	AssertSize(1, pMissile1->NumPointerTos());
	CPointer pTarget1(pMissile1->GetTarget());
	AssertTrue(pTarget1.IsNull());
	AssertSize(1, pMissile2->NumPointerTos());
	CPointer pTarget2 = pMissile2->GetTarget();
	AssertTrue(pTarget2.IsNull());
	AssertLong(10, gcObjects.NumMemoryIndexes());
	AssertLong(4, gcObjects.NumMemoryNames());
	AssertLong(11, pcDatabase->NumIndices());
	AssertLong(5, pcDatabase->NumNames());
	AssertInt(2, pWorld.GetDistToRoot());
	AssertInt(3, pHarrier.GetDistToRoot());
	AssertInt(3, pJeep.GetDistToRoot());
	AssertInt(4, pRedJetMaverick.GetDistToRoot());
	AssertInt(4, pMissile1.GetDistToRoot());
	AssertInt(4, pMissile2.GetDistToRoot());

	pMissile1->Kill();
	pMissile2->Kill();
	AssertSize(0, pHarrier->GetMissiles()->NumElements());
	AssertTrue(pMissile1.IsNull());
	AssertTrue(pMissile2.IsNull());

	pJeep = NULL;
	pRedJetMaverick = NULL;
	pHarrier = NULL;

	//pHarrier.maMissiles.moi == 6
	//pWorld.maTickables.moi == 4
	//pWorld is pointed to from:
	//  pRoot.mpObjects
	//  pHarrier
	//  pJeep
	//  pRedJetMaverick

	pRoot->Remove(pWorld);
	//ERROR: (baseobject.cpp:1159) void __thiscall CBaseObject::ValidateCanFindRoot(void) Object {02596128 [ 4]:CArrayObject(128) 6} has a positive dist to root and should be able to find the Root object.

	AssertLong(8, gcObjects.NumMemoryIndexes());
	pWorld = NULL;

	AssertLong(2, gcObjects.NumMemoryIndexes());
	AssertLong(1, gcObjects.NumMemoryNames());
	AssertLong(11, pcDatabase->NumIndices());
	AssertLong(5, pcDatabase->NumNames());

	AssertInt('X', sHarrierBefore.sPoint.x);
	AssertInt('Y', sHarrierBefore.sPoint.y);
	AssertInt('Z', sHarrierBefore.sPoint.z);
	AssertString("012345678901234", sHarrierBefore.cPicture.mszPretenedImAPicture);
	AssertString("Alas I am Dead!", sHarrierAfter.cPicture.mszPretenedImAPicture);
	AssertString("012345678901234", sJeepBefore.cPicture.mszPretenedImAPicture);
	AssertString("Alas I am Dead!", sJeepAfter.cPicture.mszPretenedImAPicture);

	pRoot = NULL;

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
void TestKillCanFindRoot(void)
{
	CCodabase*	pcDatabase;
	CSequence*	pcSequence;
	CFileUtil	cFileUtil;

	cFileUtil.RemoveDir("Output" _FS_ "TestKillCanFindRoot");
	cFileUtil.TouchDir("Output" _FS_ "TestKillCanFindRoot");

	pcSequence = CSequenceFactory::Create("Output" _FS_ "TestKillCanFindRoot", DS_No);
	pcDatabase = CCodabaseFactory::Create("Output" _FS_ "TestKillCanFindRoot", IWT_Yes, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);

	Ptr<CRoot>			pRoot;
	Ptr<CGameWorld>		pWorld;

	pRoot = ORoot();

	pWorld = OMalloc<CGameWorld>();

	pRoot->Add(pWorld);

	Ptr<CHarrier> pHarrier = ONMalloc<CHarrier>("Harrier", pWorld);

	Ptr<CJeep> pJeep = ONMalloc<CJeep>("Jeep", pWorld);

	pWorld->AddPlayer(pHarrier);
	pWorld->AddPlayer(pJeep);

	SStateOnKill	sHarrierBefore;
	SStateOnKill	sHarrierAfter;
	SStateOnKill	sJeepBefore;
	SStateOnKill	sJeepAfter;

	pHarrier->SetKillHook(&sHarrierBefore, &sHarrierAfter);
	pJeep->SetKillHook(&sJeepBefore, &sJeepAfter);

	SStateOnKill	sGooseBefore;
	SStateOnKill	sGooseAfter;
	SStateOnKill	sMaverickBefore;
	SStateOnKill	sMaverickAfter;

	Ptr<CRedJet>	pRedJetGoose = ONMalloc<CRedJet>("Goose", pWorld);
	Ptr<CRedJet>	pRedJetMaverick = ONMalloc<CRedJet>("Maverick", pWorld);

	pRedJetGoose->SetKillHook(&sGooseBefore, &sGooseAfter);
	pRedJetMaverick->SetKillHook(&sMaverickBefore, &sMaverickAfter);

	AssertTrue(pJeep->CanFindRoot());
	AssertTrue(pRedJetMaverick->CanFindRoot());
	AssertTrue(pHarrier->CanFindRoot());
	AssertTrue(pHarrier->GetMissiles()->CanFindRoot());
	AssertTrue(pRedJetGoose->CanFindRoot());
	AssertTrue(pWorld->CanFindRoot());
	AssertTrue(pWorld->GetTickables()->CanFindRoot());
	AssertTrue(pRoot->CanFindRoot());
	AssertTrue(pRoot->TestGetSet()->CanFindRoot());
	AssertLong(9, gcObjects.NumMemoryIndexes());

	//     
	// 4        ArrayObject[6](4)
	// 4                 ^
	// 4                 |                     pRedJetGoose[8,Goose](4)
	// 4                 |                            ^/
	//                   |                           //  pRedJetMaverick[9,Maverick](4)
	// 3  pHarrier[5,Harrier](3)   pJeep[7,Jeep](3) //   ^/  
	// 3                |^     ^   /^     ^        //   //
	// 3                ||      \ //      |       //   //
	// 3                ||       X/       |      //   //
	// 3                ||      //\       |     //   //
	// 3                ||     //  \      |    /.   //
	// 3                ||    //   ArrayObject[4](3)/
	//                  ||   //    ^          .    /
	// 2                ||  //    /          /    /
	// 2                v| v/    /          v    v 
	// 2               pWorld[3](2)---------------
	//                   ^
	// 1                 |
	// 1             SetObject[2](1)
	//                   ^
	// 0                 |
	// 0              pRoot[1,GraphRoot](0)
	//  

	pRoot->Remove(pWorld);
	AssertFalse(pJeep->CanFindRoot());
	AssertFalse(pRedJetMaverick->CanFindRoot());
	AssertFalse(pHarrier->CanFindRoot());
	AssertFalse(pHarrier->GetMissiles()->CanFindRoot());  //The destruction of the pointer created by pHarrier->GetMissiles() set a lot of the root distances to -2.
	AssertFalse(pRedJetGoose->CanFindRoot());
	AssertFalse(pWorld->CanFindRoot());
	AssertFalse(pWorld->GetTickables()->CanFindRoot());
	AssertTrue(pRoot->CanFindRoot());
	AssertTrue(pRoot->TestGetSet()->CanFindRoot());
	AssertInt(-1, pJeep->GetDistToRoot());
	AssertInt(-1, pRedJetMaverick->GetDistToRoot());
	AssertInt(-1, pHarrier->GetDistToRoot());
	AssertInt(-1, pHarrier->GetMissiles()->GetDistToRoot());
	AssertInt(-1, pRedJetGoose->GetDistToRoot());
	AssertInt(-1, pWorld->GetDistToRoot());
	AssertInt(-1, pWorld->GetTickables()->GetDistToRoot());
	AssertLong(9, gcObjects.NumMemoryIndexes());
	AssertLong(5, gcObjects.NumMemoryNames());
	AssertLong(9, pcDatabase->NumIndices());
	AssertLong(5, pcDatabase->NumNames());


	pJeep = NULL;
	pRedJetMaverick = NULL;
	pHarrier = NULL;
	pRedJetGoose = NULL;
	pWorld = NULL;

	AssertLong(2, gcObjects.NumMemoryIndexes());
	AssertLong(1, gcObjects.NumMemoryNames());

	AssertInt('X', sHarrierBefore.sPoint.x);
	AssertInt('Y', sHarrierBefore.sPoint.y);
	AssertInt('Z', sHarrierBefore.sPoint.z);
	AssertString("012345678901234", sHarrierBefore.cPicture.mszPretenedImAPicture);
	AssertString("Alas I am Dead!", sHarrierAfter.cPicture.mszPretenedImAPicture);
	AssertString("012345678901234", sJeepBefore.cPicture.mszPretenedImAPicture);
	AssertString("Alas I am Dead!", sJeepAfter.cPicture.mszPretenedImAPicture);


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
void TestKill(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestKillSelfPointer1();
	TestKillSelfPointer2();
	TestKillLongCyclicSelfPointer();
	TestKillBestPractice();
	TestKillCanFindRoot();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}



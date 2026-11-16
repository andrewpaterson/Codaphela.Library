#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/SequenceFactory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/WeakPointer.h"
#include "StandardLib/ArrayWeakObject.h"
#include "TestLib/Assert.h"
#include "ObjectTestClasses.h"
#include "EmbeddedObjectTestClasses.h"


//Holds one object strongly and one weakly, and says whether the weak one was NULL when it was freed.
class CTestWeakHolder : public CObject
{
CONSTRUCTABLE(CTestWeakHolder)
DESTRUCTABLE(CTestWeakHolder)
public:
	CPointer	mpStrong;
	Weak<>		mwWeak;
	bool*		mpbWeakNullWhenFreed;
	bool		mbPointStrongsWeakAtThisWhenFreed;

	void	Init(bool* pbWeakNullWhenFreed = NULL);
	void	Class(void) override;
	void	Free(void) override;
};


//Holds objects weakly in an array.
class CTestWeakArrayHolder : public CObject
{
CONSTRUCTABLE(CTestWeakArrayHolder)
DESTRUCTABLE(CTestWeakArrayHolder)
public:
	CArrayWeakObject	maWeak;

	void	Init(void);
	void	Class(void) override;
	void	Free(void) override;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestWeakArrayHolder::Init(void)
{
	PreInit();
	maWeak.Init();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestWeakArrayHolder::Class(void)
{
	M_Embedded(maWeak);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestWeakArrayHolder::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestWeakHolder::Init(bool* pbWeakNullWhenFreed)
{
	PreInit();
	mpbWeakNullWhenFreed = pbWeakNullWhenFreed;
	mbPointStrongsWeakAtThisWhenFreed = false;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestWeakHolder::Class(void)
{
	M_Pointer(mpStrong);
	W_Pointer(mwWeak);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestWeakHolder::Free(void)
{
	if (mpbWeakNullWhenFreed)
	{
		*mpbWeakNullWhenFreed = mwWeak.IsNull();
	}
	if (mbPointStrongsWeakAtThisWhenFreed)
	{
		((CTestWeakHolder*)mpStrong.Object())->mwWeak = this;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A weak pointer doesn't keep its object: when the stack lets go it's
// freed, and the weak pointer is NULL.  Nothing is weakly held after.
//
//////////////////////////////////////////////////////////////////////////
void TestWeakPointerDoesNotKeep(void)
{
	Ptr<CTestWeakHolder>		pHolder;
	Ptr<CTestObject>			pObject;
	STestObjectFreedNotifier	sFreed;

	ObjectsInit();
	pHolder = OMalloc<CTestWeakHolder>();
	ORoot()->Add(pHolder);
	pObject = OMalloc<CTestObject>(&sFreed);
	pHolder->mwWeak = pObject;
	AssertTrue(pHolder->mwWeak.Get() == pObject);
	AssertSize(1, gcObjects.GetWeakFroms()->NumWeakFroms(&pObject));
	AssertSize(0, pObject->NumHeapFroms());

	pObject = NULL;
	AssertTrue(sFreed.bFreed);
	AssertTrue(pHolder->mwWeak.IsNull());
	AssertTrue(pHolder->mwWeak.Get().IsNull());
	AssertSize(0, gcObjects.GetWeakFroms()->NumObjects());

	ORoot()->Remove(pHolder);
	pHolder = NULL;
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Held strongly too, the object lives while the strong pointer does.
//
//////////////////////////////////////////////////////////////////////////
void TestWeakPointerWithStrong(void)
{
	Ptr<CTestWeakHolder>		pHolder;
	STestObjectFreedNotifier	sFreed;

	ObjectsInit();
	pHolder = OMalloc<CTestWeakHolder>();
	ORoot()->Add(pHolder);
	pHolder->mpStrong = OMalloc<CTestObject>(&sFreed);
	pHolder->mwWeak = pHolder->mpStrong;
	AssertFalse(sFreed.bFreed);
	AssertTrue(pHolder->mwWeak.Get() == pHolder->mpStrong);

	pHolder->mpStrong = NULL;
	AssertTrue(sFreed.bFreed);
	AssertTrue(pHolder->mwWeak.IsNull());
	AssertSize(0, gcObjects.GetWeakFroms()->NumObjects());

	ORoot()->Remove(pHolder);
	pHolder = NULL;
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// A holds B, B weakly holds A: when the root lets go of A both are freed.
//
//////////////////////////////////////////////////////////////////////////
void TestWeakPointerCycle(void)
{
	Ptr<CTestWeakHolder>	pA;
	Ptr<CTestWeakHolder>	pB;
	CBaseObject*			pcA;
	int64					iBefore;

	ObjectsInit();
	pA = OMalloc<CTestWeakHolder>();
	pB = OMalloc<CTestWeakHolder>();
	pA->mpStrong = pB;
	pB->mwWeak = pA;
	ORoot()->Add(pA);
	pcA = &pA;
	pA = NULL;
	pB = NULL;
	iBefore = gcObjects.NumMemoryIndexes();

	ORoot()->Remove(pcA);
	AssertLong(iBefore - 2, gcObjects.NumMemoryIndexes());
	AssertSize(0, gcObjects.GetWeakFroms()->NumObjects());
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Pointed elsewhere, a weak pointer leaves its old object's weak froms.
//
//////////////////////////////////////////////////////////////////////////
void TestWeakPointerReassign(void)
{
	Ptr<CTestWeakHolder>	pHolder;
	Ptr<CTestObject>		pFirst;
	Ptr<CTestObject>		pSecond;
	CWeakFroms*				pcWeakFroms;

	ObjectsInit();
	pcWeakFroms = gcObjects.GetWeakFroms();
	pHolder = OMalloc<CTestWeakHolder>();
	pFirst = OMalloc<CTestObject>();
	pSecond = OMalloc<CTestObject>();

	pHolder->mwWeak = pFirst;
	AssertSize(1, pcWeakFroms->NumWeakFroms(&pFirst));
	pHolder->mwWeak = pSecond;
	AssertSize(0, pcWeakFroms->NumWeakFroms(&pFirst));
	AssertSize(1, pcWeakFroms->NumWeakFroms(&pSecond));
	AssertTrue(pcWeakFroms->Contains(&pSecond, pHolder->mwWeak.This(), 0));
	pHolder->mwWeak = NULL;
	AssertSize(0, pcWeakFroms->NumObjects());

	pHolder->mwWeak = pFirst;
	pHolder = NULL;
	AssertSize(0, pcWeakFroms->NumObjects());

	pFirst = NULL;
	pSecond = NULL;
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Freed together (the holder holding its object both ways), the weak
// pointer is NULL by the time the holder's Free runs.
//
//////////////////////////////////////////////////////////////////////////
void TestWeakPointerFreedTogether(void)
{
	Ptr<CTestWeakHolder>		pHolder;
	STestObjectFreedNotifier	sFreed;
	bool						bWeakNullWhenFreed;
	CBaseObject*				pcHolder;

	ObjectsInit();
	bWeakNullWhenFreed = false;
	pHolder = OMalloc<CTestWeakHolder>(&bWeakNullWhenFreed);
	ORoot()->Add(pHolder);
	pHolder->mpStrong = OMalloc<CTestObject>(&sFreed);
	pHolder->mwWeak = pHolder->mpStrong;
	pcHolder = &pHolder;
	pHolder = NULL;

	ORoot()->Remove(pcHolder);
	AssertTrue(sFreed.bFreed);
	AssertTrue(bWeakNullWhenFreed);
	AssertSize(0, gcObjects.GetWeakFroms()->NumObjects());
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Weak references left both ways at ObjectsKill are cleared with their
// objects.
//
//////////////////////////////////////////////////////////////////////////
void TestWeakPointerObjectsKill(void)
{
	Ptr<CTestWeakHolder>	pA;
	Ptr<CTestWeakHolder>	pB;

	ObjectsInit();
	pA = OMalloc<CTestWeakHolder>();
	pB = OMalloc<CTestWeakHolder>();
	ORoot()->Add(pA);
	ORoot()->Add(pB);
	pA->mwWeak = pB;
	pB->mwWeak = pA;
	pA = NULL;
	pB = NULL;
	AssertSize(2, gcObjects.GetWeakFroms()->NumObjects());
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// A weak pointer to an object embedded in another is NULL when the
// container is freed.
//
//////////////////////////////////////////////////////////////////////////
void TestWeakPointerEmbedded(void)
{
	Ptr<CTestWeakHolder>		pHolder;
	Ptr<CEmbeddedContainer>		pContainer;

	ObjectsInit();
	pHolder = OMalloc<CTestWeakHolder>();
	ORoot()->Add(pHolder);
	pContainer = OMalloc<CEmbeddedContainer>();
	pHolder->mwWeak = pContainer->GetTwo();
	AssertTrue(pHolder->mwWeak.Object() == &pContainer->mcTwo);
	AssertSize(1, gcObjects.GetWeakFroms()->NumWeakFroms(&pContainer->mcTwo));

	pContainer = NULL;
	AssertTrue(pHolder->mwWeak.IsNull());
	AssertSize(0, gcObjects.GetWeakFroms()->NumObjects());

	ORoot()->Remove(pHolder);
	pHolder = NULL;
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// A stored object evicted while strongly held is a hollow, and the weak
// pointer follows it there; loaded again, to the loaded object.  An
// evicted holder loads with its weak pointer NULL (weak pointers aren't
// saved).
//
//////////////////////////////////////////////////////////////////////////
void TestWeakPointerHollow(void)
{
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "WeakPointerHollow";
	CCodabase*				pcDatabase;
	CSequence*				pcSequence;
	Ptr<CTestWeakHolder>	pHolder;
	Ptr<CTestObject>		pObject;
	CWeakFroms*				pcWeakFroms;

	cFileUtil.RemoveDir(szDirectory);
	AssertTrue(cFileUtil.TouchDir(szDirectory));
	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	gcObjects.AddConstructor<CTestObject>();
	gcObjects.AddConstructor<CTestWeakHolder>();
	pcWeakFroms = gcObjects.GetWeakFroms();

	pHolder = OMalloc<CTestWeakHolder>();
	ORoot()->Add(pHolder);
	pObject = OMalloc<CTestObject>();
	pHolder->mpStrong = pObject;
	pHolder->mwWeak = pObject;
	ObjectsFlush();

	pObject->Kill();
	AssertTrue(pObject.IsHollow());
	AssertTrue(pHolder->mwWeak.Object() == pObject.Object());
	AssertSize(1, pcWeakFroms->NumWeakFroms(pObject.Object()));

	AssertInt((int32)0x07070707, pObject->mi);
	AssertFalse(pObject.IsHollow());
	AssertTrue(pHolder->mwWeak.Object() == pObject.Object());
	AssertTrue(pHolder->mwWeak.Get() == pObject);
	AssertSize(1, pcWeakFroms->NumWeakFroms(pObject.Object()));
	AssertSize(1, pcWeakFroms->NumObjects());

	pHolder->Kill();
	AssertTrue(pHolder.IsHollow());
	AssertSize(0, pcWeakFroms->NumObjects());
	AssertTrue(pHolder->mwWeak.IsNull());
	AssertTrue(pHolder->mpStrong == pObject);

	pHolder = NULL;
	pObject = NULL;
	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// A Free() pointing a weak reference at its dying object (itself) leaves
// it NULL: the object's weak references were NULLed before Free ran.
//
//////////////////////////////////////////////////////////////////////////
void TestWeakPointerToObjectBeingFreed(void)
{
	Ptr<CTestWeakHolder>	pKept;
	Ptr<CTestWeakHolder>	pDying;

	ObjectsInit();
	pKept = OMalloc<CTestWeakHolder>();
	ORoot()->Add(pKept);
	pDying = OMalloc<CTestWeakHolder>();
	pDying->mpStrong = pKept;
	pDying->mbPointStrongsWeakAtThisWhenFreed = true;

	pDying = NULL;
	AssertTrue(pKept->mwWeak.IsNull());
	AssertSize(0, gcObjects.GetWeakFroms()->NumObjects());

	ORoot()->Remove(pKept);
	pKept = NULL;
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// A weak array keeps nothing: an object freed leaves its entry NULL (the
// array keeps its size), Compact takes the NULLs out; Remove moves the
// last entry into the gap, its weak from with it, so freeing that object
// later NULLs the right entry.  Freeing the holder takes its entries off
// the objects, which live on.
//
//////////////////////////////////////////////////////////////////////////
void TestWeakPointerArray(void)
{
	Ptr<CTestWeakArrayHolder>	pHolder;
	Ptr<CTestObject>			apObjects[4];
	CWeakFroms*					pcWeakFroms;
	CArrayWeakObject*			pcArray;
	size						i;

	ObjectsInit();
	pcWeakFroms = gcObjects.GetWeakFroms();
	pHolder = OMalloc<CTestWeakArrayHolder>();
	ORoot()->Add(pHolder);
	pcArray = &pHolder->maWeak;
	for (i = 0; i < 4; i++)
	{
		apObjects[i] = OMalloc<CTestObject>();
		AssertSize(i, pcArray->Add(apObjects[i]));
	}
	AssertSize(4, pcWeakFroms->NumObjects());
	AssertTrue(pcWeakFroms->Contains(&apObjects[2], pcArray, 2));

	apObjects[1] = NULL;
	AssertSize(4, pcArray->NumElements());
	AssertTrue(pcArray->Get(1).IsNull());
	AssertTrue(pcArray->Get(2) == apObjects[2]);

	pcArray->Compact();
	AssertSize(3, pcArray->NumElements());
	AssertTrue(pcArray->Contains(&apObjects[0]));
	AssertTrue(pcArray->Contains(&apObjects[2]));
	AssertTrue(pcArray->Contains(&apObjects[3]));
	AssertTrue(pcArray->Get(1) == apObjects[3]);
	AssertTrue(pcWeakFroms->Contains(&apObjects[3], pcArray, 1));
	AssertSize(3, pcWeakFroms->NumObjects());

	AssertTrue(pcArray->Remove(&apObjects[0]));
	AssertSize(2, pcArray->NumElements());
	AssertTrue(pcArray->Get(0) == apObjects[2]);
	AssertTrue(pcWeakFroms->Contains(&apObjects[2], pcArray, 0));
	AssertSize(0, pcWeakFroms->NumWeakFroms(&apObjects[0]));
	apObjects[2] = NULL;
	AssertTrue(pcArray->Get(0).IsNull());
	AssertTrue(pcArray->Get(1) == apObjects[3]);

	pcArray->Add(apObjects[0]);
	ORoot()->Remove(pHolder);
	pHolder = NULL;
	AssertSize(0, pcWeakFroms->NumObjects());
	AssertInt((int32)0x07070707, apObjects[0]->mi);

	for (i = 0; i < 4; i++)
	{
		apObjects[i] = NULL;
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestWeakPointer(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestWeakPointerDoesNotKeep();
	TestWeakPointerWithStrong();
	TestWeakPointerCycle();
	TestWeakPointerReassign();
	TestWeakPointerFreedTogether();
	TestWeakPointerObjectsKill();
	TestWeakPointerEmbedded();
	TestWeakPointerHollow();
	TestWeakPointerToObjectBeingFreed();
	TestWeakPointerArray();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}


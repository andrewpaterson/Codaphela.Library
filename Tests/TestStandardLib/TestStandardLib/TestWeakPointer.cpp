#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/WeakPointer.h"
#include "TestLib/Assert.h"
#include "ObjectTestClasses.h"


//Holds one object strongly and one weakly, and says whether the weak one was NULL when it was freed.
class CTestWeakHolder : public CObject
{
CONSTRUCTABLE(CTestWeakHolder)
DESTRUCTABLE(CTestWeakHolder)
public:
	CPointer	mpStrong;
	Weak<>		mwWeak;
	bool*		mpbWeakNullWhenFreed;

	void	Init(bool* pbWeakNullWhenFreed = NULL);
	void	Class(void) override;
	void	Free(void) override;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestWeakHolder::Init(bool* pbWeakNullWhenFreed)
{
	PreInit();
	mpbWeakNullWhenFreed = pbWeakNullWhenFreed;
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

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}


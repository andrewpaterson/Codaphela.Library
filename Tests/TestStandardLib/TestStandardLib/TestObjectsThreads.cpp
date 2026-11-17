//The 65C64 and the W65C816 run one thread (and their compiler is given every test's source).
#ifndef __LIBC65__
#include <string.h>
#include "BaseLib/Atomic.h"
#include "BaseLib/DataIO.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/Mutex.h"
#include "BaseLib/NativeThread.h"
#include "BaseLib/StdRandom.h"
#include "BaseLib/TypeNames.h"
#include "StandardLib/ArrayObject.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Object.h"
#include "StandardLib/Objects.h"
#include "StandardLib/WeakPointer.h"
#include "TestLib/Assert.h"
#include "UnknownTestClasses.h"


//Objects used from several threads at once (StandardLib/ThreadSafetyPlan.txt).  Each test starts its threads with the
//locking on, has them register, and ends with the object system checked and every object accounted for.  They are
//run under ThreadSanitizer too (a TestStandardLib built with -DBASELIB_TSAN=ON).

#define TEST_OBJECTS_THREADS			8
#define TEST_OBJECTS_THREADS_STEPS		2000
#define TEST_OBJECTS_THREADS_SLOTS		16
#define TEST_OBJECTS_THREADS_LIVE		0x11FE
#define TEST_OBJECTS_THREADS_FREED		0xDEAD


static int32	giTestThreadNodesFreed = 0;


class CTestThreadNode : public CObject
{
CONSTRUCTABLE(CTestThreadNode)
DESTRUCTABLE(CTestThreadNode)
public:
	Ptr<CTestThreadNode>	mpNext;
	Ptr<CTestThreadNode>	mpOther;
	Weak<CTestThreadNode>	mwWeak;
	int32					miState;	//TEST_OBJECTS_THREADS_LIVE until it is freed.

	void	Init(void);
	void	Class(void) override;
	void	Free(void) override;
	bool	IsLive(void);
};


void CTestThreadNode::Init(void)
{
	PreInit();
	miState = TEST_OBJECTS_THREADS_LIVE;
	PostInit();
}


void CTestThreadNode::Class(void)
{
	M_Pointer(mpNext);
	M_Pointer(mpOther);
	W_Pointer(mwWeak);
	U_Int32(miState);
}


void CTestThreadNode::Free(void)
{
	miState = TEST_OBJECTS_THREADS_FREED;
	AtomicAdd(&giTestThreadNodesFreed, 1);
}


//A node that a thread holds a Ptr to is never a freed one.
bool CTestThreadNode::IsLive(void)
{
	return miState == TEST_OBJECTS_THREADS_LIVE;
}


struct STestObjectsThread
{
	CNativeThread			cThread;
	NativeThreadFunction	fRun;			//What the thread does, registered.
	int32*					piStarted;		//How many of the threads have registered.
	uint32					uiSeed;
	CMutex*					pcMutex;		//The test's own, over the fields the threads share.
	CTestThreadNode*		pcShared;
	CArrayObject*			pcArray;
	int						iErrors;
	bool					bRegistered;
	bool					bUnregistered;
};


//////////////////////////////////////////////////////////////////////////
//
// A thread of a test: it registers, waits until every thread of the test has (so they all run at once however
// quickly the first would have finished), does its work and is done with objects.
//
//////////////////////////////////////////////////////////////////////////
static void TestObjectsThread(void* pvThread)
{
	STestObjectsThread*		psThread;

	psThread = (STestObjectsThread*)pvThread;
	psThread->bRegistered = ObjectsThreadInit();
	AtomicAdd(psThread->piStarted, 1);
	while (AtomicLoad(psThread->piStarted) < TEST_OBJECTS_THREADS)
	{
		ThreadYield();
	}

	psThread->fRun(psThread);
	psThread->bUnregistered = ObjectsThreadKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Runs TEST_OBJECTS_THREADS threads on fRun (the last on fLast, if it isn't NULL) and waits for them.  Returns the
// errors they counted; a thread that couldn't register, or ended still holding a Ptr, is one.
//
//////////////////////////////////////////////////////////////////////////
static int TestObjectsRunThreads(NativeThreadFunction fRun, NativeThreadFunction fLast, CMutex* pcMutex, CTestThreadNode* pcShared, CArrayObject* pcArray)
{
	STestObjectsThread	asThreads[TEST_OBJECTS_THREADS];
	int32				iStarted;
	int					iErrors;
	size				i;

	iStarted = 0;
	for (i = 0; i < TEST_OBJECTS_THREADS; i++)
	{
		asThreads[i].fRun = (fLast && (i == TEST_OBJECTS_THREADS - 1)) ? fLast : fRun;
		asThreads[i].piStarted = &iStarted;
		asThreads[i].uiSeed = 1000 + (uint32)i * 7919;
		asThreads[i].pcMutex = pcMutex;
		asThreads[i].pcShared = pcShared;
		asThreads[i].pcArray = pcArray;
		asThreads[i].iErrors = 0;
		asThreads[i].bRegistered = false;
		asThreads[i].bUnregistered = false;
		asThreads[i].cThread.Init();
		asThreads[i].cThread.Start(TestObjectsThread, &asThreads[i]);
	}

	iErrors = 0;
	for (i = 0; i < TEST_OBJECTS_THREADS; i++)
	{
		asThreads[i].cThread.Join();
		asThreads[i].cThread.Kill();
		iErrors += asThreads[i].iErrors;
		if (!asThreads[i].bRegistered || !asThreads[i].bUnregistered)
		{
			iErrors++;
		}
	}
	return iErrors;
}


static void TestObjectsThreadsBegin(void)
{
	ObjectsInit();
	gcObjects.DisableValidation();
	giTestThreadNodesFreed = 0;
	AssertTrue(ObjectsLocking(true));
}


//The threads are done: the locking goes off, the object system is checked and only uiObjects are left.
static void TestObjectsThreadsEnd(size uiObjects)
{
	AssertSize(0, ObjectsNumThreads());
	AssertTrue(ObjectsLocking(false));
	gcObjects.EnableValidation();
	gcObjects.ValidateObjectsConsistency();
	AssertLong((int64)uiObjects, gcObjects.NumMemoryIndexes());
	AssertSize(0, gcStackPointers.NumElements());
	ObjectsFlush();
	ObjectsKill();
}


static int32	giTestObjectsThreadsRelease = 0;	//Set for the registered thread to finish.


static void TestObjectsThreadsRegisteredThread(void* pvRegistered)
{
	*((bool*)pvRegistered) = ObjectsThreadInit();
	while (AtomicLoad(&giTestObjectsThreadsRelease) == 0)
	{
		ThreadYield();
	}
	if (*((bool*)pvRegistered))
	{
		ObjectsThreadKill();
	}
}


static void TestObjectsThreadsSwitchThread(void* pvSwitched)
{
	*((bool*)pvSwitched) = ObjectsLocking(true);
}


//A thread that would end holding a Ptr is told so by ObjectsThreadKill (a Debug build keeps count).
static void TestObjectsThreadsHoldingThread(void* pvThread)
{
	STestObjectsThread*		psThread;
	bool					bKilled;

	psThread = (STestObjectsThread*)pvThread;
	psThread->bRegistered = ObjectsThreadInit();
	{
		Ptr<CTestThreadNode>	pHeld = psThread->pcShared;

		bKilled = ObjectsThreadKill();
#ifdef OBJECTS_THREAD_CHECKS
		if (bKilled)
		{
			psThread->iErrors++;
		}
#else
		(void)bKilled;
#endif // OBJECTS_THREAD_CHECKS

		//It lets go of the Ptr as a registered thread.
		if (!ObjectsThreadInit())
		{
			psThread->iErrors++;
		}
	}
	psThread->bUnregistered = ObjectsThreadKill();
}


//////////////////////////////////////////////////////////////////////////
//
// The locking is the home thread's to switch, threads register only while it is on, and it can't go off (nor the
// objects be killed) while one is registered.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsRegistration(void)
{
	CNativeThread			cThread;
	STestObjectsThread		sHolding;
	Ptr<CTestThreadNode>	pNode;
	bool					bRegistered;
	bool					bSwitched;

	ObjectsInit();
	AssertFalse(IsObjectsLocking());
	AssertSize(0, ObjectsNumThreads());

	//No registering while the locking is off, and no switching it on from another thread.
	AtomicStore(&giTestObjectsThreadsRelease, 1);
	bRegistered = true;
	cThread.Init();
	cThread.Start(TestObjectsThreadsRegisteredThread, &bRegistered);
	cThread.Join();
	cThread.Kill();
	AssertFalse(bRegistered);

	bSwitched = true;
	cThread.Init();
	cThread.Start(TestObjectsThreadsSwitchThread, &bSwitched);
	cThread.Join();
	cThread.Kill();
	AssertFalse(bSwitched);
	AssertFalse(IsObjectsLocking());

	//The home thread isn't one of the registered, and doesn't register.
	AssertTrue(ObjectsLocking(true));
	AssertTrue(IsObjectsLocking());
	AssertFalse(ObjectsThreadInit());
	AssertFalse(ObjectsThreadKill());

	AtomicStore(&giTestObjectsThreadsRelease, 0);
	bRegistered = false;
	cThread.Init();
	cThread.Start(TestObjectsThreadsRegisteredThread, &bRegistered);
	while (ObjectsNumThreads() == 0)
	{
		ThreadYield();
	}
	AssertSize(1, ObjectsNumThreads());
	AssertFalse(ObjectsLocking(false));
	AssertTrue(IsObjectsLocking());
	ObjectsKill();
	AssertTrue(gbObjects);
	AtomicStore(&giTestObjectsThreadsRelease, 1);
	cThread.Join();
	cThread.Kill();
	AssertTrue(bRegistered);
	AssertSize(0, ObjectsNumThreads());

	pNode = OMalloc<CTestThreadNode>();
	ORoot()->Add(pNode);
	sHolding.pcShared = &pNode;
	sHolding.iErrors = 0;
	sHolding.bRegistered = false;
	sHolding.bUnregistered = false;
	cThread.Init();
	cThread.Start(TestObjectsThreadsHoldingThread, &sHolding);
	cThread.Join();
	cThread.Kill();
	AssertTrue(sHolding.bRegistered);
	AssertTrue(sHolding.bUnregistered);
	AssertInt(0, sHolding.iErrors);
	AssertSize(1, pNode.NumStackFroms());
	pNode = NULL;

	//Killing the objects switches the locking off.
	AssertTrue(IsObjectsLocking());
	ORoot()->RemoveAll();
	ObjectsFlush();
	ObjectsKill();
	AssertFalse(gbObjects);
	AssertFalse(IsObjectsLocking());
}


static void TestObjectsThreadsCopyPointers(void* pvThread)
{
	STestObjectsThread*		psThread;
	size					i;

	psThread = (STestObjectsThread*)pvThread;
	Ptr<CTestThreadNode>	pShared = psThread->pcShared;
	for (i = 0; i < TEST_OBJECTS_THREADS_STEPS; i++)
	{
		Ptr<CTestThreadNode>	pCopy = pShared;
		Ptr<CTestThreadNode>	pNext = pCopy->mpNext;

		if (!pCopy->IsLive() || !pNext->IsLive())
		{
			psThread->iErrors++;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Threads take and drop Ptrs to the same objects, attached to the root, at once: each object's stack froms are what
// they were when the threads have gone.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsSharedObject(void)
{
	Ptr<CTestThreadNode>	pShared;
	Ptr<CTestThreadNode>	pNext;

	TestObjectsThreadsBegin();
	pShared = OMalloc<CTestThreadNode>();
	pNext = OMalloc<CTestThreadNode>();
	pShared->mpNext = pNext;
	ORoot()->Add(pShared);

	AssertInt(0, TestObjectsRunThreads(TestObjectsThreadsCopyPointers, NULL, NULL, &pShared, NULL));

	AssertSize(1, pShared.NumStackFroms());
	AssertSize(1, pNext.NumStackFroms());
	AssertSize(1, pNext.NumHeapFroms());
	AssertInt(0, giTestThreadNodesFreed);
	pShared = NULL;
	pNext = NULL;
	TestObjectsThreadsEnd(4);
}


static void TestObjectsThreadsBuildGraphs(void* pvThread)
{
	STestObjectsThread*		psThread;
	size					i;

	psThread = (STestObjectsThread*)pvThread;
	for (i = 0; i < TEST_OBJECTS_THREADS_STEPS; i++)
	{
		Ptr<CTestThreadNode>	pFirst = OMalloc<CTestThreadNode>();
		Ptr<CTestThreadNode>	pSecond = OMalloc<CTestThreadNode>();
		Ptr<CTestThreadNode>	pThird = OMalloc<CTestThreadNode>();

		//A cycle, with a way back across it: nothing holds it but this thread's Ptrs.
		pFirst->mpNext = pSecond;
		pSecond->mpNext = pThird;
		pThird->mpNext = pFirst;
		pThird->mpOther = pSecond;
		pFirst->mwWeak = pThird;
		if (!pFirst->IsLive() || !pSecond->IsLive() || !pThird->IsLive() || (pFirst->mwWeak.Get() != pThird))
		{
			psThread->iErrors++;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Each thread builds graphs of its own, cycles and all, and lets them go: every object is freed, once.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsOwnGraphs(void)
{
	TestObjectsThreadsBegin();
	ORoot();

	AssertInt(0, TestObjectsRunThreads(TestObjectsThreadsBuildGraphs, NULL, NULL, NULL, NULL));

	AssertInt(TEST_OBJECTS_THREADS * TEST_OBJECTS_THREADS_STEPS * 3, giTestThreadNodesFreed);
	TestObjectsThreadsEnd(2);
}


//One thread points the fields of the array's nodes at new nodes, which frees the old ones unless another thread holds
//them.  The fields are the test's to guard: its mutex is held for each read and write of one.
static void TestObjectsThreadsRepoint(void* pvThread)
{
	STestObjectsThread*		psThread;
	CRandom					cRandom;
	size					i;

	psThread = (STestObjectsThread*)pvThread;
	cRandom.Init(psThread->uiSeed);
	for (i = 0; i < TEST_OBJECTS_THREADS_STEPS; i++)
	{
		Ptr<CTestThreadNode>	pNew = OMalloc<CTestThreadNode>();
		Ptr<CTestThreadNode>	pSlot = psThread->pcArray->Get((size)cRandom.Next(0, TEST_OBJECTS_THREADS_SLOTS - 1));

		psThread->pcMutex->Lock();
		pSlot->mpNext = pNew;
		psThread->pcMutex->Unlock();
	}
	cRandom.Kill();
}


static void TestObjectsThreadsHoldNeighbours(void* pvThread)
{
	STestObjectsThread*		psThread;
	CRandom					cRandom;
	size					i;

	psThread = (STestObjectsThread*)pvThread;
	cRandom.Init(psThread->uiSeed);
	for (i = 0; i < TEST_OBJECTS_THREADS_STEPS; i++)
	{
		Ptr<CTestThreadNode>	pSlot = psThread->pcArray->Get((size)cRandom.Next(0, TEST_OBJECTS_THREADS_SLOTS - 1));
		Ptr<CTestThreadNode>	pHeld;

		psThread->pcMutex->Lock();
		pHeld = pSlot->mpNext;
		psThread->pcMutex->Unlock();

		//Held: whatever the other thread does with the field now, this node stays.  Letting go of it may well free it.
		ThreadYield();
		if (!pHeld->IsLive() || !pSlot->IsLive())
		{
			psThread->iErrors++;
		}
	}
	cRandom.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Objects are freed by one thread's changes while other threads take Ptrs to them and to their neighbours: a Ptr
// never holds a freed object, and every object that was replaced is freed, once.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsFreedWhileHeld(void)
{
	Ptr<CArrayObject>		pArray;
	Ptr<CTestThreadNode>	pSlot;
	CMutex					cMutex;
	size					i;

	TestObjectsThreadsBegin();
	cMutex.Init();
	pArray = OMalloc<CArrayObject>();
	ORoot()->Add(pArray);
	for (i = 0; i < TEST_OBJECTS_THREADS_SLOTS; i++)
	{
		pSlot = OMalloc<CTestThreadNode>();
		pSlot->mpNext = OMalloc<CTestThreadNode>();
		pArray->Add(pSlot);
	}
	pSlot = NULL;

	AssertInt(0, TestObjectsRunThreads(TestObjectsThreadsHoldNeighbours, TestObjectsThreadsRepoint, &cMutex, NULL, &pArray));

	//One thread replaced a node at every step: the ones it replaced are freed and the last in each slot is not.
	AssertInt(TEST_OBJECTS_THREADS_STEPS, giTestThreadNodesFreed);
	AssertSize(TEST_OBJECTS_THREADS_SLOTS, pArray->NumElements());
	pArray = NULL;
	cMutex.Kill();
	TestObjectsThreadsEnd(3 + 2 * TEST_OBJECTS_THREADS_SLOTS);
}


//One thread keeps replacing what the shared node holds strongly and weakly.
static void TestObjectsThreadsReplaceWeak(void* pvThread)
{
	STestObjectsThread*		psThread;
	size					i;

	psThread = (STestObjectsThread*)pvThread;
	Ptr<CTestThreadNode>	pShared = psThread->pcShared;
	for (i = 0; i < TEST_OBJECTS_THREADS_STEPS; i++)
	{
		Ptr<CTestThreadNode>	pNew = OMalloc<CTestThreadNode>();

		pShared->mpNext = pNew;
		pShared->mwWeak = pNew;
	}
}


static void TestObjectsThreadsGetWeak(void* pvThread)
{
	STestObjectsThread*		psThread;
	size					i;

	psThread = (STestObjectsThread*)pvThread;
	Ptr<CTestThreadNode>	pShared = psThread->pcShared;
	for (i = 0; i < TEST_OBJECTS_THREADS_STEPS; i++)
	{
		Ptr<CTestThreadNode>	pHeld = pShared->mwWeak.Get();

		//NULL (what it pointed at has just been freed) or an object that stays while it is held.
		if (pHeld.IsNotNull())
		{
			ThreadYield();
			if (!pHeld->IsLive())
			{
				psThread->iErrors++;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A weak pointer read on one thread, while the last strong pointer to its object goes on another, gives NULL or an
// object that is kept: never one that is freed under the Ptr.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsWeak(void)
{
	Ptr<CTestThreadNode>	pShared;

	TestObjectsThreadsBegin();
	pShared = OMalloc<CTestThreadNode>();
	ORoot()->Add(pShared);

	AssertInt(0, TestObjectsRunThreads(TestObjectsThreadsGetWeak, TestObjectsThreadsReplaceWeak, NULL, &pShared, NULL));

	AssertInt(TEST_OBJECTS_THREADS_STEPS - 1, giTestThreadNodesFreed);
	AssertTrue(pShared->mwWeak.Get() == pShared->mpNext);
	AssertTrue(pShared->mpNext->IsLive());
	pShared = NULL;
	TestObjectsThreadsEnd(4);
}


//One thread adds to and removes from the array (under the test's mutex, which guards what is in the array).
static void TestObjectsThreadsChangeArray(void* pvThread)
{
	STestObjectsThread*		psThread;
	size					i;

	psThread = (STestObjectsThread*)pvThread;
	for (i = 0; i < TEST_OBJECTS_THREADS_STEPS; i++)
	{
		Ptr<CTestThreadNode>	pNew = OMalloc<CTestThreadNode>();

		psThread->pcMutex->Lock();
		psThread->pcArray->Add(pNew);
		if (psThread->pcArray->NumElements() > TEST_OBJECTS_THREADS_SLOTS)
		{
			psThread->pcArray->Remove((size)0);
		}
		psThread->pcMutex->Unlock();
	}
}


static void TestObjectsThreadsHoldMembers(void* pvThread)
{
	STestObjectsThread*		psThread;
	CRandom					cRandom;
	size					uiElements;
	size					i;

	psThread = (STestObjectsThread*)pvThread;
	cRandom.Init(psThread->uiSeed);
	for (i = 0; i < TEST_OBJECTS_THREADS_STEPS; i++)
	{
		Ptr<CTestThreadNode>	pHeld;

		psThread->pcMutex->Lock();
		uiElements = psThread->pcArray->NumElements();
		if (uiElements > 0)
		{
			pHeld = psThread->pcArray->Get((size)cRandom.Next(0, (int32)uiElements - 1));
		}
		psThread->pcMutex->Unlock();

		if (pHeld.IsNotNull())
		{
			ThreadYield();
			if (!pHeld->IsLive())
			{
				psThread->iErrors++;
			}
		}
	}
	cRandom.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A collection changed by one thread while others hold its members: a member that is removed stays for as long as a
// thread holds it.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsCollection(void)
{
	Ptr<CArrayObject>	pArray;
	CMutex				cMutex;

	TestObjectsThreadsBegin();
	cMutex.Init();
	pArray = OMalloc<CArrayObject>();
	ORoot()->Add(pArray);

	AssertInt(0, TestObjectsRunThreads(TestObjectsThreadsHoldMembers, TestObjectsThreadsChangeArray, &cMutex, NULL, &pArray));

	AssertSize(TEST_OBJECTS_THREADS_SLOTS, pArray->NumElements());
	AssertInt(TEST_OBJECTS_THREADS_STEPS - TEST_OBJECTS_THREADS_SLOTS, giTestThreadNodesFreed);
	pArray = NULL;
	cMutex.Kill();
	TestObjectsThreadsEnd(3 + TEST_OBJECTS_THREADS_SLOTS);
}


static void TestObjectsThreadsAddUnknowns(void* pvThread)
{
	STestObjectsThread*		psThread;
	CTestUnknown*			apcUnknowns[8];
	size					i;
	size					j;

	psThread = (STestObjectsThread*)pvThread;
	for (i = 0; i < TEST_OBJECTS_THREADS_STEPS; i++)
	{
		for (j = 0; j < 8; j++)
		{
			apcUnknowns[j] = gcUnknowns.Add<CTestUnknown>();
			apcUnknowns[j]->Init((int)(i + j));
		}
		for (j = 0; j < 8; j++)
		{
			if (apcUnknowns[j]->miCount != (int32)(i + j))
			{
				psThread->iErrors++;
			}
			apcUnknowns[j]->Kill();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Unknowns that aren't objects are added and removed by several threads at once.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsUnknowns(void)
{
	size	uiUnknowns;

	TestObjectsThreadsBegin();
	uiUnknowns = gcUnknowns.NumElements();

	AssertInt(0, TestObjectsRunThreads(TestObjectsThreadsAddUnknowns, NULL, NULL, NULL, NULL));

	AssertSize(uiUnknowns, gcUnknowns.NumElements());
	TestObjectsThreadsEnd(0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreads(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestObjectsThreadsRegistration();
	TestObjectsThreadsSharedObject();
	TestObjectsThreadsOwnGraphs();
	TestObjectsThreadsFreedWhileHeld();
	TestObjectsThreadsWeak();
	TestObjectsThreadsCollection();
	TestObjectsThreadsUnknowns();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}


#endif // !__LIBC65__


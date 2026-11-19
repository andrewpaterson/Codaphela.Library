//The 65C64 and the W65C816 run one thread (and their compiler is given every test's source).
#ifndef __LIBC65__
#include <string.h>
#include "BaseLib/Atomic.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/DataIO.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/Logger.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/SequenceFactory.h"
#include "BaseLib/Mutex.h"
#include "BaseLib/NativeThread.h"
#include "BaseLib/StdRandom.h"
#include "BaseLib/TypeNames.h"
#include "StandardLib/ArrayObject.h"
#include "StandardLib/HollowObject.h"
#include "StandardLib/ObjectsThreads.h"
#include "StandardLib/ObjectsWorker.h"
#include "StandardLib/ArrayWeakObject.h"
#include "StandardLib/IndexObject.h"
#include "StandardLib/MapObject.h"
#include "StandardLib/SetObject.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Object.h"
#include "StandardLib/Objects.h"
#include "StandardLib/String.h"
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
#define TEST_OBJECTS_THREADS_DIRECTORY	"Output" _FS_ "ObjectsThreads"


void StartCapturingLog(CMemoryFile* pcMemory, SLogConfig* psLogConfig);  //TestObjectFreeOrdering.cpp
void StopCapturingLog(CMemoryFile* pcMemory, SLogConfig* psLogConfig, char* szLogged, size uiSize);


static int32	giTestThreadNodesFreed = 0;
static int32	giTestThreadNodesFreedOffHome = 0;	//Freed on a thread that isn't its object system's home: none should be.
static int32	giTestThreadNeedersFreedLate = 0;	//Freed after what they need: none should be.


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
	if (!GetObjectsOrDefault()->IsHomeThread())
	{
		AtomicAdd(&giTestThreadNodesFreedOffHome, 1);
	}
}


//A node that a thread holds a Ptr to is never a freed one.
bool CTestThreadNode::IsLive(void)
{
	return miState == TEST_OBJECTS_THREADS_LIVE;
}


//An object that needs another to be there when it is freed (a Nes pointer): it is freed first.
class CTestThreadNeeder : public CObject
{
CONSTRUCTABLE(CTestThreadNeeder)
DESTRUCTABLE(CTestThreadNeeder)
public:
	Ptr<CTestThreadNode>	mpNeeds;

	void	Init(Ptr<CTestThreadNode> pNeeds);
	void	Class(void) override;
	void	Free(void) override;
};


void CTestThreadNeeder::Init(Ptr<CTestThreadNode> pNeeds)
{
	PreInit();
	mpNeeds = pNeeds;
	PostInit();
}


void CTestThreadNeeder::Class(void)
{
	Nes(M_Pointer(mpNeeds));
}


void CTestThreadNeeder::Free(void)
{
	if (mpNeeds.IsNull() || !mpNeeds->IsLive() || !GetObjectsOrDefault()->IsHomeThread())
	{
		AtomicAdd(&giTestThreadNeedersFreedLate, 1);
	}
}


struct STestObjectsThread
{
	CNativeThread			cThread;
	NativeThreadFunction	fRun;			//What the thread does, registered.
	int32*					piStarted;		//How many of the threads have registered,
	int32*					piFinished;		//and how many are done with objects.
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
// A thread of a test: it registers (as a worker if the test says), waits until every thread of the test has (so they all run at once however
// quickly the first would have finished), does its work and is done with objects.
//
//////////////////////////////////////////////////////////////////////////
static bool	gbTestObjectsThreadsWorkers = false;	//The threads are workers, each with an object system of its own.


static void TestObjectsThread(void* pvThread)
{
	STestObjectsThread*		psThread;

	psThread = (STestObjectsThread*)pvThread;
	psThread->bRegistered = gbTestObjectsThreadsWorkers ? ObjectsWorkerInit() : ObjectsThreadInit();
	AtomicAdd(psThread->piStarted, 1);
	while (AtomicLoad(psThread->piStarted) < TEST_OBJECTS_THREADS)
	{
		ThreadYield();
	}

	psThread->fRun(psThread);
	psThread->bUnregistered = ObjectsThreadKill();
	AtomicAdd(psThread->piFinished, 1);
}


typedef void (*TestObjectsHomeWork)(void);
static TestObjectsHomeWork	gfTestObjectsThreadsWaiting = NULL;	//What the home thread does over and over while it waits.


//////////////////////////////////////////////////////////////////////////
//
// Runs TEST_OBJECTS_THREADS threads on fRun (the last on fLast, if it isn't NULL) and waits for them.  Returns the
// errors they counted; a thread that couldn't register, or ended still holding a Ptr, is one.
//
// While it waits this thread, the home thread, does gfTestObjectsThreadsWaiting over and over if it is set: with
// ObjectsCollect it frees what the threads let go of, as one with a tick would.  Without it nothing is freed until the
// threads are done, which is what some tests are there to see, and is no way to wait for threads that let go of
// objects by the thousand.
//
//////////////////////////////////////////////////////////////////////////
static int TestObjectsRunThreads(NativeThreadFunction fRun, NativeThreadFunction fLast, CMutex* pcMutex, CTestThreadNode* pcShared, CArrayObject* pcArray)
{
	STestObjectsThread	asThreads[TEST_OBJECTS_THREADS];
	int32				iStarted;
	int32				iFinished;
	int					iErrors;
	size				i;

	iStarted = 0;
	iFinished = 0;
	for (i = 0; i < TEST_OBJECTS_THREADS; i++)
	{
		asThreads[i].fRun = (fLast && (i == TEST_OBJECTS_THREADS - 1)) ? fLast : fRun;
		asThreads[i].piStarted = &iStarted;
		asThreads[i].piFinished = &iFinished;
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

	while ((gfTestObjectsThreadsWaiting != NULL) && (AtomicLoad(&iFinished) < TEST_OBJECTS_THREADS))
	{
		gfTestObjectsThreadsWaiting();
		ThreadYield();
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
	giTestThreadNodesFreedOffHome = 0;
	giTestThreadNeedersFreedLate = 0;
	AssertTrue(ObjectsLocking(true));
}


//The threads are done: the locking goes off, the object system is checked and only uiObjects are left.  Whatever was
//freed, was freed by this thread.
static void TestObjectsThreadsEnd(size uiObjects)
{
	AssertInt(0, giTestThreadNodesFreedOffHome);
	AssertInt(0, giTestThreadNeedersFreedLate);
	AssertSize(0, ObjectsNumThreadErrors());
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


//A thread that would end holding a Ptr is told so by ObjectsThreadKill, which lets go of it for the thread: the Ptr
//is NULL after.
static void TestObjectsThreadsHoldingThread(void* pvThread)
{
	STestObjectsThread*		psThread;

	psThread = (STestObjectsThread*)pvThread;
	psThread->bRegistered = ObjectsThreadInit();
	{
		Ptr<CTestThreadNode>	pHeld = psThread->pcShared;

		if (ObjectsThreadKill() || pHeld.IsNotNull())
		{
			psThread->iErrors++;
		}
	}
	psThread->bRegistered = psThread->bRegistered && ObjectsThreadInit();
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
	AssertTrue(pNode->IsLive());
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


//Takes the shared object off the root and puts it back, over and over: each is a write, and between the two the
//object is held by stack pointers alone.
static void TestObjectsThreadsReRoot(void* pvThread)
{
	STestObjectsThread*		psThread;
	Ptr<CTestThreadNode>	pShared;
	Ptr<CRoot>				pRoot;
	size					i;

	psThread = (STestObjectsThread*)pvThread;
	pShared = psThread->pcShared;
	pRoot = ORoot();
	for (i = 0; i < TEST_OBJECTS_THREADS_STEPS; i++)
	{
		if (!pRoot->Remove(pShared))
		{
			psThread->iErrors++;
		}
		pRoot->Add(pShared);
	}
}


//Puts one of the array's objects (each thread its own) in the root and takes it out again, over and over: the object
//is held by the array throughout, and how far it is from the root changes each time.
static void TestObjectsThreadsRootOwn(void* pvThread)
{
	STestObjectsThread*		psThread;
	Ptr<CTestThreadNode>	pOwn;
	Ptr<CRoot>				pRoot;
	size					i;

	psThread = (STestObjectsThread*)pvThread;
	pOwn = psThread->pcArray->Get((psThread->uiSeed - 1000) / 7919);
	pRoot = ORoot();
	for (i = 0; i < TEST_OBJECTS_THREADS_STEPS; i++)
	{
		pRoot->Add(pOwn);
		if (!pRoot->Remove(pOwn))
		{
			psThread->iErrors++;
		}
	}
}


//Takes and drops a Ptr to each of the array's objects in turn, over and over.
static void TestObjectsThreadsCopyEach(void* pvThread)
{
	STestObjectsThread*		psThread;
	CTestThreadNode*		apcNodes[TEST_OBJECTS_THREADS];
	size					i;
	size					j;

	psThread = (STestObjectsThread*)pvThread;
	for (j = 0; j < TEST_OBJECTS_THREADS; j++)
	{
		apcNodes[j] = (CTestThreadNode*)psThread->pcArray->GetPlain(j);
	}
	for (i = 0; i < TEST_OBJECTS_THREADS_STEPS; i++)
	{
		for (j = 0; j < TEST_OBJECTS_THREADS; j++)
		{
			Ptr<CTestThreadNode>	pCopy = apcNodes[j];

			if (!pCopy->IsLive())
			{
				psThread->iErrors++;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Readers against writers that never rest: what a reader reads (how far an object is from the root) is what the
// writers write, and ThreadSanitizer says if a reader ever reads while one writes.
//
// First the other threads take and drop Ptrs to an object while one keeps taking it off the root and putting it
// back: every Ptr taken meets the lock held, or the object off the root, or neither.  Then one thread takes and drops
// Ptrs while all the others write, each handing the lock straight to the next: the reader waits for the lock to be
// let go, over and over, and must look again each time before it reads.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsReadersAndWriter(void)
{
	Ptr<CTestThreadNode>	pShared;
	Ptr<CTestThreadNode>	pNext;
	Ptr<CArrayObject>		pArray;
	size					i;

	TestObjectsThreadsBegin();
	pShared = OMalloc<CTestThreadNode>();
	pNext = OMalloc<CTestThreadNode>();
	pShared->mpNext = pNext;
	ORoot()->Add(pShared);

	AssertInt(0, TestObjectsRunThreads(TestObjectsThreadsCopyPointers, TestObjectsThreadsReRoot, NULL, &pShared, NULL));

	ObjectsCollect();
	AssertSize(1, pShared.NumStackFroms());
	AssertSize(1, pShared.NumHeapFroms());
	AssertSize(1, pNext.NumStackFroms());
	AssertInt(0, giTestThreadNodesFreed);
	AssertTrue(pShared->IsLive());
	pShared = NULL;
	pNext = NULL;

	pArray = OMalloc<CArrayObject>();
	ORoot()->Add(pArray);
	for (i = 0; i < TEST_OBJECTS_THREADS; i++)
	{
		pNext = OMalloc<CTestThreadNode>();
		pArray->Add(pNext);
	}
	pNext = NULL;

	AssertInt(0, TestObjectsRunThreads(TestObjectsThreadsRootOwn, TestObjectsThreadsCopyEach, NULL, NULL, &pArray));

	ObjectsCollect();
	AssertSize(TEST_OBJECTS_THREADS, pArray->NumElements());
	AssertSize(2, ORoot()->NumObjects());
	AssertInt(0, giTestThreadNodesFreed);
	pArray = NULL;
	TestObjectsThreadsEnd(5 + TEST_OBJECTS_THREADS);
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

	gfTestObjectsThreadsWaiting = ObjectsCollect;
	AssertInt(0, TestObjectsRunThreads(TestObjectsThreadsBuildGraphs, NULL, NULL, NULL, NULL));
	gfTestObjectsThreadsWaiting = NULL;

	//The threads found them dead; this thread, the home thread, freed them as it waited, and frees the last now.
	ObjectsCollect();
	AssertInt(TEST_OBJECTS_THREADS * TEST_OBJECTS_THREADS_STEPS * 3, giTestThreadNodesFreed);
	ObjectsCollect();
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
	ObjectsCollect();
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

	ObjectsCollect();
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
	ObjectsCollect();
	AssertInt(TEST_OBJECTS_THREADS_STEPS - TEST_OBJECTS_THREADS_SLOTS, giTestThreadNodesFreed);
	pArray = NULL;
	cMutex.Kill();
	TestObjectsThreadsEnd(3 + TEST_OBJECTS_THREADS_SLOTS);
}


static void TestObjectsThreadsLetGo(void* pvThread)
{
	STestObjectsThread*		psThread;

	psThread = (STestObjectsThread*)pvThread;
	Ptr<CTestThreadNode>	pShared = psThread->pcShared;
	{
		Ptr<CTestThreadNode>	pNeeded = OMalloc<CTestThreadNode>();
		Ptr<CTestThreadNeeder>	pNeeder = OMalloc<CTestThreadNeeder>(pNeeded);
		Ptr<CTestThreadNode>	pOther = OMalloc<CTestThreadNode>();

		//Held weakly by an object every thread shares (each thread's assignment replaces the last's, so what the weak
		//pointer gives is another thread's object, or nothing, as often as this one's), and by each other.
		pOther->mpNext = pNeeded;
		pShared->mwWeak = pOther;
	}

	//This thread has let go of them: they are not freed (the home thread will free them), but nothing reaches them.
	if (AtomicLoad(&giTestThreadNodesFreed) != 0)
	{
		psThread->iErrors++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Objects a thread lets go of are freed by the home thread, when it next uses objects or collects: each once, on the
// home thread, an object that needs another before the one it needs.  Until then nothing reaches them: a weak pointer
// to one is NULL at once.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsFreedOnHomeThread(void)
{
	Ptr<CTestThreadNode>	pShared;

	TestObjectsThreadsBegin();
	pShared = OMalloc<CTestThreadNode>();
	ORoot()->Add(pShared);

	AssertInt(0, TestObjectsRunThreads(TestObjectsThreadsLetGo, NULL, NULL, &pShared, NULL));

	//Nothing has been freed, and this thread hasn't touched an object since.  Its next use of one frees them all.
	AssertInt(0, giTestThreadNodesFreed);
	AssertTrue(pShared->mwWeak.Get().IsNull());
	AssertInt(TEST_OBJECTS_THREADS * 2, giTestThreadNodesFreed);
	AssertInt(0, giTestThreadNeedersFreedLate);
	AssertInt(0, giTestThreadNodesFreedOffHome);
	AssertSize(0, gcObjects.NumWaitingToBeFreed());

	pShared = NULL;
	TestObjectsThreadsEnd(3);
}


static bool	gbTestThreadCheckerAllowed = true;


//An object whose Free() does what a class with a lock of its own does before it takes the lock.
class CTestThreadChecker : public CObject
{
CONSTRUCTABLE(CTestThreadChecker)
DESTRUCTABLE(CTestThreadChecker)
public:
	void	Init(void) { PreInit(); PostInit(); }
	void	Class(void) override {}
	void	Free(void) override { gbTestThreadCheckerAllowed = ObjectsAssertNoFree(__METHOD__); }
};


//////////////////////////////////////////////////////////////////////////
//
// A class can find out that its own lock is being taken inside a Free() (a Debug build tells it).
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsAssertNoFree(void)
{
	Ptr<CTestThreadChecker>	pChecker;
	CMemoryFile				cLog;
	SLogConfig				sLogConfig;
	char					szLog[1024];

	ObjectsInit();
	AssertTrue(ObjectsAssertNoFree(__METHOD__));

	gbTestThreadCheckerAllowed = true;
	pChecker = OMalloc<CTestThreadChecker>();
	StartCapturingLog(&cLog, &sLogConfig);
	pChecker = NULL;
	StopCapturingLog(&cLog, &sLogConfig, szLog, sizeof(szLog));
#ifdef OBJECTS_THREAD_CHECKS
	AssertFalse(gbTestThreadCheckerAllowed);
	AssertTrue(strstr(szLog, "was reached from inside an object's Free()") != NULL);
#else
	AssertTrue(gbTestThreadCheckerAllowed);
	AssertString("", szLog);
#endif // OBJECTS_THREAD_CHECKS
	AssertTrue(ObjectsAssertNoFree(__METHOD__));

	ObjectsFlush();
	ObjectsKill();
}


//Runs statement, which is uiWaysIn ways into the object system and nothing more, and checks that each took the object
//system's lock once and held it throughout: a way in that has lost its guard takes the lock for each thing it calls,
//or not at all, and reaches the machinery behind it without the lock.
#define AssertWaysIn(uiWaysIn, statement) \
	do \
	{ \
		size	uiLocksTaken = ObjectsNumWaysIn(); \
		statement; \
		AssertSize(uiLocksTaken + (uiWaysIn), ObjectsNumWaysIn()); \
		AssertSize(0, ObjectsNumThreadErrors()); \
	} \
	while (false)


#define AssertWayIn(statement)	AssertWaysIn(1, statement)


//The same, for a way in that returns a pointer.  The pointer returned is assigned to pointer after the lock is
//counted, as assigning it, and letting go of what was returned, are ways in of their own.
#define AssertWayInReturning(pointer, expression) \
	do \
	{ \
		size	uiLocksTaken = ObjectsNumWaysIn(); \
		{ \
			auto&&	pReturned = (expression); \
			AssertSize(uiLocksTaken + 1, ObjectsNumWaysIn()); \
			pointer = pReturned; \
		} \
		AssertSize(0, ObjectsNumThreadErrors()); \
	} \
	while (false)


//////////////////////////////////////////////////////////////////////////
//
// The pointers' ways in.
//
//////////////////////////////////////////////////////////////////////////
static void TestObjectsThreadsEveryPointerWayIn(void)
{
	Ptr<CTestThreadNode>	pNode;
	Ptr<CTestThreadNode>	pOther;
	Ptr<CTestThreadNode>	pHeld;
	CTestThreadNode*		pcNode;
	CTestThreadNode*		pcOther;
	CEmbeddedObject*		pcReturned;
	CPointer*				ppFirst;
	CPointer*				ppSecond;
	CPointer*				ppThird;
	CChars					sz;
	size					uiCount;
	size					uiBefore;
	size					uiReads;
	int32					iFreed;
	int						iDist;
	bool					bDirty;

	pNode = OMalloc<CTestThreadNode>();
	pOther = OMalloc<CTestThreadNode>();
	ORoot()->Add(pNode);
	ORoot()->Add(pOther);
	pcNode = &pNode;
	pcOther = &pOther;

	//Made with new so that each constructor, and the destructor, is a statement of its own.  The objects are held by
	//the root, so each of a stack pointer's ways in is taken as a reader, without the lock (level 2).
	uiReads = ObjectsNumReads();
	AssertWayIn(ppFirst = new CPointer(pcNode));
	AssertSize(uiReads + 1, ObjectsNumReads());
	AssertWayIn(ppSecond = new CPointer(*ppFirst));
	AssertWayIn(ppThird = new CPointer((const CPointer&)*ppFirst));
	AssertWayIn(*ppSecond = pcOther);
	AssertWayIn(*ppSecond = *ppFirst);
	AssertWayIn(*ppSecond = (const CPointer&)*ppThird);
	AssertWayIn(pcReturned = ppSecond->Return());
	AssertPointer(pcNode, pcReturned);
	AssertWayIn(ppSecond->AssignObject(pcOther));
	AssertWayIn(ppSecond->PointToNull());
	ppSecond->AssignObject(pcOther);
	AssertWayIn(ppThird->LogicalDirty());
	AssertWayIn(uiCount = ppThird->NumStackFroms());
	AssertSize(3, uiCount);
	AssertWayIn(uiCount = ppThird->NumHeapFroms());
	AssertSize(1, uiCount);
	AssertWayIn(uiCount = ppThird->NumEmbedded());
	AssertSize(1, uiCount);
	AssertWayIn(uiCount = ppThird->NumPointerTos());
	AssertSize(0, uiCount);
	AssertWayIn(iDist = ppThird->GetDistToRoot());
	AssertInt(2, iDist);
	AssertWayIn(bDirty = ppThird->IsDirty());
	AssertBool(pcNode->IsDirty(), bDirty);
	AssertWayIn(delete ppSecond);
	AssertWayIn(delete ppThird);
	uiReads = ObjectsNumReads();
	AssertWayIn(delete ppFirst);
	AssertSize(uiReads + 1, ObjectsNumReads());

	//The same ways in with an object the root doesn't hold, which a stack pointer taken or let go of can free: each
	//is taken with the lock, and so is a pointer between it and one the root does hold.
	pHeld = OMalloc<CTestThreadNode>();
	uiReads = ObjectsNumReads();
	iFreed = giTestThreadNodesFreed;
	AssertWayIn(ppFirst = new CPointer(&pHeld));
	AssertWayIn(ppSecond = new CPointer(*ppFirst));
	AssertWayIn(*ppSecond = pcNode);
	AssertWayIn(*ppSecond = *ppFirst);
	AssertWayIn(ppSecond->PointToNull());
	AssertWayIn(ppSecond->AssignObject(&pHeld));
	AssertWayIn(delete ppSecond);
	AssertWayIn(delete ppFirst);
	AssertSize(uiReads, ObjectsNumReads());
	AssertInt(iFreed, giTestThreadNodesFreed);
	AssertWayIn(pHeld = NULL);
	AssertInt(iFreed + 1, giTestThreadNodesFreed);

	//A field, and a weak one.
	AssertWayIn(pNode->mpNext = pOther);
	AssertWayIn(pNode->mwWeak = pOther);
	AssertWayIn(pNode->mwWeak = pcNode);
	AssertWayIn(pOther->mwWeak = pNode->mwWeak);
	AssertWayInReturning(pHeld, pNode->mwWeak.Get());
	AssertTrue(pHeld == pNode);
	pHeld = NULL;

	//An object's own: its Init (the end of it), and being dirtied, marked stored and killed.
	pHeld = OMallocNoI<CTestThreadNode>();
	AssertWayIn(pHeld->Init());
	AssertWayIn(pHeld->SetDirty(false));
	AssertWayIn(pHeld->SetDirty(true));
	AssertWayIn(pHeld->SetStored(true));
	AssertWayIn(pHeld->SetStored(false));
	AssertWayIn(pHeld->Kill());
	AssertTrue(pHeld.IsNull());

	//A pointer whose object is killed through it, and a pointer's objects swapped for another's.
	pHeld = OMalloc<CTestThreadNode>();
	AssertWayIn(pHeld.Kill());
	pHeld = OMalloc<CTestThreadNode>();
	pNode->mpOther = pHeld;
	AssertWayIn(uiCount = pHeld.MorphInto(pcOther));
	AssertSize(2, uiCount);
	AssertTrue(pNode->mpOther == pOther);
	pHeld = NULL;

	//An object on the stack: its class is completed in the shared classes, and its destructor unhooks it.
	uiBefore = ObjectsNumWaysIn();
	{
		CTestThreadNode		cOnStack;

		AssertWaysIn(2, cOnStack.Init());  //Its PreInit and its PostInit.
		AssertWayIn(cOnStack.mpNext = pNode);
	}
	AssertSize(uiBefore + 5, ObjectsNumWaysIn());  //And its destructor, which kills it and then unhooks it.

	//What points at an object, and what it points at, written out.
	sz.Init();
	AssertWayIn(pcNode->PrintFroms(&sz));
	AssertWayIn(pcNode->PrintPointerTos(&sz));
	AssertWayIn(pcNode->PrintHeapFroms(&sz));
	AssertTrue(sz.Contains("Total Heap Froms [1], Stack Froms [1]"));
	sz.Kill();

	pNode = NULL;
	pOther = NULL;
	ORoot()->RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
//
// The collections' ways in, and the root's.
//
//////////////////////////////////////////////////////////////////////////
static void TestObjectsThreadsEveryCollectionWayIn(void)
{
	Ptr<CTestThreadNode>	apNodes[4];
	Ptr<CTestThreadNode>	pHeld;
	Ptr<CTestThreadNode>	pOther;
	Ptr<CArrayObject>		pArray;
	Ptr<CArrayObject>		pOtherArray;
	Ptr<CSetObject>			pSet;
	Ptr<CIndexObject>		pIndex;
	Ptr<CMapObject>			pMap;
	Ptr<CArrayWeakObject>	pWeakArray;
	Ptr<CRoot>				pRoot;
	Ptr<CSetObject>			pAll;
	CPointer				pFound;
	SSetIterator			sIter;
	size					uiCount;
	bool					bHollow;
	size					i;

	pRoot = ORoot();
	for (i = 0; i < 4; i++)
	{
		apNodes[i] = OMalloc<CTestThreadNode>();
	}
	pArray = OMalloc<CArrayObject>();
	pOtherArray = OMalloc<CArrayObject>();
	pSet = OMalloc<CSetObject>();
	pIndex = OMalloc<CIndexObject>();
	pMap = OMalloc<CMapObject>();
	pWeakArray = OMalloc<CArrayWeakObject>();

	AssertWayIn(pRoot->Add(pArray));
	pRoot->Add(pOtherArray);
	pRoot->Add(pSet);
	pRoot->Add(pIndex);
	pRoot->Add(pMap);
	pRoot->Add(pWeakArray);

	//Arrays.
	AssertWayIn(pArray->Add(apNodes[0]));
	AssertWayIn(pArray->CArrayCommonObject::Add((CEmbeddedObject*)&apNodes[1]));
	AssertWayIn(pArray->Insert(0, apNodes[2]));
	AssertWayIn(pArray->Set(1, apNodes[3]));
	AssertWayIn(pOtherArray->CArrayCommonObject::AddAll(&pArray));
	AssertSize(3, pOtherArray->NumElements());
	AssertWayIn(pArray->Remove((size)0));
	AssertWayIn(pArray->Remove(apNodes[1]));
	AssertWayIn(pArray->CArrayCommonObject::Remove((CEmbeddedObject*)&apNodes[3]));
	AssertSize(0, pArray->NumElements());
	AssertWayIn(pArray->CArrayCommonObject::RemoveAll(&pOtherArray));
	AssertWayIn(pOtherArray->RemoveEnd(1));
	AssertSize(1, pOtherArray->NumElements());
	AssertWayIn(pOtherArray->TouchAll());
	AssertWayIn(pOtherArray->RemoveAll());
	pHeld = OMalloc<CTestThreadNode>();
	pOther = OMalloc<CTestThreadNode>();
	pOtherArray->Add(pHeld);
	pOtherArray->Add(pOther);
	pHeld = NULL;
	pOther = NULL;
	AssertWayIn(pOtherArray->KillAll());

	//A set.
	pSet->Add(apNodes[0]);
	pSet->Add(apNodes[1]);
	AssertWayIn(pSet->TouchAll());
	pSet->StartIteration(&sIter);
	AssertWayIn(pSet->RemoveDuringIteration(&sIter));
	AssertSize(1, pSet->NumElements());

	//An index and a map.
	AssertWayIn(pIndex->Put("First", apNodes[0]));
	AssertWayIn(pIndex->Put((uint8*)"Second", 6, apNodes[1]));
	AssertWayIn(pIndex->TouchAll());
	AssertWayIn(pIndex->Remove("First"));
	AssertWayIn(pIndex->Remove((uint8*)"Second", 6));
	pHeld = OMalloc<CTestThreadNode>();
	pOther = OMalloc<CTestThreadNode>();
	pIndex->Put("Killed", pHeld);
	pIndex->Put("Too", pOther);
	pHeld = NULL;
	pOther = NULL;
	AssertWayIn(pIndex->KillAll());
	AssertWayIn(pMap->Put(apNodes[0], apNodes[1]));
	AssertWayIn(pMap->TouchAll());
	AssertWayIn(pMap->Remove(apNodes[0]));
	pHeld = OMalloc<CTestThreadNode>();
	pOther = OMalloc<CTestThreadNode>();
	pMap->Put(pHeld, pOther);
	pHeld = NULL;
	pOther = NULL;
	AssertWayIn(pMap->KillAll());

	//An array of weak pointers.
	AssertWayIn(pWeakArray->Add(&apNodes[0]));
	AssertWayIn(pWeakArray->Add(apNodes[1]));
	AssertWayInReturning(pHeld, pWeakArray->Get(0));
	AssertTrue(pHeld == apNodes[0]);
	pHeld = NULL;
	AssertWayIn(pWeakArray->RemoveAt(0));
	AssertWayIn(pWeakArray->Remove(&apNodes[0]));
	AssertWayIn(pWeakArray->Compact());
	pWeakArray->Add(apNodes[2]);
	AssertWayIn(pWeakArray->RemoveAll());

	//The root.
	AssertWayIn(uiCount = pRoot->NumObjects());
	AssertSize(6, uiCount);
	AssertWayInReturning(pFound, pRoot->UnsafeGet(0));
	AssertTrue(pFound.IsNotNull());
	AssertWayInReturning(pFound, pRoot->Get("Nothing by this name"));
	AssertTrue(pFound.IsNull());
	AssertWayInReturning(pAll, pRoot->GetAll());
	AssertSize(6, pAll->NumElements());
	pAll = NULL;
	AssertWayIn(bHollow = pRoot->IsSetHollow());
	AssertFalse(bHollow);
	AssertWayIn(pRoot->TouchAll());
	AssertWayIn(pRoot->Remove(pOtherArray));
	AssertWayIn(pRoot->Remove((CBaseObject*)&pWeakArray));

	for (i = 0; i < 4; i++)
	{
		apNodes[i] = NULL;
	}
	pArray = NULL;
	pOtherArray = NULL;
	pSet = NULL;
	pIndex = NULL;
	pMap = NULL;
	pWeakArray = NULL;
	AssertWayIn(pRoot->RemoveAll());
	pRoot = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// CObjects' ways in (those that need objects in a database among them).
//
//////////////////////////////////////////////////////////////////////////
static void TestObjectsThreadsEveryObjectsWayIn(void)
{
	Ptr<CString>			pStored;
	Ptr<CString>			pOther;
	Ptr<CRoot>				pRoot;
	Ptr<CFreeObjects>		pFreeObjects;
	CPointer				pFound;
	CBaseObject*			pcObject;
	CArrayOIndex			aoi;
	CArrayBaseObjectPtr		apcNone;
	SIndexesIterator		sIter;
	CChars					sz;
	OIndex					oi;
	OIndex					oiStored;
	OIndex					oiOther;
	size					uiCount;
	int64					iCount;
	bool					bResult;

	AssertWayInReturning(pRoot, gcObjects.Root());
	AssertWayInReturning(pStored, gcObjects.Malloc<CString>("Stored"));
	pStored->Init("kept");
	pOther = ONMalloc<CString>("Other", "too");
	pRoot->Add(pStored);
	pRoot->Add(pOther);
	oiStored = pStored.GetIndex();
	oiOther = pOther.GetIndex();

	AssertWayIn(bResult = gcObjects.Flush());
	AssertTrue(bResult);
	AssertWayIn(bResult = gcObjects.ValidateNoDirty());
	AssertTrue(bResult);
	AssertWayIn(bResult = pStored->Flush());
	AssertTrue(bResult);
	AssertWayIn(bResult = gcObjects.ForceSave(&pStored));
	AssertTrue(bResult);
	AssertWayIn(gcObjects.LogicalDirty(&pStored));
	aoi.Init();
	AssertWayIn(bResult = gcObjects.GetStoredHeapFroms(&pStored, &aoi));
	AssertTrue(bResult);
	aoi.ReInit();
	AssertWayIn(bResult = gcObjects.LoadForward(&pStored, &aoi, 10));
	AssertTrue(bResult);
	aoi.Kill();

	//Found by index and by name, and counted.
	AssertWayInReturning(pFound, gcObjects.Get(oiStored));
	AssertTrue(pFound == pStored);
	AssertWayInReturning(pFound, gcObjects.Get("Stored"));
	AssertTrue(pFound == pStored);
	AssertWayInReturning(pFound, gcObjects.GetPossiblyHollow(oiOther));
	AssertTrue(pFound == pOther);
	AssertWayInReturning(pFound, gcObjects.GetPossiblyHollow("Other"));
	AssertTrue(pFound == pOther);
	AssertWayInReturning(pFound, gcObjects.TestGetFromMemory(oiStored));
	AssertWayInReturning(pFound, gcObjects.TestGetFromMemory("Stored"));
	pFound = NULL;
	AssertWayIn(bResult = gcObjects.Contains(oiStored));
	AssertTrue(bResult);
	AssertWayIn(bResult = gcObjects.Contains("Stored"));
	AssertTrue(bResult);
	AssertWayIn(uiCount = gcObjects.NumMemoryIndexes());
	AssertSize(4, uiCount);
	AssertWayIn(uiCount = gcObjects.NumMemoryNames());
	AssertSize(3, uiCount);
	AssertWayIn(iCount = gcObjects.NumDatabaseIndexes());
	AssertLong(4, iCount);
	AssertWayIn(iCount = gcObjects.NumDatabaseNames());
	AssertLong(3, iCount);
	AssertWayIn(oi = gcObjects.StartMemoryIteration(&sIter));
	AssertTrue(oi != INVALID_O_INDEX);
	AssertWayIn(oi = gcObjects.IterateMemory(&sIter));
	AssertWayIn(uiCount = gcObjects.NumWaitingToBeFreed());
	AssertSize(0, uiCount);
	AssertWayIn(gcObjects.Collect());
	AssertWayIn(uiCount = gcObjects.NumForwarders());
	AssertSize(0, uiCount);
	AssertWayIn(gcObjects.FreeForwarders());

	//Written out and checked.
	sz.Init();
	AssertWayIn(gcObjects.PrintMemoryNames(&sz));
	AssertWayIn(gcObjects.PrintMemoryUseIteration(&sz));
	AssertWayIn(gcObjects.PrintMemoryPages(&sz));
	AssertWayIn(gcObjects.PrintStackPointers(&sz));
	AssertWayIn(gcObjects.PrintGraph(&sz));
	AssertTrue(sz.Length() > 0);
	sz.Kill();
	gcObjects.EnableValidation();
	AssertWayIn(gcObjects.ValidateObjectsConsistency());
	gcObjects.DisableValidation();

	//Objects kept without a holder.
	AssertWayInReturning(pFreeObjects, gcObjects.FreeObjects());
	pFreeObjects = NULL;
	AssertWayIn(pcObject = gcObjects.AddFreeObject(pOther));
	AssertWayIn(uiCount = gcObjects.NumFreeObjects());
	AssertSize(1, uiCount);
	AssertWayIn(bResult = gcObjects.RemoveFreeObject(pcObject));
	AssertTrue(bResult);

	//A batch of none removed, an object made by class name, and one made in place of another by name.
	apcNone.Init();
	AssertWayIn(bResult = gcObjects.Remove(&apcNone, &apcNone));
	AssertTrue(bResult);
	apcNone.Kill();
	AssertWayIn(pcObject = gcObjects.AllocateUninitialisedByClassNameAndAddIntoMemory("CString", ""));
	pFound = pcObject;
	((CString*)pcObject)->Init("made by name");
	pFound = NULL;
	AssertWayIn(pcObject = gcObjects.AllocateReplacingNamed("CString", "Other"));
	pFound = pcObject;
	((CString*)pcObject)->Init("in its place");
	pOther = pFound;
	pFound = NULL;

	//Put out of memory and loaded again (once the root's objects are counted they are in memory, hollow until they are
	//used), and deleted from the database.
	pStored = NULL;
	pOther = NULL;
	pRoot = NULL;
	AssertWayIn(bResult = gcObjects.EvictInMemory());
	AssertTrue(bResult);
	pRoot = gcObjects.Root();
	AssertSize(3, pRoot->NumObjects());  //The two and the free objects.
	pFound = gcObjects.GetPossiblyHollow(oiStored);
	AssertTrue(pFound.IsHollow());
	AssertWayIn(pcObject = gcObjects.Dehollow(oiStored));
	AssertNotNull(pcObject);
	AssertFalse(pFound.IsHollow());
	pFound = NULL;
	pRoot = NULL;
	gcObjects.EvictInMemory();
	pRoot = gcObjects.Root();
	AssertSize(3, pRoot->NumObjects());  //The two and the free objects.
	pFound = gcObjects.GetPossiblyHollow(oiStored);
	AssertTrue(pFound.IsHollow());
	AssertWayIn(pcObject = gcObjects.Dehollow("Stored"));
	AssertNotNull(pcObject);
	AssertFalse(pFound.IsHollow());
	pFound = NULL;
	pRoot = NULL;
	gcObjects.EvictInMemory();
	pRoot = gcObjects.Root();
	AssertSize(3, pRoot->NumObjects());  //The two and the free objects.
	pFound = gcObjects.GetPossiblyHollow(oiStored);
	AssertTrue(pFound.IsHollow());
	AssertWayIn(pcObject = gcObjects.Dehollow((CHollowObject*)pFound.Object()));
	AssertNotNull(pcObject);
	AssertFalse(pFound.IsHollow());
	pStored = pFound;
	pFound = NULL;
	AssertString("kept", pStored->Text());

	AssertWayIn(gcObjects.Discard());
	pOther = gcObjects.Get<CString>("Other");
	AssertWayIn(bResult = pOther->Destroy());
	AssertTrue(bResult);
	AssertTrue(pOther.IsNull());
	aoi.Init();
	aoi.Add(oiStored);
	AssertWayIn(bResult = gcObjects.Destroy(&aoi));
	AssertTrue(bResult);
	AssertTrue(pStored.IsNull());
	aoi.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Every way into the object system takes its lock: each method ThreadSafetyPlan.txt's audit calls a writer or a reader
// is called here with the locking on, and is seen to take the lock, and none of the machinery behind them is reached
// without it.  (One thread is enough to tell: a way in without its guard reaches the machinery with the lock not
// held.)  A method that is only another's with its arguments changed is not here.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsEveryWayIn(void)
{
	CCodabase*		pcDatabase;
	CSequence*		pcSequence;
	CFileUtil		cFileUtil;

	cFileUtil.RemoveDir(TEST_OBJECTS_THREADS_DIRECTORY);
	pcSequence = CSequenceFactory::Create(TEST_OBJECTS_THREADS_DIRECTORY, DS_No);
	pcDatabase = CCodabaseFactory::Create(TEST_OBJECTS_THREADS_DIRECTORY, IWT_No, DS_No);
	AssertTrue(pcDatabase->Open());
	ObjectsInit(pcDatabase, pcSequence);
	gcObjects.DisableValidation();
	AssertTrue(ObjectsLocking(true));
	AssertSize(0, ObjectsNumThreadErrors());
	AssertWayIn(gcObjects.ValidateEmpty());

	TestObjectsThreadsEveryPointerWayIn();
	TestObjectsThreadsEveryCollectionWayIn();
	TestObjectsThreadsEveryObjectsWayIn();

	AssertSize(0, ObjectsNumThreadErrors());
	AssertTrue(ObjectsLocking(false));
	gcObjects.EnableValidation();
	ObjectsFlush();
	ObjectsKill(false);
	AssertTrue(pcDatabase->Close());
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	cFileUtil.RemoveDir(TEST_OBJECTS_THREADS_DIRECTORY);
}


static int32	giTestObjectsThreadsHolding = 0;	//The holding thread has its Ptr.


//Takes a Ptr to the stored object that the home thread holds too, and keeps it until it is released.
static void TestObjectsThreadsHoldStored(void* pvStored)
{
	if (ObjectsThreadInit())
	{
		{
			Ptr<CString>	pHeld = (CString*)pvStored;

			AtomicStore(&giTestObjectsThreadsHolding, 1);
			while (AtomicLoad(&giTestObjectsThreadsRelease) == 0)
			{
				ThreadYield();
			}
		}
		ObjectsThreadKill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Threads share stored objects, as any (two threads hold this one), but objects aren't evicted while another thread
// holds a pointer: eviction puts hollows in place of everything in memory, and is refused.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsStoredObjects(void)
{
	CNativeThread	cThread;
	CCodabase*		pcDatabase;
	CSequence*		pcSequence;
	Ptr<CString>	pStored;
	CFileUtil		cFileUtil;
	CMemoryFile		cLog;
	SLogConfig		sLogConfig;
	char			szLog[2048];

	cFileUtil.RemoveDir(TEST_OBJECTS_THREADS_DIRECTORY);
	pcSequence = CSequenceFactory::Create(TEST_OBJECTS_THREADS_DIRECTORY, DS_No);
	pcDatabase = CCodabaseFactory::Create(TEST_OBJECTS_THREADS_DIRECTORY, IWT_No, DS_No);
	AssertTrue(pcDatabase->Open());
	ObjectsInit(pcDatabase, pcSequence);

	pStored = ONMalloc<CString>("Stored", "kept");
	ORoot()->Add(pStored);
	AssertTrue(ObjectsFlush());
	AssertTrue(ObjectsLocking(true));

	AtomicStore(&giTestObjectsThreadsHolding, 0);
	AtomicStore(&giTestObjectsThreadsRelease, 0);
	StartCapturingLog(&cLog, &sLogConfig);
	cThread.Init();
	cThread.Start(TestObjectsThreadsHoldStored, &pStored);
	while (AtomicLoad(&giTestObjectsThreadsHolding) == 0)
	{
		ThreadYield();
	}
	AssertFalse(gcObjects.EvictInMemory());
	AtomicStore(&giTestObjectsThreadsRelease, 1);
	cThread.Join();
	cThread.Kill();
	StopCapturingLog(&cLog, &sLogConfig, szLog, sizeof(szLog));
	AssertTrue(strstr(szLog, "Cannot put hollows in place of objects while other threads hold [1] stack pointers") != NULL);

	//With the other thread gone its objects can be evicted, and are there to be loaded again.
	AssertString("kept", pStored->Text());
	pStored = NULL;
	AssertTrue(gcObjects.EvictInMemory());
	AssertLong(0, gcObjects.NumMemoryIndexes());
	pStored = gcObjects.Get<CString>("Stored");
	AssertTrue(pStored.IsNotNull());
	AssertString("kept", pStored->Text());
	pStored = NULL;

	AssertTrue(ObjectsLocking(false));
	AssertTrue(ObjectsFlush());
	ObjectsKill(false);
	AssertTrue(pcDatabase->Close());
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	cFileUtil.RemoveDir(TEST_OBJECTS_THREADS_DIRECTORY);
}


//What the home thread and one other do in turn: the home thread asks for a step and waits for it to be done.
struct STestObjectsSteps
{
	CNativeThread		cThread;
	int32				iAsked;
	int32				iDone;
	CTestThreadNode*	apcNodes[4];
	CPointer*			ppOnHeap;
	bool				bRegistered;
	bool				bUnregistered;
	bool				bNullOnceKilled;
	bool				bHollow;
	bool				bLoaded;
};


static void TestObjectsStepWait(STestObjectsSteps* psSteps, int32 iStep)
{
	while (AtomicLoad(&psSteps->iAsked) < iStep)
	{
		ThreadYield();
	}
}


static void TestObjectsStep(STestObjectsSteps* psSteps, int32 iStep)
{
	AtomicStore(&psSteps->iAsked, iStep);
	while (AtomicLoad(&psSteps->iDone) < iStep)
	{
		ThreadYield();
	}
}


//The stack froms an object is said to have, asked as the object system asks.
static size TestObjectsNumStackFroms(CTestThreadNode* pcNode)
{
	size	uiNumStackFroms;

	ObjectsLock();
	uiNumStackFroms = pcNode->NumStackFroms();
	ObjectsUnlock();
	return uiNumStackFroms;
}


static void TestObjectsThreadsKeeper(void* pvSteps)
{
	STestObjectsSteps*	psSteps;

	psSteps = (STestObjectsSteps*)pvSteps;
	psSteps->bRegistered = ObjectsThreadInit();
	{
		Ptr<CTestThreadNode>	pHeld;

		TestObjectsStepWait(psSteps, 1);
		pHeld = psSteps->apcNodes[0];
		AtomicStore(&psSteps->iDone, 1);

		TestObjectsStepWait(psSteps, 2);
		pHeld = NULL;
		AtomicStore(&psSteps->iDone, 2);

		TestObjectsStepWait(psSteps, 3);
		pHeld = psSteps->apcNodes[1];
		AtomicStore(&psSteps->iDone, 3);

		TestObjectsStepWait(psSteps, 4);
		psSteps->bNullOnceKilled = pHeld.IsNull();
		psSteps->ppOnHeap = new CPointer(psSteps->apcNodes[2]);
		AtomicStore(&psSteps->iDone, 4);

		//From the third to the fourth as a writer, which keeps a from of each until the old one goes.
		TestObjectsStepWait(psSteps, 5);
		pHeld = psSteps->apcNodes[2];
		ObjectsLock();
		pHeld = psSteps->apcNodes[3];
		ObjectsUnlock();
		AtomicStore(&psSteps->iDone, 5);

		TestObjectsStepWait(psSteps, 6);
	}
	psSteps->bUnregistered = ObjectsThreadKill();
	AtomicStore(&psSteps->iDone, 6);
}


//////////////////////////////////////////////////////////////////////////
//
// A registered thread keeps its own stack froms of objects attached to the root (level 2), and the object system
// asks it for them.  An object taken off the root while such a thread holds it stays, with a stack from to its
// name, until the thread lets go; one killed while it is held leaves the thread's pointer NULL; a pointer one thread
// took and another lets go of is found; a pointer moved from one to another with the lock held keeps the right one;
// and they are written out with the rest.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsKeptStackFroms(void)
{
	STestObjectsSteps		sSteps;
	Ptr<CTestThreadNode>	pNode;
	CChars					sz;
	size					i;

	TestObjectsThreadsBegin();
	for (i = 0; i < 4; i++)
	{
		pNode = OMalloc<CTestThreadNode>();
		ORoot()->Add(pNode);
		sSteps.apcNodes[i] = &pNode;
	}
	pNode = NULL;
	sSteps.iAsked = 0;
	sSteps.iDone = 0;
	sSteps.ppOnHeap = NULL;
	sSteps.bNullOnceKilled = false;
	sSteps.cThread.Init();
	sSteps.cThread.Start(TestObjectsThreadsKeeper, &sSteps);

	//The thread takes a pointer to the first, which is the first's only stack from, and is kept by the thread.
	TestObjectsStep(&sSteps, 1);
	AssertSize(1, TestObjectsNumStackFroms(sSteps.apcNodes[0]));
	AssertSize(0, gcStackPointers.NumElements());
	sz.Init();
	gcObjects.PrintStackPointers(&sz, false);
	AssertTrue(sz.Contains("] kept by thread "));
	sz.Kill();
	gcObjects.EnableValidation();
	gcObjects.ValidateObjectsConsistency();
	gcObjects.DisableValidation();

	//Off the root it is held by that alone,
	pNode = sSteps.apcNodes[0];
	ORoot()->Remove(pNode);
	pNode = NULL;
	ObjectsCollect();
	AssertInt(0, giTestThreadNodesFreed);
	AssertTrue(sSteps.apcNodes[0]->IsLive());
	AssertSize(1, TestObjectsNumStackFroms(sSteps.apcNodes[0]));

	//until the thread lets go: the thread finds it dead and this one frees it.
	TestObjectsStep(&sSteps, 2);
	ObjectsCollect();
	AssertInt(1, giTestThreadNodesFreed);

	//The second is killed while the thread holds it: the thread's pointer is NULL.
	TestObjectsStep(&sSteps, 3);
	AssertSize(1, TestObjectsNumStackFroms(sSteps.apcNodes[1]));
	sSteps.apcNodes[1]->Kill();
	AssertInt(2, giTestThreadNodesFreed);
	TestObjectsStep(&sSteps, 4);
	AssertTrue(sSteps.bNullOnceKilled);

	//The thread has made a pointer to the third that this thread lets go of.
	AssertSize(1, TestObjectsNumStackFroms(sSteps.apcNodes[2]));
	delete sSteps.ppOnHeap;
	AssertSize(0, TestObjectsNumStackFroms(sSteps.apcNodes[2]));
	AssertTrue(sSteps.apcNodes[2]->IsLive());

	//It points a pointer from the third to the fourth with the lock held: the third's from is the one that goes.  And it
	//lets go of the fourth as its own function ends.
	TestObjectsStep(&sSteps, 5);
	AssertSize(0, TestObjectsNumStackFroms(sSteps.apcNodes[2]));
	AssertSize(1, TestObjectsNumStackFroms(sSteps.apcNodes[3]));
	TestObjectsStep(&sSteps, 6);
	sSteps.cThread.Join();
	sSteps.cThread.Kill();
	AssertTrue(sSteps.bRegistered);
	AssertTrue(sSteps.bUnregistered);
	AssertSize(0, TestObjectsNumStackFroms(sSteps.apcNodes[3]));

	ORoot()->RemoveAll();
	AssertInt(4, giTestThreadNodesFreed);
	TestObjectsThreadsEnd(2);
}


static void TestObjectsThreadsLoader(void* pvSteps)
{
	STestObjectsSteps*	psSteps;

	psSteps = (STestObjectsSteps*)pvSteps;
	psSteps->bRegistered = ObjectsThreadInit();
	{
		Ptr<CRoot>		pRoot;
		Ptr<CString>	pStored;

		pRoot = ORoot();
		pStored = pRoot->Get("Stored");
		psSteps->bHollow = pStored.IsHollow();
		psSteps->bLoaded = (strcmp(pStored->Text(), "kept") == 0) && !pStored.IsHollow();
	}
	psSteps->bUnregistered = ObjectsThreadKill();
}


//////////////////////////////////////////////////////////////////////////
//
// A hollow that is loaded through a pointer a registered thread keeps the from of: the pointer points at what was
// loaded, and is a from of it.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsKeptStackFromOfHollow(void)
{
	STestObjectsSteps	sSteps;
	CCodabase*			pcDatabase;
	CSequence*			pcSequence;
	Ptr<CString>		pStored;
	CFileUtil			cFileUtil;

	cFileUtil.RemoveDir(TEST_OBJECTS_THREADS_DIRECTORY);
	pcSequence = CSequenceFactory::Create(TEST_OBJECTS_THREADS_DIRECTORY, DS_No);
	pcDatabase = CCodabaseFactory::Create(TEST_OBJECTS_THREADS_DIRECTORY, IWT_No, DS_No);
	AssertTrue(pcDatabase->Open());
	ObjectsInit(pcDatabase, pcSequence);

	pStored = ONMalloc<CString>("Stored", "kept");
	ORoot()->Add(pStored);
	pStored = NULL;
	AssertTrue(ObjectsFlush());
	AssertTrue(gcObjects.EvictInMemory());
	AssertLong(0, gcObjects.NumMemoryIndexes());
	AssertTrue(ObjectsLocking(true));

	sSteps.bHollow = false;
	sSteps.bLoaded = false;
	sSteps.cThread.Init();
	sSteps.cThread.Start(TestObjectsThreadsLoader, &sSteps);
	sSteps.cThread.Join();
	sSteps.cThread.Kill();
	AssertTrue(sSteps.bRegistered);
	AssertTrue(sSteps.bUnregistered);
	AssertTrue(sSteps.bHollow);
	AssertTrue(sSteps.bLoaded);
	AssertSize(0, ObjectsNumThreadErrors());

	pStored = gcObjects.Get<CString>("Stored");
	AssertString("kept", pStored->Text());
	AssertSize(1, pStored.NumStackFroms());
	pStored = NULL;

	AssertTrue(ObjectsLocking(false));
	gcObjects.ValidateObjectsConsistency();
	AssertTrue(ObjectsFlush());
	ObjectsKill(false);
	AssertTrue(pcDatabase->Close());
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	cFileUtil.RemoveDir(TEST_OBJECTS_THREADS_DIRECTORY);
}


//Registers, comes into the object system once when asked (step 2), and unregisters (step 3).
static void TestObjectsThreadsComesIn(void* pvSteps)
{
	STestObjectsSteps*	psSteps;

	psSteps = (STestObjectsSteps*)pvSteps;
	psSteps->bRegistered = ObjectsThreadInit();
	AtomicStore(&psSteps->iDone, 1);

	TestObjectsStepWait(psSteps, 2);
	psSteps->bLoaded = gcObjects.NumMemoryIndexes() > 0;
	AtomicStore(&psSteps->iDone, 2);

	TestObjectsStepWait(psSteps, 3);
	psSteps->bUnregistered = ObjectsThreadKill();
	AtomicStore(&psSteps->iDone, 3);
}


static void TestObjectsThreadsComesInStart(STestObjectsSteps* psSteps)
{
	psSteps->iAsked = 0;
	psSteps->iDone = 0;
	psSteps->bRegistered = false;
	psSteps->bUnregistered = false;
	psSteps->bLoaded = false;
	psSteps->cThread.Init();
	psSteps->cThread.Start(TestObjectsThreadsComesIn, psSteps);
	TestObjectsStep(psSteps, 1);
	AssertTrue(psSteps->bRegistered);
}


static void TestObjectsThreadsComesInEnd(STestObjectsSteps* psSteps)
{
	TestObjectsStep(psSteps, 3);
	psSteps->cThread.Join();
	psSteps->cThread.Kill();
	AssertTrue(psSteps->bUnregistered);
}


//////////////////////////////////////////////////////////////////////////
//
// A hollow loaded while another thread is registered forwards: a thread
// that read a pointer to it before it was loaded (a plain pointer kept,
// here) asks it for its object and is given what it was loaded as, with
// nothing loaded again, and a pointer pointed at that plain pointer points
// at what it was loaded as too.  It is freed once every other thread has
// come into the object system since the home thread marked them
// (ObjectsCollect, which marks the first time and frees the next), and not
// before; a thread that has unregistered doesn't hold it up.  With no
// other thread registered a loaded hollow is freed at once, and switching
// the locking off frees what forwarders are left.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsForwardingHollow(void)
{
	STestObjectsSteps	sSteps;
	CCodabase*			pcDatabase;
	CSequence*			pcSequence;
	CFileUtil			cFileUtil;
	Ptr<CRoot>			pRoot;
	Ptr<CString>		pStored;
	Ptr<CString>		pHeld;
	CEmbeddedObject*	pcHollow;
	size				uiIndexes;

	cFileUtil.RemoveDir(TEST_OBJECTS_THREADS_DIRECTORY);
	pcSequence = CSequenceFactory::Create(TEST_OBJECTS_THREADS_DIRECTORY, DS_No);
	pcDatabase = CCodabaseFactory::Create(TEST_OBJECTS_THREADS_DIRECTORY, IWT_No, DS_No);
	AssertTrue(pcDatabase->Open());
	ObjectsInit(pcDatabase, pcSequence);

	pRoot = ORoot();
	pStored = ONMalloc<CString>("One", "first");
	pRoot->Add(pStored);
	pStored = ONMalloc<CString>("Two", "second");
	pRoot->Add(pStored);
	pStored = ONMalloc<CString>("Three", "third");
	pRoot->Add(pStored);
	pStored = ONMalloc<CString>("Four", "fourth");
	pRoot->Add(pStored);
	pStored = NULL;
	pRoot = NULL;
	AssertTrue(ObjectsFlush());
	AssertTrue(gcObjects.EvictInMemory());
	AssertTrue(ObjectsLocking(true));

	TestObjectsThreadsComesInStart(&sSteps);
	//Getting One loads the root's set, which was a hollow too: it forwards as well.
	pRoot = ORoot();
	pStored = pRoot->Get("One");
	AssertTrue(pStored.IsHollow());
	AssertSize(1, gcObjects.NumForwarders());
	pcHollow = pStored.Object();
	uiIndexes = gcObjects.NumMemoryIndexes();
	AssertString("first", pStored->Text());
	AssertFalse(pStored.IsHollow());
	AssertSize(uiIndexes, gcObjects.NumMemoryIndexes());
	AssertSize(2, gcObjects.NumForwarders());
	AssertSize(2, guiObjectsForwarders);  //What pointing a pointer looks at first.
	AssertPointer(pStored.Object(), pcHollow->Dehollow());
	AssertSize(uiIndexes, gcObjects.NumMemoryIndexes());

	//A pointer pointed at the plain pointer kept to the hollow is pointed at what it was loaded as.
	pHeld = pcHollow;
	AssertFalse(pHeld.IsHollow());
	AssertPointer(pStored.Object(), pHeld.Object());
	pHeld = NULL;

	//Marked, then kept while the other thread hasn't come in, then freed once it has.
	ObjectsCollect();
	AssertSize(2, gcObjects.NumForwarders());
	ObjectsCollect();
	AssertSize(2, gcObjects.NumForwarders());
	TestObjectsStep(&sSteps, 2);
	AssertTrue(sSteps.bLoaded);
	ObjectsCollect();
	AssertSize(0, gcObjects.NumForwarders());
	TestObjectsThreadsComesInEnd(&sSteps);

	pStored = pRoot->Get("Two");
	AssertTrue(pStored.IsHollow());
	AssertString("second", pStored->Text());
	AssertSize(0, gcObjects.NumForwarders());

	//A thread that has gone no longer holds them up.
	TestObjectsThreadsComesInStart(&sSteps);
	pStored = pRoot->Get("Three");
	AssertString("third", pStored->Text());
	AssertSize(1, gcObjects.NumForwarders());
	ObjectsCollect();
	ObjectsCollect();
	AssertSize(1, gcObjects.NumForwarders());
	TestObjectsThreadsComesInEnd(&sSteps);
	AssertSize(1, gcObjects.NumForwarders());
	ObjectsCollect();
	AssertSize(0, gcObjects.NumForwarders());

	TestObjectsThreadsComesInStart(&sSteps);
	pStored = pRoot->Get("Four");
	AssertString("fourth", pStored->Text());
	AssertSize(1, gcObjects.NumForwarders());
	TestObjectsThreadsComesInEnd(&sSteps);
	AssertSize(0, ObjectsNumThreadErrors());
	AssertTrue(ObjectsLocking(false));
	AssertSize(0, gcObjects.NumForwarders());
	AssertSize(0, guiObjectsForwarders);

	pStored = NULL;
	pRoot = NULL;
	gcObjects.ValidateObjectsConsistency();
	AssertTrue(ObjectsFlush());
	ObjectsKill(false);
	AssertTrue(pcDatabase->Close());
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	cFileUtil.RemoveDir(TEST_OBJECTS_THREADS_DIRECTORY);
}


#define TEST_OBJECTS_THREADS_TREE_DEPTH		6
#define TEST_OBJECTS_THREADS_TREE_NODES		((2 << TEST_OBJECTS_THREADS_TREE_DEPTH) - 1)
#define TEST_OBJECTS_THREADS_STORED_ROUNDS	3
#define TEST_OBJECTS_THREADS_MEMBERS		64
#define TEST_OBJECTS_THREADS_PASSES			20


static CCodabase*	gpcTestObjectsDatabase = NULL;
static CSequence*	gpcTestObjectsSequence = NULL;
static size			guiTestObjectsNodes = 0;	//How many nodes a stored test's threads are to reach.


//////////////////////////////////////////////////////////////////////////
//
// The object system over a database of its own, with the locking off:
// what the threads load is made, saved and evicted first.
//
//////////////////////////////////////////////////////////////////////////
static void TestObjectsThreadsBeginStored(void)
{
	CFileUtil	cFileUtil;

	cFileUtil.RemoveDir(TEST_OBJECTS_THREADS_DIRECTORY);
	gpcTestObjectsSequence = CSequenceFactory::Create(TEST_OBJECTS_THREADS_DIRECTORY, DS_No);
	gpcTestObjectsDatabase = CCodabaseFactory::Create(TEST_OBJECTS_THREADS_DIRECTORY, IWT_No, DS_No);
	AssertTrue(gpcTestObjectsDatabase->Open());
	ObjectsInit(gpcTestObjectsDatabase, gpcTestObjectsSequence);
	gcObjects.AddConstructor<CTestThreadNode>();
	gcObjects.DisableValidation();
	giTestThreadNodesFreed = 0;
	giTestThreadNodesFreedOffHome = 0;
	giTestThreadNeedersFreedLate = 0;
}


//The threads are done and the locking is off.
static void TestObjectsThreadsEndStored(void)
{
	CFileUtil	cFileUtil;

	AssertInt(0, giTestThreadNodesFreedOffHome);
	AssertSize(0, ObjectsNumThreadErrors());
	AssertSize(0, gcObjects.NumForwarders());
	gcObjects.EnableValidation();
	gcObjects.ValidateObjectsConsistency();
	AssertTrue(ObjectsFlush());
	ObjectsKill(false);
	AssertTrue(gpcTestObjectsDatabase->Close());
	SafeKill(gpcTestObjectsDatabase);
	SafeKill(gpcTestObjectsSequence);
	cFileUtil.RemoveDir(TEST_OBJECTS_THREADS_DIRECTORY);
}


//A round of a stored test: everything in memory evicted, then the threads let loose on the database with the
//locking on.  The home thread does fWaiting while it waits, and frees the forwarders once they have gone.
static int TestObjectsThreadsStoredRound(NativeThreadFunction fRun, NativeThreadFunction fLast, TestObjectsHomeWork fWaiting)
{
	int		iErrors;

	AssertTrue(gcObjects.EvictInMemory());
	AssertTrue(ObjectsLocking(true));
	gfTestObjectsThreadsWaiting = fWaiting;
	iErrors = TestObjectsRunThreads(fRun, fLast, NULL, NULL, NULL);
	gfTestObjectsThreadsWaiting = NULL;
	ObjectsCollect();
	AssertSize(0, gcObjects.NumForwarders());
	AssertTrue(ObjectsLocking(false));
	return iErrors;
}


//A tree below pNode, uiDepth deep: its mpNext and mpOther are its children.  Returns how many nodes it has.
static size TestObjectsThreadsMakeTree(Ptr<CTestThreadNode>& pNode, size uiDepth)
{
	Ptr<CTestThreadNode>	pChild;
	size					uiNodes;

	uiNodes = 1;
	if (uiDepth > 0)
	{
		pChild = OMalloc<CTestThreadNode>();
		pNode->mpNext = pChild;
		uiNodes += TestObjectsThreadsMakeTree(pChild, uiDepth - 1);
		pChild = OMalloc<CTestThreadNode>();
		pNode->mpOther = pChild;
		uiNodes += TestObjectsThreadsMakeTree(pChild, uiDepth - 1);
	}
	return uiNodes;
}


//The nodes of the tree below pNode, each reached through its pointers and so loaded if it is a hollow.  A node that
//isn't live is an error.
static size TestObjectsThreadsWalkTree(Ptr<CTestThreadNode>& pNode, int* piErrors)
{
	Ptr<CTestThreadNode>	pChild;
	size					uiNodes;

	if (pNode.IsNull())
	{
		return 0;
	}
	if (!pNode->IsLive())
	{
		(*piErrors)++;
	}
	uiNodes = 1;
	pChild = pNode->mpNext;
	uiNodes += TestObjectsThreadsWalkTree(pChild, piErrors);
	pChild = pNode->mpOther;
	uiNodes += TestObjectsThreadsWalkTree(pChild, piErrors);
	return uiNodes;
}


static void TestObjectsThreadsLoadTree(void* pvThread)
{
	STestObjectsThread*		psThread;
	Ptr<CTestThreadNode>	pTop;

	psThread = (STestObjectsThread*)pvThread;
	pTop = ORoot()->Get("Top");
	if (TestObjectsThreadsWalkTree(pTop, &psThread->iErrors) != guiTestObjectsNodes)
	{
		psThread->iErrors++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Every thread walks one stored tree at once, loading it as it goes, so
// most of its hollows are asked for by several threads together.  Each
// object is loaded once (memory holds what it held before it was
// evicted), every thread reaches every node, the distances are right and
// no forwarder is left.  Three rounds, the tree evicted between them.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsLoadStoredTree(void)
{
	size	uiObjects;
	size	i;

	TestObjectsThreadsBeginStored();
	{
		Ptr<CTestThreadNode>	pTop;

		pTop = ONMalloc<CTestThreadNode>("Top");
		ORoot()->Add(pTop);
		guiTestObjectsNodes = TestObjectsThreadsMakeTree(pTop, TEST_OBJECTS_THREADS_TREE_DEPTH);
	}
	AssertTrue(ObjectsFlush());
	uiObjects = gcObjects.NumMemoryIndexes();

	for (i = 0; i < TEST_OBJECTS_THREADS_STORED_ROUNDS; i++)
	{
		AssertInt(0, TestObjectsThreadsStoredRound(TestObjectsThreadsLoadTree, NULL, ObjectsCollect));
		AssertSize(uiObjects, gcObjects.NumMemoryIndexes());
		gcObjects.EnableValidation();
		gcObjects.ValidateObjectsConsistency();
		gcObjects.DisableValidation();
	}
	TestObjectsThreadsEndStored();
}


//Reads the set of stored nodes, and the map from them, over and over: each pass sees every member, and finds each
//one's value, while other threads load them.
static void TestObjectsThreadsReadStored(void* pvThread)
{
	STestObjectsThread*		psThread;
	Ptr<CRoot>				pRoot;
	Ptr<CSetObject>			pSet;
	Ptr<CMapObject>			pMap;
	SSetIterator			sIter;
	CPointer				pMember;
	size					uiMembers;
	size					i;

	psThread = (STestObjectsThread*)pvThread;
	pRoot = ORoot();
	pSet = pRoot->Get("Set");
	pMap = pRoot->Get("Map");
	for (i = 0; i < TEST_OBJECTS_THREADS_PASSES; i++)
	{
		uiMembers = 0;
		pMember = pSet->StartIteration(&sIter);
		while (pMember.IsNotNull())
		{
			uiMembers++;
			if (pMap->Get(pMember).IsNull())
			{
				psThread->iErrors++;
			}
			pMember = pSet->Iterate(&sIter);
		}
		if (uiMembers != TEST_OBJECTS_THREADS_MEMBERS)
		{
			psThread->iErrors++;
		}
	}
}


//Loads each member of the set, and the value the map has for it.
static void TestObjectsThreadsLoadStored(void* pvThread)
{
	STestObjectsThread*		psThread;
	Ptr<CRoot>				pRoot;
	Ptr<CSetObject>			pSet;
	Ptr<CMapObject>			pMap;
	SSetIterator			sIter;
	Ptr<CTestThreadNode>	pMember;
	Ptr<CTestThreadNode>	pValue;

	psThread = (STestObjectsThread*)pvThread;
	pRoot = ORoot();
	pSet = pRoot->Get("Set");
	pMap = pRoot->Get("Map");
	pMember = pSet->StartIteration(&sIter);
	while (pMember.IsNotNull())
	{
		pValue = pMap->Get(pMember);
		if (!pMember->IsLive() || pValue.IsNull() || !pValue->IsLive())
		{
			psThread->iErrors++;
		}
		pMember = pSet->Iterate(&sIter);
	}
}


//Every other thread reads the set and the map; the rest load what they hold.
static void TestObjectsThreadsReadOrLoadStored(void* pvThread)
{
	STestObjectsThread*		psThread;

	psThread = (STestObjectsThread*)pvThread;
	if ((psThread->uiSeed / 7919) % 2 == 0)
	{
		TestObjectsThreadsReadStored(pvThread);
	}
	else
	{
		TestObjectsThreadsLoadStored(pvThread);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A set of stored nodes and a map keyed by them, read on some threads
// (each member, and its value) while others load the members and the
// values.  A read always sees every member, in order without the set
// being sorted again, and never a forwarding hollow in place of one.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsStoredCollections(void)
{
	size	uiObjects;
	size	i;

	TestObjectsThreadsBeginStored();
	{
		Ptr<CSetObject>			pSet;
		Ptr<CMapObject>			pMap;
		Ptr<CTestThreadNode>	pKey;
		Ptr<CTestThreadNode>	pValue;

		pSet = ONMalloc<CSetObject>("Set");
		pMap = ONMalloc<CMapObject>("Map");
		ORoot()->Add(pSet);
		ORoot()->Add(pMap);
		for (i = 0; i < TEST_OBJECTS_THREADS_MEMBERS; i++)
		{
			pKey = OMalloc<CTestThreadNode>();
			pValue = OMalloc<CTestThreadNode>();
			pSet->Add(pKey);
			pMap->Put(pKey, pValue);
		}
	}
	AssertTrue(ObjectsFlush());
	uiObjects = gcObjects.NumMemoryIndexes();

	for (i = 0; i < TEST_OBJECTS_THREADS_STORED_ROUNDS; i++)
	{
		AssertInt(0, TestObjectsThreadsStoredRound(TestObjectsThreadsReadOrLoadStored, NULL, ObjectsCollect));
		AssertSize(uiObjects, gcObjects.NumMemoryIndexes());
		AssertTrue(gcObjects.Get<CSetObject>("Set")->IsSorted());
	}
	TestObjectsThreadsEndStored();
}


//The tree, walked over and over while the home thread flushes it.
static void TestObjectsThreadsWalkStoredTree(void* pvThread)
{
	size	i;

	for (i = 0; i < TEST_OBJECTS_THREADS_PASSES; i++)
	{
		TestObjectsThreadsLoadTree(pvThread);
	}
}


//The home thread's while the others read: the tree's top changed and everything saved.
static void TestObjectsThreadsFlushTop(void)
{
	gcObjects.Get<CTestThreadNode>("Top").LogicalDirty();
	ObjectsFlush();
	ObjectsCollect();
}


//////////////////////////////////////////////////////////////////////////
//
// The home thread flushes, over and over, while the other threads read
// what it saves (and load it as they go).
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsFlushWhileRead(void)
{
	TestObjectsThreadsBeginStored();
	{
		Ptr<CTestThreadNode>	pTop;

		pTop = ONMalloc<CTestThreadNode>("Top");
		ORoot()->Add(pTop);
		guiTestObjectsNodes = TestObjectsThreadsMakeTree(pTop, TEST_OBJECTS_THREADS_TREE_DEPTH - 2);
	}
	AssertTrue(ObjectsFlush());
	AssertInt(0, TestObjectsThreadsStoredRound(TestObjectsThreadsWalkStoredTree, NULL, TestObjectsThreadsFlushTop));
	TestObjectsThreadsEndStored();
}


static void TestObjectsThreadsHolder(void* pvSteps)
{
	STestObjectsSteps*		psSteps;

	psSteps = (STestObjectsSteps*)pvSteps;
	psSteps->bRegistered = ObjectsThreadInit();
	{
		Ptr<CTestThreadNode>	pHeld;

		pHeld = ORoot()->Get("Held");
		psSteps->bLoaded = pHeld->IsLive();
		AtomicStore(&psSteps->iDone, 1);

		TestObjectsStepWait(psSteps, 2);
		pHeld = NULL;
		AtomicStore(&psSteps->iDone, 2);

		TestObjectsStepWait(psSteps, 3);
	}
	psSteps->bUnregistered = ObjectsThreadKill();
	AtomicStore(&psSteps->iDone, 3);
}


//////////////////////////////////////////////////////////////////////////
//
// A stored object another thread holds isn't killed (its Kill would put
// a hollow in its place under that thread): it is refused, and said.
// Once the thread has let go it is killed, and what held it holds a
// hollow for it.  Another stored object, held by no other thread, is
// killed while the thread holds the first.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsKillStoredHeld(void)
{
	STestObjectsSteps		sSteps;
	Ptr<CTestThreadNode>	pHeld;
	Ptr<CTestThreadNode>	pOther;
	Ptr<CArrayObject>		pHolders;
	CMemoryFile				cLog;
	SLogConfig				sLogConfig;
	char					szLog[1024];

	TestObjectsThreadsBeginStored();
	pHeld = ONMalloc<CTestThreadNode>("Held");
	pOther = ONMalloc<CTestThreadNode>("Other");
	ORoot()->Add(pHeld);
	pHolders = ONMalloc<CArrayObject>("Holders");
	ORoot()->Add(pHolders);
	pHolders->Add(pHeld);
	pHolders->Add(pOther);
	pHeld = NULL;
	pOther = NULL;
	pHolders = NULL;
	AssertTrue(ObjectsFlush());
	AssertTrue(gcObjects.EvictInMemory());
	AssertTrue(ObjectsLocking(true));

	sSteps.iAsked = 0;
	sSteps.iDone = 0;
	sSteps.bLoaded = false;
	sSteps.cThread.Init();
	sSteps.cThread.Start(TestObjectsThreadsHolder, &sSteps);
	TestObjectsStep(&sSteps, 1);
	AssertTrue(sSteps.bRegistered);
	AssertTrue(sSteps.bLoaded);

	pHolders = ORoot()->Get("Holders");
	pHeld = pHolders->Get(0);
	pOther = pHolders->Get(1);
	StartCapturingLog(&cLog, &sLogConfig);
	pHeld->Kill();
	StopCapturingLog(&cLog, &sLogConfig, szLog, sizeof(szLog));
	AssertNotNull(strstr(szLog, "other threads hold"));
	AssertFalse(pHeld.IsHollow());
	AssertTrue(pHeld->IsLive());

	//Killed, what held it (this thread's pointer, the array) holds a hollow for it.
	pOther->Kill();
	AssertTrue(pOther.IsHollow());

	TestObjectsStep(&sSteps, 2);
	pHeld->Kill();
	AssertTrue(pHeld.IsHollow());

	TestObjectsStep(&sSteps, 3);
	sSteps.cThread.Join();
	sSteps.cThread.Kill();
	AssertTrue(sSteps.bUnregistered);
	pHeld = NULL;
	pOther = NULL;
	pHolders = NULL;
	ObjectsCollect();
	AssertTrue(ObjectsLocking(false));
	TestObjectsThreadsEndStored();
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


static CArrayOIndex	gaoiTestObjectsWorkerIndexes;	//Every worker's objects' indexes, under the test's mutex.


//Builds graphs as TestObjectsThreadsBuildGraphs does, in the worker's own object system: the only ways into the main
//one are for blocks of indexes, and the weak pointer's (pointed, got and let go of: a weak pointer doesn't know the
//object it is in, so it takes the main lock).
static void TestObjectsThreadsWorkerGraphs(void* pvThread)
{
	STestObjectsThread*		psThread;
	size					uiWaysIn;
	size					uiObjects;

	psThread = (STestObjectsThread*)pvThread;
	if (ObjectsCurrent() == &gcObjects)
	{
		psThread->iErrors++;
	}
	uiObjects = ObjectsCurrent()->NumMemoryIndexes();
	uiWaysIn = ObjectsNumWaysIn();
	TestObjectsThreadsBuildGraphs(pvThread);
	uiWaysIn = ObjectsNumWaysIn() - uiWaysIn;
	if (uiWaysIn > (TEST_OBJECTS_THREADS_STEPS * 3) + ((TEST_OBJECTS_THREADS_STEPS * 3) / OBJECTS_WORKER_INDEXES) + 1)
	{
		psThread->iErrors++;
	}
	if (ObjectsCurrent()->NumMemoryIndexes() != uiObjects)
	{
		psThread->iErrors++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Workers, each with an object system of its own, build graphs at once and
// let them go: no lock is taken but one for each block of indexes (and
// the weak pointer's), every object is freed once and on its worker, and
// the main object system doesn't change.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsWorkerOwnGraphs(void)
{
	size	uiObjects;

	TestObjectsThreadsBegin();
	ORoot();
	uiObjects = gcObjects.NumMemoryIndexes();

	gbTestObjectsThreadsWorkers = true;
	AssertInt(0, TestObjectsRunThreads(TestObjectsThreadsWorkerGraphs, NULL, NULL, NULL, NULL));
	gbTestObjectsThreadsWorkers = false;

	AssertInt(TEST_OBJECTS_THREADS * TEST_OBJECTS_THREADS_STEPS * 3, giTestThreadNodesFreed);
	TestObjectsThreadsEnd(uiObjects);
}


//A tree in the worker's own root, its indexes noted; left for ObjectsThreadKill.
static void TestObjectsThreadsWorkerLeaves(void* pvThread)
{
	STestObjectsThread*		psThread;
	Ptr<CRoot>				pRoot;
	Ptr<CTestThreadNode>	pTop;
	SIndexesIterator		sIter;
	OIndex					oi;

	psThread = (STestObjectsThread*)pvThread;
	pRoot = ORoot();
	if (pRoot->GetObjectsThisIn() != ObjectsCurrent())
	{
		psThread->iErrors++;
	}
	pTop = OMalloc<CTestThreadNode>();
	pRoot->Add(pTop);
	TestObjectsThreadsMakeTree(pTop, TEST_OBJECTS_THREADS_TREE_DEPTH);

	psThread->pcMutex->Lock();
	oi = ObjectsCurrent()->StartMemoryIteration(&sIter);
	while (oi != INVALID_O_INDEX)
	{
		gaoiTestObjectsWorkerIndexes.Add(oi);
		oi = ObjectsCurrent()->IterateMemory(&sIter);
	}
	psThread->pcMutex->Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
// A worker's objects have their own root, and indexes no other object in
// the process has.  What it leaves in its object system is freed, on the
// worker, when it unregisters.  Validation is on: a worker's object system
// validates as the main one does, and its own objects alone.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsWorkerLeftObjects(void)
{
	CMutex	cMutex;
	size	uiObjects;
	size	i;

	TestObjectsThreadsBegin();
	ORoot();
	uiObjects = gcObjects.NumMemoryIndexes();
	gaoiTestObjectsWorkerIndexes.Init();
	cMutex.Init();

	gcObjects.EnableValidation();
	gbTestObjectsThreadsWorkers = true;
	AssertInt(0, TestObjectsRunThreads(TestObjectsThreadsWorkerLeaves, NULL, &cMutex, NULL, NULL));
	gbTestObjectsThreadsWorkers = false;
	gcObjects.DisableValidation();
	AssertInt(TEST_OBJECTS_THREADS * TEST_OBJECTS_THREADS_TREE_NODES, giTestThreadNodesFreed);

	gaoiTestObjectsWorkerIndexes.QuickSort();
	AssertTrue(gaoiTestObjectsWorkerIndexes.NumElements() > TEST_OBJECTS_THREADS * TEST_OBJECTS_THREADS_TREE_NODES);
	for (i = 1; i < gaoiTestObjectsWorkerIndexes.NumElements(); i++)
	{
		if (gaoiTestObjectsWorkerIndexes.GetValue(i - 1) == gaoiTestObjectsWorkerIndexes.GetValue(i))
		{
			AssertLong(-1, gaoiTestObjectsWorkerIndexes.GetValue(i));
			break;
		}
		AssertFalse(gcObjects.Contains(gaoiTestObjectsWorkerIndexes.GetValue(i)));
	}
	gaoiTestObjectsWorkerIndexes.Kill();
	cMutex.Kill();
	TestObjectsThreadsEnd(uiObjects);
}


//A worker's stack pointers between its own objects and the main object system's shared node, over and over.
static void TestObjectsThreadsWorkerHolds(void* pvThread)
{
	STestObjectsThread*		psThread;
	Ptr<CTestThreadNode>	pHeld;
	Ptr<CTestThreadNode>	pOwn;
	size					i;

	psThread = (STestObjectsThread*)pvThread;
	pOwn = OMalloc<CTestThreadNode>();
	ORoot()->Add(pOwn);
	for (i = 0; i < TEST_OBJECTS_THREADS_STEPS; i++)
	{
		pHeld = psThread->pcShared;
		if (!pHeld->IsLive())
		{
			psThread->iErrors++;
		}
		pHeld = pOwn;
		pHeld = psThread->pcShared;
		pHeld = OMalloc<CTestThreadNode>();
		pHeld = NULL;
	}
}


static CTestThreadNode*	gpcTestObjectsThreadsCounted = NULL;


//The home thread's while the workers move their pointers: the counted node's stack froms, which reads every thread's
//table with the lock.
static void TestObjectsThreadsCountStackFroms(void)
{
	ObjectsLock();
	gpcTestObjectsThreadsCounted->NumStackFroms();
	ObjectsUnlock();
}


//////////////////////////////////////////////////////////////////////////
//
// A worker's stack pointers point at main objects as any registered
// thread's do, and move between its own objects and main ones, while the
// home thread reads the threads' tables.  The main object is held exactly
// while they point at it.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsWorkerHoldsMain(void)
{
	Ptr<CTestThreadNode>	pShared;
	size					uiObjects;

	TestObjectsThreadsBegin();
	pShared = OMalloc<CTestThreadNode>();
	ORoot()->Add(pShared);
	uiObjects = gcObjects.NumMemoryIndexes();

	gpcTestObjectsThreadsCounted = &pShared;
	gfTestObjectsThreadsWaiting = TestObjectsThreadsCountStackFroms;
	gbTestObjectsThreadsWorkers = true;
	AssertInt(0, TestObjectsRunThreads(TestObjectsThreadsWorkerHolds, NULL, NULL, &pShared, NULL));
	gbTestObjectsThreadsWorkers = false;
	gfTestObjectsThreadsWaiting = NULL;
	gpcTestObjectsThreadsCounted = NULL;

	AssertSize(1, pShared.NumStackFroms());
	AssertTrue(pShared->IsLive());
	AssertInt(TEST_OBJECTS_THREADS * (TEST_OBJECTS_THREADS_STEPS + 1), giTestThreadNodesFreed);
	pShared = NULL;
	TestObjectsThreadsEnd(uiObjects);
}


#define TEST_OBJECTS_THREADS_MAIN_NODES	16


//A worker's objects point at the main nodes in pcArray, by their fields and in a collection of the worker's own, and
//are pointed between them and the worker's own objects, over and over.
static void TestObjectsThreadsWorkerPointsAtMain(void* pvThread)
{
	STestObjectsThread*		psThread;
	Ptr<CArrayObject>		pMain;
	size					i;

	psThread = (STestObjectsThread*)pvThread;
	pMain = psThread->pcArray;
	for (i = 0; i < TEST_OBJECTS_THREADS_STEPS; i++)
	{
		Ptr<CTestThreadNode>	pOwn = OMalloc<CTestThreadNode>();
		Ptr<CTestThreadNode>	pOther = OMalloc<CTestThreadNode>();
		Ptr<CArrayObject>		pList = OMalloc<CArrayObject>();
		Ptr<CTestThreadNode>	pMainNode = pMain->Get((i + psThread->uiSeed) % TEST_OBJECTS_THREADS_MAIN_NODES);

		pOwn->mpNext = pMainNode;
		pOwn->mpOther = pOther;
		pOther->mpNext = pOwn;
		pList->Add(pMainNode);
		pList->Add(pOther);
		if (!pOwn->mpNext->IsLive() || !pList->Get<CTestThreadNode>(0)->IsLive())
		{
			psThread->iErrors++;
		}

		//From a main object to one of its own and back, and from one main object to another.
		pOwn->mpNext = pOther;
		pOwn->mpNext = pMainNode;
		pOther->mpOther = pMain->Get((i + 1) % TEST_OBJECTS_THREADS_MAIN_NODES);
		pOther->mpOther = pMain->Get((i + 2) % TEST_OBJECTS_THREADS_MAIN_NODES);
		pList->Remove((size)0);
		pList->Add(pMainNode);
		pOther->mpNext = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Workers' objects point at main objects (decision 11), by their fields and
// in their collections, and are pointed between those and the workers' own
// objects: the main objects are held while they are pointed at, as by the
// workers' stack pointers, and are left as they were, held by main alone.
// Every worker object is freed on its worker.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsWorkerObjectsPointAtMain(void)
{
	Ptr<CArrayObject>		pMain;
	Ptr<CTestThreadNode>	pNode;
	size					uiObjects;
	size					i;

	TestObjectsThreadsBegin();
	pMain = OMalloc<CArrayObject>();
	ORoot()->Add(pMain);
	for (i = 0; i < TEST_OBJECTS_THREADS_MAIN_NODES; i++)
	{
		pNode = OMalloc<CTestThreadNode>();
		pMain->Add(pNode);
	}
	pNode = NULL;
	uiObjects = gcObjects.NumMemoryIndexes();

	gbTestObjectsThreadsWorkers = true;
	gfTestObjectsThreadsWaiting = ObjectsCollect;
	AssertInt(0, TestObjectsRunThreads(TestObjectsThreadsWorkerPointsAtMain, NULL, NULL, NULL, &pMain));
	gfTestObjectsThreadsWaiting = NULL;
	gbTestObjectsThreadsWorkers = false;
	ObjectsCollect();

	AssertInt(TEST_OBJECTS_THREADS * TEST_OBJECTS_THREADS_STEPS * 2, giTestThreadNodesFreed);
	for (i = 0; i < TEST_OBJECTS_THREADS_MAIN_NODES; i++)
	{
		pNode = pMain->Get(i);
		AssertTrue(pNode->IsLive());
		AssertSize(1, pNode.NumStackFroms());
		AssertSize(1, pNode.NumHeapFroms());
	}
	pNode = NULL;
	pMain = NULL;
	TestObjectsThreadsEnd(uiObjects);
}


static void TestObjectsThreadsWorkerHolder(void* pvSteps)
{
	STestObjectsSteps*		psSteps;

	psSteps = (STestObjectsSteps*)pvSteps;
	psSteps->bRegistered = ObjectsWorkerInit();
	{
		Ptr<CTestThreadNode>	pOwn;

		Ptr<CArrayObject>		pList;
		Ptr<CTestThreadNode>	pListed;

		pOwn = OMalloc<CTestThreadNode>();
		ORoot()->Add(pOwn);
		pOwn->mpNext = psSteps->apcNodes[0];
		pList = OMalloc<CArrayObject>();
		ORoot()->Add(pList);
		pListed = psSteps->apcNodes[1];
		pList->Add(pListed);
		pListed = NULL;
		AtomicStore(&psSteps->iDone, 1);

		TestObjectsStepWait(psSteps, 2);
		psSteps->bLoaded = pOwn->mpNext->IsLive() && pList->Get<CTestThreadNode>(0)->IsLive();
		ORoot()->Remove(pOwn);
		pOwn = NULL;
		pList->Remove((size)0);
		AtomicStore(&psSteps->iDone, 2);

		TestObjectsStepWait(psSteps, 3);
	}
	psSteps->bUnregistered = ObjectsThreadKill();
	AtomicStore(&psSteps->iDone, 3);
}


//////////////////////////////////////////////////////////////////////////
//
// A main object that only a worker's object points at stays while it
// does, though main lets go of it, and is freed by main, on the home
// thread, once the worker's object has gone; and so is one only a
// worker's collection holds, once it is taken out.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsWorkerObjectHoldsMain(void)
{
	STestObjectsSteps		sSteps;
	Ptr<CTestThreadNode>	pShared;
	Ptr<CTestThreadNode>	pListed;
	size					uiObjects;

	TestObjectsThreadsBegin();
	ORoot();
	uiObjects = gcObjects.NumMemoryIndexes();
	pShared = OMalloc<CTestThreadNode>();
	ORoot()->Add(pShared);
	pListed = OMalloc<CTestThreadNode>();
	ORoot()->Add(pListed);
	sSteps.apcNodes[0] = &pShared;
	sSteps.apcNodes[1] = &pListed;
	sSteps.iAsked = 0;
	sSteps.iDone = 0;
	sSteps.bLoaded = false;
	sSteps.cThread.Init();
	sSteps.cThread.Start(TestObjectsThreadsWorkerHolder, &sSteps);
	TestObjectsStep(&sSteps, 1);
	AssertTrue(sSteps.bRegistered);

	//Off main's root and let go of here: the worker's object and collection hold them.
	ORoot()->Remove(pShared);
	ORoot()->Remove(pListed);
	pShared = NULL;
	pListed = NULL;
	ObjectsCollect();
	AssertInt(0, giTestThreadNodesFreed);
	AssertTrue(sSteps.apcNodes[0]->IsLive());
	AssertTrue(sSteps.apcNodes[1]->IsLive());

	//The worker's object goes, and with it the main object, freed here; the other is taken out of the collection.
	TestObjectsStep(&sSteps, 2);
	AssertTrue(sSteps.bLoaded);
	AssertInt(1, giTestThreadNodesFreed);
	ObjectsCollect();
	AssertInt(3, giTestThreadNodesFreed);

	TestObjectsStep(&sSteps, 3);
	sSteps.cThread.Join();
	sSteps.cThread.Kill();
	AssertTrue(sSteps.bUnregistered);
	TestObjectsThreadsEnd(uiObjects);
}


static void TestObjectsThreadsWorkerLends(void* pvSteps)
{
	STestObjectsSteps*		psSteps;

	psSteps = (STestObjectsSteps*)pvSteps;
	psSteps->bRegistered = ObjectsWorkerInit();
	{
		Ptr<CTestThreadNode>	pOwn;
		Ptr<CTestThreadNode>	pMain;

		pOwn = OMalloc<CTestThreadNode>();
		ORoot()->Add(pOwn);
		psSteps->apcNodes[1] = &pOwn;
		AtomicStore(&psSteps->iDone, 1);

		//A main object pointed at the worker's own object.
		TestObjectsStepWait(psSteps, 2);
		pMain = psSteps->apcNodes[0];
		pMain->mpNext = pOwn;
		pMain->mpNext = NULL;
		AtomicStore(&psSteps->iDone, 2);

		TestObjectsStepWait(psSteps, 3);
	}
	psSteps->bUnregistered = ObjectsThreadKill();
	AtomicStore(&psSteps->iDone, 3);
}


//////////////////////////////////////////////////////////////////////////
//
// A worker's objects are its own until it adopts them: another thread
// holding one, or a main object pointing at one, is said (a Debug build
// checks).
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsWorkerObjectsAreItsOwn(void)
{
	STestObjectsSteps		sSteps;
	Ptr<CTestThreadNode>	pMain;
	CMemoryFile				cLog;
	SLogConfig				sLogConfig;
	char					szLog[2048];
	size					uiObjects;

	TestObjectsThreadsBegin();
	pMain = OMalloc<CTestThreadNode>();
	ORoot()->Add(pMain);
	uiObjects = gcObjects.NumMemoryIndexes();
	sSteps.apcNodes[0] = &pMain;
	sSteps.iAsked = 0;
	sSteps.iDone = 0;
	sSteps.cThread.Init();
	sSteps.cThread.Start(TestObjectsThreadsWorkerLends, &sSteps);
	TestObjectsStep(&sSteps, 1);
	AssertTrue(sSteps.bRegistered);

	//This thread holds the worker's object, while the worker waits.
	StartCapturingLog(&cLog, &sLogConfig);
	{
		Ptr<CTestThreadNode>	pBorrowed;

		pBorrowed = sSteps.apcNodes[1];
		pBorrowed = NULL;
	}
	StopCapturingLog(&cLog, &sLogConfig, szLog, sizeof(szLog));
#ifdef _DEBUG
	AssertNotNull(strstr(szLog, "A thread that isn't its worker holds"));
#endif // _DEBUG

	StartCapturingLog(&cLog, &sLogConfig);
	TestObjectsStep(&sSteps, 2);
	StopCapturingLog(&cLog, &sLogConfig, szLog, sizeof(szLog));
#ifdef _DEBUG
	AssertNotNull(strstr(szLog, "of another object system points at"));
#endif // _DEBUG

	TestObjectsStep(&sSteps, 3);
	sSteps.cThread.Join();
	sSteps.cThread.Kill();
	AssertTrue(sSteps.bUnregistered);
	pMain = NULL;
	TestObjectsThreadsEnd(uiObjects);
}


//The leaves of the tree below pNode point at pcShared (by mpOther).
static void TestObjectsThreadsPointLeaves(Ptr<CTestThreadNode>& pNode, CTestThreadNode* pcShared)
{
	Ptr<CTestThreadNode>	pChild;

	if (pNode->mpNext.IsNull())
	{
		pNode->mpOther = pcShared;
		return;
	}
	pChild = pNode->mpNext;
	TestObjectsThreadsPointLeaves(pChild, pcShared);
	pChild = pNode->mpOther;
	TestObjectsThreadsPointLeaves(pChild, pcShared);
}


//A worker builds a named tree in its own root, its leaves pointing at the main node, adopts it and puts it in the
//main root.  The pointer it kept into the tree is to a main object then, and its object system and unknowns have the
//tree no more.
static void TestObjectsThreadsWorkerAdopts(void* pvThread)
{
	STestObjectsThread*		psThread;
	Ptr<CTestThreadNode>	pTop;
	Ptr<CTestThreadNode>	pMiddle;
	char					szName[32];
	size					uiObjects;
	size					uiUnknowns;
	CUnknowns*				pcUnknowns;

	psThread = (STestObjectsThread*)pvThread;
	snprintf(szName, sizeof(szName), "Tree%u", (unsigned)psThread->uiSeed);
	pTop = ONMalloc<CTestThreadNode>(szName);
	ORoot()->Add(pTop);
	TestObjectsThreadsMakeTree(pTop, TEST_OBJECTS_THREADS_TREE_DEPTH);
	TestObjectsThreadsPointLeaves(pTop, psThread->pcShared);
	pMiddle = pTop->mpNext->mpOther;
	uiObjects = ObjectsCurrent()->NumMemoryIndexes();
	pcUnknowns = ObjectsCurrent()->GetUnknowns();
	uiUnknowns = pcUnknowns->NumElements();

	if (!ObjectsAdopt(pTop) || (pTop->GetObjectsThisIn() != &gcObjects) || (pMiddle->GetObjectsThisIn() != &gcObjects))
	{
		psThread->iErrors++;
	}
	if (ObjectsCurrent()->NumMemoryIndexes() != uiObjects - TEST_OBJECTS_THREADS_TREE_NODES)
	{
		psThread->iErrors++;
	}
	if ((pcUnknowns == &gcUnknowns) || (pcUnknowns->NumElements() != uiUnknowns - TEST_OBJECTS_THREADS_TREE_NODES) || (pMiddle->GetUnknownsThisIn() != &gcUnknowns))
	{
		psThread->iErrors++;
	}
	gcObjects.Root()->Add(pTop);
	ORoot()->Remove(pTop);
	if (!pMiddle->IsLive() || !pMiddle->mpNext->IsLive())
	{
		psThread->iErrors++;
	}
}


//The nodes of the tree named szName, reached through the main root.
static size TestObjectsThreadsCountTree(const char* szName)
{
	Ptr<CTestThreadNode>	pTop;
	int						iErrors;

	iErrors = 0;
	pTop = ORoot()->Get(szName);
	return TestObjectsThreadsWalkTree(pTop, &iErrors) + iErrors;
}


//////////////////////////////////////////////////////////////////////////
//
// Workers build trees at once, each in its own object system, their leaves
// pointing at a main node, and adopt them into the main object system: the
// main root holds every tree, each found by its name, with every node, and
// the main node held by every leaf.  Nothing is freed until main lets go,
// and then on the home thread.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsWorkersAdopt(void)
{
	Ptr<CTestThreadNode>	pShared;
	CChars					szName;
	size					uiObjects;
	size					uiUnknowns;
	size					i;

	TestObjectsThreadsBegin();
	pShared = ONMalloc<CTestThreadNode>("Shared");
	ORoot()->Add(pShared);
	uiObjects = gcObjects.NumMemoryIndexes();
	uiUnknowns = gcUnknowns.NumElements();

	gbTestObjectsThreadsWorkers = true;
	AssertInt(0, TestObjectsRunThreads(TestObjectsThreadsWorkerAdopts, NULL, NULL, &pShared, NULL));
	gbTestObjectsThreadsWorkers = false;

	AssertInt(0, giTestThreadNodesFreed);
	AssertLong((int64)(uiObjects + TEST_OBJECTS_THREADS * TEST_OBJECTS_THREADS_TREE_NODES), gcObjects.NumMemoryIndexes());
	AssertSize(uiUnknowns + TEST_OBJECTS_THREADS * TEST_OBJECTS_THREADS_TREE_NODES, gcUnknowns.NumElements());
	AssertSize((size)((TEST_OBJECTS_THREADS << TEST_OBJECTS_THREADS_TREE_DEPTH) + 1), pShared.NumHeapFroms());  //Every leaf, and the root.
	for (i = 0; i < TEST_OBJECTS_THREADS; i++)
	{
		szName.Init();
		szName.Append("Tree");
		szName.Append((uint32)(1000 + i * 7919));
		AssertSize(TEST_OBJECTS_THREADS_TREE_NODES + (1 << TEST_OBJECTS_THREADS_TREE_DEPTH), TestObjectsThreadsCountTree(szName.Text()));  //Each leaf reaches the shared node too.
		szName.Kill();
	}
	gcObjects.EnableValidation();
	gcObjects.ValidateObjectsConsistency();
	gcObjects.DisableValidation();

	ORoot()->RemoveAll();
	AssertInt(TEST_OBJECTS_THREADS * TEST_OBJECTS_THREADS_TREE_NODES, giTestThreadNodesFreed);
	pShared = NULL;
	AssertInt(TEST_OBJECTS_THREADS * TEST_OBJECTS_THREADS_TREE_NODES + 1, giTestThreadNodesFreed);
	AssertSize(uiUnknowns - 1, gcUnknowns.NumElements());
	TestObjectsThreadsEnd(uiObjects - 1);
}


//////////////////////////////////////////////////////////////////////////
//
// Workers' trees adopted into a main object system with a database are
// saved by its flush like any of its objects, and load again: evicted,
// each tree is found by its name with every node.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsWorkersAdoptStored(void)
{
	Ptr<CTestThreadNode>	pShared;
	CChars					szName;
	size					uiObjects;
	size					i;

	TestObjectsThreadsBeginStored();
	pShared = ONMalloc<CTestThreadNode>("Shared");
	ORoot()->Add(pShared);
	AssertTrue(ObjectsLocking(true));

	gbTestObjectsThreadsWorkers = true;
	AssertInt(0, TestObjectsRunThreads(TestObjectsThreadsWorkerAdopts, NULL, NULL, &pShared, NULL));
	gbTestObjectsThreadsWorkers = false;
	pShared = NULL;
	ObjectsCollect();
	AssertTrue(ObjectsLocking(false));

	uiObjects = gcObjects.NumMemoryIndexes();
	AssertTrue(ObjectsFlush());
	AssertTrue(gcObjects.EvictInMemory());
	for (i = 0; i < TEST_OBJECTS_THREADS; i++)
	{
		szName.Init();
		szName.Append("Tree");
		szName.Append((uint32)(1000 + i * 7919));
		AssertSize(TEST_OBJECTS_THREADS_TREE_NODES + (1 << TEST_OBJECTS_THREADS_TREE_DEPTH), TestObjectsThreadsCountTree(szName.Text()));
		szName.Kill();
	}
	AssertLong((int64)uiObjects, gcObjects.NumMemoryIndexes());
	TestObjectsThreadsEndStored();
}


//A graph with a name the main object system has already is refused whole: it stays the worker's, and is freed with it.
static void TestObjectsThreadsWorkerAdoptsClash(void* pvThread)
{
	STestObjectsThread*		psThread;
	Ptr<CTestThreadNode>	pTop;

	psThread = (STestObjectsThread*)pvThread;
	pTop = OMalloc<CTestThreadNode>();
	pTop->mpNext = ONMalloc<CTestThreadNode>("Shared");
	ORoot()->Add(pTop);
	if (ObjectsAdopt(pTop) || (pTop->GetObjectsThisIn() != ObjectsCurrent()) || (pTop->mpNext->GetObjectsThisIn() != ObjectsCurrent()))
	{
		psThread->iErrors++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A worker's graph with a name the main object system has is refused, and
// said, and nothing of it moves (decision 14).
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsThreadsWorkerAdoptClash(void)
{
	Ptr<CTestThreadNode>	pShared;
	CMemoryFile				cLog;
	SLogConfig				sLogConfig;
	char					szLog[1024];
	size					uiObjects;

	TestObjectsThreadsBegin();
	pShared = ONMalloc<CTestThreadNode>("Shared");
	ORoot()->Add(pShared);
	pShared = NULL;
	uiObjects = gcObjects.NumMemoryIndexes();

	StartCapturingLog(&cLog, &sLogConfig);
	gbTestObjectsThreadsWorkers = true;
	AssertInt(0, TestObjectsRunThreads(TestObjectsThreadsWorkerAdoptsClash, NULL, NULL, NULL, NULL));
	gbTestObjectsThreadsWorkers = false;
	StopCapturingLog(&cLog, &sLogConfig, szLog, sizeof(szLog));
	AssertNotNull(strstr(szLog, "Cannot adopt a graph with an object named [Shared]"));

	AssertInt(TEST_OBJECTS_THREADS * 2, giTestThreadNodesFreed);
	TestObjectsThreadsEnd(uiObjects);
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
	TestObjectsThreadsReadersAndWriter();
	TestObjectsThreadsOwnGraphs();
	TestObjectsThreadsFreedOnHomeThread();
	TestObjectsThreadsFreedWhileHeld();
	TestObjectsThreadsWeak();
	TestObjectsThreadsCollection();
	TestObjectsThreadsUnknowns();
	TestObjectsThreadsStoredObjects();
	TestObjectsThreadsKeptStackFroms();
	TestObjectsThreadsKeptStackFromOfHollow();
	TestObjectsThreadsForwardingHollow();
	TestObjectsThreadsLoadStoredTree();
	TestObjectsThreadsStoredCollections();
	TestObjectsThreadsFlushWhileRead();
	TestObjectsThreadsKillStoredHeld();
	TestObjectsThreadsWorkerOwnGraphs();
	TestObjectsThreadsWorkerLeftObjects();
	TestObjectsThreadsWorkerHoldsMain();
	TestObjectsThreadsWorkerObjectsPointAtMain();
	TestObjectsThreadsWorkerObjectHoldsMain();
	TestObjectsThreadsWorkerObjectsAreItsOwn();
	TestObjectsThreadsWorkersAdopt();
	TestObjectsThreadsWorkersAdoptStored();
	TestObjectsThreadsWorkerAdoptClash();
	TestObjectsThreadsAssertNoFree();
	TestObjectsThreadsEveryWayIn();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}


#endif // !__LIBC65__


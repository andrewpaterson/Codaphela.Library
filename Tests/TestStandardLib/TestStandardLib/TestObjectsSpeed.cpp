//The object system's speed isn't measured on the 65C64 or the W65C816, whose compiler is given every test's source.
#ifndef __LIBC65__
#include <stdlib.h>
#include <string.h>
#include "BaseLib/Atomic.h"
#include "BaseLib/Chars.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/CodabaseFactory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/NativeThread.h"
#include "BaseLib/SequenceFactory.h"
#include "BaseLib/Timer.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/DataIO.h"
#include "StandardLib/ArrayObject.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Object.h"
#include "StandardLib/Objects.h"
#include "StandardLib/SetObject.h"
#include "TestLib/Assert.h"


//How long the object system's everyday operations take: the tables in StandardLib/ThreadSafetyPlan.txt.  The test
//asserts only that each ran and left nothing behind.
//
//To time them: a Release build with OBJECTS_SPEED set to how many times longer to run (100, say).  The nanoseconds
//each operation took are logged.  Take the least of several runs; the machine's other work only ever adds.
//
//To count instructions: a difference of a nanosecond or two between two builds can't be told by the clock (where
//things land in memory moves a 14 ns loop by 2 ns with the same instructions), so small changes to the short
//operations are judged by counting.  Each operation is a function of its own, not inlined, so that callgrind can be
//told to count only inside it:
//
//  valgrind --tool=callgrind --callgrind-out-file=/dev/null --toggle-collect='TestObjectsSpeedStackPointer(*' TestStandardLib
//
//with OBJECTS_SPEED set, and again with it set to twice as much: the difference in the instructions collected,
//divided by the difference in how many times the operation ran (TEST_OBJECTS_SPEED_COUNT each time OBJECTS_SPEED goes
//up by one), is the instructions one operation takes.
//
//The tree loaded from a database and the sets are counted the same way.  Each set function is called once for each
//size of set, so to count one size OBJECTS_SPEED_SET_MEMBERS says which (the only size the sets are then made at).
//Two runs at once mustn't share a directory: the tree is saved in Output.
//
//With threads: OBJECTS_SPEED_THREADS set to a number (with OBJECTS_SPEED) switches the object system's locking on
//and registers that many other threads while the operations are timed.  They do nothing but be registered, unless
//OBJECTS_SPEED_THREADS_WORK says what each does over and over until the operations are done: "copy" (a stack Ptr to
//the one node, copied and dropped), "own" (the same, each thread to a node of its own), "malloc" (OMalloc and
//free) or "workermalloc" (the same, each thread a worker with an object system of its own: ObjectsWorkerInit).  What
//a thread's operation took, on average over the threads, is logged with the rest.  With
//OBJECTS_SPEED_ONLY_COPIES set as well this thread does nothing but copy and drop a stack Ptr meanwhile, so nothing
//writes: what threads that only read cost each other.


#define TEST_OBJECTS_SPEED_COUNT		2000
#define TEST_OBJECTS_SPEED_TREE_DEPTH	5


class CTestSpeedNode : public CObject
{
CONSTRUCTABLE(CTestSpeedNode)
DESTRUCTABLE(CTestSpeedNode)
public:
	CPointer	mpObject1;
	CPointer	mpObject2;
	CPointer	mpObject3;
	int32		miValue;

	void			Init(void);
	void			Class(void) override;
	void			Free(void) override;

	NO_INLINE void	Touch(void);
};


void CTestSpeedNode::Init(void)
{
	PreInit();
	miValue = 0;
	PostInit();
}


void CTestSpeedNode::Class(void)
{
	M_Pointer(mpObject1);
	M_Pointer(mpObject2);
	M_Pointer(mpObject3);
	U_Int32(miValue);
}


void CTestSpeedNode::Free(void)
{
}


void CTestSpeedNode::Touch(void)
{
	miValue++;
}


//////////////////////////////////////////////////////////////////////////
//
// How many times longer than the quick run that is the test: 1 unless OBJECTS_SPEED says.
//
//////////////////////////////////////////////////////////////////////////
static size TestObjectsSpeedScale(void)
{
	char*	szScale;
	int		iScale;

	szScale = getenv("OBJECTS_SPEED");
	iScale = szScale ? atoi(szScale) : 1;
	return (iScale < 1) ? 1 : (size)iScale;
}


//////////////////////////////////////////////////////////////////////////
//
// An operation run on uiCount things took iMicroseconds.
//
//////////////////////////////////////////////////////////////////////////
static void TestObjectsSpeedLogTime(const char* szWhat, int64 iMicroseconds, size uiCount)
{
	float	fNanoseconds;

	fNanoseconds = (float)iMicroseconds * 1000.0f / (float)uiCount;
	if (getenv("OBJECTS_SPEED"))
	{
		//In tenths, as whole numbers: printing a float needs the number control.
		gcLogger.Info2(szWhat, ": ", IntToString((int)fNanoseconds), ".", IntToString((int)(fNanoseconds * 10.0f) % 10), " ns", NULL);
	}
	AssertTrue(fNanoseconds >= 0.0f);
}


//fOperation run on uiCount things took from iStart until now.
static void TestObjectsSpeedLog(const char* szWhat, int64 iStart, size uiCount)
{
	TestObjectsSpeedLogTime(szWhat, GetDebugMicroseconds() - iStart, uiCount);
}


//////////////////////////////////////////////////////////////////////////
//
// The operations.  Each is given objects that are attached to the root, as objects in use are.
//
//////////////////////////////////////////////////////////////////////////
NO_INLINE void TestObjectsSpeedStackPointer(Ptr<CTestSpeedNode>& pNode, size uiCount)
{
	size	i;

	for (i = 0; i < uiCount; i++)
	{
		Ptr<CTestSpeedNode> pCopy = pNode;
	}
}


NO_INLINE void TestObjectsSpeedTouch(Ptr<CTestSpeedNode> pNode)
{
	pNode->Touch();
}


NO_INLINE void TestObjectsSpeedPointerByValue(Ptr<CTestSpeedNode>& pNode, size uiCount)
{
	size	i;

	for (i = 0; i < uiCount; i++)
	{
		TestObjectsSpeedTouch(pNode);
	}
}


NO_INLINE void TestObjectsSpeedDereference(Ptr<CTestSpeedNode>& pNode, size uiCount)
{
	size	i;

	for (i = 0; i < uiCount; i++)
	{
		pNode->Touch();
	}
}


NO_INLINE void TestObjectsSpeedField(Ptr<CTestSpeedNode>& pNode, Ptr<CTestSpeedNode>& pFirst, Ptr<CTestSpeedNode>& pSecond, size uiCount)
{
	size	i;

	for (i = 0; i < uiCount; i += 2)
	{
		pNode->mpObject1 = pFirst;
		pNode->mpObject1 = pSecond;
	}
	pNode->mpObject1 = NULL;
}


NO_INLINE void TestObjectsSpeedMalloc(size uiCount)
{
	size	i;

	for (i = 0; i < uiCount; i++)
	{
		Ptr<CTestSpeedNode> pNew = OMalloc<CTestSpeedNode>();
	}
}


NO_INLINE void TestObjectsSpeedMallocIntoField(Ptr<CTestSpeedNode>& pNode, size uiCount)
{
	size	i;

	for (i = 0; i < uiCount; i++)
	{
		Ptr<CTestSpeedNode> pNew = OMalloc<CTestSpeedNode>();
		pNode->mpObject1 = pNew;
		pNew = NULL;
		pNode->mpObject1 = NULL;
	}
}


NO_INLINE void TestObjectsSpeedMallocIntoArray(Ptr<CArrayObject>& pArray, size uiCount)
{
	size	i;

	for (i = 0; i < uiCount; i++)
	{
		Ptr<CTestSpeedNode> pNew = OMalloc<CTestSpeedNode>();
		pArray->Add(pNew);
		pNew = NULL;
		pArray->Remove((size)0);
	}
}


NO_INLINE void TestObjectsSpeedArrayGet(Ptr<CArrayObject>& pArray, size uiCount)
{
	size	i;

	for (i = 0; i < uiCount; i++)
	{
		pArray->Get<CTestSpeedNode>(0)->Touch();
	}
}


//A window's worth of components: every node points to the window and to its parent, and holds its children in an
//array.  Returns how many nodes it made.
NO_INLINE size TestObjectsSpeedBuildTree(Ptr<CTestSpeedNode> pWindow, Ptr<CTestSpeedNode> pParent, size uiDepth, CArrayBaseObjectPtr* papcNodes)
{
	Ptr<CArrayObject>		pChildren;
	Ptr<CTestSpeedNode>		pChild;
	size					uiNodes;
	size					i;

	papcNodes->Add(&pParent);
	pParent->mpObject2 = pWindow;
	if (uiDepth == 0)
	{
		return 1;
	}

	uiNodes = 1;
	pChildren = OMalloc<CArrayObject>();
	pParent->mpObject3 = pChildren;
	for (i = 0; i < 3; i++)
	{
		pChild = OMalloc<CTestSpeedNode>();
		pChild->mpObject1 = pParent;
		pChildren->Add(pChild);
		uiNodes += TestObjectsSpeedBuildTree(pWindow, pChild, uiDepth - 1, papcNodes);
	}
	return uiNodes;
}


//The window's focus moved to each node in turn.
NO_INLINE void TestObjectsSpeedFocus(Ptr<CTestSpeedNode>& pFocus, CArrayBaseObjectPtr* papcNodes)
{
	size	i;

	for (i = 0; i < papcNodes->NumElements(); i++)
	{
		pFocus->mpObject1 = papcNodes->GetPtr(i);
	}
	pFocus->mpObject1 = NULL;
}


NO_INLINE void TestObjectsSpeedFreeTree(Ptr<CTestSpeedNode>& pWindow)
{
	ORoot()->Remove(pWindow);
	pWindow = NULL;
}


//pNode and what is below it, as TestObjectsSpeedLoad.
static void TestObjectsSpeedLoadNode(Ptr<CTestSpeedNode>& pNode, size* puiObjects)
{
	Ptr<CArrayObject>		pChildren;
	Ptr<CTestSpeedNode>		pChild;
	size					uiNumChildren;
	size					i;

	(*puiObjects)++;
	pChildren = pNode->mpObject3;
	if (pChildren.IsNull())
	{
		return;
	}

	(*puiObjects)++;
	uiNumChildren = pChildren->NumElements();
	for (i = 0; i < uiNumChildren; i++)
	{
		pChild = pChildren->Get<CTestSpeedNode>(i);
		TestObjectsSpeedLoadNode(pChild, puiObjects);
	}
}


//A tree read back from the database by walking it through its pointers: each node's array of children is a hollow
//until it is used, and so is each child.  Returns how many objects it loaded.
NO_INLINE size TestObjectsSpeedLoad(Ptr<CTestSpeedNode>& pTop)
{
	size	uiObjects;

	uiObjects = 0;
	TestObjectsSpeedLoadNode(pTop, &uiObjects);
	return uiObjects;
}


//Each member is added in the order of the ring the members' mpObject1s make, starting after pcFirst.
NO_INLINE void TestObjectsSpeedSetFill(Ptr<CSetObject>& pSet, CTestSpeedNode* pcFirst, size uiMembers)
{
	CTestSpeedNode*		pcNode;
	size				i;

	pcNode = pcFirst;
	for (i = 0; i < uiMembers; i++)
	{
		pSet->Add(pcNode->mpObject1);
		pcNode = (CTestSpeedNode*)pcNode->mpObject1.Object();
	}
}


//Returns how many of the uiCount looked for were found.
NO_INLINE size TestObjectsSpeedSetContains(Ptr<CSetObject>& pSet, CTestSpeedNode* pcFirst, size uiCount)
{
	CTestSpeedNode*		pcNode;
	size				uiFound;
	size				i;

	pcNode = pcFirst;
	uiFound = 0;
	for (i = 0; i < uiCount; i++)
	{
		uiFound += pSet->Contains(pcNode->mpObject1) ? 1 : 0;
		pcNode = (CTestSpeedNode*)pcNode->mpObject1.Object();
	}
	return uiFound;
}


NO_INLINE void TestObjectsSpeedSetRemoveAdd(Ptr<CSetObject>& pSet, CTestSpeedNode* pcFirst, size uiCount)
{
	CTestSpeedNode*		pcNode;
	size				i;

	pcNode = pcFirst;
	for (i = 0; i < uiCount; i++)
	{
		pSet->Remove(pcNode->mpObject1);
		pSet->Add(pcNode->mpObject1);
		pcNode = (CTestSpeedNode*)pcNode->mpObject1.Object();
	}
}


#define TEST_OBJECTS_SPEED_MAX_THREADS	64
#define TEST_OBJECTS_SPEED_THREAD_BATCH	200


struct STestSpeedThread
{
	CNativeThread		cThread;
	CTestSpeedNode*		pcNode;		//What it copies pointers to.
	const char*			szWork;		//NULL for nothing.
	int64				iMicroseconds;
	size				uiOperations;
};


static STestSpeedThread	gasTestSpeedThreads[TEST_OBJECTS_SPEED_MAX_THREADS];
static size				guiTestSpeedThreads = 0;
static int32			giTestSpeedThreadsReady = 0;
static int32			giTestSpeedThreadsStop = 0;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestObjectsSpeedThread(void* pvThread)
{
	STestSpeedThread*	psThread;
	int64				iStart;

	psThread = (STestSpeedThread*)pvThread;
	psThread->uiOperations = 0;
	psThread->iMicroseconds = 0;
	if ((psThread->szWork != NULL) && (strcmp(psThread->szWork, "workermalloc") == 0))
	{
		ObjectsWorkerInit();
	}
	else
	{
		ObjectsThreadInit();
	}
	{
		Ptr<CTestSpeedNode>		pNode;

		//Only a thread that copies pointers holds the node: held, the tree it is the top of wouldn't be freed.
		if ((psThread->szWork != NULL) && (strstr(psThread->szWork, "malloc") == NULL))
		{
			pNode = psThread->pcNode;
		}
		AtomicAdd(&giTestSpeedThreadsReady, 1);
		iStart = GetDebugMicroseconds();
		while (AtomicLoad(&giTestSpeedThreadsStop) == 0)
		{
			if (psThread->szWork == NULL)
			{
				ThreadSleep(1);
			}
			else if (strstr(psThread->szWork, "malloc") != NULL)
			{
				TestObjectsSpeedMalloc(TEST_OBJECTS_SPEED_THREAD_BATCH);
				psThread->uiOperations += TEST_OBJECTS_SPEED_THREAD_BATCH;
			}
			else
			{
				TestObjectsSpeedStackPointer(pNode, TEST_OBJECTS_SPEED_THREAD_BATCH);
				psThread->uiOperations += TEST_OBJECTS_SPEED_THREAD_BATCH;
			}
		}
		psThread->iMicroseconds = GetDebugMicroseconds() - iStart;
	}
	ObjectsThreadKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Registers OBJECTS_SPEED_THREADS threads and sets them to OBJECTS_SPEED_THREADS_WORK, with the locking on.  Each
// is given a node of its own, in the root, for "own"; pcShared otherwise.
//
//////////////////////////////////////////////////////////////////////////
static void TestObjectsSpeedThreadsStart(CTestSpeedNode* pcShared)
{
	Ptr<CTestSpeedNode>		pOwn;
	char*					szThreads;
	const char*				szWork;
	size					i;

	szThreads = getenv("OBJECTS_SPEED_THREADS");
	guiTestSpeedThreads = ((szThreads != NULL) && (getenv("OBJECTS_SPEED") != NULL)) ? (size)atoi(szThreads) : 0;
	if (guiTestSpeedThreads > TEST_OBJECTS_SPEED_MAX_THREADS)
	{
		guiTestSpeedThreads = TEST_OBJECTS_SPEED_MAX_THREADS;
	}
	if (guiTestSpeedThreads == 0)
	{
		return;
	}

	szWork = getenv("OBJECTS_SPEED_THREADS_WORK");
	AtomicStore(&giTestSpeedThreadsReady, 0);
	AtomicStore(&giTestSpeedThreadsStop, 0);
	for (i = 0; i < guiTestSpeedThreads; i++)
	{
		gasTestSpeedThreads[i].pcNode = pcShared;
		if ((szWork != NULL) && (strcmp(szWork, "own") == 0))
		{
			pOwn = OMalloc<CTestSpeedNode>();
			ORoot()->Add(pOwn);
			gasTestSpeedThreads[i].pcNode = &pOwn;
		}
		gasTestSpeedThreads[i].szWork = szWork;
	}
	pOwn = NULL;

	AssertTrue(ObjectsLocking(true));
	for (i = 0; i < guiTestSpeedThreads; i++)
	{
		gasTestSpeedThreads[i].cThread.Init();
		gasTestSpeedThreads[i].cThread.Start(TestObjectsSpeedThread, &gasTestSpeedThreads[i]);
	}
	while (AtomicLoad(&giTestSpeedThreadsReady) < (int32)guiTestSpeedThreads)
	{
		ThreadYield();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestObjectsSpeedThreadsStop(void)
{
	float	fNanoseconds;
	size	uiWorked;
	size	i;

	if (guiTestSpeedThreads == 0)
	{
		return;
	}

	AtomicStore(&giTestSpeedThreadsStop, 1);
	fNanoseconds = 0.0f;
	uiWorked = 0;
	for (i = 0; i < guiTestSpeedThreads; i++)
	{
		gasTestSpeedThreads[i].cThread.Join();
		gasTestSpeedThreads[i].cThread.Kill();
		if (gasTestSpeedThreads[i].uiOperations > 0)
		{
			fNanoseconds += (float)gasTestSpeedThreads[i].iMicroseconds * 1000.0f / (float)gasTestSpeedThreads[i].uiOperations;
			uiWorked++;
		}
	}
	if (uiWorked > 0)
	{
		fNanoseconds /= (float)uiWorked;
		gcLogger.Info2("Each of ", SizeToString(uiWorked), " other threads, an operation (", gasTestSpeedThreads[0].szWork, "): ", IntToString((int)fNanoseconds), ".", IntToString((int)(fNanoseconds * 10.0f) % 10), " ns", NULL);
	}
	else
	{
		gcLogger.Info2(SizeToString(guiTestSpeedThreads), " other threads were registered.", NULL);
	}
	ObjectsCollect();
	AssertTrue(ObjectsLocking(false));
	guiTestSpeedThreads = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsSpeedOperations(void)
{
	Ptr<CTestSpeedNode>		pNode;
	Ptr<CTestSpeedNode>		pFirst;
	Ptr<CTestSpeedNode>		pSecond;
	Ptr<CArrayObject>		pArray;
	size					uiCount;
	int64					iStart;

	ObjectsInit();
	gcObjects.DisableValidation();

	uiCount = TEST_OBJECTS_SPEED_COUNT * TestObjectsSpeedScale();
	pNode = OMalloc<CTestSpeedNode>();
	pFirst = OMalloc<CTestSpeedNode>();
	pSecond = OMalloc<CTestSpeedNode>();
	pArray = OMalloc<CArrayObject>();
	ORoot()->Add(pNode);
	ORoot()->Add(pFirst);
	ORoot()->Add(pSecond);
	ORoot()->Add(pArray);
	TestObjectsSpeedThreadsStart(&pNode);

	if (getenv("OBJECTS_SPEED_ONLY_COPIES") != NULL)
	{
		iStart = GetDebugMicroseconds();
		TestObjectsSpeedStackPointer(pNode, uiCount * 50);
		TestObjectsSpeedLog("Stack Ptr copied and dropped, and nothing else", iStart, uiCount * 50);
		TestObjectsSpeedThreadsStop();
		pNode = NULL;
		pFirst = NULL;
		pSecond = NULL;
		pArray = NULL;
		ORoot()->RemoveAll();
		ObjectsFlush();
		ObjectsKill();
		return;
	}

	iStart = GetDebugMicroseconds();
	TestObjectsSpeedStackPointer(pNode, uiCount);
	TestObjectsSpeedLog("Stack Ptr copied and dropped", iStart, uiCount);

	iStart = GetDebugMicroseconds();
	TestObjectsSpeedPointerByValue(pNode, uiCount);
	TestObjectsSpeedLog("Ptr passed by value and a method called", iStart, uiCount);

	iStart = GetDebugMicroseconds();
	TestObjectsSpeedDereference(pNode, uiCount * 4);
	TestObjectsSpeedLog("Method called through a Ptr", iStart, uiCount * 4);
	AssertInt((int)(uiCount * 5), pNode->miValue);

	iStart = GetDebugMicroseconds();
	TestObjectsSpeedField(pNode, pFirst, pSecond, uiCount);
	TestObjectsSpeedLog("Field pointed at another object", iStart, uiCount);

	iStart = GetDebugMicroseconds();
	TestObjectsSpeedMalloc(uiCount);
	TestObjectsSpeedLog("OMalloc and free", iStart, uiCount);

	iStart = GetDebugMicroseconds();
	TestObjectsSpeedMallocIntoField(pNode, uiCount);
	TestObjectsSpeedLog("OMalloc, held by a field, free", iStart, uiCount);

	iStart = GetDebugMicroseconds();
	TestObjectsSpeedMallocIntoArray(pArray, uiCount);
	TestObjectsSpeedLog("OMalloc, added to an array, removed", iStart, uiCount);

	pArray->Add(pFirst);
	iStart = GetDebugMicroseconds();
	TestObjectsSpeedArrayGet(pArray, uiCount);
	TestObjectsSpeedLog("Array's Get and a method called", iStart, uiCount);
	pArray->RemoveAll();
	TestObjectsSpeedThreadsStop();

	gcObjects.EnableValidation();
	gcObjects.ValidateObjectsConsistency();
	if (getenv("OBJECTS_SPEED_THREADS") == NULL)
	{
		AssertLong(6, gcObjects.NumMemoryIndexes());
	}
	pNode = NULL;
	pFirst = NULL;
	pSecond = NULL;
	pArray = NULL;
	ORoot()->RemoveAll();
	AssertLong(2, gcObjects.NumMemoryIndexes());

	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsSpeedTree(void)
{
	Ptr<CTestSpeedNode>		pWindow;
	Ptr<CTestSpeedNode>		pTop;
	Ptr<CTestSpeedNode>		pFocus;
	CArrayBaseObjectPtr		apcNodes;
	size					uiDepth;
	size					uiNodes;
	int64					iStart;

	ObjectsInit();
	gcObjects.DisableValidation();
	apcNodes.Init();

	//Two levels deeper (nine times the nodes) when it is being timed.
	uiDepth = TEST_OBJECTS_SPEED_TREE_DEPTH + (getenv("OBJECTS_SPEED") ? 2 : 0);
	pWindow = OMalloc<CTestSpeedNode>();
	ORoot()->Add(pWindow);
	pTop = OMalloc<CTestSpeedNode>();
	TestObjectsSpeedThreadsStart(&pWindow);

	iStart = GetDebugMicroseconds();
	pTop->mpObject1 = pWindow;
	pWindow->mpObject1 = pTop;
	uiNodes = TestObjectsSpeedBuildTree(pWindow, pTop, uiDepth, &apcNodes);
	TestObjectsSpeedLog("Tree of objects built, a node", iStart, uiNodes);
	AssertSize(uiNodes, apcNodes.NumElements());

	pFocus = OMalloc<CTestSpeedNode>();
	pWindow->mpObject3 = pFocus;
	iStart = GetDebugMicroseconds();
	TestObjectsSpeedFocus(pFocus, &apcNodes);
	TestObjectsSpeedLog("Focus moved across the tree, a move", iStart, uiNodes);

	gcObjects.EnableValidation();
	gcObjects.ValidateObjectsConsistency();
	gcObjects.DisableValidation();

	pTop = NULL;
	pFocus = NULL;
	iStart = GetDebugMicroseconds();
	TestObjectsSpeedFreeTree(pWindow);
	TestObjectsSpeedLog("Tree of objects freed, a node", iStart, uiNodes);
	TestObjectsSpeedThreadsStop();
	ORoot()->RemoveAll();
	AssertLong(2, gcObjects.NumMemoryIndexes());

	apcNodes.Kill();
	gcObjects.EnableValidation();
	ObjectsFlush();
	ObjectsKill();
}


//What a worker's build and Adopt took.
struct STestSpeedAdopt
{
	size	uiNodes;
	size	uiObjects;
	int64	iBuild;
	int64	iAdopt;
	bool	bAdopted;
};


//A worker builds TestObjectsSpeedTree's tree in its own object system and gives it to the main one.
static void TestObjectsSpeedAdoptThread(void* pvAdopt)
{
	STestSpeedAdopt*		psAdopt;
	CArrayBaseObjectPtr		apcNodes;
	int64					iStart;

	psAdopt = (STestSpeedAdopt*)pvAdopt;
	ObjectsWorkerInit();
	{
		Ptr<CTestSpeedNode>		pWindow;
		Ptr<CTestSpeedNode>		pTop;

		apcNodes.Init();
		pWindow = OMalloc<CTestSpeedNode>();
		pTop = OMalloc<CTestSpeedNode>();
		iStart = GetDebugMicroseconds();
		pTop->mpObject1 = pWindow;
		pWindow->mpObject1 = pTop;
		psAdopt->uiNodes = TestObjectsSpeedBuildTree(pWindow, pTop, TEST_OBJECTS_SPEED_TREE_DEPTH + (getenv("OBJECTS_SPEED") ? 2 : 0), &apcNodes);
		psAdopt->iBuild = GetDebugMicroseconds() - iStart;
		psAdopt->uiObjects = ObjectsCurrent()->NumMemoryIndexes();

		iStart = GetDebugMicroseconds();
		psAdopt->bAdopted = ObjectsAdopt(pWindow);
		psAdopt->iAdopt = GetDebugMicroseconds() - iStart;
		gcObjects.Root()->Add(pWindow);
		apcNodes.Kill();
	}
	ObjectsThreadKill();
}


//////////////////////////////////////////////////////////////////////////
//
// TestObjectsSpeedTree's tree built on a worker, in an object system of
// its own (no lock), and adopted by the main object system: a node, and
// Adopt's cost for each object.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsSpeedAdopt(void)
{
	STestSpeedAdopt		sAdopt;
	CNativeThread		cThread;

	ObjectsInit();
	gcObjects.DisableValidation();
	ORoot();
	AssertTrue(ObjectsLocking(true));
	sAdopt.bAdopted = false;
	cThread.Init();
	cThread.Start(TestObjectsSpeedAdoptThread, &sAdopt);
	cThread.Join();
	cThread.Kill();
	AssertTrue(ObjectsLocking(false));

	AssertTrue(sAdopt.bAdopted);
	TestObjectsSpeedLogTime("Tree of objects built on a worker, a node", sAdopt.iBuild, sAdopt.uiNodes);
	TestObjectsSpeedLogTime("Tree adopted, an object", sAdopt.iAdopt, sAdopt.uiObjects);
	AssertLong((int64)(2 + sAdopt.uiObjects), gcObjects.NumMemoryIndexes());

	gcObjects.EnableValidation();
	gcObjects.ValidateObjectsConsistency();
	ORoot()->RemoveAll();
	AssertLong(2, gcObjects.NumMemoryIndexes());
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// The tree TestObjectsSpeedTree builds, saved, and read back with the
// objects and the database opened again from nothing.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsSpeedLoadTree(void)
{
	CFileUtil				cFileUtil;
	CCodabase*				pcDatabase;
	CSequence*				pcSequence;
	Ptr<CTestSpeedNode>		pWindow;
	Ptr<CTestSpeedNode>		pTop;
	CArrayBaseObjectPtr		apcNodes;
	size					uiDepth;
	size					uiNodes;
	size					uiObjects;
	int64					iStart;
	char					szDirectory[] = "Output" _FS_ "ObjectsSpeed" _FS_ "Load";

	cFileUtil.RemoveDir(szDirectory);
	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	gcObjects.DisableValidation();
	apcNodes.Init();

	uiDepth = TEST_OBJECTS_SPEED_TREE_DEPTH + (getenv("OBJECTS_SPEED") ? 2 : 0);
	pWindow = ONMalloc<CTestSpeedNode>("Window");
	ORoot()->Add(pWindow);
	pTop = OMalloc<CTestSpeedNode>();
	pTop->mpObject1 = pWindow;
	pWindow->mpObject1 = pTop;
	uiNodes = TestObjectsSpeedBuildTree(pWindow, pTop, uiDepth, &apcNodes);
	apcNodes.Kill();
	pTop = NULL;
	pWindow = NULL;

	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);

	pcSequence = CSequenceFactory::Create(szDirectory, DS_No);
	pcDatabase = CCodabaseFactory::Create(szDirectory, IWT_No, DS_No);
	pcDatabase->Open();
	ObjectsInit(pcDatabase, pcSequence);
	gcObjects.DisableValidation();
	gcObjects.AddConstructor<CTestSpeedNode>();

	pWindow = ORoot()->Get<CTestSpeedNode>("Window");
	pTop = pWindow->mpObject1;
	iStart = GetDebugMicroseconds();
	uiObjects = TestObjectsSpeedLoad(pTop);
	TestObjectsSpeedLog("Tree loaded through its pointers, an object", iStart, uiObjects);

	//Every node, and an array for each node that has children: a third of the rest.
	AssertSize(uiNodes + (uiNodes - 1) / 3, uiObjects);
	gcObjects.EnableValidation();
	gcObjects.ValidateObjectsConsistency();
	AssertLong(3 + uiObjects, gcObjects.NumMemoryIndexes());

	pTop = NULL;
	pWindow = NULL;
	ObjectsFlush();
	pcDatabase->Close();
	SafeKill(pcDatabase);
	SafeKill(pcSequence);
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// A set of uiMembers nodes: filled, looked in uiCount times, and a member
// removed and added back uiCount times (fewer for a big set, where each
// moves the members after it).  Every node is held in an array as well,
// so nothing the set does frees one.  They are added, looked for and
// removed in a shuffled order (the same every run): each node's mpObject1
// points to the next, round in a ring.
//
//////////////////////////////////////////////////////////////////////////
static void TestObjectsSpeedSet(size uiMembers, size uiCount)
{
	Ptr<CArrayObject>		pHolder;
	Ptr<CSetObject>			pSet;
	Ptr<CTestSpeedNode>		pNode;
	CArrayBaseObjectPtr		apcNodes;
	CTestSpeedNode*			pcFirst;
	CBaseObject*			pcSwap;
	uint32					uiRandom;
	size					uiPairs;
	size					i;
	size					j;
	int64					iStart;
	CChars					szFill;
	CChars					szContains;
	CChars					szPair;

	szFill.Init("Set of ")->Append(SizeToString(uiMembers))->Append(" members filled, an Add");
	szContains.Init("Set of ")->Append(SizeToString(uiMembers))->Append(" members, Contains");
	szPair.Init("Set of ")->Append(SizeToString(uiMembers))->Append(" members, a member removed and added back");

	pHolder = OMalloc<CArrayObject>();
	ORoot()->Add(pHolder);
	apcNodes.Init();
	for (i = 0; i < uiMembers; i++)
	{
		pNode = OMalloc<CTestSpeedNode>();
		pHolder->Add(pNode);
		apcNodes.Add(&pNode);
	}
	pNode = NULL;

	uiRandom = 1;
	for (i = uiMembers - 1; i > 0; i--)
	{
		uiRandom = uiRandom * 1103515245 + 12345;
		j = (size)((uiRandom >> 8) % (uint32)(i + 1));
		pcSwap = apcNodes.GetPtr(i);
		apcNodes.SetPtr(i, apcNodes.GetPtr(j));
		apcNodes.SetPtr(j, pcSwap);
	}
	for (i = 0; i < uiMembers; i++)
	{
		((CTestSpeedNode*)apcNodes.GetPtr(i))->mpObject1 = apcNodes.GetPtr((i + 1) % uiMembers);
	}
	pcFirst = (CTestSpeedNode*)apcNodes.GetPtr(0);
	apcNodes.Kill();

	pSet = OMalloc<CSetObject>();
	ORoot()->Add(pSet);

	iStart = GetDebugMicroseconds();
	TestObjectsSpeedSetFill(pSet, pcFirst, uiMembers);
	TestObjectsSpeedLog(szFill.Text(), iStart, uiMembers);
	AssertSize(uiMembers, pSet->NumElements());

	iStart = GetDebugMicroseconds();
	AssertSize(uiCount, TestObjectsSpeedSetContains(pSet, pcFirst, uiCount));
	TestObjectsSpeedLog(szContains.Text(), iStart, uiCount);

	uiPairs = (uiMembers > 1000) ? uiCount / 100 : uiCount;
	iStart = GetDebugMicroseconds();
	TestObjectsSpeedSetRemoveAdd(pSet, pcFirst, uiPairs);
	TestObjectsSpeedLog(szPair.Text(), iStart, uiPairs);
	AssertSize(uiMembers, pSet->NumElements());

	ORoot()->Remove(pSet);
	pSet = NULL;
	ORoot()->Remove(pHolder);
	pHolder = NULL;
	AssertLong(2, gcObjects.NumMemoryIndexes());

	szPair.Kill();
	szContains.Kill();
	szFill.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Sets of 10 and 1000 members, and 100000 when timed; or only as many as
// OBJECTS_SPEED_SET_MEMBERS says.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsSpeedSets(void)
{
	size	uiCount;
	char*	szMembers;

	ObjectsInit();
	gcObjects.DisableValidation();

	uiCount = TEST_OBJECTS_SPEED_COUNT * TestObjectsSpeedScale();
	szMembers = getenv("OBJECTS_SPEED_SET_MEMBERS");
	if ((szMembers != NULL) && (atoi(szMembers) > 1))
	{
		TestObjectsSpeedSet((size)atoi(szMembers), uiCount);
	}
	else
	{
		TestObjectsSpeedSet(10, uiCount);
		TestObjectsSpeedSet(1000, uiCount);
		if (getenv("OBJECTS_SPEED"))
		{
			TestObjectsSpeedSet(100000, uiCount);
		}
	}

	gcObjects.EnableValidation();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectsSpeed(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestObjectsSpeedOperations();
	TestObjectsSpeedTree();
	TestObjectsSpeedAdopt();
	TestObjectsSpeedLoadTree();
	TestObjectsSpeedSets();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}


#endif // !__LIBC65__


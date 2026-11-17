//The object system's speed isn't measured on the 65C64 or the W65C816, whose compiler is given every test's source.
#ifndef __LIBC65__
#include <stdlib.h>
#include <string.h>
#include "BaseLib/Atomic.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/NativeThread.h"
#include "BaseLib/Timer.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/DataIO.h"
#include "StandardLib/ArrayObject.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Object.h"
#include "StandardLib/Objects.h"
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
//With threads: OBJECTS_SPEED_THREADS set to a number (with OBJECTS_SPEED) switches the object system's locking on
//and registers that many other threads while the operations are timed.  They do nothing but be registered, unless
//OBJECTS_SPEED_THREADS_WORK says what each does over and over until the operations are done: "copy" (a stack Ptr to
//the one node, copied and dropped), "own" (the same, each thread to a node of its own) or "malloc" (OMalloc and
//free).  What a thread's operation took, on average over the threads, is logged with the rest.


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
// fOperation run on uiCount things took from iStart until now.
//
//////////////////////////////////////////////////////////////////////////
static void TestObjectsSpeedLog(const char* szWhat, int64 iStart, size uiCount)
{
	float	fNanoseconds;

	fNanoseconds = (float)(GetDebugMicroseconds() - iStart) * 1000.0f / (float)uiCount;
	if (getenv("OBJECTS_SPEED"))
	{
		//In tenths, as whole numbers: printing a float needs the number control.
		gcLogger.Info2(szWhat, ": ", IntToString((int)fNanoseconds), ".", IntToString((int)(fNanoseconds * 10.0f) % 10), " ns", NULL);
	}
	AssertTrue(fNanoseconds >= 0.0f);
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
	ObjectsThreadInit();
	{
		Ptr<CTestSpeedNode>		pNode;

		//Only a thread that copies pointers holds the node: held, the tree it is the top of wouldn't be freed.
		if ((psThread->szWork != NULL) && (strcmp(psThread->szWork, "malloc") != 0))
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
			else if (strcmp(psThread->szWork, "malloc") == 0)
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

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}


#endif // !__LIBC65__


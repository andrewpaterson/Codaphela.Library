#include "BaseLib/GlobalMemory.h"
#include "BaseLib/Logger.h"
#include "BaseLib/MemoryFile.h"
#include "StandardLib/Unknowns.h"
#include "TestLib/Assert.h"
#include "UnknownTestClasses.h"


void StartCapturingLog(CMemoryFile* pcMemory, SLogConfig* psLogConfig);  //TestObjectFreeOrdering.cpp
void StopCapturingLog(CMemoryFile* pcMemory, SLogConfig* psLogConfig, char* szLogged, size uiSize);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUnknownListAddKill(void)
{
	MemoryInit();

	CUnknowns				cUnknowns;
	CTestUnknown*			pcTest;
	CFreeList*				pcFreeList;
	CGeneralMemory*			pcMemory;
	CLifeInit<CMallocator>	cLifeMalloc = LifeAlloc<CMemoryAllocator, CMallocator>();
	CMemoryAllocator*		pcMalloc;

	pcMalloc = (CMemoryAllocator*)cLifeMalloc.GetLife();
	cUnknowns.Init(cLifeMalloc, "Test1", &gcConstructors);
	pcMemory = pcMalloc->GetMemory();
	AssertNotNull(pcMemory);
	AssertSize(0, pcMemory->NumElements());
	AssertTrue(pcMemory->ByteSize() <= 64);

	pcTest = cUnknowns.Add<CTestUnknown>();
	AssertPointer(&cUnknowns, pcTest->GetUnknownsThisIn());
	pcFreeList = pcMemory->GetFreeList(sizeof(CTestUnknown));
	AssertSize(1, pcFreeList->NumElements());
	AssertSize(1, pcMemory->NumElements());
#ifdef BUILD_64
	AssertLSize(65640, pcMemory->ByteSize());
#elif defined(SIZE_16)
	AssertLSize(65588, pcMemory->ByteSize());
#else
	AssertLSize(65592, pcMemory->ByteSize());
#endif // BUILD_64

	pcTest->Init();
	pcTest->Kill();
	pcFreeList = pcMemory->GetFreeList(sizeof(CTestUnknown));
	AssertNotNull(pcFreeList);
	AssertSize(0, pcFreeList->NumElements());
	pcMemory->Compact();
	pcFreeList = pcMemory->GetFreeList(sizeof(CTestUnknown));
	AssertNull(pcFreeList);
	AssertSize(0, pcMemory->NumElements());
	AssertTrue(pcMemory->ByteSize() <= 64);

	cUnknowns.Kill();

	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUnknownListIterables(void)
{
	MemoryInit();

	CUnknowns									cUnknowns;
	CTestIterableUnknown*						pcTest7;
	CTestIterableUnknown*						pcTest5;
	CTestIterableUnknown*						pcTest3;
	CTestIterableUnknown*						pcTest4;
	CTestIterableUnknown*						pcTest1;
	CTestIterableUnknown*						pcTest;
	SIteratorTemplate<CTestIterableUnknown>		sIter; 

	cUnknowns.Init("Test2", &gcConstructors);

	pcTest7 = cUnknowns.Add<CTestIterableUnknown>();
	pcTest7->Init(7);
	pcTest5 = cUnknowns.Add<CTestIterableUnknown>();
	pcTest5->Init(5);
	pcTest3 = cUnknowns.Add<CTestIterableUnknown>();
	pcTest3->Init(3);
	pcTest4 = cUnknowns.Add<CTestIterableUnknown>();
	pcTest4->Init(4);
	pcTest1 = cUnknowns.Add<CTestIterableUnknown>();
	pcTest1->Init(1);

	pcTest = cUnknowns.StartIteration(&sIter);
	AssertInt(7, pcTest->miID);
	pcTest = cUnknowns.Iterate(&sIter);
	AssertInt(5, pcTest->miID);
	pcTest = cUnknowns.Iterate(&sIter);
	AssertInt(3, pcTest->miID);
	pcTest = cUnknowns.Iterate(&sIter);
	AssertInt(4, pcTest->miID);
	pcTest = cUnknowns.Iterate(&sIter);
	AssertInt(1, pcTest->miID);
	pcTest = cUnknowns.Iterate(&sIter);
	AssertNull(pcTest);

	pcTest7->Kill();
	pcTest1->Kill();

	pcTest = cUnknowns.StartIteration(&sIter);
	AssertInt(5, pcTest->miID);
	pcTest = cUnknowns.Iterate(&sIter);
	AssertInt(3, pcTest->miID);
	pcTest = cUnknowns.Iterate(&sIter);
	AssertInt(4, pcTest->miID);
	pcTest = cUnknowns.Iterate(&sIter);
	AssertNull(pcTest);

	pcTest3->Kill();

	pcTest = cUnknowns.StartIteration(&sIter);
	AssertInt(5, pcTest->miID);
	pcTest = cUnknowns.Iterate(&sIter);
	AssertInt(4, pcTest->miID);
	pcTest = cUnknowns.Iterate(&sIter);
	AssertNull(pcTest);

	pcTest5->Kill();
	pcTest4->Kill();

	pcTest = cUnknowns.StartIteration(&sIter);
	AssertNull(pcTest);

	cUnknowns.Kill();

	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUnknownListRemoveDuringIteration(void)
{
	MemoryInit();

	CUnknowns									cUnknowns;
	CTestIterableUnknown*						pcTest7;
	CTestIterableUnknown*						pcTest5;
	CTestIterableUnknown*						pcTest3;
	CTestIterableUnknown*						pcTest4;
	CTestIterableUnknown*						pcTest1;
	CTestIterableUnknown*						pcTest;
	SIteratorTemplate<CTestIterableUnknown>		sIter; 

	cUnknowns.Init("Test3", &gcConstructors);

	pcTest7 = cUnknowns.Add<CTestIterableUnknown>();
	pcTest7->Init(7);
	pcTest5 = cUnknowns.Add<CTestIterableUnknown>();
	pcTest5->Init(5);
	pcTest3 = cUnknowns.Add<CTestIterableUnknown>();
	pcTest3->Init(3);
	pcTest4 = cUnknowns.Add<CTestIterableUnknown>();
	pcTest4->Init(4);
	pcTest1 = cUnknowns.Add<CTestIterableUnknown>();
	pcTest1->Init(1);

	pcTest = cUnknowns.StartIteration(&sIter);
	AssertInt(7, pcTest->miID);
	cUnknowns.RemoveDuringIteration(&sIter);

	pcTest = cUnknowns.Iterate(&sIter);
	AssertInt(5, pcTest->miID);
	cUnknowns.RemoveDuringIteration(&sIter);

	pcTest = cUnknowns.Iterate(&sIter);
	AssertInt(3, pcTest->miID);
	cUnknowns.RemoveDuringIteration(&sIter);
	AssertSize(2, cUnknowns.GetIterableListsHeadNumElements());
	AssertSize(2, cUnknowns.NumElements());

	pcTest = cUnknowns.Iterate(&sIter);
	AssertInt(4, pcTest->miID);
	cUnknowns.RemoveDuringIteration(&sIter);
	AssertSize(1, cUnknowns.GetIterableListsHeadNumElements());
	AssertSize(1, cUnknowns.NumElements());

	pcTest = cUnknowns.Iterate(&sIter);
	cUnknowns.RemoveDuringIteration(&sIter);
	AssertSize(0, cUnknowns.GetIterableListsHeadNumElements());

	cUnknowns.Kill();

	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUnknownsFreeMacro(void)
{
	MemoryInit();
	UnknownsInit();

	CTestUnknown*	pcTest;
	bool			bFree;

	pcTest = UMalloc(CTestUnknown);
	AssertSize(1, gcUnknowns.NumElements());

	//UFree is one statement: under an if that's false it neither frees nor clears.
	bFree = false;
	if (bFree)
		UFree(pcTest);
	AssertNotNull(pcTest);
	AssertSize(1, gcUnknowns.NumElements());

	UFree(pcTest);
	AssertNull(pcTest);
	AssertSize(0, gcUnknowns.NumElements());

	UnknownsKill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUnknownsInitKillTwice(void)
{
	MemoryInit();
	UnknownsInit();

	CTestUnknown*	pcTest;
	CMemoryFile		cLog;
	SLogConfig		sLogConfig;
	char			szLogged[256];

	pcTest = UMalloc(CTestUnknown);

	//A second Init is an error, and must not Init (and lose) the unknowns again.
	StartCapturingLog(&cLog, &sLogConfig);
	UnknownsInit();
	StopCapturingLog(&cLog, &sLogConfig, szLogged, sizeof(szLogged));
	AssertNotNull(strstr(szLogged, "Unknowns has already been initialised."));
	AssertSize(1, gcUnknowns.NumElements());

	UFree(pcTest);
	UnknownsKill();

	StartCapturingLog(&cLog, &sLogConfig);
	UnknownsKill();
	StopCapturingLog(&cLog, &sLogConfig, szLogged, sizeof(szLogged));
	AssertNotNull(strstr(szLogged, "Unknowns has already been Killed."));

	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUnknownsRemoveFromOther(void)
{
	MemoryInit();

	CUnknowns		cA;
	CUnknowns		cB;
	CTestUnknown*	pcTest;
	CMemoryFile		cLog;
	SLogConfig		sLogConfig;
	char			szLogged[512];

	cA.Init("A", &gcConstructors);
	cB.Init("B", &gcConstructors);
	pcTest = cA.Add<CTestUnknown>();
	pcTest->Init();

	//An unknown in another Unknowns isn't removed, and that's an error.
	StartCapturingLog(&cLog, &sLogConfig);
	AssertFalse(cB.Remove(pcTest));
	StopCapturingLog(&cLog, &sLogConfig, szLogged, sizeof(szLogged));
	AssertNotNull(strstr(szLogged, "Cannot remove [CTestUnknown] from Unknowns [B]: it is in Unknowns [A]."));
	AssertSize(1, cA.NumElements());

	AssertFalse(cA.Remove(NULL));
	AssertTrue(cA.Remove(pcTest));
	AssertSize(0, cA.NumElements());

	cB.Kill();
	cA.Kill();

	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUnknownsWithoutConstructors(void)
{
	MemoryInit();

	CUnknowns		cUnknowns;
	CTestUnknown*	pcTest;
	CMemoryFile		cLog;
	SLogConfig		sLogConfig;
	char			szLogged[512];

	//No constructors: Add<M> still works, adding by class name is an error.
	cUnknowns.Init("Plain", NULL);
	AssertSize(0, cUnknowns.GetIterableListsHeadNumElements());

	pcTest = cUnknowns.Add<CTestUnknown>();
	AssertNotNull(pcTest);
	pcTest->Init();

	StartCapturingLog(&cLog, &sLogConfig);
	AssertNull(cUnknowns.Add("CTestUnknown"));
	StopCapturingLog(&cLog, &sLogConfig, szLogged, sizeof(szLogged));
	AssertNotNull(strstr(szLogged, "Unknowns [Plain] has no constructors to make class [CTestUnknown]."));

	pcTest->Kill();
	AssertSize(0, cUnknowns.NumElements());
	cUnknowns.Kill();

	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUnknownsIterablesMissing(void)
{
	MemoryInit();

	CUnknowns								cUnknowns;
	CIterables								cIterables;
	CTestIterableUnknown*					pcIterable;
	CTestIterableUnknown*					pcOther;
	SIteratorTemplate<CTestUnknown>			sIter;

	cUnknowns.Init("Iterables", &gcConstructors);
	pcIterable = cUnknowns.Add<CTestIterableUnknown>();
	pcIterable->Init(1);
	pcOther = cUnknowns.Add<CTestIterableUnknown>();
	pcOther->Init(2);
	AssertSize(2, cUnknowns.GetIterableListsHeadNumElements());

	//A class with no list (it isn't iterable) iterates nothing, and removing from that iteration does nothing.
	sIter.pcList = (CIterableList*)1;
	AssertNull(cUnknowns.StartIteration(&sIter));
	AssertNull(sIter.pcList);
	cUnknowns.RemoveDuringIteration(&sIter);
	AssertSize(2, cUnknowns.NumElements());

	//Remove says whether the list had the unknown.
	cIterables.Init();
	cIterables.Add(pcIterable);
	AssertFalse(cIterables.Remove(pcOther));
	AssertTrue(cIterables.Remove(pcIterable));
	AssertFalse(cIterables.Remove(pcIterable));
	cIterables.Kill();

	pcOther->Kill();
	pcIterable->Kill();
	cUnknowns.Kill();

	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUnknowns(void)
{
	BeginTests();

	TestUnknownListAddKill();
	TestUnknownListIterables();
	TestUnknownListRemoveDuringIteration();
	TestUnknownsFreeMacro();
	TestUnknownsInitKillTwice();
	TestUnknownsRemoveFromOther();
	TestUnknownsWithoutConstructors();
	TestUnknownsIterablesMissing();

	TestStatistics();
}


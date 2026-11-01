#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Unknowns.h"
#include "TestLib/Assert.h"
#include "UnknownTestClasses.h"


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
void TestUnknowns(void)
{
	BeginTests();

	TestUnknownListAddKill();
	TestUnknownListIterables();
	TestUnknownListRemoveDuringIteration();

	TestStatistics();
}


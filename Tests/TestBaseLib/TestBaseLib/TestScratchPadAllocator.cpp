#include "BaseLib/GlobalMemory.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/ScratchPadAllocator.h"
#include "TestLib/Assert.h"
#include "ConstructorsTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestScratchPadAllocatorPop(void)
{
	MemoryInit();

	CScratchPadAllocator		cAlloc;
	CConstructorTestClass*		pcTest1;
	CConstructorTestClass*		pcTest2;

	gcConstructors.Add<CConstructorTestClass>("CConstructorTestClass");

	cAlloc.Init();
	AssertSize(0, cAlloc.GetScratchPad()->GetUsedSize());

	pcTest1 = (CConstructorTestClass*)gcConstructors.Construct("CConstructorTestClass", &cAlloc);
	AssertSize(sizeof(SSPNode) + sizeof(CConstructorTestClass), cAlloc.GetScratchPad()->GetUsedSize());

	pcTest1->k = 7;
	AssertInt(3, pcTest1->Method1());
	AssertInt(7, pcTest1->k);

	pcTest2 = (CConstructorTestClass*)gcConstructors.Construct("CConstructorTestClass", &cAlloc);
	AssertSize(2 * (sizeof(SSPNode) + sizeof(CConstructorTestClass)), cAlloc.GetScratchPad()->GetUsedSize());

	cAlloc.Free(pcTest1);
	AssertSize(2 * (sizeof(SSPNode) + sizeof(CConstructorTestClass)), cAlloc.GetScratchPad()->GetUsedSize());
	cAlloc.Free(pcTest2);
	AssertSize(2 * (sizeof(SSPNode) + sizeof(CConstructorTestClass)), cAlloc.GetScratchPad()->GetUsedSize());

	cAlloc.Kill();

	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestScratchPadAllocatorReadWrite(void)
{
	MemoryInit();

	CScratchPadAllocator	cAlloc;
	CFileBasic				cFile;
	CScratchPadAllocator	cAllocIn;
	int32						i;
	SScratchPadParams		sParams;

	cFile.Init(MemoryFile());
	cFile.Open(EFM_ReadWrite_Create);

	cFile.WriteInt32(789);

	cAlloc.Init(32 KB);
	AssertTrue(cAlloc.Write(&cFile));

	cFile.WriteInt32(124);

	cFile.Close();
	cAlloc.Kill();

	cFile.Open(EFM_Read);

	cFile.ReadInt32(&i);
	AssertInt((int32)789, i);

	AssertTrue(cAllocIn.Read(&cFile));

	cFile.ReadInt32(&i);
	AssertInt((int32)124, i);

	AssertSize(0, cAllocIn.GetScratchPad()->GetMemorySize());
	AssertSize(0, cAllocIn.GetScratchPad()->GetUsedSize());
	cAllocIn.GetScratchPad()->GetParams(&sParams);
	AssertInt(32 KB, sParams.iChunkSize);
	cFile.Close();
	cFile.Kill();
	cAlloc.Kill();

	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestScratchPadAllocator(void)
{
	BeginTests();

	TestScratchPadAllocatorPop();
	TestScratchPadAllocatorReadWrite();

	TestStatistics();
}


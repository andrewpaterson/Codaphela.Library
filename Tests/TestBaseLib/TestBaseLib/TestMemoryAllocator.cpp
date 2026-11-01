#include "BaseLib/GlobalMemory.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/MemoryAllocator.h"
#include "TestLib/Assert.h"
#include "ConstructorsTestClasses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryAllocatorFree(void)
{
	MemoryInit();

	CMemoryAllocator		cAlloc;
	CConstructorTestClass*	pcTest;

	gcConstructors.Add<CConstructorTestClass>("CConstructorTestClass");

	cAlloc.Init();
	AssertSize(0, cAlloc.GetMemory()->NumElements());

	pcTest = (CConstructorTestClass*)gcConstructors.Construct("CConstructorTestClass", &cAlloc);
	AssertSize(1, cAlloc.GetMemory()->NumElements());

	pcTest->k = 7;
	AssertInt(3, pcTest->Method1());
	AssertInt(7, pcTest->k);
	
	cAlloc.Free(pcTest);
	AssertSize(0, cAlloc.GetMemory()->NumElements());

	cAlloc.Kill();

	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryAllocatorReadWrite(void)
{
	MemoryInit();

	CMemoryAllocator		cAlloc;
	CFileBasic				cFile;
	CMemoryAllocator		cAllocIn;
	int32					i;
	SMemoryFreeListParams*	psBlockParams;

	cFile.Init(MemoryFile());
	cFile.Open(EFM_ReadWrite_Create);

	cFile.WriteInt32(789);

	cAlloc.Init(16, false);
	cAlloc.GetMemory()->GetFreeListParams()->AddParamBlock(24  , 16  , 32*32);
	cAlloc.GetMemory()->GetFreeListParams()->AddParamBlock(32  , 24  , 28*32);
	cAlloc.GetMemory()->GetFreeListParams()->AddParamBlock(40  , 32  , 24*32);

	AssertTrue(cAlloc.Write(&cFile));

	cFile.WriteInt32(124);

	cFile.Close();
	cAlloc.Kill();

	cFile.Open(EFM_Read);

	cFile.ReadInt32(&i);
	AssertInt((int32)789, i);

	AssertTrue(cAllocIn.Read(&cFile));

	cFile.ReadInt32(&i);
	cFile.Close();
	cFile.Kill();
	AssertInt((int32)124, i);

#ifdef BUILD_64
	AssertSize(32, cAllocIn.GetMemory()->ByteSize());
#else
	AssertSize(16, cAllocIn.GetMemory()->ByteSize());
#endif // BUILD_64
	AssertSize(16, cAllocIn.GetMemory()->GetDefaultAlignment());
#ifdef BUILD_64
	AssertSize(64, cAllocIn.GetMemory()->GetFreeListParams()->GetFreeListSizeLimit());
#else
	AssertSize(56, cAllocIn.GetMemory()->GetFreeListParams()->GetFreeListSizeLimit());
#endif // BUILD_64
	AssertSize(3, cAllocIn.GetMemory()->GetFreeListParams()->NumParams());

	psBlockParams = cAllocIn.GetMemory()->GetFreeListParams()->GetFreeListParams(0);
	AssertNotNull(psBlockParams);
	AssertSize(32*32, psBlockParams->iChunkSize);
	AssertSize(17, psBlockParams->iMinElementSize);
	AssertSize(24, psBlockParams->iMaxElementSize);

	psBlockParams = cAllocIn.GetMemory()->GetFreeListParams()->GetFreeListParams(1);
	AssertNotNull(psBlockParams);
	AssertSize(28*32, psBlockParams->iChunkSize);
	AssertSize(25, psBlockParams->iMinElementSize);
	AssertSize(32, psBlockParams->iMaxElementSize);

	psBlockParams = cAllocIn.GetMemory()->GetFreeListParams()->GetFreeListParams(2);
	AssertNotNull(psBlockParams);
	AssertSize(24*32, psBlockParams->iChunkSize);
	AssertSize(33, psBlockParams->iMinElementSize);
	AssertSize(40, psBlockParams->iMaxElementSize);

	psBlockParams = cAllocIn.GetMemory()->GetFreeListParams()->GetFreeListParams(3);
	AssertNull(psBlockParams);

	cAllocIn.Kill();
	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryAllocator(void)
{
	BeginTests();

	TestMemoryAllocatorFree();
	TestMemoryAllocatorReadWrite();

	TestStatistics();
}


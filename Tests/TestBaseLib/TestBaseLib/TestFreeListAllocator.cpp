#include "BaseLib/GlobalMemory.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/FreeListAllocator.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFreeListAllocatorReadWrite(void)
{
	MemoryInit();

	CFreeListAllocator		cAlloc;
	CFileBasic				cFile;
	CFreeListAllocator		cAllocIn;
	int32					i;
	SFreeListParams			sParams;

	cFile.Init(MemoryFile());
	cFile.Open(EFM_ReadWrite_Create);

	cFile.WriteInt32(789);

	cAlloc.Init(48, 16, 4);
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

	AssertInt(16, cAllocIn.GetFreeList()->GetAlignment());
	AssertInt(4, cAllocIn.GetFreeList()->GetOffset());
	AssertSize(48, cAllocIn.GetFreeList()->GetElementSize());
	AssertSize(0, cAllocIn.GetFreeList()->ByteSize());
	cAllocIn.GetFreeList()->GetParams(&sParams);
	AssertInt(4, sParams.iOffset);

	cFile.Close();
	cFile.Kill();

	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFreeListAllocator(void)
{
	BeginTests();

	TestFreeListAllocatorReadWrite();

	TestStatistics();
}


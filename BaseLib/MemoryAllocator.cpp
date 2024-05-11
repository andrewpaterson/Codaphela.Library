#include "ConstructorCall.h"
#include "MemoryAllocator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryAllocator::Init(void)
{
	mcMemory.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryAllocator::Init(int uiDefaultAlignment, bool bDefaultFreeListParams)
{
	mcMemory.Init(uiDefaultAlignment, bDefaultFreeListParams);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryAllocator::Kill(void)
{
	mcMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryAllocator::Malloc(size uiSize)
{
	return mcMemory.Add((uint32)uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryAllocator::Malloc(size uiSize, char(**pacDebugName)[4])
{
	void*						pv;
	SGeneralMemoryAllocation*	psGeneralMemoryAllocation;

	pv = mcMemory.Add((uint32)uiSize);
	psGeneralMemoryAllocation = GENERAL_MEMORY_GET_ALLOCATION(pv);
	if (pacDebugName)
	{
		*pacDebugName = &psGeneralMemoryAllocation->szDebug;
	}
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryAllocator::Realloc(void* pv, size uiSize)
{
	void*	pvNew;

	pvNew = mcMemory.Grow(pv, (uint32)uiSize);
	return pvNew;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMemoryAllocator::Free(void* pv)
{
	return mcMemory.Remove(pv);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMemoryAllocator::FreeMultiple(CArrayVoidPtr* pav)
{
	return mcMemory.RemoveMultiple(pav);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
const char* CMemoryAllocator::GetName(void)
{
	return "CMemoryAllocator";
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMemoryAllocator::Read(CFileReader* pcFileReader)
{
	size					uiDefaultAlignment;
	CMemoryFreeListParams*	pcParams;

	if (!pcFileReader->ReadSize(&uiDefaultAlignment))
	{
		return false;
	}

	mcMemory.Init((uint16)uiDefaultAlignment, false);
	pcParams = mcMemory.GetFreeListParams();

	return pcParams->Read(pcFileReader);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMemoryAllocator::Write(CFileWriter* pcFileWriter)
{
	CMemoryFreeListParams*	pcParams;

	if (!pcFileWriter->WriteSize(mcMemory.GetDefaultAlignment()))
	{
		return false;
	}

	pcParams = mcMemory.GetFreeListParams();

	return pcParams->Write(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CGeneralMemory* CMemoryAllocator::GetMemory(void)
{
	return &mcMemory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMemoryAllocator::SizeOffset(void)
{
	return sizeof(SGeneralMemoryAllocation);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLifeInit<CMallocator> CMemoryAllocator::Create(void)
{
	return LifeAlloc<CMemoryAllocator, CMallocator>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLifeInit<CMallocator> CMemoryAllocator::Create(int uiDefaultAlignment, bool bDefaultFreeListParams)
{
	return LifeAlloc<CMemoryAllocator, CMallocator>(uiDefaultAlignment, bDefaultFreeListParams);
}


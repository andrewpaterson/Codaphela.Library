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
void CMemoryAllocator::Init(int iDefaultAlignment, BOOL bDefaultFreeListParams)
{
	mcMemory.Init(iDefaultAlignment, bDefaultFreeListParams);
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
void* CMemoryAllocator::Malloc(size_t tSize)
{
	return mcMemory.Add((unsigned int)tSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryAllocator::Malloc(size_t tSize, char(**pacDebugName)[4])
{
	void*						pv;
	SGeneralMemoryAllocation*	psGeneralMemoryAllocation;

	pv = mcMemory.Add((unsigned int)tSize);
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
void* CMemoryAllocator::Realloc(void* pv, size_t tSize)
{
	void*	pvNew;

	pvNew = mcMemory.Grow(pv, (unsigned int)tSize);
	return pvNew;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMemoryAllocator::Free(void* pv)
{
	return mcMemory.Remove(pv);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMemoryAllocator::FreeMultiple(CArrayVoidPtr* pav)
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
BOOL CMemoryAllocator::Read(CFileReader* pcFileReader)
{
	int						iDefaultAlignment;
	CMemoryFreeListParams*	pcParams;

	if (!pcFileReader->ReadInt(&iDefaultAlignment))
	{
		return FALSE;
	}

	mcMemory.Init(iDefaultAlignment, FALSE);
	pcParams = mcMemory.GetFreeListParams();

	return pcParams->Read(pcFileReader);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMemoryAllocator::Write(CFileWriter* pcFileWriter)
{
	CMemoryFreeListParams*	pcParams;

	if (!pcFileWriter->WriteInt(mcMemory.GetDefaultAlignment()))
	{
		return FALSE;
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
size_t CMemoryAllocator::SizeOffset(void)
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
CLifeInit<CMallocator> CMemoryAllocator::Create(int iDefaultAlignment, BOOL bDefaultFreeListParams)
{
	return LifeAlloc<CMemoryAllocator, CMallocator>(iDefaultAlignment, bDefaultFreeListParams);
}


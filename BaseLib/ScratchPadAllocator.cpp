#include "DataMacro.h"
#include "ScratchPadAllocator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScratchPadAllocator::Init(void)
{
	mcScratchPad.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScratchPadAllocator::Init(int iChunkSize)
{
	mcScratchPad.Init(iChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScratchPadAllocator::Kill(void)
{
	mcScratchPad.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CScratchPadAllocator::Malloc(size_t tSize)
{
	return mcScratchPad.Add(tSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CScratchPadAllocator::Realloc(void* pv, size_t tSize)
{
	SSPNode*	psNode;
	void*		pvNew;

	pvNew = mcScratchPad.Add(tSize); 
	psNode =  DataGetHeader<SSPNode, void>(pv);
	psNode->bUsed = false;
	if (psNode->iSize > tSize)
	{
		memcpy(pvNew, pv, tSize);
	}
	else
	{
		memcpy(pvNew, pv, psNode->iSize);
	}
	return pvNew;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CScratchPadAllocator::Free(void* pv)
{
	SSPNode*	psNode;

	if (pv)
	{
		psNode = DataGetHeader<SSPNode, void>(pv);
		psNode->bUsed = false;
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CScratchPad* CScratchPadAllocator::GetScratchPad(void)
{
	return &mcScratchPad;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CScratchPadAllocator::GetName(void)
{
	return "CScratchPadAllocator";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CScratchPadAllocator::Read(CFileReader* pcFileReader)
{
	//Do not call .Init() before Read().

	SScratchPadParams	sParams;

	if (!pcFileReader->ReadData(&sParams, sizeof(SScratchPadParams)))
	{
		return false;
	}

	mcScratchPad.Init(sParams.iChunkSize);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CScratchPadAllocator::Write(CFileWriter* pcFileWriter)
{
	SScratchPadParams	sParams;

	mcScratchPad.GetParams(&sParams);

	if (!pcFileWriter->WriteData(&sParams, sizeof(SScratchPadParams)))
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CScratchPadAllocator::SizeOffset(void)
{
	return sizeof(SSPNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLifeInit<CMallocator> CScratchPadAllocator::Create(void)
{
	return LifeAlloc<CScratchPadAllocator, CMallocator>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLifeInit<CMallocator> CScratchPadAllocator::Create(int iChunkSize)
{
	return LifeAlloc<CScratchPadAllocator, CMallocator>(iChunkSize);
}


#include "MemoryFreeListParams.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryFreeListParams* SMemoryFreeListParams::Init(unsigned int iFreeListSize, int iPrevSize, int iChunkSize, int iHeaderSize)
{
	iMaxListSize = iFreeListSize;
	iMaxElementSize = iFreeListSize - iHeaderSize;
	if (iPrevSize >= iHeaderSize + 1)
	{
		iMinElementSize = iPrevSize - iHeaderSize + 1;
	}
	else
	{
		iMinElementSize = 1;
	}

	this->iChunkSize = iChunkSize;
	return this;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareFreeListParam(const void* arg1, const void* arg2)
{
	unsigned int			uiElementSize;
	SMemoryFreeListParams*	psParams;

	uiElementSize = *((unsigned int*)arg1);
	psParams = (SMemoryFreeListParams*)arg2;

	if (uiElementSize < psParams->iMinElementSize)
	{
		return -1;
	}
	else if (uiElementSize > psParams->iMaxElementSize)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryFreeListParams::Init(int iHeaderSize, BOOL bDefaultFreeListParams)
{
	miHeaderSize = iHeaderSize;
	if (bDefaultFreeListParams)
	{
		muiFreeListSizeLimit = MEMORY_FREE_LIST_SIZE_LIMIT;
		InitFreeListParams();
	}
	else
	{
		mcParams.Init();
		muiFreeListSizeLimit = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryFreeListParams::Kill(void)
{
	mcParams.Kill();
	miHeaderSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryFreeListParams::InitFreeListParams(void)
{
	SMemoryFreeListParams		sParam;

	mcParams.Init();
	mcParams.Add(sParam.Init(24, 16, 32 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(32, 24, 28 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(40, 32, 24 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(48, 40, 22 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(64, 48, 20 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(80, 64, 18 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(96, 80, 17 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(112, 96, 16 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(160, 112, 12 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(192, 160, 10 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(224, 192, 9 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(256, 224, 8 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(288, 256, 7 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(320, 288, 6 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(384, 320, 5 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(448, 384, 4 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(512, 448, 4 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(576, 512, 3 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(640, 576, 3 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(704, 640, 2 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(768, 704, 2 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(896, 768, 2 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(1024, 896, 2 * 32, miHeaderSize));
	mcParams.Add(sParam.Init(1152, 1024, 32, miHeaderSize));
	mcParams.Add(sParam.Init(1280, 1152, 32, miHeaderSize));
	mcParams.Add(sParam.Init(1408, 1280, 32, miHeaderSize));
	mcParams.Add(sParam.Init(1536, 1408, 32, miHeaderSize));
	mcParams.Add(sParam.Init(1664, 1536, 32, miHeaderSize));
	mcParams.Add(sParam.Init(1792, 1664, 16, miHeaderSize));
	mcParams.Add(sParam.Init(1920, 1792, 16, miHeaderSize));
	mcParams.Add(sParam.Init(muiFreeListSizeLimit, 1920, 16, miHeaderSize));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryFreeListParams::AddParamBlock(unsigned int iFreeListSize, int iPrevSize, int iChunkSize)
{
	SMemoryFreeListParams		sParam;

	iFreeListSize += miHeaderSize;
	iPrevSize += miHeaderSize;

	mcParams.Add(sParam.Init(iFreeListSize, iPrevSize, iChunkSize, miHeaderSize));
	if (muiFreeListSizeLimit < iFreeListSize)
	{
		muiFreeListSizeLimit = iFreeListSize;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryFreeListParams::AddParamBlock(SMemoryFreeListParams* psParam)
{
	mcParams.Add(psParam);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryFreeListParams* CMemoryFreeListParams::GetFreeListParams(int iIndex)
{
	return mcParams.SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryFreeListParams* CMemoryFreeListParams::GetFreeListParamsForSize(size_t iElementSize)
{
	int						iIndex;
	SMemoryFreeListParams*	psParams;

	if ((iElementSize > 0) && (iElementSize <= muiFreeListSizeLimit))
	{
		mcParams.FindInSorted((SMemoryFreeListParams*)&iElementSize, CompareFreeListParam, &iIndex);
		psParams = mcParams.Get(iIndex);
		return psParams;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryFreeListParams::SetFreeListSizeLimit(unsigned int uiFreeListSizeLimit)
{
	muiFreeListSizeLimit = uiFreeListSizeLimit;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMemoryFreeListParams::Read(CFileReader* pcFileReader)
{
	SMemoryFreeListParams	sBlockParams;
	int						i;
	int						iHeaderSize;
	unsigned int			uiFreeListSizeLimit;
	int						iFreeListParams;


	if (!pcFileReader->ReadInt(&iHeaderSize))
	{
		return FALSE;
	}
	if (!pcFileReader->ReadInt((int*)&uiFreeListSizeLimit))
	{
		return FALSE;
	}
	if (!pcFileReader->ReadInt(&iFreeListParams))
	{
		return FALSE;
	}


	Init(iHeaderSize, FALSE);

	for (i = 0; i < iFreeListParams; i++)
	{
		if (!pcFileReader->ReadData(&sBlockParams, sizeof(SMemoryFreeListParams)))
		{
			return FALSE;
		}

		AddParamBlock(&sBlockParams);
	}

	SetFreeListSizeLimit(uiFreeListSizeLimit);

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMemoryFreeListParams::Write(CFileWriter* pcFileWriter)
{
	SMemoryFreeListParams*	psBlockParams;
	int						i;

	if (!pcFileWriter->WriteInt(miHeaderSize))
	{
		return FALSE;
	}
	if (!pcFileWriter->WriteInt(muiFreeListSizeLimit))
	{
		return FALSE;
	}
	if (!pcFileWriter->WriteInt(mcParams.NumElements()))
	{
		return FALSE;
	}

	for (i = 0; i < mcParams.NumElements(); i++)
	{
		psBlockParams = GetFreeListParams(i);
		if (!pcFileWriter->WriteData(psBlockParams, sizeof(SMemoryFreeListParams)))
		{
			return FALSE;
		}
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
unsigned int CMemoryFreeListParams::GetMaxFreeListElementSize(void)
{
	return muiFreeListSizeLimit - miHeaderSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
unsigned int CMemoryFreeListParams::GetFreeListSizeLimit(void) { return muiFreeListSizeLimit; }
int CMemoryFreeListParams::NumParams(void) { return mcParams.NumElements(); }


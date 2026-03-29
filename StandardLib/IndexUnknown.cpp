#include "Unknowns.h"
#include "IndexUnknown.h"


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
void CIndexUnknown::Init(bool bKillElements, bool bOverwriteExisting)
{
	muiFlags = 0;
	SetFlagShort(&muiFlags, INDEX_TREE_KILL_ELEMENT, bKillElements);
	mcIndex.Init();
	mcIndex.SetDataFreeCallback(this);
	mcIndex.SetDataIOCallback(this, this);
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
void CIndexUnknown::Kill(void)
{
	mcIndex.Kill();
	CUnknown::Kill();
	muiFlags = 0;
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
void CIndexUnknown::ReInit(void)
{
	uint16	uiFlags;

	uiFlags = muiFlags;
	mcIndex.ReInit();
	muiFlags = uiFlags;

	//Does not call CUnknown::Kill, Init.
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::Put(uint8* pvKey, size iKeySize, CUnknown* pcValue)
{
	void*	pvData;

	pvData = mcIndex.Put(pvKey, iKeySize, &pcValue, sizeof(CUnknown*));
	return pvData != NULL;
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::Put(char* szKey, CUnknown* pcValue)
{
	void*	pvData;
	size	uiLength;

	uiLength = strlen(szKey);
	pvData = mcIndex.Put((uint8*)szKey, uiLength, &pcValue, sizeof(CUnknown*));
	return pvData != NULL;
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
CUnknown** CIndexUnknown::PutNode(uint8* pvKey, size iKeySize)
{
	void* pvData;

	pvData = mcIndex.Put(pvKey, iKeySize, NULL, sizeof(CUnknown*));
	return (CUnknown**)pvData;
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
CUnknown* CIndexUnknown::Get(uint8* pvKey, size iKeySize)
{
	size		uiDataSize;
	CUnknown*	pcUnknown;
	bool		bFound;

	bFound = mcIndex.Get(pvKey, iKeySize, &pcUnknown, &uiDataSize, sizeof(CUnknown*));
	if (bFound)
	{
		return pcUnknown;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
CUnknown* CIndexUnknown::Get(char* szKey)
{
	size	uiLength;

	uiLength = strlen(szKey);
	return Get((uint8*)szKey, uiLength);
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::Remove(char* szKey)
{
	size	uiLength;

	uiLength = strlen(szKey);
	return Remove((uint8*)szKey, uiLength);
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::Remove(uint8* pvKey, size iKeySize)
{
	return mcIndex.Remove(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
size CIndexUnknown::Remove(CUnknown* pvData)
{
	SIndexTreeMemoryIterator	sIter;
	CUnknown*					pcUnknown;
	CUnknown*					pcUnknownNext;
	bool						bExists;
	uint8						auiKey[MAX_KEY_SIZE];
	size						uiKeySize;
	bool						bResult;
	size						uiCount;

	uiCount = 0;
	bExists = StartIteration(&sIter, auiKey, &uiKeySize, MAX_KEY_SIZE, &pcUnknown);
	while (bExists)
	{
		//Iterate must be called before the node is removed.
		bExists = Iterate(&sIter, NULL, NULL, 0, &pcUnknownNext);
		if (pcUnknown == pvData)
		{
			bResult = Remove(auiKey, uiKeySize);
			if (!bResult)
			{
				return ARRAY_ELEMENT_NOT_FOUND;
			}
			uiCount++;
		}
		pcUnknown = pcUnknownNext;
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
size CIndexUnknown::NumElements(void)
{
	return mcIndex.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
size CIndexUnknown::NonNullElements(void)
{
	return mcIndex.NonNullElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::WriteIndexUnknownHeader(CFileWriter* pcFileWriter)
{
	ReturnOnFalse(pcFileWriter->WriteInt16(muiFlags));
	return mcIndex.WriteIndexTreeHeader(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
size CIndexUnknown::ReadIndexUnknownHeader(CFileReader* pcFileReader)
{
	if (!pcFileReader->ReadInt16(&muiFlags))
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}

	return mcIndex.ReadIndexTreeHeader(pcFileReader);
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
size CIndexUnknown::ReadIndexUnknownHeader(CFileReader* pcFileReader, CDataIO* pcDataIO, CIndexTreeDataSize* pcDataSize, CDataFree* pcDataFree)
{
	if (!pcFileReader->ReadInt16(&muiFlags))
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	return mcIndex.ReadIndexTreeHeader(pcFileReader, pcDataIO, pcDataSize, pcDataFree);
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::Save(CFileWriter* pcFileWriter)
{
	ReturnOnFalse(WriteIndexUnknownHeader(pcFileWriter));
	return mcIndex.WriteElements(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::Load(CFileReader* pcFileReader)
{
	size	iCount;

	iCount = ReadIndexUnknownHeader(pcFileReader, this, this, this);
	if (iCount == ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}
	return mcIndex.ReadElements(pcFileReader, iCount);
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::IsKillElements(void)
{
	return muiFlags & INDEX_TREE_KILL_ELEMENT;
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::IsEmpty(void)
{
	return NumElements() == 0;
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::IsNotEmpty(void)
{
	return NumElements() != 0;
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::StartIteration(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, CUnknown** ppvData)
{
	return mcIndex.StartIteration(psIterator, pvKey, piKeySize, iMaxKeySize, ppvData, NULL, sizeof(CUnknown*));
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::Iterate(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, CUnknown** ppvData)
{
	return mcIndex.Iterate(psIterator, pvKey, piKeySize, iMaxKeySize, ppvData, NULL, sizeof(CUnknown*));
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, CUnknown** ppvData)
{
	CUnknown**	ppcValue;
	bool		bResult;

	bResult = mcIndex.StartIteration(psIterator, pvKey, piKeySize, iMaxKeySize, (void**)&ppcValue, NULL);
	if (bResult)
	{
		SafeAssign(ppvData, *ppcValue);
	}
	else
	{
		SafeAssign(ppvData, NULL);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, CUnknown** ppvData)
{
	CUnknown**	ppcValue;
	bool		bResult;

	bResult = mcIndex.Iterate(psIterator, pvKey, piKeySize, iMaxKeySize, (void**)&ppcValue, NULL);
	if (bResult)
	{
		SafeAssign(ppvData, *ppcValue);
	}
	else
	{
		SafeAssign(ppvData, NULL);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, CUnknown** ppvData)
{
	CUnknown**	ppcValue;
	bool		bResult;

	bResult = mcIndex.StartIteration(psIterator, (void**)&ppcValue, NULL);
	if (bResult)
	{
		SafeAssign(ppvData, *ppcValue);
	}
	else
	{
		SafeAssign(ppvData, NULL);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, CUnknown** ppvData)
{
	CUnknown**	ppcValue;
	bool		bResult;

	bResult = mcIndex.Iterate(psIterator, (void**)&ppcValue, NULL);
	if (bResult)
	{
		SafeAssign(ppvData, *ppcValue);
	}
	else
	{
		SafeAssign(ppvData, NULL);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexUnknown::FreeData(void* pvData)
{
	CUnknown**	ppcValue;

	if (muiFlags & INDEX_TREE_KILL_ELEMENT)
	{
		ppcValue = (CUnknown**)pvData;
		if (ppcValue)
		{
			(*ppcValue)->Kill();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::WriteData(CFileWriter* pcFileWriter, void* pvData)
{
	SIndexIOElement*	psElement;
	CUnknown*			pcUnknownValue;
	bool				bResult;

	psElement = (SIndexIOElement*)pvData;
	pcUnknownValue = *((CUnknown**)psElement->pvData);
	bResult = pcUnknownValue->SaveHeader(pcFileWriter);
	if (!bResult)
	{
		return false;
	}
	bResult = pcUnknownValue->Save(pcFileWriter);
	if (!bResult)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::ReadData(CFileReader* pcFileReader, void* pvData)
{
	SIndexIOElement*	psElement;
	CUnknown**			ppcUnknownValue;
	CUnknown*			pcUnknownValue;
	bool				bResult;

	psElement = (SIndexIOElement*)pvData;
	pvData = mcIndex.Put((uint8*)psElement->pvData, psElement->iKeySize, NULL, psElement->iValueSize);
	if (!pvData)
	{
		return false;
	}

	ppcUnknownValue = (CUnknown**)pvData;
	pcUnknownValue = gcUnknowns.AddFromHeader(pcFileReader);
	if (!pcUnknownValue)
	{
		return false;
	}
	bResult = pcUnknownValue->Load(pcFileReader);
	if (!bResult)
	{
		return false;
	}
	*ppcUnknownValue = pcUnknownValue;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
size CIndexUnknown::AdjustDataSize(void* pvValue, size iValueSize)
{
	return iValueSize;
}


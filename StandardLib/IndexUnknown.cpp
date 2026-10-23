#include "Unknowns.h"
#include "IndexUnknown.h"


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
void CIndexUnknown::_Init(void)
{
	muiFlags = 0;
	mcIndex._Init();
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
void CIndexUnknown::Init(bool bKillElements, bool bOverwriteExisting, EIndexKeyReverse eKeyReverse)
{
	muiFlags = 0;
	SetFlagShort(&muiFlags, INDEX_TREE_KILL_ELEMENT, bKillElements);
	SetFlagShort(&muiFlags, INDEX_TREE_NO_OVERWRITE, !bOverwriteExisting);
	mcIndex.Init(eKeyReverse);
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
	void*		pvData;
	CUnknown*	pcOldValue;
	size		uiDataSize;

	if (mcIndex.Get(pvKey, iKeySize, &pcOldValue, &uiDataSize, sizeof(CUnknown*)))
	{
		if (!IsOverwriteExisting())
		{
			return false;
		}

		//The index frees the old value when it is replaced, so the value being put again mustn't be put.
		if (pcOldValue == pcValue)
		{
			return true;
		}
	}

	pvData = mcIndex.Put(pvKey, iKeySize, &pcValue, sizeof(CUnknown*));
	return pvData != NULL;
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::Put(const char* szKey, CUnknown* pcValue)
{
	return Put((uint8*)szKey, strlen(szKey), pcValue);
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
CUnknown* CIndexUnknown::Get(const char* szKey)
{
	size	uiLength;

	uiLength = strlen(szKey);
	return Get((uint8*)szKey, uiLength);
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::Remove(const char* szKey)
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
	uint8						auiKey1[MAX_KEY_SIZE];
	uint8						auiKey2[MAX_KEY_SIZE];
	uint8*						puiKey;
	uint8*						puiKeyNext;
	uint8*						puiKeySwap;
	size						uiKeySize;
	size						uiKeySizeNext;
	bool						bResult;
	size						uiCount;

	uiCount = 0;
	puiKey = auiKey1;
	puiKeyNext = auiKey2;
	bExists = StartIteration(&sIter, puiKey, &uiKeySize, MAX_KEY_SIZE, &pcUnknown);
	while (bExists)
	{
		//Iterate must be called before the node is removed.  The next key is needed as well as the next value.
		bExists = Iterate(&sIter, puiKeyNext, &uiKeySizeNext, MAX_KEY_SIZE, &pcUnknownNext);
		if (pcUnknown == pvData)
		{
			bResult = Remove(puiKey, uiKeySize);
			if (!bResult)
			{
				return ARRAY_ELEMENT_NOT_FOUND;
			}
			uiCount++;
		}
		pcUnknown = pcUnknownNext;
		uiKeySize = uiKeySizeNext;
		puiKeySwap = puiKey;
		puiKey = puiKeyNext;
		puiKeyNext = puiKeySwap;
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
bool CIndexUnknown::IsOverwriteExisting(void)
{
	return !(muiFlags & INDEX_TREE_NO_OVERWRITE);
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
		if (ppcValue && *ppcValue)
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

	psElement = (SIndexIOElement*)pvData;
	return gcUnknowns.SaveUnknown(pcFileWriter, *((CUnknown**)psElement->pvData));
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::ReadData(CFileReader* pcFileReader, void* pvData)
{
	SIndexIOElement*	psElement;
	CUnknown**			ppcUnknownValue;

	psElement = (SIndexIOElement*)pvData;
	ppcUnknownValue = (CUnknown**)mcIndex.Put((uint8*)psElement->pvData, psElement->iKeySize, NULL, psElement->iValueSize);
	if (!ppcUnknownValue)
	{
		return false;
	}

	*ppcUnknownValue = NULL;
	return gcUnknowns.LoadUnknown(pcFileReader, ppcUnknownValue);
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
size CIndexUnknown::AdjustDataSize(void* pvValue, size iValueSize)
{
	return iValueSize;
}


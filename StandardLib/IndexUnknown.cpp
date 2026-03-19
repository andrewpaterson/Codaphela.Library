#include "Unknowns.h"
#include "IndexUnknown.h"


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
void CIndexUnknown::Init(bool bKillElements, bool bOverwriteExisting)
{
	miFlags = 0;
	SetFlagShort(&miFlags, INDEX_TREE_KILL_ELEMENT, bKillElements);
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
size CIndexUnknown::NumElements(void)
{
	return mcIndex.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::Save(CFileWriter* pcFile)
{
	return mcIndex.Write(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::Load(CFileReader* pcFile)
{
	mcIndex.SetDataFreeCallback(this);
	return mcIndex.Read(pcFile, this, this);
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::IsKillElements(void)
{
	return miFlags & INDEX_TREE_KILL_ELEMENT;
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
	size	uiDataSize;

	return mcIndex.StartIteration(psIterator, pvKey, piKeySize, iMaxKeySize, ppvData, &uiDataSize, sizeof(CUnknown*));
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
bool CIndexUnknown::Iterate(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, CUnknown** ppvData)
{
	size	uiDataSize;

	return mcIndex.Iterate(psIterator, pvKey, piKeySize, iMaxKeySize, ppvData, &uiDataSize, sizeof(CUnknown*));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexUnknown::FreeData(void* pvData)
{
	CUnknown**	ppcValue;

	if (miFlags & INDEX_TREE_KILL_ELEMENT)
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


/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "MarkupTag.h"
#include "MarkupText.h"
#include "MarkupReference.h"
#include "Markup.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupTag::Init(CMarkupTag* pcParent)
{
	CMarkupBase::Init(MUT_Tag, pcParent);
	macBases.Init();
	mcNamedAttributes.Init();
	mcOrderedAttributes.Init();
	mszName.Init();
	miLine = ARRAY_ELEMENT_NOT_FOUND;
	miColumn = ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupTag::Init(char* szName, CMarkupTag* pcParent)
{
	CMarkupBase::Init(MUT_Tag, pcParent);
	macBases.Init();
	mcNamedAttributes.Init();
	mcOrderedAttributes.Init();
	mszName.Init(szName);
	miLine = ARRAY_ELEMENT_NOT_FOUND;
	miColumn = ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupTag::Kill(void)
{
	CMarkupBase*	pcBase;
	size			i;
	size			uiNumElements;
	CMallocator*	pcMalloc;
	SMapIterator	sIter;
	void*			pvData;
	bool			bValid;
	size			uiType;

	mszName.Kill();

	pcMalloc = GetMalloc();
	bValid = mcNamedAttributes.StartIteration(&sIter, NULL, NULL, &pvData, &uiType);
	while (bValid)
	{
		if (pvData)
		{
			pcMalloc->Free(pvData);
		}
		bValid = mcNamedAttributes.Iterate(&sIter, NULL, NULL, &pvData, &uiType);
	}
	mcNamedAttributes.Kill();

	uiNumElements = mcOrderedAttributes.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		mcOrderedAttributes.Get(i, &pvData, &uiType);
		if (pvData)
		{
			pcMalloc->Free(pvData);
		}
	}
	mcOrderedAttributes.Kill();

	uiNumElements = macBases.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcBase = *(macBases.Get(i));
		pcBase->Kill();
	}
	macBases.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupTag::IsEmpty(void)
{
	return macBases.NumElements() == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMarkupTag::GetNamedAttribute(char* szAttribute, uint* puiType)
{
	return mcNamedAttributes.Get(szAttribute, puiType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMarkupTag::GetNumOrderedAttributes(void)
{
	return mcOrderedAttributes.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMarkupTag::GetOrderedAttribute(size uiIndex, uint* puiType)
{
	void*	pvData;

	mcOrderedAttributes.Get(uiIndex, &pvData, puiType);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupTag* CMarkupTag::GetTag(char* szTagName, STagIterator* psIter)
{
	STagIterator	sTemp;

	if (!psIter)
	{
		psIter = &sTemp;
	}

	psIter->iIndex = ARRAY_ELEMENT_NOT_FOUND;
	psIter->mpcCurrent = NULL;
	psIter->bNamed = true;
	return GetTagFromIndex(szTagName, psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupTag* CMarkupTag::GetTag(char* szTagName, size uiTagNumber)
{
	STagIterator	sIter;
	CMarkupTag*		pcTag;
	size			i;

	sIter.iIndex = ARRAY_ELEMENT_NOT_FOUND;
	sIter.mpcCurrent = NULL;
	sIter.bNamed = true;

	pcTag = NULL;
	if (uiTagNumber != ARRAY_ELEMENT_NOT_FOUND)
	{
		for (i = 0; i <= uiTagNumber; i++)
		{
			pcTag = GetTagFromIndex(szTagName, &sIter);
			if (!pcTag)
			{
				return NULL;
			}
		}
	}
	return pcTag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupTag* CMarkupTag::GetTagFromIndex(char* szTagName, STagIterator* psIter)
{
	CMarkupBase*	pcBase;
	CMarkupTag*		pcTag;
	CMarkupRefDoc*	pcRefDoc;
	size			i;
	size			iIndex;
	size			uiNumElements;

	uiNumElements = macBases.NumElements();
	iIndex = psIter->iIndex +1;
	for (i = iIndex; i < uiNumElements; i++)
	{
		pcBase = *(macBases.Get(i));
		if (pcBase->IsTag())
		{
			pcTag = (CMarkupTag*)pcBase;
			if (pcTag->Is(szTagName))
			{
				psIter->iIndex = i;
				psIter->mpcCurrent = pcTag;
				return pcTag;
			}
		}
		else if (pcBase->IsRefDoc())
		{
			pcRefDoc = (CMarkupRefDoc*)pcBase;
			if (pcRefDoc->Is(szTagName))
			{
				pcTag = pcRefDoc->GetRootTag(); 
				psIter->iIndex = i;
				psIter->mpcCurrent = pcTag;
				return pcTag;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupTag* CMarkupTag::GetTagFromIndex(STagIterator* psIter)
{
	CMarkupBase*	pcBase;
	CMarkupTag*		pcTag;
	CMarkupRefDoc*	pcRefDoc;
	size			i;
	size			iIndex;
	size			uiNumElements;

	uiNumElements = macBases.NumElements();
	iIndex = psIter->iIndex + 1;
	for (i = iIndex; i < uiNumElements; i++)
	{
		pcBase = *(macBases.Get(i));
		if (pcBase->IsTag())
		{
			pcTag = (CMarkupTag*)pcBase;
			psIter->iIndex = i;
			psIter->mpcCurrent = pcTag;
			return pcTag;
		}
		else if (pcBase->IsRefDoc())
		{
			pcRefDoc = (CMarkupRefDoc*)pcBase;
			pcTag = pcRefDoc->GetRootTag(); 
			psIter->iIndex = i;
			psIter->mpcCurrent = pcTag;
			return pcTag;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupTag* CMarkupTag::GetTag(STagIterator* psIter)
{
	STagIterator	sTemp;

	if (!psIter)
	{
		psIter = &sTemp;
	}

	psIter->iIndex = ARRAY_ELEMENT_NOT_FOUND;
	psIter->mpcCurrent = NULL;
	psIter->bNamed = false;
	return GetTagFromIndex(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupTag* CMarkupTag::GetNextTag(STagIterator* psIter)
{
	if (psIter->bNamed)
	{
		return GetTagFromIndex(psIter->mpcCurrent->mszName.Text(), psIter);
	}
	else
	{
		return GetTagFromIndex(psIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupTag::Is(char* szName)
{
	return mszName.Equals(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupTag::GetText(CChars* psz, bool bFirstContiguous, bool bFirstTag)
{
	CMarkupBase*	pcBase;
	CMarkupText*	pcText;
	CMarkupRefText*	pcRefText;
	size			i;
	bool			bContiguous;
	bool			bPreviousTag;
	size			uiNumElements;

	uiNumElements = macBases.NumElements();
	bContiguous = true;
	bPreviousTag = false;
	for (i = 0; i < uiNumElements; i++)
	{
		pcBase = *(macBases.Get(i));
		if (pcBase->IsText())
		{
			if (bPreviousTag)
			{
				bContiguous = false;
				if (bFirstContiguous)
				{
					break;
				}
			}
			pcText = (CMarkupText*)pcBase;
			psz->Append(pcText->mszText);
			bPreviousTag = false;

			if (bFirstTag)
			{
				break;
			}
		}
		else if (pcBase->IsRefText())
		{
			if (bPreviousTag)
			{
				bContiguous = false;
				if (bFirstContiguous)
				{
					break;
				}
			}
			pcRefText = (CMarkupRefText*)pcBase;
			psz->Append(pcRefText->mpcRef->mszText);
			bPreviousTag = false;

			if (bFirstTag)
			{
				break;
			}
		}
		else if (pcBase->IsTag())
		{
			bPreviousTag = true;
		}
		else if (pcBase->IsRefDoc())
		{
			bPreviousTag = true;
		}
	}
	return bContiguous;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupTag::SetName(char* szName)
{
	mszName.Set(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CMarkupTag::GetName(void)
{
	return mszName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupTag* CMarkupTag::AppendTag(void)
{
	CMarkupTag*	pcTag;

	pcTag = mpcDoc->mpcMarkup->AllocateTag(mpcDoc);
	pcTag->Init(this);
	macBases.Add((CMarkupBase**)&pcTag);
	return pcTag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupTag* CMarkupTag::AppendTag(char* szName)
{
	CMarkupTag*	pcTag;

	pcTag = mpcDoc->mpcMarkup->AllocateTag(mpcDoc);
	pcTag->Init(szName, this);
	macBases.Add((CMarkupBase**)&pcTag);
	return pcTag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupText* CMarkupTag::AppendText(void)
{
	CMarkupText*	pcText;

	pcText = mpcDoc->mpcMarkup->AllocateText(mpcDoc);
	pcText->Init(this);
	macBases.Add((CMarkupBase**)&pcText);
	return pcText;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupText* CMarkupTag::AppendText(char* szText)
{
	CMarkupText*	pcText;

	pcText = mpcDoc->mpcMarkup->AllocateText(mpcDoc);
	pcText->Init(szText, this);
	macBases.Add((CMarkupBase**)&pcText);
	return pcText;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupNamedRef* CMarkupTag::AppendNamedReference(char* szIdentifier)
{
	CMarkupNamedRef*	pcNamedRef;

	pcNamedRef = mpcDoc->mpcMarkup->AllocateNamedRef(mpcDoc);
	pcNamedRef->Init(szIdentifier, this);
	macBases.Add((CMarkupBase**)&pcNamedRef);
	mpcDoc->AddReference((CMarkupReference*)pcNamedRef);
	return pcNamedRef;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupTag::AddStringAttribute(char* szAttribute, char* szString)
{
	CMallocator*	pcMalloc;
	size			uiLength;
	char*			szAllocatedString;

	if (mcNamedAttributes.Get(szAttribute))
	{
		return false;
	}
	
	pcMalloc = GetMalloc();

	uiLength = strlen(szString) + 1;

	szAllocatedString = (char*)pcMalloc->Malloc(uiLength);
	memcpy(szAllocatedString, szString, uiLength);

	mcNamedAttributes.Put(szAttribute, PT_char8Pointer, szAllocatedString);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupTag::AddStringAttribute(char* szString)
{
	CMallocator*	pcMalloc;
	size			uiLength;
	char*			szAllocatedString;

	pcMalloc = GetMalloc();

	uiLength = strlen(szString) + 1;

	szAllocatedString = (char*)pcMalloc->Malloc(uiLength);
	memcpy(szAllocatedString, szString, uiLength);

	mcOrderedAttributes.Add(szAllocatedString, PT_char8Pointer);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupTag::AddNumberAttribute(char* szAttribute, CNumber* pcNumber)
{
	CMallocator*	pcMalloc;
	size			uiByteSize;
	CNumber*		pcAllocatedNumber;

	if (mcNamedAttributes.Get(szAttribute))
	{
		return false;
	}

	pcMalloc = GetMalloc();

	uiByteSize = pcNumber->ByteSize();

	pcAllocatedNumber = (CNumber*)pcMalloc->Malloc(uiByteSize);
	memcpy(pcAllocatedNumber, pcNumber, uiByteSize);

	mcNamedAttributes.Put(szAttribute, PT_Number, pcAllocatedNumber);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupTag::AddNumberAttribute(CNumber* pcNumber)
{
	CMallocator*	pcMalloc;
	size			uiByteSize;
	CNumber*		pcAllocatedNumber;

	pcMalloc = GetMalloc();

	uiByteSize = pcNumber->ByteSize();

	pcAllocatedNumber = (CNumber*)pcMalloc->Malloc(uiByteSize);
	memcpy(pcAllocatedNumber, pcNumber, uiByteSize);

	mcOrderedAttributes.Add(pcAllocatedNumber, PT_Number);
	return true;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupTag::AddBooleanAttribute(char* szAttribute, bool bValue)
{
	CMallocator*	pcMalloc;
	bool*			pbValue;

	if (mcNamedAttributes.Get(szAttribute))
	{
		return false;
	}

	pcMalloc = GetMalloc();

	pbValue = (bool*)pcMalloc->Malloc(sizeof(bool));
	*pbValue = bValue;

	mcNamedAttributes.Put(szAttribute, PT_bool, pbValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupTag::AddBooleanAttribute(bool bValue)
{
	CMallocator*	pcMalloc;
	bool*			pbValue;

	pcMalloc = GetMalloc();

	pbValue = (bool*)pcMalloc->Malloc(sizeof(bool));
	*pbValue = bValue;

	mcOrderedAttributes.Add(pbValue, PT_bool);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupTag::AddNullAttribute(char* szAttribute)
{
	CMallocator*	pcMalloc;

	if (mcNamedAttributes.Get(szAttribute))
	{
		return false;
	}

	pcMalloc = GetMalloc();

	mcNamedAttributes.Put(szAttribute, PT_void, NULL);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupTag::AddNullAttribute(void)
{
	CMallocator* pcMalloc;


	pcMalloc = GetMalloc();

	mcOrderedAttributes.Add(NULL, PT_void);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupTag::ContainsOnlyText(void)
{
	size			i;
	CMarkupBase*	pcBase;
	size			uiNumElements;

	uiNumElements = macBases.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcBase = *macBases.Get(i);
		if ((!pcBase->IsText()) && (!pcBase->IsRefText()))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupTag::Swap(CMarkupBase* pcNew, CMarkupBase* pcOld)
{
	size			i;
	CMarkupBase*	pcBase;
	size			uiNumElements;

	uiNumElements = macBases.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcBase = *macBases.Get(i);
		if (pcBase == pcOld)
		{
			macBases.Set(i, &pcNew);
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMallocator* CMarkupTag::GetMalloc(void)
{
	return mpcDoc->GetMarkup()->GetMemory();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupTag::Print(CChars* psz)
{
	Print(psz, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupTag::PrintType(CChars* psz, char* szValue, uint uiType)
{
	CNumber*	pcNumber;

	if (uiType == PT_char8Pointer)
	{
		psz->Append('"');
		psz->Append(szValue);
		psz->Append('"');
	}
	else if (uiType == PT_Number)
	{
		pcNumber = (CNumber*)szValue;
		pcNumber->Print(psz);
	}
	else if (uiType == PT_bool)
	{
		if (*((bool*)szValue))
		{
			psz->Append("true");
		}
		else
		{
			psz->Append("false");
		}
	}
	else if (uiType == PT_void)
	{
		psz->Append("null");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMarkupTag::Print(CChars* psz, size iDepth, size iLine)
{
	size			i;
	CMarkupBase*	pcBase;
	CChars			szText;
	bool			bPadClosing;
	char*			szKey;
	SMapIterator	sIter;
	bool			bResult;
	size			uiNumBases;
	size			uiNumNamedAttributes;
	size			uiNumOrderedAttributes;
	uint			uiType;
	char*			szValue;
	bool			bAddSpace;
	bool			bNameEmpty;

	mcNamedAttributes.FinaliseSorted();

	uiNumBases = macBases.NumElements();
	uiNumNamedAttributes = mcNamedAttributes.NumElements();
	uiNumOrderedAttributes = mcOrderedAttributes.NumElements();
	bNameEmpty = mszName.Empty();
	
	miLine = iLine;
	miColumn = iDepth * 2;

	if ((uiNumBases != 0) || (uiNumNamedAttributes != 0) || (uiNumOrderedAttributes != 0) || !bNameEmpty)
	{
		psz->Append(' ', iDepth * 2);

		if ((uiNumBases != 0) || (uiNumNamedAttributes != 0) || !bNameEmpty)
		{
			psz->Append('<');
		}
		else
		{
			psz->Append('[');
		}

		bAddSpace = false;
		if (!bNameEmpty)
		{
			psz->Append(mszName);
			bAddSpace = true;
		}

		bResult = mcNamedAttributes.StartIteration(&sIter, (void**)&szKey, NULL, (void**)&szValue, &uiType);
		while (bResult)
		{
			if (bAddSpace)
			{
				psz->Append(' ');
			}
			bAddSpace = true;

			psz->Append(szKey);
			psz->Append('=');
			PrintType(psz, szValue, uiType);

			bResult = mcNamedAttributes.Iterate(&sIter, (void**)&szKey, NULL, (void**)&szValue, &uiType);
		}

		for (i = 0; i < uiNumOrderedAttributes; i++)
		{
			bResult = mcOrderedAttributes.Get(i, (void**)&szValue, &uiType);
			if (bResult)
			{
				if (i != 0)
				{
					psz->Append(",");
				}
				if (bAddSpace)
				{
					psz->Append(' ');
				}
				bAddSpace = true;

				PrintType(psz, szValue, uiType);
			}
		}

		if ((uiNumBases != 0) || (uiNumNamedAttributes != 0) || !bNameEmpty)
		{
			if (uiNumBases == 0)
			{
				psz->Append('/');
			}
			psz->Append('>');
		}
		else
		{
			psz->Append(']');
		}

		iLine++;
		psz->AppendNewLine();

		if (uiNumBases > 0)
		{
			bPadClosing = true;
			if (ContainsOnlyText())
			{
				szText.Init();
				GetText(&szText);
				iLine = CMarkupBase::Print(psz, &szText, iDepth + 1, iLine, true);
				szText.Kill();
			}
			else
			{
				for (i = 0; i < uiNumBases; i++)
				{
					pcBase = *macBases.Get(i);
					iLine = pcBase->Print(psz, iDepth + 1, iLine);
				}
			}

			if (miLine != iLine)
			{
				psz->Append(' ', iDepth * 2);
			}
			psz->Append('<');
			psz->Append('/');
			psz->Append(mszName);
			psz->Append('>');
			iLine++;
			psz->AppendNewLine();
		}
	}
	return iLine;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupTag::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.Dump();
	sz.Kill();
}


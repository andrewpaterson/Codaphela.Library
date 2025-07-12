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
	mcAttributes.Init();
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
	mcAttributes.Init();
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
	void*			pvData;
	size			uiType;
	char*			szName;

	mszName.Kill();

	pcMalloc = GetMalloc();

	uiNumElements = mcAttributes.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		mcAttributes.Get(i, &pvData, &uiType, &szName);
		if (pvData)
		{
			pcMalloc->Free(pvData);
		}
		if (szName)
		{
			pcMalloc->Free(szName);
		}
	}
	mcAttributes.Kill();

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
bool CMarkupTag::GetAttribute(char* szAttribute, void** ppvData, uint* puiType)
{
	size			i;
	size			uiNumElements;
	CMallocator*	pcMalloc;
	void*			pvData;
	size			uiType;
	char*			szName;
	bool			bExists;

	pcMalloc = GetMalloc();

	uiNumElements = mcAttributes.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		bExists = mcAttributes.Get(i, &pvData, &uiType, &szName);
		if (bExists && szName)
		{
			if (strcmp(szAttribute, szName) == 0)
			{
				SafeAssign(puiType, uiType);
				SafeAssign(ppvData, pvData);
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMarkupTag::GetNumAttributes(void)
{
	return mcAttributes.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMarkupTag::GetAttribute(size uiIndex, uint* puiType, char** szName)
{
	void*	pvData;

	mcAttributes.Get(uiIndex, &pvData, puiType, szName);
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
char* CMarkupTag::MallocString(char* szSource)
{
	size			uiLength;
	char*			szAllocatedString;
	CMallocator*	pcMalloc;

	pcMalloc = GetMalloc();

	uiLength = strlen(szSource) + 1;
	szAllocatedString = (char*)pcMalloc->Malloc(uiLength);
	if (szAllocatedString)
	{
		memcpy(szAllocatedString, szSource, uiLength);
	}

	return szAllocatedString;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupTag::AddStringAttribute(char* szAttribute, char* szString, bool bAllowDuplicates)
{
	char*	szAllocatedString;
	char*	szAllocatedName;
	bool	bExists;

	if (StrEmpty(szAttribute))
	{
		return false;
	}

	if (!bAllowDuplicates)
	{
		bExists = GetAttribute(szAttribute, NULL, NULL);
		if (bExists)
		{
			return false;
		}
	}
	
	szAllocatedName = MallocString(szAttribute);
	szAllocatedString = MallocString(szString);

	mcAttributes.Add(szAllocatedString, PT_char8Pointer, szAllocatedName);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupTag::AddStringAttribute(char* szString)
{
	char*			szAllocatedString;

	szAllocatedString = MallocString(szString);

	mcAttributes.Add(szAllocatedString, PT_char8Pointer, NULL);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupTag::AddNumberAttribute(char* szAttribute, CNumber* pcNumber, bool bAllowDuplicates)
{
	CMallocator*	pcMalloc;
	size			uiByteSize;
	CNumber*		pcAllocatedNumber;
	char*			szAllocatedName;
	bool			bExists;

	if (!bAllowDuplicates)
	{
		bExists = GetAttribute(szAttribute, NULL, NULL);
		if (bExists)
		{
			return false;
		}
	}

	szAllocatedName = MallocString(szAttribute);

	pcMalloc = GetMalloc();
	uiByteSize = pcNumber->ByteSize();
	pcAllocatedNumber = (CNumber*)pcMalloc->Malloc(uiByteSize);
	memcpy(pcAllocatedNumber, pcNumber, uiByteSize);

	mcAttributes.Add(pcAllocatedNumber, PT_Number, szAllocatedName);
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

	mcAttributes.Add(pcAllocatedNumber, PT_Number, NULL);
	return true;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupTag::AddBooleanAttribute(char* szAttribute, bool bValue, bool bAllowDuplicates)
{
	CMallocator*	pcMalloc;
	bool*			pbValue;
	char*			szAllocatedName;
	bool			bExists;

	if (!bAllowDuplicates)
	{
		bExists = GetAttribute(szAttribute, NULL, NULL);
		if (bExists)
		{
			return false;
		}
	}

	szAllocatedName = MallocString(szAttribute);

	pcMalloc = GetMalloc();
	pbValue = (bool*)pcMalloc->Malloc(sizeof(bool));
	*pbValue = bValue;


	mcAttributes.Add(pbValue, PT_bool, szAllocatedName);
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

	mcAttributes.Add(pbValue, PT_bool, NULL);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupTag::AddNullAttribute(char* szAttribute, bool bAllowDuplicates)
{
	CMallocator*	pcMalloc;
	bool			bExists;

	if (!bAllowDuplicates)
	{
		bExists = GetAttribute(szAttribute, NULL, NULL);
		if (bExists)
		{
			return false;
		}
	}

	pcMalloc = GetMalloc();

	mcAttributes.Add(NULL, PT_void, szAttribute);
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

	mcAttributes.Add(NULL, PT_void, NULL);
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
	bool			bResult;
	size			uiNumBases;
	size			uiNumOrderedAttributes;
	uint			uiType;
	char*			szValue;
	bool			bAddSpace;
	bool			bNameEmpty;

	uiNumBases = macBases.NumElements();
	uiNumOrderedAttributes = mcAttributes.NumElements();
	bNameEmpty = mszName.Empty();
	
	miLine = iLine;
	miColumn = iDepth * 2;

	if ((uiNumBases != 0) || (uiNumOrderedAttributes != 0) || !bNameEmpty)
	{
		psz->Append(' ', iDepth * 2);

		if ((uiNumBases != 0) || !bNameEmpty)
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


		for (i = 0; i < uiNumOrderedAttributes; i++)
		{
			bResult = mcAttributes.Get(i, (void**)&szValue, &uiType, &szKey);
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

				if (szKey)
				{
					psz->Append(szKey);
					psz->Append('=');
				}
				PrintType(psz, szValue, uiType);
			}
		}

		if ((uiNumBases != 0) || !bNameEmpty)
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


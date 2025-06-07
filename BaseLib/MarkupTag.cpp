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

	mszName.Kill();
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
char* CMarkupTag::GetAttribute(char* szAttribute)
{
	return mcAttributes.Get(szAttribute);
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
bool CMarkupTag::AddStringAttribute(char* szAttribute, char* szValue)
{
	if (mcAttributes.Get(szAttribute))
	{
		return false;
	}
	mcAttributes.Put(szAttribute, szValue);
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
void CMarkupTag::Print(CChars* psz)
{
	Print(psz, 0, 0);
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
	char*			szValue;
	SMapIterator	sIter;
	bool			bResult;
	size			uiNumElements;

	uiNumElements = macBases.NumElements();

	miLine = iLine;
	miColumn = iDepth*2;
	psz->Append(' ', iDepth*2);
	psz->Append('<');
	psz->Append(mszName);

	mcAttributes.FinaliseSorted();
	bResult = mcAttributes.StartIteration(&sIter, (void**)&szKey, NULL, (void**)&szValue, NULL);
	while (bResult)
	{
		psz->Append(' ');
		psz->Append(szKey);
		psz->Append('=');
		psz->Append('"');
		psz->Append(szValue);
		psz->Append('"');
		bResult = mcAttributes.Iterate(&sIter, (void**)&szKey, NULL, (void**)&szValue, NULL);
	}

	if (macBases.NumElements() == 0)
	{
		psz->Append('/');
	}
	psz->Append('>');
	iLine++;
	psz->AppendNewLine();

	if (uiNumElements > 0)
	{
		bPadClosing = true;
		if (ContainsOnlyText())
		{
			szText.Init();
			GetText(&szText);
			iLine = CMarkupBase::Print(psz, &szText, iDepth+1, iLine, true);
			szText.Kill();
		}
		else
		{
			for (i = 0; i < uiNumElements; i++)
			{
				pcBase = *macBases.Get(i);
				iLine = pcBase->Print(psz, iDepth + 1, iLine);
			}
		}

		if (miLine != iLine)
		{
			psz->Append(' ', iDepth*2);
		}
		psz->Append('<');
		psz->Append('/');
		psz->Append(mszName);
		psz->Append('>');
		iLine++;
		psz->AppendNewLine();
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


/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
	macBases.Init(16);
	mcAttributes.Init(1);
	mszName.Init();
	miLine = -1;
	miColumn = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupTag::Init(char* szName, CMarkupTag* pcParent)
{
	CMarkupBase::Init(MUT_Tag, pcParent);
	macBases.Init(16);
	mcAttributes.Init(1);
	mszName.Init(szName);
	miLine = -1;
	miColumn = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupTag::Kill(void)
{
	CMarkupBase*	pcBase;
	int				i;

	mszName.Kill();
	mcAttributes.Kill();

	for (i = 0; i < macBases.NumElements(); i++)
	{
		pcBase = *(macBases.Get(i));
		if (pcBase->IsTag())
		{
			((CMarkupTag*)pcBase)->Kill();
		}
		else if (pcBase->IsText())
		{
			((CMarkupText*)pcBase)->Kill();
		}
	}
	macBases.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMarkupTag::IsEmpty(void)
{
	return macBases.NumElements() == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CMarkupTag::GetAttribute(char* szAttribute)
{
	return mcAttributes.GetWithKey(szAttribute);
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

	psIter->iIndex = -1;
	psIter->mpcCurrent = NULL;
	psIter->bNamed = TRUE;
	return GetTagFromIndex(szTagName, psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupTag* CMarkupTag::GetTag(char* szTagName, int iTagNumber)
{
	STagIterator	sIter;
	CMarkupTag*		pcTag;
	int				i;

	sIter.iIndex = -1;
	sIter.mpcCurrent = NULL;
	sIter.bNamed = TRUE;
	
	for (i = 0; i <= iTagNumber; i++)
	{
		pcTag = GetTagFromIndex(szTagName, &sIter);
		if (!pcTag)
		{
			return NULL;
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
	int				i;
	int				iIndex;

	iIndex = psIter->iIndex +1;
	for (i = iIndex; i < macBases.NumElements(); i++)
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
	int				i;
	int				iIndex;

	iIndex = psIter->iIndex +1;
	for (i = iIndex; i < macBases.NumElements(); i++)
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

	psIter->iIndex = -1;
	psIter->mpcCurrent = NULL;
	psIter->bNamed = FALSE;
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
BOOL CMarkupTag::Is(char* szName)
{
	return mszName.Equals(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMarkupTag::GetText(CChars* psz, BOOL bFirstContiguous, BOOL bFirstTag)
{
	CMarkupBase*	pcBase;
	CMarkupText*	pcText;
	CMarkupRefText*	pcRefText;
	int				i;
	BOOL			bContiguous;
	BOOL			bPreviousTag;

	bContiguous = TRUE;
	bPreviousTag = FALSE;
	for (i = 0; i < macBases.NumElements(); i++)
	{
		pcBase = *(macBases.Get(i));
		if (pcBase->IsText())
		{
			if (bPreviousTag)
			{
				bContiguous = FALSE;
				if (bFirstContiguous)
				{
					break;
				}
			}
			pcText = (CMarkupText*)pcBase;
			psz->Append(pcText->mszText);
			bPreviousTag = FALSE;

			if (bFirstTag)
			{
				break;
			}
		}
		else if (pcBase->IsRefText())
		{
			if (bPreviousTag)
			{
				bContiguous = FALSE;
				if (bFirstContiguous)
				{
					break;
				}
			}
			pcRefText = (CMarkupRefText*)pcBase;
			psz->Append(pcRefText->mpcRef->mszText);
			bPreviousTag = FALSE;

			if (bFirstTag)
			{
				break;
			}
		}
		else if (pcBase->IsTag())
		{
			bPreviousTag = TRUE;
		}
		else if (pcBase->IsRefDoc())
		{
			bPreviousTag = TRUE;
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
BOOL CMarkupTag::AddAttribute(char* szAttribute, char* szValue)
{
	if (mcAttributes.GetWithKey(szAttribute))
	{
		return FALSE;
	}
	mcAttributes.Put(szAttribute, szValue);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMarkupTag::ContainsOnlyText(void)
{
	int				i;
	CMarkupBase*	pcBase;

	for (i = 0; i < macBases.NumElements(); i++)
	{
		pcBase = *macBases.Get(i);
		if ((!pcBase->IsText()) && (!pcBase->IsRefText()))
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMarkupTag::Swap(CMarkupBase* pcNew, CMarkupBase* pcOld)
{
	int				i;
	CMarkupBase*	pcBase;

	for (i = 0; i < macBases.NumElements(); i++)
	{
		pcBase = *macBases.Get(i);
		if (pcBase == pcOld)
		{
			macBases.Set(i, &pcNew);
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupTag::ToString(CChars* psz)
{
	ToString(psz, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMarkupTag::ToString(CChars* psz, int iDepth, int iLine)
{
	int				i;
	CChars*			pszAttribute;
	CChars*			pszValue;
	CMarkupBase*	pcBase;
	CChars			szText;
	BOOL			bPadClosing;

	miLine = iLine;
	miColumn = iDepth*2;
	psz->Append(' ', iDepth*2);
	psz->Append('<');
	psz->Append(mszName);

	for (i = 0; i < mcAttributes.NumElements(); i++)
	{
		psz->Append(' ');
		mcAttributes.GetAtIndex(i, &pszAttribute, &pszValue);
		psz->Append(pszAttribute);
		psz->Append('=');
		psz->Append('"');
		psz->Append(pszValue);
		psz->Append('"');
	}

	if (macBases.NumElements() == 0)
	{
		psz->Append('/');
	}
	psz->Append('>');
	iLine++;
	psz->AppendNewLine();

	if (macBases.NumElements() > 0)
	{
		bPadClosing = TRUE;
		if (ContainsOnlyText())
		{
			szText.Init();
			GetText(&szText);
			iLine = CMarkupBase::ToString(psz, &szText, iDepth+1, iLine, TRUE);
			szText.Kill();
		}
		else
		{
			for (i = 0; i < macBases.NumElements(); i++)
			{
				pcBase = *macBases.Get(i);
				if (pcBase->IsTag())
				{
					iLine = ((CMarkupTag*)pcBase)->ToString(psz, iDepth+1, iLine);
				}
				else if (pcBase->IsText())
				{
					iLine = ((CMarkupText*)pcBase)->ToString(psz, iDepth+1, iLine);
				}
				else if (pcBase->IsNamedRef())
				{
					iLine = ((CMarkupNamedRef*)pcBase)->ToString(psz, iDepth+1, iLine);
				}
				else if (pcBase->IsRefDoc())
				{
					iLine = ((CMarkupRefDoc*)pcBase)->ToString(psz, iDepth+1, iLine);
				}
				else if (pcBase->IsRefText())
				{
					iLine = ((CMarkupRefText*)pcBase)->ToString(psz, iDepth+1, iLine);
				}
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
	ToString(&sz);
	sz.Dump();
	sz.Kill();
}


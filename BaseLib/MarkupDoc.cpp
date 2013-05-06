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
#include "MarkupDoc.h"
#include "Markup.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupDoc::Init(CMarkup* pcMarkup)
{
	mpcRootTag = NULL;
	macRefs.Init(8);
	mpcMarkup = pcMarkup;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupDoc::Kill(void)
{
	if (mpcRootTag)
	{
		mpcRootTag->Kill();
	}

	macRefs.Kill();
	mpcMarkup = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupTag* CMarkupDoc::GetRootTag(void)
{
	return mpcRootTag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupTag* CMarkupDoc::SetRootTag(char* szTagName)
{
	CMarkupTag*	pcTag;

	if (mpcRootTag == NULL)
	{
		pcTag = mpcMarkup->AllocateTag(this);
		pcTag->Init(szTagName, NULL);
		mpcRootTag = pcTag;
		return pcTag;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMarkupDoc::ToString(CChars* psz)
{
	return ToString(psz, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMarkupDoc::ToString(CChars* psz, int iDepth, int iLine)
{
	if (!mpcRootTag)
	{
		psz->Append(' ', (iDepth+1)*2);
		psz->Append("No root element!\n");
		iLine++;
	}
	else
	{
		iLine = mpcRootTag->ToString(psz, iDepth, iLine);
	}
	return iLine;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMarkupDoc::Is(char* szName)
{
	if (mpcRootTag)
	{
		if (mpcRootTag->Is(szName))
		{
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMarkupDoc::FixReferences(void)
{
	CMarkupReference*	pcRef;
	int					i;
	CMarkupNamedRef*	pcNamedRef;
	CMarkupSubstitute*	pcSubstitute;
	CMarkupRefDoc*		pcMarkupRefDoc;
	CMarkupRefText*		pcMarkupRefText;
	CMarkupReference*	pcReference;

	for (i = 0; i < macRefs.NumElements(); i++)
	{
		pcRef = (*(macRefs.Get(i)));
		if (pcRef->IsNamedRef())
		{
			pcNamedRef = (CMarkupNamedRef*)pcRef;
			pcSubstitute = mpcMarkup->GetSubstitute(pcNamedRef->mszName.Text());
			if (!pcSubstitute)
			{
				return FALSE;
			}

			pcReference = NULL;
			if (pcSubstitute->IsText())
			{
				pcMarkupRefText = mpcMarkup->AllocateRefText(this);
				pcMarkupRefText->Init((CMarkupSubText*)pcSubstitute, pcNamedRef->mpcParent);
				pcReference = pcMarkupRefText;
			}
			else if (pcSubstitute->IsDoc())
			{
				pcMarkupRefDoc = mpcMarkup->AllocateRefDoc(this);
				pcMarkupRefDoc->Init((CMarkupSubDoc*)pcSubstitute, pcNamedRef->mpcParent);
				pcReference = pcMarkupRefDoc;
			}

			if (pcReference)
			{
				pcNamedRef->mpcParent->Swap(pcReference, pcNamedRef);
				pcNamedRef->Kill();
				mpcMarkup->Deallocated(pcNamedRef);
			}
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupDoc::AddReference(CMarkupReference* pcRef)
{
	macRefs.Add(&pcRef);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupDoc::Dump(void)
{
	CChars	sz;

	sz.Init();
	ToString(&sz);
	sz.Dump();
	sz.Kill();
}


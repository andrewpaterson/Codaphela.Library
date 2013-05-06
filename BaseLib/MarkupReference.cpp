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
#include "MarkupReference.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupRefText::Init(CMarkupSubText* pcRef, CMarkupTag* pcParent)
{
	CMarkupBase::Init(MUT_RefText, pcParent);
	mpcRef = pcRef;
	mpcRef->AddRef();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupRefText::Kill(void)
{
	mpcRef->RemoveAndKill();
	mpcRef = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMarkupRefText::ToString(CChars* psz, int iDepth, int iLine)
{
	CChars	szText;

	szText.Init(mpcRef->mszText);
	iLine = CMarkupBase::ToString(psz, &szText, iDepth, iLine, FALSE);
	szText.Kill();
	return iLine;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupRefDoc::Init(CMarkupSubDoc* pcRef, CMarkupTag* pcParent)
{
	CMarkupBase::Init(MUT_RefDoc, pcParent);
	mpcRef = pcRef;
	mpcRef->AddRef();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupRefDoc::Kill(void)
{
	mpcRef->RemoveAndKill();
	mpcRef = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMarkupRefDoc::ToString(CChars* psz, int iDepth, int iLine)
{
	iLine = mpcRef->mpcDoc->ToString(psz, iDepth, iLine);
	return iLine;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMarkupRefDoc::Is(char* szName)
{
	return mpcRef->mpcDoc->Is(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMarkupTag* CMarkupRefDoc::GetRootTag(void)
{
	return mpcRef->mpcDoc->GetRootTag();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupNamedRef::Init(char* szName, CMarkupTag* pcParent)
{
	CMarkupBase::Init(MUT_NamedRef, pcParent);
	mszName.Init(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupNamedRef::Kill(void)
{
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMarkupNamedRef::ToString(CChars* psz, int iDepth, int iLine)
{
	miLine = iLine;
	miColumn = iDepth*2;
	psz->Append(' ', miColumn);
	psz->Append('&');
	psz->Append(&mszName);
	iLine++;
	psz->AppendNewLine();
	return iLine;
}

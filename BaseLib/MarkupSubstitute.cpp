/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "MarkupSubstitute.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupSubstitute::Init(char* szName, EMarkupSubstituteType eType)
{
	mszName.Init(szName);
	meType = eType;
	miReferences = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupSubstitute::Kill(void)
{
	mszName.Kill();
	miReferences = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupSubstitute::AddRef(void)
{
	miReferences++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupSubstitute::RemoveRef(void)
{
	if (miReferences == 0)
	{
		return true;
	}

	miReferences--;
	if (miReferences == 0)
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupSubstitute::RemoveAndKill(void)
{
	bool	bMustKill;

	bMustKill = RemoveRef();
	if (bMustKill)
	{
		Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupSubstitute::Is(char* szName)
{
	return mszName.Equals(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupSubstitute::IsText(void)
{
	return meType == MST_Text;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMarkupSubstitute::IsDoc(void)
{
	return meType == MST_Doc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupSubText::Init(char* szName, char* szText)
{
	CMarkupSubstitute::Init(szName, MST_Text);
	mszText.Init(szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupSubText::Kill(void)
{
	CMarkupSubstitute::Kill();
	mszText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupSubDoc::Init(char* szName, CMarkupDoc* pcDoc)
{
	CMarkupSubstitute::Init(szName, MST_Doc);
	mpcDoc = pcDoc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupSubDoc::Kill(void)
{
	CMarkupSubstitute::Kill();
	mpcDoc->Kill();
}


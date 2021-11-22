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
#include "MarkupBase.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupBase::Init(EMarkupType eType, CMarkupTag* pcParent)
{
	meType = eType;
	mpcParent = pcParent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMarkupBase::IsTag(void)
{
	return meType == MUT_Tag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMarkupBase::IsText(void)
{
	return meType == MUT_Text;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMarkupBase::IsRefDoc(void)
{
	return meType == MUT_RefDoc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMarkupBase::IsRefText(void)
{
	return meType == MUT_RefText;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMarkupBase::IsNamedRef(void)
{
	return meType == MUT_NamedRef;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMarkupBase::GetLine(void)
{
	return miLine;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMarkupBase::GetColumn(void)
{
	return miColumn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupBase::SetLineAndColumn(int iLine, int iColumn)
{
	miLine = iLine;
	miColumn = iColumn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMarkupBase::Print(CChars* pszDest, CChars* pszMutableSource, int iDepth, int iLine, BOOL bAllowSameLine)
{
	int				iNewLines;
	CChars			szPadding;

	iNewLines = pszMutableSource->CountNewLines();
	if ((iNewLines == 0) && (bAllowSameLine))
	{
		iLine--;
		pszDest->RemoveLastCharacter();
		ReplaceIllegalChars(pszMutableSource);
		pszDest->Append(pszMutableSource->Text());
	}
	else
	{
		szPadding.Init();
		szPadding.Append(' ', iDepth*2);
		pszMutableSource->PassifyNewlines();
		pszMutableSource->StripWhiteSpace(TRUE);
		iNewLines = pszMutableSource->CountNewLines();
		pszMutableSource->Insert(0, &szPadding);
		szPadding.Insert(0, '\n');
		pszMutableSource->Replace("\n", szPadding.Text());
		szPadding.Kill();
		ReplaceIllegalChars(pszMutableSource);
		iLine += iNewLines;
		iLine++;
		pszMutableSource->AppendNewLine();
		pszDest->Append(pszMutableSource->Text());
	}
	return iLine;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarkupBase::ReplaceIllegalChars(CChars* pszMutableSource)
{
	pszMutableSource->Replace("&", "&amp;");
	pszMutableSource->Replace("<", "&gt;");
	pszMutableSource->Replace(">", "&lt;");
	pszMutableSource->Replace("\'", "&apos;");
	pszMutableSource->Replace("\"", "&quot;");
}


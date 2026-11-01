/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Numbers.h"
#include "Punctuators.h"
#include "PreprocessedTokens.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SPreprocessedToken::Init(EPreprocessedTokenKind eKind, const char* szText, size uiLength, CSourceFile* pcFile, int iLine, int iColumn)
{
	this->szText = szText;
	this->uiLength = (uint32)uiLength;
	uiKind = (uint8)eKind;
	uiFlags = 0;
	iParameter = -1;
	this->pcFile = pcFile;
	this->iLine = iLine;
	this->iColumn = iColumn;
	iExpansion = -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SPreprocessedToken::Is(char* szText)
{
	size	uiLength;

	uiLength = strlen(szText);
	if (uiLength != this->uiLength)
	{
		return false;
	}
	return memcmp(this->szText, szText, uiLength) == 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SPreprocessedToken::IsPunctuator(char* szText)
{
	if (uiKind != PTK_Punctuator)
	{
		return false;
	}
	return Is(szText);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SPreprocessedToken::IsIdentifier(void)
{
	return uiKind == PTK_Identifier;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SPreprocessedToken::HasLeadingSpace(void)
{
	return (uiFlags & PTF_LEADING_SPACE) != 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SPreprocessedToken::IsStartOfLine(void)
{
	return (uiFlags & PTF_START_OF_LINE) != 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SPreprocessedToken::Append(CChars* psz)
{
	psz->Append(szText, uiLength);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessedTokens::Init(void)
{
	macTokens.Init();
	macExpansions.Init();
	macInactiveRegions.Init();
	macDirectiveLines.Init();
	mcStrings.Init(4 KB);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessedTokens::Kill(void)
{
	mcStrings.Kill();
	macDirectiveLines.Kill();
	macInactiveRegions.Kill();
	macExpansions.Kill();
	macTokens.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessedTokens::Clear(void)
{
	Kill();
	Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SPreprocessedToken* CPreprocessedTokens::Add(SPreprocessedToken* psToken)
{
	return macTokens.Add(psToken);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CPreprocessedTokens::NumTokens(void)
{
	return macTokens.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SPreprocessedToken* CPreprocessedTokens::Get(size iIndex)
{
	return macTokens.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CPreprocessedTokens::AddExpansion(const char* szName, size uiNameLength, CSourceFile* pcFile, int iLine, int iColumn, int iParent)
{
	SMacroExpansion*	psExpansion;
	size				iIndex;

	psExpansion = macExpansions.AddGetIndex(&iIndex);
	psExpansion->szName = szName;
	psExpansion->uiNameLength = (uint32)uiNameLength;
	psExpansion->pcFile = pcFile;
	psExpansion->iLine = iLine;
	psExpansion->iColumn = iColumn;
	psExpansion->iParent = iParent;
	return (int)iIndex;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CPreprocessedTokens::NumExpansions(void)
{
	return macExpansions.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SMacroExpansion* CPreprocessedTokens::GetExpansion(size iIndex)
{
	return macExpansions.SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessedTokens::AddInactiveLine(CSourceFile* pcFile, int iLine)
{
	SInactiveRegion*	psRegion;

	//Consecutive skipped lines join into one region.
	psRegion = macInactiveRegions.Tail();
	if (psRegion && (psRegion->pcFile == pcFile) && (iLine >= psRegion->iStartLine) && (iLine <= psRegion->iEndLine + 1))
	{
		if (iLine > psRegion->iEndLine)
		{
			psRegion->iEndLine = iLine;
		}
		return;
	}

	psRegion = macInactiveRegions.Add();
	psRegion->pcFile = pcFile;
	psRegion->iStartLine = iLine;
	psRegion->iEndLine = iLine;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CPreprocessedTokens::NumInactiveRegions(void)
{
	return macInactiveRegions.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SInactiveRegion* CPreprocessedTokens::GetInactiveRegion(size iIndex)
{
	return macInactiveRegions.SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessedTokens::AddDirectiveLine(CSourceFile* pcFile, int iLine, bool bActive)
{
	SDirectiveLine*	psLine;

	psLine = macDirectiveLines.Add();
	psLine->pcFile = pcFile;
	psLine->iLine = iLine;
	psLine->bActive = bActive;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CPreprocessedTokens::NumDirectiveLines(void)
{
	return macDirectiveLines.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDirectiveLine* CPreprocessedTokens::GetDirectiveLine(size iIndex)
{
	return macDirectiveLines.SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CPreprocessedTokens::AddString(const char* szText, size uiLength)
{
	char*	sz;

	sz = (char*)mcStrings.Add(uiLength + 1);
	memcpy(sz, szText, uiLength);
	sz[uiLength] = '\0';
	return sz;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CPreprocessedTokens::Print(CChars* psz)
{
	size				i;
	SPreprocessedToken*	psToken;
	SPreprocessedToken*	psPrevious;

	psPrevious = NULL;
	for (i = 0; i < macTokens.NumElements(); i++)
	{
		psToken = macTokens.Get(i);
		if (psPrevious)
		{
			if (psToken->IsStartOfLine())
			{
				psz->AppendNewLine();
			}
			else if (psToken->HasLeadingSpace() || PreprocessedTokensNeedSpace(psPrevious, psToken))
			{
				psz->Append(' ');
			}
		}
		psToken->Append(psz);
		psPrevious = psToken;
	}

	if (psPrevious)
	{
		psz->AppendNewLine();
	}
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessedTokens::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool PreprocessedTokensNeedSpace(SPreprocessedToken* psLeft, SPreprocessedToken* psRight)
{
	char	c;
	char	cLast;
	char	sz[8];

	//Printing two tokens next to each other must not make different tokens when the output is read back.
	c = psRight->szText[0];
	cLast = psLeft->szText[psLeft->uiLength - 1];
	if ((psLeft->uiKind == PTK_Identifier) || (psLeft->uiKind == PTK_Number))
	{
		if ((psRight->uiKind == PTK_Identifier) || (psRight->uiKind == PTK_Number) || (psRight->uiKind == PTK_Character) || (psRight->uiKind == PTK_String))
		{
			return true;
		}
		if (psLeft->uiKind == PTK_Number)
		{
			if (c == '.')
			{
				return true;
			}
			if (((c == '+') || (c == '-')) && ((cLast == 'e') || (cLast == 'E') || (cLast == 'p') || (cLast == 'P')))
			{
				return true;
			}
		}
		return false;
	}

	if (psLeft->uiKind == PTK_Punctuator)
	{
		if ((psRight->uiKind == PTK_Number) && (cLast == '.'))
		{
			return true;
		}

		if ((psRight->uiKind == PTK_Punctuator) || (psRight->uiKind == PTK_Other))
		{
			if (psLeft->uiLength >= 4)
			{
				return false;
			}
			memcpy(sz, psLeft->szText, psLeft->uiLength);
			sz[psLeft->uiLength] = c;
			if ((cLast == '/') && ((c == '/') || (c == '*')))
			{
				return true;
			}
			return IsPunctuatorPrefix(sz, psLeft->uiLength + 1);
		}
	}
	return false;
}


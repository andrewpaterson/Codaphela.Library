/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Numbers.h"
#include "CPPErrors.h"
#include "Punctuators.h"
#include "SourceFile.h"
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
	mpsMallocators = NULL;
	InitArrays();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessedTokens::Kill(void)
{
	KillArrays();
	mpsMallocators = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Empty, with the same allocators.
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessedTokens::Clear(void)
{
	KillArrays();
	InitArrays();
}


//////////////////////////////////////////////////////////////////////////
//
// Empties the tokens, which are then allocated by psMallocators' (NULL for
// the system allocator).  The tokens' memory goes back to the allocators
// they were made by.
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessedTokens::SetMallocators(SPreprocessedTokensMallocators* psMallocators)
{
	KillArrays();
	mpsMallocators = psMallocators;
	InitArrays();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessedTokens::InitArrays(void)
{
	if (mpsMallocators)
	{
		macTokens.Init(mpsMallocators->pcTokens);
		macExpansions.Init(mpsMallocators->pcExpansions);
		macInactiveRegions.Init(mpsMallocators->pcInactiveRegions);
		macDirectiveLines.Init(mpsMallocators->pcDirectiveLines);
	}
	else
	{
		macTokens.Init();
		macExpansions.Init();
		macInactiveRegions.Init();
		macDirectiveLines.Init();
	}
	macLineDirectives.Init();
	mcStrings.Init(4 KB);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessedTokens::KillArrays(void)
{
	mcStrings.Kill();
	macLineDirectives.Kill();
	macDirectiveLines.Kill();
	macInactiveRegions.Kill();
	macExpansions.Kill();
	macTokens.Kill();
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
//
// As AddInactiveLine for each line from iStartLine to iEndLine.
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessedTokens::AddInactiveLines(CSourceFile* pcFile, int iStartLine, int iEndLine)
{
	SInactiveRegion*	psRegion;

	AddInactiveLine(pcFile, iStartLine);
	psRegion = macInactiveRegions.Tail();
	if (iEndLine > psRegion->iEndLine)
	{
		psRegion->iEndLine = iEndLine;
	}
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


//////////////////////////////////////////////////////////////////////////
//
// A #line whose next line is iLine: errors from there on are reported at
// their line plus iOffset, and in the file szQuotedName names as it was
// written (quotes and all; empty for the file's own).
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessedTokens::AddLineDirective(CSourceFile* pcFile, int iLine, int iOffset, const char* szQuotedName)
{
	SLineDirective*	psDirective;
	size			uiLength;
	char*			szName;

	psDirective = macLineDirectives.Add();
	psDirective->pcFile = pcFile;
	psDirective->iLine = iLine;
	psDirective->iOffset = iOffset;
	psDirective->szName = NULL;
	uiLength = strlen(szQuotedName);
	if (uiLength >= 2)
	{
		szName = AddString(&szQuotedName[1], uiLength - 2);
		psDirective->szName = szName;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Where #line says line *piLine of pcFile is: the last #line above it in
// the file (a file included again has its directives after the last time's)
// changes *piLine and, if it named one, *pszFile.  Otherwise they are left.
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessedTokens::ReportedPosition(CSourceFile* pcFile, const char** pszFile, int* piLine)
{
	SLineDirective*	psDirective;
	size			ui;

	for (ui = macLineDirectives.NumElements(); ui > 0; ui--)
	{
		psDirective = macLineDirectives.Get(ui - 1);
		if ((psDirective->pcFile == pcFile) && (psDirective->iLine <= *piLine))
		{
			*piLine += psDirective->iOffset;
			if (psDirective->szName)
			{
				*pszFile = psDirective->szName;
			}
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An error at psToken, where #line says that is.  iToken is what the error
// remembers as its token.
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessedTokens::AddError(CCPPErrors* pcErrors, SPreprocessedToken* psToken, int iToken, const char* szMessage, const char* szDetail)
{
	const char*	szFile;
	int			iLine;

	if (!psToken->pcFile)
	{
		pcErrors->Add(NULL, psToken->iLine, psToken->iColumn, iToken, szMessage, szDetail);
		return;
	}
	szFile = psToken->pcFile->ShortName();
	iLine = psToken->iLine;
	ReportedPosition(psToken->pcFile, &szFile, &iLine);
	pcErrors->Add(szFile, iLine, psToken->iColumn, iToken, szMessage, szDetail);
}

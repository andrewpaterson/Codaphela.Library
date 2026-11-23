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
#include <string.h>
#include "CPP65DebugWriter.h"


//////////////////////////////////////////////////////////////////////////
//
// szText as an assembler string: in quotes, with a backslash or a quote
// in it escaped ("C:\\Path\\File.cpp").
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugWriter::AppendString(CChars* pszDest, const char* szText)
{
	pszDest->Append('"');
	for (; *szText != '\0'; szText++)
	{
		if ((*szText == '\\') || (*szText == '"'))
		{
			pszDest->Append('\\');
		}
		pszDest->Append(*szText);
	}
	pszDest->Append('"');
}


//////////////////////////////////////////////////////////////////////////
//
// .dfunction "szName" after the function's label in its code: it starts
// there.  False if the code has no such label.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWriter::DescribeFunction(CChars* pszCode, const char* szLabel, const char* szName)
{
	CChars	szDirective;
	size	uiAfter;

	uiAfter = FindAfterLabel(pszCode->Text(), szLabel);
	if (uiAfter == MAX_SIZE)
	{
		return false;
	}
	szDirective.Init("\t\t.dfunction ");
	AppendString(&szDirective, szName);
	szDirective.AppendNewLine();
	pszCode->Insert(uiAfter, &szDirective);
	szDirective.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the line after szLabel's in szCode starts (the label at the start
// of a line, then a colon), MAX_SIZE if it has none.
//
//////////////////////////////////////////////////////////////////////////
size CCPP65DebugWriter::FindAfterLabel(const char* szCode, const char* szLabel)
{
	const char*	szAt;
	size		uiLength;

	uiLength = strlen(szLabel);
	for (szAt = strstr(szCode, szLabel); szAt; szAt = strstr(szAt + 1, szLabel))
	{
		if (((szAt == szCode) || (szAt[-1] == '\n')) && (szAt[uiLength] == ':') && (szAt[uiLength + 1] == '\n'))
		{
			return (size)(szAt - szCode) + uiLength + 2;
		}
	}
	return MAX_SIZE;
}

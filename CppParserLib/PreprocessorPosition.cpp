/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2021 Andrew Paterson

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
#include "BaseLib/Chars.h"
#include "SourceFile.h"
#include "PreprocessorTokenParser.h"
#include "PreprocessorPosition.h"



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SPreprocessorPosition::Init(size iLine, size iColumn, char* szShortName)
{
	miLine = iLine;
	miColumn = iColumn;
	mszShortName = szShortName;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SPreprocessorPosition::Init(CPreprocessorTokenParser* pcParser, char* szShortName)
{
	if (pcParser)
	{
		Init(pcParser->Line(), pcParser->Column(), szShortName);
	}
	else
	{
		Init(-1, -1, szShortName);
	}
	
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* SPreprocessorPosition::Message(CChars* pszUninitialised)
{
	pszUninitialised->Init();
	pszUninitialised->Append(mszShortName);
	pszUninitialised->Append(" (");
	pszUninitialised->Append(miLine + 1);
	pszUninitialised->Append(", ");
	pszUninitialised->Append(miColumn + 1);
	pszUninitialised->Append("): ");
	return pszUninitialised;
}


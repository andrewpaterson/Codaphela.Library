/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LanguageLib

Codaphela LanguageLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LanguageLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LanguageLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdio.h>
#include "BaseLib/DebugOutput.h"
#include "BaseLib/Define.h"
#include "BaseLib/Chars.h"
#include "TokenDumper.h"
#include "ElementTypes.h"
#include "Tokens.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DumpTokens(int* piTokenArray, int iLength, CEnumeratorVoid* pcKeywords, CEnumeratorVoid* pcIdentifiers)
{
	int*	pi;
	int		i;

	if ((piTokenArray == NULL) || (iLength == 0))
	{
		return;
	}

	i = 0;
	pi = piTokenArray;
	for (;;)
	{
		if (((i >= iLength) && (iLength != -1)) || (pi == NULL))
		{
			EngineOutput("End");
			break;
		}

		switch(*pi)
		{
		case ET_Error:
		case ET_Unknown: 
		case ET_Node:
			DumpTokenSingle(pi);
			break;

		case ET_Identifier:
			DumpTokenIdentifier(pi, pcIdentifiers);
			break;

		case ET_Text:
			DumpTokenKeywordText(pi, pcKeywords);
			break;

		case ET_Char:
			DumpTokenKeywordChar(pi);
			break;

		case PT_IntegerType:
			DumpTokenInteger(pi);
			break;

		case PT_RealType:
			DumpTokenReal(pi);
			break;

		case PT_StringType:
			DumpTokenUnknownText(pi);
			break;

		case PT_CharType:
			DumpTokenUnknownChar(pi);
			break;
		}
		EngineOutput(" ");
		i = i + GetTokenSize(pi);
		pi = GetNextToken(pi);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DumpTokenSingle(void* pi)
{
	STokenSingle*	ps;

	ps = (STokenSingle*)pi;
	
	switch(ps->eElementType)
	{
	case ET_Error:
		EngineOutput("Error!");
		break;
	case ET_Unknown: 
		EngineOutput("Unknown");
		break;
	case ET_Node:
		EngineOutput("Node");
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DumpTokenIdentifier(void* pi, CEnumeratorVoid* pcIdentifiers)
{
	STokenIdentifier*	ps;
	char*				szName;

	ps = (STokenIdentifier*)pi;
	if (pcIdentifiers)
	{
		pcIdentifiers->GetWithID(ps->iID, NULL, &szName);
		if (szName != NULL)
		{
			EngineOutput("@");
			EngineOutput(szName);
			return;
		}
	}
	EngineOutput("$Identifier");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DumpTokenKeywordText(void* pi, CEnumeratorVoid* pcKeywords)
{
	STokenKeywordText*	ps;
	char*				szName;

	ps = (STokenKeywordText*)pi;
	if (pcKeywords)
	{
		pcKeywords->GetWithID(ps->iID, NULL, &szName);
		if (szName != NULL)
		{
			EngineOutput(szName);
			return;
		}
	}
	EngineOutput("$Keyword");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DumpTokenKeywordChar(void* pi)
{
	STokenKeywordChar*	ps;
	char				sz[2];

	ps = (STokenKeywordChar*)pi;
	sz[0] = ps->cValue;
	sz[1] = 0;
	EngineOutput(sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DumpTokenInteger(void* pi)
{
	STokenInteger*	ps;
	CChars			sz;

	ps = (STokenInteger*)pi;
	sz.Init();
	sz.Append(ps->iValue);
	EngineOutput(sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DumpTokenReal(void* pi)
{
	STokenReal*		ps;

	CChars			sz;

	ps = (STokenReal*)pi;
	sz.Init();
	sz.Append(ps->fValue);
	EngineOutput(sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DumpTokenUnknownText(void* pi)
{
	STokenUnknownText*	ps;

	ps = (STokenUnknownText*)pi;
	EngineOutput("\"");
	EngineOutput(ps->szStringCharacters);
	EngineOutput("\"");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DumpTokenUnknownChar(void* pi)
{
	STokenUnknownChar*	ps;
	char				sz[4];

	ps = (STokenUnknownChar*)pi;
	sz[0] = '\'';
	sz[1] = ps->cValue;
	sz[2] = '\'';
	sz[3] = 0;
	EngineOutput(sz);
}


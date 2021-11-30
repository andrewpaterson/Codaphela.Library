/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "PPDirective.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPDirective::Init(int iLine, int iColumn)
{
	//Ignored.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPDirective::Init(EPreprocessorDirective eType, int iLine, int iColumn)
{
	CPPLine::Init(iLine, iColumn);
	Set(eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPDirective::IsDirective(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPDirective::Set(EPreprocessorDirective eType)
{
	meType = eType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPPDirective::Print(CChars* psz)
{
	psz->Append('#');

	switch (meType)
	{
	case PPD_define:
		psz->Append("define");
		break;
	case PPD_ifdef:
		psz->Append("ifdef");
		break;
	case PPD_include:
		psz->Append("include");
		break;
	case PPD_ifndef:
		psz->Append("ifndef");
		break;
	case PPD_endif:
		psz->Append("endif");
		break;
	case PPD_else:
		psz->Append("else");
		break;
	case PPD_if:
		psz->Append("if");
		break;
	case PPD_elif:
		psz->Append("elif");
		break;
	case PPD_undef:
		psz->Append("undef");
		break;
	case PPD_error:
		psz->Append("error");
		break;
	case PPD_pragma:
		psz->Append("pragma");
		break;
	}
	if (mcTokens.mcArray.IsNotEmpty())
	{
		psz->Append(' ');
		CPPLine::Print(psz);
	}
	else
	{
		psz->AppendNewLine();
	}

	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPDirective::Copy(CPPToken* pcSource, CMemoryStackExtended* pcStack)
{
	CPPDirective*	pcCast;

	if (pcSource->IsDirective())
	{
		pcCast = (CPPDirective*)pcSource;
		CPPLine::Copy(pcSource, pcStack);
		meType = pcCast->meType;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPDirective::Is(EPreprocessorDirective eType)
{
	return meType == eType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPDirective::IsConditional(void)
{
	return meType <= PPD_elif;
}


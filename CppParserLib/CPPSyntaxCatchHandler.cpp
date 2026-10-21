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
#include "CPPSyntaxes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxCatchHandler::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntax::Init(pcTree, pcParent);
	mpcParameter = NULL;
	mpcBlock = NULL;
	mpcScope = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxCatchHandler::Kill(void)
{
	CCPPSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPSyntaxCatchHandler::GetType(void)
{
	return "CatchHandler";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPSyntax CCPPSyntaxCatchHandler::GetKind(void)
{
	return CPPS_CatchHandler;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxCatchHandler::PrettyPrint(CChars* pszDest, int iDepth)
{
	Indent(pszDest, iDepth);
	pszDest->Append("catch (");
	if (mpcParameter)
	{
		mpcParameter->PrettyPrint(pszDest, iDepth);
	}
	else
	{
		pszDest->Append("...");
	}
	pszDest->Append(")\n");
	PrintNode(pszDest, mpcBlock, iDepth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxCatchHandler::NumChildren(void)
{
	return 1 + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPSyntaxCatchHandler::GetChild(size iIndex)
{
	if (iIndex == 0)
	{
		return mpcParameter;
	}
	iIndex--;

	if (iIndex == 0)
	{
		return mpcBlock;
	}
	iIndex--;

	return NULL;
}


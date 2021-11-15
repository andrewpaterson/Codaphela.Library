/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "AbstractSyntaxTree.h"
#include "ASTListDecl.h"
#include "BaseLib/PointerFunctions.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAbstractSyntaxTree::Init(void)
{
	mpcElement = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAbstractSyntaxTree::Kill(void)
{
	SafeKill(mpcElement);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CASTCommon* CAbstractSyntaxTree::Create(AbstractSyntaxTypes eType)
{
	CASTCommon*		pcCommon;

	pcCommon = NULL;
	switch (eType)
	{
	case EAST_ListDecl:
		pcCommon = CASTListDecl::Construct(malloc(sizeof(CASTListDecl)));
		break;
	}

	if (pcCommon)
	{
		pcCommon->Init();
	}
	return pcCommon;
}


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
#include "CPPSyntaxes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxTryStatement::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxStatement::Init(pcTree, pcParent);
	mpcBlock = NULL;
	macpHandlers.Init(NodeArrayMallocator());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxTryStatement::Kill(void)
{
	macpHandlers.Kill();
	CCPPSyntaxStatement::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxTryStatement::PrettyPrint(CChars* pszDest, int iDepth)
{
	Indent(pszDest, iDepth);
	pszDest->Append("try\n");
	PrintBlocks(pszDest, iDepth);
}


//////////////////////////////////////////////////////////////////////////
//
// The block and the handlers, which is all of it after try: a function's
// definition prints its member initializers between them.
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxTryStatement::PrintBlocks(CChars* pszDest, int iDepth)
{
	size	i;

	PrintNode(pszDest, mpcBlock, iDepth);
	for (i = 0; i < macpHandlers.NumElements(); i++)
	{
		macpHandlers.GetPtr(i)->PrettyPrint(pszDest, iDepth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether it is a function-try-block: a function's whole body, whose
// handlers also catch what the function's member initializers, and its
// bases' and members' constructors and destructors, throw.  [except.handle]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntaxTryStatement::IsFunctionBody(void)
{
	return mpcParent && (mpcParent->GetKind() == CPPS_FunctionDefinition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxTryStatement::VisitFields(CCPPSyntaxFields* pcFields)
{
	pcFields->Child(&mpcBlock);
	pcFields->Children(&macpHandlers);
}


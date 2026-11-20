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
void CCPPSyntaxClassSpecifier::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntaxTypeSpecifier::Init(pcTree, pcParent);
	meKey = CT_class;
	macpAttributes.Init(NodeArrayMallocator());
	mpcName = NULL;
	mbFinal = false;
	macpBases.Init(NodeArrayMallocator());
	macpMembers.Init(NodeArrayMallocator());
	mpcScope = NULL;
	mpcSymbol = NULL;
	miPack = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxClassSpecifier::Kill(void)
{
	macpAttributes.Kill();
	macpBases.Kill();
	macpMembers.Kill();
	CCPPSyntaxTypeSpecifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxClassSpecifier::PrettyPrint(CChars* pszDest, int iDepth)
{
	PrintKeyword(pszDest, meKey);
	if (macpAttributes.NumElements() > 0)
	{
		pszDest->Append(' ');
		PrintList(pszDest, &macpAttributes, " ", iDepth);
	}
	if (mpcName)
	{
		pszDest->Append(' ');
		mpcName->PrettyPrint(pszDest, iDepth);
	}
	if (mbFinal)
	{
		pszDest->Append(" final");
	}
	if (macpBases.NumElements() > 0)
	{
		pszDest->Append(" : ");
		PrintList(pszDest, &macpBases, ", ", iDepth);
	}
	pszDest->AppendNewLine();
	Indent(pszDest, iDepth);
	pszDest->Append("{\n");
	PrintDeclarations(pszDest, &macpMembers, iDepth + 1);
	Indent(pszDest, iDepth);
	pszDest->Append('}');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxClassSpecifier::VisitFields(CCPPSyntaxFields* pcFields)
{
	pcFields->Enum(&meKey);
	pcFields->Children(&macpAttributes);
	pcFields->Child(&mpcName);
	pcFields->Bool(&mbFinal);
	pcFields->Children(&macpBases);
	pcFields->Children(&macpMembers);
	pcFields->Scope(&mpcScope);
	pcFields->Symbol(&mpcSymbol);
	pcFields->Integer(&miPack);
}


//////////////////////////////////////////////////////////////////////////
//
// True if a member function of the class is declared override or final.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntaxClassSpecifier::HasVirtSpecifiedMember(void)
{
	CCPPSyntax*					pcMember;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxDeclarator*		pcDeclarator;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	size						i;
	size						j;

	for (i = 0; i < macpMembers.NumElements(); i++)
	{
		pcMember = macpMembers.GetPtr(i);
		if (pcMember->GetKind() == CPPS_FunctionDefinition)
		{
			pcDeclarator = ((CCPPSyntaxFunctionDefinition*)pcMember)->mpcDeclarator;
			pcSuffix = pcDeclarator ? pcDeclarator->GetFunctionSuffix() : NULL;
			if (pcSuffix && (pcSuffix->macpVirtSpecifiers.NumElements() > 0))
			{
				return true;
			}
		}
		else if (pcMember->GetKind() == CPPS_SimpleDeclaration)
		{
			pcSimple = (CCPPSyntaxSimpleDeclaration*)pcMember;
			for (j = 0; j < pcSimple->macpDeclarators.NumElements(); j++)
			{
				pcDeclarator = (pcSimple->macpDeclarators.GetPtr(j)->GetKind() == CPPS_InitDeclarator) ? ((CCPPSyntaxInitDeclarator*)pcSimple->macpDeclarators.GetPtr(j))->mpcDeclarator : NULL;
				pcSuffix = pcDeclarator ? pcDeclarator->GetFunctionSuffix() : NULL;
				if (pcSuffix && (pcSuffix->macpVirtSpecifiers.NumElements() > 0))
				{
					return true;
				}
			}
		}
	}
	return false;
}

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
#include <stdlib.h>
#include "BaseLib/ConstructorCall.h"
#include "CPPUnit.h"
#include "CPPTypeWriter.h"
#include "CPPSemanticPass.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSemanticPass::Init(void)
{
	mcNames.Init(true);
	mcErrors.Init();
	mpcUnit = NULL;
	mcTypes.Init(true);
	mcPlaces.Init(true);
	mcChanges.Init(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSemanticPass::Kill(void)
{
	if (mpcUnit)
	{
		mpcUnit->Kill();
		free(mpcUnit);
		mpcUnit = NULL;
	}
	mcChanges.Kill();
	mcPlaces.Kill();
	mcTypes.Kill();
	mcErrors.Kill();
	mcNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Checks every function the unit defines (and the classes it defines,
// with their member functions) for eTarget.  What the names resolved to
// is kept (GetResolved), and what couldn't be checked is in GetErrors.
// Returns false if anything couldn't be.
//
// With bKeep the unit checked lives on until the pass is killed, and the
// expressions' types and the changes to their places are remembered
// (GetType, FindChanges).  A pass keeps one check.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSemanticPass::Check(CCPPSyntaxTree* pcTree, ECPPTarget eTarget, bool bKeep)
{
	CCPPUnit	cGenerator;
	CCPPUnit*	pcUnit;
	bool		bResult;

	if (bKeep && !mpcUnit)
	{
		mpcUnit = NewMalloc<CCPPUnit>();
		pcUnit = mpcUnit;
	}
	else
	{
		bKeep = false;
		pcUnit = &cGenerator;
	}
	pcUnit->Init(pcTree, eTarget);
	bResult = pcUnit->Check(this, bKeep);
	pcUnit->GetErrors()->AppendTo(&mcErrors);
	if (!bKeep)
	{
		pcUnit->Kill();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The type checker typed pcExpression (a kept check).  The first answer
// is kept, as a name's is: a template's body is typed again for each of
// its instances.
//
//////////////////////////////////////////////////////////////////////////
void CCPPSemanticPass::Typed(CCPPSyntax* pcExpression, SCPP65Type* psType)
{
	if (psType && !mcTypes.Get((int64)(size)pcExpression))
	{
		mcTypes.Put((int64)(size)pcExpression, &psType);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The type checker found the type of the place pcExpression is (what an
// assignment's left is typed as): an array's is the array.
//
//////////////////////////////////////////////////////////////////////////
void CCPPSemanticPass::PlaceTyped(CCPPSyntax* pcExpression, SCPP65Type* psType)
{
	if (psType && !mcPlaces.Get((int64)(size)pcExpression))
	{
		mcPlaces.Put((int64)(size)pcExpression, &psType);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The type checker found the place pcExpression is can be changed there
// (CPPPC_REFERENCE, CPPPC_ADDRESS or CPPPC_MEMBER_CALL).
//
//////////////////////////////////////////////////////////////////////////
void CCPPSemanticPass::Changed(CCPPSyntax* pcExpression, uint8 uiChanges)
{
	uint8*	puiChanges;

	if (!pcExpression)
	{
		return;
	}
	puiChanges = mcChanges.Get((int64)(size)pcExpression);
	if (puiChanges)
	{
		*puiChanges |= uiChanges;
	}
	else
	{
		mcChanges.Put((int64)(size)pcExpression, &uiChanges);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The type of an expression of a kept check: its value's (a reference
// for a call to a function returning one), or its place's if it was only
// typed as a place.  NULL if it wasn't typed: it couldn't be, it is in a
// template nothing in the unit instantiates, or it isn't an expression.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPSemanticPass::GetType(CCPPSyntax* pcExpression)
{
	SCPP65Type**	ppsType;

	ppsType = mcTypes.Get((int64)(size)pcExpression);
	if (!ppsType)
	{
		ppsType = mcPlaces.Get((int64)(size)pcExpression);
	}
	return ppsType ? *ppsType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// How the type checker found the place of this node itself changed.
//
//////////////////////////////////////////////////////////////////////////
uint8 CCPPSemanticPass::GetChanges(CCPPSyntax* pcExpression)
{
	uint8*	puiChanges;

	puiChanges = mcChanges.Get((int64)(size)pcExpression);
	return puiChanges ? *puiChanges : 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsCPPAssignment(ECppTokens eOperator)
{
	switch (eOperator)
	{
	case CT_Assignment:
	case CT_AssignmentAdd:
	case CT_AssignmentSubtract:
	case CT_AssignmentMultiply:
	case CT_AssignmentDivide:
	case CT_AssignmentModulus:
	case CT_AssignmentBitwiseAnd:
	case CT_AssignmentBitwiseOr:
	case CT_AssignmentBitwiseXor:
	case CT_AssignmentLeftShift:
	case CT_AssignmentRightShift:
		return true;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How the variable (or field) pcExpression names can be changed where
// it is written, CPPPC_ flags, 0 if it is only read: by what is done to
// it, and to what is a part of it - in parentheses, a member of it
// (v.m, not p->m, which is what p points to), or an element (v[i]: of an
// array, and of a pointer too, which is more than is changed).  From the
// syntax an assignment, ++ and --; from a kept check a reference that
// isn't const bound to it, its address taken, and a member function that
// isn't const called on it.
//
// What isn't found: an array passed as a pointer (no & is written), and
// anything done through a pointer or reference made earlier.
//
//////////////////////////////////////////////////////////////////////////
uint8 CCPPSemanticPass::FindChanges(CCPPSyntax* pcExpression)
{
	uint8						uiChanges;
	CCPPSyntax*					pcParent;
	CCPPSyntaxBinaryExpression*	pcBinary;
	CCPPSyntaxUnaryExpression*	pcUnary;

	uiChanges = 0;
	for (;;)
	{
		uiChanges |= GetChanges(pcExpression);
		pcParent = pcExpression->mpcParent;
		if (!pcParent)
		{
			return uiChanges;
		}
		switch (pcParent->GetKind())
		{
		case CPPS_ParenthesisedExpression:
			break;
		case CPPS_MemberExpression:
			if ((((CCPPSyntaxMemberExpression*)pcParent)->mpcObject != pcExpression) || ((CCPPSyntaxMemberExpression*)pcParent)->mbArrow)
			{
				return uiChanges;
			}
			break;
		case CPPS_SubscriptExpression:
			if (((CCPPSyntaxSubscriptExpression*)pcParent)->mpcArray != pcExpression)
			{
				return uiChanges;
			}
			break;
		case CPPS_BinaryExpression:
			pcBinary = (CCPPSyntaxBinaryExpression*)pcParent;
			if ((pcBinary->mpcLeft == pcExpression) && IsCPPAssignment(pcBinary->meOperator))
			{
				uiChanges |= CPPPC_ASSIGNED;
			}
			return uiChanges;
		case CPPS_UnaryExpression:
			pcUnary = (CCPPSyntaxUnaryExpression*)pcParent;
			if ((pcUnary->meOperator == CT_Increment) || (pcUnary->meOperator == CT_Decrement))
			{
				uiChanges |= CPPPC_ASSIGNED;
			}
			return uiChanges;
		default:
			return uiChanges;
		}
		pcExpression = pcParent;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A type of a kept check as C++ would write it ("const char*",
// "NS::CFoo&", "CArray<int>"), and a declaration of szName with it
// ("int (*pf)(char)", "char ac[4]").  See CCPPTypeWriter for what is
// written and what can't be (false, with nothing appended).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSemanticPass::AppendType(SCPP65Type* psType, CChars* pszDest)
{
	return AppendDeclaration(psType, NULL, pszDest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSemanticPass::AppendDeclaration(SCPP65Type* psType, const char* szName, CChars* pszDest)
{
	CCPPTypeWriter	cWriter;

	if (!mpcUnit || !psType)
	{
		return false;
	}
	cWriter.Init(mpcUnit);
	return cWriter.AppendDeclaration(psType, szName, pszDest);
}


//////////////////////////////////////////////////////////////////////////
//
// The name whose last component is iToken names pcSymbol (the overload
// pcDeclarator declares, for a function).  The first answer is kept: a
// template's body checked again with other arguments may resolve its
// names differently.
//
//////////////////////////////////////////////////////////////////////////
void CCPPSemanticPass::Resolved(int iToken, CCPPSymbol* pcSymbol, CCPPSyntaxDeclarator* pcDeclarator)
{
	SCPPResolvedName	sName;

	if ((iToken < 0) || !pcSymbol || mcNames.Get(iToken))
	{
		return;
	}
	sName.pcSymbol = pcSymbol;
	sName.pcDeclarator = pcDeclarator;
	mcNames.Put(iToken, &sName);
}


//////////////////////////////////////////////////////////////////////////
//
// What the name whose last component is iToken resolved to, or NULL.
//
//////////////////////////////////////////////////////////////////////////
SCPPResolvedName* CCPPSemanticPass::GetResolved(int iToken)
{
	return mcNames.Get(iToken);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPSemanticPass::NumResolved(void)
{
	return mcNames.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPErrors* CCPPSemanticPass::GetErrors(void)
{
	return &mcErrors;
}


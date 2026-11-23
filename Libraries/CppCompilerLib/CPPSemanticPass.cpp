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
	masOthers.Init();
	mcErrors.Init();
	mcWarnings.Init();
	mpcUnit = NULL;
	mbTemplatesAsThemselves = false;
	mszRecordedEnum.Init();
	mpsRecordedEnum = NULL;
	masEnumCasts.Init();
	mcOptions = *GetDefaultCPPCompileOptions();
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
	masEnumCasts.Kill();
	mszRecordedEnum.Kill();
	mcChanges.Kill();
	mcPlaces.Kill();
	mcTypes.Kill();
	mcWarnings.Kill();
	mcErrors.Kill();
	masOthers.Kill();
	mcNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Checks every function the unit defines (and the classes it defines,
// with their member functions) for eTarget.  What the names resolved to
// is kept (GetResolved), what couldn't be checked is in GetErrors, and
// what the compiler would warn of is in GetWarnings.  Returns false if
// anything couldn't be.
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
	pcUnit->SetOptions(&mcOptions);
	bResult = pcUnit->GetCheck()->Check(this, bKeep);
	pcUnit->GetErrors()->AppendTo(&mcErrors);
	pcUnit->GetWarnings()->AppendTo(&mcWarnings);
	if (!bKeep)
	{
		pcUnit->Kill();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The options the next check is made with (a copy of pcOptions; NULL for
// the defaults): a build's, so that what is an error is what it says
// (CCPPCompileOptions::mbExceptions, mbMultiWordCasts, mbCConversions).
//
//////////////////////////////////////////////////////////////////////////
void CCPPSemanticPass::SetOptions(CCPPCompileOptions* pcOptions)
{
	mcOptions = pcOptions ? *pcOptions : *GetDefaultCPPCompileOptions();
}


CCPPCompileOptions* CCPPSemanticPass::GetOptions(void) { return &mcOptions; }


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
// An enum to check as an enum class, by its name with its scopes; before
// Check.  An empty name is none.
//
//////////////////////////////////////////////////////////////////////////
void CCPPSemanticPass::SetRecordedEnum(const char* szQualifiedName)
{
	mszRecordedEnum.Set(szQualifiedName ? szQualifiedName : "");
	mpsRecordedEnum = NULL;
	masEnumCasts.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a type (whatever its const, and through a reference) is the
// recorded enum's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSemanticPass::IsRecordedEnum(SCPP65Type* psType)
{
	if (!psType || !mpsRecordedEnum)
	{
		return false;
	}
	if (psType->IsReference())
	{
		psType = psType->psBase;
	}
	return psType->Unqualified() == mpsRecordedEnum;
}


//////////////////////////////////////////////////////////////////////////
//
// A place the type checker would refuse an enum class's value.  True if
// it is the recorded enum's: it is recorded (once for a node and a kind)
// and the checker goes on as if it were allowed.  False for any other
// enum class, which is refused.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSemanticPass::RecordEnumCast(CCPPSyntax* pcNode, SCPP65Type* psFrom, SCPP65Type* psTo, uint8 uiKind)
{
	SCPPEnumCast*	psCast;
	size			i;

	if (!IsRecordedEnum(psFrom) && !IsRecordedEnum(psTo))
	{
		return false;
	}
	for (i = 0; i < masEnumCasts.NumElements(); i++)
	{
		psCast = masEnumCasts.Get(i);
		if ((psCast->pcNode == pcNode) && (psCast->uiKind == uiKind))
		{
			return true;
		}
	}
	psCast = masEnumCasts.Add();
	psCast->pcNode = pcNode;
	psCast->psFrom = psFrom;
	psCast->psTo = psTo;
	psCast->uiKind = uiKind;
	return true;
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
// The label the compiler gives the function a declarator of a kept
// check declares (or defines), for the target it was checked for:
// "_CFoo__Init", "_CFoo__Run__Fi", "_Blend__Fss".  Its parameters' types
// are in it where the function is overloaded or isn't a member, as the
// target sizes them, so it is what hand written assembly must call it
// by.  False if the declarator declares no function the unit has (a
// template's, whose instances have labels of their own).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSemanticPass::AppendFunctionLabel(CCPPSyntaxDeclarator* pcDeclarator, CChars* pszDest)
{
	CCPPFunction*	pcFunction;

	pcFunction = mpcUnit ? mpcUnit->GetCheck()->FindDeclaredFunction(pcDeclarator) : NULL;
	if (!pcFunction)
	{
		return false;
	}
	mpcUnit->GetMangler()->AppendFunctionLabel(pszDest, pcFunction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The name whose last component is iToken names pcSymbol (the overload
// pcDeclarator declares, for a function).  A template's body checked
// again with other arguments may resolve its names differently: every
// different answer is kept, in the order they were found.
//
//////////////////////////////////////////////////////////////////////////
void CCPPSemanticPass::Resolved(int iToken, CCPPSymbol* pcSymbol, CCPPSyntaxDeclarator* pcDeclarator)
{
	SCPPResolvedName	sName;
	SCPPResolvedName*	psName;
	int32				iOther;

	if ((iToken < 0) || !pcSymbol)
	{
		return;
	}
	sName.pcSymbol = pcSymbol;
	sName.pcDeclarator = pcDeclarator;
	sName.iNext = -1;
	psName = mcNames.Get(iToken);
	if (!psName)
	{
		mcNames.Put(iToken, &sName);
		return;
	}

	for (;;)
	{
		if ((psName->pcSymbol == pcSymbol) && (psName->pcDeclarator == pcDeclarator))
		{
			return;
		}
		if (psName->iNext < 0)
		{
			break;
		}
		psName = masOthers.Get((size)psName->iNext);
	}
	iOther = (int32)masOthers.NumElements();
	psName->iNext = iOther;
	masOthers.Add(&sName);
}


//////////////////////////////////////////////////////////////////////////
//
// What the name whose last component is iToken resolved to (the first
// thing, if it did to several), or NULL.
//
//////////////////////////////////////////////////////////////////////////
SCPPResolvedName* CCPPSemanticPass::GetResolved(int iToken)
{
	return mcNames.Get(iToken);
}


//////////////////////////////////////////////////////////////////////////
//
// The next thing the name psResolved is of resolved to, in another
// instance of the template it is in; NULL after the last.
//
//////////////////////////////////////////////////////////////////////////
SCPPResolvedName* CCPPSemanticPass::GetNextResolved(SCPPResolvedName* psResolved)
{
	return (psResolved->iNext >= 0) ? masOthers.Get((size)psResolved->iNext) : NULL;
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


CCPPErrors* CCPPSemanticPass::GetWarnings(void) { return &mcWarnings; }  //What a build would warn of in what was checked.


bool CCPPSemanticPass::IsKept(void) const { return mpcUnit != NULL; }
bool CCPPSemanticPass::IsTemplatesAsThemselves(void) const { return mbTemplatesAsThemselves; }
const char* CCPPSemanticPass::GetRecordedEnum(void) const { return mszRecordedEnum.Text(); }


//////////////////////////////////////////////////////////////////////////
//
// The unit has made an enum's type.  If it is the enum a refactoring asks
// about (SetRecordedEnum), by its name with its scopes, it is checked as an
// enum class from here, and what that refuses is recorded (RecordEnumCast).
// The first type of the name is the one.
//
//////////////////////////////////////////////////////////////////////////
void CCPPSemanticPass::EnumMade(CCPPUnit* pcUnit, SCPP65Type* psType)
{
	CCPPTypeWriter	cWriter;
	CChars			szName;

	if (mszRecordedEnum.Empty() || mpsRecordedEnum)
	{
		return;
	}
	szName.Init();
	cWriter.Init(pcUnit);
	if (cWriter.AppendType(psType, &szName) && szName.Equals(mszRecordedEnum.Text()))
	{
		psType->bScopedEnum = true;
		mpsRecordedEnum = psType;
	}
	szName.Kill();
}
SCPP65Type* CCPPSemanticPass::GetRecordedEnumType(void) const { return mpsRecordedEnum; }
size CCPPSemanticPass::NumEnumCasts(void) const { return masEnumCasts.NumElements(); }

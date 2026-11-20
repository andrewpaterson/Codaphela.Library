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
void CCPPSyntaxFunctionSuffix::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntax::Init(pcTree, pcParent);
	macpParameters.Init(NodeArrayMallocator());
	mbVariadic = false;
	mbVariadicComma = false;
	macpQualifiers.Init(NodeArrayMallocator());
	mpcExceptionSpecification = NULL;
	macpAttributes.Init(NodeArrayMallocator());
	mpcTrailingReturn = NULL;
	macpVirtSpecifiers.Init(NodeArrayMallocator());
	mpcScope = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxFunctionSuffix::Kill(void)
{
	macpParameters.Kill();
	macpQualifiers.Kill();
	macpAttributes.Kill();
	macpVirtSpecifiers.Kill();
	CCPPSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxFunctionSuffix::PrettyPrint(CChars* pszDest, int iDepth)
{
	pszDest->Append('(');
	PrintList(pszDest, &macpParameters, ", ", iDepth);
	if (mbVariadic)
	{
		if (mbVariadicComma)
		{
			pszDest->Append(", ");
		}
		pszDest->Append("...");
	}
	pszDest->Append(')');
	if (macpQualifiers.NumElements() > 0)
	{
		pszDest->Append(' ');
		PrintList(pszDest, &macpQualifiers, " ", iDepth);
	}
	if (mpcExceptionSpecification)
	{
		pszDest->Append(' ');
		mpcExceptionSpecification->PrettyPrint(pszDest, iDepth);
	}
	if (macpAttributes.NumElements() > 0)
	{
		pszDest->Append(' ');
		PrintList(pszDest, &macpAttributes, " ", iDepth);
	}
	if (mpcTrailingReturn)
	{
		pszDest->Append(" -> ");
		mpcTrailingReturn->PrettyPrint(pszDest, iDepth);
	}
	if (macpVirtSpecifiers.NumElements() > 0)
	{
		pszDest->Append(' ');
		PrintList(pszDest, &macpVirtSpecifiers, " ", iDepth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxFunctionSuffix::VisitFields(CCPPSyntaxFields* pcFields)
{
	pcFields->Children(&macpParameters);
	pcFields->Bool(&mbVariadic);
	pcFields->Bool(&mbVariadicComma);
	pcFields->Children(&macpQualifiers);
	pcFields->Child(&mpcExceptionSpecification);
	pcFields->Children(&macpAttributes);
	pcFields->Child(&mpcTrailingReturn);
	pcFields->Children(&macpVirtSpecifiers);
	pcFields->Scope(&mpcScope);
}


//////////////////////////////////////////////////////////////////////////
//
// The const, volatile and mutable after the parameters: a member function's
// (int Get(void) const), whose this points to an object that is, and a
// lambda's.  [dcl.fct] 6, [expr.prim.lambda] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntaxFunctionSuffix::IsConst(void) { return CPPHasKeyword(&macpQualifiers, CT_const, CT___const__); }
bool CCPPSyntaxFunctionSuffix::IsVolatile(void) { return CPPHasKeyword(&macpQualifiers, CT_volatile, CT___volatile__); }
bool CCPPSyntaxFunctionSuffix::IsMutable(void) { return CPPHasKeyword(&macpQualifiers, CT_mutable); }


//////////////////////////////////////////////////////////////////////////
//
// Whether the last parameter is a function parameter pack (Args... args,
// auto... args), which takes any number of arguments.  [dcl.fct] 13
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntaxFunctionSuffix::EndsWithPack(void)
{
	CCPPSyntaxParameter*	pcLast;

	pcLast = (macpParameters.NumElements() > 0) ? (CCPPSyntaxParameter*)macpParameters.GetPtr(macpParameters.NumElements() - 1) : NULL;
	return pcLast && (pcLast->GetKind() == CPPS_Parameter) && pcLast->mpcDeclarator && pcLast->mpcDeclarator->mbPack;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function with these parameters takes uiArguments arguments:
// defaults make up the rest, and ... or a function parameter pack (Args...
// args) take any number more.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntaxFunctionSuffix::TakesArgumentCount(size uiArguments)
{
	size	uiParameters;
	size	uiRequired;

	uiParameters = NumParameters();
	uiRequired = RequiredParameters();
	if (EndsWithPack())
	{
		return uiArguments + 1 >= uiRequired;
	}
	return (uiArguments >= uiRequired) && ((uiArguments <= uiParameters) || mbVariadic);
}


//////////////////////////////////////////////////////////////////////////
//
// The parameter a call's argument is for (the first is 0): its own, or the
// last, a function parameter pack (auto... args), which takes each
// argument from its place on.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxParameter* CCPPSyntaxFunctionSuffix::ArgumentParameter(size uiArgument)
{
	size	uiParameters;

	uiParameters = NumParameters();
	return (CCPPSyntaxParameter*)macpParameters.GetPtr((uiArgument < uiParameters) ? uiArgument : uiParameters - 1);
}


//////////////////////////////////////////////////////////////////////////
//
// The number of parameters declared: none for (void).
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxFunctionSuffix::NumParameters(void)
{
	CCPPSyntaxParameter*	pcParameter;

	if (macpParameters.NumElements() != 1)
	{
		return macpParameters.NumElements();
	}
	pcParameter = (CCPPSyntaxParameter*)macpParameters.GetPtr(0);
	//(void), but not void (M::*f)(...) or void (*f)(...).
	if (pcParameter->mpcSpecifiers && (pcParameter->mpcSpecifiers->muiTypeKeywords == CPPTK_VOID) &&
		(!pcParameter->mpcDeclarator || ((pcParameter->mpcDeclarator->NumPointers() == 0) && !pcParameter->mpcDeclarator->mpcNested && (pcParameter->mpcDeclarator->macpSuffixes.NumElements() == 0))))
	{
		return 0;
	}
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
// The number of parameters a call must give: those before the trailing
// ones with defaults.
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxFunctionSuffix::RequiredParameters(void)
{
	size	uiRequired;

	uiRequired = NumParameters();
	while ((uiRequired > 0) && ((CCPPSyntaxParameter*)macpParameters.GetPtr(uiRequired - 1))->mpcDefault)
	{
		uiRequired--;
	}
	return uiRequired;
}

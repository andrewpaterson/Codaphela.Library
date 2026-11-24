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
#include <string.h>
#include "CTInt.h"
#include "CTShort.h"
#include "CTLongLong.h"
#include "CTChar.h"
#include "CTBoolean.h"
#include "CTString.h"
#include "CTIdentifier.h"
#include "CTUserLiteral.h"
#include "CppTokeniser.h"
#include "CPP65IRFrontEnd.h"
#include "CPPSyntaxTree.h"
#include "CPPUnit.h"
#include "CPPConstants.h"
#include "CPPSemanticPass.h"
#include "CPPTypeWriter.h"
#include "LiteralHelper.h"


//////////////////////////////////////////////////////////////////////////
//
// Constant, without reporting an error when the expression isn't constant
// (most aren't: it is only looking), and without changing whether an
// earlier quiet evaluation failed.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::QuietConstant(CCPPSyntax* pcExpression, int64* plliValue)
{
	bool	bConstant;

	mcDiagnostics.BeginTrial();
	bConstant = mcFolder.Constant(pcExpression, plliValue);
	mcDiagnostics.EndTrial();
	return bConstant;
}


//////////////////////////////////////////////////////////////////////////
//
// The value of a variable template's instance that is a const or
// constexpr integer (size<long>): its initialiser, worked out with its
// template's parameters its arguments.  False if it isn't one.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::VariableTemplateValue(CCPPSyntaxIdExpression* pcId, int64* plliValue)
{
	CCPPVariable*					pcVariable;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxInitDeclarator*		pcInit;
	bool							bResult;

	pcVariable = mcInstantiator.VariableInstance(pcId->mpcName, pcId);
	if (!pcVariable || !pcVariable->mpsType->IsInteger())
	{
		return false;
	}
	pcInit = mcInstantiator.VariableDeclarator(pcVariable);
	pcSimple = (CCPPSyntaxSimpleDeclaration*)pcInit->mpcParent;
	if (!pcSimple->mpcSpecifiers || !(pcSimple->mpcSpecifiers->muiFlags & (CPPDS_CONST | CPPDS_CONSTEXPR)) || !pcInit->mpcInitializer)
	{
		return false;
	}
	mcContext.EnterBindings(pcVariable->mpcBindings);
	bResult = mcFolder.Constant(pcInit->mpcInitializer, plliValue);
	mcContext.LeaveBindings();
	if (bResult)
	{
		*plliValue = mcFolder.ConstantOfType(pcVariable->mpsType, *plliValue);
	}
	return bResult;
}




//////////////////////////////////////////////////////////////////////////
//
// Whether pcValue can initialise a psTo without losing const or volatile
// (CCPP65TypeChecker::KeepsQualifiers), for the IR front end's initialisers
// and returns.  It is an error where it is, in quiet mode too, as ReportTypeOf's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckInitialiser(CCPPSyntax* pcValue, SCPP65Type* psTo)
{
	bool	bKept;

	mcDiagnostics.BeginLoud();
	bKept = mcTypeChecker.KeepsQualifiers(pcValue, psTo);
	mcDiagnostics.EndLoud();
	return bKept;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcValue, the one value in braces that initialises a psTo, is not
// narrowed by its conversion (CCPP65TypeChecker::NotNarrowed), for the
// variables' initialisers the unit converts itself.  An error
// where it is, as CheckInitialiser's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckBracedValue(CCPPSyntax* pcValue, SCPP65Type* psTo)
{
	bool	bNotNarrowed;

	mcDiagnostics.BeginLoud();
	bNotNarrowed = mcTypeChecker.NotNarrowed(pcValue, psTo);
	mcDiagnostics.EndLoud();
	return bNotNarrowed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::Supported(SCPP65Type* psType, CCPPSyntax* pcNode)
{
	if (psType->IsScalar() && (psType->iBytes > 8))
	{
		return mcDiagnostics.Error(pcNode, "Integers wider than 64 bits are not supported by the compiler");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A local declared constexpr is initialised with a constant expression:
// one of an integer type is worked out here, and any other as a global's is
// (CCPPConstants::CheckLocal), and is an error (and false) where it has no
// value.  The local is still made when the function runs.
// [dcl.constexpr] 9
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckConstexprLocal(CCPPVariable* pcVariable, CCPPSyntaxInitDeclarator* pcInit)
{
	CChars	szError;
	int64	lliValue;

	if (!pcInit->mpcInitializer)
	{
		return mcDiagnostics.ReportError(pcInit, "A constexpr variable needs an initialiser");
	}
	if (!pcVariable->mpsType->IsInteger())
	{
		return mpcConstants->CheckLocal(pcVariable, pcInit);
	}
	if (mcFolder.Constant(pcInit->mpcInitializer, &lliValue))
	{
		return true;
	}
	szError.Init();
	mcFolder.ConstantMessage("A constexpr variable's initialiser must be a constant", &szError);
	mcDiagnostics.ReportError(pcInit, szError.Text());
	szError.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// What is in the brackets or braces of Type(arguments) or Type{arguments}, or
// NULL if it isn't either.
//
//////////////////////////////////////////////////////////////////////////
CArraySyntax* CCPPUnit::ConstructElements(CCPPSyntaxConstructExpression* pcConstruct)
{
	CCPPSyntax*	pcInitialiser;

	pcInitialiser = pcConstruct->mpcInitializer;
	if (pcInitialiser && ((pcInitialiser->GetKind() == CPPS_ParenthesisedInitializer) || (pcInitialiser->GetKind() == CPPS_InitializerList)))
	{
		return mcInstantiator.ListElements(pcInitialiser);
	}
	return NULL;
}




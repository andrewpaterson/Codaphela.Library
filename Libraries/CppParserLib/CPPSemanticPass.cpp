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
#include "CPP65Generator.h"
#include "CPPSemanticPass.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSemanticPass::Init(void)
{
	mcNames.Init(true);
	mcErrors.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSemanticPass::Kill(void)
{
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
//////////////////////////////////////////////////////////////////////////
bool CCPPSemanticPass::Check(CCPPSyntaxTree* pcTree, ECPPTarget eTarget)
{
	CCPP65Generator	cGenerator;
	bool			bResult;

	cGenerator.Init(pcTree, eTarget);
	bResult = cGenerator.Check(this);
	cGenerator.GetErrors()->AppendTo(&mcErrors);
	cGenerator.Kill();
	return bResult;
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


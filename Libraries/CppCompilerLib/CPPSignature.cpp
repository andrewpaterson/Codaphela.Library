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
#include "CPPSignature.h"


//////////////////////////////////////////////////////////////////////////
//
// A signature still to be read from pcDeclarator: what declares it, and
// nothing it says.
//
//////////////////////////////////////////////////////////////////////////
void SCPPSignature::Init(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, CCPPClass* pcClass, ECPPDeclaredAs eAs)
{
	this->pcSpecifiers = pcSpecifiers;
	pcWritten = pcDeclarator;
	this->pcClass = pcClass;
	pcTemplate = NULL;
	pszArguments = NULL;
	this->eAs = eAs;

	pcTypeSpecifiers = pcSpecifiers;
	this->pcDeclarator = pcDeclarator;
	pcSuffix = NULL;
	pcName = NULL;
	pcComponent = NULL;
	lliName = -1;
	eKind = CPPFK_Function;
	psReturn = NULL;
	apsParameters.Init();
	apcDefaults.Init();
	bVariadic = false;
	bConstThis = false;
	bVolatileThis = false;
	eRefQualifier = CPPRQ_None;
	pcNamespace = NULL;
	bCLinkage = false;
	uiFlags = pcSpecifiers ? pcSpecifiers->muiFlags : 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SCPPSignature::Kill(void)
{
	apcDefaults.Kill();
	apsParameters.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the specifiers say it: static, virtual, explicit, constexpr...
//
//////////////////////////////////////////////////////////////////////////
bool SCPPSignature::Says(uint32 uiFlag) const
{
	return (uiFlags & uiFlag) != 0;
}


bool SCPPSignature::IsDefinition(void) const { return eAs == CPPDA_Definition; }
bool SCPPSignature::IsPure(void) const { return eAs == CPPDA_Pure; }


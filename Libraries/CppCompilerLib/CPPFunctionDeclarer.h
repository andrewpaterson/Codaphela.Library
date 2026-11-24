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
#ifndef __C_P_P_FUNCTION_DECLARER_H__
#define __C_P_P_FUNCTION_DECLARER_H__
#include "BaseLib/ArrayTemplate.h"
#include "CPPSyntaxes.h"
#include "CPPFunction.h"


class CCPPUnit;
class CCPPClass;
class CCPPTemplate;
class CCPPErrors;
class CCPPDiagnostics;
class CCPPDeclarators;
class CCPPContext;
class CCPP65Types;
struct SCPP65Type;


//A function declared quietly that couldn't be (DeclareQuietly): its declarator, and what was said declaring it, which
//is said where it is called (ReportFailed).
struct SCPPFailedDeclaration
{
	CCPPSyntaxDeclarator*	pcDeclarator;
	CCPPErrors*				pcErrors;		//The diagnostics' to free; NULL for nothing said.
};


typedef CArrayTemplate<SCPPFailedDeclaration>	CArrayCPPFailedDeclarations;


//Declares the unit's functions (CCPPUnit::GetFunctionDeclarer): a function's signature from its declarator, the
//function it is a declaration of or a new one, what it returns when that is left to its body, and the checks of what a
//function's declaration and definition may be.  [dcl.fct], [over.oper], [basic.stc.dynamic], [dcl.fct.def]
class CCPPFunctionDeclarer
{
protected:
	CCPPUnit*						mpcUnit;
	CCPP65Types*					mpcTypes;
	CCPPDiagnostics*				mpcDiagnostics;
	CCPPDeclarators*				mpcDeclarators;
	CCPPContext*					mpcContext;
	CArrayCPPFailedDeclarations		masFailed;	//Functions whose declarations were not supported, and why (ReportFailed).

public:
	void					Init(CCPPUnit* pcUnit);
	void					Kill(void);

	CCPPFunction*			Declare(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, bool bDefinition, CCPPClass* pcClass, bool bPure, CCPPTemplate* pcTemplate = NULL, CChars* pszArguments = NULL);
	CCPPFunction*			DeclareQuietly(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, bool bDefinition, CCPPClass* pcClass, bool bPure, CCPPErrors** ppcErrors = NULL);
	void					DeclareBlockFunction(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator);
	CCPPFunction*			AddImplicit(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, SCPP65Type* psReturn, bool bDefined);
	bool					ResolveReturn(CCPPFunction* pcFunction, CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator);
	void					ReportFailed(int64 lliName);

	SCPP65Type*				FunctionType(CCPPFunction* pcFunction);
	SCPP65Type*				ParameterType(SCPP65Type* psType);
	bool					IsWeak(CCPPFunction* pcFunction);
	bool					DeclaresAllocation(CCPPSyntaxDeclarator* pcDeclarator);

	bool					CheckAbstractSignature(CCPPFunction* pcFunction, CCPPSyntax* pcNode);
	bool					CheckDefaultedOrDeleted(CCPPFunction* pcFunction, CCPPSyntaxFunctionDefinition* pcDefinition);
	bool					CheckMemberInitialisers(CCPPFunction* pcFunction);

protected:
	CCPPFunction*			DeclareSignature(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, bool bDefinition, CCPPClass* pcClass, bool bPure, CCPPTemplate* pcTemplate, CChars* pszArguments);
	SCPP65Type*				ReturnedType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntaxReturnStatement* pcReturn);
	void					FindReturns(CCPPSyntax* pcNode, CArraySyntax* papcReturns);
	SCPP65Type*				ReturnDeclaratorType(SCPP65Type* psReturn, CCPPSyntaxDeclarator* pcDeclarator);
	bool					IsDefinedWhereDeclared(CCPPSyntaxName* pcName);
	bool					IsAllocationFunction(int64 lliName);
	bool					CheckAllocationFunction(int64 lliName, SCPP65Type* psReturn, CArrayCPP65Types* papsParameters, CCPPSyntax* pcNode);
	bool					CheckOperatorSignature(CCPPSyntaxNameComponent* pcComponent, int64 lliName, CArrayCPP65Types* papsParameters, CArraySyntax* papcDefaults, CCPPClass* pcClass, CCPPSyntax* pcNode);
	bool					OperatorError(CCPPClass* pcClass, CCPPSyntax* pcNode, const char* szMessage);
	bool					CheckDefaultsFollow(CCPPFunction* pcFunction, CCPPSyntax* pcNode);
};


#endif // __C_P_P_FUNCTION_DECLARER_H__


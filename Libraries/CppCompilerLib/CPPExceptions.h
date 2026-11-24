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
#ifndef __C_P_P_EXCEPTIONS_H__
#define __C_P_P_EXCEPTIONS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "CPPSyntaxes.h"
#include "CPPExceptionSpec.h"
#include "CPPFunction.h"
#include "CPPUnitData.h"


class CCPPUnit;
class CCPPClass;
struct SCPP65Type;


//Exception specifications for the unit (CCPPUnit::GetExceptions): what a function is declared to throw (noexcept and
//throw(...) written, or what an implicit member's parts may throw), what an expression may throw (noexcept(e)), the
//checks that a pointer to a function or a redeclaration allows no more than it should, and what a throw and a try
//block's jumps may not do.  [except.spec]
class CCPPExceptions
{
protected:
	CCPPUnit*				mpcUnit;
	CArrayCPPUnitRoutines	masThrownDestroyers;	//The functions that destroy thrown objects, still to write (ThrownDestroyer).

public:
	void					Init(CCPPUnit* pcUnit);
	void					Kill(void);

	bool					CheckThrown(CCPPSyntaxThrowExpression* pcThrow);
	bool					CollectTryJumps(CCPPSyntax* pcNode, CCPPSyntax* pcBody, CArraySyntax* papcLabels, CArraySyntax* papcGotos);
	bool					CheckTryJumps(CCPPSyntax* pcBody);
	CCPPSyntaxExceptionSpecification*	WrittenSpecification(CCPPSyntaxFunctionSuffix* pcSuffix);
	void					WrittenExceptions(CCPPSyntaxExceptionSpecification* pcSpecification, CCPPExceptionSpec* pcDest);
	void					DeclaredExceptions(CCPPFunction* pcFunction, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntaxExceptionSpecification* pcSpecification, CCPPExceptionSpec* pcDest);
	void					SetDeclaredExceptions(CCPPFunction* pcFunction, CCPPSyntaxDeclarator* pcDeclarator);
	bool					CheckRedeclaredExceptions(CCPPFunction* pcFunction, CCPPSyntaxDeclarator* pcDeclarator);
	CCPPExceptionSpec*		ExceptionsOf(CCPPFunction* pcFunction);
	bool					IsNoexcept(CCPPFunction* pcFunction);
	void					ImplicitExceptions(CCPPFunction* pcFunction, CCPPExceptionSpec* pcDest);
	void					PartsExceptions(CCPPClass* pcClass, ECPPSpecialMember eMember, CCPPExceptionSpec* pcDest);
	void					ClassExceptions(CCPPClass* pcClass, ECPPSpecialMember eMember, CCPPExceptionSpec* pcDest);
	bool					ClassMayThrow(CCPPClass* pcClass, ECPPSpecialMember eMember);
	bool					AllowsOnlyWhat(CCPPExceptionSpec* pcTighter, CCPPExceptionSpec* pcLooser);
	bool					PointerExceptions(CCPPSyntax* pcCallee, CCPPExceptionSpec* pcDest);
	bool					CheckPointerExceptions(CCPPSyntaxInitDeclarator* pcInit);
	bool					CheckAssignedExceptions(CCPPSyntaxBinaryExpression* pcAssignment);
	bool					ValueAllowsOnly(CCPPSyntax* pcValue, SCPP65Type* psPointer, CCPPExceptionSpec* pcPointer);
	bool					ConstructionMayThrow(CCPPClass* pcClass, CArraySyntax* papcArguments, CCPPSyntax* pcNode, bool bTemporary);
	bool					ArgumentMayThrow(CCPPSyntax* pcArgument, SCPP65Type* psParameter);
	bool					CallMayThrow(CCPPSyntaxCallExpression* pcCall);
	bool					NewMayThrow(CCPPSyntaxNewExpression* pcNew);
	bool					DeleteMayThrow(CCPPSyntaxDeleteExpression* pcDelete);
	bool					MayThrow(CCPPSyntax* pcExpression);

	CCPPSyntax*				TryRegionOf(CCPPSyntax* pcNode, CCPPSyntax* pcStop);
	SCPP65Type*				HandlerType(CCPPSyntaxCatchHandler* pcHandler);
	bool					HandlerLocal(CCPPSyntaxCatchHandler* pcHandler);
	void					ThrownDestroyer(SCPP65Type* psType, CChars* pszLabel);
	bool					WriteThrownDestroyers(void);
};


#endif // __C_P_P_EXCEPTIONS_H__


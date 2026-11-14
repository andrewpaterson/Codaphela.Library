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
#ifndef __C_P_P_BOUND_CALL_H__
#define __C_P_P_BOUND_CALL_H__
#include "BaseLib/MapLongTemplate.h"
#include "CPPSyntaxes.h"
#include "CPPSymbol.h"
#include "CPPFunction.h"
#include "CPPTemplate.h"


class CCPPUnit;


//How a call is made.
enum ECPPBoundCall
{
	CPPBC_Failed,			//It can't be: an error says why.
	CPPBC_PackExpansion,	//Its arguments have a pack to expand first; then it is bound again.
	CPPBC_Member,			//A member function's: object.f(), pointer->f(), Class::f(), or f() where a member is meant.
	CPPBC_MemberPointer,	//(object.*pointer)() and (pointer->*pointer)().
	CPPBC_Function,			//The function, by its label.
	CPPBC_Pointer,			//Through a pointer to a function, which the callee is.
	CPPBC_Builtin,			//A __builtin_ function, written where it is called.
	CPPBC_InlineMemcpy,		//memcpy of a few bytes, copied where it is called.
	CPPBC_Unchecked,		//An intrinsic of the host's compiler that no header declares (only when checking).
};


//What a call expression means, worked out once (CCPPCallBinder::Bind): how it is made, and for a function called
//by its label, which.  The type checker gives its type from this and the IR front end writes it from this.
class CCPPBoundCall
{
public:
	ECPPBoundCall		meHow;
	CCPPSyntax*			mpcCallee;		//What is called, without its parentheses.
	CCPPSyntaxName*		mpcName;		//The callee's name, or NULL if it isn't one.
	int64				mlliName;		//The function's name (-1 if the callee has none).
	CCPPFunction*		mpcFunction;	//CPPBC_Function: the function chosen (a template's instance is made).

	//Where it was bound: the same call in another instance of a template is another call.
	CCPPTemplateArguments*	mpcBindings;
	CCPPFunction*			mpcIn;

	void	Init(void);
};


typedef CMapLongTemplate<CCPPBoundCall>	CMapCPPBoundCalls;


//Binds calls.  [expr.call]  It has the one cascade that says what a call is: a member's, a template's instance, a
//builtin, a function found by its name or by its arguments' namespaces, or a pointer's.  A call bound once is
//remembered for the template arguments and the function it is in; one that failed is bound again when it is
//asked for with its errors wanted, so they are reported again.
class CCPPCallBinder
{
protected:
	CCPPUnit*			mpcUnit;
	CMapCPPBoundCalls	mcBound;

public:
	void	Init(CCPPUnit* pcUnit);
	void	Kill(void);

	bool	Bind(CCPPSyntaxCallExpression* pcCall, CCPPBoundCall* pcBound, bool bReport);

protected:
	void	Resolve(CCPPSyntaxCallExpression* pcCall, CCPPBoundCall* pcBound);
};


#endif // __C_P_P_BOUND_CALL_H__

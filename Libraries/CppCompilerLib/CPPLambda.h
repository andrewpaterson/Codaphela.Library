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
#ifndef __C_P_P_LAMBDA_H__
#define __C_P_P_LAMBDA_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/Chars.h"
#include "CPPSymbol.h"
#include "CPPFunctionPacks.h"


class CCPPErrors;
class CCPPClass;
class CCPPFunction;
class CCPPSyntax;
class CCPPSyntaxLambdaExpression;
class CCPPTemplateArguments;


//What a closure holds of one thing its lambda captures: a field of the closure, made where the lambda is from a
//variable of the function it is in (its name there), an init-capture's initialiser, or this.
struct SCPPLambdaCapture
{
	CCPPSymbol*		pcSymbol;		//What the body names: the enclosing function's variable, or an init-capture's own.  NULL for this.
	CCPPSyntax*		pcInitialiser;	//What the field is made from, in the enclosing function.  NULL for this.
	size			uiField;		//Its field in the closure's class.
	bool			bReference;		//The field is a reference to it.
	bool			bThis;			//The field is the enclosing object's this.
};


typedef CArrayTemplate<SCPPLambdaCapture>	CArrayCPPLambdaCaptures;


//A lambda's closure: an unnamed class whose fields are what it captures and whose operator() is its body
//(CCPPUnit::GetLambda).  The closure of a lambda in a template's instance, or in another function, is another.
//[expr.prim.lambda] 3
struct SCPPLambda
{
	CCPPSyntaxLambdaExpression*	pcSyntax;
	CCPPFunction*				pcEnclosing;	//The function the lambda is in (another closure's operator() for one in a lambda), or NULL.
	CCPPTemplateArguments*		pcBindings;
	CCPPClass*					pcClass;
	CArrayCPPLambdaCaptures		asCaptures;
	bool						bThis;			//It captures this: the enclosing object's members are reached through it.
	CCPPFunctionPacks			cFunctionPacks;	//The parameter packs of the functions the lambda is in: their elements are what it captures of one, and what its body expands it into (AddParameters).
	bool						bGeneric;		//A parameter is auto: operator() is made for each call's arguments.
	bool						bMutable;		//operator() isn't const: its copies can be changed.
	bool						bFailed;		//It couldn't be made...
	CCPPErrors*					pcErrors;		//...and what was said making it, said again each time it is asked for (the diagnostics' to free).
	CCPPFunction*				pcCall;			//operator() when it isn't generic.
	CChars						szName;			//In a function every unit may write: the function's label, __L and the lambda's number in its
	//definition, so the closure is one type, with the same labels, in every unit.  Else empty: the
	//unit's own (Unnamed<n>).  [expr.prim.lambda] 3, [basic.def.odr] 6
};


typedef CArrayTemplatePtr<SCPPLambda>	CArrayCPPLambdas;


#endif // __C_P_P_LAMBDA_H__

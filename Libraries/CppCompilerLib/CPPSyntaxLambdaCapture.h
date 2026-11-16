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
#ifndef __C_P_P_SYNTAX_LAMBDA_CAPTURE_H__
#define __C_P_P_SYNTAX_LAMBDA_CAPTURE_H__
#include "CPPSyntax.h"


class CCPPSyntaxIdExpression;


//What a lambda captures, and how.
enum ECPPLambdaCapture
{
	CPPLC_None,			//A lambda's default when it has none.
	CPPLC_Copy,			//name, = as the default, or name = initialiser.
	CPPLC_Reference,	//&name, & as the default, or &name = initialiser.
	CPPLC_This,			//this.
};


//One of a lambda's captures: a variable of the function it is in (mpcName, an expression of it there), this, or an
//init-capture (name = initialiser, the name a variable of the lambda's own, mpcSymbol).  [expr.prim.lambda] 1
class CCPPSyntaxLambdaCapture : public CCPPSyntax
{
public:
	ECPPLambdaCapture		meKind;
	CCPPSyntaxIdExpression*	mpcName;
	int32					miName;
	CCPPSyntax*				mpcInitializer;
	CCPPSymbol*				mpcSymbol;

	void					Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent) override;
	CPP_SYNTAX_NODE(LambdaCapture)
	void					PrettyPrint(CChars* pszDest, int iDepth) override;
	void					VisitFields(CCPPSyntaxFields* pcFields) override;
};


#endif // __C_P_P_SYNTAX_LAMBDA_CAPTURE_H__

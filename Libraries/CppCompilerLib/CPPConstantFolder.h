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
#ifndef __C_P_P_CONSTANT_FOLDER_H__
#define __C_P_P_CONSTANT_FOLDER_H__
#include "BaseLib/PrimitiveTypes.h"
#include "CPP65Type.h"
#include "CPPSyntaxes.h"
#include "CppTokens.h"


class CCPPUnit;
class CCPPToken;
class CCPPFunction;


//A constant's value (CCPPConstantFolder::Evaluate): an integer's, or a floating point number's with its type.
struct SCPPConstant
{
	SCPP65Type*	psFloat;	//The floating point type, NULL for an integer.
	int64		lliValue;	//An integer's value.
	double		dValue;		//A floating point number's, as its type keeps it.

	double		AsDouble(void) const;
	void		SetFloat(SCPP65Type* psType, double d);
	void		SetInteger(int64 lli);
};


//Works out the value of a constant expression for the unit (CCPPUnit::GetFolder): an integer's or a floating point
//number's (SCPPConstant), by folding its syntax tree in one walk (Fold), or, for what the tree can't give, by running
//it (RunConstant: CCPPConstants, the IR interpreter).  An integer is kept as its type keeps it, and a float rounded
//to a float: as the target would.  Why a constant had no value is kept for its error (ConstantError).
class CCPPConstantFolder
{
protected:
	CCPPUnit*				mpcUnit;

public:
	void					Init(CCPPUnit* pcUnit);
	void					Kill(void);

	bool					Evaluate(CCPPSyntax* pcExpression, SCPPConstant* psValue);
	bool					Constant(CCPPSyntax* pcExpression, int64* plliValue);
	bool					FloatConstant(CCPPSyntax* pcExpression, double* pdValue);
	bool					Fold(CCPPSyntax* pcExpression, SCPPConstant* psValue);
	bool					BinaryFloat(ECppTokens eOperator, SCPPConstant* psLeft, SCPPConstant* psRight, SCPPConstant* psValue);
	bool					CastConstant(CCPPSyntaxTypeId* pcType, CCPPSyntax* pcOperand, SCPPConstant* psValue);
	bool					ConstexprFloat(CCPPSyntaxIdExpression* pcId, SCPPConstant* psValue);
	bool					NameConstant(CCPPSyntaxIdExpression* pcId, int64* plliValue);
	bool					UnaryConstant(CCPPSyntaxUnaryExpression* pcUnary, int64 lliOperand, int64* plliValue);
	bool					ConstantError(CCPPSyntax* pcNode, const char* szMessage);
	void					ConstantMessage(const char* szMessage, CChars* pszDest);
	bool					RunConstant(CCPPSyntax* pcExpression, int64* plliValue);
	SCPP65Type*				ObjectConstantType(CCPPSyntax* pcExpression);
	bool					NamesConstexpr(CCPPSyntax* pcNode);
	SCPP65Type*				ConstantTypeOf(CCPPSyntax* pcExpression);
	bool					ConstexprCall(CCPPSyntaxCallExpression* pcCall, int64* plliValue);
	bool					ConstexprResult(CCPPFunction* pcFunction, CCPPSyntax* pcExpression, int64* plliValue);
	int64					ConstantOfType(SCPP65Type* psType, int64 lliValue);
	bool					BinaryConstant(CCPPSyntaxBinaryExpression* pcBinary, int64 lliLeft, int64 lliRight, int64* plliValue);
	bool					IsSignedOverflow(ECppTokens eOperator, int iBits, int64 lliLeft, int64 lliRight, int64 lliResult);
	void					ConstantOverflow(CCPPSyntax* pcExpression, int64 lliValue, bool bUnsignedWraps);
	bool					TokenConstant(CCPPToken* pcToken, int64* plliValue);
	bool					TokenFloat(CCPPToken* pcToken, double* pdValue, SCPP65Type** ppsType);
	bool					FloatBuiltin(CCPPSyntax* pcExpression, double* pdValue, SCPP65Type** ppsType);
	int64					FloatBits(SCPP65Type* psType, double dValue);

	bool					VariableTemplateValue(CCPPSyntaxIdExpression* pcId, int64* plliValue);
	bool					QuietConstant(CCPPSyntax* pcExpression, int64* plliValue);
};


#endif // __C_P_P_CONSTANT_FOLDER_H__


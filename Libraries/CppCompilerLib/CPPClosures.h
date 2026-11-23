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
#ifndef __C_P_P_CLOSURES_H__
#define __C_P_P_CLOSURES_H__
#include "CPPLambda.h"


class CCPPUnit;
class CCPPClass;
class CCPPFunction;
class CCPPSymbol;
class CCPPSyntax;
class CCPPSyntaxIdExpression;
class CCPPSyntaxLambdaExpression;
class CCPPSyntaxParameter;
class CCPPVariable;
struct SCPP65Type;
struct SCPPFunctionPack;


//The unit's lambdas (CCPPUnit::GetClosures): each lambda's closure (SCPPLambda), made where the function it is in is
//worked out, what it captures, its operator() and conversion, a generic lambda's instances, and what the body of one
//being written is in.  [expr.prim.lambda]
class CCPPClosures
{
protected:
	CCPPUnit*				mpcUnit;
	CArrayCPPLambdas		mapsLambdas;		//Every lambda's closure, as each function it is in makes it.

public:
	void					Init(CCPPUnit* pcUnit);
	void					Kill(void);

	SCPPLambda*				GetLambda(CCPPSyntaxLambdaExpression* pcSyntax);
	void					NameLambda(SCPPLambda* psLambda);
	bool					LambdaNumber(CCPPSyntax* pcNode, CCPPSyntaxLambdaExpression* pcLambda, size* puiNumber);
	bool					LambdaCaptures(SCPPLambda* psLambda);
	bool					UsedCaptures(SCPPLambda* psLambda, CCPPSyntax* pcNode);
	bool					CapturePack(SCPPLambda* psLambda, SCPPFunctionPack* psPack, CCPPSyntaxIdExpression* pcName, bool bReference, CCPPSyntax* pcNode);
	bool					IsPackCaptured(SCPPLambda* psLambda, SCPPFunctionPack* psPack);
	bool					UseThis(SCPPLambda* psLambda, CCPPSyntax* pcNode);
	bool					IsEnclosingLocal(CCPPSymbol* pcSymbol, SCPPLambda* psLambda);
	bool					IsEnclosingConstant(CCPPSymbol* pcSymbol, SCPPLambda* psLambda);
	bool					IsOuterLocal(CCPPSymbol* pcSymbol);
	bool					IsOuterConstant(CCPPSymbol* pcSymbol);
	CCPPVariable*			EnclosingStatic(CCPPSymbol* pcSymbol);
	SCPP65Type*				EnclosingConstantType(CCPPSymbol* pcSymbol);
	SCPP65Type*				EnclosingConstantObjectType(CCPPSymbol* pcSymbol);
	bool					IsOdrUse(CCPPSyntax* pcNode);
	bool					IsThisMember(CCPPSymbol* pcSymbol);
	SCPPLambdaCapture*		FindLambdaCapture(SCPPLambda* psLambda, CCPPSymbol* pcSymbol);
	bool					AddCapture(SCPPLambda* psLambda, CCPPSymbol* pcSymbol, CCPPSyntax* pcInitialiser, bool bReference, bool bOwn, CCPPSyntax* pcNode);
	bool					AddThisCapture(SCPPLambda* psLambda, CCPPSyntax* pcNode);
	size					AddClosureField(SCPPLambda* psLambda, SCPP65Type* psType);
	bool					LayoutClosure(SCPPLambda* psLambda);
	bool					LambdaCall(SCPPLambda* psLambda);
	void					AddLambdaConversion(SCPPLambda* psLambda, CCPPFunction* pcCall);
	CCPPFunction*			LambdaFunction(SCPPLambda* psLambda, CArrayCPP65Types* papsParameters);
	CCPPFunction*			AddLambdaFunction(SCPPLambda* psLambda, int64 lliName, CArrayCPP65Types* papsParameters, SCPP65Type* psReturn);
	void					GenericLambdaCall(SCPPLambda* psLambda, CArraySyntax* papcArguments);
	CCPPFunction*			GenericLambdaInstance(SCPPLambda* psLambda, CArrayCPP65Types* papsParameters);
	void					GenericLambdaConversion(SCPPLambda* psLambda, SCPP65Type* psFunction);
	bool					ParameterTakesType(CCPPSyntaxParameter* pcParameter, SCPP65Type* psType);
	SCPPLambda*				WritingLambda(void);
	SCPPLambda*				BodyLambda(CCPPFunction* pcFunction);
	SCPPLambdaCapture*		FindCapture(CCPPSymbol* pcSymbol);
	CCPPFunction*			OuterFunction(void);
	CCPPClass*				ThisClass(void);
	SCPP65Type*				LambdaType(CCPPSyntaxLambdaExpression* pcSyntax);
	void					FreeLambdas(void);
};


#endif // __C_P_P_CLOSURES_H__


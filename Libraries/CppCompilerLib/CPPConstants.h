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
#ifndef __C_P_P_CONSTANTS_H__
#define __C_P_P_CONSTANTS_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "IRInterpreter.h"
#include "CPPFunction.h"


class CCPPUnit;
class CCPP65IRFrontEnd;


//A function the IR of a constant names: the C++ function, and its IR once it has been asked for.
struct SCPPConstantFunction
{
	CChars			szLabel;
	CCPPFunction*	pcFunction;
	CIRFunction*	pcIR;			//NULL until it is written, and if it can't be.
	CIRModule*		pcModule;
	bool			bAsked;			//It has been written, or tried: once is enough.
};


typedef CArrayTemplate<SCPPConstantFunction>	CArrayCPPConstantFunctions;
typedef CArrayTemplatePtr<CCPP65IRFrontEnd>		CArrayCPPConstantFrontEnds;


//Works out the constant expressions the syntax tree's folder (CCPPUnit::Constant) can't: a call of a constexpr
//function, and what such functions do (ConstexprPlan.txt).  The expression is written as the body of a function of
//no arguments, in IR, by a front end that generates no code, and CIRInterpreter runs it; the constexpr functions it
//calls are written the same way when the interpreter asks for them, and kept.  A front end holds one function
//being written, and a constant may be needed in the middle of one (an array's size in a constexpr function), so
//there are as many as are in use at once.
class CCPPConstants : public CIRInterpreterHost
{
protected:
	CCPPUnit*					mpcUnit;
	CIRInterpreter				mcInterpreter;
	CArrayCPPConstantFrontEnds	mapcFrontEnds;
	size						muiWriting;			//How many of the front ends are writing a function now.
	CArrayCPPConstantFunctions	masFunctions;
	size						muiNames;			//Names made (NewName).
	CChars						mszReason;			//Why the last expression asked for had no value.

public:
	void			Init(CCPPUnit* pcUnit);
	void			Kill(void);

	bool			Value(CCPPSyntax* pcExpression, SCPP65Type* psType, int64* plliValue);
	char*			GetReason(void);

	void			NoteFunction(const char* szLabel, CCPPFunction* pcFunction);
	void			NewName(const char* szPrefix, CChars* pszName);

	CIRFunction*	ConstantFunction(const char* szName, CIRModule** ppcModule) override;
	CIRGlobal*		ConstantGlobal(const char* szName, CIRModule** ppcModule) override;

protected:
	CCPP65IRFrontEnd*		BeginWriting(void);
	void					EndWriting(void);
	SCPPConstantFunction*	FindFunction(const char* szLabel);
	bool					IsConstexpr(CCPPFunction* pcFunction);
};


#endif // __C_P_P_CONSTANTS_H__

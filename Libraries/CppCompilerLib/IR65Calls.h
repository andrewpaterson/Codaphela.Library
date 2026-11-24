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
#ifndef __I_R65_CALLS_H__
#define __I_R65_CALLS_H__
#include "IRBackend.h"


class CIR65Backend;


//A call, for the 65C64: the arguments pushed or loaded into A, B and Y as the function called takes them, the
//call itself, and the stack put right after it; a call of a runtime routine (CallRuntime).  And the callee's side
//of the register rule: a register function's plain label is a stub that loads the registers from the stack
//(RegisterStub).
class CIR65Calls
{
protected:
	CIR65Backend*	mpcBackend;

public:
	void	Init(CIR65Backend* pcBackend);
	void	Kill(void);

	bool				Call(CIRBlock* pcBlock, size uiIndex);
	size				CallArguments(CIRBlock* pcBlock, size uiCall);
	int32				PushArguments(CIRBlock* pcBlock, size uiFirst, size uiEnd, CArrayIRRegisterArguments* pasArguments);
	void				CalleeOperand(SIRReference sCallee, bool bThisCall, CChars* pszCallee);
	void				LoadRegisterArguments(CIRBlock* pcBlock, size uiFirst, bool bThisCall, CArrayIRRegisterArguments* pasArguments);
	uint8				ReadsRegisters(SIRReference sValue);
	void				CallerCleanUp(int32 iBytes, SIRInstruction* psCall);
	void				RemoveArguments(int32 iBytes);
	void				CallRuntime(const char* szName, int iBytes);

	void				RegisterStub(const char* szBody);
};


#endif // __I_R65_CALLS_H__

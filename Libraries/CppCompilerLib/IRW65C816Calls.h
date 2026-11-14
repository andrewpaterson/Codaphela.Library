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
#ifndef __I_R_W65_C816_CALLS_H__
#define __I_R_W65_C816_CALLS_H__
#include "IRBackend.h"


class CIRW65C816Backend;


//A call, for the W65C816: the arguments pushed or loaded into A, X and Y as the function called takes them, the
//call itself, and the stack put right after it.
class CIRW65C816Calls
{
protected:
	CIRW65C816Backend*	mpcBackend;

public:
	void	Init(CIRW65C816Backend* pcBackend);
	void	Kill(void);

	void			Call(CIRBlock* pcBlock, SIRInstruction* psInstruction);
	int32			PushArguments(CIRBlock* pcBlock, SIRInstruction* psInstruction, CArrayIRRegisterArguments* pasArguments);
	void			LoadRegisterArguments(CIRBlock* pcBlock, SIRInstruction* psInstruction, CArrayIRRegisterArguments* pasArguments);
	void			LoadRegisters(SIRReference* asValues, int* aiWords, int32* aiHidden, int iRegisters);
	bool			LoadsRegisterDirectly(SIRReference sValue, int iWord);
	void			LoadRegisterWord(int iRegister, SIRReference sValue, int iWord, int32 iHiddenResult);
	void			CallerCleanUp(int32 iBytes, SIRInstruction* psInstruction);
};


#endif // __I_R_W65_C816_CALLS_H__

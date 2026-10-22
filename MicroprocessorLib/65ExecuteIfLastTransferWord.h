/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __65_EXECUTE_IF_LAST_TRANSFER_WORD_H__
#define __65_EXECUTE_IF_LAST_TRANSFER_WORD_H__
#include "65C64Func.h"
#include "65Operation.h"


//Calls a processor method after the last read word is read.
class C65ExecuteIfLastTransferWord : public C65Operation
{
CONSTRUCTABLE(C65ExecuteIfLastTransferWord);
protected:
	C65C64Func	mfFunction;
	uint8		muiWord;
public:
	void	Init(uint8 uiWord, C65C64Func fFunction);

	void	Execute(C65C64* pcCPU) override;
	bool	IsInitialSide(void) override;
	void	Print(CChars* psz) override;
};


#endif // __65_EXECUTE_IF_LAST_TRANSFER_WORD_H__


/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __I_R_S_S_A_UPDATER_H__
#define __I_R_S_S_A_UPDATER_H__
#include "IRFunction.h"


//Puts a value back into SSA form when a pass has given it several definitions, one per block at the block's end
//(Braun, Buchwald, Hack, Leissa, Mallon and Zwinkau, "Simple and Efficient Construction of Static Single Assignment
//Form").  A block sees its only predecessor's value, or a phi of its predecessors' made the first time it is asked
//for (it may be trivial: RemoveTrivialPhis takes it out).  The function's predecessors must be computed.
class CIRSSAUpdater
{
protected:
	CIRFunction*		mpcFunction;
	SIRType				msType;
	CArrayIRReferences	masEnd;			//Per block index: the value defined in the block, IRNone for none.
	CArrayIRReferences	masStart;		//Per block index: the value at its start once worked out, IRNone until then.

public:
	void			Init(CIRFunction* pcFunction);
	void			Kill(void);

	void			Begin(SIRType sType);
	void			Define(CIRBlock* pcBlock, SIRReference sValue);
	SIRReference	ValueAtStart(CIRBlock* pcBlock);
	SIRReference	ValueAtEnd(CIRBlock* pcBlock);
	bool			IsUnreachedCycle(CIRBlock* pcBlock);
};


#endif // __I_R_S_S_A_UPDATER_H__

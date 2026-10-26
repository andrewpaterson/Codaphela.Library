/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela GerberLib

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
#ifndef __W65C816_H__
#define __W65C816_H__
#include "W65CCore.h"
#include "W65C816Pins.h"


//The W65C816S.  Native mode, 16 bit registers, banks, the direct page and the
//instructions the W65C02S does not have.
class CW65C816 : public CW65CCore
{
protected:
	CW65C816Pins*		mpcW65C816Pins;

public:
	void				Init(CW65C816Pins* pcPins);

	CW65C816Pins*		GetPins(void);

	using				CW65CCore::GetInstruction;
	CW65Instruction*	GetInstruction(uint16 uiOpcode) override;
	CW65Instruction*	GetResetOpcode(void) override;
	CW65Instruction*	GetIrqOpcode(void) override;
	CW65Instruction*	GetNmiOpcode(void) override;
	CW65Instruction*	GetAbortOpcode(void) override;
	CW65Instruction*	GetFetchNextOpcode(void) override;
	void				WriteSignalsPhi2Falling(CTimeline* pcTimeline, CW65DataOperation* pcDataOperation, CW65BusCycle* pcBusCycle) override;
	void				WriteSignalsPhi2Rising(CTimeline* pcTimeline) override;
	bool				IsReadCycle(CW65DataOperation* pcDataOperation) override;
	char*				GetType(void) override;

	void				PER(void);
	void				PLD(void);
	void				PHD(void);
	void				PLB(void);
	void				PHB(void);
	void				PHK(void);
	void				COP(void);
	void				TCS(void);
	void				TSC(void);
	void				WDM(void);
	void				MVP(void);
	void				MVN(void);
	void				TCD(void);
	void				TDC(void);
	void				TYX(void);
	void				TXY(void);
	void				REP(void);
	void				SEP(void);
	void				XBA(void);
	void				XCE(void);
	void				BlockMoveNext(void);
	void				BlockMovePrevious(void);

	//Only for tests.
	void				TestSetDirectPage(uint16 uiDirectPage);
	void				TestSetDataBank(uint8 uiDataBank);
};


#endif //  __W65C816_H__


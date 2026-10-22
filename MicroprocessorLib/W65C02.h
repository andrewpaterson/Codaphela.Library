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
#ifndef __W65C02_H__
#define __W65C02_H__
#include "W65CCore.h"
#include "W65C02Pins.h"


//The W65C02S.  It shares its machinery with the W65C816S and always runs as a W65C816S
//in emulation mode.  It adds the Rockwell bit instructions (RMB, SMB, BBR, BBS), SYNC
//and SOB.
class CW65C02 : public CW65CCore
{
protected:
	CW65C02Pins*	mpcW65C02Pins;
	bool			mbBitBranch;
	bool			mbPreviousSOB;

public:
	void			Init(CW65C02Pins* pcPins);

	CW65C02Pins*	GetPins(void);

	using			CW65CCore::GetInstruction;
	CInstruction*	GetInstruction(uint16 uiOpcode) override;
	CInstruction*	GetResetOpcode(void) override;
	CInstruction*	GetIrqOpcode(void) override;
	CInstruction*	GetNmiOpcode(void) override;
	CInstruction*	GetAbortOpcode(void) override;
	CInstruction*	GetFetchNextOpcode(void) override;
	void			WriteSignalsPhi2Falling(CTimeline* pcTimeline, CDataOperation* pcDataOperation, CBusCycle* pcBusCycle) override;
	char*			GetType(void) override;

	void			RMB(void);
	void			SMB(void);
	void			BitBranchTest(void);
	void			BitBranch(void);
	void			Execute8BitBCDSBC(void) override;

protected:
	uint8			GetOpcodeBit(void);
};


#endif //  __W65C02_H__


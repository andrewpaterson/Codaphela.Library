/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __NUMBER_CONTROL_H__
#define __NUMBER_CONTROL_H__
#include "ScratchPad.h"
#include "Numbers.h"
#include "Number.h"


class CNumberControl
{
public:
	CScratchPad			mcScratchPad;
	CNumber				mc_pi;
	CNumber				mc_e;
	UNumber(1, 255)		mc_pi_lots;
	UNumber(1, 255)		mc_e_lots;

	void 		Init(int iScratchPadSize);
	void 		Kill(void);
	void 		Pi(CNumber* pcDest);
	void 		Pi(CNumber* pcDest, short cMaxWholeNumbers, short cMaxDecimals);
	void 		E(CNumber* pcDest);
	void 		E(CNumber* pcDest, short cMaxWholeNumbers, short cMaxDecimals);
	CNumber*	Add(short cMaxWholeNumbers, short cMaxDecimals);
	void		Remove(void);
	void		Remove(int iNumToRemove);
};


extern CNumberControl gcNumberControl;


void NumberInit(int iScratchPadSize = 1 MB);
void NumberKill(void);


#endif //  __NUMBER_CONTROL_H__



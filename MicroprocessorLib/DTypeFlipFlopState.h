/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela MicroprocessorLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MicroprocessorLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MicroprocessorLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __D_TYPE_FLIP_FLOP_STATE_H__
#define __D_TYPE_FLIP_FLOP_STATE_H__
#include "State.h"


//Logicim's DTypeFlipFlopState.  SetState returns true if the state changed.
class CDTypeFlipFlopState : public CState
{
CONSTRUCTABLE(CDTypeFlipFlopState);
protected:
	bool	mbState;

public:
	void	Init(bool bState);
	bool	GetState(void);
	bool	SetState(bool bState);
};


#endif // __D_TYPE_FLIP_FLOP_STATE_H__


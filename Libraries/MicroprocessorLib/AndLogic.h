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
#ifndef __AND_LOGIC_H__
#define __AND_LOGIC_H__
#include "LogicPort.h"


//Logicim's AndLogic: any low input drives the output low.  Otherwise an undriven input makes the output unknown, so it
//stops driving, and the unknown spreads to what it drives; Logicim ignores undriven inputs and keeps the output.
//Otherwise any high input drives it high.  Unsettled inputs, slewing between levels, are ignored; with only them the
//output is left alone.
class CAndLogic
{
public:
			void	InputTransition(CTimeline* pcTimeline, CArrayLogicPortPtr* papcInputs, CLogicPort* pcOutput);

protected:
	virtual bool	TransformOutput(bool bValue);
};


//Logicim's NandLogic.
class CNandLogic : public CAndLogic
{
protected:
	virtual bool	TransformOutput(bool bValue);
};


#endif // __AND_LOGIC_H__


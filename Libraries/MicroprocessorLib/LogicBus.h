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
#ifndef __LOGIC_BUS_H__
#define __LOGIC_BUS_H__
#include "BaseLib/PrimitiveTypes.h"


//A bus a chip's pins drive and read as a whole, see CLogicSignal.  The value is truncated to the bus's width.
//CMetaBus8, CMetaBus16 and CMetaBus32 are ideal buses; CLogicPortBus is a CLogicPort per bit.
class CTimeline;
class CLogicBus
{
public:
	virtual void			Write(CTimeline* pcTimeline, uint32 uiValue) =0;
	virtual void			WriteImpedance(CTimeline* pcTimeline) =0;
	virtual uint32			Read(CTimeline* pcTimeline) =0;
	virtual size			GetWidth(void) =0;
};


#endif // __LOGIC_BUS_H__


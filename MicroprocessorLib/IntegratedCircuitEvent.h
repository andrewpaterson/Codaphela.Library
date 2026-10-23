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
#ifndef __INTEGRATED_CIRCUIT_EVENT_H__
#define __INTEGRATED_CIRCUIT_EVENT_H__
#include "Event.h"


//Logicim's IntegratedCircuitEvent: an event a chip schedules for itself.  The chip keeps it in its list of events
//from Init until it executes or is removed.
class CIntegratedCircuit;
class CIntegratedCircuitEvent : public CEvent
{
CONSTRUCTABLE(CIntegratedCircuitEvent);
protected:
	CIntegratedCircuit*		mpcIntegratedCircuit;

public:
			void				Init(int64 iTime, CIntegratedCircuit* pcIntegratedCircuit);
			void				Init(int64 iTime, int64 iID, CIntegratedCircuit* pcIntegratedCircuit);

	virtual void				RemoveFromOwner(void);
			CIntegratedCircuit*	GetIntegratedCircuit(void);
};


#endif // __INTEGRATED_CIRCUIT_EVENT_H__


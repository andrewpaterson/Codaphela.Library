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
#ifndef __PULL_PORT_H__
#define __PULL_PORT_H__
#include "Port.h"


//A pull's port on the trace it pulls (see CPull): it drives the voltage its supply port reads, weakly, through its
//resistance, so the trace's strong drivers win over it (CTrace::GetVoltage).
class CPowerInPort;
class CPullPort : public CPort
{
CONSTRUCTABLE(CPullPort);
protected:
	CPowerInPort*	mpcSupply;
	float32			mfResistance;

public:
			void		Init(const char* szName, CPortHolder* pcHolder, CPowerInPort* pcSupply, float32 fResistance);

	virtual float32		GetVoltageOut(int64 iTime);
	virtual float32		GetDrive(int64 iTime, float32* pfResistance);
			float32		GetResistance(void) const;
};


#endif // __PULL_PORT_H__


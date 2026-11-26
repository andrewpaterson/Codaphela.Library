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
#ifndef __PULL_H__
#define __PULL_H__
#include "Passive.h"
#include "PowerInPort.h"
#include "PullPort.h"


#define PULL_PIN_SUPPLY		"Supply"
#define PULL_PIN_SIGNAL		"Signal"


//A pull-up or pull-down: a resistor from a supply (its Supply port) to a signal (its Signal port, a CPullPort), which
//gives the signal the supply's voltage while nothing drives it strongly.  When the supply moves, as it ramps, the
//signal's ports are told if only pulls hold it; the voltage they are told it moved from is the supply's last, which
//is the signal's when this is the only pull on it.
class CPull : public CPassive
{
CONSTRUCTABLE(CPull);
protected:
	CPowerInPort*	mpcSupplyPort;
	CPullPort*		mpcSignalPort;
	float32			mfSupplyVoltage;  //The supply's voltage when the signal's ports were last told.

public:
			void			Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, float32 fResistance);
			void			Class(void) override;

	virtual const char*		GetType(void);
	virtual void			SimulationStarted(CTimeline* pcTimeline);
	virtual void			TraceConnected(CTimeline* pcTimeline, CPort* pcPort);

			CPowerInPort*	GetSupplyPort(void) const;
			CPullPort*		GetSignalPort(void) const;
};


#endif // __PULL_H__


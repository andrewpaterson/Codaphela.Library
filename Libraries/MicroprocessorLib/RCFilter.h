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
#ifndef __RC_FILTER_H__
#define __RC_FILTER_H__
#include "IntegratedCircuit.h"
#include "RCFilterPins.h"


#define RC_FILTER_TYPE		"RC Filter"


//An RC element: a resistor from its input to its output and a capacitor from its output to ground, so its output
//follows its input's voltage along RC curves (CLogicPort::WriteCurve), V(t) = Vin + (Vstart - Vin) e^(-t / RC).  A
//pulse on the input shorter than the curve's time to an input's threshold on the output changes nothing that reads
//it: the curve turns back before it gets there, and its pending transition then reads the level it had.  Rising and
//falling have their own time constants, as a diode across the resistor charges the capacitor quickly one way.  With
//its input undriven the capacitor holds its voltage.  It is passive: it needs no power, and what it remembers is its
//output's curve.
class CRCFilter : public CIntegratedCircuit
{
CONSTRUCTABLE(CRCFilter);
protected:
	int64	miRiseTimeConstant;
	int64	miFallTimeConstant;

public:
			void				Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CRCFilterPins> pPins, int64 iRiseTimeConstant, int64 iFallTimeConstant);
			void				Class(void) override;
	virtual void				SimulationStarted(CTimeline* pcTimeline);
	virtual void				InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual bool				NeedsPower(void) const;
	virtual const char*			GetType(void);
			void				SetTimeConstants(int64 iRiseTimeConstant, int64 iFallTimeConstant);
			int64				GetRiseTimeConstant(void) const;
			int64				GetFallTimeConstant(void) const;
			CRCFilterPins*		GetRCFilterPins(void);
};


#endif // __RC_FILTER_H__


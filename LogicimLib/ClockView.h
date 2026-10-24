/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __CLOCK_VIEW_H__
#define __CLOCK_VIEW_H__
#include "IntegratedCircuitView.h"


#define CLOCK_WAVEFORM_LINES	5


//A clock oscillator, as Logicim's ClockView: a two grid unit box with a square wave in it and its output on top.  An
//inverse output makes the box a grid unit wider with the inverted output on top of the extra unit.  The wave is drawn
//in the colour of the clock's own voltage.  (Logicim moved the wave of a clock with an inverse output off its box when
//the clock faced North or South; here it's always in the middle of the box.)
class CClockView : public CIntegratedCircuitView
{
CONSTRUCTABLE(CClockView);
protected:
	float64		mfFrequency;
	bool		mbInverseOut;
	size		muiFirstWaveformLine;

public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, float64 fFrequency, bool bInverseOut);

	virtual void					Paint(CCircuitPainter* pcPainter);

	virtual const char*				GetType(void);
	virtual const char*				GetXMLName(void);
	virtual bool					LoadXML(CMarkupTag* pcTag);
	virtual CIntegratedCircuit*		CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily);

			float64					GetFrequency(void);
			bool					IsInverseOut(void);

protected:
	virtual	void					SaveXMLAttributes(CXMLWriter* pcWriter);
};


#endif // __CLOCK_VIEW_H__

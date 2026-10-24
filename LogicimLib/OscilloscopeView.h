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
#ifndef __OSCILLOSCOPE_VIEW_H__
#define __OSCILLOSCOPE_VIEW_H__
#include "IntegratedCircuitView.h"


//An oscilloscope, as Logicim's OscilloscopeView: a cyan frame round a white screen with an input per trace down its
//left edge, iDivHeight grid units apart.  Each trace is drawn from its input across iNumberOfDivsWide grid units, a
//sample bar from the lowest to the highest voltage seen in each sample (8V is iDivHeight up), with the tick line where
//the next sample goes; the samples just past the tick, the oldest, fade to white.
//
//Logicim kept a colour per sample; the C++ oscilloscope keeps only voltages, so a bar is coloured by the middle of its
//voltages.  Logicim drew the samples unturned; here they turn with the oscilloscope.
class COscilloscopeView : public CIntegratedCircuitView
{
CONSTRUCTABLE(COscilloscopeView);
protected:
	int32		miInputCount;
	int32		miNumberOfDivsWide;
	int32		miSamplesPerDiv;
	int32		miDivHeight;
	float64		mfSampleFrequency;

public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, int32 iInputCount, int32 iNumberOfDivsWide, int32 iSamplesPerDiv, int32 iDivHeight, float64 fSampleFrequency);

	virtual void					Paint(CCircuitPainter* pcPainter);

	virtual const char*				GetType(void);
	virtual const char*				GetXMLName(void);
	virtual bool					LoadXML(CMarkupTag* pcTag);
	virtual CIntegratedCircuit*		CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily);

			int32					GetInputCount(void);
			int32					GetNumberOfDivsWide(void);
			int32					GetSamplesPerDiv(void);
			int32					GetDivHeight(void);
			float64					GetSampleFrequency(void);

protected:
	virtual	void					SaveXMLAttributes(CXMLWriter* pcWriter);
			void					PaintSamples(CCircuitPainter* pcPainter);
};


#endif // __OSCILLOSCOPE_VIEW_H__

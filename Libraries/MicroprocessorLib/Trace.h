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
#ifndef __TRACE_H__
#define __TRACE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "Port.h"
#include "PortOutput.h"


//A trace's capacitance unless whoever builds the circuit knows better: a few centimetres of track and its pads.
#define TRACE_CAPACITANCE	3e-12f


//Logicim's Trace: an analog wire.  Its voltage is the average of what the ports connected to it drive it with, NaN
//when nothing drives it.  Only strong drives count while there are any; with none its weak drives (pulls, see
//CPort::GetDrive) set it, sharing by conductance as a divider does.  When it stops being driven its ports are told,
//see NotifyUndriven.  It doesn't own its ports.  Every trace gets the next id unless it is given one.
//
//Let go by its last strong driver, or its pulls' level moving, a weakly held trace doesn't jump: it curves from where
//it was to its pulls' level, an RC curve of the pulls' resistance in parallel and its capacitance (SetCapacitance), and
//its logic ports are told as for a slew, so each transitions where the curve crosses its threshold (see
//NotifyVoltageChanged).  A strong driver taking it ends the curve.
class CTrace
{
protected:
	CArrayPortPtr	mapcConnectedPorts;
	int64			miID;
	float32			mfCapacitance;  //Farads.
	CPortOutput		mcWeakCurve;  //None while there is no curve.

public:
	void			Init(void);
	void			Init(int64 iID);
	void			Kill(void);

	float32			GetVoltage(int64 iTime);
	float32			GetShortVoltage(int64 iTime);
	float32			GetMinimumVoltage(int64 iTime);
	float32			GetMaximumVoltage(int64 iTime);
	bool			HasStrongDriver(int64 iTime);
	float32			GetWeakVoltage(int64 iTime, float32* pfResistance);
	void			SetCapacitance(float32 fCapacitance);
	float32			GetCapacitance(void) const;
	CPortOutput*	GetWeakCurve(void);
	void			StopWeakCurve(void);
	bool			HasLogicDriver(int64 iTime);
	void			AppendVoltage(CChars* psz, int64 iTime);

	void			Connect(CPort* pcPort);
	void			NotifyUndriven(CTimeline* pcTimeline, CPort* pcExcept);
	void			NotifyVoltageChanged(CTimeline* pcTimeline, float32 fOldVoltage, CPort* pcExcept);
	void			Disconnect(CPort* pcPort);
	size			NumConnectedPorts(void) const;
	CPort*			GetConnectedPort(size uiIndex);
	int64			GetID(void) const;

	static int64	GetID(CTrace* pcTrace);
	static void		ResetNextID(void);
};


typedef CArrayTemplatePtr<CTrace>	CArrayTracePtr;


#endif // __TRACE_H__


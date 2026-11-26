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
#ifndef __PORT_OUTPUT_H__
#define __PORT_OUTPUT_H__
#include "BaseLib/PrimitiveTypes.h"


enum EPortOutput
{
	PO_None,
	PO_Drive,
	PO_Slew,
};


//The time constants an RC curve slews over before it is taken to have arrived: within e^-5, under 1%, of its end.
#define RC_CURVE_TIME_CONSTANTS		5


//What a CLogicPort is driving its trace with: nothing, a fixed voltage from a CDriveEvent or a slew from a
//CSlewEvent.  Logicim's LogicPort keeps its last executed output event; here the timeline frees an event once it has
//executed, so the port keeps a copy of what it was.
//
//A slew is a straight line from its start to its end voltage over its slew time, or, with a time constant, an RC
//curve: a capacitor charging towards its end voltage, V(t) = Vend + (Vstart - Vend) e^(-t / RC), which never quite
//arrives, so its slew time is RC_CURVE_TIME_CONSTANTS of them and the drive at its end takes it the last step.
//CalculateTimeAtVoltage is when either shape reaches a voltage, as an input's threshold.
class CPortOutput
{
protected:
	EPortOutput	meType;
	int64		miTime;
	float32		mfStartVoltage;
	float32		mfEndVoltage;  //The drive voltage of a drive.
	int64		miSlewTime;
	int64		miTimeConstant;  //0 for a straight line.

public:
	void		Init(void);
	void		InitDrive(int64 iTime, float32 fVoltage);
	void		InitSlew(int64 iTime, float32 fStartVoltage, float32 fEndVoltage, int64 iSlewTime, int64 iTimeConstant = 0);

	float32		GetVoltage(int64 iTime);
	float32		CalculateVoltageAtTime(int64 iTime, float32 fStartVoltage);
	int64		CalculateTimeAtVoltage(float32 fStartVoltage, float32 fVoltage);
	bool		ForTime(int64 iTime);
	int64		GetEndTime(void) const;

	bool		IsNone(void) const;
	bool		IsDrive(void) const;
	bool		IsSlew(void) const;
	int64		GetTime(void) const;
	float32		GetStartVoltage(void) const;
	float32		GetEndVoltage(void) const;
	int64		GetSlewTime(void) const;
	int64		GetTimeConstant(void) const;
	bool		IsCurve(void) const;
};


#endif // __PORT_OUTPUT_H__


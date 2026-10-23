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


//What a CLogicPort is driving its trace with: nothing, a fixed voltage from a CDriveEvent or a slew from a
//CSlewEvent.  Logicim's LogicPort keeps its last executed output event; here the timeline frees an event once it has
//executed, so the port keeps a copy of what it was.
class CPortOutput
{
protected:
	EPortOutput	meType;
	int64		miTime;
	float32		mfStartVoltage;
	float32		mfEndVoltage;  //The drive voltage of a drive.
	int64		miSlewTime;

public:
	void		Init(void);
	void		InitDrive(int64 iTime, float32 fVoltage);
	void		InitSlew(int64 iTime, float32 fStartVoltage, float32 fEndVoltage, int64 iSlewTime);

	float32		GetVoltage(int64 iTime);
	float32		CalculateVoltageAtTime(int64 iTime, float32 fStartVoltage);
	bool		ForTime(int64 iTime);
	int64		GetEndTime(void);

	bool		IsNone(void);
	bool		IsDrive(void);
	bool		IsSlew(void);
	int64		GetTime(void);
	float32		GetStartVoltage(void);
	float32		GetEndVoltage(void);
	int64		GetSlewTime(void);
};


#endif // __PORT_OUTPUT_H__


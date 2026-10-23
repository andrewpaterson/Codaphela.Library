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
#ifndef __VOLTAGE_CONFIGURATION_SOURCE_H__
#define __VOLTAGE_CONFIGURATION_SOURCE_H__
#include "TraceValue.h"
#include "VoltageConfiguration.h"


//Logicim's VoltageConfigurationSource: the input thresholds, output voltages and timings of a logic family at a
//supply voltage, VCC.  See CFamilyVoltageConfiguration and CFixedVoltageConfiguration.
class CVoltageConfigurationSource
{
public:
	virtual CVoltageConfiguration*	Get(float32 fVcc) =0;

	virtual float32					GetLowVoltageIn(float32 fVcc) =0;
	virtual float32					GetHighVoltageIn(float32 fVcc) =0;
	virtual float32					GetVoltageOut(bool bValue, float32 fVcc) =0;
	virtual ETraceValue				GetValue(float32 fVoltage, float32 fVcc) =0;
	virtual float32					CalculateStartVoltage(float32 fPortVoltage, float32 fVcc) =0;
	virtual float32					GetVoltsPerTimeLowToHigh(float32 fVcc) =0;
	virtual float32					GetVoltsPerTimeHighToLow(float32 fVcc) =0;
	virtual int64					CalculateHoldTime(float32 fOutVoltage, float32 fPortVoltage, float32 fVcc) =0;
};


#endif // __VOLTAGE_CONFIGURATION_SOURCE_H__


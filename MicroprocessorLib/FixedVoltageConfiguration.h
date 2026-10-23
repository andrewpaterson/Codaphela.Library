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
#ifndef __FIXED_VOLTAGE_CONFIGURATION_H__
#define __FIXED_VOLTAGE_CONFIGURATION_H__
#include "VoltageConfigurationSource.h"


//Logicim's TestVoltageConfiguration: one configuration whatever the VCC, for tests and simple circuits.
class CFixedVoltageConfiguration : public CVoltageConfigurationSource
{
protected:
	CVoltageConfiguration	mcConfiguration;

public:
			void					Init(CVoltageConfiguration* pcConfiguration);
			bool					Init(float32 fVcc, float32 fLowVoltageIn, float32 fHighVoltageIn, float32 fLowVoltageOut, float32 fHighVoltageOut, int64 iHighToLowPropagation, int64 iLowToHighPropagation);
			void					Kill(void);

	virtual CVoltageConfiguration*	Get(float32 fVcc);
	virtual float32					GetLowVoltageIn(float32 fVcc);
	virtual float32					GetHighVoltageIn(float32 fVcc);
	virtual float32					GetVoltageOut(bool bValue, float32 fVcc);
	virtual ETraceValue				GetValue(float32 fVoltage, float32 fVcc);
	virtual float32					CalculateStartVoltage(float32 fPortVoltage, float32 fVcc);
	virtual float32					GetVoltsPerTimeLowToHigh(float32 fVcc);
	virtual float32					GetVoltsPerTimeHighToLow(float32 fVcc);
	virtual int64					CalculateHoldTime(float32 fOutVoltage, float32 fPortVoltage, float32 fVcc);
};


#endif // __FIXED_VOLTAGE_CONFIGURATION_H__


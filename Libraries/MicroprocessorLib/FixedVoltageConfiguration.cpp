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
#include "FixedVoltageConfiguration.h"


//////////////////////////////////////////////////////////////////////////
//
// Copies pcConfiguration.
//
//////////////////////////////////////////////////////////////////////////
void CFixedVoltageConfiguration::Init(CVoltageConfiguration* pcConfiguration)
{
	mcConfiguration = *pcConfiguration;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFixedVoltageConfiguration::Init(float32 fVcc, float32 fLowVoltageIn, float32 fHighVoltageIn, float32 fLowVoltageOut, float32 fHighVoltageOut, int64 iHighToLowPropagation, int64 iLowToHighPropagation)
{
	return mcConfiguration.Init(fVcc, fLowVoltageIn, fHighVoltageIn, fLowVoltageOut, fHighVoltageOut, iHighToLowPropagation, iLowToHighPropagation);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFixedVoltageConfiguration::Kill(void)
{
	mcConfiguration.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The configuration is the same at every VCC.
//
//////////////////////////////////////////////////////////////////////////
CVoltageConfiguration* CFixedVoltageConfiguration::Get(float32 fVcc) { return &mcConfiguration; }
float32 CFixedVoltageConfiguration::GetLowVoltageIn(float32 fVcc) { return mcConfiguration.GetLowVoltageIn(); }
float32 CFixedVoltageConfiguration::GetHighVoltageIn(float32 fVcc) { return mcConfiguration.GetHighVoltageIn(); }
float32 CFixedVoltageConfiguration::GetVoltageOut(bool bValue, float32 fVcc) { return mcConfiguration.GetVoltageOut(bValue); }
ETraceValue CFixedVoltageConfiguration::GetValue(float32 fVoltage, float32 fVcc) { return mcConfiguration.GetValue(fVoltage); }
float32 CFixedVoltageConfiguration::CalculateStartVoltage(float32 fPortVoltage, float32 fVcc) { return mcConfiguration.CalculateStartVoltage(fPortVoltage); }
float32 CFixedVoltageConfiguration::GetVoltsPerTimeLowToHigh(float32 fVcc) { return mcConfiguration.GetVoltsPerTimeLowToHigh(); }
float32 CFixedVoltageConfiguration::GetVoltsPerTimeHighToLow(float32 fVcc) { return mcConfiguration.GetVoltsPerTimeHighToLow(); }
int64 CFixedVoltageConfiguration::CalculateHoldTime(float32 fOutVoltage, float32 fPortVoltage, float32 fVcc) { return mcConfiguration.CalculateHoldTime(fOutVoltage, fPortVoltage); }


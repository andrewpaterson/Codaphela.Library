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
#ifndef __VOLTAGE_CONFIGURATION_H__
#define __VOLTAGE_CONFIGURATION_H__
#include "BaseLib/PrimitiveTypes.h"
#include "TraceValue.h"


//Logicim's VoltageConfiguration: a logic family at one supply voltage, VCC.  Inputs at or below the low voltage in
//read low, at or above the high voltage in read high and between them unsettled.  An output changing level holds its
//old voltage for the hold time then slews to the new one over the slew time.  Times are in 1/1024ths of a
//nanosecond, see LongTime.h.
//
//Changing high to low and low to high have their own times, as do disabling and enabling an output, from either
//level.  Given only propagation times the hold is half the propagation time and the slew the whole of it.
//
//Logicim throws when the voltages or slew times are invalid; here Init logs an error and returns false.  Logicim's
//full constructor takes the high voltage out before the low; Init takes them low then high, as the other two do.
class CVoltageConfiguration
{
protected:
	float32		mfVcc;

	float32		mfLowVoltageIn;
	float32		mfHighVoltageIn;

	float32		mfLowVoltageOut;
	float32		mfHighVoltageOut;

	int64		miHighToLowHoldTime;
	int64		miHighToLowSlewTime;
	int64		miLowToHighHoldTime;
	int64		miLowToHighSlewTime;

	int64		miHighToDisableHoldTime;
	int64		miHighToDisableSlewTime;
	int64		miLowToDisableHoldTime;
	int64		miLowToDisableSlewTime;

	int64		miHighToEnableHoldTime;
	int64		miHighToEnableSlewTime;
	int64		miLowToEnableHoldTime;
	int64		miLowToEnableSlewTime;

public:
	bool		Init(float32 fVcc, float32 fLowVoltageIn, float32 fHighVoltageIn, float32 fLowVoltageOut, float32 fHighVoltageOut, int64 iHighToLowPropagation, int64 iLowToHighPropagation);
	bool		Init(float32 fVcc, float32 fLowVoltageIn, float32 fHighVoltageIn, float32 fLowVoltageOut, float32 fHighVoltageOut, int64 iHighToLowPropagation, int64 iLowToHighPropagation, int64 iHighToDisEnPropagation, int64 iLowToDisEnPropagation);
	bool		Init(float32 fVcc, float32 fLowVoltageIn, float32 fHighVoltageIn, float32 fLowVoltageOut, float32 fHighVoltageOut,
					 int64 iHighToLowHoldTime, int64 iHighToLowSlewTime, int64 iLowToHighHoldTime, int64 iLowToHighSlewTime,
					 int64 iHighToDisableHoldTime, int64 iHighToDisableSlewTime, int64 iLowToDisableHoldTime, int64 iLowToDisableSlewTime,
					 int64 iHighToEnableHoldTime, int64 iHighToEnableSlewTime, int64 iLowToEnableHoldTime, int64 iLowToEnableSlewTime);
	void		Kill(void);

	ETraceValue	GetValue(float32 fVoltage);
	float32		GetVcc(void);
	float32		GetLowVoltageIn(void);
	float32		GetHighVoltageIn(void);
	float32		GetLowVoltageOut(void);
	float32		GetHighVoltageOut(void);
	float32		GetVoltageOut(bool bValue);

	int64		CalculateHoldTime(float32 fOutVoltage, float32 fPortVoltage);
	float32		CalculateStartVoltage(float32 fPortVoltage);
	float32		GetVoltsPerTimeLowToHigh(void);
	float32		GetVoltsPerTimeHighToLow(void);
	int64		GetAverageDelayTime(void);

	int64		GetHighToLowHoldTime(void);
	int64		GetHighToLowSlewTime(void);
	int64		GetLowToHighHoldTime(void);
	int64		GetLowToHighSlewTime(void);
	int64		GetHighToDisableHoldTime(void);
	int64		GetHighToDisableSlewTime(void);
	int64		GetLowToDisableHoldTime(void);
	int64		GetLowToDisableSlewTime(void);
	int64		GetHighToEnableHoldTime(void);
	int64		GetHighToEnableSlewTime(void);
	int64		GetLowToEnableHoldTime(void);
	int64		GetLowToEnableSlewTime(void);

protected:
	void		SetVoltages(float32 fVcc, float32 fLowVoltageIn, float32 fHighVoltageIn, float32 fLowVoltageOut, float32 fHighVoltageOut);
	void		SetPropagationTime(int64 iHighToLowPropagation, int64 iLowToHighPropagation);
	void		SetEnableTime(int64 iHighToLowEnable, int64 iLowToHighEnable);
	void		SetDisableTime(int64 iHighToLowDisable, int64 iLowToHighDisable);
	bool		IsValid(void);
	float32		GetVoltsPerTime(int64 iTime);
};


int CompareVoltageConfigurationVcc(const void* pvLeft, const void* pvRight);


#endif // __VOLTAGE_CONFIGURATION_H__


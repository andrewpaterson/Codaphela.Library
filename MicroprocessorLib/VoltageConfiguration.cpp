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
#include "BaseLib/Logger.h"
#include "Voltage.h"
#include "VoltageConfiguration.h"


//////////////////////////////////////////////////////////////////////////
//
// Half of iTime rounded half up, as Logicim's Math.round(time / 2.0).
//
//////////////////////////////////////////////////////////////////////////
static int64 HalfTimeRounded(int64 iTime)
{
	if (iTime >= 0)
	{
		return (iTime + 1) / 2;
	}
	else
	{
		return -((-iTime) / 2);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The disable and enable times are 1.2 times the propagation times.
//
//////////////////////////////////////////////////////////////////////////
bool CVoltageConfiguration::Init(float32 fVcc, float32 fLowVoltageIn, float32 fHighVoltageIn, float32 fLowVoltageOut, float32 fHighVoltageOut, int64 iHighToLowPropagation, int64 iLowToHighPropagation)
{
	SetVoltages(fVcc, fLowVoltageIn, fHighVoltageIn, fLowVoltageOut, fHighVoltageOut);
	SetPropagationTime(iHighToLowPropagation, iLowToHighPropagation);

	miHighToDisableHoldTime = (int64)(miHighToLowHoldTime * 1.2f);
	miHighToDisableSlewTime = (int64)(miHighToLowSlewTime * 1.2f);
	miLowToDisableHoldTime = (int64)(miLowToHighHoldTime * 1.2f);
	miLowToDisableSlewTime = (int64)(miLowToHighSlewTime * 1.2f);

	miHighToEnableHoldTime = (int64)(miHighToLowHoldTime * 1.2f);
	miHighToEnableSlewTime = (int64)(miHighToLowSlewTime * 1.2f);
	miLowToEnableHoldTime = (int64)(miLowToHighHoldTime * 1.2f);
	miLowToEnableSlewTime = (int64)(miLowToHighSlewTime * 1.2f);

	return IsValid();
}


//////////////////////////////////////////////////////////////////////////
//
// Disabling and enabling take the same times.
//
//////////////////////////////////////////////////////////////////////////
bool CVoltageConfiguration::Init(float32 fVcc, float32 fLowVoltageIn, float32 fHighVoltageIn, float32 fLowVoltageOut, float32 fHighVoltageOut, int64 iHighToLowPropagation, int64 iLowToHighPropagation, int64 iHighToDisEnPropagation, int64 iLowToDisEnPropagation)
{
	SetVoltages(fVcc, fLowVoltageIn, fHighVoltageIn, fLowVoltageOut, fHighVoltageOut);
	SetPropagationTime(iHighToLowPropagation, iLowToHighPropagation);
	SetDisableTime(iHighToDisEnPropagation, iLowToDisEnPropagation);
	SetEnableTime(iHighToDisEnPropagation, iLowToDisEnPropagation);

	return IsValid();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVoltageConfiguration::Init(float32 fVcc, float32 fLowVoltageIn, float32 fHighVoltageIn, float32 fLowVoltageOut, float32 fHighVoltageOut,
								 int64 iHighToLowHoldTime, int64 iHighToLowSlewTime, int64 iLowToHighHoldTime, int64 iLowToHighSlewTime,
								 int64 iHighToDisableHoldTime, int64 iHighToDisableSlewTime, int64 iLowToDisableHoldTime, int64 iLowToDisableSlewTime,
								 int64 iHighToEnableHoldTime, int64 iHighToEnableSlewTime, int64 iLowToEnableHoldTime, int64 iLowToEnableSlewTime)
{
	SetVoltages(fVcc, fLowVoltageIn, fHighVoltageIn, fLowVoltageOut, fHighVoltageOut);

	miHighToLowHoldTime = iHighToLowHoldTime;
	miHighToLowSlewTime = iHighToLowSlewTime;
	miLowToHighHoldTime = iLowToHighHoldTime;
	miLowToHighSlewTime = iLowToHighSlewTime;

	miHighToDisableHoldTime = iHighToDisableHoldTime;
	miHighToDisableSlewTime = iHighToDisableSlewTime;
	miLowToDisableHoldTime = iLowToDisableHoldTime;
	miLowToDisableSlewTime = iLowToDisableSlewTime;

	miHighToEnableHoldTime = iHighToEnableHoldTime;
	miHighToEnableSlewTime = iHighToEnableSlewTime;
	miLowToEnableHoldTime = iLowToEnableHoldTime;
	miLowToEnableSlewTime = iLowToEnableSlewTime;

	return IsValid();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVoltageConfiguration::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVoltageConfiguration::SetVoltages(float32 fVcc, float32 fLowVoltageIn, float32 fHighVoltageIn, float32 fLowVoltageOut, float32 fHighVoltageOut)
{
	mfVcc = fVcc;

	mfLowVoltageIn = fLowVoltageIn;
	mfHighVoltageIn = fHighVoltageIn;

	mfLowVoltageOut = fLowVoltageOut;
	mfHighVoltageOut = fHighVoltageOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVoltageConfiguration::SetPropagationTime(int64 iHighToLowPropagation, int64 iLowToHighPropagation)
{
	miHighToLowHoldTime = HalfTimeRounded(iHighToLowPropagation);
	miHighToLowSlewTime = iHighToLowPropagation;
	miLowToHighHoldTime = HalfTimeRounded(iLowToHighPropagation);
	miLowToHighSlewTime = iLowToHighPropagation;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVoltageConfiguration::SetEnableTime(int64 iHighToLowEnable, int64 iLowToHighEnable)
{
	miHighToEnableHoldTime = HalfTimeRounded(iHighToLowEnable);
	miHighToEnableSlewTime = iHighToLowEnable;
	miLowToEnableHoldTime = HalfTimeRounded(iLowToHighEnable);
	miLowToEnableSlewTime = iLowToHighEnable;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVoltageConfiguration::SetDisableTime(int64 iHighToLowDisable, int64 iLowToHighDisable)
{
	miHighToDisableHoldTime = HalfTimeRounded(iHighToLowDisable);
	miHighToDisableSlewTime = iHighToLowDisable;
	miLowToDisableHoldTime = HalfTimeRounded(iLowToHighDisable);
	miLowToDisableSlewTime = iLowToHighDisable;
}


//////////////////////////////////////////////////////////////////////////
//
// The high voltages must be above the low ones and every slew time must
// be greater than zero.
//
//////////////////////////////////////////////////////////////////////////
bool CVoltageConfiguration::IsValid(void)
{
	if (!(mfHighVoltageOut > mfLowVoltageOut))
	{
		gcLogger.Error2(__METHOD__, " High voltage out must be greater than low voltage out.", NULL);
		return false;
	}

	if (!(mfHighVoltageIn > mfLowVoltageIn))
	{
		gcLogger.Error2(__METHOD__, " High voltage in must be greater than low voltage in.", NULL);
		return false;
	}

	if ((miHighToLowSlewTime <= 0) || (miLowToHighSlewTime <= 0) ||
		(miHighToDisableSlewTime <= 0) || (miLowToDisableSlewTime <= 0) ||
		(miHighToEnableSlewTime <= 0) || (miLowToEnableSlewTime <= 0))
	{
		gcLogger.Error2(__METHOD__, " Slew time must be greater than zero.", NULL);
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CVoltageConfiguration::GetVoltsPerTime(int64 iTime)
{
	return (mfHighVoltageOut - mfLowVoltageOut) / iTime;
}


//////////////////////////////////////////////////////////////////////////
//
// Undriven when there is no voltage.
//
//////////////////////////////////////////////////////////////////////////
ETraceValue CVoltageConfiguration::GetValue(float32 fVoltage)
{
	if (!IsVoltage(fVoltage))
	{
		return TV_Undriven;
	}
	else if (fVoltage <= mfLowVoltageIn)
	{
		return TV_Low;
	}
	else if (fVoltage >= mfHighVoltageIn)
	{
		return TV_High;
	}
	else
	{
		return TV_Unsettled;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The hold time of a change from the port's voltage to fOutVoltage; a
// port with no voltage starts from the low voltage out.  An output that
// doesn't change voltage holds for the average of the two hold times.
//
//////////////////////////////////////////////////////////////////////////
int64 CVoltageConfiguration::CalculateHoldTime(float32 fOutVoltage, float32 fPortVoltage)
{
	float32	fVoltageDiff;

	fPortVoltage = CalculateStartVoltage(fPortVoltage);
	fOutVoltage = CalculateStartVoltage(fOutVoltage);
	fVoltageDiff = fOutVoltage - fPortVoltage;

	if (fVoltageDiff > 0)
	{
		return miLowToHighHoldTime;
	}
	else if (fVoltageDiff < 0)
	{
		return miHighToLowHoldTime;
	}
	else
	{
		return (miLowToHighHoldTime + miHighToLowHoldTime) / 2;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CVoltageConfiguration::CalculateStartVoltage(float32 fPortVoltage)
{
	if (IsVoltage(fPortVoltage))
	{
		return fPortVoltage;
	}
	else
	{
		return mfLowVoltageOut;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CVoltageConfiguration::GetVoltageOut(bool bValue)
{
	if (bValue)
	{
		return mfHighVoltageOut;
	}
	else
	{
		return mfLowVoltageOut;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CVoltageConfiguration::GetAverageDelayTime(void)
{
	return (miHighToLowHoldTime + miLowToHighHoldTime) / 2 + (miHighToLowSlewTime + miLowToHighSlewTime) / 2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CVoltageConfiguration::GetVoltsPerTimeLowToHigh(void) { return GetVoltsPerTime(miLowToHighSlewTime); }
float32 CVoltageConfiguration::GetVoltsPerTimeHighToLow(void) { return GetVoltsPerTime(miHighToLowSlewTime); }
float32 CVoltageConfiguration::GetVcc(void) { return mfVcc; }
float32 CVoltageConfiguration::GetLowVoltageIn(void) { return mfLowVoltageIn; }
float32 CVoltageConfiguration::GetHighVoltageIn(void) { return mfHighVoltageIn; }
float32 CVoltageConfiguration::GetLowVoltageOut(void) { return mfLowVoltageOut; }
float32 CVoltageConfiguration::GetHighVoltageOut(void) { return mfHighVoltageOut; }
int64 CVoltageConfiguration::GetHighToLowHoldTime(void) { return miHighToLowHoldTime; }
int64 CVoltageConfiguration::GetHighToLowSlewTime(void) { return miHighToLowSlewTime; }
int64 CVoltageConfiguration::GetLowToHighHoldTime(void) { return miLowToHighHoldTime; }
int64 CVoltageConfiguration::GetLowToHighSlewTime(void) { return miLowToHighSlewTime; }
int64 CVoltageConfiguration::GetHighToDisableHoldTime(void) { return miHighToDisableHoldTime; }
int64 CVoltageConfiguration::GetHighToDisableSlewTime(void) { return miHighToDisableSlewTime; }
int64 CVoltageConfiguration::GetLowToDisableHoldTime(void) { return miLowToDisableHoldTime; }
int64 CVoltageConfiguration::GetLowToDisableSlewTime(void) { return miLowToDisableSlewTime; }
int64 CVoltageConfiguration::GetHighToEnableHoldTime(void) { return miHighToEnableHoldTime; }
int64 CVoltageConfiguration::GetHighToEnableSlewTime(void) { return miHighToEnableSlewTime; }
int64 CVoltageConfiguration::GetLowToEnableHoldTime(void) { return miLowToEnableHoldTime; }
int64 CVoltageConfiguration::GetLowToEnableSlewTime(void) { return miLowToEnableSlewTime; }


//////////////////////////////////////////////////////////////////////////
//
// Orders CVoltageConfigurations by their VCC, as Logicim's compareTo.
//
//////////////////////////////////////////////////////////////////////////
int CompareVoltageConfigurationVcc(const void* pvLeft, const void* pvRight)
{
	float32	fLeft;
	float32	fRight;

	fLeft = ((CVoltageConfiguration*)pvLeft)->GetVcc();
	fRight = ((CVoltageConfiguration*)pvRight)->GetVcc();
	if (fLeft < fRight)
	{
		return -1;
	}
	else if (fLeft > fRight)
	{
		return 1;
	}
	return 0;
}


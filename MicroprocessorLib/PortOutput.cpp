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
#include "Voltage.h"
#include "PortOutput.h"


//////////////////////////////////////////////////////////////////////////
//
// Driving nothing.
//
//////////////////////////////////////////////////////////////////////////
void CPortOutput::Init(void)
{
	meType = PO_None;
	miTime = 0;
	mfStartVoltage = NoVoltage();
	mfEndVoltage = NoVoltage();
	miSlewTime = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPortOutput::InitDrive(int64 iTime, float32 fVoltage)
{
	meType = PO_Drive;
	miTime = iTime;
	mfStartVoltage = NoVoltage();
	mfEndVoltage = fVoltage;
	miSlewTime = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPortOutput::InitSlew(int64 iTime, float32 fStartVoltage, float32 fEndVoltage, int64 iSlewTime)
{
	meType = PO_Slew;
	miTime = iTime;
	mfStartVoltage = fStartVoltage;
	mfEndVoltage = fEndVoltage;
	miSlewTime = iSlewTime;
}


//////////////////////////////////////////////////////////////////////////
//
// A drive's voltage from its time on.  A slew's voltage from its time to
// its end time; outside it there is none, the drive at the end of the
// slew takes over.  A slew that never ends, with no power, drives nothing;
// in Logicim its end time overflows, so it never matches a time.
//
//////////////////////////////////////////////////////////////////////////
float32 CPortOutput::GetVoltage(int64 iTime)
{
	if (meType == PO_Drive)
	{
		if (iTime >= miTime)
		{
			return mfEndVoltage;
		}
	}
	else if (meType == PO_Slew)
	{
		if (ForTime(iTime))
		{
			return CalculateVoltageAtTime(iTime, mfStartVoltage);
		}
	}
	return NoVoltage();
}


//////////////////////////////////////////////////////////////////////////
//
// Straight from fStartVoltage at the slew's time to its end voltage at
// its end time, and on beyond it.
//
//////////////////////////////////////////////////////////////////////////
float32 CPortOutput::CalculateVoltageAtTime(int64 iTime, float32 fStartVoltage)
{
	float32	fFractionStart;

	fFractionStart = (iTime - miTime) / (float32)miSlewTime;
	return fFractionStart * mfEndVoltage + (1 - fFractionStart) * fStartVoltage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPortOutput::ForTime(int64 iTime)
{
	if (miSlewTime == MAX_INT64)
	{
		return false;
	}
	return (iTime <= GetEndTime()) && (iTime >= miTime);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CPortOutput::GetEndTime(void)
{
	if (miSlewTime == MAX_INT64)
	{
		return MAX_INT64;
	}
	return miTime + miSlewTime;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPortOutput::IsNone(void) { return meType == PO_None; }
bool CPortOutput::IsDrive(void) { return meType == PO_Drive; }
bool CPortOutput::IsSlew(void) { return meType == PO_Slew; }
int64 CPortOutput::GetTime(void) { return miTime; }
float32 CPortOutput::GetStartVoltage(void) { return mfStartVoltage; }
float32 CPortOutput::GetEndVoltage(void) { return mfEndVoltage; }
int64 CPortOutput::GetSlewTime(void) { return miSlewTime; }


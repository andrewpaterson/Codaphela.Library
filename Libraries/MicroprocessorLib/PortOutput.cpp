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
#include <math.h>
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
	miTimeConstant = 0;
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
	miTimeConstant = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPortOutput::InitSlew(int64 iTime, float32 fStartVoltage, float32 fEndVoltage, int64 iSlewTime, int64 iTimeConstant)
{
	meType = PO_Slew;
	miTime = iTime;
	mfStartVoltage = fStartVoltage;
	mfEndVoltage = fEndVoltage;
	miSlewTime = iSlewTime;
	miTimeConstant = iTimeConstant;
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
// its end time, and on beyond it; or along the RC curve from
// fStartVoltage towards the end voltage.
//
//////////////////////////////////////////////////////////////////////////
float32 CPortOutput::CalculateVoltageAtTime(int64 iTime, float32 fStartVoltage)
{
	float32	fFractionStart;

	if (miTimeConstant > 0)
	{
		return mfEndVoltage + (fStartVoltage - mfEndVoltage) * (float32)exp(-(float64)(iTime - miTime) / (float64)miTimeConstant);
	}
	fFractionStart = (iTime - miTime) / (float32)miSlewTime;
	return fFractionStart * mfEndVoltage + (1 - fFractionStart) * fStartVoltage;
}


//////////////////////////////////////////////////////////////////////////
//
// How long after the slew's time it reaches fVoltage, from fStartVoltage,
// rounded: on the line, or t = RC ln((Vstart - Vend) / (V - Vend)) on the
// curve.  The slew time if the curve reaches it only at its end (fVoltage
// is its end voltage, or beyond it).
//
//////////////////////////////////////////////////////////////////////////
int64 CPortOutput::CalculateTimeAtVoltage(float32 fStartVoltage, float32 fVoltage)
{
	float64	fFraction;
	int64	iTime;

	if (miTimeConstant > 0)
	{
		fFraction = ((float64)fStartVoltage - mfEndVoltage) / ((float64)fVoltage - mfEndVoltage);
		if (fFraction <= 1.0)
		{
			return miSlewTime;
		}
		iTime = (int64)(miTimeConstant * log(fFraction) + 0.5);
		return (iTime < miSlewTime) ? iTime : miSlewTime;
	}
	return RoundFloat32ToInt64((fVoltage - fStartVoltage) / ((mfEndVoltage - fStartVoltage) / miSlewTime));
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
int64 CPortOutput::GetEndTime(void) const
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
bool CPortOutput::IsNone(void) const { return meType == PO_None; }
bool CPortOutput::IsDrive(void) const { return meType == PO_Drive; }
bool CPortOutput::IsSlew(void) const { return meType == PO_Slew; }
int64 CPortOutput::GetTime(void) const { return miTime; }
float32 CPortOutput::GetStartVoltage(void) const { return mfStartVoltage; }
float32 CPortOutput::GetEndVoltage(void) const { return mfEndVoltage; }
int64 CPortOutput::GetSlewTime(void) const { return miSlewTime; }
int64 CPortOutput::GetTimeConstant(void) const { return miTimeConstant; }
bool CPortOutput::IsCurve(void) const { return miTimeConstant > 0; }


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
#include "LongTime.h"
#include "Timeline.h"
#include "LogicPort.h"
#include "Trace.h"


//The id the next trace is given.
int64	giNextTraceID = 1;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrace::Init(void)
{
	Init(giNextTraceID);
}


//////////////////////////////////////////////////////////////////////////
//
// Later traces are given ids after iID.
//
//////////////////////////////////////////////////////////////////////////
void CTrace::Init(int64 iID)
{
	mapcConnectedPorts.Init();
	miID = iID;
	mfCapacitance = TRACE_CAPACITANCE;
	mcWeakCurve.Init();
	if (iID >= giNextTraceID)
	{
		giNextTraceID = iID + 1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Ports still on the trace forget it, without telling anyone, as when
// a circuit is freed before its components.
//
//////////////////////////////////////////////////////////////////////////
void CTrace::Kill(void)
{
	size	i;

	for (i = 0; i < mapcConnectedPorts.NumElements(); i++)
	{
		mapcConnectedPorts.GetPtr(i)->TraceKilled();
	}
	mapcConnectedPorts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The average of the strong drives, or with none the weak drives' voltages
// weighted by their conductance, or where the curve to that has got to;
// NaN when nothing drives the trace.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrace::GetVoltage(int64 iTime)
{
	size	i;
	float32	fVoltage;
	float32	fResistance;
	float32	fDrivenVoltage;
	size	uiDrivers;

	fDrivenVoltage = 0;
	uiDrivers = 0;
	for (i = 0; i < mapcConnectedPorts.NumElements(); i++)
	{
		fVoltage = mapcConnectedPorts.GetPtr(i)->GetDrive(iTime, &fResistance);
		if (IsVoltage(fVoltage) && (fResistance == 0.0f))
		{
			fDrivenVoltage += fVoltage;
			uiDrivers++;
		}
	}

	if (uiDrivers == 1)
	{
		return fDrivenVoltage;
	}
	if (uiDrivers > 1)
	{
		return fDrivenVoltage / uiDrivers;
	}
	if (mcWeakCurve.IsSlew() && mcWeakCurve.ForTime(iTime))
	{
		return mcWeakCurve.GetVoltage(iTime);
	}
	return GetWeakVoltage(iTime, &fResistance);
}


//////////////////////////////////////////////////////////////////////////
//
// The weak drives' voltages weighted by their conductance, and their
// resistance in parallel; NaN with no weak drives.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrace::GetWeakVoltage(int64 iTime, float32* pfResistance)
{
	size	i;
	float32	fVoltage;
	float32	fResistance;
	float32	fWeakCurrent;
	float32	fWeakConductance;

	fWeakCurrent = 0;
	fWeakConductance = 0;
	for (i = 0; i < mapcConnectedPorts.NumElements(); i++)
	{
		fVoltage = mapcConnectedPorts.GetPtr(i)->GetDrive(iTime, &fResistance);
		if (IsVoltage(fVoltage) && (fResistance > 0.0f))
		{
			fWeakCurrent += fVoltage / fResistance;
			fWeakConductance += 1.0f / fResistance;
		}
	}
	if (fWeakConductance > 0.0f)
	{
		*pfResistance = 1.0f / fWeakConductance;
		return fWeakCurrent / fWeakConductance;
	}
	*pfResistance = 0.0f;
	return NoVoltage();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a port drives the trace strongly, so its weak drives don't
// count.
//
//////////////////////////////////////////////////////////////////////////
bool CTrace::HasStrongDriver(int64 iTime)
{
	size	i;
	float32	fResistance;

	for (i = 0; i < mapcConnectedPorts.NumElements(); i++)
	{
		if (IsVoltage(mapcConnectedPorts.GetPtr(i)->GetDrive(iTime, &fResistance)) && (fResistance == 0.0f))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a chip's output drives the trace, rather than only supplies and
// pulls.
//
//////////////////////////////////////////////////////////////////////////
bool CTrace::HasLogicDriver(int64 iTime)
{
	size	i;
	CPort*	pcPort;

	for (i = 0; i < mapcConnectedPorts.NumElements(); i++)
	{
		pcPort = mapcConnectedPorts.GetPtr(i);
		if (pcPort->IsLogicPort() && IsVoltage(pcPort->GetVoltageOut(iTime)))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The difference between the highest and lowest driven voltages, 0 when
// nothing drives the trace.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrace::GetShortVoltage(int64 iTime)
{
	float32	fMinimum;
	float32	fMaximum;

	fMinimum = GetMinimumVoltage(iTime);
	if (!IsVoltage(fMinimum))
	{
		return 0;
	}
	fMaximum = GetMaximumVoltage(iTime);
	return fMaximum - fMinimum;
}


//////////////////////////////////////////////////////////////////////////
//
// Of the strong drives, or the weak ones when there are none; NaN when
// nothing drives the trace.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrace::GetMinimumVoltage(int64 iTime)
{
	size	i;
	float32	fVoltage;
	float32	fResistance;
	float32	fMinimum;
	bool	bStrong;

	bStrong = HasStrongDriver(iTime);
	if (!bStrong && mcWeakCurve.IsSlew() && mcWeakCurve.ForTime(iTime))
	{
		return GetVoltage(iTime);
	}
	fMinimum = NoVoltage();
	for (i = 0; i < mapcConnectedPorts.NumElements(); i++)
	{
		fVoltage = mapcConnectedPorts.GetPtr(i)->GetDrive(iTime, &fResistance);
		if (IsVoltage(fVoltage) && ((fResistance == 0.0f) == bStrong))
		{
			if (!IsVoltage(fMinimum) || (fVoltage < fMinimum))
			{
				fMinimum = fVoltage;
			}
		}
	}
	return fMinimum;
}


//////////////////////////////////////////////////////////////////////////
//
// Of the strong drives, or the weak ones when there are none; NaN when
// nothing drives the trace.
//
//////////////////////////////////////////////////////////////////////////
float32 CTrace::GetMaximumVoltage(int64 iTime)
{
	size	i;
	float32	fVoltage;
	float32	fResistance;
	float32	fMaximum;
	bool	bStrong;

	bStrong = HasStrongDriver(iTime);
	if (!bStrong && mcWeakCurve.IsSlew() && mcWeakCurve.ForTime(iTime))
	{
		return GetVoltage(iTime);
	}
	fMaximum = NoVoltage();
	for (i = 0; i < mapcConnectedPorts.NumElements(); i++)
	{
		fVoltage = mapcConnectedPorts.GetPtr(i)->GetDrive(iTime, &fResistance);
		if (IsVoltage(fVoltage) && ((fResistance == 0.0f) == bStrong))
		{
			if (!IsVoltage(fMaximum) || (fVoltage > fMaximum))
			{
				fMaximum = fVoltage;
			}
		}
	}
	return fMaximum;
}


//////////////////////////////////////////////////////////////////////////
//
// Like "3.3V", or "----" when nothing drives the trace.
//
//////////////////////////////////////////////////////////////////////////
void CTrace::AppendVoltage(CChars* psz, int64 iTime)
{
	::AppendVoltage(psz, GetVoltage(iTime));
}


//////////////////////////////////////////////////////////////////////////
//
// A port is only connected once.
//
//////////////////////////////////////////////////////////////////////////
void CTrace::Connect(CPort* pcPort)
{
	size	i;

	for (i = 0; i < mapcConnectedPorts.NumElements(); i++)
	{
		if (mapcConnectedPorts.GetPtr(i) == pcPort)
		{
			return;
		}
	}
	mapcConnectedPorts.Add(pcPort);
}


//////////////////////////////////////////////////////////////////////////
//
// Tells the ports, except pcExcept, that the trace has stopped being
// driven, see CPort::TraceUndriven.  Logicim doesn't; its ports never
// learn their trace went undriven.
//
//////////////////////////////////////////////////////////////////////////
void CTrace::NotifyUndriven(CTimeline* pcTimeline, CPort* pcExcept)
{
	size	i;
	CPort*	pcPort;

	for (i = 0; i < mapcConnectedPorts.NumElements(); i++)
	{
		pcPort = mapcConnectedPorts.GetPtr(i);
		if (pcPort != pcExcept)
		{
			pcPort->TraceUndriven(pcTimeline);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The trace's voltage has moved from fOldVoltage without a slew; its
// ports but pcExcept are told, see CPort::TraceVoltageChanged.  Held only
// weakly, it curves from fOldVoltage (or where a curve it is already on
// has got to) to its weak voltage, R x C, R the weak drives' in parallel
// and C its capacitance, and its logic ports are told of the curve as of
// a slew (CLogicPort::TraceSlew).
//
//////////////////////////////////////////////////////////////////////////
void CTrace::NotifyVoltageChanged(CTimeline* pcTimeline, float32 fOldVoltage, CPort* pcExcept)
{
	size	i;
	CPort*	pcPort;
	int64	iTime;
	int64	iTimeConstant;
	float32	fWeakVoltage;
	float32	fResistance;
	float32	fStartVoltage;

	iTime = pcTimeline->GetTime();
	iTimeConstant = 0;
	if (!HasStrongDriver(iTime))
	{
		fWeakVoltage = GetWeakVoltage(iTime, &fResistance);
		fStartVoltage = (mcWeakCurve.IsSlew() && mcWeakCurve.ForTime(iTime)) ? mcWeakCurve.GetVoltage(iTime) : fOldVoltage;
		if (IsVoltage(fWeakVoltage) && IsVoltage(fStartVoltage) && (fStartVoltage != fWeakVoltage))
		{
			iTimeConstant = NanosecondsToTime((float64)fResistance * mfCapacitance * 1e9);
		}
		if (iTimeConstant >= 1)
		{
			mcWeakCurve.InitSlew(iTime, fStartVoltage, fWeakVoltage, RC_CURVE_TIME_CONSTANTS * iTimeConstant, iTimeConstant);
		}
		else
		{
			mcWeakCurve.Init();
		}
	}

	for (i = 0; i < mapcConnectedPorts.NumElements(); i++)
	{
		pcPort = mapcConnectedPorts.GetPtr(i);
		if (pcPort != pcExcept)
		{
			if ((iTimeConstant >= 1) && pcPort->IsLogicPort())
			{
				((CLogicPort*)pcPort)->TraceSlew(pcTimeline, &mcWeakCurve);
			}
			else
			{
				pcPort->TraceVoltageChanged(pcTimeline, fOldVoltage);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A strong driver has taken the trace: the curve to its weak voltage ends.
//
//////////////////////////////////////////////////////////////////////////
void CTrace::StopWeakCurve(void)
{
	mcWeakCurve.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps the other ports in the order they were connected.
//
//////////////////////////////////////////////////////////////////////////
void CTrace::Disconnect(CPort* pcPort)
{
	size	i;

	for (i = 0; i < mapcConnectedPorts.NumElements(); i++)
	{
		if (mapcConnectedPorts.GetPtr(i) == pcPort)
		{
			mapcConnectedPorts.RemoveAt(i, true);
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// 0 for NULL.
//
//////////////////////////////////////////////////////////////////////////
int64 CTrace::GetID(CTrace* pcTrace)
{
	if (pcTrace != NULL)
	{
		return pcTrace->GetID();
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrace::ResetNextID(void)
{
	giNextTraceID = 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTrace::NumConnectedPorts(void) const { return mapcConnectedPorts.NumElements(); }
CPort* CTrace::GetConnectedPort(size uiIndex) { return mapcConnectedPorts.SafeGetPtr(uiIndex); }
int64 CTrace::GetID(void) const { return miID; }
void CTrace::SetCapacitance(float32 fCapacitance) { mfCapacitance = fCapacitance; }
float32 CTrace::GetCapacitance(void) const { return mfCapacitance; }
CPortOutput* CTrace::GetWeakCurve(void) { return &mcWeakCurve; }


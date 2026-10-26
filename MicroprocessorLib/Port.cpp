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
#include "Timeline.h"
#include "Trace.h"
#include "PortHolder.h"
#include "Port.h"


//////////////////////////////////////////////////////////////////////////
//
// Adds the port to pcHolder, which owns it.
//
//////////////////////////////////////////////////////////////////////////
void CPort::Init(EPortType eType, const char* szName, CPortHolder* pcHolder)
{
	meType = eType;
	mszName.Init(szName);
	mpcHolder = pcHolder;
	mpcTrace = NULL;
	mbExplicit = true;
	mpcHolder->AddPort(this);
}


//////////////////////////////////////////////////////////////////////////
//
// Disconnects from the trace, which must not have been killed.
//
//////////////////////////////////////////////////////////////////////////
void CPort::Kill(void)
{
	if (mpcTrace != NULL)
	{
		mpcTrace->Disconnect(this);
		mpcTrace = NULL;
	}
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPort::Connect(CTrace* pcTrace, bool bExplicit)
{
	mbExplicit = bExplicit;
	return Connect(pcTrace);
}


//////////////////////////////////////////////////////////////////////////
//
// Fails if the port is already connected.  Call TraceConnected once the
// circuit is wired, as Logicim does.
//
//////////////////////////////////////////////////////////////////////////
bool CPort::Connect(CTrace* pcTrace)
{
	if (mpcTrace != NULL)
	{
		gcLogger.Error2(__METHOD__, " Port [", mszName.Text(), "] is already connected.", NULL);
		return false;
	}

	if (pcTrace == NULL)
	{
		gcLogger.Error2(__METHOD__, " Cannot connect NULL Trace to Port [", mszName.Text(), "].", NULL);
		return false;
	}

	mpcTrace = pcTrace;
	mpcTrace->Connect(this);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Connects the port while the simulation runs, as Logicim's editor does.
// The port is told its trace is connected, so a logic port sees the
// trace's level, low or high, or a slew in progress.  If the port drives
// a trace that was undriven, the trace's other ports are told too, so
// they see the new level and chips see their power arrive.  The other
// ports on a trace that was already driven are left alone, so a clock
// input doesn't see an edge that never happened.
//
//////////////////////////////////////////////////////////////////////////
bool CPort::ConnectLive(CTrace* pcTrace, CTimeline* pcTimeline)
{
	int64	iTime;
	float32	fVoltageBefore;
	float32	fVoltageAfter;
	size	i;
	CPort*	pcOther;

	iTime = pcTimeline->GetTime();
	fVoltageBefore = NoVoltage();
	if (pcTrace != NULL)
	{
		fVoltageBefore = pcTrace->GetVoltage(iTime);
	}

	if (!Connect(pcTrace))
	{
		return false;
	}
	TraceConnected(pcTimeline);

	fVoltageAfter = mpcTrace->GetVoltage(iTime);
	if (!IsVoltage(fVoltageBefore) && IsVoltage(fVoltageAfter))
	{
		for (i = 0; i < mpcTrace->NumConnectedPorts(); i++)
		{
			pcOther = mpcTrace->GetConnectedPort(i);
			if (pcOther != this)
			{
				pcOther->TraceConnected(pcTimeline);
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Called by CTrace::Kill.  The trace is going, so it isn't told.
//
//////////////////////////////////////////////////////////////////////////
void CPort::TraceKilled(void)
{
	mpcTrace = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPort::Disconnect(CTimeline* pcTimeline)
{
	CTrace*	pcTrace;
	int64	iTime;
	float32	fVoltageBefore;

	if (mpcTrace == NULL)
	{
		return;
	}

	pcTrace = mpcTrace;
	iTime = pcTimeline->GetTime();
	fVoltageBefore = pcTrace->GetVoltage(iTime);
	pcTrace->Disconnect(this);
	mpcTrace = NULL;

	if (IsVoltage(fVoltageBefore))
	{
		if (!IsVoltage(pcTrace->GetVoltage(iTime)))
		{
			pcTrace->NotifyUndriven(pcTimeline, NULL);
		}
		TraceUndriven(pcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The port's trace has stopped being driven, or the port has been
// disconnected from a driven trace.  The holder is told through
// TraceConnected, Logicim's hook for a change to a port's trace, as a
// clock uses it to stop driving when its power goes.  CLogicPort
// schedules a transition instead.
//
//////////////////////////////////////////////////////////////////////////
void CPort::TraceUndriven(CTimeline* pcTimeline)
{
	mpcHolder->TraceConnected(pcTimeline, this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPort::TraceConnected(CTimeline* pcTimeline)
{
	mpcHolder->TraceConnected(pcTimeline, this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPort::Reset(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Only CLogicPort and CPowerOutPort drive their trace.
//
//////////////////////////////////////////////////////////////////////////
float32 CPort::GetVoltageOut(int64 iTime)
{
	return NoVoltage();
}


//////////////////////////////////////////////////////////////////////////
//
// 0 when not connected.
//
//////////////////////////////////////////////////////////////////////////
int64 CPort::GetTraceID(void)
{
	if (mpcTrace != NULL)
	{
		return mpcTrace->GetID();
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Like "AND \"U1\".In 1".
//
//////////////////////////////////////////////////////////////////////////
void CPort::AppendDescription(CChars* psz)
{
	mpcHolder->AppendDescription(psz);
	psz->Append('.');
	psz->Append(mszName);
}


//////////////////////////////////////////////////////////////////////////
//
// Like "U1.Output(7)": the holder's name, the port's and its trace's id.
//
//////////////////////////////////////////////////////////////////////////
void CPort::AppendDebugString(CChars* psz)
{
	char*	szHolderName;

	szHolderName = mpcHolder->GetComponentName();
	if (szHolderName != NULL)
	{
		psz->Append(szHolderName);
	}
	psz->Append('.');
	psz->Append(mszName);
	psz->Append('(');
	psz->Append(GetTraceID());
	psz->Append(')');
}


//////////////////////////////////////////////////////////////////////////
//
// The class without its C and Port, like "Logic", as Logicim's getType.
//
//////////////////////////////////////////////////////////////////////////
void CPort::AppendType(CChars* psz)
{
	CChars	szName;

	szName.Init(ClassName());
	if (szName.StartsWith("C"))
	{
		szName.RemoveFromStart(1);
	}
	szName.Replace("Port", "");
	psz->Append(szName);
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPort::IsLogicPort(void) { return false; }
bool CPort::IsPowerIn(void) { return false; }
char* CPort::GetName(void) { return mszName.Text(); }
EPortType CPort::GetPortType(void) { return meType; }
CTrace* CPort::GetTrace(void) { return mpcTrace; }
CPortHolder* CPort::GetHolder(void) { return mpcHolder; }
bool CPort::IsExplicit(void) { return mbExplicit; }


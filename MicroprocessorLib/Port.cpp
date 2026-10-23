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
//
//////////////////////////////////////////////////////////////////////////
void CPort::Disconnect(CTimeline* pcTimeline)
{
	if (mpcTrace != NULL)
	{
		mpcTrace->Disconnect(this);
		mpcTrace = NULL;
	}
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
//
//////////////////////////////////////////////////////////////////////////
bool CPort::IsLogicPort(void) { return false; }
bool CPort::IsPowerIn(void) { return false; }
char* CPort::GetName(void) { return mszName.Text(); }
EPortType CPort::GetPortType(void) { return meType; }
CTrace* CPort::GetTrace(void) { return mpcTrace; }
CPortHolder* CPort::GetHolder(void) { return mpcHolder; }
bool CPort::IsExplicit(void) { return mbExplicit; }


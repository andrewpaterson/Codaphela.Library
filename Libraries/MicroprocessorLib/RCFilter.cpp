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
#include "BaseLib/ConstructorCall.h"
#include "Voltage.h"
#include "Timeline.h"
#include "Trace.h"
#include "RCFilter.h"


//////////////////////////////////////////////////////////////////////////
//
// Time constants in simulation time (1/1024ns), each at least 1.
//
//////////////////////////////////////////////////////////////////////////
void CRCFilter::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CRCFilterPins> pPins, int64 iRiseTimeConstant, int64 iFallTimeConstant)
{
	PreInit();
	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pPins);
	SetTimeConstants(iRiseTimeConstant, iFallTimeConstant);
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRCFilter::Class(void)
{
	CIntegratedCircuit::Class();
	U_Int64(miRiseTimeConstant);
	U_Int64(miFallTimeConstant);
}


//////////////////////////////////////////////////////////////////////////
//
// Its output takes its input's voltage when the input is first told.
//
//////////////////////////////////////////////////////////////////////////
void CRCFilter::SimulationStarted(CTimeline* pcTimeline)
{
}


//////////////////////////////////////////////////////////////////////////
//
// The input is going to a new voltage: the output curves towards it from
// where it is, at the rising or the falling time constant.  Undriven, the
// output holds.
//
//////////////////////////////////////////////////////////////////////////
void CRCFilter::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	float32		fTarget;
	float32		fVoltage;
	CLogicPort*	pcOut;
	int64		iTime;

	fTarget = GetRCFilterPins()->GetIn()->GetTarget();
	if (!IsVoltage(fTarget))
	{
		return;
	}
	pcOut = GetRCFilterPins()->GetOut();
	iTime = pcTimeline->GetTime();
	fVoltage = pcOut->GetVoltageOut(iTime);
	if (!IsVoltage(fVoltage) && (pcOut->GetTrace() != NULL))
	{
		fVoltage = pcOut->GetTrace()->GetVoltage(iTime);
	}
	if (IsVoltage(fVoltage) && (fTarget < fVoltage))
	{
		pcOut->WriteCurve(pcTimeline, fTarget, miFallTimeConstant, 1);
	}
	else
	{
		pcOut->WriteCurve(pcTimeline, fTarget, miRiseTimeConstant, 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRCFilter::SetTimeConstants(int64 iRiseTimeConstant, int64 iFallTimeConstant)
{
	miRiseTimeConstant = (iRiseTimeConstant >= 1) ? iRiseTimeConstant : 1;
	miFallTimeConstant = (iFallTimeConstant >= 1) ? iFallTimeConstant : 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRCFilter::NeedsPower(void) const { return false; }
const char* CRCFilter::GetType(void) { return RC_FILTER_TYPE; }
int64 CRCFilter::GetRiseTimeConstant(void) const { return miRiseTimeConstant; }
int64 CRCFilter::GetFallTimeConstant(void) const { return miFallTimeConstant; }
CRCFilterPins* CRCFilter::GetRCFilterPins(void) { return (CRCFilterPins*)&mpPins; }


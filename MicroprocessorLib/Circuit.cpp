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
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/Logger.h"
#include "Circuit.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuit::Init(CTimeline* pcTimeline)
{
	mpcTimeline = pcTimeline;
	mapcIntegratedCircuits.Init();
	mapcTraces.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// The chips, then the traces.  The timeline must not have been killed.
//
//////////////////////////////////////////////////////////////////////////
void CCircuit::Kill(void)
{
	size				i;
	CIntegratedCircuit*	pcIntegratedCircuit;
	CTrace*				pcTrace;

	for (i = 0; i < mapcIntegratedCircuits.NumElements(); i++)
	{
		pcIntegratedCircuit = mapcIntegratedCircuits.GetPtr(i);
		SafeKill(pcIntegratedCircuit);
	}
	mapcIntegratedCircuits.Kill();

	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		pcTrace = mapcTraces.GetPtr(i);
		SafeKill(pcTrace);
	}
	mapcTraces.Kill();
	mpcTimeline = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Called by CIntegratedCircuit::Init.
//
//////////////////////////////////////////////////////////////////////////
void CCircuit::Add(CIntegratedCircuit* pcIntegratedCircuit)
{
	mapcIntegratedCircuits.Add(pcIntegratedCircuit);
}


//////////////////////////////////////////////////////////////////////////
//
// The circuit no longer owns the chip; the caller kills and frees it.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuit::Remove(CIntegratedCircuit* pcIntegratedCircuit)
{
	size	i;
	CChars	sz;

	for (i = 0; i < mapcIntegratedCircuits.NumElements(); i++)
	{
		if (mapcIntegratedCircuits.GetPtr(i) == pcIntegratedCircuit)
		{
			mapcIntegratedCircuits.RemoveAt(i, true);
			return true;
		}
	}

	sz.Init(" Could not remove Integrated Circuit [");
	pcIntegratedCircuit->AppendDescription(&sz);
	sz.Append("] from Circuit.");
	gcLogger.Error2(__METHOD__, sz.Text(), NULL);
	sz.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A new trace with the next id, owned by the circuit.
//
//////////////////////////////////////////////////////////////////////////
CTrace* CCircuit::AddTrace(void)
{
	CTrace*	pcTrace;

	pcTrace = NewMalloc<CTrace>();
	pcTrace->Init();
	mapcTraces.Add(pcTrace);
	return pcTrace;
}


//////////////////////////////////////////////////////////////////////////
//
// Resets every chip, as Logicim's simulation does when it starts.
//
//////////////////////////////////////////////////////////////////////////
void CCircuit::SimulationStarted(void)
{
	size	i;

	for (i = 0; i < mapcIntegratedCircuits.NumElements(); i++)
	{
		mapcIntegratedCircuits.GetPtr(i)->Reset(mpcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the chips of type szType to papcIntegratedCircuits.
//
//////////////////////////////////////////////////////////////////////////
void CCircuit::GetIntegratedCircuits(const char* szType, CArrayIntegratedCircuitPtr* papcIntegratedCircuits)
{
	size				i;
	CIntegratedCircuit*	pcIntegratedCircuit;

	for (i = 0; i < mapcIntegratedCircuits.NumElements(); i++)
	{
		pcIntegratedCircuit = mapcIntegratedCircuits.GetPtr(i);
		if (StringCompare(pcIntegratedCircuit->GetType(), szType) == 0)
		{
			papcIntegratedCircuits->Add(pcIntegratedCircuit);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCircuit::IsEmpty(void) { return mapcIntegratedCircuits.NumElements() == 0; }
size CCircuit::NumIntegratedCircuits(void) { return mapcIntegratedCircuits.NumElements(); }
CIntegratedCircuit* CCircuit::GetIntegratedCircuit(size uiIndex) { return mapcIntegratedCircuits.SafeGetPtr(uiIndex); }
size CCircuit::NumTraces(void) { return mapcTraces.NumElements(); }
CTrace* CCircuit::GetTrace(size uiIndex) { return mapcTraces.SafeGetPtr(uiIndex); }
CTimeline* CCircuit::GetTimeline(void) { return mpcTimeline; }


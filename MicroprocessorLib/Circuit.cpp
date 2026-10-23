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
void CCircuit::Init(void)
{
	mapcIntegratedCircuits.Init();
	mapcPassives.Init();
	mapcTraces.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// The components, then the traces.  The timeline must not have been
// killed.
//
//////////////////////////////////////////////////////////////////////////
void CCircuit::Kill(void)
{
	size				i;
	CIntegratedCircuit*	pcIntegratedCircuit;
	CPassive*			pcPassive;
	CTrace*				pcTrace;

	for (i = 0; i < mapcIntegratedCircuits.NumElements(); i++)
	{
		pcIntegratedCircuit = mapcIntegratedCircuits.GetPtr(i);
		SafeKill(pcIntegratedCircuit);
	}
	mapcIntegratedCircuits.Kill();

	for (i = 0; i < mapcPassives.NumElements(); i++)
	{
		pcPassive = mapcPassives.GetPtr(i);
		SafeKill(pcPassive);
	}
	mapcPassives.Kill();

	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		pcTrace = mapcTraces.GetPtr(i);
		SafeKill(pcTrace);
	}
	mapcTraces.Kill();
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
// Called by CPassive::Init.
//
//////////////////////////////////////////////////////////////////////////
void CCircuit::Add(CPassive* pcPassive)
{
	mapcPassives.Add(pcPassive);
}


//////////////////////////////////////////////////////////////////////////
//
// The circuit no longer owns the chip; the caller kills and frees it.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuit::Remove(CIntegratedCircuit* pcIntegratedCircuit)
{
	size	i;

	for (i = 0; i < mapcIntegratedCircuits.NumElements(); i++)
	{
		if (mapcIntegratedCircuits.GetPtr(i) == pcIntegratedCircuit)
		{
			mapcIntegratedCircuits.RemoveAt(i, true);
			return true;
		}
	}
	LogRemoveError(pcIntegratedCircuit);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The circuit no longer owns the passive; the caller kills and frees it.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuit::Remove(CPassive* pcPassive)
{
	size	i;

	for (i = 0; i < mapcPassives.NumElements(); i++)
	{
		if (mapcPassives.GetPtr(i) == pcPassive)
		{
			mapcPassives.RemoveAt(i, true);
			return true;
		}
	}
	LogRemoveError(pcPassive);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuit::LogRemoveError(CCircuitComponent* pcComponent)
{
	CChars	sz;

	sz.Init(" Could not remove [");
	pcComponent->AppendDescription(&sz);
	sz.Append("] from Circuit.");
	gcLogger.Error2(__METHOD__, sz.Text(), NULL);
	sz.Kill();
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
// Resets every component, as Logicim does when a simulation starts.
//
//////////////////////////////////////////////////////////////////////////
void CCircuit::SimulationStarted(CTimeline* pcTimeline)
{
	size	i;

	for (i = 0; i < mapcIntegratedCircuits.NumElements(); i++)
	{
		mapcIntegratedCircuits.GetPtr(i)->Reset(pcTimeline);
	}
	for (i = 0; i < mapcPassives.NumElements(); i++)
	{
		mapcPassives.GetPtr(i)->Reset(pcTimeline);
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
bool CCircuit::IsEmpty(void) { return (mapcIntegratedCircuits.NumElements() == 0) && (mapcPassives.NumElements() == 0); }
size CCircuit::NumIntegratedCircuits(void) { return mapcIntegratedCircuits.NumElements(); }
CIntegratedCircuit* CCircuit::GetIntegratedCircuit(size uiIndex) { return mapcIntegratedCircuits.SafeGetPtr(uiIndex); }
size CCircuit::NumTraces(void) { return mapcTraces.NumElements(); }
CTrace* CCircuit::GetTrace(size uiIndex) { return mapcTraces.SafeGetPtr(uiIndex); }
size CCircuit::NumPassives(void) { return mapcPassives.NumElements(); }
CPassive* CCircuit::GetPassive(size uiIndex) { return mapcPassives.SafeGetPtr(uiIndex); }


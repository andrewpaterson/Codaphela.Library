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
	PreInit();

	maIntegratedCircuits.Init();
	maPassives.Init();
	mapcTraces.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuit::Class(void)
{
	M_Embedded(maIntegratedCircuits);
	M_Embedded(maPassives);
	U_Data(CArrayTracePtr, mapcTraces);
}


//////////////////////////////////////////////////////////////////////////
//
// Kills the traces.  The components are freed with the circuit, before
// or after it, see the class comment.
//
//////////////////////////////////////////////////////////////////////////
void CCircuit::Free(void)
{
	size		i;
	CTrace*		pcTrace;

	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		pcTrace = mapcTraces.GetPtr(i);
		SafeKill(pcTrace);
	}
	mapcTraces.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCircuit::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCircuit::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Called by CIntegratedCircuit::Init.
//
//////////////////////////////////////////////////////////////////////////
void CCircuit::Add(CIntegratedCircuit* pcIntegratedCircuit)
{
	maIntegratedCircuits.Add(pcIntegratedCircuit);
}


//////////////////////////////////////////////////////////////////////////
//
// Called by CPassive::Init.
//
//////////////////////////////////////////////////////////////////////////
void CCircuit::Add(CPassive* pcPassive)
{
	maPassives.Add(pcPassive);
}


//////////////////////////////////////////////////////////////////////////
//
// Frees the chip, unless something else still points to it.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuit::Remove(CIntegratedCircuit* pcIntegratedCircuit)
{
	if (RemoveFrom(&maIntegratedCircuits, pcIntegratedCircuit))
	{
		return true;
	}
	LogRemoveError(pcIntegratedCircuit);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Frees the passive, unless something else still points to it.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuit::Remove(CPassive* pcPassive)
{
	if (RemoveFrom(&maPassives, pcPassive))
	{
		return true;
	}
	LogRemoveError(pcPassive);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A chip or a passive, freed unless something else still points to it.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuit::Remove(CCircuitComponent* pcComponent)
{
	if (RemoveFrom(&maIntegratedCircuits, pcComponent))
	{
		return true;
	}
	if (RemoveFrom(&maPassives, pcComponent))
	{
		return true;
	}
	LogRemoveError(pcComponent);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// False, without an error, if the component isn't in the array.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CCircuit::RemoveFrom(CArray<M>* paComponents, CCircuitComponent* pcComponent)
{
	size	i;

	for (i = 0; i < paComponents->NumElements(); i++)
	{
		if (paComponents->UnsafeGetObject(i) == pcComponent)
		{
			return paComponents->RemoveAt(i);
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Kills and frees the trace, which should have no ports left on it.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuit::RemoveTrace(CTrace* pcTrace)
{
	size	i;

	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		if (mapcTraces.GetPtr(i) == pcTrace)
		{
			mapcTraces.RemoveAt(i, true);
			SafeKill(pcTrace);
			return true;
		}
	}
	return gcLogger.Error2(__METHOD__, " Could not remove a trace from Circuit.", NULL);
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
// As Logicim's disconnectComponent: takes the component's own events off
// the timeline.  Its ports are disconnected with DisconnectPorts.
//
//////////////////////////////////////////////////////////////////////////
void CCircuit::DisconnectComponent(CCircuitComponent* pcComponent, CTimeline* pcTimeline)
{
	pcComponent->Disconnect(pcTimeline);
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

	for (i = 0; i < maIntegratedCircuits.NumElements(); i++)
	{
		GetIntegratedCircuit(i)->Reset(pcTimeline);
	}
	for (i = 0; i < maPassives.NumElements(); i++)
	{
		GetPassive(i)->Reset(pcTimeline);
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

	for (i = 0; i < maIntegratedCircuits.NumElements(); i++)
	{
		pcIntegratedCircuit = GetIntegratedCircuit(i);
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
bool CCircuit::IsEmpty(void) { return (maIntegratedCircuits.NumElements() == 0) && (maPassives.NumElements() == 0); }
size CCircuit::NumIntegratedCircuits(void) { return maIntegratedCircuits.NumElements(); }
size CCircuit::NumTraces(void) { return mapcTraces.NumElements(); }
CTrace* CCircuit::GetTrace(size uiIndex) { return mapcTraces.SafeGetPtr(uiIndex); }
size CCircuit::NumPassives(void) { return maPassives.NumElements(); }


//////////////////////////////////////////////////////////////////////////
//
// NULL if uiIndex is past the end.
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuit* CCircuit::GetIntegratedCircuit(size uiIndex)
{
	if (uiIndex >= maIntegratedCircuits.NumElements())
	{
		return NULL;
	}
	return (CIntegratedCircuit*)maIntegratedCircuits.UnsafeGetObject(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if uiIndex is past the end.
//
//////////////////////////////////////////////////////////////////////////
CPassive* CCircuit::GetPassive(size uiIndex)
{
	if (uiIndex >= maPassives.NumElements())
	{
		return NULL;
	}
	return (CPassive*)maPassives.UnsafeGetObject(uiIndex);
}


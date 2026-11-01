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
#include "BaseLib/ConstructorCall.h"
#include "CircuitSimulation.h"
#include "SubcircuitSimulations.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitSimulations::Init(void)
{
	mapsCircuitSimulations.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitSimulations::Kill(void)
{
	Clear();
	mapsCircuitSimulations.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitSimulations::Clear(void)
{
	size							i;
	SCircuitSubcircuitSimulations*	psCircuit;

	for (i = 0; i < mapsCircuitSimulations.NumElements(); i++)
	{
		psCircuit = mapsCircuitSimulations.GetPtr(i);
		psCircuit->apcSubcircuitSimulations.Kill();
		free(psCircuit);
	}
	mapsCircuitSimulations.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// Fails if it has already been added.
//
//////////////////////////////////////////////////////////////////////////
bool CSubcircuitSimulations::Add(CSubcircuitSimulation* pcSubcircuitSimulation)
{
	SCircuitSubcircuitSimulations*	psCircuit;
	size							i;
	CChars							sz;

	psCircuit = Find(pcSubcircuitSimulation->GetCircuitSimulation(), NULL);
	if (psCircuit != NULL)
	{
		for (i = 0; i < psCircuit->apcSubcircuitSimulations.NumElements(); i++)
		{
			if (psCircuit->apcSubcircuitSimulations.GetPtr(i) == pcSubcircuitSimulation)
			{
				sz.Init(" A simulation [");
				pcSubcircuitSimulation->AppendDescription(&sz);
				sz.Append("] for circuit [");
				pcSubcircuitSimulation->GetCircuitSimulation()->AppendDescription(&sz);
				sz.Append("] already exists.");
				gcLogger.Error2(__METHOD__, sz.Text(), NULL);
				sz.Kill();
				return false;
			}
		}
	}
	else
	{
		psCircuit = NewMalloc<SCircuitSubcircuitSimulations>();
		mapsCircuitSimulations.Add(psCircuit);
		psCircuit->pcCircuitSimulation = pcSubcircuitSimulation->GetCircuitSimulation();
		psCircuit->apcSubcircuitSimulations.Init();
	}

	psCircuit->apcSubcircuitSimulations.Add(pcSubcircuitSimulation);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A circuit simulation with none left is dropped.
//
//////////////////////////////////////////////////////////////////////////
bool CSubcircuitSimulations::Remove(CSubcircuitSimulation* pcSubcircuitSimulation)
{
	SCircuitSubcircuitSimulations*	psCircuit;
	size							iCircuit;
	size							i;
	CChars							sz;

	psCircuit = Find(pcSubcircuitSimulation->GetCircuitSimulation(), &iCircuit);
	if (psCircuit != NULL)
	{
		for (i = 0; i < psCircuit->apcSubcircuitSimulations.NumElements(); i++)
		{
			if (psCircuit->apcSubcircuitSimulations.GetPtr(i) == pcSubcircuitSimulation)
			{
				psCircuit->apcSubcircuitSimulations.RemoveAt(i, true);
				if (psCircuit->apcSubcircuitSimulations.NumElements() == 0)
				{
					psCircuit->apcSubcircuitSimulations.Kill();
					free(psCircuit);
					mapsCircuitSimulations.RemoveAt(iCircuit, true);
				}
				return true;
			}
		}
	}

	sz.Init(" Could not remove Subcircuit Simulation [");
	pcSubcircuitSimulation->AppendDescription(&sz);
	sz.Append("].");
	gcLogger.Error2(__METHOD__, sz.Text(), NULL);
	sz.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if none have been added for pcCircuitSimulation.
//
//////////////////////////////////////////////////////////////////////////
CArraySubcircuitSimulationPtr* CSubcircuitSimulations::GetSubcircuitSimulations(CCircuitSimulation* pcCircuitSimulation)
{
	SCircuitSubcircuitSimulations*	psCircuit;

	psCircuit = Find(pcCircuitSimulation, NULL);
	if (psCircuit != NULL)
	{
		return &psCircuit->apcSubcircuitSimulations;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitSimulations::GetSubcircuitTopSimulations(CArraySubcircuitSimulationPtr* papcSubcircuitSimulations)
{
	size					i;
	size					j;
	CSubcircuitSimulation*	pcSubcircuitSimulation;

	for (i = 0; i < mapsCircuitSimulations.NumElements(); i++)
	{
		for (j = 0; j < mapsCircuitSimulations.GetPtr(i)->apcSubcircuitSimulations.NumElements(); j++)
		{
			pcSubcircuitSimulation = mapsCircuitSimulations.GetPtr(i)->apcSubcircuitSimulations.GetPtr(j);
			if (pcSubcircuitSimulation->IsTop())
			{
				papcSubcircuitSimulations->Add(pcSubcircuitSimulation);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitSimulations::GetSubcircuitInstanceSimulations(CArraySubcircuitSimulationPtr* papcSubcircuitSimulations)
{
	size					i;
	size					j;
	CSubcircuitSimulation*	pcSubcircuitSimulation;

	for (i = 0; i < mapsCircuitSimulations.NumElements(); i++)
	{
		for (j = 0; j < mapsCircuitSimulations.GetPtr(i)->apcSubcircuitSimulations.NumElements(); j++)
		{
			pcSubcircuitSimulation = mapsCircuitSimulations.GetPtr(i)->apcSubcircuitSimulations.GetPtr(j);
			if (pcSubcircuitSimulation->IsInstance())
			{
				papcSubcircuitSimulations->Add(pcSubcircuitSimulation);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Every subcircuit simulation.
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitSimulations::GetSubcircuitSimulations(CArraySubcircuitSimulationPtr* papcSubcircuitSimulations)
{
	size	i;
	size	j;

	for (i = 0; i < mapsCircuitSimulations.NumElements(); i++)
	{
		for (j = 0; j < mapsCircuitSimulations.GetPtr(i)->apcSubcircuitSimulations.NumElements(); j++)
		{
			papcSubcircuitSimulations->Add(mapsCircuitSimulations.GetPtr(i)->apcSubcircuitSimulations.GetPtr(j));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCircuitSubcircuitSimulations* CSubcircuitSimulations::Find(CCircuitSimulation* pcCircuitSimulation, size* piIndex)
{
	size	i;

	for (i = 0; i < mapsCircuitSimulations.NumElements(); i++)
	{
		if (mapsCircuitSimulations.GetPtr(i)->pcCircuitSimulation == pcCircuitSimulation)
		{
			if (piIndex != NULL)
			{
				*piIndex = i;
			}
			return mapsCircuitSimulations.GetPtr(i);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CSubcircuitSimulations::NumCircuitSimulations(void)
{
	return mapsCircuitSimulations.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
// NULL past the end.
//
//////////////////////////////////////////////////////////////////////////
CCircuitSimulation* CSubcircuitSimulations::GetCircuitSimulation(size uiIndex)
{
	SCircuitSubcircuitSimulations*	psCircuit;

	psCircuit = mapsCircuitSimulations.SafeGetPtr(uiIndex);
	if (psCircuit != NULL)
	{
		return psCircuit->pcCircuitSimulation;
	}
	return NULL;
}


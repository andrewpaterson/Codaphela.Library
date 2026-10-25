/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/Logger.h"
#include "MicroprocessorLib/CircuitSimulation.h"
#include "MicroprocessorLib/SubcircuitSimulation.h"
#include "CircuitView.h"
#include "ViewPath.h"


int64	giNextViewPathID = 1;


//////////////////////////////////////////////////////////////////////////
//
// Adds nothing; only an instance of a subcircuit has pins outside.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitInstanceView::GetPinPortJoins(CArrayPinPortJoin* pasJoins)
{
}


//////////////////////////////////////////////////////////////////////////
//
// The path's views are copied.
//
//////////////////////////////////////////////////////////////////////////
void CViewPath::Init(CArrayCircuitInstanceViewPtr* papcPath, CViewPath* pcPrevious, CViewPaths* pcViewPaths)
{
	size	i;

	mapcPath.Init();
	for (i = 0; i < papcPath->NumElements(); i++)
	{
		mapcPath.Add(papcPath->GetPtr(i));
	}
	masSimulations.Init();
	miID = giNextViewPathID;
	giNextViewPathID++;
	mpcPrevious = pcPrevious;
	mpcViewPaths = pcViewPaths;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CViewPath::Kill(void)
{
	masSimulations.Kill();
	mapcPath.Kill();
	mpcPrevious = NULL;
	mpcViewPaths = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Fails if the path already has a subcircuit simulation for the
// subcircuit simulation's circuit simulation.
//
//////////////////////////////////////////////////////////////////////////
bool CViewPath::AddSubcircuitSimulation(CSubcircuitSimulation* pcSubcircuitSimulation)
{
	CCircuitSimulation*		pcCircuitSimulation;
	SViewPathSimulation*	psSimulation;
	CChars					sz;

	pcCircuitSimulation = pcSubcircuitSimulation->GetCircuitSimulation();
	if (FindSimulation(pcCircuitSimulation, NULL) != NULL)
	{
		sz.Init();
		AppendDescription(&sz);
		gcLogger.Error2(__METHOD__, " Path [", sz.Text(), "] already has a subcircuit simulation for circuit simulation [", pcCircuitSimulation->GetName(), "].", NULL);
		sz.Kill();
		return false;
	}

	psSimulation = masSimulations.Add();
	psSimulation->pcCircuitSimulation = pcCircuitSimulation;
	psSimulation->pcSubcircuitSimulation = pcSubcircuitSimulation;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CViewPath::RemoveSubcircuitSimulation(CCircuitSimulation* pcCircuitSimulation)
{
	size	uiIndex;

	if (FindSimulation(pcCircuitSimulation, &uiIndex) == NULL)
	{
		return false;
	}
	masSimulations.RemoveAt(uiIndex, true);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CViewPath::ContainsCircuitSimulation(CCircuitSimulation* pcCircuitSimulation)
{
	return FindSimulation(pcCircuitSimulation, NULL) != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if the path isn't simulated in the circuit simulation.
//
//////////////////////////////////////////////////////////////////////////
CSubcircuitSimulation* CViewPath::GetSubcircuitSimulation(CCircuitSimulation* pcCircuitSimulation)
{
	SViewPathSimulation*	psSimulation;

	psSimulation = FindSimulation(pcCircuitSimulation, NULL);
	if (psSimulation == NULL)
	{
		return NULL;
	}
	return psSimulation->pcSubcircuitSimulation;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SViewPathSimulation* CViewPath::FindSimulation(CCircuitSimulation* pcCircuitSimulation, size* puiIndex)
{
	size					i;
	SViewPathSimulation*	psSimulation;

	for (i = 0; i < masSimulations.NumElements(); i++)
	{
		psSimulation = masSimulations.Get(i);
		if (psSimulation->pcCircuitSimulation == pcCircuitSimulation)
		{
			if (puiIndex != NULL)
			{
				*puiIndex = i;
			}
			return psSimulation;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CViewPath::NumCircuitSimulations(void)
{
	return masSimulations.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCircuitSimulation* CViewPath::GetCircuitSimulation(size uiIndex)
{
	SViewPathSimulation*	psSimulation;

	psSimulation = masSimulations.SafeGet(uiIndex);
	if (psSimulation == NULL)
	{
		return NULL;
	}
	return psSimulation->pcCircuitSimulation;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CViewPath::Length(void)
{
	return mapcPath.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCircuitInstanceView* CViewPath::Get(size uiIndex)
{
	return mapcPath.SafeGetPtr(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCircuitInstanceView* CViewPath::GetFirst(void)
{
	return mapcPath.SafeGetPtr(0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCircuitInstanceView* CViewPath::GetLast(void)
{
	if (mapcPath.NumElements() == 0)
	{
		return NULL;
	}
	return mapcPath.GetPtr(mapcPath.NumElements() - 1);
}


//////////////////////////////////////////////////////////////////////////
//
// The circuit whose views the path's components are made from.
//
//////////////////////////////////////////////////////////////////////////
CCircuitView* CViewPath::GetCircuitView(void)
{
	CCircuitInstanceView*	pcLast;

	pcLast = GetLast();
	if (pcLast == NULL)
	{
		return NULL;
	}
	return pcLast->GetInstanceCircuitView();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CViewPath::IsTop(void)
{
	return mapcPath.NumElements() == 1;
}


//////////////////////////////////////////////////////////////////////////
//
// The same views in the same order.
//
//////////////////////////////////////////////////////////////////////////
bool CViewPath::EqualsPath(CArrayCircuitInstanceViewPtr* papcPath)
{
	size	i;

	if (mapcPath.NumElements() != papcPath->NumElements())
	{
		return false;
	}
	for (i = 0; i < mapcPath.NumElements(); i++)
	{
		if (mapcPath.GetPtr(i) != papcPath->GetPtr(i))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CViewPath* CViewPath::GetPrevious(void)
{
	return mpcPrevious;
}


//////////////////////////////////////////////////////////////////////////
//
// The top path this one goes down from, or this one if it's a top path.
//
//////////////////////////////////////////////////////////////////////////
CViewPath* CViewPath::GetTop(void)
{
	CViewPath*	pcViewPath;

	pcViewPath = this;
	while (pcViewPath->mpcPrevious != NULL)
	{
		pcViewPath = pcViewPath->mpcPrevious;
	}
	return pcViewPath;
}


//////////////////////////////////////////////////////////////////////////
//
// This path with the instance view, which must be in this path's last
// circuit, on the end; made if it isn't there yet.  NULL if the
// instance's subcircuit is already on the path.
//
//////////////////////////////////////////////////////////////////////////
CViewPath* CViewPath::GetChild(CCircuitInstanceView* pcInstanceView)
{
	return mpcViewPaths->GetOrAddViewPath(this, pcInstanceView);
}


//////////////////////////////////////////////////////////////////////////
//
// As GetChild, but NULL rather than made if there isn't one.
//
//////////////////////////////////////////////////////////////////////////
CViewPath* CViewPath::FindChild(CCircuitInstanceView* pcInstanceView)
{
	return mpcViewPaths->GetViewPath(this, pcInstanceView);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the circuit is shown by any of the path's views.
//
//////////////////////////////////////////////////////////////////////////
bool CViewPath::ContainsCircuitView(CCircuitView* pcCircuitView)
{
	size	i;

	for (i = 0; i < mapcPath.NumElements(); i++)
	{
		if (mapcPath.GetPtr(i)->GetInstanceCircuitView() == pcCircuitView)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CViewPath::GetID(void)
{
	return miID;
}


//////////////////////////////////////////////////////////////////////////
//
// Each view's description, " -> " between them, as Logicim's.
//
//////////////////////////////////////////////////////////////////////////
void CViewPath::AppendDescription(CChars* psz)
{
	size	i;

	for (i = 0; i < mapcPath.NumElements(); i++)
	{
		if (i != 0)
		{
			psz->Append(" -> ");
		}
		mapcPath.GetPtr(i)->AppendDescription(psz);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CViewPath::ResetNextID(void)
{
	giNextViewPathID = 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CViewPaths::Init(void)
{
	mapcViewPaths.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CViewPaths::Kill(void)
{
	size		i;
	CViewPath*	pcViewPath;

	for (i = 0; i < mapcViewPaths.NumElements(); i++)
	{
		pcViewPath = mapcViewPaths.GetPtr(i);
		SafeKill(pcViewPath);
	}
	mapcViewPaths.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The path of the circuit on its own; fails if it already has one.
//
//////////////////////////////////////////////////////////////////////////
CViewPath* CViewPaths::AddTop(CCircuitView* pcCircuitView)
{
	CArrayCircuitInstanceViewPtr	apcPath;
	CViewPath*						pcViewPath;

	MakeTopPath(&apcPath, pcCircuitView);
	if (Find(&apcPath) != NULL)
	{
		apcPath.Kill();
		gcLogger.Error2(__METHOD__, " The circuit already has a top path.", NULL);
		return NULL;
	}
	pcViewPath = Add(&apcPath, NULL);
	apcPath.Kill();
	return pcViewPath;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CViewPath* CViewPaths::GetTop(CCircuitView* pcCircuitView)
{
	CArrayCircuitInstanceViewPtr	apcPath;
	CViewPath*						pcViewPath;

	MakeTopPath(&apcPath, pcCircuitView);
	pcViewPath = Find(&apcPath);
	apcPath.Kill();
	return pcViewPath;
}


//////////////////////////////////////////////////////////////////////////
//
// The path of the parent with the instance view on the end, as Logicim's
// ViewPaths.getViewPath; NULL if there isn't one.
//
//////////////////////////////////////////////////////////////////////////
CViewPath* CViewPaths::GetViewPath(CViewPath* pcParent, CCircuitInstanceView* pcInstanceView)
{
	CArrayCircuitInstanceViewPtr	apcPath;
	CViewPath*						pcViewPath;

	MakeChildPath(&apcPath, pcParent, pcInstanceView);
	pcViewPath = Find(&apcPath);
	apcPath.Kill();
	return pcViewPath;
}


//////////////////////////////////////////////////////////////////////////
//
// As GetViewPath, but makes the path if there isn't one.  Fails (NULL) if
// the instance's subcircuit is already on the parent path, which would
// put a subcircuit inside itself.
//
//////////////////////////////////////////////////////////////////////////
CViewPath* CViewPaths::GetOrAddViewPath(CViewPath* pcParent, CCircuitInstanceView* pcInstanceView)
{
	CArrayCircuitInstanceViewPtr	apcPath;
	CViewPath*						pcViewPath;
	CChars							sz;

	MakeChildPath(&apcPath, pcParent, pcInstanceView);
	pcViewPath = Find(&apcPath);
	if (pcViewPath == NULL)
	{
		if (pcParent->ContainsCircuitView(pcInstanceView->GetInstanceCircuitView()))
		{
			sz.Init();
			pcParent->AppendDescription(&sz);
			sz.Append(" -> ");
			pcInstanceView->AppendDescription(&sz);
			gcLogger.Error2(__METHOD__, " Path [", sz.Text(), "] would put a subcircuit inside itself.", NULL);
			sz.Kill();
		}
		else
		{
			pcViewPath = Add(&apcPath, pcParent);
		}
	}
	apcPath.Kill();
	return pcViewPath;
}


//////////////////////////////////////////////////////////////////////////
//
// The circuit on its own, into an uninitialised array.
//
//////////////////////////////////////////////////////////////////////////
void CViewPaths::MakeTopPath(CArrayCircuitInstanceViewPtr* papcPath, CCircuitView* pcCircuitView)
{
	papcPath->Init();
	papcPath->Add(pcCircuitView);
}


//////////////////////////////////////////////////////////////////////////
//
// The parent's views and then the instance view, into an uninitialised
// array.
//
//////////////////////////////////////////////////////////////////////////
void CViewPaths::MakeChildPath(CArrayCircuitInstanceViewPtr* papcPath, CViewPath* pcParent, CCircuitInstanceView* pcInstanceView)
{
	size	i;

	papcPath->Init();
	for (i = 0; i < pcParent->Length(); i++)
	{
		papcPath->Add(pcParent->Get(i));
	}
	papcPath->Add(pcInstanceView);
}


//////////////////////////////////////////////////////////////////////////
//
// Adds every path whose last circuit is the circuit view.
//
//////////////////////////////////////////////////////////////////////////
void CViewPaths::GetPathsEndingWith(CCircuitView* pcCircuitView, CArrayViewPathPtr* papcViewPaths)
{
	size		i;
	CViewPath*	pcViewPath;

	for (i = 0; i < mapcViewPaths.NumElements(); i++)
	{
		pcViewPath = mapcViewPaths.GetPtr(i);
		if (pcViewPath->GetCircuitView() == pcCircuitView)
		{
			papcViewPaths->Add(pcViewPath);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Kills every path through the instance, which is being taken out of its
// circuit.  Their simulations must already have been taken off them.
//
//////////////////////////////////////////////////////////////////////////
void CViewPaths::RemovePathsThrough(CCircuitInstanceView* pcInstanceView)
{
	size		i;
	size		j;
	CViewPath*	pcViewPath;
	bool		bThrough;

	for (i = mapcViewPaths.NumElements(); i > 0; i--)
	{
		pcViewPath = mapcViewPaths.GetPtr(i - 1);
		bThrough = false;
		for (j = 0; j < pcViewPath->Length(); j++)
		{
			if (pcViewPath->Get(j) == pcInstanceView)
			{
				bThrough = true;
				break;
			}
		}
		if (bThrough)
		{
			mapcViewPaths.RemoveAt(i - 1, true);
			SafeKill(pcViewPath);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CViewPaths::NumViewPaths(void)
{
	return mapcViewPaths.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CViewPath* CViewPaths::GetViewPath(size uiIndex)
{
	return mapcViewPaths.SafeGetPtr(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CViewPath* CViewPaths::Find(CArrayCircuitInstanceViewPtr* papcPath)
{
	size		i;
	CViewPath*	pcViewPath;

	for (i = 0; i < mapcViewPaths.NumElements(); i++)
	{
		pcViewPath = mapcViewPaths.GetPtr(i);
		if (pcViewPath->EqualsPath(papcPath))
		{
			return pcViewPath;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CViewPath* CViewPaths::Add(CArrayCircuitInstanceViewPtr* papcPath, CViewPath* pcPrevious)
{
	CViewPath*	pcViewPath;

	pcViewPath = NewMalloc<CViewPath>();
	pcViewPath->Init(papcPath, pcPrevious, this);
	mapcViewPaths.Add(pcViewPath);
	return pcViewPath;
}


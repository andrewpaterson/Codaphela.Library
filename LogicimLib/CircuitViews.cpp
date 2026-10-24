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
#include "BaseLib/StringHelper.h"
#include "SubcircuitInstanceView.h"
#include "CircuitViews.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitViews::Init(void)
{
	mapcCircuits.Init();
	mcViewPaths.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// The simulations keep their components and traces.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitViews::Kill(void)
{
	size			i;
	CCircuitView*	pcCircuit;

	mcViewPaths.Kill();
	for (i = 0; i < mapcCircuits.NumElements(); i++)
	{
		pcCircuit = mapcCircuits.GetPtr(i);
		SafeKill(pcCircuit);
	}
	mapcCircuits.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A new, empty circuit and its top path.  NULL if the type name is empty
// or another circuit has it.
//
//////////////////////////////////////////////////////////////////////////
CCircuitView* CCircuitViews::Add(const char* szTypeName)
{
	CCircuitView*	pcCircuit;

	if (StrEmpty(szTypeName))
	{
		gcLogger.Error2(__METHOD__, " A circuit needs a type name.", NULL);
		return NULL;
	}
	if (Get(szTypeName) != NULL)
	{
		gcLogger.Error2(__METHOD__, " There is already a circuit [", szTypeName, "].", NULL);
		return NULL;
	}

	pcCircuit = NewMalloc<CCircuitView>();
	pcCircuit->Init();
	pcCircuit->SetTypeName(szTypeName);
	mapcCircuits.Add(pcCircuit);
	mcViewPaths.AddTop(pcCircuit);
	return pcCircuit;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if there's no circuit of the type.
//
//////////////////////////////////////////////////////////////////////////
CCircuitView* CCircuitViews::Get(const char* szTypeName)
{
	size			i;
	CCircuitView*	pcCircuit;

	for (i = 0; i < mapcCircuits.NumElements(); i++)
	{
		pcCircuit = mapcCircuits.GetPtr(i);
		if (StringCompare(pcCircuit->GetTypeName(), szTypeName) == 0)
		{
			return pcCircuit;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCircuitViews::NumCircuits(void) { return mapcCircuits.NumElements(); }
CCircuitView* CCircuitViews::Get(size uiIndex) { return mapcCircuits.SafeGetPtr(uiIndex); }
CViewPaths* CCircuitViews::GetViewPaths(void) { return &mcViewPaths; }
CViewPath* CCircuitViews::GetTopPath(CCircuitView* pcCircuit) { return mcViewPaths.GetTop(pcCircuit); }


//////////////////////////////////////////////////////////////////////////
//
// Gives each instance of a subcircuit without its subcircuit the circuit
// of its type.  Fails if a type isn't here; the other instances still get
// theirs.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitViews::SetInstanceCircuitViews(void)
{
	size						i;
	size						j;
	CCircuitView*				pcCircuit;
	CCircuitInstanceView*		pcInstanceView;
	CSubcircuitInstanceView*	pcInstance;
	CCircuitView*				pcSubcircuit;
	bool						bResult;

	bResult = true;
	for (i = 0; i < mapcCircuits.NumElements(); i++)
	{
		pcCircuit = mapcCircuits.GetPtr(i);
		for (j = 0; j < pcCircuit->NumComponents(); j++)
		{
			pcInstanceView = pcCircuit->GetComponent(j)->GetCircuitInstanceView();
			if ((pcInstanceView != NULL) && (pcInstanceView->GetInstanceCircuitView() == NULL))
			{
				pcInstance = (CSubcircuitInstanceView*)pcCircuit->GetComponent(j);
				pcSubcircuit = Get(pcInstance->GetTypeName());
				if (pcSubcircuit == NULL)
				{
					bResult = gcLogger.Error2(__METHOD__, " Circuit [", pcCircuit->GetTypeName(), "] has an instance of unknown subcircuit [", pcInstance->GetTypeName(), "].", NULL);
				}
				else
				{
					bResult &= pcInstance->SetInstanceCircuitView(pcSubcircuit);
				}
			}
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitViews::SaveXML(CXMLWriter* pcWriter)
{
	size	i;

	pcWriter->Begin("Circuits");
	for (i = 0; i < mapcCircuits.NumElements(); i++)
	{
		mapcCircuits.GetPtr(i)->SaveXML(pcWriter);
	}
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
// Stops at the first circuit it can't read; what was read before it
// stays.  Every circuit needs a type, and no two the same.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitViews::LoadXML(CMarkupTag* pcCircuitsTag)
{
	STagIterator	sIter;
	CMarkupTag*		pcTag;
	char*			szTypeName;
	CCircuitView*	pcCircuit;

	if (!pcCircuitsTag->Is("Circuits"))
	{
		return gcLogger.Error2(__METHOD__, " Expected a [Circuits] element, not [", pcCircuitsTag->GetName(), "].", NULL);
	}

	pcTag = pcCircuitsTag->GetTag(&sIter);
	while (pcTag != NULL)
	{
		if (!pcTag->Is("Circuit"))
		{
			return gcLogger.Error2(__METHOD__, " Expected a [Circuit] element, not [", pcTag->GetName(), "].", NULL);
		}
		szTypeName = pcTag->GetStringAttribute("type");
		pcCircuit = Add(szTypeName);
		if (pcCircuit == NULL)
		{
			return false;
		}
		if (!pcCircuit->LoadXML(pcTag))
		{
			return false;
		}
		pcTag = pcCircuitsTag->GetNextTag(&sIter);
	}
	return SetInstanceCircuitViews();
}


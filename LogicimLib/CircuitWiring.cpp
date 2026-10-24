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
#include <stdlib.h>
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/StringHelper.h"
#include "MicroprocessorLib/CircuitSimulation.h"
#include "MicroprocessorLib/Circuit.h"
#include "CircuitView.h"
#include "CircuitWiring.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitWiring::Init(CCircuitSimulation* pcSimulation)
{
	mpcSimulation = pcSimulation;
	masLevels.Init();
	muiNumNodes = 0;
	maiParents.Init();
	maiNetWidths.Init();
	maiNetIndices.Init();
	muiNumNets = 0;
	mapcNetTraces = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The traces belong to the simulation's circuit.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitWiring::Kill(void)
{
	size	i;

	if (mapcNetTraces != NULL)
	{
		for (i = 0; i < muiNumNets; i++)
		{
			mapcNetTraces[i].Kill();
		}
		free(mapcNetTraces);
		mapcNetTraces = NULL;
	}
	maiNetIndices.Kill();
	maiNetWidths.Kill();
	maiParents.Kill();
	for (i = 0; i < masLevels.NumElements(); i++)
	{
		masLevels.Get(i)->apcPortViews.Kill();
	}
	masLevels.Kill();
	mpcSimulation = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Wires the circuit of the path and the subcircuits in it.  The traces are
// given to the wires and the simulation is started (if bStart) before the
// ports are wired, as Logicim's editor wires them, so each port sees its
// trace go from undriven to whatever drives it.  Gates only act on
// transitions; wired before the start, a supply driving a gate's input
// wouldn't be seen.  A simulation that's going to be loaded is wired
// without events; its state comes from the file.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitWiring::Wire(CViewPath* pcViewPath, bool bStart)
{
	size			i;
	size			uiNode;
	bool			bResult;
	SWiringLevel*	psLevel;

	bResult = AddLevels(pcViewPath, -1, NULL);

	uiNode = 0;
	for (i = 0; i < masLevels.NumElements(); i++)
	{
		psLevel = masLevels.Get(i);
		psLevel->uiFirstNode = uiNode;
		uiNode += NumLevelTraces(psLevel) + psLevel->apcPortViews.NumElements();
	}
	muiNumNodes = uiNode;
	for (i = 0; i < muiNumNodes; i++)
	{
		maiParents.Add((int)i);
	}

	for (i = 0; i < masLevels.NumElements(); i++)
	{
		JoinLevel(masLevels.Get(i));
	}
	for (i = 0; i < masLevels.NumElements(); i++)
	{
		JoinPins(masLevels.Get(i));
	}
	bResult &= FindNets();
	CreateNetTraces();

	for (i = 0; i < masLevels.NumElements(); i++)
	{
		SetLevelTraces(masLevels.Get(i));
	}
	if (bStart)
	{
		mpcSimulation->SimulationStarted();
	}
	for (i = 0; i < masLevels.NumElements(); i++)
	{
		bResult &= ConnectLevel(masLevels.Get(i), bStart);
	}
	for (i = 0; i < masLevels.NumElements(); i++)
	{
		masLevels.Get(i)->pcCircuit->CalculateJunctions();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A level for the path and then, depth first, for each subcircuit
// instance in its circuit.  Levels are added as they're found, so a
// pointer to one is only good until the next is added.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitWiring::AddLevels(CViewPath* pcViewPath, int iParent, CCircuitInstanceView* pcInstanceView)
{
	SWiringLevel*			psLevel;
	CCircuitView*			pcCircuit;
	size					i;
	int						iLevel;
	CCircuitInstanceView*	pcChildInstance;
	CViewPath*				pcChild;
	bool					bResult;

	pcCircuit = pcViewPath->GetCircuitView();
	iLevel = (int)masLevels.NumElements();
	psLevel = masLevels.Add();
	psLevel->pcViewPath = pcViewPath;
	psLevel->pcCircuit = pcCircuit;
	psLevel->apcPortViews.Init();
	psLevel->uiFirstNode = 0;
	psLevel->iParent = iParent;
	psLevel->pcInstanceView = pcInstanceView;
	pcCircuit->CollectPortViews(&psLevel->apcPortViews);

	bResult = true;
	for (i = 0; i < pcCircuit->NumComponents(); i++)
	{
		pcChildInstance = pcCircuit->GetComponent(i)->GetCircuitInstanceView();
		if (pcChildInstance != NULL)
		{
			pcChild = pcViewPath->GetChild(pcChildInstance);
			if ((pcChild == NULL) || !pcChild->ContainsCircuitSimulation(mpcSimulation))
			{
				bResult = false;
			}
			else
			{
				bResult &= AddLevels(pcChild, iLevel, pcChildInstance);
			}
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// In a circuit, wires that touch, port views on a wire, port views on the
// same grid point and the port views of views with the same connection
// name are joined.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitWiring::JoinLevel(SWiringLevel* psLevel)
{
	CCircuitView*		pcCircuit;
	CArrayPortViewPtr*	papcPortViews;
	size				uiTraces;
	size				i;
	size				j;
	int					iFirst;
	CPortView*			pcPortView;
	CPortView*			pcOtherPortView;
	const char*			szName;
	const char*			szOtherName;

	pcCircuit = psLevel->pcCircuit;
	papcPortViews = &psLevel->apcPortViews;
	uiTraces = NumLevelTraces(psLevel);
	iFirst = (int)psLevel->uiFirstNode;
	for (i = 0; i < uiTraces; i++)
	{
		for (j = i + 1; j < uiTraces; j++)
		{
			if (pcCircuit->GetTrace(i)->Touches(pcCircuit->GetTrace(j)))
			{
				Join(&maiParents, iFirst + (int)i, iFirst + (int)j);
			}
		}
	}
	for (i = 0; i < papcPortViews->NumElements(); i++)
	{
		pcPortView = papcPortViews->GetPtr(i);
		for (j = 0; j < uiTraces; j++)
		{
			if (pcCircuit->GetTrace(j)->Contains(pcPortView->GetGridPosition()))
			{
				Join(&maiParents, iFirst + (int)(uiTraces + i), iFirst + (int)j);
			}
		}
		szName = pcPortView->GetOwner()->GetConnectionName();
		for (j = i + 1; j < papcPortViews->NumElements(); j++)
		{
			pcOtherPortView = papcPortViews->GetPtr(j);
			if (pcOtherPortView->GetGridPosition() == pcPortView->GetGridPosition())
			{
				Join(&maiParents, iFirst + (int)(uiTraces + i), iFirst + (int)(uiTraces + j));
			}
			else if (szName != NULL)
			{
				szOtherName = pcOtherPortView->GetOwner()->GetConnectionName();
				if ((szOtherName != NULL) && (StringCompare(szName, szOtherName) == 0))
				{
					Join(&maiParents, iFirst + (int)(uiTraces + i), iFirst + (int)(uiTraces + j));
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Joins the port view of each of an instance's pins, in its parent's
// circuit, to the port view of the pin in the subcircuit.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitWiring::JoinPins(SWiringLevel* psLevel)
{
	SWiringLevel*		psParent;
	CArrayPinPortJoin	asJoins;
	size				i;
	SPinPortJoin*		psJoin;
	int					iOutside;
	int					iInside;

	if (psLevel->iParent < 0)
	{
		return;
	}

	psParent = masLevels.Get(psLevel->iParent);
	asJoins.Init();
	psLevel->pcInstanceView->GetPinPortJoins(&asJoins);
	for (i = 0; i < asJoins.NumElements(); i++)
	{
		psJoin = asJoins.Get(i);
		iOutside = GetPortViewNode(psParent, psJoin->pcOutside);
		iInside = GetPortViewNode(psLevel, psJoin->pcInside);
		if ((iOutside != -1) && (iInside != -1))
		{
			Join(&maiParents, iOutside, iInside);
		}
	}
	asJoins.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A net is as wide as its widest port view; one with no ports gets a
// single trace to show it's undriven.  Roots that are nets (anything with
// a wire, and port views joined to another) are numbered in maiNetIndices
// (-1 for the others) and their widths are in maiNetWidths, both by root.
// Fails if ports of different widths are joined.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitWiring::FindNets(void)
{
	size			i;
	size			j;
	int				iRoot;
	int				iWidth;
	SWiringLevel*	psLevel;
	CPortView*		pcPortView;
	bool			bResult;
	size			uiTraces;

	bResult = true;
	for (i = 0; i < muiNumNodes; i++)
	{
		maiNetWidths.Add(0);
		maiNetIndices.Add(-1);
	}
	for (i = 0; i < masLevels.NumElements(); i++)
	{
		psLevel = masLevels.Get(i);
		uiTraces = NumLevelTraces(psLevel);
		for (j = 0; j < psLevel->apcPortViews.NumElements(); j++)
		{
			pcPortView = psLevel->apcPortViews.GetPtr(j);
			iRoot = FindRoot(&maiParents, (int)(psLevel->uiFirstNode + uiTraces + j));
			iWidth = (int)pcPortView->NumPorts(psLevel->pcViewPath, mpcSimulation);
			if ((maiNetWidths.GetValue(iRoot) != 0) && (iWidth != 0) && (maiNetWidths.GetValue(iRoot) != iWidth))
			{
				gcLogger.Error2(__METHOD__, " Ports of different widths are connected at [", IntToString(pcPortView->GetGridPosition().x), ", ", IntToString(pcPortView->GetGridPosition().y), "].", NULL);
				bResult = false;
			}
			if (iWidth > maiNetWidths.GetValue(iRoot))
			{
				maiNetWidths.SetValue(iRoot, iWidth);
			}
		}
	}

	muiNumNets = 0;
	for (i = 0; i < muiNumNodes; i++)
	{
		iRoot = FindRoot(&maiParents, (int)i);
		if (IsTraceNode(i) || (iRoot != (int)i))
		{
			if (maiNetIndices.GetValue(iRoot) == -1)
			{
				maiNetIndices.SetValue(iRoot, (int)muiNumNets);
				muiNumNets++;
			}
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The traces of each net by bit, into mapcNetTraces.  Bits the components
// join (GetBitJoins) share a trace; a join to a port view that isn't on a
// net joins nothing.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitWiring::CreateNetTraces(void)
{
	size					i;
	size					j;
	size					k;
	int						iRoot;
	int						iNet;
	CArrayInt				aiNetFirstBit;
	CArrayInt				aiNetWidth;
	CArrayInt				aiBitParents;
	CArrayTraceViewTracePtr	apcBitTraces;
	int						iBits;
	int						iWidth;
	CArrayPortBitJoin		asJoins;
	SPortBitJoin*			psJoin;
	int						iFromNet;
	int						iToNet;
	int						iBit;
	CTrace*					pcTrace;
	SWiringLevel*			psLevel;
	CCircuit*				pcCircuit;

	mapcNetTraces = (CArrayTraceViewTracePtr*)malloc(sizeof(CArrayTraceViewTracePtr) * (muiNumNets + 1));
	for (i = 0; i < muiNumNets; i++)
	{
		mapcNetTraces[i].Init();
	}

	aiNetFirstBit.Init();
	aiNetWidth.Init();
	for (i = 0; i < muiNumNets; i++)
	{
		aiNetFirstBit.Add(0);
		aiNetWidth.Add(0);
	}
	for (i = 0; i < muiNumNodes; i++)
	{
		iRoot = FindRoot(&maiParents, (int)i);
		if ((int)i == iRoot)
		{
			iNet = maiNetIndices.GetValue(iRoot);
			if (iNet != -1)
			{
				iWidth = maiNetWidths.GetValue(iRoot);
				aiNetWidth.SetValue(iNet, (iWidth == 0) ? 1 : iWidth);
			}
		}
	}
	iBits = 0;
	for (i = 0; i < muiNumNets; i++)
	{
		aiNetFirstBit.SetValue(i, iBits);
		iBits += aiNetWidth.GetValue(i);
	}

	aiBitParents.Init();
	for (iBit = 0; iBit < iBits; iBit++)
	{
		aiBitParents.Add(iBit);
	}
	for (i = 0; i < masLevels.NumElements(); i++)
	{
		psLevel = masLevels.Get(i);
		asJoins.Init();
		for (j = 0; j < psLevel->pcCircuit->NumComponents(); j++)
		{
			psLevel->pcCircuit->GetComponent(j)->GetBitJoins(&asJoins);
		}
		for (j = 0; j < asJoins.NumElements(); j++)
		{
			psJoin = asJoins.Get(j);
			iFromNet = GetPortViewNet(psLevel, psJoin->pcFrom);
			iToNet = GetPortViewNet(psLevel, psJoin->pcTo);
			if ((iFromNet != -1) && (iToNet != -1) &&
				(psJoin->uiFromBit < (size)aiNetWidth.GetValue(iFromNet)) && (psJoin->uiToBit < (size)aiNetWidth.GetValue(iToNet)))
			{
				Join(&aiBitParents, aiNetFirstBit.GetValue(iFromNet) + (int)psJoin->uiFromBit, aiNetFirstBit.GetValue(iToNet) + (int)psJoin->uiToBit);
			}
		}
		asJoins.Kill();
	}

	pcCircuit = mpcSimulation->GetCircuit();
	apcBitTraces.Init();
	for (iBit = 0; iBit < iBits; iBit++)
	{
		apcBitTraces.Add(NULL);
	}
	for (i = 0; i < muiNumNets; i++)
	{
		for (k = 0; k < (size)aiNetWidth.GetValue(i); k++)
		{
			iRoot = FindRoot(&aiBitParents, aiNetFirstBit.GetValue(i) + (int)k);
			pcTrace = apcBitTraces.GetPtr(iRoot);
			if (pcTrace == NULL)
			{
				pcTrace = pcCircuit->AddTrace();
				apcBitTraces.SetPtr(iRoot, pcTrace);
			}
			mapcNetTraces[i].Add(pcTrace);
		}
	}

	apcBitTraces.Kill();
	aiBitParents.Kill();
	aiNetWidth.Kill();
	aiNetFirstBit.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Every wire is on a net.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitWiring::SetLevelTraces(SWiringLevel* psLevel)
{
	size	i;
	int		iNet;

	for (i = 0; i < NumLevelTraces(psLevel); i++)
	{
		iNet = maiNetIndices.GetValue(FindRoot(&maiParents, (int)(psLevel->uiFirstNode + i)));
		psLevel->pcCircuit->GetTrace(i)->SetTraces(psLevel->pcViewPath, mpcSimulation, &mapcNetTraces[iNet]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Connects the ports of each port view on a net to its traces, live if
// the simulation was started.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitWiring::ConnectLevel(SWiringLevel* psLevel, bool bStart)
{
	size			i;
	size			k;
	int				iNet;
	size			uiTraces;
	CPortView*		pcPortView;
	CTimeline*		pcTimeline;
	CViewPath*		pcViewPath;
	bool			bResult;

	pcTimeline = mpcSimulation->GetTimeline();
	pcViewPath = psLevel->pcViewPath;
	uiTraces = NumLevelTraces(psLevel);
	bResult = true;
	for (i = 0; i < psLevel->apcPortViews.NumElements(); i++)
	{
		pcPortView = psLevel->apcPortViews.GetPtr(i);
		iNet = maiNetIndices.GetValue(FindRoot(&maiParents, (int)(psLevel->uiFirstNode + uiTraces + i)));
		if (iNet != -1)
		{
			for (k = 0; k < pcPortView->NumPorts(pcViewPath, mpcSimulation); k++)
			{
				if (bStart)
				{
					bResult &= pcPortView->GetPort(pcViewPath, mpcSimulation, k)->ConnectLive(mapcNetTraces[iNet].GetPtr(k), pcTimeline);
				}
				else
				{
					bResult &= pcPortView->GetPort(pcViewPath, mpcSimulation, k)->Connect(mapcNetTraces[iNet].GetPtr(k));
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
size CCircuitWiring::NumLevelTraces(SWiringLevel* psLevel)
{
	return psLevel->pcCircuit->NumTraces();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the node is a wire rather than a port view.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitWiring::IsTraceNode(size uiNode)
{
	size			i;
	SWiringLevel*	psLevel;

	for (i = 0; i < masLevels.NumElements(); i++)
	{
		psLevel = masLevels.Get(i);
		if ((uiNode >= psLevel->uiFirstNode) && (uiNode < psLevel->uiFirstNode + NumLevelTraces(psLevel) + psLevel->apcPortViews.NumElements()))
		{
			return uiNode < psLevel->uiFirstNode + NumLevelTraces(psLevel);
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The node of the port view in the level, or -1.
//
//////////////////////////////////////////////////////////////////////////
int CCircuitWiring::GetPortViewNode(SWiringLevel* psLevel, CPortView* pcPortView)
{
	size	i;

	for (i = 0; i < psLevel->apcPortViews.NumElements(); i++)
	{
		if (psLevel->apcPortViews.GetPtr(i) == pcPortView)
		{
			return (int)(psLevel->uiFirstNode + NumLevelTraces(psLevel) + i);
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The net the port view in the level is on, or -1.
//
//////////////////////////////////////////////////////////////////////////
int CCircuitWiring::GetPortViewNet(SWiringLevel* psLevel, CPortView* pcPortView)
{
	int		iNode;

	iNode = GetPortViewNode(psLevel, pcPortView);
	if (iNode == -1)
	{
		return -1;
	}
	return maiNetIndices.GetValue(FindRoot(&maiParents, iNode));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCircuitWiring::FindRoot(CArrayInt* paiParents, int iNode)
{
	int		iParent;

	iParent = paiParents->GetValue(iNode);
	while (iParent != iNode)
	{
		//Halve the path as it's walked.
		paiParents->SetValue(iNode, paiParents->GetValue(iParent));
		iNode = iParent;
		iParent = paiParents->GetValue(iNode);
	}
	return iNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitWiring::Join(CArrayInt* paiParents, int iNode1, int iNode2)
{
	int		iRoot1;
	int		iRoot2;

	iRoot1 = FindRoot(paiParents, iNode1);
	iRoot2 = FindRoot(paiParents, iNode2);
	if (iRoot1 != iRoot2)
	{
		paiParents->SetValue(iRoot2, iRoot1);
	}
}


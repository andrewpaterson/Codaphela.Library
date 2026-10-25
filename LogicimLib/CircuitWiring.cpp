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
#include "MicroprocessorLib/Port.h"
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
	maiNetFirstBits.Init();
	maiNetBitWidths.Init();
	maiBitParents.Init();
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
	maiBitParents.Kill();
	maiNetBitWidths.Kill();
	maiNetFirstBits.Kill();
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
	size					i;
	bool					bResult;
	CArrayTraceViewTracePtr	apcBitTraces;

	bResult = JoinNodes(pcViewPath);
	FindBits();
	apcBitTraces.Init();
	AddBitTraces(&apcBitTraces);
	CreateNetTraces(&apcBitTraces);
	apcBitTraces.Kill();

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
// Wires the running simulation of the path again after its circuits have
// been edited, as Logicim's editor makes new traces for the connections
// an edit touched.  The nets are found afresh, but each set of joined
// bits keeps the trace most of its ports (and wires) are already on, so
// ports whose nets didn't change aren't disturbed.  Every port on the
// wrong trace is disconnected first, then they're all connected live to
// their new traces.  Components added to the circuits must already be
// made and started, and those taken out destroyed.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitWiring::Rewire(CViewPath* pcViewPath)
{
	size					i;
	bool					bResult;
	CArrayTraceViewTracePtr	apcBitTraces;

	bResult = JoinNodes(pcViewPath);
	FindBits();
	apcBitTraces.Init();
	ReuseBitTraces(&apcBitTraces);
	AddBitTraces(&apcBitTraces);
	CreateNetTraces(&apcBitTraces);
	apcBitTraces.Kill();

	for (i = 0; i < masLevels.NumElements(); i++)
	{
		DisconnectLevel(masLevels.Get(i));
	}
	for (i = 0; i < masLevels.NumElements(); i++)
	{
		SetLevelTraces(masLevels.Get(i));
	}
	for (i = 0; i < masLevels.NumElements(); i++)
	{
		bResult &= ConnectLevel(masLevels.Get(i), true);
	}
	for (i = 0; i < masLevels.NumElements(); i++)
	{
		masLevels.Get(i)->pcCircuit->CalculateJunctions();
	}
	RemoveUnusedTraces();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes the levels and joins their nodes into nets.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitWiring::JoinNodes(CViewPath* pcViewPath)
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
// Numbers the bits of the nets, from maiNetFirstBits for each net, and
// joins those the components join (GetBitJoins) in maiBitParents; each
// set of joined bits gets one trace.  A join to a port view that isn't on
// a net joins nothing.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitWiring::FindBits(void)
{
	size					i;
	size					j;
	int						iRoot;
	int						iNet;
	int						iBits;
	int						iWidth;
	CArrayPortBitJoin		asJoins;
	SPortBitJoin*			psJoin;
	int						iFromNet;
	int						iToNet;
	int						iBit;
	SWiringLevel*			psLevel;

	for (i = 0; i < muiNumNets; i++)
	{
		maiNetFirstBits.Add(0);
		maiNetBitWidths.Add(0);
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
				maiNetBitWidths.SetValue(iNet, (iWidth == 0) ? 1 : iWidth);
			}
		}
	}
	iBits = 0;
	for (i = 0; i < muiNumNets; i++)
	{
		maiNetFirstBits.SetValue(i, iBits);
		iBits += maiNetBitWidths.GetValue(i);
	}

	for (iBit = 0; iBit < iBits; iBit++)
	{
		maiBitParents.Add(iBit);
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
				(psJoin->uiFromBit < (size)maiNetBitWidths.GetValue(iFromNet)) && (psJoin->uiToBit < (size)maiNetBitWidths.GetValue(iToNet)))
			{
				Join(&maiBitParents, maiNetFirstBits.GetValue(iFromNet) + (int)psJoin->uiFromBit, maiNetFirstBits.GetValue(iToNet) + (int)psJoin->uiToBit);
			}
		}
		asJoins.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A trace the ports (or wires) of a set of joined bits are on, and how
// many of them.
//
//////////////////////////////////////////////////////////////////////////
struct SBitTraceVote
{
	int		iRoot;
	CTrace*	pcTrace;
	int		iVotes;
};


typedef CArrayTemplate<SBitTraceVote>	CArrayBitTraceVote;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AddBitTraceVote(CArrayBitTraceVote* pasVotes, int iRoot, CTrace* pcTrace)
{
	size			i;
	SBitTraceVote*	psVote;

	if (pcTrace == NULL)
	{
		return;
	}
	for (i = 0; i < pasVotes->NumElements(); i++)
	{
		psVote = pasVotes->Get(i);
		if ((psVote->iRoot == iRoot) && (psVote->pcTrace == pcTrace))
		{
			psVote->iVotes++;
			return;
		}
	}
	psVote = pasVotes->Add();
	psVote->iRoot = iRoot;
	psVote->pcTrace = pcTrace;
	psVote->iVotes = 1;
}


//////////////////////////////////////////////////////////////////////////
//
// Most votes first.
//
//////////////////////////////////////////////////////////////////////////
static int CompareBitTraceVotes(const void* pv1, const void* pv2)
{
	return ((SBitTraceVote*)pv2)->iVotes - ((SBitTraceVote*)pv1)->iVotes;
}


//////////////////////////////////////////////////////////////////////////
//
// Gives each set of joined bits (by root) the trace most of its ports and
// wires are already on, if another set hasn't got it.  Sets on no trace
// are left NULL.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitWiring::ReuseBitTraces(CArrayTraceViewTracePtr* papcBitTraces)
{
	size						i;
	size						j;
	size						k;
	SWiringLevel*				psLevel;
	size						uiTraces;
	int							iNet;
	CPortView*					pcPortView;
	CArrayTraceViewTracePtr*	papcOldTraces;
	CArrayBitTraceVote			asVotes;
	SBitTraceVote*				psVote;

	asVotes.Init();
	for (i = 0; i < masLevels.NumElements(); i++)
	{
		psLevel = masLevels.Get(i);
		uiTraces = NumLevelTraces(psLevel);
		for (j = 0; j < psLevel->apcPortViews.NumElements(); j++)
		{
			iNet = maiNetIndices.GetValue(FindRoot(&maiParents, (int)(psLevel->uiFirstNode + uiTraces + j)));
			if (iNet != -1)
			{
				pcPortView = psLevel->apcPortViews.GetPtr(j);
				for (k = 0; (k < pcPortView->NumPorts(psLevel->pcViewPath, mpcSimulation)) && (k < (size)maiNetBitWidths.GetValue(iNet)); k++)
				{
					AddBitTraceVote(&asVotes, GetBitRoot(iNet, k), pcPortView->GetPort(psLevel->pcViewPath, mpcSimulation, k)->GetTrace());
				}
			}
		}
		for (j = 0; j < uiTraces; j++)
		{
			iNet = maiNetIndices.GetValue(FindRoot(&maiParents, (int)(psLevel->uiFirstNode + j)));
			papcOldTraces = psLevel->pcCircuit->GetTrace(j)->GetTraces(psLevel->pcViewPath, mpcSimulation);
			if (papcOldTraces != NULL)
			{
				for (k = 0; (k < papcOldTraces->NumElements()) && (k < (size)maiNetBitWidths.GetValue(iNet)); k++)
				{
					AddBitTraceVote(&asVotes, GetBitRoot(iNet, k), papcOldTraces->GetPtr(k));
				}
			}
		}
	}

	for (i = 0; i < maiBitParents.NumElements(); i++)
	{
		papcBitTraces->Add(NULL);
	}
	asVotes.TimSort(CompareBitTraceVotes);
	for (i = 0; i < asVotes.NumElements(); i++)
	{
		psVote = asVotes.Get(i);
		if ((papcBitTraces->GetPtr(psVote->iRoot) == NULL) && (papcBitTraces->Find(psVote->pcTrace) == ARRAY_ELEMENT_NOT_FOUND))
		{
			papcBitTraces->SetPtr(psVote->iRoot, psVote->pcTrace);
		}
	}
	asVotes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A new trace for each set of joined bits (by root) that hasn't got one.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitWiring::AddBitTraces(CArrayTraceViewTracePtr* papcBitTraces)
{
	size		i;
	size		k;
	int			iRoot;
	CCircuit*	pcCircuit;

	while (papcBitTraces->NumElements() < maiBitParents.NumElements())
	{
		papcBitTraces->Add(NULL);
	}
	pcCircuit = mpcSimulation->GetCircuit();
	for (i = 0; i < muiNumNets; i++)
	{
		for (k = 0; k < (size)maiNetBitWidths.GetValue(i); k++)
		{
			iRoot = GetBitRoot((int)i, k);
			if (papcBitTraces->GetPtr(iRoot) == NULL)
			{
				papcBitTraces->SetPtr(iRoot, pcCircuit->AddTrace());
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The traces of each net by bit, into mapcNetTraces, from those of the
// sets of joined bits.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitWiring::CreateNetTraces(CArrayTraceViewTracePtr* papcBitTraces)
{
	size	i;
	size	k;

	mapcNetTraces = (CArrayTraceViewTracePtr*)malloc(sizeof(CArrayTraceViewTracePtr) * (muiNumNets + 1));
	for (i = 0; i < muiNumNets; i++)
	{
		mapcNetTraces[i].Init();
		for (k = 0; k < (size)maiNetBitWidths.GetValue(i); k++)
		{
			mapcNetTraces[i].Add(papcBitTraces->GetPtr(GetBitRoot((int)i, k)));
		}
	}
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
// Disconnects, live, each port of the level that's on a trace other than
// its net's (or on any trace when it's on no net).
//
//////////////////////////////////////////////////////////////////////////
void CCircuitWiring::DisconnectLevel(SWiringLevel* psLevel)
{
	size			i;
	size			k;
	int				iNet;
	size			uiTraces;
	CPortView*		pcPortView;
	CPort*			pcPort;
	CTimeline*		pcTimeline;
	CViewPath*		pcViewPath;

	pcTimeline = mpcSimulation->GetTimeline();
	pcViewPath = psLevel->pcViewPath;
	uiTraces = NumLevelTraces(psLevel);
	for (i = 0; i < psLevel->apcPortViews.NumElements(); i++)
	{
		pcPortView = psLevel->apcPortViews.GetPtr(i);
		iNet = maiNetIndices.GetValue(FindRoot(&maiParents, (int)(psLevel->uiFirstNode + uiTraces + i)));
		for (k = 0; k < pcPortView->NumPorts(pcViewPath, mpcSimulation); k++)
		{
			pcPort = pcPortView->GetPort(pcViewPath, mpcSimulation, k);
			if ((pcPort->GetTrace() != NULL) && ((iNet == -1) || (pcPort->GetTrace() != mapcNetTraces[iNet].GetPtr(k))))
			{
				pcPort->Disconnect(pcTimeline);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Connects the ports of each port view on a net to its traces, live if
// the simulation was started.  Ports already on their traces are left.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitWiring::ConnectLevel(SWiringLevel* psLevel, bool bStart)
{
	size			i;
	size			k;
	int				iNet;
	size			uiTraces;
	CPortView*		pcPortView;
	CPort*			pcPort;
	CTrace*			pcTrace;
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
				pcPort = pcPortView->GetPort(pcViewPath, mpcSimulation, k);
				pcTrace = mapcNetTraces[iNet].GetPtr(k);
				if (pcPort->GetTrace() == pcTrace)
				{
					continue;
				}
				if (bStart)
				{
					bResult &= pcPort->ConnectLive(pcTrace, pcTimeline);
				}
				else
				{
					bResult &= pcPort->Connect(pcTrace);
				}
			}
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Removes the traces of the simulation's circuit left with no ports and
// on no net.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitWiring::RemoveUnusedTraces(void)
{
	CCircuit*	pcCircuit;
	CTrace*		pcTrace;
	size		i;

	pcCircuit = mpcSimulation->GetCircuit();
	for (i = pcCircuit->NumTraces(); i > 0; i--)
	{
		pcTrace = pcCircuit->GetTrace(i - 1);
		if ((pcTrace->NumConnectedPorts() == 0) && !IsNetTrace(pcTrace))
		{
			pcCircuit->RemoveTrace(pcTrace);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitWiring::IsNetTrace(CTrace* pcTrace)
{
	size	i;

	for (i = 0; i < muiNumNets; i++)
	{
		if (mapcNetTraces[i].Find(pcTrace) != ARRAY_ELEMENT_NOT_FOUND)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The set of joined bits the net's bit is in.
//
//////////////////////////////////////////////////////////////////////////
int CCircuitWiring::GetBitRoot(int iNet, size uiBit)
{
	return FindRoot(&maiBitParents, maiNetFirstBits.GetValue(iNet) + (int)uiBit);
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


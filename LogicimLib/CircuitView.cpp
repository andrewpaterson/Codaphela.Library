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
#include "BaseLib/LogString.h"
#include "BaseLib/StringHelper.h"
#include "MicroprocessorLib/CircuitSimulation.h"
#include "MicroprocessorLib/SubcircuitSimulation.h"
#include "MicroprocessorLib/Circuit.h"
#include "ComponentViewFactory.h"
#include "CircuitView.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::Init(void)
{
	mapcComponents.Init();
	mapcTraces.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// The simulation keeps its components and traces.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::Kill(void)
{
	size				i;
	CComponentView*		pcComponent;
	CTraceView*			pcTrace;

	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		pcTrace = mapcTraces.GetPtr(i);
		SafeKill(pcTrace);
	}
	mapcTraces.Kill();
	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		pcComponent = mapcComponents.GetPtr(i);
		SafeKill(pcComponent);
	}
	mapcComponents.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The circuit view frees the component when it's killed.
//
//////////////////////////////////////////////////////////////////////////
CComponentView* CCircuitView::Add(CComponentView* pcComponent)
{
	mapcComponents.Add(pcComponent);
	return pcComponent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTraceView* CCircuitView::AddTrace(int32 x0, int32 y0, int32 x1, int32 y1)
{
	CTraceView*		pcTrace;

	pcTrace = NewMalloc<CTraceView>();
	pcTrace->Init(x0, y0, x1, y1);
	mapcTraces.Add(pcTrace);
	return pcTrace;
}


//////////////////////////////////////////////////////////////////////////
//
// A wire from one point to another, horizontal first and then vertical if
// they aren't in a line.  Returns the last piece.
//
//////////////////////////////////////////////////////////////////////////
CTraceView* CCircuitView::AddTraces(int32 x0, int32 y0, int32 x1, int32 y1)
{
	if ((x0 == x1) || (y0 == y1))
	{
		return AddTrace(x0, y0, x1, y1);
	}
	AddTrace(x0, y0, x1, y0);
	return AddTrace(x1, y0, x1, y1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCircuitView::NumComponents(void) { return mapcComponents.NumElements(); }
CComponentView* CCircuitView::GetComponent(size uiIndex) { return mapcComponents.SafeGetPtr(uiIndex); }
size CCircuitView::NumTraces(void) { return mapcTraces.NumElements(); }
CTraceView* CCircuitView::GetTrace(size uiIndex) { return mapcTraces.SafeGetPtr(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::CollectPortViews(CArrayPortViewPtr* papcPortViews)
{
	size				i;
	size				j;
	CComponentView*		pcComponent;

	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		pcComponent = mapcComponents.GetPtr(i);
		for (j = 0; j < pcComponent->NumPortViews(); j++)
		{
			papcPortViews->Add(pcComponent->GetPortView(j));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCircuitView::FindRoot(CArrayInt* paiParents, int iNode)
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
void CCircuitView::Join(CArrayInt* paiParents, int iNode1, int iNode2)
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


//////////////////////////////////////////////////////////////////////////
//
// Makes every component in the simulation's top subcircuit, starts the
// simulation (if bStart) and wires the nets.  The circuit view must not be
// connected to another simulation.
//
// Nets are found in two steps.  Wires, port views and tunnels are joined
// into nets, each a bus as wide as its widest port view.  Then the bits of
// the nets that components join (a splitter joins each bit of its bus to a
// bit of one of its ends) are joined, and each set of joined bits gets one
// trace.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitView::CreateSimulation(CCircuitSimulation* pcSimulation, CFamilyVoltageConfigurationStore* pcFamilies, bool bStart)
{
	CArrayPortViewPtr			apcPortViews;
	CArrayInt					aiParents;
	CArrayInt					aiNetWidths;
	CArrayInt					aiNetIndices;
	size						uiTraces;
	size						uiNodes;
	size						uiNumNets;
	size						i;
	size						k;
	int							iNet;
	CPortView*					pcPortView;
	CComponentView*				pcComponent;
	CSubcircuitSimulation*		pcTop;
	CTimeline*					pcTimeline;
	bool						bResult;
	CArrayTraceViewTracePtr*	papcNetTraces;

	pcTop = pcSimulation->GetSubcircuitTopSimulation();

	bResult = true;
	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		pcComponent = mapcComponents.GetPtr(i);
		if (pcComponent->MakesComponent())
		{
			if (pcComponent->CreateComponent(pcTop, pcFamilies) == NULL)
			{
				bResult = false;
			}
		}
	}

	apcPortViews.Init();
	CollectPortViews(&apcPortViews);
	uiTraces = mapcTraces.NumElements();
	uiNodes = uiTraces + apcPortViews.NumElements();

	aiParents.Init();
	for (i = 0; i < uiNodes; i++)
	{
		aiParents.Add((int)i);
	}
	JoinNodes(&aiParents, &apcPortViews);
	bResult &= FindNets(&aiParents, &apcPortViews, &aiNetWidths, &aiNetIndices, &uiNumNets);

	papcNetTraces = (CArrayTraceViewTracePtr*)malloc(sizeof(CArrayTraceViewTracePtr) * (uiNumNets + 1));
	for (i = 0; i < uiNumNets; i++)
	{
		papcNetTraces[i].Init();
	}
	CreateNetTraces(pcSimulation->GetCircuit(), &aiParents, &apcPortViews, &aiNetWidths, &aiNetIndices, uiNumNets, papcNetTraces);

	for (i = 0; i < uiTraces; i++)
	{
		iNet = aiNetIndices.GetValue(FindRoot(&aiParents, (int)i));
		mapcTraces.GetPtr(i)->SetTraces(&papcNetTraces[iNet]);
	}
	//Started before the ports are wired, as Logicim's editor wires them, so each port sees its trace go from undriven to
	//whatever drives it.  Gates only act on transitions; wired before the start, a supply driving a gate's input
	//wouldn't be seen.  A simulation that's going to be loaded is wired without events; its state comes from the file.
	if (bStart)
	{
		pcSimulation->SimulationStarted();
	}
	pcTimeline = pcSimulation->GetTimeline();
	for (i = 0; i < apcPortViews.NumElements(); i++)
	{
		pcPortView = apcPortViews.GetPtr(i);
		iNet = aiNetIndices.GetValue(FindRoot(&aiParents, (int)(uiTraces + i)));
		if (iNet != -1)
		{
			for (k = 0; k < pcPortView->NumPorts(); k++)
			{
				if (bStart)
				{
					bResult &= pcPortView->GetPort(k)->ConnectLive(papcNetTraces[iNet].GetPtr(k), pcTimeline);
				}
				else
				{
					bResult &= pcPortView->GetPort(k)->Connect(papcNetTraces[iNet].GetPtr(k));
				}
			}
		}
	}

	for (i = 0; i < uiNumNets; i++)
	{
		papcNetTraces[i].Kill();
	}
	free(papcNetTraces);
	aiNetIndices.Kill();
	aiNetWidths.Kill();
	aiParents.Kill();
	apcPortViews.Kill();

	CalculateJunctions();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Nodes are the wires and then the port views.  Wires that touch, port
// views on a wire, port views on the same grid point and the port views of
// views with the same connection name are joined.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::JoinNodes(CArrayInt* paiParents, CArrayPortViewPtr* papcPortViews)
{
	size			uiTraces;
	size			i;
	size			j;
	CPortView*		pcPortView;
	CPortView*		pcOtherPortView;
	const char*		szName;
	const char*		szOtherName;

	uiTraces = mapcTraces.NumElements();
	for (i = 0; i < uiTraces; i++)
	{
		for (j = i + 1; j < uiTraces; j++)
		{
			if (mapcTraces.GetPtr(i)->Touches(mapcTraces.GetPtr(j)))
			{
				Join(paiParents, (int)i, (int)j);
			}
		}
	}
	for (i = 0; i < papcPortViews->NumElements(); i++)
	{
		pcPortView = papcPortViews->GetPtr(i);
		for (j = 0; j < uiTraces; j++)
		{
			if (mapcTraces.GetPtr(j)->Contains(pcPortView->GetGridPosition()))
			{
				Join(paiParents, (int)(uiTraces + i), (int)j);
			}
		}
		szName = pcPortView->GetOwner()->GetConnectionName();
		for (j = i + 1; j < papcPortViews->NumElements(); j++)
		{
			pcOtherPortView = papcPortViews->GetPtr(j);
			if (pcOtherPortView->GetGridPosition() == pcPortView->GetGridPosition())
			{
				Join(paiParents, (int)(uiTraces + i), (int)(uiTraces + j));
			}
			else if (szName != NULL)
			{
				szOtherName = pcOtherPortView->GetOwner()->GetConnectionName();
				if ((szOtherName != NULL) && (StringCompare(szName, szOtherName) == 0))
				{
					Join(paiParents, (int)(uiTraces + i), (int)(uiTraces + j));
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A net is as wide as its widest port view; one with no ports gets a
// single trace to show it's undriven.  Roots that are nets (anything with
// a wire, and port views joined to another) are numbered in paiNetIndices
// (-1 for the others) and their widths are in paiNetWidths, both by root.
// Fails if ports of different widths are joined.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitView::FindNets(CArrayInt* paiParents, CArrayPortViewPtr* papcPortViews, CArrayInt* paiNetWidths, CArrayInt* paiNetIndices, size* puiNumNets)
{
	size			uiTraces;
	size			uiNodes;
	size			i;
	int				iRoot;
	int				iWidth;
	CPortView*		pcPortView;
	bool			bResult;

	uiTraces = mapcTraces.NumElements();
	uiNodes = uiTraces + papcPortViews->NumElements();
	bResult = true;
	paiNetWidths->Init();
	paiNetIndices->Init();
	for (i = 0; i < uiNodes; i++)
	{
		paiNetWidths->Add(0);
		paiNetIndices->Add(-1);
	}
	for (i = 0; i < papcPortViews->NumElements(); i++)
	{
		pcPortView = papcPortViews->GetPtr(i);
		iRoot = FindRoot(paiParents, (int)(uiTraces + i));
		iWidth = (int)pcPortView->NumPorts();
		if ((paiNetWidths->GetValue(iRoot) != 0) && (iWidth != 0) && (paiNetWidths->GetValue(iRoot) != iWidth))
		{
			gcLogger.Error2(__METHOD__, " Ports of different widths are connected at [", IntToString(pcPortView->GetGridPosition().x), ", ", IntToString(pcPortView->GetGridPosition().y), "].", NULL);
			bResult = false;
		}
		if (iWidth > paiNetWidths->GetValue(iRoot))
		{
			paiNetWidths->SetValue(iRoot, iWidth);
		}
	}

	*puiNumNets = 0;
	for (i = 0; i < uiNodes; i++)
	{
		iRoot = FindRoot(paiParents, (int)i);
		if ((i < uiTraces) || (iRoot != (int)i))
		{
			if (paiNetIndices->GetValue(iRoot) == -1)
			{
				paiNetIndices->SetValue(iRoot, (int)*puiNumNets);
				(*puiNumNets)++;
			}
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The traces of each net by bit, into papcNetTraces.  Bits the components
// join (GetBitJoins) share a trace; a join to a port view that isn't on a
// net joins nothing.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::CreateNetTraces(CCircuit* pcCircuit, CArrayInt* paiParents, CArrayPortViewPtr* papcPortViews, CArrayInt* paiNetWidths, CArrayInt* paiNetIndices, size uiNumNets, CArrayTraceViewTracePtr* papcNetTraces)
{
	size				uiTraces;
	size				uiNodes;
	size				i;
	size				k;
	int					iRoot;
	int					iNet;
	CArrayInt			aiNetFirstBit;
	CArrayInt			aiNetWidth;
	CArrayInt			aiBitParents;
	CArrayTraceViewTracePtr	apcBitTraces;
	int					iBits;
	int					iWidth;
	CArrayPortBitJoin	asJoins;
	SPortBitJoin*		psJoin;
	int					iFromNet;
	int					iToNet;
	int					iBit;
	CTrace*				pcTrace;

	uiTraces = mapcTraces.NumElements();
	uiNodes = uiTraces + papcPortViews->NumElements();

	aiNetFirstBit.Init();
	aiNetWidth.Init();
	for (i = 0; i < uiNumNets; i++)
	{
		aiNetFirstBit.Add(0);
		aiNetWidth.Add(0);
	}
	for (i = 0; i < uiNodes; i++)
	{
		iRoot = FindRoot(paiParents, (int)i);
		if ((int)i == iRoot)
		{
			iNet = paiNetIndices->GetValue(iRoot);
			if (iNet != -1)
			{
				iWidth = paiNetWidths->GetValue(iRoot);
				aiNetWidth.SetValue(iNet, (iWidth == 0) ? 1 : iWidth);
			}
		}
	}
	iBits = 0;
	for (i = 0; i < uiNumNets; i++)
	{
		aiNetFirstBit.SetValue(i, iBits);
		iBits += aiNetWidth.GetValue(i);
	}

	aiBitParents.Init();
	for (iBit = 0; iBit < iBits; iBit++)
	{
		aiBitParents.Add(iBit);
	}
	asJoins.Init();
	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		mapcComponents.GetPtr(i)->GetBitJoins(&asJoins);
	}
	for (i = 0; i < asJoins.NumElements(); i++)
	{
		psJoin = asJoins.Get(i);
		iFromNet = GetPortViewNet(paiParents, papcPortViews, paiNetIndices, psJoin->pcFrom);
		iToNet = GetPortViewNet(paiParents, papcPortViews, paiNetIndices, psJoin->pcTo);
		if ((iFromNet != -1) && (iToNet != -1) &&
			(psJoin->uiFromBit < (size)aiNetWidth.GetValue(iFromNet)) && (psJoin->uiToBit < (size)aiNetWidth.GetValue(iToNet)))
		{
			Join(&aiBitParents, aiNetFirstBit.GetValue(iFromNet) + (int)psJoin->uiFromBit, aiNetFirstBit.GetValue(iToNet) + (int)psJoin->uiToBit);
		}
	}
	asJoins.Kill();

	apcBitTraces.Init();
	for (iBit = 0; iBit < iBits; iBit++)
	{
		apcBitTraces.Add(NULL);
	}
	for (i = 0; i < uiNumNets; i++)
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
			papcNetTraces[i].Add(pcTrace);
		}
	}

	apcBitTraces.Kill();
	aiBitParents.Kill();
	aiNetWidth.Kill();
	aiNetFirstBit.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The net the port view is on, or -1.
//
//////////////////////////////////////////////////////////////////////////
int CCircuitView::GetPortViewNet(CArrayInt* paiParents, CArrayPortViewPtr* papcPortViews, CArrayInt* paiNetIndices, CPortView* pcPortView)
{
	size	i;

	for (i = 0; i < papcPortViews->NumElements(); i++)
	{
		if (papcPortViews->GetPtr(i) == pcPortView)
		{
			return paiNetIndices->GetValue(FindRoot(paiParents, (int)(mapcTraces.NumElements() + i)));
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// Forgets the simulation's components and traces.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::DisconnectSimulation(void)
{
	size	i;

	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		mapcComponents.GetPtr(i)->DisconnectComponent();
	}
	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		mapcTraces.GetPtr(i)->ClearTraces();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A wire's end is a junction where a port is or where three or more wire
// arms meet (a wire passing through counts as two).
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::CalculateJunctions(void)
{
	CArrayPortViewPtr	apcPortViews;
	size				i;
	size				j;
	CTraceView*			pcTrace;
	SInt32Vec2			asEnds[2];
	bool				abJunction[2];
	int					iArms;
	int					iPorts;
	int					iEnd;
	CTraceView*			pcOther;

	apcPortViews.Init();
	CollectPortViews(&apcPortViews);

	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		pcTrace = mapcTraces.GetPtr(i);
		asEnds[0] = pcTrace->GetStart();
		asEnds[1] = pcTrace->GetEnd();
		for (iEnd = 0; iEnd < 2; iEnd++)
		{
			iArms = 0;
			iPorts = 0;
			for (j = 0; j < mapcTraces.NumElements(); j++)
			{
				pcOther = mapcTraces.GetPtr(j);
				if (pcOther->IsEnd(asEnds[iEnd]))
				{
					iArms++;
				}
				else if (pcOther->Contains(asEnds[iEnd]))
				{
					iArms += 2;
				}
			}
			for (j = 0; j < apcPortViews.NumElements(); j++)
			{
				if (apcPortViews.GetPtr(j)->GetGridPosition() == asEnds[iEnd])
				{
					iPorts++;
				}
			}
			abJunction[iEnd] = (iPorts > 0) || (iArms >= 3);
		}
		pcTrace->SetJunctions(abJunction[0], abJunction[1]);
	}
	apcPortViews.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::SaveXML(CXMLWriter* pcWriter)
{
	size	i;

	pcWriter->Begin("Circuit");
	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		mapcComponents.GetPtr(i)->SaveXML(pcWriter);
	}
	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		mapcTraces.GetPtr(i)->SaveXML(pcWriter);
	}
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
// Stops at the first element it can't read; what was read before it stays
// in the circuit view.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitView::LoadXML(CMarkupTag* pcCircuitTag)
{
	STagIterator		sIter;
	CMarkupTag*			pcTag;
	CComponentView*		pcComponent;
	CTraceView*			pcTrace;

	if (!pcCircuitTag->Is("Circuit"))
	{
		return gcLogger.Error2(__METHOD__, " Expected a [Circuit] element, not [", pcCircuitTag->GetName(), "].", NULL);
	}

	pcTag = pcCircuitTag->GetTag(&sIter);
	while (pcTag != NULL)
	{
		if (pcTag->Is("Trace"))
		{
			pcTrace = NewMalloc<CTraceView>();
			if (!pcTrace->LoadXML(pcTag))
			{
				free(pcTrace);
				return false;
			}
			mapcTraces.Add(pcTrace);
		}
		else
		{
			pcComponent = NewComponentView(pcTag->GetName());
			if (pcComponent == NULL)
			{
				return gcLogger.Error2(__METHOD__, " Unknown component [", pcTag->GetName(), "].", NULL);
			}
			if (!pcComponent->LoadXML(pcTag))
			{
				free(pcComponent);
				return false;
			}
			mapcComponents.Add(pcComponent);
		}
		pcTag = pcCircuitTag->GetNextTag(&sIter);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Wires under components.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::Paint(CCircuitPainter* pcPainter)
{
	size	i;

	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		mapcTraces.GetPtr(i)->Paint(pcPainter);
	}
	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		mapcComponents.GetPtr(i)->Paint(pcPainter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// In grid units.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitView::GetBoundingBox(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum)
{
	size			i;
	bool			bHasBounds;
	SFloat64Vec2	sMinimum;
	SFloat64Vec2	sMaximum;
	CTraceView*		pcTrace;

	bHasBounds = false;
	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		if (mapcComponents.GetPtr(i)->GetBoundingBox(&sMinimum, &sMaximum))
		{
			if (!bHasBounds)
			{
				*psMinimum = sMinimum;
				*psMaximum = sMaximum;
				bHasBounds = true;
			}
			IncludeBoundsPoint(psMinimum, psMaximum, sMinimum.x, sMinimum.y);
			IncludeBoundsPoint(psMinimum, psMaximum, sMaximum.x, sMaximum.y);
		}
	}
	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		pcTrace = mapcTraces.GetPtr(i);
		if (!bHasBounds)
		{
			psMinimum->Init(pcTrace->GetStart().x, pcTrace->GetStart().y);
			psMaximum->Init(pcTrace->GetStart().x, pcTrace->GetStart().y);
			bHasBounds = true;
		}
		IncludeBoundsPoint(psMinimum, psMaximum, pcTrace->GetStart().x, pcTrace->GetStart().y);
		IncludeBoundsPoint(psMinimum, psMaximum, pcTrace->GetEnd().x, pcTrace->GetEnd().y);
	}
	return bHasBounds;
}


//////////////////////////////////////////////////////////////////////////
//
// Logicim's dot grid: none when zoomed far out, single pixel dots, darker
// dots, and from zoom 2 small squares.
//
//////////////////////////////////////////////////////////////////////////
void PaintCircuitGrid(CCircuitPainter* pcPainter)
{
	CCircuitViewport*	pcViewport;
	CCircuitColours*	pcColours;
	CGeometryMap*		pcMap;
	float64				fZoom;
	SFloat64Vec2		sScreen;
	SFloat64Vec2		sTopLeft;
	SFloat64Vec2		sBottomRight;
	SInt32Vec2			sSize;
	int32				iLeft;
	int32				iTop;
	int32				iRight;
	int32				iBottom;
	int32				x;
	int32				y;
	int32				iDot;
	ARGB32				sColour;

	pcViewport = pcPainter->GetViewport();
	pcColours = pcPainter->GetColours();
	pcMap = pcPainter->GetMap();
	fZoom = pcViewport->GetZoom();
	if (fZoom < 0.3)
	{
		return;
	}

	sColour = (fZoom < 1.5) ? pcColours->msSmallGridDot : pcColours->msLargeGridDot;
	iDot = (fZoom < 2.0) ? 1 : (int32)fZoom;

	sSize = pcViewport->GetSize();
	sScreen.Init(0, 0);
	pcViewport->ScreenToGrid(&sTopLeft, &sScreen);
	sScreen.Init(sSize.x, sSize.y);
	pcViewport->ScreenToGrid(&sBottomRight, &sScreen);
	iLeft = (int32)floor(sTopLeft.x) - 1;
	iTop = (int32)floor(sTopLeft.y) - 1;
	iRight = (int32)ceil(sBottomRight.x) + 1;
	iBottom = (int32)ceil(sBottomRight.y) + 1;

	for (y = iTop; y <= iBottom; y++)
	{
		for (x = iLeft; x <= iRight; x++)
		{
			sScreen.Init(pcViewport->GridToScreenX(x), pcViewport->GridToScreenY(y));
			pcMap->AddRectangle((int32)floor(sScreen.x + 0.5) - iDot / 2, (int32)floor(sScreen.y + 0.5) - iDot / 2, iDot, iDot, sColour);
		}
	}
}


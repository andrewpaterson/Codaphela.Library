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
// simulation and wires the nets.  The circuit view must not be connected
// to another simulation.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitView::CreateSimulation(CCircuitSimulation* pcSimulation, CFamilyVoltageConfigurationStore* pcFamilies)
{
	CArrayPortViewPtr			apcPortViews;
	CArrayInt					aiParents;
	CArrayInt					aiNetWidths;
	size						uiTraces;
	size						uiNodes;
	size						i;
	size						j;
	size						k;
	int							iRoot;
	int							iWidth;
	CPortView*					pcPortView;
	CPortView*					pcOtherPortView;
	CSubcircuitSimulation*		pcTop;
	CCircuit*					pcCircuit;
	CTimeline*					pcTimeline;
	bool						bResult;
	CArrayTraceViewTracePtr*	papcNetTraces;
	size						uiNumNets;
	CArrayInt					aiNetIndices;
	int							iNet;

	pcTop = pcSimulation->GetSubcircuitTopSimulation();
	pcCircuit = pcSimulation->GetCircuit();

	bResult = true;
	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		if (mapcComponents.GetPtr(i)->CreateComponent(pcTop, pcFamilies) == NULL)
		{
			bResult = false;
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

	for (i = 0; i < uiTraces; i++)
	{
		for (j = i + 1; j < uiTraces; j++)
		{
			if (mapcTraces.GetPtr(i)->Touches(mapcTraces.GetPtr(j)))
			{
				Join(&aiParents, (int)i, (int)j);
			}
		}
	}
	for (i = 0; i < apcPortViews.NumElements(); i++)
	{
		pcPortView = apcPortViews.GetPtr(i);
		for (j = 0; j < uiTraces; j++)
		{
			if (mapcTraces.GetPtr(j)->Contains(pcPortView->GetGridPosition()))
			{
				Join(&aiParents, (int)(uiTraces + i), (int)j);
			}
		}
		for (j = i + 1; j < apcPortViews.NumElements(); j++)
		{
			pcOtherPortView = apcPortViews.GetPtr(j);
			if (pcOtherPortView->GetGridPosition() == pcPortView->GetGridPosition())
			{
				Join(&aiParents, (int)(uiTraces + i), (int)(uiTraces + j));
			}
		}
	}

	//A net is as wide as its widest port view; one with no ports gets a single trace to show it's undriven.
	aiNetWidths.Init();
	aiNetIndices.Init();
	for (i = 0; i < uiNodes; i++)
	{
		aiNetWidths.Add(0);
		aiNetIndices.Add(-1);
	}
	for (i = 0; i < apcPortViews.NumElements(); i++)
	{
		pcPortView = apcPortViews.GetPtr(i);
		iRoot = FindRoot(&aiParents, (int)(uiTraces + i));
		iWidth = (int)pcPortView->NumPorts();
		if ((aiNetWidths.GetValue(iRoot) != 0) && (iWidth != 0) && (aiNetWidths.GetValue(iRoot) != iWidth))
		{
			gcLogger.Error2(__METHOD__, " Ports of different widths are connected at [", IntToString(pcPortView->GetGridPosition().x), ", ", IntToString(pcPortView->GetGridPosition().y), "].", NULL);
			bResult = false;
		}
		if (iWidth > aiNetWidths.GetValue(iRoot))
		{
			aiNetWidths.SetValue(iRoot, iWidth);
		}
	}

	//Which roots are nets: anything with a wire, and ports joined to another port.
	uiNumNets = 0;
	for (i = 0; i < uiNodes; i++)
	{
		iRoot = FindRoot(&aiParents, (int)i);
		if ((i < uiTraces) || (iRoot != (int)i))
		{
			if (aiNetIndices.GetValue(iRoot) == -1)
			{
				aiNetIndices.SetValue(iRoot, (int)uiNumNets);
				uiNumNets++;
			}
		}
	}

	papcNetTraces = (CArrayTraceViewTracePtr*)malloc(sizeof(CArrayTraceViewTracePtr) * (uiNumNets + 1));
	for (i = 0; i < uiNumNets; i++)
	{
		papcNetTraces[i].Init();
	}
	for (i = 0; i < uiNodes; i++)
	{
		iRoot = FindRoot(&aiParents, (int)i);
		iNet = aiNetIndices.GetValue(iRoot);
		if ((iNet != -1) && (papcNetTraces[iNet].NumElements() == 0))
		{
			iWidth = aiNetWidths.GetValue(iRoot);
			if (iWidth == 0)
			{
				iWidth = 1;
			}
			for (k = 0; k < (size)iWidth; k++)
			{
				papcNetTraces[iNet].Add(pcCircuit->AddTrace());
			}
		}
	}

	for (i = 0; i < uiTraces; i++)
	{
		iNet = aiNetIndices.GetValue(FindRoot(&aiParents, (int)i));
		mapcTraces.GetPtr(i)->SetTraces(&papcNetTraces[iNet]);
	}
	//Started before the ports are wired, as Logicim's editor wires them, so each port sees its trace go from undriven to
	//whatever drives it.  Gates only act on transitions; wired before the start, a supply driving a gate's input
	//wouldn't be seen.
	pcSimulation->SimulationStarted();
	pcTimeline = pcSimulation->GetTimeline();
	for (i = 0; i < apcPortViews.NumElements(); i++)
	{
		pcPortView = apcPortViews.GetPtr(i);
		iNet = aiNetIndices.GetValue(FindRoot(&aiParents, (int)(uiTraces + i)));
		if (iNet != -1)
		{
			for (k = 0; k < pcPortView->NumPorts(); k++)
			{
				bResult &= pcPortView->GetPort(k)->ConnectLive(papcNetTraces[iNet].GetPtr(k), pcTimeline);
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


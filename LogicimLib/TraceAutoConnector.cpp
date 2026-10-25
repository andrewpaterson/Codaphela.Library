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
#include <math.h>
#include "BaseLib/Logger.h"
#include "BaseLib/PointerFunctions.h"
#include "TraceAutoConnector.h"


//What is under a grid cell besides wires: nothing, a component's body or a port (or the way out of its component)
//that isn't on any of the nets being connected.  Anything else is the net it's kept for.
#define OWNER_FREE		-1
#define OWNER_FOREIGN	-2
#define OWNER_BODY		-3


static int32 gaiDirectionX[4] = { 1, 0, -1, 0 };
static int32 gaiDirectionY[4] = { 0, 1, 0, -1 };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsDirectionHorizontal(int iDirection)
{
	return (iDirection == TRACE_DIRECTION_EAST) || (iDirection == TRACE_DIRECTION_WEST);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTraceAutoConnector::OppositeDirection(int iDirection)
{
	return (iDirection + 2) % 4;
}


//////////////////////////////////////////////////////////////////////////
//
// The set the wire is in, for the circuit's wires joined into sets.
//
//////////////////////////////////////////////////////////////////////////
static int FindTraceGroup(CArrayInt* paiParents, int iTrace)
{
	while (paiParents->GetValue(iTrace) != iTrace)
	{
		iTrace = paiParents->GetValue(iTrace);
	}
	return iTrace;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int32 IntAbs(int32 i)
{
	return (i < 0) ? -i : i;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTraceAutoConnector::Init(CCircuitView* pcCircuit)
{
	mpcCircuit = pcCircuit;
	masConnections.Init();
	muiNumNets = 0;
	muiNumRuns = 0;
	mapcPorts.Init();
	maiPortNets.Init();

	msGridMinimum.Init(0, 0);
	miGridWidth = 0;
	miGridHeight = 0;
	maiOwners.Init();
	maiHorizontal.Init();
	maiVertical.Init();
	maiNodes.Init();
	maiNear.Init();

	maiCosts.Init();
	maiFrom.Init();
	maiStamps.Init();
	miStamp = 0;
	masHeap.Init();
	maiGoalStamps.Init();
	maiGoalQueue.Init();

	mapcAddedTraces.Init();
	maiAddedTraceNets.Init();
	maiNetsWithExisting.Init();
	maiNetRuns.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTraceAutoConnector::Kill(void)
{
	maiNetRuns.Kill();
	maiNetsWithExisting.Kill();
	maiAddedTraceNets.Kill();
	mapcAddedTraces.Kill();
	maiGoalQueue.Kill();
	maiGoalStamps.Kill();
	masHeap.Kill();
	maiStamps.Kill();
	maiFrom.Kill();
	maiCosts.Kill();

	maiNear.Kill();
	maiNodes.Kill();
	maiVertical.Kill();
	maiHorizontal.Kill();
	maiOwners.Kill();

	maiPortNets.Kill();
	mapcPorts.Kill();
	masConnections.Kill();
	mpcCircuit = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTraceAutoConnector::AddNet(void)
{
	muiNumNets++;
	return muiNumNets - 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTraceAutoConnector::AddRun(void)
{
	muiNumRuns++;
	return muiNumRuns - 1;
}


//////////////////////////////////////////////////////////////////////////
//
// Fails if either port is already on another net.  fBend is from 0 (at
// the from component) to 1 (at the to component) or
// TRACE_AUTO_CONNECT_NO_BEND.
//
//////////////////////////////////////////////////////////////////////////
bool CTraceAutoConnector::AddConnection(size uiNet, CPortView* pcFrom, CPortView* pcTo, float64 fBend, int iRun)
{
	STraceAutoConnection*	psConnection;

	if (uiNet >= muiNumNets)
	{
		return gcLogger.Error2(__METHOD__, " There is no net [", IntToString((int)uiNet), "].", NULL);
	}
	if (!AddPort(pcFrom, uiNet) || !AddPort(pcTo, uiNet))
	{
		return false;
	}

	psConnection = masConnections.Add();
	psConnection->uiNet = uiNet;
	psConnection->pcFrom = pcFrom;
	psConnection->pcTo = pcTo;
	psConnection->fBend = fBend;
	psConnection->iRun = iRun;
	psConnection->bHorizontal = true;
	psConnection->iBend = 0;
	psConnection->bHasBend = false;
	psConnection->bConnected = false;
	psConnection->iLength = 0;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTraceAutoConnector::AddPort(CPortView* pcPortView, size uiNet)
{
	int		iNet;

	iNet = GetPortNet(pcPortView);
	if (iNet == -1)
	{
		mapcPorts.Add(pcPortView);
		maiPortNets.Add((int)uiNet);
		return true;
	}
	if (iNet != (int)uiNet)
	{
		return gcLogger.Error2(__METHOD__, " Port [", pcPortView->GetPortName(0), "] of [", pcPortView->GetOwner()->GetName(), "] is already on another net.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// -1 if the port isn't being connected.
//
//////////////////////////////////////////////////////////////////////////
int CTraceAutoConnector::GetPortNet(CPortView* pcPortView)
{
	size	uiIndex;

	uiIndex = mapcPorts.Find(pcPortView);
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return -1;
	}
	return maiPortNets.GetValue(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// Routes every connection and adds their wires to the circuit.  False if
// any couldn't be routed.
//
//////////////////////////////////////////////////////////////////////////
bool CTraceAutoConnector::Connect(void)
{
	CArrayInt				aiOrder;
	CArrayInt				aiFailed;
	CArrayInt				aiConflicts;
	size					i;
	size					j;
	size					uiStates;
	size					uiHead;
	size					uiRips;
	size					uiMaxRips;
	STraceAutoConnection*	psConnection;
	int						iNet;

	if (masConnections.NumElements() == 0)
	{
		return true;
	}

	maiNetsWithExisting.ReInit();
	maiNetRuns.ReInit();
	for (i = 0; i < muiNumNets; i++)
	{
		maiNetsWithExisting.Add(0);
		maiNetRuns.Add(-1);
	}
	for (i = 0; i < masConnections.NumElements(); i++)
	{
		psConnection = masConnections.Get(i);
		if (maiNetRuns.GetValue(psConnection->uiNet) == -1)
		{
			maiNetRuns.SetValue(psConnection->uiNet, psConnection->iRun);
		}
	}
	mapcAddedTraces.ReInit();
	maiAddedTraceNets.ReInit();

	CreateGrid();
	MarkComponents();
	MarkPorts();
	MarkExistingTraces();
	CalculateBends();

	uiStates = (size)miGridWidth * (size)miGridHeight * 4;
	maiCosts.Kill();
	maiFrom.Kill();
	maiStamps.Kill();
	maiCosts.Init();
	maiFrom.Init();
	maiStamps.Init();
	maiCosts.GrowToAtLeastNumElements(uiStates, true, 0);
	maiFrom.GrowToAtLeastNumElements(uiStates, true, 0);
	maiStamps.GrowToAtLeastNumElements(uiStates, true, 0);
	maiGoalStamps.ReInit();
	maiGoalStamps.GrowToAtLeastNumElements(uiStates / 2, true, 0);
	miStamp = 0;

	aiOrder.Init();
	aiFailed.Init();
	SortConnections(&aiOrder);
	for (i = 0; i < aiOrder.NumElements(); i++)
	{
		if (!Route(masConnections.Get(aiOrder.GetValue(i)), false, NULL))
		{
			aiFailed.Add(aiOrder.GetValue(i));
		}
	}

	//Rip up what's in the way of each that failed and route it again.
	aiConflicts.Init();
	uiRips = 0;
	uiMaxRips = 2 * aiFailed.NumElements() + 8;
	for (uiHead = 0; (uiHead < aiFailed.NumElements()) && (uiRips < uiMaxRips); uiHead++)
	{
		psConnection = masConnections.Get(aiFailed.GetValue(uiHead));
		if (psConnection->bConnected || Route(psConnection, false, NULL))
		{
			continue;
		}
		aiConflicts.ReInit();
		if (!Route(psConnection, true, &aiConflicts) || (aiConflicts.NumElements() == 0))
		{
			continue;
		}
		for (i = 0; i < aiConflicts.NumElements(); i++)
		{
			iNet = aiConflicts.GetValue(i);
			RipNet(iNet);
			for (j = 0; j < aiOrder.NumElements(); j++)
			{
				if ((int)masConnections.Get(aiOrder.GetValue(j))->uiNet == iNet)
				{
					aiFailed.Add(aiOrder.GetValue(j));
				}
			}
		}
		Route(psConnection, false, NULL);
		uiRips++;
	}
	aiConflicts.Kill();
	aiFailed.Kill();
	aiOrder.Kill();

	return NumUnconnected() == 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Around the components, their ports and the wires, with a margin.
//
//////////////////////////////////////////////////////////////////////////
void CTraceAutoConnector::CreateGrid(void)
{
	SInt32Vec2			sMinimum;
	SInt32Vec2			sMaximum;
	bool				bHasBounds;
	size				i;
	size				j;
	CComponentView*		pcComponent;
	SFloat64Vec2		sBoxMinimum;
	SFloat64Vec2		sBoxMaximum;
	SInt32Vec2			sPosition;
	CTraceView*			pcTrace;
	size				uiCells;

	bHasBounds = false;
	sMinimum.Init(0, 0);
	sMaximum.Init(0, 0);
	for (i = 0; i < mpcCircuit->NumComponents(); i++)
	{
		pcComponent = mpcCircuit->GetComponent(i);
		if (pcComponent->GetBoundingBox(&sBoxMinimum, &sBoxMaximum))
		{
			IncludeGridPoint(&sMinimum, &sMaximum, &bHasBounds, (int32)floor(sBoxMinimum.x), (int32)floor(sBoxMinimum.y));
			IncludeGridPoint(&sMinimum, &sMaximum, &bHasBounds, (int32)ceil(sBoxMaximum.x), (int32)ceil(sBoxMaximum.y));
		}
		for (j = 0; j < pcComponent->NumPortViews(); j++)
		{
			sPosition = pcComponent->GetPortView(j)->GetGridPosition();
			IncludeGridPoint(&sMinimum, &sMaximum, &bHasBounds, sPosition.x, sPosition.y);
		}
	}
	for (i = 0; i < mpcCircuit->NumTraces(); i++)
	{
		pcTrace = mpcCircuit->GetTrace(i);
		IncludeGridPoint(&sMinimum, &sMaximum, &bHasBounds, pcTrace->GetStart().x, pcTrace->GetStart().y);
		IncludeGridPoint(&sMinimum, &sMaximum, &bHasBounds, pcTrace->GetEnd().x, pcTrace->GetEnd().y);
	}

	msGridMinimum.Init(sMinimum.x - TRACE_AUTO_CONNECT_MARGIN, sMinimum.y - TRACE_AUTO_CONNECT_MARGIN);
	miGridWidth = (sMaximum.x - sMinimum.x) + 1 + 2 * TRACE_AUTO_CONNECT_MARGIN;
	miGridHeight = (sMaximum.y - sMinimum.y) + 1 + 2 * TRACE_AUTO_CONNECT_MARGIN;
	uiCells = (size)miGridWidth * (size)miGridHeight;

	maiOwners.Kill();
	maiHorizontal.Kill();
	maiVertical.Kill();
	maiNodes.Kill();
	maiNear.Kill();
	maiOwners.Init();
	maiHorizontal.Init();
	maiVertical.Init();
	maiNodes.Init();
	maiNear.Init();
	maiOwners.GrowToAtLeastNumElements(uiCells, true, 0xff);
	maiHorizontal.GrowToAtLeastNumElements(uiCells, true, 0xff);
	maiVertical.GrowToAtLeastNumElements(uiCells, true, 0xff);
	maiNodes.GrowToAtLeastNumElements(uiCells, true, 0xff);
	maiNear.GrowToAtLeastNumElements(uiCells, true, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTraceAutoConnector::IncludeGridPoint(SInt32Vec2* psMinimum, SInt32Vec2* psMaximum, bool* pbHasBounds, int32 x, int32 y)
{
	if (!*pbHasBounds)
	{
		psMinimum->Init(x, y);
		psMaximum->Init(x, y);
		*pbHasBounds = true;
		return;
	}
	if (x < psMinimum->x)
	{
		psMinimum->x = x;
	}
	if (y < psMinimum->y)
	{
		psMinimum->y = y;
	}
	if (x > psMaximum->x)
	{
		psMaximum->x = x;
	}
	if (y > psMaximum->y)
	{
		psMaximum->y = y;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Every cell in a component's box is its body, and the cells around a
// body are near it.  A box that doesn't end on the grid (a tunnel is 1.3
// high) only covers the cells inside it.
//
//////////////////////////////////////////////////////////////////////////
void CTraceAutoConnector::MarkComponents(void)
{
	size				i;
	CComponentView*		pcComponent;
	SFloat64Vec2		sBoxMinimum;
	SFloat64Vec2		sBoxMaximum;
	int32				x;
	int32				y;
	int32				iMinimumX;
	int32				iMinimumY;
	int32				iMaximumX;
	int32				iMaximumY;
	int					iCell;
	int					iDirection;

	for (i = 0; i < mpcCircuit->NumComponents(); i++)
	{
		pcComponent = mpcCircuit->GetComponent(i);
		if (!pcComponent->GetBoundingBox(&sBoxMinimum, &sBoxMaximum))
		{
			continue;
		}
		iMinimumX = (int32)ceil(sBoxMinimum.x);
		iMinimumY = (int32)ceil(sBoxMinimum.y);
		iMaximumX = (int32)floor(sBoxMaximum.x);
		iMaximumY = (int32)floor(sBoxMaximum.y);
		for (y = iMinimumY; y <= iMaximumY; y++)
		{
			for (x = iMinimumX; x <= iMaximumX; x++)
			{
				maiOwners.SetValue(GetCell(x, y), OWNER_BODY);
			}
		}
	}

	for (y = 0; y < miGridHeight; y++)
	{
		for (x = 0; x < miGridWidth; x++)
		{
			iCell = x + y * miGridWidth;
			if (maiOwners.GetValue(iCell) == OWNER_BODY)
			{
				continue;
			}
			for (iDirection = 0; iDirection < 4; iDirection++)
			{
				if ((x + gaiDirectionX[iDirection] >= 0) && (x + gaiDirectionX[iDirection] < miGridWidth) && (y + gaiDirectionY[iDirection] >= 0) && (y + gaiDirectionY[iDirection] < miGridHeight))
				{
					if (maiOwners.GetValue(iCell + gaiDirectionX[iDirection] + gaiDirectionY[iDirection] * miGridWidth) == OWNER_BODY)
					{
						maiNear.SetValue(iCell, 1);
					}
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A port, the way out from it to the edge of its component's body and the
// cells straight out in front of it are kept for its net; a port that isn't being
// connected is kept clear of.
//
//////////////////////////////////////////////////////////////////////////
void CTraceAutoConnector::MarkPorts(void)
{
	size				i;
	size				j;
	CComponentView*		pcComponent;
	CPortView*			pcPortView;
	int					iNet;
	int					iOwner;
	SInt32Vec2			sPosition;
	int					iFacing;
	int					iCell;
	int					iApproach;

	for (i = 0; i < mpcCircuit->NumComponents(); i++)
	{
		pcComponent = mpcCircuit->GetComponent(i);
		for (j = 0; j < pcComponent->NumPortViews(); j++)
		{
			pcPortView = pcComponent->GetPortView(j);
			iNet = GetPortNet(pcPortView);
			iOwner = (iNet == -1) ? OWNER_FOREIGN : iNet;
			sPosition = pcPortView->GetGridPosition();
			iCell = GetCell(sPosition);
			maiOwners.SetValue(iCell, iOwner);
			maiNodes.SetValue(iCell, iOwner);

			iFacing = GetPortFacing(pcPortView);
			for (;;)
			{
				sPosition.x += gaiDirectionX[iFacing];
				sPosition.y += gaiDirectionY[iFacing];
				if (!IsInGrid(sPosition.x, sPosition.y))
				{
					break;
				}
				iCell = GetCell(sPosition);
				if (maiOwners.GetValue(iCell) != OWNER_BODY)
				{
					//The cells in front of a port being connected are kept for its net, so another net can't block the way in.
					for (iApproach = 0; (iNet != -1) && (iApproach < TRACE_AUTO_CONNECT_APPROACH); iApproach++)
					{
						if (!IsInGrid(sPosition.x, sPosition.y) || (maiOwners.GetValue(GetCell(sPosition)) != OWNER_FREE))
						{
							break;
						}
						maiOwners.SetValue(GetCell(sPosition), iNet);
						sPosition.x += gaiDirectionX[iFacing];
						sPosition.y += gaiDirectionY[iFacing];
					}
					break;
				}
				maiOwners.SetValue(iCell, iOwner);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The circuit's wires joined into groups as Logicim's wiring joins them.
// A group on a port being connected is that port's net; any other group
// is a net of its own that routes keep clear of.
//
//////////////////////////////////////////////////////////////////////////
void CTraceAutoConnector::MarkExistingTraces(void)
{
	CArrayInt		aiParents;
	CArrayInt		aiGroupNets;
	size			uiTraces;
	size			i;
	size			j;
	int				iRoot;
	int				iOther;
	int				iNet;
	CTraceView*		pcTrace;
	SInt32Vec2		sPosition;

	uiTraces = mpcCircuit->NumTraces();
	if (uiTraces == 0)
	{
		return;
	}

	aiParents.Init();
	aiGroupNets.Init();
	for (i = 0; i < uiTraces; i++)
	{
		aiParents.Add((int)i);
		aiGroupNets.Add(-1);
	}
	for (i = 0; i < uiTraces; i++)
	{
		for (j = i + 1; j < uiTraces; j++)
		{
			if (mpcCircuit->GetTrace(i)->Touches(mpcCircuit->GetTrace(j)))
			{
				iRoot = FindTraceGroup(&aiParents, (int)i);
				iOther = FindTraceGroup(&aiParents, (int)j);
				if (iRoot != iOther)
				{
					aiParents.SetValue(iOther, iRoot);
				}
			}
		}
	}

	for (i = 0; i < mapcPorts.NumElements(); i++)
	{
		sPosition = mapcPorts.GetPtr(i)->GetGridPosition();
		iNet = maiPortNets.GetValue(i);
		for (j = 0; j < uiTraces; j++)
		{
			if (mpcCircuit->GetTrace(j)->Contains(sPosition))
			{
				iRoot = FindTraceGroup(&aiParents, (int)j);
				if (aiGroupNets.GetValue(iRoot) == -1)
				{
					aiGroupNets.SetValue(iRoot, iNet);
					maiNetsWithExisting.SetValue(iNet, 1);
				}
				else if (aiGroupNets.GetValue(iRoot) != iNet)
				{
					//Already joins two nets; it's no use to either.
					aiGroupNets.SetValue(iRoot, (int)(muiNumNets + iRoot));
				}
			}
		}
	}

	for (i = 0; i < uiTraces; i++)
	{
		iRoot = FindTraceGroup(&aiParents, (int)i);
		iNet = aiGroupNets.GetValue(iRoot);
		if (iNet == -1)
		{
			iNet = (int)(muiNumNets + iRoot);
		}
		pcTrace = mpcCircuit->GetTrace(i);
		MarkTrace(pcTrace->GetStart().x, pcTrace->GetStart().y, pcTrace->GetEnd().x, pcTrace->GetEnd().y, iNet);
	}
	aiGroupNets.Kill();
	aiParents.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A wire that isn't horizontal or vertical only has its ends.
//
//////////////////////////////////////////////////////////////////////////
void CTraceAutoConnector::MarkTrace(int32 x0, int32 y0, int32 x1, int32 y1, int iNet)
{
	int32	i;
	int32	iMinimum;
	int32	iMaximum;

	if (y0 == y1)
	{
		iMinimum = (x0 < x1) ? x0 : x1;
		iMaximum = (x0 > x1) ? x0 : x1;
		for (i = iMinimum; i <= iMaximum; i++)
		{
			maiHorizontal.SetValue(GetCell(i, y0), iNet);
		}
	}
	else if (x0 == x1)
	{
		iMinimum = (y0 < y1) ? y0 : y1;
		iMaximum = (y0 > y1) ? y0 : y1;
		for (i = iMinimum; i <= iMaximum; i++)
		{
			maiVertical.SetValue(GetCell(x0, i), iNet);
		}
	}
	maiNodes.SetValue(GetCell(x0, y0), iNet);
	maiNodes.SetValue(GetCell(x1, y1), iNet);
}


//////////////////////////////////////////////////////////////////////////
//
// Where each connection should turn, from its components' boxes.
//
//////////////////////////////////////////////////////////////////////////
void CTraceAutoConnector::CalculateBends(void)
{
	size					i;
	STraceAutoConnection*	psConnection;
	SFloat64Vec2			sFromMinimum;
	SFloat64Vec2			sFromMaximum;
	SFloat64Vec2			sToMinimum;
	SFloat64Vec2			sToMaximum;
	SInt32Vec2				sFrom;
	SInt32Vec2				sTo;
	float64					fFromEdge;
	float64					fToEdge;
	int						iFacing;

	for (i = 0; i < masConnections.NumElements(); i++)
	{
		psConnection = masConnections.Get(i);
		sFrom = psConnection->pcFrom->GetGridPosition();
		sTo = psConnection->pcTo->GetGridPosition();
		psConnection->iLength = IntAbs(sTo.x - sFrom.x) + IntAbs(sTo.y - sFrom.y);
		iFacing = GetPortFacing(psConnection->pcFrom);
		psConnection->bHorizontal = IsDirectionHorizontal(iFacing);
		psConnection->bHasBend = false;
		if (psConnection->fBend < 0)
		{
			continue;
		}
		if (!psConnection->pcFrom->GetOwner()->GetBoundingBox(&sFromMinimum, &sFromMaximum) || !psConnection->pcTo->GetOwner()->GetBoundingBox(&sToMinimum, &sToMaximum))
		{
			continue;
		}

		if (psConnection->bHorizontal)
		{
			if (sToMinimum.x + sToMaximum.x >= sFromMinimum.x + sFromMaximum.x)
			{
				fFromEdge = sFromMaximum.x;
				fToEdge = sToMinimum.x;
			}
			else
			{
				fFromEdge = sFromMinimum.x;
				fToEdge = sToMaximum.x;
			}
		}
		else
		{
			if (sToMinimum.y + sToMaximum.y >= sFromMinimum.y + sFromMaximum.y)
			{
				fFromEdge = sFromMaximum.y;
				fToEdge = sToMinimum.y;
			}
			else
			{
				fFromEdge = sFromMinimum.y;
				fToEdge = sToMaximum.y;
			}
		}
		psConnection->iBend = (int32)floor(fFromEdge + psConnection->fBend * (fToEdge - fFromEdge) + 0.5);
		psConnection->bHasBend = true;
	}

	for (i = 0; i < muiNumRuns; i++)
	{
		CalculateRunBends((int)i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Steps the bends of a run's connections apart so they turn in a stair:
// going right and down the one from the top port turns last, so none
// crosses another.  They share the middle of their bends.
//
//////////////////////////////////////////////////////////////////////////
void CTraceAutoConnector::CalculateRunBends(int iRun)
{
	CArrayInt				aiMembers;
	size					i;
	size					j;
	int						iMember;
	STraceAutoConnection*	psConnection;
	STraceAutoConnection*	psOther;
	int32					iAcross;
	int32					iAlong;
	int32					iBendSum;
	int						iSign;
	int						iCentre;
	SInt32Vec2				sFrom;
	SInt32Vec2				sTo;
	SInt32Vec2				sOtherFrom;
	bool					bHorizontal;

	aiMembers.Init();
	for (i = 0; i < masConnections.NumElements(); i++)
	{
		psConnection = masConnections.Get(i);
		if ((psConnection->iRun == iRun) && psConnection->bHasBend)
		{
			//Sorted across the way they leave.
			sFrom = psConnection->pcFrom->GetGridPosition();
			for (j = 0; j < aiMembers.NumElements(); j++)
			{
				psOther = masConnections.Get(aiMembers.GetValue(j));
				sOtherFrom = psOther->pcFrom->GetGridPosition();
				if ((psConnection->bHorizontal && (sFrom.y < sOtherFrom.y)) || (!psConnection->bHorizontal && (sFrom.x < sOtherFrom.x)))
				{
					break;
				}
			}
			aiMembers.InsertAt((int)i, j);
		}
	}
	if (aiMembers.NumElements() < 2)
	{
		aiMembers.Kill();
		return;
	}

	iAcross = 0;
	iAlong = 0;
	iBendSum = 0;
	bHorizontal = masConnections.Get(aiMembers.GetValue(0))->bHorizontal;
	for (i = 0; i < aiMembers.NumElements(); i++)
	{
		psConnection = masConnections.Get(aiMembers.GetValue(i));
		sFrom = psConnection->pcFrom->GetGridPosition();
		sTo = psConnection->pcTo->GetGridPosition();
		if (bHorizontal)
		{
			iAcross += sTo.y - sFrom.y;
			iAlong += sTo.x - sFrom.x;
		}
		else
		{
			iAcross += sTo.x - sFrom.x;
			iAlong += sTo.y - sFrom.y;
		}
		iBendSum += psConnection->iBend;
	}

	iSign = (iAcross > 0) ? -1 : 1;
	if (iAlong < 0)
	{
		iSign = -iSign;
	}
	iCentre = (int)(aiMembers.NumElements() - 1) / 2;
	for (i = 0; i < aiMembers.NumElements(); i++)
	{
		iMember = aiMembers.GetValue(i);
		psConnection = masConnections.Get(iMember);
		psConnection->iBend = (int32)floor((float64)iBendSum / (float64)aiMembers.NumElements() + 0.5) + iSign * ((int)i - iCentre);
	}
	aiMembers.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The runs, shortest first and each in its stair's order, and then the
// connections not in a run, shortest first.
//
//////////////////////////////////////////////////////////////////////////
void CTraceAutoConnector::SortConnections(CArrayInt* paiOrder)
{
	CArrayInt				aiRunLengths;
	CArrayInt				aiRuns;
	CArrayInt				aiSingles;
	size					i;
	size					j;
	STraceAutoConnection*	psConnection;
	int						iRun;
	SInt32Vec2				sFrom;
	SInt32Vec2				sOtherFrom;
	size					uiFirst;

	aiRunLengths.Init();
	for (i = 0; i < muiNumRuns; i++)
	{
		aiRunLengths.Add(-1);
	}
	aiSingles.Init();
	for (i = 0; i < masConnections.NumElements(); i++)
	{
		psConnection = masConnections.Get(i);
		if ((psConnection->iRun >= 0) && (psConnection->iRun < (int)muiNumRuns))
		{
			if ((aiRunLengths.GetValue(psConnection->iRun) == -1) || (psConnection->iLength < aiRunLengths.GetValue(psConnection->iRun)))
			{
				aiRunLengths.SetValue(psConnection->iRun, psConnection->iLength);
			}
		}
		else
		{
			for (j = 0; j < aiSingles.NumElements(); j++)
			{
				if (psConnection->iLength < masConnections.Get(aiSingles.GetValue(j))->iLength)
				{
					break;
				}
			}
			aiSingles.InsertAt((int)i, j);
		}
	}

	aiRuns.Init();
	for (i = 0; i < muiNumRuns; i++)
	{
		if (aiRunLengths.GetValue(i) == -1)
		{
			continue;
		}
		for (j = 0; j < aiRuns.NumElements(); j++)
		{
			if (aiRunLengths.GetValue(i) < aiRunLengths.GetValue(aiRuns.GetValue(j)))
			{
				break;
			}
		}
		aiRuns.InsertAt((int)i, j);
	}

	for (i = 0; i < aiRuns.NumElements(); i++)
	{
		iRun = aiRuns.GetValue(i);
		uiFirst = paiOrder->NumElements();
		for (j = 0; j < masConnections.NumElements(); j++)
		{
			psConnection = masConnections.Get(j);
			if (psConnection->iRun == iRun)
			{
				//Across the way they leave, as their bends are stepped.
				sFrom = psConnection->pcFrom->GetGridPosition();
				size	k;
				for (k = uiFirst; k < paiOrder->NumElements(); k++)
				{
					sOtherFrom = masConnections.Get(paiOrder->GetValue(k))->pcFrom->GetGridPosition();
					if ((psConnection->bHorizontal && (sFrom.y < sOtherFrom.y)) || (!psConnection->bHorizontal && (sFrom.x < sOtherFrom.x)))
					{
						break;
					}
				}
				paiOrder->InsertAt((int)j, k);
			}
		}
	}
	for (i = 0; i < aiSingles.NumElements(); i++)
	{
		paiOrder->Add(aiSingles.GetValue(i));
	}

	aiRuns.Kill();
	aiSingles.Kill();
	aiRunLengths.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A search from the from port, leaving the way it faces, to its to port
// (arriving the way it faces in) or to a wire already on its net.  While
// ripping it may go through other nets' new wires, at a cost, and the
// route isn't added: the nets in its way are collected.
//
//////////////////////////////////////////////////////////////////////////
bool CTraceAutoConnector::Route(STraceAutoConnection* psConnection, bool bRipping, CArrayInt* paiConflicts)
{
	int							iNet;
	int							iStart;
	int							iTarget;
	int							iFacing;
	int							iArriving;
	SInt32Vec2					sTarget;
	STraceAutoConnectHeapNode	sNode;
	int							iCell;
	int							iDirection;
	int							iNewDirection;
	int							iNewCell;
	int							iNewState;
	int32						iCost;
	int32						x;
	int32						y;
	size						uiPops;
	size						uiMaxPops;

	iNet = (int)psConnection->uiNet;
	iStart = GetCell(psConnection->pcFrom->GetGridPosition());
	sTarget = psConnection->pcTo->GetGridPosition();
	iTarget = GetCell(sTarget);
	miStamp++;
	MarkGoal(iTarget, iNet);
	if ((iStart == iTarget) || IsGoal(iStart))
	{
		psConnection->bConnected = true;
		return true;
	}

	iFacing = GetPortFacing(psConnection->pcFrom);
	iArriving = OppositeDirection(GetPortFacing(psConnection->pcTo));

	masHeap.ReInit();
	SetVisited(iStart * 4 + iFacing, 0, -1);
	PushHeap(GetHeuristic(iStart, sTarget), iStart * 4 + iFacing);

	uiPops = 0;
	uiMaxPops = (size)miGridWidth * (size)miGridHeight * 4;
	while (PopHeap(&sNode))
	{
		uiPops++;
		if (uiPops > uiMaxPops)
		{
			break;
		}

		iCell = sNode.iState / 4;
		iDirection = sNode.iState % 4;
		if (sNode.iCost - GetHeuristic(iCell, sTarget) > maiCosts.GetValue(sNode.iState))
		{
			//A cheaper way here was found after this was pushed.
			continue;
		}

		if (iCell != iStart)
		{
			if (((iCell == iTarget) && (iDirection == iArriving)) || ((iCell != iTarget) && IsGoal(iCell) && CanNode(iCell, iNet)))
			{
				if (bRipping)
				{
					CollectConflicts(sNode.iState, iNet, paiConflicts);
					return true;
				}
				AddRoute(sNode.iState, iNet);
				psConnection->bConnected = true;
				return true;
			}
		}

		x = iCell % miGridWidth;
		y = iCell / miGridWidth;
		for (iNewDirection = 0; iNewDirection < 4; iNewDirection++)
		{
			if (iNewDirection == OppositeDirection(iDirection))
			{
				continue;
			}
			if ((iCell == iStart) && (iNewDirection != iFacing))
			{
				continue;
			}
			iCost = 0;
			if ((iNewDirection != iDirection) && !CanNode(iCell, iNet))
			{
				if (!bRipping || !CanNodeRipping(iCell, iNet))
				{
					continue;
				}
				iCost += TRACE_AUTO_CONNECT_RIP_COST;
			}
			if ((x + gaiDirectionX[iNewDirection] < 0) || (x + gaiDirectionX[iNewDirection] >= miGridWidth) || (y + gaiDirectionY[iNewDirection] < 0) || (y + gaiDirectionY[iNewDirection] >= miGridHeight))
			{
				continue;
			}
			iNewCell = iCell + gaiDirectionX[iNewDirection] + gaiDirectionY[iNewDirection] * miGridWidth;
			if (!CanPass(iNewCell, iNet, IsDirectionHorizontal(iNewDirection)))
			{
				if (!bRipping || !CanPassRipping(iNewCell, iNet, IsDirectionHorizontal(iNewDirection)))
				{
					continue;
				}
				iCost += TRACE_AUTO_CONNECT_RIP_COST;
			}

			iCost += maiCosts.GetValue(sNode.iState) + GetStepCost(psConnection, iNewCell, iNewDirection);
			if (iNewDirection != iDirection)
			{
				iCost += TRACE_AUTO_CONNECT_BEND_COST;
			}
			iNewState = iNewCell * 4 + iNewDirection;
			if (!IsVisited(iNewState) || (iCost < maiCosts.GetValue(iNewState)))
			{
				SetVisited(iNewState, iCost, sNode.iState);
				PushHeap(iCost + GetHeuristic(iNewCell, sTarget), iNewState);
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The route back from the goal as straight wires, one for each run
// between turns.
//
//////////////////////////////////////////////////////////////////////////
void CTraceAutoConnector::AddRoute(int iGoalState, int iNet)
{
	int		iState;
	int		iPrevious;
	int		iCell;
	int		iEndCell;
	int		iDirection;

	iState = iGoalState;
	iEndCell = iState / 4;
	for (;;)
	{
		iPrevious = maiFrom.GetValue(iState);
		iDirection = iState % 4;
		if ((iPrevious == -1) || ((iPrevious % 4) != iDirection))
		{
			//The run that entered this state's cell in its direction starts at the previous state's cell.
			iCell = (iPrevious == -1) ? iState / 4 : iPrevious / 4;
			if (iCell != iEndCell)
			{
				mapcAddedTraces.Add(mpcCircuit->AddTrace(iCell % miGridWidth + msGridMinimum.x, iCell / miGridWidth + msGridMinimum.y, iEndCell % miGridWidth + msGridMinimum.x, iEndCell / miGridWidth + msGridMinimum.y));
				maiAddedTraceNets.Add(iNet);
				MarkTrace(iCell % miGridWidth + msGridMinimum.x, iCell / miGridWidth + msGridMinimum.y, iEndCell % miGridWidth + msGridMinimum.x, iEndCell / miGridWidth + msGridMinimum.y, iNet);
			}
			iEndCell = iCell;
		}
		if (iPrevious == -1)
		{
			break;
		}
		iState = iPrevious;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The nets a route found while ripping goes through: along their wires,
// or turning or ending on them, or over their turns and ends.  Crossing a
// wire is no conflict.
//
//////////////////////////////////////////////////////////////////////////
void CTraceAutoConnector::CollectConflicts(int iGoalState, int iNet, CArrayInt* paiConflicts)
{
	int		iState;
	int		iCell;
	int		iDirection;
	int		iNextDirection;
	bool	bNode;

	iState = iGoalState;
	iNextDirection = -1;
	while (iState != -1)
	{
		iCell = iState / 4;
		iDirection = iState % 4;
		bNode = (iNextDirection == -1) || (iNextDirection != iDirection) || (maiFrom.GetValue(iState) == -1);
		AddConflict(maiNodes.GetValue(iCell), iNet, paiConflicts);
		if (bNode || IsDirectionHorizontal(iDirection))
		{
			AddConflict(maiHorizontal.GetValue(iCell), iNet, paiConflicts);
		}
		if (bNode || !IsDirectionHorizontal(iDirection))
		{
			AddConflict(maiVertical.GetValue(iCell), iNet, paiConflicts);
		}
		iNextDirection = iDirection;
		iState = maiFrom.GetValue(iState);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTraceAutoConnector::AddConflict(int iOccupant, int iNet, CArrayInt* paiConflicts)
{
	if ((iOccupant != iNet) && IsRippable(iOccupant))
	{
		paiConflicts->AddIfUnique(iOccupant);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Takes out every wire routed for the net and leaves its connections
// unconnected.  Its ports stay kept for it.
//
//////////////////////////////////////////////////////////////////////////
void CTraceAutoConnector::RipNet(int iNet)
{
	size			i;
	CTraceView*		pcTrace;
	int				iCell;
	int				iCells;

	for (i = mapcAddedTraces.NumElements(); i > 0; i--)
	{
		if (maiAddedTraceNets.GetValue(i - 1) == iNet)
		{
			pcTrace = mapcAddedTraces.GetPtr(i - 1);
			mpcCircuit->RemoveTrace(pcTrace);
			SafeKill(pcTrace);
			mapcAddedTraces.RemoveAt(i - 1);
			maiAddedTraceNets.RemoveAt(i - 1);
		}
	}

	iCells = miGridWidth * miGridHeight;
	for (iCell = 0; iCell < iCells; iCell++)
	{
		if (maiHorizontal.GetValue(iCell) == iNet)
		{
			maiHorizontal.SetValue(iCell, -1);
		}
		if (maiVertical.GetValue(iCell) == iNet)
		{
			maiVertical.SetValue(iCell, -1);
		}
		if (maiNodes.GetValue(iCell) == iNet)
		{
			maiNodes.SetValue(iCell, -1);
		}
	}
	for (i = 0; i < mapcPorts.NumElements(); i++)
	{
		if (maiPortNets.GetValue(i) == iNet)
		{
			maiNodes.SetValue(GetCell(mapcPorts.GetPtr(i)->GetGridPosition()), iNet);
		}
	}
	for (i = 0; i < masConnections.NumElements(); i++)
	{
		if ((int)masConnections.Get(i)->uiNet == iNet)
		{
			masConnections.Get(i)->bConnected = false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A net being connected that has only wires routed here (none of its
// wires were already in the circuit).
//
//////////////////////////////////////////////////////////////////////////
bool CTraceAutoConnector::IsRippable(int iOccupant)
{
	return (iOccupant >= 0) && (iOccupant < (int)muiNumNets) && (maiNetsWithExisting.GetValue(iOccupant) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// As CanPass, but through the wires of nets that can be ripped up.
//
//////////////////////////////////////////////////////////////////////////
bool CTraceAutoConnector::CanPassRipping(int iCell, int iNet, bool bHorizontal)
{
	int		iOwner;
	int		iNode;
	int		iAlong;

	iOwner = maiOwners.GetValue(iCell);
	if ((iOwner == OWNER_BODY) || (iOwner == OWNER_FOREIGN) || ((iOwner >= 0) && (iOwner != iNet)))
	{
		return false;
	}
	iNode = maiNodes.GetValue(iCell);
	if ((iNode != -1) && (iNode != iNet) && !IsRippable(iNode))
	{
		return false;
	}
	iAlong = bHorizontal ? maiHorizontal.GetValue(iCell) : maiVertical.GetValue(iCell);
	return (iAlong == -1) || (iAlong == iNet) || IsRippable(iAlong);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTraceAutoConnector::CanNodeRipping(int iCell, int iNet)
{
	return CanPassRipping(iCell, iNet, true) && CanPassRipping(iCell, iNet, false);
}


//////////////////////////////////////////////////////////////////////////
//
// Which way the port faces out of its component: across the edge of the
// component's box it's nearest, left or right before up or down.
//
//////////////////////////////////////////////////////////////////////////
int CTraceAutoConnector::GetPortFacing(CPortView* pcPortView)
{
	SFloat64Vec2	sMinimum;
	SFloat64Vec2	sMaximum;
	SInt32Vec2		sPosition;
	float64			afDistances[4];
	int				iDirection;
	int				iBest;

	if (!pcPortView->GetOwner()->GetBoundingBox(&sMinimum, &sMaximum))
	{
		return TRACE_DIRECTION_EAST;
	}
	sPosition = pcPortView->GetGridPosition();
	afDistances[TRACE_DIRECTION_EAST] = sMaximum.x - sPosition.x;
	afDistances[TRACE_DIRECTION_SOUTH] = sMaximum.y - sPosition.y;
	afDistances[TRACE_DIRECTION_WEST] = sPosition.x - sMinimum.x;
	afDistances[TRACE_DIRECTION_NORTH] = sPosition.y - sMinimum.y;

	iBest = TRACE_DIRECTION_WEST;
	if (afDistances[TRACE_DIRECTION_EAST] < afDistances[iBest])
	{
		iBest = TRACE_DIRECTION_EAST;
	}
	for (iDirection = TRACE_DIRECTION_SOUTH; iDirection <= TRACE_DIRECTION_NORTH; iDirection += 2)
	{
		if (afDistances[iDirection] < afDistances[iBest])
		{
			iBest = iDirection;
		}
	}
	return iBest;
}


//////////////////////////////////////////////////////////////////////////
//
// A route can go through a cell along a way unless the cell is a
// component's, or kept for another net, or has another net's port, turn
// or wire end on it, or another net's wire along the same way.
//
//////////////////////////////////////////////////////////////////////////
bool CTraceAutoConnector::CanPass(int iCell, int iNet, bool bHorizontal)
{
	int		iOwner;
	int		iNode;
	int		iAlong;

	iOwner = maiOwners.GetValue(iCell);
	if ((iOwner == OWNER_BODY) || (iOwner == OWNER_FOREIGN) || ((iOwner >= 0) && (iOwner != iNet)))
	{
		return false;
	}
	iNode = maiNodes.GetValue(iCell);
	if ((iNode != -1) && (iNode != iNet))
	{
		return false;
	}
	iAlong = bHorizontal ? maiHorizontal.GetValue(iCell) : maiVertical.GetValue(iCell);
	return (iAlong == -1) || (iAlong == iNet);
}


//////////////////////////////////////////////////////////////////////////
//
// A route can turn or end in a cell it can pass both ways.
//
//////////////////////////////////////////////////////////////////////////
bool CTraceAutoConnector::CanNode(int iCell, int iNet)
{
	return CanPass(iCell, iNet, true) && CanPass(iCell, iNet, false);
}


//////////////////////////////////////////////////////////////////////////
//
// The cells of the net's wires that are joined to the target port, for
// this search: along a wire, and from one way to the other only where a
// wire ends or turns (wires of a net that cross aren't joined).
//
//////////////////////////////////////////////////////////////////////////
void CTraceAutoConnector::MarkGoal(int iTarget, int iNet)
{
	size	uiHead;
	int		iState;
	int		iCell;
	int		iAxis;
	int		iDirection;
	int		iNewCell;
	int32	x;
	int32	y;

	maiGoalQueue.ReInit();
	maiGoalStamps.SetValue(iTarget * 2, miStamp);
	maiGoalStamps.SetValue(iTarget * 2 + 1, miStamp);
	maiGoalQueue.Add(iTarget * 2);
	maiGoalQueue.Add(iTarget * 2 + 1);
	for (uiHead = 0; uiHead < maiGoalQueue.NumElements(); uiHead++)
	{
		iState = maiGoalQueue.GetValue(uiHead);
		iCell = iState / 2;
		iAxis = iState % 2;
		x = iCell % miGridWidth;
		y = iCell / miGridWidth;
		for (iDirection = iAxis; iDirection < 4; iDirection += 2)
		{
			if ((x + gaiDirectionX[iDirection] < 0) || (x + gaiDirectionX[iDirection] >= miGridWidth) || (y + gaiDirectionY[iDirection] < 0) || (y + gaiDirectionY[iDirection] >= miGridHeight))
			{
				continue;
			}
			iNewCell = iCell + gaiDirectionX[iDirection] + gaiDirectionY[iDirection] * miGridWidth;
			if ((iAxis == 0) ? ((maiHorizontal.GetValue(iCell) != iNet) || (maiHorizontal.GetValue(iNewCell) != iNet)) : ((maiVertical.GetValue(iCell) != iNet) || (maiVertical.GetValue(iNewCell) != iNet)))
			{
				continue;
			}
			if (maiGoalStamps.GetValue(iNewCell * 2 + iAxis) != miStamp)
			{
				maiGoalStamps.SetValue(iNewCell * 2 + iAxis, miStamp);
				maiGoalQueue.Add(iNewCell * 2 + iAxis);
			}
			if ((maiNodes.GetValue(iNewCell) == iNet) && (maiGoalStamps.GetValue(iNewCell * 2 + 1 - iAxis) != miStamp))
			{
				maiGoalStamps.SetValue(iNewCell * 2 + 1 - iAxis, miStamp);
				maiGoalQueue.Add(iNewCell * 2 + 1 - iAxis);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTraceAutoConnector::IsGoal(int iCell)
{
	return (maiGoalStamps.GetValue(iCell * 2) == miStamp) || (maiGoalStamps.GetValue(iCell * 2 + 1) == miStamp);
}


//////////////////////////////////////////////////////////////////////////
//
// The cost of going into the cell in the direction, not counting a turn.
//
//////////////////////////////////////////////////////////////////////////
int CTraceAutoConnector::GetStepCost(STraceAutoConnection* psConnection, int iCell, int iDirection)
{
	int		iCost;
	int		iAcross;
	int		iNet;
	int32	iOffBend;
	bool	bHorizontal;
	int32	x;
	int32	y;

	iNet = (int)psConnection->uiNet;
	bHorizontal = IsDirectionHorizontal(iDirection);
	iCost = TRACE_AUTO_CONNECT_STEP_COST;
	if (maiNear.GetValue(iCell) != 0)
	{
		iCost += TRACE_AUTO_CONNECT_NEAR_COST;
	}
	iAcross = bHorizontal ? maiVertical.GetValue(iCell) : maiHorizontal.GetValue(iCell);
	if ((iAcross != -1) && (iAcross != iNet))
	{
		iCost += TRACE_AUTO_CONNECT_CROSS_COST;
	}

	//Another net's wire running alongside, either side.
	x = iCell % miGridWidth;
	y = iCell / miGridWidth;
	if (bHorizontal)
	{
		if ((y > 0) && IsCrowding(maiHorizontal.GetValue(iCell - miGridWidth), iNet))
		{
			iCost += TRACE_AUTO_CONNECT_CROWD_COST;
		}
		if ((y < miGridHeight - 1) && IsCrowding(maiHorizontal.GetValue(iCell + miGridWidth), iNet))
		{
			iCost += TRACE_AUTO_CONNECT_CROWD_COST;
		}
	}
	else
	{
		if ((x > 0) && IsCrowding(maiVertical.GetValue(iCell - 1), iNet))
		{
			iCost += TRACE_AUTO_CONNECT_CROWD_COST;
		}
		if ((x < miGridWidth - 1) && IsCrowding(maiVertical.GetValue(iCell + 1), iNet))
		{
			iCost += TRACE_AUTO_CONNECT_CROWD_COST;
		}
	}
	if (psConnection->bHasBend && (bHorizontal != psConnection->bHorizontal))
	{
		if (psConnection->bHorizontal)
		{
			iOffBend = IntAbs(iCell % miGridWidth + msGridMinimum.x - psConnection->iBend);
		}
		else
		{
			iOffBend = IntAbs(iCell / miGridWidth + msGridMinimum.y - psConnection->iBend);
		}
		iOffBend *= TRACE_AUTO_CONNECT_HINT_COST;
		iCost += (iOffBend < TRACE_AUTO_CONNECT_HINT_LIMIT) ? iOffBend : TRACE_AUTO_CONNECT_HINT_LIMIT;
	}
	return iCost;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a wire of the occupant beside a route of the net crowds it: it's
// another net, and not one in the same run.
//
//////////////////////////////////////////////////////////////////////////
bool CTraceAutoConnector::IsCrowding(int iOccupant, int iNet)
{
	if ((iOccupant == -1) || (iOccupant == iNet))
	{
		return false;
	}
	if ((iOccupant < (int)muiNumNets) && (maiNetRuns.GetValue(iNet) != -1) && (maiNetRuns.GetValue(iOccupant) == maiNetRuns.GetValue(iNet)))
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The length to the to port.  It's not always the least the cost could be
// (a route can end on its net's wires before it), so a route is good
// rather than the cheapest.
//
//////////////////////////////////////////////////////////////////////////
int CTraceAutoConnector::GetHeuristic(int iCell, SInt32Vec2 sTarget)
{
	int32	x;
	int32	y;

	x = iCell % miGridWidth + msGridMinimum.x;
	y = iCell / miGridWidth + msGridMinimum.y;
	return (IntAbs(sTarget.x - x) + IntAbs(sTarget.y - y)) * TRACE_AUTO_CONNECT_STEP_COST;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTraceAutoConnector::IsInGrid(int32 x, int32 y)
{
	x -= msGridMinimum.x;
	y -= msGridMinimum.y;
	return (x >= 0) && (x < miGridWidth) && (y >= 0) && (y < miGridHeight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTraceAutoConnector::GetCell(int32 x, int32 y)
{
	return (x - msGridMinimum.x) + (y - msGridMinimum.y) * miGridWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTraceAutoConnector::GetCell(SInt32Vec2 sPosition)
{
	return GetCell(sPosition.x, sPosition.y);
}


//////////////////////////////////////////////////////////////////////////
//
// A binary heap, cheapest first.
//
//////////////////////////////////////////////////////////////////////////
void CTraceAutoConnector::PushHeap(int32 iCost, int32 iState)
{
	size						uiIndex;
	size						uiParent;
	STraceAutoConnectHeapNode*	psNode;
	STraceAutoConnectHeapNode	sSwap;

	psNode = masHeap.Add();
	psNode->iCost = iCost;
	psNode->iState = iState;
	uiIndex = masHeap.NumElements() - 1;
	while (uiIndex > 0)
	{
		uiParent = (uiIndex - 1) / 2;
		if (masHeap.Get(uiParent)->iCost <= masHeap.Get(uiIndex)->iCost)
		{
			break;
		}
		sSwap = *masHeap.Get(uiParent);
		*masHeap.Get(uiParent) = *masHeap.Get(uiIndex);
		*masHeap.Get(uiIndex) = sSwap;
		uiIndex = uiParent;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTraceAutoConnector::PopHeap(STraceAutoConnectHeapNode* psNode)
{
	size						uiIndex;
	size						uiChild;
	size						uiNum;
	STraceAutoConnectHeapNode	sSwap;

	uiNum = masHeap.NumElements();
	if (uiNum == 0)
	{
		return false;
	}
	*psNode = *masHeap.Get(0);
	*masHeap.Get(0) = *masHeap.Get(uiNum - 1);
	masHeap.RemoveTail();
	uiNum--;

	uiIndex = 0;
	for (;;)
	{
		uiChild = uiIndex * 2 + 1;
		if (uiChild >= uiNum)
		{
			break;
		}
		if ((uiChild + 1 < uiNum) && (masHeap.Get(uiChild + 1)->iCost < masHeap.Get(uiChild)->iCost))
		{
			uiChild++;
		}
		if (masHeap.Get(uiIndex)->iCost <= masHeap.Get(uiChild)->iCost)
		{
			break;
		}
		sSwap = *masHeap.Get(uiChild);
		*masHeap.Get(uiChild) = *masHeap.Get(uiIndex);
		*masHeap.Get(uiIndex) = sSwap;
		uiIndex = uiChild;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// States are visited in this search if they have its stamp.
//
//////////////////////////////////////////////////////////////////////////
void CTraceAutoConnector::SetVisited(int iState, int32 iCost, int32 iFrom)
{
	maiStamps.SetValue(iState, miStamp);
	maiCosts.SetValue(iState, iCost);
	maiFrom.SetValue(iState, iFrom);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTraceAutoConnector::IsVisited(int iState)
{
	return maiStamps.GetValue(iState) == miStamp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTraceAutoConnector::NumConnections(void) { return masConnections.NumElements(); }
bool CTraceAutoConnector::IsConnected(size uiIndex) { return masConnections.Get(uiIndex)->bConnected; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTraceAutoConnector::NumUnconnected(void)
{
	size	i;
	size	uiUnconnected;

	uiUnconnected = 0;
	for (i = 0; i < masConnections.NumElements(); i++)
	{
		if (!masConnections.Get(i)->bConnected)
		{
			uiUnconnected++;
		}
	}
	return uiUnconnected;
}


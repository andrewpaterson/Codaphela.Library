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
#ifndef __TRACE_AUTO_CONNECTOR_H__
#define __TRACE_AUTO_CONNECTOR_H__
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayTemplate.h"
#include "CircuitView.h"


//A connection with no hint takes the cheapest route.
#define TRACE_AUTO_CONNECT_NO_BEND		-1.0


//The costs of a route, in tenths of a grid unit of wire.
#define TRACE_AUTO_CONNECT_STEP_COST	10
#define TRACE_AUTO_CONNECT_BEND_COST	40
#define TRACE_AUTO_CONNECT_CROSS_COST	30
#define TRACE_AUTO_CONNECT_NEAR_COST	4
#define TRACE_AUTO_CONNECT_HINT_COST	6
#define TRACE_AUTO_CONNECT_HINT_LIMIT	60
#define TRACE_AUTO_CONNECT_RIP_COST		200
#define TRACE_AUTO_CONNECT_CROWD_COST	4


//How many cells straight out in front of a port are kept for its net.
#define TRACE_AUTO_CONNECT_APPROACH		1


//The grid margin around everything in the circuit that routes can use.
#define TRACE_AUTO_CONNECT_MARGIN		8


//A port to be wired to another on the same net.  A connection leaves its from port in the direction the port faces
//out of its component (across the nearest edge of its box) and its first change of direction is at its bend: that
//fraction of the way across the gap between the two components' facing edges (not the ports), plus its offset in
//its run.  It ends on its to port or on any wire already routed for its net.
struct STraceAutoConnection
{
	size			uiNet;
	CPortView*		pcFrom;
	CPortView*		pcTo;
	float64			fBend;
	int				iRun;
	bool			bHorizontal;
	int32			iBend;
	bool			bHasBend;
	bool			bConnected;
	int32			iLength;
};


typedef CArrayTemplate<STraceAutoConnection>	CArrayTraceAutoConnection;


struct STraceAutoConnectHeapNode
{
	int32	iCost;
	int32	iState;
};


typedef CArrayTemplate<STraceAutoConnectHeapNode>	CArrayTraceAutoConnectHeapNode;


//Wires ports together in a circuit view, as straight horizontal and vertical traces that keep off the components and
//off the other nets, so Logicim's wiring (see CCircuitWiring) joins exactly the ports of each net.  A route may cross
//another net's wire, as wires that cross without an end on each other aren't joined, but it never runs along it,
//turns or ends on it, or passes over a port that isn't on its net.  Wires already in the circuit stay: one touching
//a port of a net is that net's (a connection can end on it) and the others are kept clear of.
//
//The connections of a run (a bus: several nets between the same components) are routed one after another with their
//bends stepped apart, so they turn in a stair and don't cross each other.  Runs are routed shortest first, then the
//connections that aren't in one, shortest first.  A connection is routed by a search of the grid that charges for
//length, turns, crossings, running beside a component, running right beside another net's wire (unless they're in
//the same run, which is meant to be a bundle) and for its turn being away from its bend.  A connection that
//can't be routed is tried again through the other nets' new wires (at a cost); the nets it went through are ripped
//up, it's routed, and they're routed again.  A connection that still can't be routed is left unconnected and the
//others are still routed.
//
//It's used to wire a circuit made from somewhere else (a KiCad schematic) and to rewire a component's connections
//when it is moved.
class CTraceAutoConnector
{
protected:
	CCircuitView*					mpcCircuit;
	CArrayTraceAutoConnection		masConnections;
	size							muiNumNets;
	size							muiNumRuns;
	CArrayPortViewPtr				mapcPorts;
	CArrayInt						maiPortNets;

	SInt32Vec2						msGridMinimum;
	int32							miGridWidth;
	int32							miGridHeight;
	CArrayInt						maiOwners;
	CArrayInt						maiHorizontal;
	CArrayInt						maiVertical;
	CArrayInt						maiNodes;
	CArrayInt						maiNear;

	CArrayInt						maiCosts;
	CArrayInt						maiFrom;
	CArrayInt						maiStamps;
	int								miStamp;
	CArrayTraceAutoConnectHeapNode	masHeap;
	CArrayInt						maiGoalStamps;
	CArrayInt						maiGoalQueue;

	CArrayTraceViewPtr				mapcAddedTraces;
	CArrayInt						maiAddedTraceNets;
	CArrayInt						maiNetsWithExisting;
	CArrayInt						maiNetRuns;

public:
	void			Init(CCircuitView* pcCircuit);
	void			Kill(void);

	size			AddNet(void);
	size			AddRun(void);
	bool			AddConnection(size uiNet, CPortView* pcFrom, CPortView* pcTo, float64 fBend, int iRun = -1);

	bool			Connect(void);

	size			NumConnections(void);
	bool			IsConnected(size uiIndex);
	size			NumUnconnected(void);

	static int		GetPortFacing(CPortView* pcPortView);

protected:
	bool			AddPort(CPortView* pcPortView, size uiNet);
	int				GetPortNet(CPortView* pcPortView);
	void			CreateGrid(void);
	void			IncludeGridPoint(SInt32Vec2* psMinimum, SInt32Vec2* psMaximum, bool* pbHasBounds, int32 x, int32 y);
	void			MarkComponents(void);
	void			MarkPorts(void);
	void			MarkExistingTraces(void);
	void			MarkTrace(int32 x0, int32 y0, int32 x1, int32 y1, int iNet);
	void			CalculateBends(void);
	void			CalculateRunBends(int iRun);
	void			SortConnections(CArrayInt* paiOrder);
	bool			Route(STraceAutoConnection* psConnection, bool bRipping, CArrayInt* paiConflicts);
	void			AddRoute(int iGoalState, int iNet);
	void			CollectConflicts(int iGoalState, int iNet, CArrayInt* paiConflicts);
	void			AddConflict(int iOccupant, int iNet, CArrayInt* paiConflicts);
	void			RipNet(int iNet);
	bool			IsRippable(int iOccupant);
	bool			CanPassRipping(int iCell, int iNet, bool bHorizontal);
	bool			CanNodeRipping(int iCell, int iNet);

	bool			IsInGrid(int32 x, int32 y);
	int				GetCell(int32 x, int32 y);
	int				GetCell(SInt32Vec2 sPosition);
	bool			CanPass(int iCell, int iNet, bool bHorizontal);
	bool			CanNode(int iCell, int iNet);
	bool			IsOnNet(int iCell, int iNet);
	void			MarkGoal(int iTarget, int iNet);
	bool			IsGoal(int iCell);
	int				GetStepCost(STraceAutoConnection* psConnection, int iCell, int iDirection);
	bool			IsCrowding(int iOccupant, int iNet);
	int				GetHeuristic(int iCell, SInt32Vec2 sTarget);

	void			PushHeap(int32 iCost, int32 iState);
	bool			PopHeap(STraceAutoConnectHeapNode* psNode);
	void			SetVisited(int iState, int32 iCost, int32 iFrom);
	bool			IsVisited(int iState);
};


#endif // __TRACE_AUTO_CONNECTOR_H__


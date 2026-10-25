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
#ifndef __CIRCUIT_WIRING_H__
#define __CIRCUIT_WIRING_H__
#include "BaseLib/ArrayInt.h"
#include "ComponentView.h"
#include "TraceView.h"


class CCircuitView;
class CCircuitSimulation;


//One circuit on one view path of the circuits being wired.  Its wires and then its port views are nodes, numbered
//from uiFirstNode.  A level after the first is an instance (pcInstanceView) of a subcircuit in the circuit of its
//parent level.
struct SWiringLevel
{
	CViewPath*				pcViewPath;
	CCircuitView*			pcCircuit;
	CArrayPortViewPtr		apcPortViews;
	size					uiFirstNode;
	int						iParent;
	CCircuitInstanceView*	pcInstanceView;
};


typedef CArrayTemplate<SWiringLevel>	CArrayWiringLevel;


//Wires a circuit's views, and those of the subcircuits in it all the way down, in a circuit simulation, as Logicim's
//WireListFinder: the components must already be made for every path.  Nets are found across the levels in two
//steps.  Wires, port views and tunnels in a circuit, and the port view of each of an instance's pins with the port
//view of the pin inside its subcircuit, are joined into nets, each a bus as wide as its widest port view.  Then the
//bits of the nets that components join (a splitter joins each bit of its bus to a bit of one of its ends) are
//joined, and each set of joined bits gets one trace.  So a signal through a subcircuit's pin is one trace on both
//sides of it.
//
//Rewire wires a running simulation again after its circuits have been edited.  The simulation must be of the path's
//circuit only: traces in it that end up with no ports and no wire are removed.
class CCircuitWiring
{
protected:
	CCircuitSimulation*			mpcSimulation;
	CArrayWiringLevel			masLevels;
	size						muiNumNodes;
	CArrayInt					maiParents;
	CArrayInt					maiNetWidths;
	CArrayInt					maiNetIndices;
	size						muiNumNets;
	CArrayInt					maiNetFirstBits;
	CArrayInt					maiNetBitWidths;
	CArrayInt					maiBitParents;
	CArrayTraceViewTracePtr*	mapcNetTraces;

public:
	void	Init(CCircuitSimulation* pcSimulation);
	void	Kill(void);

	bool	Wire(CViewPath* pcViewPath, bool bStart);
	bool	Rewire(CViewPath* pcViewPath);

protected:
	bool	JoinNodes(CViewPath* pcViewPath);
	bool	AddLevels(CViewPath* pcViewPath, int iParent, CCircuitInstanceView* pcInstanceView);
	void	JoinLevel(SWiringLevel* psLevel);
	void	JoinPins(SWiringLevel* psLevel);
	bool	FindNets(void);
	void	FindBits(void);
	void	ReuseBitTraces(CArrayTraceViewTracePtr* papcBitTraces);
	void	AddBitTraces(CArrayTraceViewTracePtr* papcBitTraces);
	void	CreateNetTraces(CArrayTraceViewTracePtr* papcBitTraces);
	void	SetLevelTraces(SWiringLevel* psLevel);
	void	DisconnectLevel(SWiringLevel* psLevel);
	bool	ConnectLevel(SWiringLevel* psLevel, bool bStart);
	void	RemoveUnusedTraces(void);
	bool	IsNetTrace(CTrace* pcTrace);
	int		GetBitRoot(int iNet, size uiBit);
	size	NumLevelTraces(SWiringLevel* psLevel);
	bool	IsTraceNode(size uiNode);
	int		GetPortViewNode(SWiringLevel* psLevel, CPortView* pcPortView);
	int		GetPortViewNet(SWiringLevel* psLevel, CPortView* pcPortView);
	int		FindRoot(CArrayInt* paiParents, int iNode);
	void	Join(CArrayInt* paiParents, int iNode1, int iNode2);
};


#endif // __CIRCUIT_WIRING_H__


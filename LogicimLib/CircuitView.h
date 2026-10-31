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
#ifndef __CIRCUIT_VIEW_H__
#define __CIRCUIT_VIEW_H__
#include "BaseLib/ArrayInt.h"
#include "MicroprocessorLib/CircuitComponent.h"
#include "ComponentView.h"
#include "TraceView.h"
#include "TraceLines.h"


class CCircuitSimulation;
class CCircuit;
class CFamilyVoltageConfigurationStore;


//The components and wires of one circuit, as the views part of Logicim's SubcircuitView, named by its type name (a
//circuit on its own has none).  It owns its views.  CreateSimulation makes each component in a circuit simulation for
//a view path ending in this circuit, and those of the subcircuits in it for the paths through their instances, starts
//it, and then wires the ports the wires connect, live, as Logicim's editor does (see CCircuitWiring).  Without bStart
//it wires them without starting, for a saved simulation to be loaded into.  A circuit is the first view of its own top
//path (as Logicim's SubcircuitEditor); simulating its top path puts the circuit simulation's top simulation on the
//path.
//
//KillTraces is only for a circuit that isn't being simulated: the simulation's traces aren't rewired.
//
//A simulated circuit is edited live, as CCircuitEditor does: CreateComponents makes added views' components, and
//DestroyComponent takes removed views' components out of the running simulation, and then the simulation is wired
//again (see CCircuitWiring::Rewire).
//
//SaveXML writes a <Circuit> element (with a type attribute if it has a type name) holding an element per component
//(in the order they were added) and then a <Trace> per wire.  LoadXML adds what such an element holds to the circuit
//view.  Instances of subcircuits in it are loaded without their subcircuits, which are found by type name afterwards
//(see CCircuitViews).
class CCircuitView : public CCircuitInstanceView
{
protected:
	CChars					mszTypeName;
	CArrayComponentViewPtr	mapcComponents;
	CArrayTraceViewPtr		mapcTraces;

public:
	void				Init(void);
	void				Kill(void);

	CComponentView*		Add(CComponentView* pcComponent);
	CTraceView*			AddTrace(int32 x0, int32 y0, int32 x1, int32 y1);
	CTraceView*			AddTraces(int32 x0, int32 y0, int32 x1, int32 y1);
	bool				Remove(CComponentView* pcComponent);
	bool				Replace(CComponentView* pcOld, CComponentView* pcNew);
	bool				RemoveTrace(CTraceView* pcTrace);
	void				InsertTrace(CTraceView* pcTrace, size uiIndex);
	void				ReplaceWires(CArrayTraceLine* pasLines, CArrayTraceViewPtr* papcSeeds, CArrayTraceViewPtr* papcAdded);
	void				KillTraces(void);

	size				NumComponents(void);
	CComponentView*		GetComponent(size uiIndex);
	size				NumTraces(void);
	CTraceView*			GetTrace(size uiIndex);
	void				CollectPortViews(CArrayPortViewPtr* papcPortViews);

	bool				CreateSimulation(CViewPath* pcViewPath, CCircuitSimulation* pcSimulation, CFamilyVoltageConfigurationStore* pcFamilies, bool bStart = true);
	void				DisconnectSimulation(CViewPath* pcViewPath, CCircuitSimulation* pcSimulation);
	bool				CreateComponents(CViewPath* pcViewPath, CCircuitSimulation* pcSimulation, CFamilyVoltageConfigurationStore* pcFamilies, CArrayComponentViewPtr* papcComponents, CArrayCircuitComponentPtr* papcCreated);
	void				DestroyComponent(CComponentView* pcComponent, CViewPath* pcViewPath, CCircuitSimulation* pcSimulation);
	void				CalculateJunctions(void);

	CCircuitView*		GetInstanceCircuitView(void);
	void				AppendDescription(CChars* psz);
	void				SetTypeName(const char* szTypeName);
	char*				GetTypeName(void);

	void				SaveXML(CXMLWriter* pcWriter);
	bool				LoadXML(CMarkupTag* pcCircuitTag);

	void				Paint(CCircuitPainter* pcPainter);
	bool				GetBoundingBox(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum);
	CComponentView*		GetComponentAt(SFloat64Vec2 sPosition);
	CTraceView*			GetTraceAt(SInt32Vec2 sPoint);
	bool				IsConnectionAt(SInt32Vec2 sPoint);
	CPortView*			GetPortViewAt(SInt32Vec2 sPoint);
	void				GetViewsInRectangle(SFloat64Vec2 sStart, SFloat64Vec2 sEnd, CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces);

protected:
	void				DestroySimulation(CViewPath* pcViewPath, CCircuitSimulation* pcSimulation);
	void				AddTouchingTraces(STraceLine* psLine, CArrayTraceViewPtr* papcTraces);
};


void	PaintCircuitGrid(CCircuitPainter* pcPainter);
int32	RoundToGrid(float64 f);
bool	IsInBox(SFloat64Vec2 sPosition, SFloat64Vec2 sMinimum, SFloat64Vec2 sMaximum);
bool	IsBoxInRectangle(SFloat64Vec2 sStart, SFloat64Vec2 sEnd, SFloat64Vec2 sMinimum, SFloat64Vec2 sMaximum, bool bIntersections);


#endif // __CIRCUIT_VIEW_H__

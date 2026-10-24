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
#include "ComponentView.h"
#include "TraceView.h"


class CCircuitSimulation;
class CFamilyVoltageConfigurationStore;


//The components and wires of one circuit, as the views part of Logicim's SubcircuitView.  It owns its views.
//CreateSimulation makes each component in a simulation, starts it, and then wires the ports the wires connect, live,
//as Logicim's editor does.  Without bStart it wires them without starting, for a saved simulation to be loaded into: wires that touch, ports on a wire and ports on the same grid point are one net with a
//trace per bit.
//
//SaveXML writes a <Circuit> element holding an element per component (in the order they were added) and then a <Trace>
//per wire.  LoadXML adds what such an element holds to the circuit view.
class CCircuitView
{
protected:
	CArrayComponentViewPtr	mapcComponents;
	CArrayTraceViewPtr		mapcTraces;

public:
	void				Init(void);
	void				Kill(void);

	CComponentView*		Add(CComponentView* pcComponent);
	CTraceView*			AddTrace(int32 x0, int32 y0, int32 x1, int32 y1);
	CTraceView*			AddTraces(int32 x0, int32 y0, int32 x1, int32 y1);

	size				NumComponents(void);
	CComponentView*		GetComponent(size uiIndex);
	size				NumTraces(void);
	CTraceView*			GetTrace(size uiIndex);

	bool				CreateSimulation(CCircuitSimulation* pcSimulation, CFamilyVoltageConfigurationStore* pcFamilies, bool bStart = true);
	void				DisconnectSimulation(void);
	void				CalculateJunctions(void);

	void				SaveXML(CXMLWriter* pcWriter);
	bool				LoadXML(CMarkupTag* pcCircuitTag);

	void				Paint(CCircuitPainter* pcPainter);
	bool				GetBoundingBox(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum);

protected:
	void				CollectPortViews(CArrayPortViewPtr* papcPortViews);
	int					FindRoot(CArrayInt* paiParents, int iNode);
	void				Join(CArrayInt* paiParents, int iNode1, int iNode2);
};


void PaintCircuitGrid(CCircuitPainter* pcPainter);


#endif // __CIRCUIT_VIEW_H__

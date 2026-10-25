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
#ifndef __CIRCUIT_SELECTION_H__
#define __CIRCUIT_SELECTION_H__
#include "ComponentView.h"
#include "TraceView.h"


class CCircuitPainter;


//How a selection rectangle changes the selection, as Logicim's SelectionMode: shift adds to it and control with shift
//takes away from it.
enum ESelectionMode
{
	SM_Replace,
	SM_Add,
	SM_Subtract,
};


//The selected components and wires of the circuit being edited, as Logicim's Selection.  The views belong to the
//circuit.
class CCircuitSelection
{
protected:
	CArrayComponentViewPtr	mapcComponents;
	CArrayTraceViewPtr		mapcTraces;

public:
	void				Init(void);
	void				Kill(void);

	void				Clear(void);
	bool				IsEmpty(void);
	void				Set(CCircuitSelection* pcSource);
	void				Set(CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces, CCircuitSelection* pcPrevious, ESelectionMode eMode);
	void				Add(CComponentView* pcComponent);
	void				Add(CTraceView* pcTrace);
	void				Remove(CComponentView* pcComponent);
	void				Remove(CTraceView* pcTrace);
	bool				Contains(CComponentView* pcComponent);
	bool				Contains(CTraceView* pcTrace);
	bool				Equals(CCircuitSelection* pcOther);

	size				NumComponents(void);
	CComponentView*		GetComponent(size uiIndex);
	CArrayComponentViewPtr*	GetComponents(void);
	size				NumTraces(void);
	CTraceView*			GetTrace(size uiIndex);
	CArrayTraceViewPtr*	GetTraces(void);
	CComponentView*		GetSingleComponent(void);
	bool				GetCentre(SInt32Vec2* psCentre);
	bool				IsOn(SFloat64Vec2 sPosition);

	void				Paint(CCircuitPainter* pcPainter);
};


ESelectionMode	CalculateSelectionMode(bool bControl, bool bShift);
void			PaintSelectionMarkers(CCircuitPainter* pcPainter, CComponentView* pcComponent, ARGB32 sColour);
void			PaintSelectionMarker(CCircuitPainter* pcPainter, float64 x, float64 y, ARGB32 sColour);


#endif // __CIRCUIT_SELECTION_H__


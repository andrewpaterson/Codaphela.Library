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
#ifndef __CIRCUIT_VIEWS_H__
#define __CIRCUIT_VIEWS_H__
#include "CircuitView.h"


typedef CArrayTemplatePtr<CCircuitView>	CArrayCircuitViewPtr;


//The circuits of a project by type name, as the subcircuit editors of Logicim's CircuitEditor: each can be simulated
//on its own, on its top path, and placed in the others as instances.  It owns the circuits and the view paths
//through them; a circuit's top path is made with it.
//
//SaveXML writes a <Circuits> element holding each circuit's <Circuit type="...">, in the order they were added.
//LoadXML adds the circuits such an element holds and then gives each instance of a subcircuit in them the circuit
//of its type (SetInstanceCircuitViews).
class CCircuitViews
{
protected:
	CArrayCircuitViewPtr	mapcCircuits;
	CViewPaths				mcViewPaths;

public:
	void			Init(void);
	void			Kill(void);

	CCircuitView*	Add(const char* szTypeName);
	CCircuitView*	Get(const char* szTypeName);
	size			NumCircuits(void);
	CCircuitView*	Get(size uiIndex);

	CViewPaths*		GetViewPaths(void);
	CViewPath*		GetTopPath(CCircuitView* pcCircuit);

	bool			SetInstanceCircuitViews(void);

	void			SaveXML(CXMLWriter* pcWriter);
	bool			LoadXML(CMarkupTag* pcCircuitsTag);
};


#endif // __CIRCUIT_VIEWS_H__


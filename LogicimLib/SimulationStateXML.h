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
#ifndef __SIMULATION_STATE_XML_H__
#define __SIMULATION_STATE_XML_H__
#include "BaseLib/XMLWriter.h"
#include "BaseLib/MarkupTag.h"
#include "CircuitView.h"


class CCircuitSimulation;


//A simulation's state: its timeline's times and, for each component of the circuit view in a <Component index="n">
//(n counting the view's components), whatever its view's CComponentStateXML saves of the component it made for the
//view path, and then an <Instance index="n"> holding the same for the path through each instance of a subcircuit in
//the circuit.  A circuit with a type name is named by a circuit attribute.  Wires keep no state; their traces are made
//again from the views.
//
//Loading needs the circuit view's components made in the simulation for the path but not started (CreateSimulation
//with bStart false).  It restores the times, the components' state and their pending events; the simulation then runs
//on from where it was saved.
void	SaveSimulationXML(CXMLWriter* pcWriter, CCircuitView* pcCircuit, CViewPath* pcViewPath, CCircuitSimulation* pcSimulation);
bool	LoadSimulationXML(CMarkupTag* pcSimulationTag, CCircuitView* pcCircuit, CViewPath* pcViewPath, CCircuitSimulation* pcSimulation);


#endif // __SIMULATION_STATE_XML_H__

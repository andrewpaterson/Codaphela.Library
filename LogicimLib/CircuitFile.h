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
#ifndef __CIRCUIT_FILE_H__
#define __CIRCUIT_FILE_H__
#include "CircuitViews.h"


#define CIRCUIT_FILE_VERSION	1


class CCircuitSimulation;
class CFamilyVoltageConfigurationStore;


//A circuit file is XML: a <Logicim version="1"> root holding the circuit view's <Circuit> element and, if one was
//saved, a <Simulation> (see SimulationStateXML.h).  Loading adds the file's circuit to an initialised circuit view,
//normally an empty one, and needs the data memory (DataMemoryInit) for the XML reader.
//
//Loading with a simulation (initialised, empty) makes the circuit in it for the view path, which must end in the
//circuit view: from the saved simulation if the file has one, otherwise started afresh.
//
//A project's circuits are saved the same way with a <Circuits> element (see CCircuitViews) in place of the <Circuit>,
//and the simulation of one of them on its top path, which names the circuit it's of.
bool	SaveCircuitFile(CCircuitView* pcCircuit, const char* szFilename);
bool	SaveCircuitFile(CCircuitView* pcCircuit, CViewPath* pcViewPath, CCircuitSimulation* pcSimulation, const char* szFilename);
bool	LoadCircuitFile(CCircuitView* pcCircuit, const char* szFilename);
bool	LoadCircuitFile(CCircuitView* pcCircuit, CViewPath* pcViewPath, CCircuitSimulation* pcSimulation, CFamilyVoltageConfigurationStore* pcFamilies, const char* szFilename);
void	SaveCircuitXML(CCircuitView* pcCircuit, CXMLWriter* pcWriter);
void	SaveCircuitXML(CCircuitView* pcCircuit, CViewPath* pcViewPath, CCircuitSimulation* pcSimulation, CXMLWriter* pcWriter);
bool	SaveCircuitsFile(CCircuitViews* pcCircuits, CCircuitView* pcTop, CCircuitSimulation* pcSimulation, const char* szFilename);
bool	LoadCircuitsFile(CCircuitViews* pcCircuits, const char* szTop, CCircuitSimulation* pcSimulation, CFamilyVoltageConfigurationStore* pcFamilies, const char* szFilename);
void	SaveCircuitsXML(CCircuitViews* pcCircuits, CCircuitView* pcTop, CCircuitSimulation* pcSimulation, CXMLWriter* pcWriter);


#endif // __CIRCUIT_FILE_H__

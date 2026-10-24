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
#include "CircuitView.h"


#define CIRCUIT_FILE_VERSION	1


//A circuit file is XML: a <Logicim version="1"> root holding the circuit view's <Circuit> element.  Loading adds the
//file's circuit to an initialised circuit view, normally an empty one, and needs the data memory (DataMemoryInit) for
//the XML reader.  The views only; simulations aren't saved yet.
bool	SaveCircuitFile(CCircuitView* pcCircuit, const char* szFilename);
bool	LoadCircuitFile(CCircuitView* pcCircuit, const char* szFilename);
void	SaveCircuitXML(CCircuitView* pcCircuit, CXMLWriter* pcWriter);


#endif // __CIRCUIT_FILE_H__

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
#ifndef __COMPONENT_PLACEMENT_H__
#define __COMPONENT_PLACEMENT_H__
#include "CircuitView.h"
#include "CircuitFonts.h"


//What separates a real 74 series view's element name from its part type in the name it's placed by: "Logic74Package:00"
//places a whole 74LVC00 and "Logic74Unit:00" its first unit.
#define PLACED_PART_SEPARATOR	':'


//A new view, as the editor places it, of the component an element is saved as (see ComponentViewFactory.h), with
//Logicim's initial properties for it (its ViewFactory's createInitialProperties): the LVC family, implicit power,
//two inputs on a gate and so on.  A real 74 series part is placed by its element name and part type (see
//PLACED_PART_SEPARATOR).  Views measured with fonts are measured with the fonts given.  A pin is given a name no other
//pin in the circuit has, as Logicim's ComponentNameHelper, and a chip (a gate, buffer, inverter, logic block, 74 series package
//or unit, flip flop, counter, clock or microprocessor) the next free reference (U1, U2...), as KiCad annotates its
//symbols.  The units placed after a 74 series unit (see NewNextUnit) take its reference, as one part's.  NULL for an
//unknown element name or part type, and for a subcircuit instance, which is placed with
//NewPlacedSubcircuitInstanceView.
CComponentView*	NewPlacedComponentView(const char* szXMLName, int32 x, int32 y, ERotation eRotation, CCircuitView* pcCircuit, CCircuitFonts* pcFonts);
void			GetFreeReference(CChars* psz, CCircuitView* pcCircuit, const char* szPrefix);
CComponentView*	NewPlacedSubcircuitInstanceView(CCircuitView* pcSubcircuit, int32 x, int32 y, ERotation eRotation, CCircuitFonts* pcFonts);
void			GetUniqueComponentName(CChars* pszName, CCircuitView* pcCircuit, const char* szXMLName, const char* szName);
void			SplitComponentName(const char* szName, CChars* pszStem, int* piNumber);


#endif // __COMPONENT_PLACEMENT_H__


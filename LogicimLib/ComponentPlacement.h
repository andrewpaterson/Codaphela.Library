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


//A new view, as the editor places it, of the component an element is saved as (see ComponentViewFactory.h), with
//Logicim's initial properties for it (its ViewFactory's createInitialProperties): the LVC family, implicit power,
//two inputs on a gate and so on.  Views measured with fonts are measured with the fonts given.  A pin is given a name
//no other pin in the circuit has, as Logicim's ComponentNameHelper, and an inverter unit the next free reference (U1,
//U2...) so the units placed after it (see NewNextUnit) are one part's.  NULL for an unknown element name, and for a
//subcircuit instance, which is placed with NewPlacedSubcircuitInstanceView.
CComponentView*	NewPlacedComponentView(const char* szXMLName, int32 x, int32 y, ERotation eRotation, CCircuitView* pcCircuit, CCircuitFonts* pcFonts);
void			GetFreeReference(CChars* psz, CCircuitView* pcCircuit, const char* szPrefix);
CComponentView*	NewPlacedSubcircuitInstanceView(CCircuitView* pcSubcircuit, int32 x, int32 y, ERotation eRotation, CCircuitFonts* pcFonts);
void			GetUniqueComponentName(CChars* pszName, CCircuitView* pcCircuit, const char* szXMLName, const char* szName);
void			SplitComponentName(const char* szName, CChars* pszStem, int* piNumber);


#endif // __COMPONENT_PLACEMENT_H__


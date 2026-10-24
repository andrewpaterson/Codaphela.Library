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
#ifndef __COMPONENT_VIEW_FACTORY_H__
#define __COMPONENT_VIEW_FACTORY_H__
#include "ComponentView.h"


//Allocates (but doesn't initialise) the view a saved element names, NULL for a name it doesn't know.  The view is
//initialised by its LoadXML.
CComponentView*	NewComponentView(const char* szXMLName);
size			NumComponentViewTypes(void);
const char*		GetComponentViewXMLName(size uiIndex);


#endif // __COMPONENT_VIEW_FACTORY_H__

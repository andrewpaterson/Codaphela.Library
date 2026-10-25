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
#ifndef __COMPONENT_LIBRARIES_H__
#define __COMPONENT_LIBRARIES_H__
#include "BaseLib/PrimitiveTypes.h"


//A component the editor can place: the element it's saved as (see NewPlacedComponentView), the name it's shown by,
//what it is and other words it's found by.
struct SComponentLibraryEntry
{
	const char*		szXMLName;
	const char*		szName;
	const char*		szDescription;
	const char*		szKeywords;
};


//The components the editor can place, in libraries, as KiCad's symbol libraries are shown in its symbol chooser.
struct SComponentLibrary
{
	const char*					szName;
	const char*					szDescription;
	SComponentLibraryEntry*		asEntries;
	size						uiNumEntries;
};


size				NumComponentLibraries(void);
SComponentLibrary*	GetComponentLibrary(size uiIndex);
bool				ComponentMatchesFilter(SComponentLibrary* psLibrary, SComponentLibraryEntry* psEntry, const char* szFilter);
bool				TextMatchesFilter(const char* szText, const char* szWord);


#endif // __COMPONENT_LIBRARIES_H__


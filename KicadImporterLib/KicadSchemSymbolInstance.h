/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela KicadImporterLib

Codaphela KicadImporterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela KicadImporterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela KicadImporterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __KICAD_SCHEM_SYMBOL_INSTANCE_H__
#define __KICAD_SCHEM_SYMBOL_INSTANCE_H__
#include "BaseLib/Chars.h"


//One use of a placed symbol: (instances (project "P" (path "/sheet-uuid/..." (reference "U3") (unit 1)))).  A sheet
//used more than once has a path, and so a reference, for each use.
class CKicadSchemSymbolInstance
{
protected:
	CChars	mszProject;
	CChars	mszPath;
	CChars	mszReference;
	int32	miUnit;

public:
	void	Init(char* szProject, char* szPath, char* szReference, int32 iUnit);
	void	Kill(void);

	char*	GetProject(void);
	char*	GetPath(void);
	char*	GetReference(void);
	int32	GetUnit(void);
};


#endif // __KICAD_SCHEM_SYMBOL_INSTANCE_H__


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
#ifndef __KICAD_SCHEM_NO_CONNECT_H__
#define __KICAD_SCHEM_NO_CONNECT_H__
#include "KicadSchemPoint.h"


//(no_connect (at x y) ...) marks the pin at its position as deliberately unconnected.
class CKicadSchemNoConnect
{
protected:
	SKicadSchemPoint	msPosition;

public:
	void				Init(SKicadSchemPoint* psPosition);
	void				Kill(void);

	SKicadSchemPoint*	GetPosition(void);
};


#endif // __KICAD_SCHEM_NO_CONNECT_H__


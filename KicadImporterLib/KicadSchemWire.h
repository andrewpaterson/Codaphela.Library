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
#ifndef __KICAD_SCHEM_WIRE_H__
#define __KICAD_SCHEM_WIRE_H__
#include "BaseLib/Chars.h"
#include "KicadSchemPoint.h"


//A straight wire or bus segment: (wire (pts (xy x1 y1) (xy x2 y2)) ...).  Wires connect where an end touches a pin,
//a label or another wire's end, or where a junction sits on them.  Two wires that only cross are not connected.
class CKicadSchemWire
{
protected:
	SKicadSchemPoint	msStart;
	SKicadSchemPoint	msEnd;
	bool				mbBus;
	CChars				mszUuid;

public:
	void				Init(SKicadSchemPoint* psStart, SKicadSchemPoint* psEnd, bool bBus);
	void				Kill(void);

	void				SetUuid(char* szUuid);

	SKicadSchemPoint*	GetStart(void);
	SKicadSchemPoint*	GetEnd(void);
	bool				IsBus(void);
	char*				GetUuid(void);
	bool				IsEnd(SKicadSchemPoint* psPoint);
	bool				Contains(SKicadSchemPoint* psPoint);
};


#endif // __KICAD_SCHEM_WIRE_H__


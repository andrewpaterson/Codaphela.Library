/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela KicadLib

Codaphela KicadLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela KicadLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela KicadLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __KICAD_SCHEM_SHEET_PIN_H__
#define __KICAD_SCHEM_SHEET_PIN_H__
#include "BaseLib/Chars.h"
#include "KicadSchemPoint.h"
#include "KicadSchemPinType.h"


//A pin on the edge of a sheet symbol: (pin "D0" input (at x y angle) ...).  It connects the wire touching it on
//this sheet to the hierarchical label of the same name inside the sheet.
class CKicadSchemSheetPin
{
protected:
	CChars					mszName;
	EKicadSchemLabelShape	meShape;
	SFloat64Vec2			msPosition;
	float64					mfAngle;

public:
	void					Init(char* szName, EKicadSchemLabelShape eShape);
	void					Kill(void);

	void					SetPosition(SFloat64Vec2* psPosition, float64 fAngle);

	char*					GetName(void);
	EKicadSchemLabelShape	GetShape(void);
	SFloat64Vec2*			GetPosition(void);
	float64					GetAngle(void);
};


#endif // __KICAD_SCHEM_SHEET_PIN_H__


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
#ifndef __KICAD_SCHEM_SHEET_H__
#define __KICAD_SCHEM_SHEET_H__
#include "BaseLib/ListVariable.h"
#include "KicadSchemProperties.h"
#include "KicadSchemSheetPin.h"


//A hierarchical sheet symbol: a box at (at x y) of (size w h) standing for the schematic in its Sheetfile.
class CKicadSchemSheet
{
protected:
	SFloat64Vec2			msPosition;
	SFloat64Vec2			msSize;
	CChars					mszUuid;
	CKicadSchemProperties	mcProperties;
	CListVariable			macPins;

public:
	void					Init(SFloat64Vec2* psPosition, SFloat64Vec2* psSize);
	void					Kill(void);

	void					SetUuid(const char* szUuid);
	CKicadSchemSheetPin*	AddPin(const char* szName, EKicadSchemLabelShape eShape);

	SFloat64Vec2*			GetPosition(void);
	SFloat64Vec2*			GetSize(void);
	const char*				GetUuid(void) const;
	CKicadSchemProperties*	GetProperties(void);
	const char*				GetSheetName(void);
	const char*				GetSheetFile(void);

	size					NumPins(void) const;
	CKicadSchemSheetPin*	GetPin(size uiIndex);
	CKicadSchemSheetPin*	GetPin(char* szName);
};


#endif // __KICAD_SCHEM_SHEET_H__


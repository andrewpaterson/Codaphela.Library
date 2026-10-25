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
#ifndef __KICAD_SCHEM_SHEET_H__
#define __KICAD_SCHEM_SHEET_H__
#include "BaseLib/ListVariable.h"
#include "KicadSchemProperties.h"
#include "KicadSchemSheetPin.h"


//A hierarchical sheet symbol: a box at (at x y) of (size w h) standing for the schematic in its Sheetfile.
class CKicadSchemSheet
{
protected:
	SKicadSchemPoint		msPosition;
	SKicadSchemPoint		msSize;
	CChars					mszUuid;
	CKicadSchemProperties	mcProperties;
	CListVariable			macPins;

public:
	void					Init(SKicadSchemPoint* psPosition, SKicadSchemPoint* psSize);
	void					Kill(void);

	void					SetUuid(char* szUuid);
	CKicadSchemSheetPin*	AddPin(char* szName, EKicadSchemLabelShape eShape);

	SKicadSchemPoint*		GetPosition(void);
	SKicadSchemPoint*		GetSize(void);
	char*					GetUuid(void);
	CKicadSchemProperties*	GetProperties(void);
	char*					GetSheetName(void);
	char*					GetSheetFile(void);

	size					NumPins(void);
	CKicadSchemSheetPin*	GetPin(size uiIndex);
	CKicadSchemSheetPin*	GetPin(char* szName);
};


#endif // __KICAD_SCHEM_SHEET_H__


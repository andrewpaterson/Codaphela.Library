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
#ifndef __KICAD_SCHEM_LIB_SYMBOL_H__
#define __KICAD_SCHEM_LIB_SYMBOL_H__
#include "BaseLib/ListVariable.h"
#include "KicadSchemProperties.h"
#include "KicadSchemLibPin.h"


//A symbol from the file's (lib_symbols ...) cache.  A schematic carries a copy of every library symbol it uses so it
//can be read without the libraries.  Its pins come from the sub-symbols named "<name>_<unit>_<body style>".
class CKicadSchemLibSymbol
{
protected:
	CChars					mszName;
	CChars					mszExtends;
	CKicadSchemProperties	mcProperties;
	CListVariable			macPins;
	bool					mbPower;
	bool					mbPinNamesHidden;
	bool					mbPinNumbersHidden;
	int32					miNumUnits;

public:
	void					Init(char* szName);
	void					Kill(void);

	CKicadSchemLibPin*		AddPin(EKicadSchemPinType eType, char* szGraphicStyle, int32 iUnit, int32 iBodyStyle);
	void					SetExtends(char* szExtends);
	void					SetPower(bool bPower);
	void					SetPinNamesHidden(bool bHidden);
	void					SetPinNumbersHidden(bool bHidden);

	char*					GetName(void);
	char*					GetExtends(void);
	CKicadSchemProperties*	GetProperties(void);
	bool					IsPower(void);
	bool					ArePinNamesHidden(void);
	bool					ArePinNumbersHidden(void);
	int32					NumUnits(void);

	size					NumPins(void);
	CKicadSchemLibPin*		GetPin(size uiIndex);
	CKicadSchemLibPin*		GetPin(char* szNumber, int32 iUnit, int32 iBodyStyle);
};


#endif // __KICAD_SCHEM_LIB_SYMBOL_H__


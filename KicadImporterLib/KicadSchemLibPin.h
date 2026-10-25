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
#ifndef __KICAD_SCHEM_LIB_PIN_H__
#define __KICAD_SCHEM_LIB_PIN_H__
#include "BaseLib/Chars.h"
#include "KicadSchemPoint.h"
#include "KicadSchemPinType.h"


//A pin of a library symbol.  Its position is the end that wires connect to, in library coordinates (Y up), and its
//angle is the direction from there into the symbol's body.  Unit 0 means the pin is on every unit and body style 0
//that it's on every body style (the "_0_" and "_x_0" sub-symbols).
class CKicadSchemLibPin
{
protected:
	EKicadSchemPinType	meType;
	CChars				mszGraphicStyle;
	CChars				mszName;
	CChars				mszNumber;
	SKicadSchemPoint	msPosition;
	float64				mfAngle;
	float64				mfLength;
	bool				mbHidden;
	int32				miUnit;
	int32				miBodyStyle;

public:
	void				Init(EKicadSchemPinType eType, char* szGraphicStyle, int32 iUnit, int32 iBodyStyle);
	void				Kill(void);

	void				SetName(char* szName);
	void				SetNumber(char* szNumber);
	void				SetPosition(SKicadSchemPoint* psPosition, float64 fAngle);
	void				SetLength(float64 fLength);
	void				SetHidden(bool bHidden);

	EKicadSchemPinType	GetType(void);
	char*				GetGraphicStyle(void);
	char*				GetName(void);
	char*				GetNumber(void);
	SKicadSchemPoint*	GetPosition(void);
	float64				GetAngle(void);
	float64				GetLength(void);
	bool				IsHidden(void);
	int32				GetUnit(void);
	int32				GetBodyStyle(void);
	bool				IsOn(int32 iUnit, int32 iBodyStyle);
};


#endif // __KICAD_SCHEM_LIB_PIN_H__


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
#ifndef __KICAD_SCHEM_PROPERTY_H__
#define __KICAD_SCHEM_PROPERTY_H__
#include "BaseLib/Chars.h"
#include "KicadSchemPoint.h"


//A named field of a symbol or sheet: (property "Reference" "U1" (at x y angle) ...).
class CKicadSchemProperty
{
protected:
	CChars				mszName;
	CChars				mszValue;
	SKicadSchemPoint	msPosition;
	float64				mfAngle;
	bool				mbHidden;

public:
	void				Init(char* szName, char* szValue);
	void				Kill(void);

	void				SetPosition(SKicadSchemPoint* psPosition, float64 fAngle);
	void				SetHidden(bool bHidden);

	char*				GetName(void);
	char*				GetValue(void);
	SKicadSchemPoint*	GetPosition(void);
	float64				GetAngle(void);
	bool				IsHidden(void);
};


#endif // __KICAD_SCHEM_PROPERTY_H__


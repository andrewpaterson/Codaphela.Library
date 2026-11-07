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
#ifndef __KICAD_SCHEM_LABEL_H__
#define __KICAD_SCHEM_LABEL_H__
#include "BaseLib/Chars.h"
#include "KicadSchemPoint.h"
#include "KicadSchemPinType.h"


enum EKicadSchemLabelType
{
	KSLT_Local,			//(label "CLK" ...) names a net on this sheet.
	KSLT_Global,		//(global_label "CLK" (shape input) ...) names a net on every sheet.
	KSLT_Hierarchical	//(hierarchical_label "CLK" (shape input) ...) connects to the sheet pin of the same name.
};


//A net label.  It names the wire whose end, or any point of which, is at its position.
class CKicadSchemLabel
{
protected:
	EKicadSchemLabelType	meType;
	CChars					mszText;
	SFloat64Vec2			msPosition;
	float64					mfAngle;
	EKicadSchemLabelShape	meShape;

public:
	void					Init(EKicadSchemLabelType eType, char* szText);
	void					Kill(void);

	void					SetPosition(SFloat64Vec2* psPosition, float64 fAngle);
	void					SetShape(EKicadSchemLabelShape eShape);

	EKicadSchemLabelType	GetType(void);
	char*					GetText(void);
	SFloat64Vec2*			GetPosition(void);
	float64					GetAngle(void);
	EKicadSchemLabelShape	GetShape(void);
};


#endif // __KICAD_SCHEM_LABEL_H__


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
#ifndef __KICAD_SCHEM_TRANSFORM_H__
#define __KICAD_SCHEM_TRANSFORM_H__
#include "KicadSchemPoint.h"


enum EKicadSchemMirror
{
	KSM_None,
	KSM_X,		//(mirror x) flips the symbol top to bottom.
	KSM_Y		//(mirror y) flips the symbol left to right.
};


//The matrix eeschema builds from a placed symbol's (at x y angle) and (mirror): it maps a point in the library symbol,
//where Y is up, to an offset from the symbol's position on the sheet, where Y is down.  The angle (0, 90, 180 or 270)
//turns the symbol anticlockwise as it's seen on the sheet and the mirror is applied after it.
class CKicadSchemTransform
{
protected:
	int32	miX1;
	int32	miY1;
	int32	miX2;
	int32	miY2;

public:
	bool	Init(float64 fAngle, EKicadSchemMirror eMirror);

	void	Transform(SKicadSchemPoint* psDest, SKicadSchemPoint* psLibrary);

protected:
	void	Multiply(int32 iX1, int32 iY1, int32 iX2, int32 iY2);
};


#endif // __KICAD_SCHEM_TRANSFORM_H__


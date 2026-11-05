/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __DIFF_VIEW_COLOURS_H__
#define __DIFF_VIEW_COLOURS_H__
#include "StandardLib/SimpleTextDiff.h"
#include "SupportLib/ColourARGB32.h"


//The colours a diff view shows changes in, by the kind of block: a band behind a block's lines (and line numbers), a
//stronger shade over the characters that changed in it (and as the line where an empty side's lines would be), and the
//divider's shape joining the two sides, filled and outlined.  Init gives IntelliJ IDEA's dark theme's.
struct SDiffViewColours
{
	ARGB32	asLines[3];		//By ETextDiffBlockType: inserted, deleted, modified.
	ARGB32	asChanges[3];
	ARGB32	asShapes[3];
	ARGB32	asOutlines[3];
	ARGB32	sDivider;		//Behind the shapes.

	void	Init(void);
	ARGB32	GetLines(ETextDiffBlockType eType);
	ARGB32	GetChanges(ETextDiffBlockType eType);
	ARGB32	GetShape(ETextDiffBlockType eType);
	ARGB32	GetOutline(ETextDiffBlockType eType);
};


#endif // __DIFF_VIEW_COLOURS_H__


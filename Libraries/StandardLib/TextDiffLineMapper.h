/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __TEXT_DIFF_LINE_MAPPER_H__
#define __TEXT_DIFF_LINE_MAPPER_H__
#include "SimpleTextDiff.h"


enum ETextDiffSide
{
	TDS_Left,	//Content 1, the old text.
	TDS_Right,	//Content 2, the new text.
};


//Maps a line of one side of a diff to the line it lines up with on the other, as IntelliJ IDEA's side-by-side viewer
//does to scroll one side with the other.  Lines are fractional (a scroll position in rows).  Before, between and after
//the blocks unchanged lines map one to one, shifted by the lines the blocks before them added or removed.  Inside a
//block a line maps proportionally across it; a block with no lines on the side mapped from is a point there, and the
//lines it has on the other side are skipped over.
class CTextDiffLineMapper
{
protected:
	CSimpleTextDiff*	mpcDiff;

public:
	void	Init(CSimpleTextDiff* pcDiff);
	void	Kill(void);

	float64	TransferLine(ETextDiffSide eFrom, float64 fLine);
};


#endif // __TEXT_DIFF_LINE_MAPPER_H__


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
#ifndef __TEXT_DIFF_PRODUCER_H__
#define __TEXT_DIFF_PRODUCER_H__
#include "SimpleTextDiff.h"


//Produces CSimpleTextDiffs (IntelliJ's ChangeDiffRequestProducer and ComparisonManager together).  Whatever the two
//sides come from (strings, files, later git revisions) the diff is always computed here, text to text, under the
//producer's whitespace and highlight policies.  Produce recomputes a diff whose contents are already set, so a view can
//change policy without reloading.
class CTextDiffProducer
{
protected:
	ETextDiffWhitespace	meWhitespace;
	ETextDiffHighlight	meHighlight;

public:
	void	Init(void);
	void	Init(ETextDiffWhitespace eWhitespace, ETextDiffHighlight eHighlight);
	void	Kill(void);

	void	SetWhitespace(ETextDiffWhitespace eWhitespace);
	void	SetHighlight(ETextDiffHighlight eHighlight);

	void	Produce(CSimpleTextDiff* pcDiff);
	void	ProduceFromText(CSimpleTextDiff* pcDiff, const char* szTitle1, const char* szText1, const char* szTitle2, const char* szText2);
	bool	ProduceFromFiles(CSimpleTextDiff* pcDiff, const char* szFilename1, const char* szFilename2);
};


#endif // __TEXT_DIFF_PRODUCER_H__

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
#ifndef __TEXT_LINE_GAPS_H__
#define __TEXT_LINE_GAPS_H__
#include "BaseLib/TextDocument.h"
#include "SupportLib/TextLayout.h"


//The empty gaps below lines of a text (lines ended by line feeds, numbered from 0), as CTextArea::SetLineGap leaves
//them for what owns the text to draw in: sorted by line, at most one a line.  Each gap has an id, from 1, that it keeps
//while it is there, through changes of height too, for finding its line after edits.  The gaps follow their lines as
//the text is edited (DocumentChanged).  Setting and clearing gaps say whether anything changed, so the text is laid out
//again only then.
class CTextLineGaps : public CTextDocumentListener
{
protected:
	CArrayTextLayoutGap	masGaps;
	uint32				muiNextId;  //The id the next new gap gets.

public:
	void					Init(void);
	void					Kill(void);

	bool					Set(size uiLine, int32 iHeight);
	bool					Copy(CArrayTextLayoutGap* pasGaps, size uiNumLines);
	bool					Clear(void);

	int32					GetHeight(size uiLine);
	uint32					GetId(size uiLine);
	size					FindLine(uint32 uiId);
	size					NumGaps(void);
	CArrayTextLayoutGap*	GetGaps(void);
	void					GetGapsFrom(size uiFirstLine, CArrayTextLayoutGap* pasDest);

	void					DocumentChanged(STextDocumentChange* psChange) override;
	void					LinesInserted(size uiLine, bool bLineStart, size uiNewLines);
	void					LinesRemoved(size uiFirst, size uiLast, bool bWhole);

protected:
	size					Find(size uiLine);
	STextLayoutGap*			Get(size uiLine);
};


#endif // __TEXT_LINE_GAPS_H__


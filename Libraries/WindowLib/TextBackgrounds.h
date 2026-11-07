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
#ifndef __TEXT_BACKGROUNDS_H__
#define __TEXT_BACKGROUNDS_H__
#include "BaseLib/ArrayTemplate.h"
#include "SupportLib/ColourARGB32.h"


enum ETextAreaBackground
{
	TAB_Range,  //Bytes [uiStart, uiEnd) of the UTF8 text, on each row as the selection is.
	TAB_Lines,  //Lines [uiStart, uiEnd), across the text area's whole width.
	TAB_Boundary  //A line iThickness high across the whole width, centred on the top of line uiStart (or the bottom of the last line, for NumLines), kept inside the text.
};


//A colour drawn under the text (see AddBackground, AddLineBackground and AddLineBoundary).
struct STextAreaBackground
{
	ETextAreaBackground	eType;
	size				uiStart;
	size				uiEnd;
	int32				iThickness;
	ARGB32				sColour;
};


typedef CArrayTemplate<STextAreaBackground>	CArrayTextAreaBackground;


//The colours drawn under a text (CTextArea::AddBackground, AddLineBackground and AddLineBoundary), in the order they
//were added: ranges of its bytes, runs of its lines and boundaries between lines.  They follow the text as it is edited:
//ranges move with it and grow with text put strictly inside them, and lines and boundaries stay with their lines.  The
//changes say whether any background changed, so they are drawn again only then.
class CTextBackgrounds
{
protected:
	CArrayTextAreaBackground	masBackgrounds;

public:
	void					Init(void);
	void					Kill(void);

	void					Add(ETextAreaBackground eType, size uiStart, size uiEnd, int32 iThickness, ARGB32 sColour);
	bool					Clear(void);
	size					NumBackgrounds(void);
	STextAreaBackground*	Get(size uiIndex);

	bool					TextInserted(size uiPosition, size uiLength);
	bool					TextRemoved(size uiStart, size uiEnd);
	bool					LinesInserted(size uiLine, bool bLineStart, size uiNewLines);
	bool					LinesRemoved(size uiFirst, size uiLast, bool bWhole);
};


#endif // __TEXT_BACKGROUNDS_H__


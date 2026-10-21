#ifndef __TEXT_AREA_H__
#define __TEXT_AREA_H__
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
#include "SupportLib/Font.h"
#include "SupportLib/Text.h"
#include "SupportLib/TextLayout.h"
#include "SupportLib/SpriteMap.h"
#include "SupportLib/MapFontDraw.h"
#include "SupportLib/Maps.h"
#include "Canvas.h"


//Lines of text drawn in one font with a CMapsCanvasDraw, for display only (there is no caret or editing yet).  Line
//feeds start new lines and, with word wrap on (it is off by default), lines are also broken at spaces to fit the width
//the text area is laid out to.  A word too wide for a line by itself is broken between glyphs.  Like a label the text
//area is a transparent canvas with its text at the top left; text that doesn't fit is clipped.  A word wrapped text
//area is variable sized horizontally: it takes the width its container gives it.  Without word wrap its lines can't be
//made narrower so its required width is the width of the laid out text.  Its required height is the height of the
//laid out lines, wrapped or not.
class CTextArea : public CCanvas
{
CONSTRUCTABLE(CTextArea);
DESTRUCTABLE(CTextArea);
protected:
	CChars				mszText;
	Ptr<CFont>			mpFont;
	Ptr<CText>			mpText;
	Ptr<CTextLayout>	mpLayout;
	Ptr<CSpriteMap>		mpSpriteMap;
	Ptr<CMapFontDraw>	mpFontDraw;
	Ptr<CMaps>			mpMaps;
	bool				mbWordWrap;
	SInt32Vec2			msTextSize;

public:
	void			Init(Ptr<CWindow> pWindow, Ptr<CFont> pFont, const char* szText);
	void			Class(void) override;
	void 			Free(void) override;

	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	void			SetText(const char* szText);
	char*			GetText(void);
	void			SetFont(Ptr<CFont> pFont);
	Ptr<CFont>		GetFont(void);
	void			SetWordWrap(bool bWordWrap);
	bool			IsWordWrap(void);

	SInt32Vec2		GetTextSize(void);
	SInt32Vec2		GetRequiredSize(void);
	SInt32Vec2		GetDesiredSize(void) override;

	void			Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;

protected:
	void			ReadText(void);
	void			LayoutText(void);
	int32			GetWrapWidth(void);
};


#endif // __TEXT_AREA_H__


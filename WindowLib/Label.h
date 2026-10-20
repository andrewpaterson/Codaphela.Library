#ifndef __LABEL_H__
#define __LABEL_H__
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


//A single line of text drawn in one font with a CMapsCanvasDraw.  Its required size is the size of the laid out text
//and the line height of the font.  The text is always drawn at the top left of the label; a label is aligned by the
//container it is in, so put it in a CFillContainer to place it with its alignment.
class CLabel : public CCanvas
{
CONSTRUCTABLE(CLabel);
DESTRUCTABLE(CLabel);
protected:
	CChars				mszText;
	Ptr<CFont>			mpFont;
	Ptr<CText>			mpText;
	Ptr<CTextLayout>	mpLayout;
	Ptr<CSpriteMap>		mpSpriteMap;
	Ptr<CMapFontDraw>	mpFontDraw;
	Ptr<CMaps>			mpMaps;
	SInt32Vec2			msRequiredSize;

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

	SInt32Vec2		GetRequiredSize(void);
	SInt32Vec2		GetDesiredSize(void) override;

protected:
	void			LayoutText(void);
};


#endif // __LABEL_H__

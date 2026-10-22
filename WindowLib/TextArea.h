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
#include "SupportLib/FontColours.h"
#include "SupportLib/FontFamily.h"
#include "SupportLib/TextStyle.h"
#include "SupportLib/Text.h"
#include "SupportLib/TextLayout.h"
#include "SupportLib/SimpleTextReader.h"
#include "SupportLib/SpriteMap.h"
#include "SupportLib/GeometryMap.h"
#include "SupportLib/MapFontDraw.h"
#include "SupportLib/Maps.h"
#include "Canvas.h"


//A range of the text in a style.  uiStart and uiLength are in bytes of the UTF8 text.
struct STextAreaSpan
{
	size				uiStart;
	size				uiLength;
	STextStyleOverride	sOverride;
};


typedef CArrayTemplate<STextAreaSpan>	CArrayTextAreaSpan;


//Lines of text drawn with a CMapsCanvasDraw, for display only (there is no caret or editing yet).  Line feeds start
//new lines and, with word wrap on (it is off by default), lines are also broken at spaces to fit the width the text
//area is laid out to.  A word too wide for a line by itself is broken between glyphs.  Like a label the text area is
//a transparent canvas with its text at the top left; text that doesn't fit is clipped.  A word wrapped text area is
//variable sized horizontally: it takes the width its container gives it.  Without word wrap its lines can't be made
//narrower so its required width is the width of the laid out text.  Its required height is the height of the laid
//out lines, wrapped or not.
//
//The text is drawn in the faces, sizes and colours of a font family (see CFontFamily), and can be underlined, overlined
//and struck through in its colour.  Text is in the text area's
//style (regular, the family's default size, opaque white unless set) except where spans override parts of it; where
//spans overlap each property takes the value of the last span added that sets it.  Spans are applied when the text
//area is next laid out, drawn or asked its size.
class CTextArea : public CCanvas
{
CONSTRUCTABLE(CTextArea);
DESTRUCTABLE(CTextArea);
protected:
	CChars				mszText;
	Ptr<CFontFamily>	mpFamily;
	STextStyle			msStyle;
	CArrayTextAreaSpan	masSpans;
	bool				mbTextDirty;  //The spans or style changed since the text was last read.
	bool				mbSizeDirty;  //Reading the text again can change its size.
	Ptr<CText>			mpText;
	Ptr<CTextLayout>	mpLayout;
	Ptr<CSpriteMap>		mpSpriteMap;
	Ptr<CGeometryMap>	mpGeometryMap;  //The lines drawn along the text, over it.
	Ptr<CMapFontDraw>	mpFontDraw;
	Ptr<CMaps>			mpMaps;
	bool				mbWordWrap;
	SInt32Vec2			msTextSize;

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CFont> pWhiteFont, const char* szText);
	void				Init(Ptr<CWindow> pWindow, Ptr<CFontColours> pColours, const char* szText);
	void				Init(Ptr<CWindow> pWindow, Ptr<CFontFamily> pFamily, const char* szText);
	void				Class(void) override;
	void 				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				SetText(const char* szText);
	void				AppendText(const char* szText, ARGB32 sColour);
	char*				GetText(void);
	void				SetFont(Ptr<CFont> pWhiteFont);
	Ptr<CFont>			GetFont(void);
	void				SetFontColours(Ptr<CFontColours> pColours);
	Ptr<CFontColours>	GetFontColours(void);
	void				SetFontFamily(Ptr<CFontFamily> pFamily);
	Ptr<CFontFamily>	GetFontFamily(void);
	void				SetWordWrap(bool bWordWrap);
	bool				IsWordWrap(void);

	void				SetStyle(STextStyle* psStyle);
	void				GetStyle(STextStyle* psStyle);
	void				SetColour(ARGB32 sColour);
	ARGB32				GetColour(void);
	bool				AddSpan(size uiStart, size uiLength, ARGB32 sColour);
	bool				AddSpan(size uiStart, size uiLength, STextStyleOverride* psOverride);
	void				ClearSpans(void);
	size				NumSpans(void);
	void				GetStyleAt(size uiPosition, STextStyle* psStyle);
	ARGB32				GetColourAt(size uiPosition);

	SInt32Vec2			GetTextSize(void);
	SInt32Vec2			GetRequiredSize(void);
	SInt32Vec2			GetDesiredSize(void) override;

	void				Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	bool				Draw(void) override;

protected:
	void				ReadText(void);
	void				MakeFontChanges(CArrayTextFontChange* pasChanges, Ptr<CFont> pDefaultFont, int16 iDefaultBaselineShift);
	void				LayoutText(void);
	void				UpdateText(void);
	void				UpdateSize(void);
	int32				GetWrapWidth(void);
};


#endif // __TEXT_AREA_H__


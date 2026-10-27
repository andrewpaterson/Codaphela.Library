#ifndef __TEXT_LAYOUT_H__
#define __TEXT_LAYOUT_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "BaseLib/UTF16.h"
#include "BaseLib/ArrayTemplate.h"
#include "StandardLib/Object.h"
#include "BaseFontDraw.h"
#include "Glyph.h"
#include "Text.h"


struct STextLayoutMark
{
	CTextRun*	pcRun;
	size		uiElementIndex;
	size		uiCharIndex;
};


//A glyph of the word being laid out, held until the end of the word is found so the whole word can be moved to the
//next line if it doesn't fit.
//The font, baseline shift and decorations of the run a glyph is from.
struct STextLayoutRunStyle
{
	CFont*		pcFont;
	int16		iBaselineShift;
	uint8		uiDecorations;		//TD_ flags.
	ARGB32		sDecorationColour;
};


struct STextLayoutGlyph
{
	CGlyph*					pcGlyph;
	int16					iAscent;   //Of the font the glyph is from, moved by its baseline shift.
	int16					iDescent;
	STextLayoutRunStyle		sStyle;
};


//A glyph placed on the line being laid out, held until the end of the line when the line's baseline is known.
struct STextLayoutLineGlyph
{
	CGlyph*					pcGlyph;
	int32					x;
	int16					iAscent;  //Of the font the glyph is from, moved by its baseline shift.
	bool					bSpace;
	STextLayoutRunStyle		sStyle;
};


//Where a character of the text was laid out, so a caret can be put before it.  Every character read from the text has
//one, in order, even those the font has no glyph for (they have no width) and line feeds (at the end of their line).
struct STextLayoutStop
{
	int32		x;
	int32		iWidth;
	size		uiLine;
};


//A line as laid out: lines end at line feeds and, when wrapping, where a word didn't fit.
struct STextLayoutLine
{
	int32		iTop;
	int32		iHeight;
	int32		iBaseline;  //Below the top of the text, not the line.
	size		uiFirstStop;
	size		uiNumStops;
};


typedef CArrayTemplate<STextLayoutGlyph>		CArrayTextLayoutGlyph;
typedef CArrayTemplate<STextLayoutLineGlyph>	CArrayTextLayoutLineGlyph;
typedef CArrayTemplate<STextLayoutStop>			CArrayTextLayoutStop;
typedef CArrayTemplate<STextLayoutLine>			CArrayTextLayoutLine;


class CTextLayout : public CObject
{
CONSTRUCTABLE(CTextLayout);
DESTRUCTABLE(CTextLayout);
protected:
	int32			miWidth;		//-1 for no max width, otherwise lines are wrapped at spaces to fit in it.
	int16			miRowAscent;	//Of the glyphs on the line being laid out.
	int16			miRowDescent;
	int16			miFontAscent;	//Of the font of the run being laid out, moved by the run's baseline shift.
	int16			miFontDescent;
	SInt32Vec2		msPosition;
	SInt32Vec2		msSize;
	CUTF16			mcUTF16;
	CBaseFontDraw*	mpcDraw;
	CArrayTextLayoutGlyph	masWord;
	CArrayTextLayoutLineGlyph	masLine;
	int32			miWordWidth;
	STextLayoutRunStyle	msRunStyle;  //Of the run being laid out.
	CArrayTextLayoutStop	masStops;  //Of the text last laid out, and one more after its last character.
	CArrayTextLayoutLine	masLines;
	size			muiLineFirstStop;  //Of the line being laid out.

public:
	void			Init(void);
	void			Init(int32 iWidth);
	void			Class(void) override;
	void 			Free(void) override;

	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	void			Layout(Ptr<CBaseFontDraw> pDraw, Ptr<CText> pText);
	SInt32Vec2		GetSize(void);
	void			SetWidth(int32 iWidth);
	int32			GetWidth(void);

	size			NumStops(void);
	STextLayoutStop*	GetStop(size uiIndex);
	size			NumLines(void);
	STextLayoutLine*	GetLine(size uiIndex);

protected:
	void			Glyph(Ptr<CGlyph> pGlyph, bool bSpace);
	void			PlaceGlyph(Ptr<CGlyph> pGlyph, int16 iAscent, int16 iDescent, bool bSize, bool bSpace, STextLayoutRunStyle* psStyle);
	void			AddStop(int32 iWidth);
	void			EndLine(void);
	void			DecorateLine(int32 iBaseline);
	void			DecorateStretch(uint8 uiDecoration, int32 iBaseline, size uiFirst, size uiEnd);
	void			EndWord(void);
	void			NewLine(void);

	void			Layout(CFont* pcFont, CTextUTF16Short* pcUTFShort);
	void			Layout(CFont* pcFont, CTextUTF16Long* pcUTFLong);
	void			Layout(CFont* pcFont, CTextUTF16Multi* pcUTFMulti);
};


#endif // __TEXT_LAYOUT_H__


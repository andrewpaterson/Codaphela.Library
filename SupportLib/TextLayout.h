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
#include "StandardLib/Object.h"
#include "BaseFontDraw.h"
#include "Text.h"


struct STextLayoutMark
{
	CTextRun*	pcRun;
	size		uiElementIndex;
	size		uiCharIndex;
};


class CTextLayout : public CObject
{
CONSTRUCTABLE(CTextLayout);
DESTRUCTABLE(CTextLayout);
protected:
	int32			miWidth;		//-1 for no max width;
	size			muiRowHeight;
	SInt32Vec2			msPosition;
	CUTF16			mcUTF16;
	CBaseFontDraw*	mpcDraw;

public:
	void			Init(void);
	void			Init(int32 iWidth);
	void			Class(void);
	void 			Free(void);

	bool			Save(CObjectWriter* pcFile);
	bool			Load(CObjectReader* pcFile);

	void			Layout(Ptr<CBaseFontDraw> pDraw, Ptr<CText> pText);

protected:
	void			Glyph(Ptr<CGlyph> pGlyph);
	void			NewLine(void);

	void			Layout(CFont* pcFont, CTextUTF16Short* pcUTFShort);
	void			Layout(CFont* pcFont, CTextUTF16Long* pcUTFLong);
	void			Layout(CFont* pcFont, CTextUTF16Multi* pcUTFMulti);
};


#endif // __TEXT_LAYOUT_H__


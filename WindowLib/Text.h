/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __TEXT_H__
#define __TEXT_H__
#include "BaseLib/Chars.h"
#include "TextParameters.h"
#include "Drawable.h"
#include "Caret.h"
#include "TextData.h"


class CText : public CDrawable
{
public:
	BASE_FUNCTIONS(CText);

	float		mfAlpha;
	CTextData	mcTextData;
	BOOL		mbCaretVisible;
	CCaret*		mpcCaret;
	SInt2		msCaretPos;

	void 		Init(CViewport* pcViewport);
	void 		Kill(void);

	BOOL 		Draw(void);
	void 		SetRequiredSize(void);
	void		Layout(SInt2 sPosition, SInt2 sAreaSize);

	void 		SetText(char* sz);
	void 		SetText(CChars sz);
	void 		AdjustColour(void);
	void 		SetAlpha(float fAlpha);
};


#endif // __TEXT_H__


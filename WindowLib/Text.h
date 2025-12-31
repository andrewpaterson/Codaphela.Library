#ifndef __TEXT_H__
#define __TEXT_H__
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
#include "BaseLib/Chars.h"
#include "BaseLib/Int2.h"
#include "StandardLib/Pointer.h"
#include "ComplexComponent.h"
#include "Caret.h"
#include "TextData.h"


class CText : public CComplexComponent
{
CONSTRUCTABLE(CText);
DESTRUCTABLE(CText);
public:
	float			mfAlpha;
	CTextData		mcTextData;
	bool			mbCaretVisible;
	Ptr<CCaret>		mpCaret;
	SInt2			msCaretPos;

public:
	Ptr<CText> 	Init(Ptr<CWindow> pWindow);
	void		Class(void) override;
	void 		Free(void) override;

	bool		Save(CObjectWriter* pcFile) override;
	bool		Load(CObjectReader* pcFile) override;

	bool		Draw(void) override;
	void 		SetRequiredSize(void);
	void		Layout(SInt2 sPosition, SInt2 sAreaSize) override;

	void 		SetText(char* sz, CTextParameters* pcFont);
	void 		SetText(CChars sz, CTextParameters* pcFont);
	void 		SetAlpha(float fAlpha);
};


#endif // __TEXT_H__


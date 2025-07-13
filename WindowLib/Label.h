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
#ifndef __LABEL_H__
#define __LABEL_H__
#include "FillContainer.h"
#include "Text.h"
#include "Widget.h"


class CLabel : public CComponent
{
public:
	CText*				mpcText;
	CWidget*			mpcWidget;
	CFillContainer*		mpcFill;

	void 	Init(CViewport* pcViewport, char* szString, bool bDefaultBorder = true, bool bDefaultBackground = true);
	void 	Kill(void);

	bool	Draw(void);
	void	Layout(SInt2 sPosition, SInt2 sAreaSize);

	void 	SetText(char* szText);
	void 	SetText(CChars cString);
	void 	SetAlpha(float fAlpha);
	void 	SetFillStyle(EFillStyleHorizontal eH, EFillStyleVertical eV);
};


#endif // __LABEL_H__


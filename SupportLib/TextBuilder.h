#ifndef __TEXT_BUILDER_H__
#define __TEXT_BUILDER_H__
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
#include "BaseLib/UnicodeReader.h"
#include "Font.h"
#include "Text.h"
#include "TextBuilderUTFCommon.h"
#include "TextBuilderUTF16Long.h"
#include "TextBuilderUTF16Short.h"


class CTextBuilder
{
protected:
	CUnicodeReader*			mpcUTF;
	CText*					mpcText;
	CFont*					mpcFont;
	CTextBuilderUTFCommon*	mpcCommon;
	CTextBuilderUTF16Short	mcShort;
	CTextBuilderUTF16Long	mcLong;
	CTextRun*				mpcRun;

public:
	void		Init(CUnicodeReader* pcUTF, CText* pcText, CFont* pcFont);
	void		Kill(void);

	bool		Push(uint32 cCodePoint32);
	bool		Push(uint8* puiCodePoint, size uiCodePointsLength);
	bool		PushNewLine(void);

	bool		PushCurrent(void);
	bool		Done(void);
	
	bool		EnsureRun(void);
};


#endif // __TEXT_BUILDER_H__


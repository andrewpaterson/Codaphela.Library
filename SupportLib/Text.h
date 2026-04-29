#ifndef __TEXT_H__
#define __TEXT_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MeshLib

Codaphela MeshLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MeshLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MeshLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Mallocator.h"
#include "StandardLib/Object.h"
#include "TextRun.h"


enum ETextRunRelativeDirection
{
	TRRD_Right,
	TRRD_Down,
};


class CTextUTF16Short;
class CTextUTF16Long;
class CTextUTF16Multi;
class CTextNewLine;
class CText : public CObject
{
CONSTRUCTABLE(CText)
DESTRUCTABLE(CText)
protected:
	CArrayTextRun				macText;
	ETextRunRelativeDirection	meRunDirection;
	CMallocator*				mpcMalloc;

public:
	void					Init(ETextRunRelativeDirection eRunDirection = TRRD_Right, CMallocator* pcMalloc = NULL);
	void 					Free(void);
	void					Class(void);

	bool					Load(CObjectReader* pcFile);
	bool					Save(CObjectWriter* pcFile);

	void					AddUTF16(Ptr<CFont> pFont, uint16* szText, size uiLength);

	CMallocator*			GetMalloc(void);

	CTextRun*				AllocateRun(CFont* pcFont);

	CTextUTF16Short*		AllocateUTF16Short(size uiNumShorts, uint16* puiData);
	CTextUTF16Long*			AllocateUTF16Long(size uiNumLongs, uint32* puiData);
	CTextUTF16Multi*		AllocateUTF16Multi(size uiByteSize, void* pvData);

	CTextNewLine*			AllocateNewLine(void);

	size					NumRuns(void);
	CTextRun*				GetRun(size uiIndex);
};


#endif // __TEXT_H__


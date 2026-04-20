#ifndef __TEXT_RUN_UTF16_MULTI_H__
#define __TEXT_RUN_UTF16_MULTI_H__
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
#include "TextRunCommon.h"


//Multi is a single, multi-code point glyph.  i.e. it's a number of codepoints connected with ZWJs (zero width joiner).
class CTextRunUTF16Multi : public CTextRunCommon
{
CONSTRUCTABLE(CTextRunUTF16Multi);
protected:
public:
	void		Init(size uiNumShorts);
	uint16*		GetChar(void);

	bool		Load(CObjectReader* pcFile) override;
	bool		Save(CObjectWriter* pcFile) override;
};


#endif // __TEXT_RUN_UTF16_MULTI_H__


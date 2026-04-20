#ifndef __TEXT_RUN_UTF16_SHORT_H__
#define __TEXT_RUN_UTF16_SHORT_H__
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


class CTextRunUTF16Short : public CTextRunCommon
{
CONSTRUCTABLE(CTextRunUTF16Short);
protected:
public:
	void		Init(size uiNumChars);
	uint16*		GetChars(void);

	bool		Load(CObjectReader* pcFile) override;
	bool		Save(CObjectWriter* pcFile) override;
};


#endif // __TEXT_RUN_UTF16_SHORT_H__


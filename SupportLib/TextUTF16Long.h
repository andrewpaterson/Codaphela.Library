#ifndef __TEXT_RUN_UTF16_LONG_H__
#define __TEXT_RUN_UTF16_LONG_H__
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
#include "TextDrawable.h"


class CTextUTF16Long : public CTextDrawable
{
CONSTRUCTABLE(CTextUTF16Long);
protected:
public:
	void		Init(size uiNumChars);
	uint32*		GetChars(void);  //This can include UTF16 encoded two byte characters padded to 4 bytes if that uses less memory.
	void		Copy(uint32* puiData);

	bool		Load(CObjectReader* pcFile) override;
	bool		Save(CObjectWriter* pcFile) override;

	bool		IsUTF16Long(void) override;
};


#endif // __TEXT_RUN_UTF16_LONG_H__


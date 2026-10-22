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

** ------------------------------------------------------------------------ **/
#ifndef __TRUE_TYPE_FACE_H__
#define __TRUE_TYPE_FACE_H__
#include "BaseLib/Chars.h"
#include "StandardLib/Object.h"
#include "TrueTypeFont.h"


//A loaded TrueType font file, shared by everything rasterised from it so that it lives as long as they need it.
class CTrueTypeFace : public CObject
{
CONSTRUCTABLE(CTrueTypeFace);
DESTRUCTABLE(CTrueTypeFace);
protected:
	CChars			mszFilename;
	CTrueTypeFont	mcFont;
	bool			mbLoaded;

public:
	void			Init(void);
	void			Class(void) override;
	void 			Free(void) override;

	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	bool			Load(const char* szFilename);
	bool			IsLoaded(void);
	CTrueTypeFont*	GetFont(void);
	char*			GetFilename(void);
};


#endif // __TRUE_TYPE_FACE_H__


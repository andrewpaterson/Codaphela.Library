#ifndef __BASE_FONT_DRAW_H__
#define __BASE_FONT_DRAW_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "SpriteMap.h"
#include "Text.h"


class CBaseFontDraw : public CObject
{
CONSTRUCTABLE(CBaseFontDraw);
DESTRUCTABLE(CBaseFontDraw);
public:
			void	Init(void);
			void	Class(void);
			void 	Free(void);

			bool	Save(CObjectWriter* pcFile);
			bool	Load(CObjectReader* pcFile);

	virtual void	Begin(void) =0;
	virtual void	Place(Ptr<CGlyph> pGlyph, int32 x, int32 y) =0;
	virtual void	End(void) =0;
};


#endif // __BASE_FONT_DRAW_H__


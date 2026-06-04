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
#ifndef __BLOCK_MAP_H__
#define __BLOCK_MAP_H__
#include "StandardLib/Object.h"
#include "StandardLib/Array.h"


class CBlockMap : public CObject
{
CONSTRUCTABLE(CBlockMap);
DESTRUCTABLE(CBlockMap);
protected:
	bool		mbActive;
	SInt32Vec2	msViewportPosition;

public:
			void	Init(void);
			void 	Free(void);
			void	Class(void);

			bool	Save(CObjectWriter* pcFile);
			bool	Load(CObjectReader* pcFile);

	virtual	void	Activate(void);
	virtual	void	Deactivate(void);

			void	SetViewportPosition(int32 x, int32 y);
			void	SetViewportPosition(SInt32Vec2	sViewportPosition);

	virtual	void	TileMapAbstract(void) =0;
};


typedef CArray<CBlockMap>	CArrayTileMap;


#endif // __BLOCK_MAP_H__


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
#include "ImageCelBlitterCache.h"
#include "BaseMap.h"


//A map of cels drawn with blitters made for its viewport and kept in a blitter cache.
class CBlockMap : public CBaseMap
{
CONSTRUCTABLE(CBlockMap);
DESTRUCTABLE(CBlockMap);
public:
			void		Init(void);
			void 		Free(void) override;
			void		Class(void) override;

			bool		Save(CObjectWriter* pcFile) override;
			bool		Load(CObjectReader* pcFile) override;

			CBlockMap*	GetBlockMap(void) override;

	virtual bool	CreateCelBlitters(void) =0;
	virtual bool	HasCelBlitters(void) =0;
	virtual bool	UsesCelBlitters(void) =0;
	virtual void	ClearCelBlitters(void) =0;

	virtual	void	TileMapAbstract(void) =0;

	virtual	void	SetBlitterCache(Ptr<CImageCelBlitterCache> pCache) =0;
};


#endif // __BLOCK_MAP_H__


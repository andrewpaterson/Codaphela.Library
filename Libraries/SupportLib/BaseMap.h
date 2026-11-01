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
#ifndef __BASE_MAP_H__
#define __BASE_MAP_H__
#include "StandardLib/Object.h"
#include "StandardLib/Array.h"
#include "Int32Rect.h"
#include "Image.h"


class CBlockMap;


//What CMaps draws: a map draws itself onto its viewport image, the part of the map in the viewport rectangle.  Maps of
//cels (tiles and sprites) are CBlockMaps.
class CBaseMap : public CObject
{
CONSTRUCTABLE(CBaseMap);
DESTRUCTABLE(CBaseMap);
protected:
	bool		mbActive;

public:
					void		Init(void);
					void 		Free(void) override;
					void		Class(void) override;

					bool		Save(CObjectWriter* pcFile) override;
					bool		Load(CObjectReader* pcFile) override;

	virtual			void		Activate(void);
	virtual			void		Deactivate(void);

	virtual 		bool		Blit(CInt32Rect* pcViewportRect, bool bEnsureBlitters) =0;
	virtual			void		SetViewport(Ptr<CImage> pViewport) =0;

	virtual			CBlockMap*	GetBlockMap(void);
};


typedef CArray<CBaseMap>	CArrayBaseMap;


#endif // __BASE_MAP_H__


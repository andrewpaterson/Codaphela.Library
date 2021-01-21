/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __QUAD_Parameters_H__
#define __QUAD_Parameters_H__
#include "StandardLib/StandardHeader.h"
#include "WorldLib/GraphicsTexture.h"
#include "WorldLib/GraphicsMaterial.h"
#include "WorldLib/GraphicsState.h"
#include "WorldLib/Quad2D.h"
#include "WorldLib/World.h"


class CQuadParameters : public CUnknown
{
CONSTRUCTABLE(CQuadParameters);
protected:
	CGraphicsState*		mpcGraphicsState;

public:
	void				Init(CGraphicsState* pcGraphicsState);
	void 				Kill(void);

	CGraphicsMaterial*	CreateMaterial(int iNumTextures, CWorld* pcWorld, BOOL bAlpha);
	CGraphicsState*		GetGraphicsState(void);
};


#endif // __QUAD_Parameters_H__


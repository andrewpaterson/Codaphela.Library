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
#ifndef __BLOCK_PARAMETERS_H__
#define __BLOCK_PARAMETERS_H__
#include "QuadParameters.h"
#include "SupportLib/Image.h"
#include "SupportLib/SubImage.h"


class CBlockParameters : public CQuadParameters
{
public:
	BASE_FUNCTIONS(CBlockParameters);

	CRectangle			macRects[MAX_TEX];
	unsigned int			miColour;
	CGraphicsMaterial*	mpcGraphicsMaterial;
	CGraphicsTexture*	mpcGraphicsTexture;  //[MAX_TEX]
	SInt2				msSize;
 
	void				Init(CGraphicsState* pcGraphicsState, CWorld* pcWorld, int r, int g, int b, int a);
	void				Kill(void);
	void				SetColour(int iColour);
	void				SetColour(int r, int g, int b, int a);
	CGraphicsMaterial*	GetGraphicsMaterial(void);
	CGraphicsTexture*	GetGraphicsTexture(void);
};


#endif // __BLOCK_PARAMETERS_H__


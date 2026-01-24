/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WorldLib

Codaphela WorldLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WorldLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WorldLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __MAP_2D_H__
#define __MAP_2D_H__
#include "StandardLib/ArrayUnknownTemplate.h"
#include "Quad2D.h"
#include "GraphicsViewport.h"


typedef CArrayUnknownTemplate<CQuad2D>		CArrayQuad2D;


class CMap2D
{
public:
	CArrayQuad2D		macActiveSet;
	CArrayQuad2D		macCacheSet;
	SInt3				mcPosition;
	CGraphicsObject*	mpcGraphicsObject;
	
	void 		Init(CGraphicsObject* pcGraphicsObject);
	void 		Kill(void);
	void 		Draw(void);
	void 		SetPosition(int x, int y);
	CQuad2D*	CreateQuad(CGraphicsMaterial* pcGraphicsMaterial, CGraphicsState* pcGraphicsState, CGraphicsViewport* pcGraphicsViewport, int iWidth, int iHeight, int x, int y);
	void		Clear(void);
	int			NumQuads(void);
	CQuad2D*	StartIteration(SSetIterator* psIter);
	CQuad2D*	Iterate(SSetIterator* psIter);
};


#endif // __MAP_2D_H__


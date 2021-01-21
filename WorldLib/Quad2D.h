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
#ifndef __QUAD_2D_H__
#define __QUAD_2D_H__
#include "BaseLib/DataTypes.h"
#include "BaseLib/GeometricTypes.h"
#include "StandardLib/StandardHeader.h"
#include "SupportLib/Rectangle.h"
#include "GraphicsObject.h"


#define MAX_QUAD_UV_LAYERS	2


class CQuad2D : public CUnknown
{
CONSTRUCTABLE(CQuad2D);
protected:
	SInt3				msPosition;
	CGraphicsMaterial*	mpcGraphicsMaterial;
	CGraphicsState*		mpcGraphicsState;
	CViewportInstance*	mpcGraphicsViewport;
	SInt2				msSize;
	unsigned int		mdwColour;
	SUVCornersFull		masUVs[MAX_QUAD_UV_LAYERS];

public:
	void				Init(CGraphicsMaterial* pcGraphicsMaterial, CGraphicsState* pcGraphicsState, CViewportInstance* pcGraphicsViewport, int iWidth, int iHeight, int x, int y);
	void 				Kill(void);
	BOOL 				Draw(CGraphicsObject* pcGraphicsObject);
	SInt3				GetPosition(void);
	void 				SetPosition(SInt3* psPosition);
	void 				SetPosition(int x, int y, int z);
	void				SetColour(float r, float g, float b, float a);
	void				MovePosition(int x, int y);

	void 				SetGraphicsState(CGraphicsState* psState);
	void 				SetGraphicsMaterial(CGraphicsMaterial* psMaterial);
	void 				SetGraphicsViewport(CViewportInstance* psViewport);
	void 				SetDimensions(int iWidth, int iHeight);
	void				SetUVCoordsFromRectangle(int iTextureLayer, CRectangle* psRect);
	void 				SetUVCoords(int iTextureLayer, float u1, float v1, float u2, float v2);
	void				SetUVCoords(int iTextureLayer, SUVCornersFull* psUVs);
	void				SetColour(unsigned int dwColour);
	SUVCornersFull*		GetUVs(int iLayer);
	BOOL				IsValid(void);
	BOOL				IsInViewport(void);
};


#endif // __QUAD_2D_H__


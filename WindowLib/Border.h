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
#ifndef __BORDER_H__
#define __BORDER_H__
#include "DirectXLib/Direct3DGlobal.h"
#include "SupportLib/ColourARGB.h"
#include "WorldLib/Quad2D.h"
#include "Drawable.h"


enum EBorderStyle
{
	BS_TopLeft =		0x001,
	BS_Top =			0x002,
	BS_TopRight =		0x004,
	BS_Right =			0x008,
	BS_BottomRight =	0x010,
	BS_Bottom =			0x020,
	BS_BottomLeft =		0x040,
	BS_Left =			0x080,
	BS_Center =			0x100
};


//BORDER_EDGE_INDEX
#define BEI_Top 0
#define BEI_Right 1
#define BEI_Bottom 2
#define BEI_Left 3


//BORDER_CORNER_INDEX
#define BCI_TopLeft 0
#define BCI_TopRight 1
#define BCI_BottomRight 2
#define BCI_BottomLeft 3


enum EBorderEdgeType
{
	BET_Stretch,
	BET_Tile
};


#define BORDER_EDGE_TOP			(BS_TopLeft | BS_Top | BS_TopRight)
#define BORDER_EDGE_RIGHT		(BS_TopRight | BS_Right | BS_BottomRight)
#define BORDER_EDGE_BOTTOM		(BS_BottomRight | BS_Bottom | BS_BottomLeft)
#define BORDER_EDGE_LEFT		(BS_BottomLeft | BS_Left | BS_TopLeft)
#define BORDER_CENTER			BS_Center
#define BORDER_EDGES_CORNERS	(BS_TopLeft | BS_Top | BS_TopRight | BS_Right | BS_BottomRight | BS_Bottom | BS_BottomLeft | BS_Left)
#define BORDER_EDGES			(BS_Top | BS_Right | BS_Bottom | BS_Left)
#define BORDER_CORNERS			(BS_TopLeft | BS_TopRight | BS_BottomRight | BS_BottomLeft)


#define BORDER_QUADS	8


//This belongs in it's own file.
struct SInsets
{
	int mafWidths[4];

	void Init(void);
	void Init(int* pafWidths);
};


class CBorderParameters;


class CBorder : public CDrawable
{
public:
	BASE_FUNCTIONS(CBorder);

	CBorderParameters*		mpcBorderParameters;
	float					mfAlpha;

	void				Init(CViewport* pcViewport);
	void				Kill(void);
	BOOL				Draw(void);
	void				SetRequiredSize(void);

	void				SetBorderParameters(CBorderParameters* pcParameters);
	void				CalculateEdgePositions(int* pafEdgeLength, int* pafEdgeStart);
	void				CalculateUVCoords(SUVCornersFull* apsUVs);
	void				CreateQuads(void);
	void				SetQuadDimensions(CQuad2D* mapcQuads[BORDER_QUADS], int* pafEdgeLength, int* pafEdgeStart);
	void				GetUsableArea(SInt2* psPosition, SInt2* psSize, SInt2* psContainerSize);
	void				SetAlpha(float fAlpha);

	CGraphicsTexture*	GetTexture(int iLayer);
};


#endif // __BORDER_H__


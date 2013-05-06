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
#ifndef __BORDER_PARAMETERS_H__
#define __BORDER_PARAMETERS_H__
#include "Border.h"
#include "QuadParameters.h"


class CBorderParameters : public CQuadParameters
{
public:
	BASE_FUNCTIONS(CBorderParameters);

	int					meBorderStyle;
	EBorderEdgeType		meEdgeType;
	CGraphicsTexture*	mpcGraphicsTexture;
	CGraphicsMaterial*	mpcGraphicsMaterial;


	//Why didn't you give these meaningful names?  Top, Right, Bottom, Left
	//Why are they floats but the setters take ints?
	int					maiEdgeWidths[4];
	SInt2				masCornerSizes[4];

	void				Init(int eStyle, EBorderEdgeType eEdgeType, CGraphicsState* pcGraphicsState, CWorld* pcWorld, char* szTexture, int iCornerWidth, int iConerHeight, int iEdgeWidth);
	void				Kill(void);
	void				SetEdgeWidths(int iWidth);
	void				SetEdgeWidths(int iWidth1, int iWidth2, int iWidth3, int iWidth4);
	void				SetCornerSizes(int iWidth, int iHeight);
	void				SetCornerSizes(int iSize1, int iSize2, int iSize3, int iSize4);
	void				SetCornerSizes(int iWidth1, int iHeight1, int iWidth2, int iHeight2, int iWidth3, int iHeight3, int iWidth4, int iHeight4);
	CGraphicsMaterial*	GetGraphicsMaterial(void);
	CGraphicsTexture*	GetGraphicsTexture(void);
};


#endif // __BORDER_PARAMETERS_H__


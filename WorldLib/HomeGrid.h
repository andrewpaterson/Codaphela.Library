/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela EditorLib

Codaphela EditorLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela EditorLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela EditorLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __HOME_GRID_H__
#define __HOME_GRID_H__
#include "DirectXLib/Direct3DGlobal.h"
#include "StandardLib/StandardHeader.h"
#include "WorldLib/GraphicsMaterial.h"
#include "WorldLib/GraphicsObject.h"
#include "BaseLib/DataTypes.h"
#include "DirectXLib/FlexibleVertexFormats.h"


class CWorldEditor;

class CHomeGrid : public CUnknown
{
CONSTRUCTABLE(CHomeGrid)
protected:
	SMatrix*			mpsMatrix;
	D3DXVECTOR2			msSpacing;
	SInt2				msMajorLineGap;
	D3DXVECTOR2			msSize;
	unsigned int			miCenterColour;
	unsigned int			miMajorColour;
	unsigned int			miMinorColour;
	CGraphicsObject*	mpcGraphicsObject;
	CWorldEditor*		mpcWorldEditor;
	CWorld*				mpcWorld;

	float				mfPictureTranslucency;
	CGraphicsMaterial*	mpcGridMaterial;
	CGraphicsState*		mpcGridState;
	CGraphicsMaterial*	mpcPictureMaterial;
	CGraphicsState*		mpcPictureState;
	SUVCorners			msPictureUVs;

public:
	void		Init(CWorldEditor* pcWorldEditor);
	void		Kill(void);

	bool		Draw(void);
	D3DCVERTEX*	DrawHalfGrid(D3DCVERTEX* psVertex, SFloat3* psCenter, SFloat3* psPerp, SFloat3* psDir, int iMin, int iMax, float fStep, float fWidth);
	void		CreateObjectAsNecessary(void);
	void		SetColours(int iCenterColour, int iMajorColour, int iMinorColour);
	void		SetGrid(float fSizeX, float fSizeY, float fSpaceX, float fSpaceY);
};


#endif //__HOME_GRID_H__


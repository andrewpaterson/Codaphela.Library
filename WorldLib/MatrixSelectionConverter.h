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
#ifndef __MATRIX_SELECTION_CONVERTER_H__
#define __MATRIX_SELECTION_CONVERTER_H__
#include "SelectionOverrides.h"
#include "WorldLib\GraphicsObject.h"
#include "WorldLib\MeshInstance.h"
#include "GraphicsInstance.h"
#include "DirectXLib/FlexibleVertexFormats.h"


class CWorld;
class CScene;


class CTransformNodeSelectionConverter
{
private:
	void PrivateConvert(CGraphicsObject* pcGraphicsObject, CMeshInstance* pcMeshInstance, int iMaterialIndex);

public:
	CWorld*				mpcWorld;
	CScene*				mpcScene;
	CGraphicsObject*	mpcObject;
	CMeshInstance*		mpcMeshInstance;
	SFloat4x4*			mpsCameraView;
	SFloat3*			mpsCameraPos;
	SFloat3*			mpsCameraDir;
	int					miMaterialIndex;

	void	Init(CScene* pcScene, CWorld* pcWorld);
	void	Kill(void);

	void	ConvertMatrices(CGraphicsObject* pcGraphicsObject, CMeshInstance* pcMeshInstance, int iMaterialIndex);
	void	AddMatrices(void);
	void	FinalisePrimitives(void);
	void	AssignVertex(D3DCVERTEX* psVertex, SFloat3* psPosition, unsigned int iColour, float fZOffset, SMatrix* psMatrix);
};


#endif //__MATRIX_SELECTION_CONVERTER_H__


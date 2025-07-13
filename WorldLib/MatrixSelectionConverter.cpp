/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "MatrixSelectionConverter.h"
#include "world.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransformNodeSelectionConverter::Init(CScene* pcScene, CWorld* pcWorld)
{
	mpcWorld = pcWorld;
	mpcScene = pcScene;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransformNodeSelectionConverter::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransformNodeSelectionConverter::PrivateConvert(CGraphicsObject* pcGraphicsObject, CMeshInstance* pcMeshInstance, int iMaterialIndex)
{
	mpcObject = pcGraphicsObject;
	mpcMeshInstance = pcMeshInstance;
	miMaterialIndex = iMaterialIndex;

	mpsCameraView = &mpcWorld->GetCurrentCamera()->GetViewTransform()->sD3DMatrix;
	mpsCameraPos = mpcWorld->GetCurrentCamera()->GetWorldTransform()->sD3DMatrix.Pos();
	mpsCameraDir = mpcWorld->GetCurrentCamera()->GetWorldTransform()->sD3DMatrix.At();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransformNodeSelectionConverter::ConvertMatrices(CGraphicsObject* pcGraphicsObject, CMeshInstance* pcMeshInstance, int iMaterialIndex)
{
	PrivateConvert(pcGraphicsObject, pcMeshInstance, iMaterialIndex);
	AddMatrices();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransformNodeSelectionConverter::AddMatrices(void)
{
	SFloat3			sPos;
	D3DCVERTEX*			psVertex;
	CMeshInstanceNode*	pcNode;
	int					iNum;

	sPos.Zero();
	psVertex = (D3DCVERTEX*)mpcObject->AddDynamicVertex(D3DFVF_CVERTEX);
	AssignVertex(psVertex, &sPos, 0xffffffff, 0.0f, mpcMeshInstance->GetWorldSpaceTransform());

	pcNode = mpcMeshInstance->GetNodes()->GetHead();
	while (pcNode)
	{
		psVertex = (D3DCVERTEX*)mpcObject->AddDynamicVertex(D3DFVF_CVERTEX);
		AssignVertex(psVertex, &sPos, 0xffffffff, 0.0f, pcNode->psWorldTransform);

		pcNode = mpcMeshInstance->GetNodes()->GetNext(pcNode);
	}

	iNum = mpcMeshInstance->GetNodes()->NumElements() + 1;
	mpcObject->GrowPrimitive(iNum, D3DPT_POINTLIST, iNum, 0, miMaterialIndex, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransformNodeSelectionConverter::FinalisePrimitives(void)
{
	size					i;
	CGraphicsPrimitive*		pcPrimitive;

	for (i = 0; i < mpcObject->GetPrimitives()->NumElements(); i++)
	{
		pcPrimitive = mpcObject->GetPrimitive(i);
		pcPrimitive->miFlags = GRAPH_PRIM_FLAGS_DRAW;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransformNodeSelectionConverter::AssignVertex(D3DCVERTEX* psVertex, SFloat3* psPosition, unsigned int iColour, float fZOffset, SMatrix* psMatrix)
{
	Float3TransformCoord((SFloat3*)psVertex, psPosition, &psMatrix->sD3DMatrix);
	psVertex->color = iColour;
}

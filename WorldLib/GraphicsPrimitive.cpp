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
#include "DirectXLib/FlexibleVertexFormats.h"
#include "GraphicsPrimitive.h"
#include "GraphicsObject.h"
#include "World.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsPrimitive::Init(CWorld* pcWorld)
{
	memset(this, -1, sizeof(CGraphicsPrimitive));
	miFlags = GRAPH_PRIM_FLAGS_INVALID;
	mpcWorld = pcWorld;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsPrimitive::Kill(void)
{
	miFlags = GRAPH_PRIM_FLAGS_INVALID;
	mpcWorld = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGraphicsPrimitive::Draw(void)
{
	CGraphicsState*			pcState;
	CGraphicsMaterial*		pcMaterial;
	SVertexBuffer*			psVertexBuffer;
	SMatrix*				psMatrix;
	SFloat4x4				sTemp;
	SFloat4x4*				psAdjMatrix;
	CViewportInstance*		pcViewport;

	if (!(miFlags & GRAPH_PRIM_FLAGS_DRAW))
	{
		return true;
	}
	if (mpcGraphicsObject == NULL)
	{
		gcUserError.Set("Cannot Draw because graphics object is NULL.");
		return false;
	}
	if (miFlags & GRAPH_PRIM_FLAGS_INVALID)
	{
		gcUserError.Set("Cannot Draw because primitive is marked as invalid.");
		return false;
	}
	if (miViewportPointerIndex == -1)
	{
		gcUserError.Set("Cannot Draw because Viewport is invalid.");
		return false;
	}
	if (miMaterialPointerIndex == -1)
	{
		gcUserError.Set("Cannot Draw because Material is invalid.");
		return false;
	}

	//Enable index'd matrix blending if more than one matrix is used per vertex.
	if (miFlags & GRAPH_PRIM_FLAGS_MATRIX_INDICIES)
	{
		gcD3D.SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, true);

		//Set the number of matricies to be used.  See D3DVERTEXBLENDFLAGS for more info.
		if (miNumMatricies == 1)
		{
			gcD3D.SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_0WEIGHTS);
		}
		else
		{
			gcD3D.SetRenderState(D3DRS_VERTEXBLEND, miNumMatricies-1);
		}
	}
	else
	{
		if (mpcGraphicsObject->GetAdjMatricies()->IsEmpty())
		{
			psMatrix = mpcGraphicsObject->GetMatrix(0);
			gcD3D.SetWorldMatrix(0, psMatrix);
		}
		else
		{
			psMatrix = mpcGraphicsObject->GetMatrix(0);
			psAdjMatrix = mpcGraphicsObject->GetAdjMatricies()->Get(0);
			Float4x4Multiply(&sTemp, psAdjMatrix, &psMatrix->sD3DMatrix);
			gcD3D.SetWorldMatrix(0, &sTemp);
		}

		gcD3D.SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, false);
		gcD3D.SetRenderState(D3DRS_VERTEXBLEND, 0);
	}

	//Setup the vertex buffer to use.
	psVertexBuffer = mpcGraphicsObject->GetVertexBufferForIndex(miVertexBufferIndex);
	if (psVertexBuffer->iFlags & Flags_Locked)
	{
		gcUserError.Set("Cannot Draw if a vertex buffer is locked.");
		return false;
	}

	gcD3D.SetVertexBuffer(psVertexBuffer);

	pcState = mpcGraphicsObject->GetState(miStatePointerIndex);
	pcMaterial = mpcGraphicsObject->GetMaterial(miMaterialPointerIndex);
	pcViewport = mpcGraphicsObject->GetViewport(miViewportPointerIndex);

	mpcWorld->SetGraphicsMaterial(pcMaterial);
	mpcWorld->SetGraphicsState(pcState);
	pcViewport->Draw();

	if (miFlags & GRAPH_PRIM_FLAGS_VERTEX_INDICIES)
	{
		return gcD3D.DrawIndexedPrimitive(mePrimitiveType, miStartVertex, psVertexBuffer->iNumVerticies, miStartIndex, miNumPrimitives);
	}
	else
	{
		return gcD3D.DrawPrimitive(mePrimitiveType, miStartVertex, miNumPrimitives);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsPrimitive::CalculateNumMatricies(void)
{
	CVertexBufferExtended*	psVertexBuffer;
	
	psVertexBuffer = mpcGraphicsObject->GetVertexBufferForIndex(miVertexBufferIndex);
	if (psVertexBuffer)
	{
		miNumMatricies = gcD3D.GetVertexFormatMatricies(psVertexBuffer->iVertexFormat);
		return;
	}
	miNumMatricies = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsPrimitive::Set(CGraphicsObject* pcGraphicsObject, int iNumPrimitives, D3DPRIMITIVETYPE eType, int iFlags, int iNumVerticies, int iStartIndex, int iStartVertex, int iMaterialPointerIndex, int iStatePointerIndex, int iVertexBufferIndex, int iViewportPointerIndex)
{
	mpcGraphicsObject = pcGraphicsObject;
	miNumPrimitives = iNumPrimitives;
	mePrimitiveType = eType;
	miFlags = iFlags;  //This should remove the invalid flag.
	miNumVertices = iNumVerticies;
	miStartIndex = iStartIndex;
	miStartVertex = iStartVertex;
	miMaterialPointerIndex = iMaterialPointerIndex;
	miStatePointerIndex = iStatePointerIndex;
	miVertexBufferIndex = iVertexBufferIndex;
	miViewportPointerIndex = iViewportPointerIndex;
	CalculateNumMatricies();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsPrimitive::Grow(int iNumAdditionalPrimitives, int iNumAdditionalVerticies)
{
	//This function assumes the vertex buffers already include the verticies for the new primitives.
	miNumVertices += iNumAdditionalVerticies;
	miNumPrimitives += iNumAdditionalPrimitives;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsPrimitive::Shrink(int iNumAdditionalPrimitives, int iNumAdditionalVerticies)
{
	//This function assumes the vertex buffers already include the verticies for the new primitives.
	miNumVertices -= iNumAdditionalVerticies;
	miNumPrimitives -= iNumAdditionalPrimitives;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsObject* CGraphicsPrimitive::GetGraphicsObject(void)
{
	return mpcGraphicsObject;
}
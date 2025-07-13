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
#ifndef __GRAPHICS_PRIMITIVE_H__
#define __GRAPHICS_PRIMITIVE_H__
#include "DirectXLib/Direct3DGlobal.h"
#include "GraphicsMaterial.h"
#include "GraphicsState.h"
#include "GraphicsVertexBuffer.h"


class CGraphicsObject;

#define GRAPH_PRIM_FLAGS_TRANSLUCENT		0x01
#define GRAPH_PRIM_FLAGS_VERTEX_INDICIES	0x02
#define GRAPH_PRIM_FLAGS_MATRIX_INDICIES	0x04
#define GRAPH_PRIM_FLAGS_INVALID			0x08
#define GRAPH_PRIM_FLAGS_DRAW				0x10

//The CGraphicsPrimitive is practically just a call to D3D->DrawPrimitive.
//
//The pointer pointers are needed so that the material array in the object can
//be changed by the instance and the primitives do not need to be written over
//to change materials/states.
//
//The miStartIndex index's the objects IndexBuffer.  
//
//Unless the per vertex matricies are used the zero'th world matrix is used.

class CWorld;
class CGraphicsPrimitive
{
public:
	CGraphicsObject*			mpcGraphicsObject;  //Index into the containing graphics object.
	int							miMaterialPointerIndex;  //Index into a material pointer array.
	int							miStatePointerIndex;	//Index into a state pointer array.		
	int							miVertexBufferIndex;	//The vertex buffer to use.
	int							miStartIndex;		//The index into the index buffer to start drawing from.
	int							miStartVertex;		//The vertex in the pointed to vertex buffer to start drawing from.
	int							miNumVertices;		//The number of vertices used.
	int							miNumPrimitives;	//The number of primitives to draw.
	int							miNumMatricies;		//The number of matricies per vertex, not the total number of matricies indexed by the primitive.
	int							miViewportPointerIndex;  //Index into a viewport pointer array.
	D3DPRIMITIVETYPE			mePrimitiveType;	//The type of primitive.
	int							miFlags;			//GRAPH_PRIM_FLAGS_...
	CWorld*						mpcWorld;

	void 				Init(CWorld* pcWorld);
	void 				Kill(void);

	bool 				Draw(void);
	void 				CalculateNumMatricies(void);
	void 				Set(CGraphicsObject* pcGraphicsObject, int iNumPrimitives, D3DPRIMITIVETYPE eType, int iFlags, int iNumVerticies, int iStartIndex, int iStartVertex, int iMaterialPointerIndex, int iStatePointerIndex, int iVertexBufferIndex, int iViewportPointerIndex);
	void 				Grow(int iNumAdditionalPrimitives, int iNumAdditionalVerticies);
	void 				Shrink(int iNumAdditionalPrimitives, int iNumAdditionalVerticies);
	CGraphicsObject*	GetGraphicsObject(void);
};


#endif // __GRAPHICS_PRIMITIVE_H__


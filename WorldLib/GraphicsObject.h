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
#ifndef __GRAPHICS_OBJECT_H__
#define __GRAPHICS_OBJECT_H__
#include "BaseLib/ArrayTemplateMinimal.h"
#include "BaseLib/ArrayIntMinimal.h"
#include "StandardLib/StandardHeader.h"
#include "DirectXLib/Direct3DGlobal.h"
#include "GraphicsPrimitive.h"
#include "GraphicsMaterial.h"
#include "GraphicsState.h"
#include "GraphicsViewport.h"
#include "GraphicsVertexBuffer.h"
#include "GraphicsDynamicVertexBuffer.h"
#include "MatrixArray.h"


#define GRAPH_OBJ_FLAGS_HAS_TRANSLUCENY			0x001
#define GRAPH_OBJ_FLAGS_INDEX_BUFFER			0x002
#define GRAPH_OBJ_FLAGS_EXTERNAL_VERTEX_BUFFER	0x004
#define GRAPH_OBJ_FLAGS_NO_PRIMITIVE_SORTING	0x008
#define GRAPH_OBJ_FLAGS_SORT_FRONT_TO_BACK		0x010
#define GRAPH_OBJ_FLAGS_SORT_BACK_TO_FRONT		0x020
#define GRAPH_OBJ_FLAGS_MATRIX_INDICIES			0x040
#define GRAPH_OBJ_FLAGS_DYNAMIC_BUFFERS			0x080
#define GRAPH_OBJ_FLAGS_SOFTWARE				0x100
#define GRAPH_OBJ_FLAGS_EDITING_DYNAMIC			0x200


typedef CArrayTemplate<CGraphicsPrimitive>	CArrayPrimitives;
typedef	CArrayTemplateMinimal<CGraphicsMaterial*>	CArrayPtrMaterials;
typedef CArrayTemplateMinimal<CGraphicsState*>		CArrayPtrStates;
typedef CArrayTemplate<SFloat4x4>			CArrayFloat4x4;
typedef CArrayTemplateMinimal<CViewportInstance*>	CArrayPtrViewports;


//Remember each vertex buffer can be used by many primitives.  

class CWorld;
class CGraphicsObject : public CUnknown
{
CONSTRUCTABLE(CGraphicsObject);
protected:
	CGraphicsDynamicVertexBufferArray	msDynamicVertexBuffers; //Array of vertex buffers.
	CGraphicsVertexBufferArray			msStaticVertexBuffers;	//Array of vertex buffers.
	SIndexBuffer						msIndexBuffer;   		//The only index buffer.
	CArrayPrimitives					maPrimitives;	 		//Array of CGraphicsPrimitive.
	CArrayPtrMatrices					mapMatricies;	 		//Pointers to the controlling matrices.  Altered by CGraphicsInstance
	CArrayPtrMaterials					mapMaterials;    		//Pointers to the materials.  Altered by CGraphicsInstance
	CArrayPtrStates						mapStates;		 		//Pointers to the states.   Altered by CGraphicsInstance
	CArrayPtrViewports					mapViewports;			//Pointers to the viewports.   Altered by CGraphicsInstance
	CArrayIntMinimal						maiPrimitives;	 		//The primitives to draw and the order to draw them in.
	CArrayFloat4x4						macAdjMatricies;		//A final matrix with which to multiply the controlling matrix.
	int									miFlags;		 		//The GRAPH_OBJ_FLAGS for this object.
	CWorld*								mpcWorld;

public:
	void 					Init(bool bDynamicBuffers, CWorld* pcWorld);
	void 					Kill(void);
	void					KillPrimitives(void);
	void					Reinit(void);
	bool 					Draw(void);
	void					Sort(void);
	void					SetFlags(int iFlag);
	void					SetFlag(int iFlag, bool bValue);
	bool					HasFlag(int iFlag);
	void					Lock(void);
	void					Unlock(void);

	void					SetNumVertexBuffers(int iNumBuffers);
	CVertexBufferExtended*	AddVertexBuffer(int iD3DVertexType, int iNumVertsInStaticBuffer, bool bUseExisting = true, int* piIndex = NULL);
	CVertexBufferExtended*	GetVertexBufferForType(int iD3DVertexType);
	CVertexBufferExtended*	GetVertexBufferForIndex(int iIndex);
	int						GetVertexBufferIndex(int iD3DVertexType);
	void					RemoveVertexBuffer(int iD3DVertexType);

	D3DFORMAT				CreateIndexBuffer(int iLastBetaUByte, int iNumIndices);
	SIndexBuffer*			GetIndexBuffer(void);

	void					SetNumMaterials(int iNumMaterials);
	int						AddMaterial(CGraphicsMaterial* pcGraphicsMaterial);
	CGraphicsMaterial**		AddMaterials(int iNumToAdd);
	CGraphicsMaterial*		GetMaterial(int iNum);
	void					RemoveMaterial(int iNum);
	int						GetIndexForGraphicsMaterialPointer(CGraphicsMaterial* pcGraphicsMaterial);

	void					SetNumMatricies(int iNumMatricies);
	void					AddMatrix(SMatrix* psMatrix);
	SMatrix**				AddMatricies(int iNumToAdd);
	SMatrix*				GetMatrix(int iNum);
	void					RemoveMatrix(int iNum);

	void					SetNumStates(int iNumStates);
	int						AddState(CGraphicsState* pcGraphicsState);
	CGraphicsState**		AddStates(int iNumToAdd);
	CGraphicsState*			GetState(int iNum);
	void					RemoveState(int iNum);
	int						GetIndexForGraphicsStatePointer(CGraphicsState* pcGraphicsState);

	void					SetNumViewports(int iNumViewports);
	int						AddViewport(CViewportInstance* pcGraphicsViewport);
	CViewportInstance**		AddViewports(int iNumToAdd);
	CViewportInstance*		GetViewport(int iNum);
	void					RemoveViewport(int iNum);
	int						GetIndexForGraphicsViewportPointer(CViewportInstance* pcGraphicsViewport);

	int						GetNumPrimitives(void);
	CGraphicsPrimitive*		AddPrimitives(size iNumToAdd);
	CGraphicsPrimitive*		AddPrimitive(int iNumPrimitives, D3DPRIMITIVETYPE eType, int iFlags, int iNumVerticies, int iStartIndex, int iStartVertex, int iMaterialPointerIndex, int iStatePointerIndex, int iVertexBufferIndex, int iViewportPointerIndex);
	CGraphicsPrimitive*		GrowPrimitive(int iNumAdditionalPrimitives, D3DPRIMITIVETYPE eType, int iNumAdditionalVerticies, int iVertexBufferIndex, int iMaterialPointerIndex, int iStatePointerIndex, int iViewportPointerIndex);
	void*					GrowPrimitiveNew(int iNumAdditionalPrimitives, D3DPRIMITIVETYPE eType, int iNumAdditionalVerticies, int iD3DVertexType, int iMaterialPointerIndex, int iStatePointerIndex, int iViewportPointerIndex, CGraphicsPrimitive** ppcPrimitive = NULL);
	CGraphicsPrimitive*		StartPrimitive(D3DPRIMITIVETYPE eType, int iD3DVertexType, CGraphicsMaterial* pcGraphicsMaterial, CGraphicsState* pcGraphicsState, CViewportInstance* pcViewport);
	CGraphicsPrimitive*		GetPrimitive(int iNum);
	CGraphicsPrimitive*		GetPrimitiveWithVertexBufferIndex(D3DPRIMITIVETYPE eType, int iMaterialPointerIndex, int iStatePointerIndex, int iVertexBufferIndex, int iViewportIndex);
	CGraphicsPrimitive*		GetPrimitiveWithVertexFormat(D3DPRIMITIVETYPE eType, int iMaterialPointerIndex, int iStatePointerIndex, int iVertexFormat, int iViewportIndex);
	int						GetPrimitiveWithTypeIndex(D3DPRIMITIVETYPE eType, int iStartIndex);
	void					RemoveAllPrimitives(void);
	void					RemovePrimitiveAtIndex(int iNum);
	void					RemovePrimitivesWithBadMaterials(void);
	void					RemovePrimitivesWithBadStates(void);
	void					RemovePrimitivesWithBadVertexBuffers(void);
	void					RemovePrimitivesWithBadPointers(void);  //Calls all of the above.
	void					SortPrimitives(void);

	CArrayPrimitives*		GetPrimitives(void);
	CArrayPtrMatrices*		GetMatricies(void);
	CArrayPtrMaterials*		GetMaterials(void);
	CArrayPtrStates*		GetStates(void);
	CArrayPtrViewports*		GetViewports(void);
	CArrayIntMinimal*		GetPrimitiveIndices(void);
	CArrayFloat4x4*			GetAdjMatricies(void);
	int						GetFlags(void);

	//Helper functions.
	bool					BeginDynamic(void);
	bool					EndDynamic(void);
	void*					AddDynamicVerticies(CGraphicsPrimitive* pcGraphicsPrimitive, int iD3DVertexType, int iNumPrimitives, int iNumVerticies);
	void*					AddDynamicVertex(int iD3DVertexType);
	int						GetNumberOfVerticesOfType(int iD3DVertexType);
	int						GetNumberOfVertices(int iVertexBufferIndex);
	int						GetNumberOfStaticBuffers(void);
	bool					UnlockDynamicVertexBuffers(void);
	bool					LockDynamicVertexBuffers(void);
	bool					RemoveDynamicVerticies(CGraphicsPrimitive* pcGraphicsPrimitive, int iD3DVertexType, int iNumPrimitives, int iNumVerticies);
};


#endif //__GRAPHICS_OBJECT_H__


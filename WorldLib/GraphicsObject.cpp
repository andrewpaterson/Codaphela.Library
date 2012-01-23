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
#include "GraphicsObject.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/PointerFunctions.h"
#include "World.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::Init(BOOL bDynamicBuffers, CWorld* pcWorld)
{
	memset(&msIndexBuffer, 0, sizeof(SIndexBuffer));
	msStaticVertexBuffers.Init();
	msDynamicVertexBuffers.Init();
	maPrimitives.Init();
	mapMatricies.Init();
	mapMaterials.Init();
	mapStates.Init();
	mapViewports.Init();
	maiPrimitives.Init();
	macAdjMatricies.Init();
	miFlags = 0;
	mpcWorld = pcWorld;

	if (bDynamicBuffers)
	{
		//Dynamic objects cannot have index buffers because there is too much admin.
		miFlags |= GRAPH_OBJ_FLAGS_DYNAMIC_BUFFERS;
	}
	else
	{
		miFlags |= GRAPH_OBJ_FLAGS_INDEX_BUFFER;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::Kill(void)
{
	macAdjMatricies.Init();
	maiPrimitives.Kill();
	KillPrimitives();
	mapViewports.Kill();
	mapStates.Kill();
	mapMaterials.Kill();
	mapMatricies.Kill();
	msStaticVertexBuffers.Kill();  
	msDynamicVertexBuffers.Kill();  
	gcD3D.ReleaseIndexBuffer(&msIndexBuffer);
	miFlags = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::Reinit(void)
{
	BOOL		bDynamicBuffers;
	CWorld*		pcWorld;

	bDynamicBuffers = FixBool(miFlags & GRAPH_OBJ_FLAGS_DYNAMIC_BUFFERS);
	pcWorld = mpcWorld;
	Kill();
	Init(bDynamicBuffers, pcWorld);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CGraphicsObject::Draw(void)
{
	int						i;
	CGraphicsPrimitive*		pcGraphPrim;
	SMatrix*				psMatrix;
	SFloat4x4				sTemp;
	SFloat4x4*				psAdjMatrix;
	BOOL					bResult;

	//Having GRAPH_OBJ_FLAGS_MATRIX_INDICIES set implies that GRAPH_PRIM_FLAGS_MATRIX_INDICIES is set on every GraphicsPrimitive also.
	if (miFlags & GRAPH_OBJ_FLAGS_MATRIX_INDICIES)
	{
		//Set all world matricies.  Matricies are always ordered: 0 to 255.
		if (macAdjMatricies.NumElements() == 0)
		{
			for (i = 0; i < mapMatricies.NumElements(); i++)
			{
				psMatrix = (*mapMatricies.Get(i));
				gcD3D.SetWorldMatrix(i, psMatrix);
			}
		}
		else
		{
			for (i = 0; i < mapMatricies.NumElements(); i++)
			{
				psMatrix = (*mapMatricies.Get(i));
				psAdjMatrix = macAdjMatricies.Get(i);
				Float4x4Multiply(&sTemp, psAdjMatrix, &psMatrix->sD3DMatrix);
				gcD3D.SetWorldMatrix(i, &sTemp);
			}
		}
	}

	if (miFlags & GRAPH_OBJ_FLAGS_SOFTWARE)
	{
		gcD3D.SetSoftwareVertexProcessing(TRUE);
	}
	else
	{
		gcD3D.SetSoftwareVertexProcessing(FALSE);
	}

	//Sort the primitives before drawing (only really necessary in cases of complex translucency.
	Sort();

	//Set the index buffer.  The one index buffer indexes into every vertex buffer.
	if (miFlags & GRAPH_OBJ_FLAGS_INDEX_BUFFER)
	{
		gcD3D.SetIndexBuffer(&msIndexBuffer);
	}
	for (i = 0; i < (int)maiPrimitives.NumElements(); i++)
	{
		pcGraphPrim = maPrimitives.Get(maiPrimitives.GetValue(i));
		bResult = pcGraphPrim->Draw();
		if (!bResult)
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::Sort(void)
{
	int						i;
	CCameraInstance*		pcCamera;
	CGraphicsPrimitive*		pcPrimitive;
	int						iOpaque;
	int						iTranslucent;

	//If the number of primitives has changed since last then change the draw order array.
	if (maiPrimitives.NumElements() != maPrimitives.NumElements())
	{
		maiPrimitives.SetArraySize(maPrimitives.NumElements());
	}

	if (miFlags & GRAPH_OBJ_FLAGS_NO_PRIMITIVE_SORTING)
	{
		for (i = 0; i < maiPrimitives.NumElements(); i++)
		{
			maiPrimitives.Set(i, &i);
		}
	}
	else
	{
		//When shapes start working again you need to sort front to back.
		pcCamera = mpcWorld->GetCurrentCamera();

		iOpaque = 0;
		iTranslucent = maPrimitives.NumElements() - 1;
		for (i = 0; i < maPrimitives.NumElements(); i++)
		{
			pcPrimitive = maPrimitives.Get(i);
			if (pcPrimitive->miFlags & GRAPH_PRIM_FLAGS_TRANSLUCENT)
			{
				maiPrimitives.Set(i, &iTranslucent);
				iTranslucent--;
			}
			else
			{
				maiPrimitives.Set(i, &iOpaque);
				iOpaque++;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::Lock(void)
{
	int						i;
	CVertexBufferExtended*	psVertexBuffer;

	gcD3D.LockIndexBuffer(GetIndexBuffer());
	for (i = 0; i < msStaticVertexBuffers.NumElements(); i++)
	{
		psVertexBuffer = msStaticVertexBuffers.Get(i);
		gcD3D.LockVertexBuffer(psVertexBuffer);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::Unlock(void)
{
	int						i;
	CVertexBufferExtended*	psVertexBuffer;

	gcD3D.UnlockIndexBuffer(GetIndexBuffer());
	for (i = 0; i < msStaticVertexBuffers.NumElements(); i++)
	{
		psVertexBuffer = msStaticVertexBuffers.Get(i);
		gcD3D.UnlockVertexBuffer(psVertexBuffer);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::SetFlags(int iFlags)
{
	miFlags = iFlags;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::SetFlag(int iFlag, BOOL bValue)
{
	::SetFlag(&miFlags, iFlag, bValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CGraphicsObject::HasFlag(int iFlag)
{
	return miFlags & iFlag;
}


//////////////////////////////////////////////////////////////////////////
// Vertex Buffers
//////////////////////////////////////////////////////////////////////////
CVertexBufferExtended* CGraphicsObject::AddVertexBuffer(int iD3DVertexType, int iNumVertsInStaticBuffer, BOOL bUseExisting, int* piIndex)
{
	CVertexBufferExtended*	psVertexBuffer;
	int						iIndex;

	if (miFlags & GRAPH_OBJ_FLAGS_DYNAMIC_BUFFERS)
	{
		psVertexBuffer = msDynamicVertexBuffers.AddVertexBuffer(iD3DVertexType, bUseExisting);
		SafeAssign(piIndex, msDynamicVertexBuffers.GetIndex((CGraphicsDynamicVertexBuffer*)psVertexBuffer));
		if (miFlags & GRAPH_OBJ_FLAGS_EDITING_DYNAMIC)
		{
			((CGraphicsDynamicVertexBuffer*)psVertexBuffer)->Lock();
		}
	}
	else
	{
		psVertexBuffer = msStaticVertexBuffers.AddVertexBuffer(iD3DVertexType, iNumVertsInStaticBuffer, bUseExisting);
		iIndex = msStaticVertexBuffers.GetIndex(psVertexBuffer);
		if (iIndex == -1)
		{
			gcUserError.Set("Could not get index for vertex buffer");
		}
		SafeAssign(piIndex, iIndex);
	}
	return psVertexBuffer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CVertexBufferExtended* CGraphicsObject::GetVertexBufferForType(int iD3DVertexType)
{
	if (miFlags & GRAPH_OBJ_FLAGS_DYNAMIC_BUFFERS)
	{
		return msDynamicVertexBuffers.GetVertexBuffer(iD3DVertexType);
	}
	else
	{
		return msStaticVertexBuffers.GetVertexBuffer(iD3DVertexType);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CVertexBufferExtended* CGraphicsObject::GetVertexBufferForIndex(int iIndex)
{
	if (miFlags & GRAPH_OBJ_FLAGS_DYNAMIC_BUFFERS)
	{
		return msDynamicVertexBuffers.SafeGet(iIndex);
	}
	else
	{
		return msStaticVertexBuffers.SafeGet(iIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::RemoveVertexBuffer(int iD3DVertexType)
{
	if (miFlags & GRAPH_OBJ_FLAGS_DYNAMIC_BUFFERS)
	{
		msDynamicVertexBuffers.RemoveVertexBuffer(iD3DVertexType);
	}
	else
	{
		msStaticVertexBuffers.RemoveVertexBuffer(iD3DVertexType);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::SetNumVertexBuffers(int iNumBuffers)
{
	if (miFlags & GRAPH_OBJ_FLAGS_DYNAMIC_BUFFERS)
	{
		msDynamicVertexBuffers.SetNumVertexBuffers(iNumBuffers);
	}
	else
	{
		msStaticVertexBuffers.SetNumVertexBuffers(iNumBuffers);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Materials.
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::SetNumMaterials(int iNumMaterials)
{
	void*					pvOld;
	void*					pvNew;

	//Keep track of where the vertex array was in memory.
	pvOld = mapMaterials.GetData();
	mapMaterials.SetArraySize(iNumMaterials, 0);

	//Keep track of where it might have moved to.
	pvNew = mapMaterials.GetData();

	//Remember to iterate over the graphics primitives and remove any primitives dependent on this material.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsMaterial** CGraphicsObject::AddMaterials(int iNumToAdd)
{
	void*				pvOld;
	void*				pvNew;
	CGraphicsMaterial**	pcMaterial;

	//Keep track of where the vertex array was in memory.
	pvOld = mapMaterials.GetData();
	pcMaterial = mapMaterials.SetArraySize(mapMaterials.NumElements() + iNumToAdd, 0);
	pvNew = mapMaterials.GetData();

	//Return the first of the new materials.  The rest can be accessed in array fashion.
	return pcMaterial;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGraphicsObject::AddMaterial(CGraphicsMaterial* pcGraphicsMaterial)
{
	if (pcGraphicsMaterial)
	{
		return mapMaterials.GetAdjustedIndex(mapMaterials.AddIfUnique(&pcGraphicsMaterial));
	}
	else
	{
		gcUserError.Set("Cannot add a NULL material to a CGraphicsObject.");
		return -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsMaterial* CGraphicsObject::GetMaterial(int iNum)
{
	return (*(mapMaterials.Get(iNum)));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::RemoveMaterial(int iNum)
{
	void*					pvOld;
	void*					pvNew;
	CGraphicsMaterial**		pcOld;
	CGraphicsMaterial**		pcNew;

	//Keep track of where the vertex array and the last element were in memory.
	pvOld = mapMaterials.GetData();
	pcOld = mapMaterials.Get(mapMaterials.NumElements() - 1);

	//ReMove it.
	mapMaterials.RemoveAt(iNum);

	//Find where the new element is and where the array has gone.
	pvNew = mapMaterials.GetData();
	pcNew = mapMaterials.Get(iNum);

	//Remember to iterate over the graphics primitives and remove any primitives dependent on this material.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGraphicsObject::GetIndexForGraphicsMaterialPointer(CGraphicsMaterial* pcGraphicsMaterial)
{
	CGraphicsMaterial* pcGraphicsMaterialComp;

	for (int i = 0; i <	mapMaterials.NumElements(); i++)
	{
		pcGraphicsMaterialComp = (*(mapMaterials.Get(i)));
		if (pcGraphicsMaterialComp == pcGraphicsMaterial)
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Matricies.
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::SetNumMatricies(int iNumMatricies)
{
	void*					pvOld;
	void*					pvNew;

	//Keep track of where the vertex array was in memory.
	pvOld = mapMatricies.GetData();
	mapMatricies.SetArraySize(iNumMatricies, 0);

	//Keep track of where it might have moved to.
	pvNew = mapMatricies.GetData();

	//Remember to iterate over the graphics primitives and remove any primitives dependent on this matrix.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMatrix** CGraphicsObject::AddMatricies(int iNumToAdd)
{
	void*			pvOld;
	void*			pvNew;
	SMatrix**		pcMatrix;

	//Keep track of where the matrix array was in memory.
	pvOld = mapMatricies.GetData();
	pcMatrix = mapMatricies.SetArraySize(mapMatricies.NumElements() + iNumToAdd, 0);

	//Keep track of where it might have moved to.
	pvNew = mapMatricies.GetData();

	//Return the first of the new matricies.  The rest can be accessed in array fashion.
	return pcMatrix;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMatrix* CGraphicsObject::GetMatrix(int iNum)
{
	return (*(mapMatricies.SafeGet(iNum)));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::AddMatrix(SMatrix* psMatrix)
{
	mapMatricies.Add(&psMatrix);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::RemoveMatrix(int iNum)
{
	void*			pvOld;
	void*			pvNew;
	SMatrix**		pcOld;
	SMatrix**		pcNew;

	//Keep track of where the vertex array and the last element were in memory.
	pvOld = mapMatricies.GetData();
	pcOld = mapMatricies.Get(mapMatricies.NumElements() - 1);

	//ReMove it.
	mapMatricies.RemoveAt(iNum);

	//Find where the new element is and where the array has gone.
	pvNew = mapMatricies.GetData();
	pcNew = mapMatricies.Get(iNum);

	//Remember to iterate over the graphics primitives and remove any primitives dependent on this matrix.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// States.
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::SetNumStates(int iNumStates)
{
	void*					pvOld;
	void*					pvNew;

	//Keep track of where the vertex array was in memory.
	pvOld = mapStates.GetData();
	mapStates.SetArraySize(iNumStates, 0);

	//Keep track of where it might have moved to.
	pvNew = mapStates.GetData();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGraphicsObject::AddState(CGraphicsState* pcGraphicsState)
{
	return mapStates.GetAdjustedIndex(mapStates.AddIfUnique(&pcGraphicsState));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsState** CGraphicsObject::AddStates(int iNumToAdd)
{
	void*				pvOld;
	void*				pvNew;
	CGraphicsState**	pcGraphicsState;

	//Keep track of where the vertex array was in memory.
	pvOld = mapStates.GetData();
	pcGraphicsState = mapStates.SetArraySize(mapMatricies.NumElements() + iNumToAdd, 0);

	//Keep track of where it might have moved to.
	pvNew = mapStates.GetData();

	//Return the first of the new materials.  The rest can be accessed in array fashion.
	return pcGraphicsState;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsState* CGraphicsObject::GetState(int iNum)
{
	CGraphicsState** ppcState;

	ppcState = mapStates.SafeGet(iNum);
	if (ppcState)
	{
		return *ppcState;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::RemoveState(int iNum)
{
	void*				pvOld;
	void*				pvNew;
	CGraphicsState**	pcOld;
	CGraphicsState**	pcNew;

	//Keep track of where the vertex array and the last element were in memory.
	pvOld = mapStates.GetData();
	pcOld = mapStates.Get(mapStates.NumElements() - 1);

	//Remove it.
	mapStates.RemoveAt(iNum);

	//Find where the new element is and where the array has gone.
	pvNew = mapStates.GetData();
	pcNew = mapStates.Get(iNum);

	//Remember to iterate over the graphics primitives and remove any primitives dependent on this state.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGraphicsObject::GetIndexForGraphicsStatePointer(CGraphicsState* pcGraphicsState)
{
	CGraphicsState* pcGraphicsStateComp;

	for (int i = 0; i <	mapStates.NumElements(); i++)
	{
		pcGraphicsStateComp = (*(mapStates.Get(i)));
		if (pcGraphicsStateComp == pcGraphicsState)
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Viewports.
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::SetNumViewports(int iNumViewports)
{
	void*					pvOld;
	void*					pvNew;

	//Keep track of where the vertex array was in memory.
	pvOld = mapViewports.GetData();
	mapViewports.SetArraySize(iNumViewports, 0);

	//Keep track of where it might have moved to.
	pvNew = mapViewports.GetData();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGraphicsObject::AddViewport(CViewportInstance* pcGraphicsViewport)
{
	return mapViewports.GetAdjustedIndex(mapViewports.AddIfUnique(&pcGraphicsViewport));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CViewportInstance** CGraphicsObject::AddViewports(int iNumToAdd)
{
	CViewportInstance**	pcGraphicsViewport;

	pcGraphicsViewport = mapViewports.SetArraySize(mapMatricies.NumElements() + iNumToAdd, 0);

	//Return the first of the new viewports.  The rest can be accessed in array fashion.
	return pcGraphicsViewport;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CViewportInstance* CGraphicsObject::GetViewport(int iNum)
{
	CViewportInstance** ppcViewport;

	ppcViewport = mapViewports.SafeGet(iNum);
	if (ppcViewport)
	{
		return *ppcViewport;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::RemoveViewport(int iNum)
{

	mapViewports.RemoveAt(iNum);
	//Remember to iterate over the graphics primitives and remove any primitives dependent on this viewport.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGraphicsObject::GetIndexForGraphicsViewportPointer(CViewportInstance* pcGraphicsViewport)
{
	CViewportInstance* pcGraphicsViewportComp;

	for (int i = 0; i <	mapViewports.NumElements(); i++)
	{
		pcGraphicsViewportComp = (*(mapViewports.Get(i)));
		if (pcGraphicsViewportComp == pcGraphicsViewport)
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Primitives.
//////////////////////////////////////////////////////////////////////////
CGraphicsPrimitive* CGraphicsObject::AddPrimitives(int iNumToAdd)
{
	int						i;
	int						iFirst;

	//This is the equivalent of calling .Init on each CGraphicsPrimitive.
	iFirst = maPrimitives.GrowByNumElements(iNumToAdd);
	for (i = 0; i < iNumToAdd; i++)
	{
		maPrimitives.Get(iFirst + i)->Init(mpcWorld);
	}
	return maPrimitives.Get(iFirst);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsPrimitive* CGraphicsObject::AddPrimitive(int iNumPrimitives, D3DPRIMITIVETYPE eType, int iFlags, int iNumVerticies, int iStartIndex, int iStartVertex, int iMaterialPointerIndex, int iStatePointerIndex, int iVertexBufferIndex, int iViewportPointerIndex)
{
	CGraphicsPrimitive*		pcPrimitive;

	pcPrimitive = maPrimitives.Add();
	pcPrimitive->Init(mpcWorld);
	pcPrimitive->Set(this, iNumPrimitives, eType, iFlags, iNumVerticies, iStartIndex, iStartVertex, iMaterialPointerIndex, iStatePointerIndex, iVertexBufferIndex, iViewportPointerIndex);
	return pcPrimitive;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsPrimitive* CGraphicsObject::GrowPrimitive(int iNumAdditionalPrimitives, D3DPRIMITIVETYPE eType, int iNumAdditionalVerticies, int iVertexBufferIndex, int iMaterialPointerIndex, int iStatePointerIndex, int iViewportPointerIndex)
{
	//This function assumes the vertex buffers already include the verticies for the new primitives.
	CGraphicsPrimitive*				pcPrimitive;
	CGraphicsDynamicVertexBuffer*	pcVertexBuffer;
	int								iNumVerticies;
	int								iStartVertex;

	if (!(miFlags & GRAPH_OBJ_FLAGS_DYNAMIC_BUFFERS))
	{
		gcUserError.Set("Can't GrowPrimitive unless GRAPH_OBJ_FLAGS_DYNAMIC_BUFFERS is set");
		return NULL;
	}

	//Should probably do error checking to make sure that the last vertex indexed is the last vertex in the vertex buffer.
	pcVertexBuffer = msDynamicVertexBuffers.Get(iVertexBufferIndex);
	if (!pcVertexBuffer)
	{
		return NULL;
	}
	iNumVerticies = pcVertexBuffer->mcVerts.NumElements();

	pcPrimitive = GetPrimitiveWithVertexBufferIndex(eType, iMaterialPointerIndex, iStatePointerIndex, iVertexBufferIndex, iViewportPointerIndex);
	if (!pcPrimitive)
	{
		if (iNumAdditionalPrimitives > 0)
		{
			iStartVertex = iNumVerticies - iNumAdditionalVerticies;
			pcPrimitive = AddPrimitive(iNumAdditionalPrimitives, eType, 0, iNumAdditionalVerticies, 0, iStartVertex, iMaterialPointerIndex, iStatePointerIndex, iVertexBufferIndex, iViewportPointerIndex);
			return pcPrimitive;
		}
		return NULL;
	}
	else
	{
		pcPrimitive->miNumPrimitives += iNumAdditionalPrimitives;
	}
	pcPrimitive->miNumVertices += iNumAdditionalVerticies;
	return pcPrimitive;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CGraphicsObject::GrowPrimitiveNew(int iNumAdditionalPrimitives, D3DPRIMITIVETYPE eType, int iNumAdditionalVerticies, int iD3DVertexType, int iMaterialPointerIndex, int iStatePointerIndex, int iViewportPointerIndex, CGraphicsPrimitive** ppcPrimitive)
{
	//This function assumes the vertex buffers already include the verticies for the new primitives.
	CGraphicsPrimitive*				pcPrimitive;
	CGraphicsDynamicVertexBuffer*	pcVertexBuffer;
	void*							pvFirstNewVert;
	int								iVertexBufferIndex;

	if (!(miFlags & GRAPH_OBJ_FLAGS_DYNAMIC_BUFFERS))
	{
		gcUserError.Set("Can't GrowPrimitiveNew unless GRAPH_OBJ_FLAGS_DYNAMIC_BUFFERS is set");
		return NULL;
	}

	pcPrimitive = GetPrimitiveWithVertexFormat(eType, iMaterialPointerIndex, iStatePointerIndex, iD3DVertexType, iViewportPointerIndex);

	if (!pcPrimitive)
	{
		if (iNumAdditionalPrimitives > 0)
		{
			pcVertexBuffer = (CGraphicsDynamicVertexBuffer*)AddVertexBuffer(iD3DVertexType, 0, FALSE, &iVertexBufferIndex);
			pcPrimitive = AddPrimitive(iNumAdditionalPrimitives, eType, 0, iNumAdditionalVerticies, 0, 0, iMaterialPointerIndex, iStatePointerIndex, iVertexBufferIndex, iViewportPointerIndex);
			pvFirstNewVert = pcVertexBuffer->AddVertices(iNumAdditionalVerticies);
		}
		else
		{
			SafeAssign(ppcPrimitive, NULL);
			return NULL;
		}
	}
	else
	{
		pcVertexBuffer = (CGraphicsDynamicVertexBuffer*)GetVertexBufferForIndex(pcPrimitive->miVertexBufferIndex);
		pvFirstNewVert = pcVertexBuffer->AddVertices(iNumAdditionalVerticies);
		pcPrimitive->Grow(iNumAdditionalPrimitives, iNumAdditionalVerticies);
	}

	SafeAssign(ppcPrimitive, pcPrimitive);
	return pvFirstNewVert;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsPrimitive* CGraphicsObject::StartPrimitive(D3DPRIMITIVETYPE eType, int iD3DVertexType, CGraphicsMaterial* pcGraphicsMaterial, CGraphicsState* pcGraphicsState, CViewportInstance* pcViewport)
{
	int					iVertexBufferIndex;
	int					iStateIndex;
	int					iMaterialIndex;
	int					iViewportIndex;
	CGraphicsPrimitive* pcGraphicsPrimitive;

	iVertexBufferIndex = GetVertexBufferIndex(iD3DVertexType);
	if (iVertexBufferIndex == -1)
	{
		AddVertexBuffer(iD3DVertexType, 0);
		iVertexBufferIndex = GetVertexBufferIndex(iD3DVertexType);
		if (iVertexBufferIndex == -1)
		{
			return NULL;
		}
	}

	iMaterialIndex = AddMaterial(pcGraphicsMaterial);
	iStateIndex = AddState(pcGraphicsState);
	iViewportIndex = AddViewport(pcViewport);

	//We must force at least one primitive (eg: triangle) into the graphics primitive to make it create.
	pcGraphicsPrimitive = GrowPrimitive(1, eType, 0, iVertexBufferIndex, iMaterialIndex, iStateIndex, iViewportIndex);

	//Remove the dummy primitive (eg: triangle) again.  These are both cheap operations.
	pcGraphicsPrimitive->Shrink(1, 0);

	return pcGraphicsPrimitive;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsPrimitive* CGraphicsObject::GetPrimitive(int iNum)
{
	return maPrimitives.Get(iNum);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGraphicsObject::GetNumPrimitives(void)
{
	return maPrimitives.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::RemovePrimitiveAtIndex(int iNum)
{
	CGraphicsPrimitive*	pcPrimitive;

	pcPrimitive = maPrimitives.SafeGet(iNum);
	if (pcPrimitive)
	{
		pcPrimitive->Kill();
		maPrimitives.RemoveAt(iNum);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::RemovePrimitivesWithBadMaterials(void)
{
	int						i;
	CGraphicsPrimitive*		pcPrimitive;

	for (i = 0; i < maPrimitives.NumElements(); i++)
	{
		pcPrimitive = maPrimitives.Get(i);

		if (pcPrimitive->miMaterialPointerIndex == -1)
		{
			RemovePrimitiveAtIndex(i);
			i--;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::RemovePrimitivesWithBadStates(void)
{
	int						i;
	CGraphicsPrimitive*		pcPrimitive;

	for (i = 0; i < maPrimitives.NumElements(); i++)
	{
		pcPrimitive = maPrimitives.Get(i);

		if (pcPrimitive->miStatePointerIndex == -1)
		{
			RemovePrimitiveAtIndex(i);
			i--;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::RemovePrimitivesWithBadVertexBuffers(void)
{
	int						i;
	CGraphicsPrimitive*		pcPrimitive;

	for (i = 0; i < maPrimitives.NumElements(); i++)
	{
		pcPrimitive = maPrimitives.Get(i);

		if (pcPrimitive->miVertexBufferIndex == -1)
		{
			RemovePrimitiveAtIndex(i);
			i--;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::RemovePrimitivesWithBadPointers(void)
{
	RemovePrimitivesWithBadMaterials();
	RemovePrimitivesWithBadStates();
	RemovePrimitivesWithBadVertexBuffers();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareGraphicsPrimitive(const void* arg1, const void* arg2)
{
	CGraphicsPrimitive*		pc1;
	CGraphicsPrimitive*		pc2;
	CVertexBufferExtended*	psVB1;
	CVertexBufferExtended*	psVB2;

	pc1 = (CGraphicsPrimitive*)arg1;
	pc2 = (CGraphicsPrimitive*)arg2;
	psVB1 = pc1->GetGraphicsObject()->GetVertexBufferForIndex(pc1->miVertexBufferIndex);
	psVB2 = pc2->GetGraphicsObject()->GetVertexBufferForIndex(pc2->miVertexBufferIndex);

	//Sort LARGEST to SMALLEST!
	if (psVB1->iVertexSize > psVB2->iVertexSize)
	{
		return -1;
	}
	if (psVB1->iVertexSize < psVB2->iVertexSize)
	{
		return 1;
	}

	if (pc1->miStartIndex < pc2->miStartIndex)
	{
		return -1;
	}
	if (pc1->miStartIndex > pc2->miStartIndex)
	{
		return 1;
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::SortPrimitives(void)
{
	maPrimitives.QuickSort(CompareGraphicsPrimitive);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
D3DFORMAT CGraphicsObject::CreateIndexBuffer(int iLastBetaUByte, int iNumIndices)
{
	D3DFORMAT	eD3DFormat;

	eD3DFormat = gcD3D.GetBestIndexBufferFormat(gcD3D.iCurrMon, iNumIndices);
	gcD3D.CreateIndexBuffer(iLastBetaUByte, iNumIndices, eD3DFormat, &msIndexBuffer);
	return eD3DFormat;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIndexBuffer* CGraphicsObject::GetIndexBuffer(void)
{
	return &msIndexBuffer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CGraphicsObject::BeginDynamic(void)
{
	if (miFlags & GRAPH_OBJ_FLAGS_DYNAMIC_BUFFERS)
	{
		SetFlag(GRAPH_OBJ_FLAGS_EDITING_DYNAMIC, TRUE);
		RemoveAllPrimitives();
		return LockDynamicVertexBuffers();
	}
	else
	{
		gcUserError.Set("Cannot begin dynamic if object is not using dynamic buffers.");
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CGraphicsObject::EndDynamic(void)
{
	if (miFlags & GRAPH_OBJ_FLAGS_DYNAMIC_BUFFERS)
	{
		SetFlag(GRAPH_OBJ_FLAGS_EDITING_DYNAMIC, FALSE);
		return UnlockDynamicVertexBuffers();
	}
	else
	{
		gcUserError.Set("Cannot end dynamic if object is not using dynamic buffers.");
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CGraphicsObject::AddDynamicVertex(int iD3DVertexType)
{
	CGraphicsDynamicVertexBuffer*	psVertexBuffer;

	//Assumes miFlags & GRAPH_OBJ_FLAGS_DYNAMIC_BUFFERS is TRUE

	psVertexBuffer = msDynamicVertexBuffers.GetVertexBuffer(iD3DVertexType);
	return psVertexBuffer->AddVertex();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CGraphicsObject::AddDynamicVerticies(CGraphicsPrimitive* pcGraphicsPrimitive, int iD3DVertexType, int iNumPrimitives, int iNumVerticies)
{
	CGraphicsDynamicVertexBuffer*	psVertexBuffer;
	void*							pvVertex;
	
	//Assumes miFlags & GRAPH_OBJ_FLAGS_DYNAMIC_BUFFERS is TRUE

	psVertexBuffer = msDynamicVertexBuffers.GetVertexBuffer(iD3DVertexType);
	pvVertex = psVertexBuffer->AddVertices(iNumVerticies);
	if (pvVertex == NULL)
	{
		return NULL;
	}
	pcGraphicsPrimitive->Grow(iNumPrimitives, iNumVerticies);
	return pvVertex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CGraphicsObject::RemoveDynamicVerticies(CGraphicsPrimitive* pcGraphicsPrimitive, int iD3DVertexType, int iNumPrimitives, int iNumVerticies)
{
	CVertexBufferExtended*	psVertexBuffer;
	BOOL					bResult;

	psVertexBuffer = GetVertexBufferForType(iD3DVertexType);
	bResult = psVertexBuffer->RemoveVerticies(iNumVerticies);
	if (!bResult)
	{
		return FALSE;
	}
	pcGraphicsPrimitive->Shrink(iNumPrimitives, iNumVerticies);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGraphicsObject::GetNumberOfVerticesOfType(int iD3DVertexType)
{
	CGraphicsDynamicVertexBuffer*	psVertexBuffer;

	psVertexBuffer = msDynamicVertexBuffers.GetVertexBuffer(iD3DVertexType);
	return psVertexBuffer->miUsedVerticies;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGraphicsObject::GetNumberOfVertices(int iVertexBufferIndex)
{
	CGraphicsDynamicVertexBuffer*	psVertexBuffer;

	psVertexBuffer = msDynamicVertexBuffers.Get(iVertexBufferIndex);
	return psVertexBuffer->miUsedVerticies;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGraphicsObject::GetNumberOfStaticBuffers(void)
{
	return msStaticVertexBuffers.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGraphicsObject::GetVertexBufferIndex(int iD3DVertexType)
{
	if (miFlags & GRAPH_OBJ_FLAGS_DYNAMIC_BUFFERS)
	{
		return msDynamicVertexBuffers.GetVertexBufferIndex(iD3DVertexType);
	}
	else
	{
		return msStaticVertexBuffers.GetVertexBufferIndex(iD3DVertexType);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CGraphicsObject::UnlockDynamicVertexBuffers(void)
{
	CGraphicsDynamicVertexBuffer*	psVertexBuffer;
	int								iVertexBuffer;
	BOOL							bResult;

	for (iVertexBuffer = 0; iVertexBuffer < msDynamicVertexBuffers.NumElements(); iVertexBuffer++)
	{
		psVertexBuffer = msDynamicVertexBuffers.Get(iVertexBuffer);
		bResult = psVertexBuffer->Unlock();
		if (!bResult)
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CGraphicsObject::LockDynamicVertexBuffers(void)
{
	CGraphicsDynamicVertexBuffer*	psVertexBuffer;
	int								iVertexBuffer;
	BOOL							bResult;

	for (iVertexBuffer = 0; iVertexBuffer < msDynamicVertexBuffers.NumElements(); iVertexBuffer++)
	{
		psVertexBuffer = msDynamicVertexBuffers.Get(iVertexBuffer);
		bResult = psVertexBuffer->Lock();
		if (!bResult)
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGraphicsObject::GetPrimitiveWithTypeIndex(D3DPRIMITIVETYPE eType, int iStartIndex)
{
	int						i;
	CGraphicsPrimitive*		pcPrimitive;

	for (i = iStartIndex; i < maPrimitives.NumElements(); i++)
	{
		pcPrimitive = maPrimitives.Get(i);
		if (pcPrimitive->mePrimitiveType == eType)
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsPrimitive* CGraphicsObject::GetPrimitiveWithVertexBufferIndex(D3DPRIMITIVETYPE eType, int iMaterialPointerIndex, int iStatePointerIndex, int iVertexBufferIndex, int iViewportIndex)
{
	int						i;
	CGraphicsPrimitive*		pcPrimitive;

	for (i = 0; i < maPrimitives.NumElements(); i++)
	{
		pcPrimitive = maPrimitives.Get(i);
		if ((pcPrimitive->mePrimitiveType == eType) && 
			(pcPrimitive->miMaterialPointerIndex == iMaterialPointerIndex) && 
			(pcPrimitive->miStatePointerIndex == iStatePointerIndex) && 
			(pcPrimitive->miVertexBufferIndex == iVertexBufferIndex) &&
			(pcPrimitive->miViewportPointerIndex == iViewportIndex))
		{
			return pcPrimitive;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsPrimitive* CGraphicsObject::GetPrimitiveWithVertexFormat(D3DPRIMITIVETYPE eType, int iMaterialPointerIndex, int iStatePointerIndex, int iVertexFormat, int iViewportIndex)
{
	int						i;
	CGraphicsPrimitive*		pcPrimitive;
	CVertexBufferExtended*	psVertexBuffer;

	for (i = 0; i < maPrimitives.NumElements(); i++)
	{
		pcPrimitive = maPrimitives.Get(i);

		if ((pcPrimitive->mePrimitiveType == eType) && 
			(pcPrimitive->miMaterialPointerIndex == iMaterialPointerIndex) && 
			(pcPrimitive->miStatePointerIndex == iStatePointerIndex) &&
			(pcPrimitive->miViewportPointerIndex == iViewportIndex))
		{
			psVertexBuffer = pcPrimitive->GetGraphicsObject()->GetVertexBufferForIndex(pcPrimitive->miVertexBufferIndex);
			if (psVertexBuffer->iVertexFormat == iVertexFormat)
			{
				return pcPrimitive;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::KillPrimitives(void)
{
	CGraphicsPrimitive*	pcPrimitive;
	int					i;

	for (i = 0; i < maPrimitives.NumElements(); i++)
	{
		pcPrimitive = maPrimitives.Get(i);
		pcPrimitive->Kill();
	}
	maPrimitives.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsObject::RemoveAllPrimitives(void)
{
	KillPrimitives();
	maPrimitives.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayPrimitives* CGraphicsObject::GetPrimitives(void) { return &maPrimitives; }
CArrayPtrMatrices* CGraphicsObject::GetMatricies(void) { return &mapMatricies; }
CArrayPtrMaterials* CGraphicsObject::GetMaterials(void) { return &mapMaterials; }
CArrayPtrStates* CGraphicsObject::GetStates(void) { return &mapStates; }
CArrayPtrViewports* CGraphicsObject::GetViewports(void) { return &mapViewports; }
CArraySimpleInt* CGraphicsObject::GetPrimitiveIndices(void) { return &maiPrimitives; }
CArrayFloat4x4* CGraphicsObject::GetAdjMatricies(void) { return &macAdjMatricies; }
int CGraphicsObject::GetFlags(void) { return miFlags; }


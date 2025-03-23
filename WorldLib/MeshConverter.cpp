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
#include "BaseLib/Numbers.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/Chars.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/Logger.h"
#include "SupportLib/Image.h"
#include "DirectXLib/FlexibleVertexFormats.h"
#include "SceneConverter.h"
#include "MeshConverter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVertexBufferArrayFormat::Init(SD3DVertexType* psVertexType)
{
	mpsVertexType = psVertexType;
	miNumVerticies = 0;
	macMeshVertexArrays.Init();
	macMeshVertexArrays.KillElements(FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVertexBufferArrayFormat::Kill(void)
{
	macMeshVertexArrays.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVertexBufferArrayFormat::Add(CMeshVertexArray* pcMeshVertexArray)
{
	macMeshVertexArrays.Add(pcMeshVertexArray);
	miNumVerticies += pcMeshVertexArray->GetVertexArray()->GetSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConverter::Init(CSceneConverter* pcSceneConverter)
{
	mpcSceneConverter = pcSceneConverter;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConverter::Kill(void)
{
	mpcSceneConverter = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshConverter::Convert(CGraphicsObject** ppcGraphicsObject, CMeshObject** ppcMeshObject, CMesh* pcMesh, CArrayIntAndPointer* pcConnectionAndIndex)
{
	//This assumes mpcMesh is already touched

	mpcMesh = pcMesh;
	mpcConnectionsAndIndices = pcConnectionAndIndex;
	mpcGraphicsObject = NULL;
	mpcMeshObject = NULL;

	if (mpcSceneConverter->GetMapper()->GetMesh(pcMesh->GetOI(), ppcGraphicsObject, ppcMeshObject))
	{
		return TRUE;
	}

	//Get the Position array, also make sure it has one.
	if (pcMesh->TestNumPositions() == 0)
	{
		CChars	sz;

		sz.Init("The Mesh [");
		sz.Append(pcMesh->GetName());
		sz.Append("] has no vertex position channel");
		gcUserError.Set(sz.Text());
		sz.Kill();
		return FALSE;
	}

	maiMatrixIndices.Init();

	//Create the graphics object for the conversion destination.
	mpcGraphicsObject = mpcSceneConverter->GetWorld()->CreateGraphicsObject(FALSE);
	mpcMeshObject = mpcSceneConverter->GetWorld()->CreateMeshObject();

	FillUniqueMatricies();

	if (!ConvertMaterials())
	{
		return FALSE;
	}

	if (!ConvertMesh())
	{
		return FALSE;
	}

	if (!CreateLinkNodes())
	{
		return FALSE;
	}

	if (!SetObjectMatricies())
	{
		return FALSE;
	}

	maiMatrixIndices.Kill();

	//Optimise the vertex and index buffers in the graphics object according to current hardware.
	OptimiseGraphicsObject();

	mpcSceneConverter->GetMapper()->AddGraphicsObject(pcMesh->GetOI(), mpcGraphicsObject, mpcMeshObject);

	SafeAssign(ppcGraphicsObject, mpcGraphicsObject);
	SafeAssign(ppcMeshObject, mpcMeshObject);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConverter::OptimiseGraphicsObject(void)
{
	int						i;
	int						j;
	ID3DXMesh*				pXMesh;
	CGraphicsPrimitive*		pcPrimitive;
	CVertexBufferExtended*	psVertexBuffer;
	void*					pvDestIndexBuffer;
	void*					pvDestVertexBuffer;
	void*					pvSrcIndexBuffer;
	void*					pvSrcVertexBuffer;
	int						iVertSize;
	DWORD*					pvAdjacency;
	int						iPrimitiveStart;
	int						iOldVertSize;
	int						iNumIndices;
	int						iOldVertexBufferIndex;
	int						iNumTriangles;
	void*					pvDestBaseIndexBuffer;
	SIndexBuffer*			psIndexBuffer;
	DWORD					iMeshOptions;

	mpcGraphicsObject->SortPrimitives();
	mpcGraphicsObject->Lock();
	psIndexBuffer = mpcGraphicsObject->GetIndexBuffer();
	iVertSize = 0;
	iNumIndices = 0;
	iOldVertexBufferIndex = 0;
	iNumTriangles = 0;
	iOldVertSize = 0;
	iMeshOptions = D3DXMESH_SYSTEMMEM;
	SetFlag((int*)&iMeshOptions, D3DXMESH_32BIT, psIndexBuffer->iIndexSize == 4);
	for (i = 0; i < mpcGraphicsObject->GetNumPrimitives(); i++)
	{
		pcPrimitive = mpcGraphicsObject->GetPrimitive(i);
		psVertexBuffer = mpcGraphicsObject->GetVertexBufferForIndex(pcPrimitive->miVertexBufferIndex);

		if (iOldVertSize != psVertexBuffer->iVertexSize)
		{
			if (iNumIndices != 0)
			{
				gcD3D.CreateMesh(iNumTriangles, iNumIndices, iMeshOptions, psVertexBuffer->iVertexFormat, &pXMesh);
				iVertSize = pXMesh->GetNumBytesPerVertex();
				pXMesh->LockIndexBuffer(D3DLOCK_NO_DIRTY_UPDATE, &pvDestBaseIndexBuffer);
				pXMesh->LockVertexBuffer(D3DLOCK_NO_DIRTY_UPDATE, &pvDestVertexBuffer);

				psVertexBuffer = mpcGraphicsObject->GetVertexBufferForIndex(iOldVertexBufferIndex);
				pvSrcVertexBuffer = psVertexBuffer->pvLockedBuffer;

				if (iVertSize != psVertexBuffer->iVertexSize)
				{
					gcLogger.Error("D3DX vertex size differs from expected size");
					break;
				}

				memcpy(pvDestVertexBuffer, pvSrcVertexBuffer, psVertexBuffer->iVertexSize * psVertexBuffer->iNumVerticies);

				pvDestIndexBuffer = pvDestBaseIndexBuffer;
				for (j = iPrimitiveStart; j < i; j++)
				{
					pcPrimitive = mpcGraphicsObject->GetPrimitive(j);
					pvSrcIndexBuffer = RemapSinglePointer(psIndexBuffer->pvLockedBuffer, 2 * pcPrimitive->miStartIndex);
					memcpy(pvDestIndexBuffer, pvSrcIndexBuffer, psIndexBuffer->iIndexSize * pcPrimitive->miNumVertices);
					pvDestIndexBuffer = RemapSinglePointer(pvDestIndexBuffer, pcPrimitive->miNumVertices);
				}

				pvAdjacency = (DWORD*)malloc(pcPrimitive->miNumPrimitives * 3 * sizeof(DWORD));
				pXMesh->GenerateAdjacency(0.0f, pvAdjacency);
				pXMesh->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE | D3DXMESHOPT_DONOTSPLIT, pvAdjacency, NULL, NULL, NULL);
				free(pvAdjacency);

				pvDestIndexBuffer = pvDestBaseIndexBuffer;
				for (j = iPrimitiveStart; j < i; j++)
				{
					pcPrimitive = mpcGraphicsObject->GetPrimitive(j);
					pvSrcIndexBuffer = RemapSinglePointer(psIndexBuffer->pvLockedBuffer, 2 * pcPrimitive->miStartIndex);
					memcpy(pvSrcIndexBuffer, pvDestIndexBuffer, psIndexBuffer->iIndexSize * pcPrimitive->miNumVertices);
					pvDestIndexBuffer = RemapSinglePointer(pvDestIndexBuffer, pcPrimitive->miNumVertices);
				}

				memcpy(pvSrcVertexBuffer, pvDestVertexBuffer, psVertexBuffer->iVertexSize * psVertexBuffer->iNumVerticies);

				pXMesh->UnlockIndexBuffer();
				pXMesh->UnlockVertexBuffer();
				pXMesh->Release();
			}
			iPrimitiveStart = i;
			iNumIndices = 0;
			iOldVertexBufferIndex = pcPrimitive->miVertexBufferIndex;
		}
		else
		{
			iNumIndices += pcPrimitive->miNumVertices;
			iNumTriangles += pcPrimitive->miNumPrimitives;
			if (iOldVertexBufferIndex != pcPrimitive->miVertexBufferIndex)
			{
				gcUserError.Set("Primitive vertex buffer index is FUCKED!");
				break;
			}
		}
	}
	mpcGraphicsObject->Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConverter::FillUniqueMatricies(void)
{
	int				iNumElements;
	SSkinWeight*	psSkinVert;
	CMeshSkinVert*	pcMeshSkinVert;
	int				i;
	int				j;

	if (mpcMesh->mcSkin.InUse())
	{
		iNumElements = mpcMesh->mcSkin.mcSkinVerts.NumElements();
		for (i = 0; i < iNumElements; i++)
		{
			pcMeshSkinVert = mpcMesh->mcSkin.mcSkinVerts.Get(i);
			for (j = 0; j < pcMeshSkinVert->mcWeights.NumElements(); j++)
			{
				psSkinVert = pcMeshSkinVert->mcWeights.Get(j);
				maiMatrixIndices.AddIfUnique(psSkinVert->iMatrix);
			}
		}
		maiMatrixIndices.QuickSort();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SD3DVertexType* CMeshConverter::GetVertexFormatForFaceType(SMeshFaceType sFaceType)
{
	BOOL				bNormal;
	BOOL				bColour; 
	int					iNumberOfTextures;
	int					iNumberOfMatrices;
	SD3DVertexType*		psVertexType;
	int					iD3DVertexFormat;

	bNormal = sFaceType.IsNormal();
	bColour = sFaceType.IsColour();
	iNumberOfTextures = sFaceType.GetUVs();
	iNumberOfMatrices = sFaceType.GetMatricies();

	if (iNumberOfMatrices == 1)
	{
		iNumberOfMatrices = 2;
	}

	if (iNumberOfMatrices > MAX_BONES)
	{
		iNumberOfMatrices = MAX_BONES;
	}

	psVertexType = gcD3D.GetVertexTypeFor(bNormal, bColour, iNumberOfTextures, iNumberOfMatrices, FALSE);
	if (psVertexType == NULL)
	{
		iD3DVertexFormat = gcD3D.GetVertexFormatFor(bNormal, bColour, iNumberOfTextures, iNumberOfMatrices, FALSE);
		gcD3D.AddVertexFormat(iD3DVertexFormat);
		psVertexType = gcD3D.GetVertexType(iD3DVertexFormat);
	}
	//NULL is an error and means the vertex format could not be found or created.
	return psVertexType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshConverter::ConvertMaterials(void)
{
	CMaterial*			pcMaterial;
	CGraphicsMaterial*	pcGraphicsMaterial;
	CGraphicsState*		pcGraphicsState;
	int					iMaterialIndex;
	BOOL				bResult;
	int					iNumMaterials;
	int					i;

	//Touch might need to remove unused materials from the mesh.

	iNumMaterials = mpcMesh->mcMaterials.mcMaterials.NumElements();

	if (iNumMaterials > 0)
	{
		//Convert Materials requires the graphics object to be set up.
		for (i = 0; i < iNumMaterials; i++)
		{
			iMaterialIndex = mpcMesh->mcMaterials.mcMaterials.GetValue(i);

			pcMaterial = (CMaterial*)mpcSceneConverter->GetScene()->mcMaterialTracker.GetWithID(iMaterialIndex);
			bResult = mpcSceneConverter->ConvertMaterialToGraphicsMaterial(&pcGraphicsMaterial, &pcGraphicsState, pcMaterial);
			if (!bResult)
			{
				return FALSE;
			}
			mpcGraphicsObject->AddMaterial(pcGraphicsMaterial);
			mpcGraphicsObject->AddState(pcGraphicsState);
		}
	}
	else
	{
		mpcGraphicsObject->AddMaterial(mpcSceneConverter->GetWorld()->GetGreyGraphicsMaterial());
		mpcGraphicsObject->AddState(mpcSceneConverter->GetWorld()->GetGreyGraphicsState());
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConverter::CollectVertexBuffersByFormat(CArrayVertexBufferArrayFormat* pcArray)
{
	int							i;
	SMeshFaceType*				psFaceType;
	CMeshVertexArray*			pcMeshVertexBuffer;
	SD3DVertexType*				psVertexType;
	SD3DVertexType*				psCurrent;
	CVertexBufferArrayFormat*	pcVertexBuffer;

	psCurrent = NULL;
	pcArray->Init();

	//mpcMesh->mcFaceTypes.mcUniqueTypes is sorted by vertex type
	for (i = 0; i < mpcMesh->mcFaceTypes.mcUniqueTypes.NumElements(); i++)
	{
		psFaceType = mpcMesh->mcFaceTypes.mcUniqueTypes.Get(i);
		pcMeshVertexBuffer = mpcMesh->mcCache.mcMeshVerticesArrays.Get(i);
		psVertexType = GetVertexFormatForFaceType(*psFaceType);
		if (psVertexType != psCurrent)
		{
			pcVertexBuffer = pcArray->Add();
			pcVertexBuffer->Init(psVertexType);
			psCurrent = psVertexType;
		}

		pcVertexBuffer->Add(pcMeshVertexBuffer);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshConverter::ConvertMesh(void)
{
	int								i;
	int								iVertexBufferIndex;
	void*							pvFirstNewVert;
	int								iStartVertex;
	CMeshVertexArray*				pcMeshVertexBuffer;
	CArrayVertexBufferArrayFormat	cArray;
	CVertexBufferArrayFormat*		pcVertexBufferFormatArray;
	int								j;
	CVertexArray*					pcVertexArray;
	int								iMaterial;
	int								iStartIndex;
	int								iNumFaces;
	int								iNumIndices;
	CVertexBufferExtended*			pcVertexBuffer;
	int								iFormat;

	if (!CreateIndexBuffer())
	{
		return FALSE;
	}

	CollectVertexBuffersByFormat(&cArray);

	mpcGraphicsObject->AddPrimitives(cArray.NumElements());

	iStartIndex = 0;
	for (i = 0; i < cArray.NumElements(); i++)
	{
		pcVertexBufferFormatArray = cArray.Get(i);

		iFormat = pcVertexBufferFormatArray->mpsVertexType->iD3DVertexFormat;
		pcVertexBuffer = mpcGraphicsObject->AddVertexBuffer(iFormat, pcVertexBufferFormatArray->miNumVerticies, TRUE, &iVertexBufferIndex);
		if (pcVertexBuffer == NULL)
		{
			return FALSE;
		}

		pvFirstNewVert = pcVertexBuffer->pvLockedBuffer;
		if (!pvFirstNewVert)
		{
			return FALSE;
		}

		iStartVertex = 0;
		for (j = 0; j < pcVertexBufferFormatArray->macMeshVertexArrays.NumElements(); j++)
		{
			pcMeshVertexBuffer = pcVertexBufferFormatArray->macMeshVertexArrays.Get(j);
			pcVertexArray = pcMeshVertexBuffer->GetVertexArray();
			PopulatePositions(pcVertexBufferFormatArray->mpsVertexType, pvFirstNewVert, pcVertexArray);
			PopulateNormals(pcVertexBufferFormatArray->mpsVertexType, pvFirstNewVert, pcVertexArray);
			PopulateColours(pcVertexBufferFormatArray->mpsVertexType, pvFirstNewVert, pcVertexArray);
			PopulateUVs(pcVertexBufferFormatArray->mpsVertexType, pvFirstNewVert, pcVertexArray);
			PopulateVertexWeights(pcVertexBufferFormatArray->mpsVertexType, pvFirstNewVert, pcVertexArray);

			iMaterial = pcMeshVertexBuffer->GetFaceType()->GetMaterial();
			iNumFaces = pcMeshVertexBuffer->GetFaceIndicies()->NumElements();
			iNumIndices = pcMeshVertexBuffer->GetFaceIndicies()->NumElements() * 3;
			if (!SetGraphicsPrimitive(i, iMaterial, iNumFaces, iNumIndices, iStartIndex, iStartVertex, iVertexBufferIndex))
			{
				return FALSE;
			}

			iStartIndex += iNumIndices;
			iStartVertex += pcMeshVertexBuffer->GetVertexArray()->GetSize();
		}
		gcD3D.UnlockVertexBuffer(pcVertexBuffer);
	}
	cArray.Kill();

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////
////
////
////////////////////////////////////////////////////////////////////////////
BOOL CMeshConverter::CreateIndexBuffer(void)
{
	int						i;
	int						iNumFaces;
	int						iStartIndex;
	int						j;
	SIndexBuffer*			psIndexBuffer;
	int*					piIndices;
	short int*				psiIndices;
	int						iNumIndices;
	SMeshFaceIndex*			psFaceIndex;
	CMeshVertexArray*		pcVertexArray;

	iNumFaces = 0;
	for (i = 0; i < mpcMesh->mcCache.mcMeshVerticesArrays.NumElements(); i++)
	{
		pcVertexArray = mpcMesh->mcCache.mcMeshVerticesArrays.Get(i);
		iNumFaces += pcVertexArray->GetFaceIndicies()->NumElements();
	}

	//Quick sanity check.  If the number of faces in the mesh does not matched the cached faces then we have a problem.
	if (iNumFaces != mpcMesh->NumFaces())
	{
		gcUserError.Set("Cached face count != Mesh face count.");
		return FALSE;
	}

	mpcGraphicsObject->CreateIndexBuffer(0, iNumFaces * 3);

	psIndexBuffer = mpcGraphicsObject->GetIndexBuffer();
	gcD3D.LockIndexBuffer(psIndexBuffer);
	iStartIndex = 0;

	for (i = 0; i < mpcMesh->mcCache.mcMeshVerticesArrays.NumElements(); i++)
	{
		pcVertexArray = mpcMesh->mcCache.mcMeshVerticesArrays.Get(i);

		iNumFaces = pcVertexArray->GetFaceIndicies()->NumElements();
		iNumIndices = iNumFaces * 3;

		if (psIndexBuffer->iIndexSize == 2)
		{
			psiIndices = (short int*)psIndexBuffer->pvLockedBuffer;
			for (j = 0; j < iNumFaces; j++)
			{
				psFaceIndex = pcVertexArray->GetFaceIndicies()->Get(j);
				psiIndices[iStartIndex + j * 3] = (short int)psFaceIndex->iVertIndex[0];
				psiIndices[iStartIndex + j * 3 + 1] = (short int)psFaceIndex->iVertIndex[1];
				psiIndices[iStartIndex + j * 3 + 2] = (short int)psFaceIndex->iVertIndex[2];
			}
		}
		else if (psIndexBuffer->iIndexSize == 4)
		{
			piIndices = (int*)psIndexBuffer->pvLockedBuffer;
			for (j = 0; j < iNumFaces; j++)
			{
				psFaceIndex = pcVertexArray->GetFaceIndicies()->Get(j);
				piIndices[iStartIndex + j * 3] = psFaceIndex->iVertIndex[0];
				piIndices[iStartIndex + j * 3 + 1] = psFaceIndex->iVertIndex[1];
				piIndices[iStartIndex + j * 3 + 2] = psFaceIndex->iVertIndex[2];
			}
		}

		iStartIndex += iNumIndices;
	}

	gcD3D.UnlockIndexBuffer(psIndexBuffer);
	mpcGraphicsObject->SetFlag(GRAPH_OBJ_FLAGS_INDEX_BUFFER, TRUE);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshConverter::SetGraphicsPrimitive(int iPrimitive, int iMaterialIndex, int iNumFaces, int iNumIndices, int iStartIndex, int iStartVertex, int iVertexBufferIndex)
{
	CGraphicsPrimitive*		pcGraphicsPrimitive;
	int						iStateIndex;
	int						iFlags;
	CMaterial*				pcMaterial;
	int						iMaterial;

	pcGraphicsPrimitive = mpcGraphicsObject->GetPrimitive(iPrimitive);

	iFlags = GRAPH_PRIM_FLAGS_VERTEX_INDICIES | GRAPH_PRIM_FLAGS_DRAW;

	if (iMaterialIndex != -1)
	{
		iMaterial = mpcMesh->mcMaterials.mcMaterials.GetValue(iMaterialIndex);
		pcMaterial = (CMaterial*)mpcSceneConverter->GetScene()->mcMaterialTracker.Get(iMaterial);
		if ((pcMaterial->msProperties.meOpacity == MO_Translucent) || (pcMaterial->msProperties.meOpacity == MO_Transparent))
		{
			iFlags |= GRAPH_PRIM_FLAGS_TRANSLUCENT;
		}
		iStateIndex = iMaterialIndex;
	}
	else
	{
		//This assumes the grey material has been added to the graphics object.
		iMaterialIndex = 0;
		iStateIndex = 0;
	}

	pcGraphicsPrimitive->Set(mpcGraphicsObject,
		iNumFaces,
		D3DPT_TRIANGLELIST,
		iFlags,
		iNumIndices,
		iStartIndex,
		iStartVertex,
		iMaterialIndex,
		iStateIndex,
		iVertexBufferIndex, 
		0);

	if (pcGraphicsPrimitive->miFlags & GRAPH_PRIM_FLAGS_TRANSLUCENT)
	{
		mpcGraphicsObject->SetFlag(GRAPH_OBJ_FLAGS_HAS_TRANSLUCENY, TRUE);
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConverter::PopulatePositions(SD3DVertexType* psVertexType, void* pvFirstNewVert, CVertexArray* pcVertexArray)
{
	int							j;
	void*						pvVert;
	int							iNumVerts;
	SFloat3*					psPosition;
	CChannelsAccessorCreator	cCreator;
	CChannelsAccessor*			pcAccessor;

	cCreator.Init(pcVertexArray);
	cCreator.AddAccess(VERTEX_POSITION_X, VERTEX_POSITION_Y, VERTEX_POSITION_Z, PT_float);
	pcAccessor = cCreator.CreateAndKill();

	pvVert = pvFirstNewVert;
	iNumVerts = pcVertexArray->GetSize();
	for (j = 0; j < iNumVerts; j++)
	{
		psPosition = (SFloat3*)pcAccessor->Get(j);
		((SFloat3*)pvVert)->Copy(psPosition);
		pvVert = RemapSinglePointer(pvVert, psVertexType->iD3DVertexSize);
	}

	pcAccessor->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConverter::PopulateNormals(SD3DVertexType* psVertexType, void* pvFirstNewVert, CVertexArray* pcVertexArray)
{
	int							j;
	void*						pvVert;
	int							iNumVerts;
	SFloat3*					psNormal;
	CChannelsAccessorCreator	cCreator;
	CChannelsAccessor*			pcAccessor;

	if (psVertexType->iNormalOffest != -1)
	{
		cCreator.Init(pcVertexArray);
		cCreator.AddAccess(VERTEX_NORMAL_X, VERTEX_NORMAL_Y, VERTEX_NORMAL_Z, PT_float);
		pcAccessor = cCreator.CreateAndKill();

		iNumVerts = pcVertexArray->GetSize();
		pvVert = RemapSinglePointer(pvFirstNewVert, psVertexType->iNormalOffest);
		for (j = 0; j < iNumVerts; j++)
		{
			psNormal = (SFloat3*)pcAccessor->Get(j);
			((SFloat3*)pvVert)->Copy(psNormal);
			((SFloat3*)pvVert)->Normalize();
			pvVert = RemapSinglePointer(pvVert, psVertexType->iD3DVertexSize);
		}

		pcAccessor->Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConverter::PopulateColours(SD3DVertexType* psVertexType, void* pvFirstNewVert, CVertexArray* pcVertexArray)
{
	int							j;
	void*						pvVert;
	int							iNumVerts;
	int*						piColour;
	CChannelsAccessorCreator	cCreator;
	CChannelsAccessor*			pcAccessor;

	if (psVertexType->iColourOffest != -1)
	{
		cCreator.Init(pcVertexArray);
		cCreator.AddAccess(VERTEX_DIFFUSE_RED, VERTEX_DIFFUSE_GREEN, VERTEX_DIFFUSE_BLUE, VERTEX_OPACITY, PT_uchar);
		pcAccessor = cCreator.CreateAndKill();

		iNumVerts = pcVertexArray->GetSize();
		pvVert = RemapSinglePointer(pvFirstNewVert, psVertexType->iColourOffest);
		for (j = 0; j < iNumVerts; j++)
		{
			piColour = (int*)pcAccessor->Get(j);
			*((int*)pvVert) = *piColour;
			pvVert = RemapSinglePointer(pvVert, psVertexType->iD3DVertexSize);
		}
		pcAccessor->Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConverter::PopulateUVs(SD3DVertexType* psVertexType, void* pvFirstNewVert, CVertexArray* pcVertexArray)
{
	int							j;
	void*						pvVert;
	int							iNumVerts;
	int							iNumTextures;
	int							k;
	CMeshUVLayer*				pcUVLayer;
	SFloat2*					psUV;
	CChannelsAccessorCreator	cCreator;
	CChannelsAccessor*			pcAccessor;

	if (psVertexType->iUVOffest != -1)
	{
		iNumVerts = pcVertexArray->GetSize();
		iNumTextures = gcD3D.GetVertetFormatTextures(psVertexType->iD3DVertexFormat);

		for (k = 0; k < iNumTextures; k++)
		{
			cCreator.Init(pcVertexArray);
			cCreator.AddAccess(CHANNEL(VP_UVCoord_(k), CT_U), CHANNEL(VP_UVCoord_(k), CT_V), PT_float);
			pcAccessor = cCreator.CreateAndKill();

			pvVert = RemapSinglePointer(pvFirstNewVert, psVertexType->iUVOffest + (k * sizeof(SFloat2)));
			for (j = 0; j < iNumVerts; j++)
			{
				pcUVLayer = mpcMesh->mcUVs.mcLayers.Get(k);
				psUV = (SFloat2*)pcAccessor->Get(j);
				((SFloat2*)pvVert)->Init(psUV->x, psUV->y);

				pvVert = RemapSinglePointer(pvVert, psVertexType->iD3DVertexSize);
			}

			pcAccessor->Kill();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConverter::PopulateVertexWeights(SD3DVertexType* psVertexType, void* pvFirstNewVert, CVertexArray* pcVertexArray)
{
	//int							iNumMatricies;
	//void*						pvVert;
	//int							j;
	//int							iNumVerts;
	//int							iIndex;
	//CMeshSkinVert*				pcSkinVert;
	//int							k;
	//SSkinWeight					sAdjustedWeights[MAX_BONES];
	//unsigned int				dwMatricies;
	//float						fWeight;
	//int							iMatrix;
	//float*						pf;
	//unsigned int*				pui;
	//CChannelsAccessorCreator	cCreator;
	//CChannelsAccessor*			pcMatrixAccessor;
	//CChannelsAccessor*			pcWeightAccessor;

	//if (psVertexType->iWeightOffest != -1)
	//{
	//	iNumMatricies = gcD3D.GetVertexFormatMatricies(psVertexType->iD3DVertexFormat);
	//	iNumVerts = pcVertexArray->GetSize();

	////The weights are sorted largest to smallest so it's okay to drop the end weights.
	//	if (iNumMatricies > MAX_BONES)
	//	{
	//		iNumMatricies = MAX_BONES;
	//	}

	//	pvVert = RemapSinglePointer(pvFirstNewVert, psVertexType->iWeightOffest);
	//	for (k = 0; k < iNumMatricies; k++)
	//	{
	//		cCreator.Init(pcVertexArray);
	//		cCreator.AddAccess(CHANNEL(VP_Matrix_(k), CT_Index), PT_float);
	//		pcMatrixAccessor = cCreator.CreateAndKill();

	//		cCreator.Init(pcVertexArray);
	//		cCreator.AddAccess(CHANNEL(VP_Weight_(k), CT_Intensity));
	//		pcWeightAccessor = cCreator.CreateAndKill();

	//		WeightAdjustIndices(iNumMatricies, sAdjustedWeights, pcSkinVert);
	//		dwMatricies = 0;

	//		pf = (float*)pvVert;
	//		for (j = 0; j < iNumVerts; j++)
	//		{
	//			fWeight = sAdjustedWeights[k].fWeight;
	//			iMatrix = sAdjustedWeights[k].iMatrix;
	//			*pf = fWeight;

	//			dwMatricies |= SetByte(iMatrix, k);
	//		}
	//		pui = (unsigned int*)RemapSinglePointer(pvVert, sizeof(float)* iNumMatricies);
	//		*pui = dwMatricies;
	//		pvVert = RemapSinglePointer(pvVert, psVertexType->iD3DVertexSize);
	//	}
	//}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshConverter::WeightAdjustIndices(int iNumWeights, SSkinWeight* psWeights, CMeshSkinVert* psSkinVert)
{
	int						i;
	DWORD					dwMatricies;
	float					fTotalWeight;
	float					fAdjustment;
	SSkinWeight*			psCurrentWeight;
	int						iAMatrix;

	fTotalWeight = 0.0f;
	dwMatricies = 0;
	iAMatrix = 0;

	//SkinVert is already sorted highest to lowest weight.
	for (i = 0; i < iNumWeights; i++)
	{
		psCurrentWeight = psSkinVert->mcWeights.SafeGet(i);
		if (!psCurrentWeight)
		{
			//Deal with the NVidia case where there must be at least two matricies.
			psWeights[i].fWeight = 0.0f;
			psWeights[i].iMatrix = iAMatrix;
		}
		else
		{
			iAMatrix = psCurrentWeight->iMatrix;
			psWeights[i].fWeight = psCurrentWeight->fWeight;
			psWeights[i].iMatrix = psCurrentWeight->iMatrix;
		}
		fTotalWeight += psWeights[i].fWeight;
	}

	//If the weights are not normalised at this point.
	if ((fTotalWeight < 1.0 - SMALL_NUMBER) && (fTotalWeight > 1.0 - SMALL_NUMBER))
	{
		fAdjustment = 1.0f/fTotalWeight;
		
		for (i = 0; i < iNumWeights; i++)
		{
			psWeights[i].fWeight *= fAdjustment;
		}
	}
}


////////////////////////////////////////////////////////////////////////////
////
////
////////////////////////////////////////////////////////////////////////////
//void CMeshConverter::DumpVertexBuffer(CVertexBufferExtended* psVertexBuffer, SD3DVertexType* psVertexType)
//{
//	void*			pc;
//	int				iVertexNum;
//	CChars			sz;
//	int				iStride;
//	SFloat3*		psPosition;
//	SFloat3*		psNormal;
//	unsigned int*	psColour;
//	SFloat2*		psUV;
//	int				iVertexBase;
//	int				i;
//	int*			pi;
//
//	psNormal = NULL;
//	psColour = NULL;
//	psUV = NULL;
//	pc = gcD3D.LockVertexBuffer(psVertexBuffer);
//	iStride = psVertexType->iD3DVertexSize;
//	for (iVertexNum = 0; iVertexNum < psVertexBuffer->iNumVerticies; iVertexNum++)
//	{
//		sz.Init();
//		iVertexBase = iVertexNum*iStride;
//		psPosition = (SFloat3*)RemapSinglePointer(pc, iVertexBase);
//		sz.Append(iVertexNum);
//		sz.Append(": ");
//		psPosition->ToString(&sz, 4);
//
//		sz.Append("   ");
//		for (i = 3; i < iStride/4; i++)
//		{
//			pi = (int*)RemapSinglePointer(pc, iVertexBase + i*4);
//			sz.AppendHexLoHi(pi, 4);
//			sz.Append(" ");
//		}
//
//		sz.Append('\n');
//
//		EngineOutput(sz.Text());
//		sz.Kill();
//	}
//
//	gcD3D.UnlockVertexBuffer(psVertexBuffer);
//}
//
//
////////////////////////////////////////////////////////////////////////////
////
////
////////////////////////////////////////////////////////////////////////////
//void CMeshConverter::DumpIndexBuffer(SIndexBuffer* psIndexBuffer)
//{
//	void*					pc;
//	D3DINDEXBUFFER_DESC		sDesc;
//	int						i;
//	unsigned int*			pni;
//	unsigned short*			psi;
//	CChars					sz;
//	
//
//	psIndexBuffer->lpD3DIndexBuffer->GetDesc(&sDesc);
//	pc = gcD3D.LockIndexBuffer(psIndexBuffer);
//	pni = (unsigned int*)pc;
//	psi = (unsigned short*)pc;
//
//	sz.Init();
//	if (sDesc.Format == D3DFMT_INDEX16)
//	{
//		for (i = 0; i < (int)sDesc.Size/2; i++)
//		{
//			if ((i % 3 == 0) && (i != 0))
//			{
//				sz.RemoveLastCharacter();
//				sz.RemoveLastCharacter();
//				sz.Append("\n");
//			}
//			sz.Append((int)psi[i]);
//			sz.Append(", ");
//		}
//		EngineOutput(sz.Text());
//	}
//	sz.Kill();
//	gcD3D.UnlockIndexBuffer(psIndexBuffer);
//}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshConverter::CreateLinkNodes(void)
{
	int					i;
	CConnection*		pcConnection;
	SFloat4x4*			psWorldMatrix;
	SFloat4x4*			psMatrix;
	float				fDeterminant;
	CMeshObjectNode*	pcLinkNode;
	int					iConnectionIndex;

	//Link nodes are created here so that the reference meshes matricies can be whatevered.
	mpcConnectionsAndIndices->Get(0, (void**)&pcConnection, &iConnectionIndex);

	if (pcConnection)
	{
		psWorldMatrix = (SFloat4x4*)&pcConnection->msWorldMatrix;
		for (i = 1; i <	mpcConnectionsAndIndices->NumElements(); i++)
		{
			mpcConnectionsAndIndices->Get(i, (void**)&pcConnection, &iConnectionIndex);
			psMatrix = (SFloat4x4*)&pcConnection->msWorldMatrix;

			if (i == 1)
			{
				pcLinkNode = mpcMeshObject->GetNodes()->InsertBeforeHead();
			}
			else
			{
				pcLinkNode = mpcMeshObject->GetNodes()->InsertAfterNode(pcLinkNode);
			}

			pcLinkNode->psToSubObjectSpaceFromZeroSpace = mpcSceneConverter->GetWorld()->CreateMatrix();
			pcLinkNode->psToZeroSpaceFromSubObjectSpace = mpcSceneConverter->GetWorld()->CreateMatrix();
			
			if (Float4x4Inverse(&pcLinkNode->psToSubObjectSpaceFromZeroSpace->sD3DMatrix, &fDeterminant, psMatrix) == NULL)
			{
				gcUserError.Set("Matrix has no inverse");
				return FALSE;
			}
		}
		return TRUE;
	}
	else
	{
		CChars	sz;

		sz.Init("The Mesh [");
		sz.Append(mpcMesh->GetName());
		sz.Append("] has no Connection (root node matrix).");
		gcUserError.Set(sz.Text());
		sz.Kill();
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshConverter::SetObjectMatricies(void)
{
	int				i;
	SMatrix**		ppsMatrix;
	SMatrix*		psIdentityMatrix;
	SFloat4x4*		psAdjMatrix;
	SFloat4x4*		psInverseSkinMatrix;

	if (maiMatrixIndices.NumElements() > 0)
	{
		if (maiMatrixIndices.NumElements() != mpcConnectionsAndIndices->NumElements())
		{
			gcLogger.Error("Number of matrix indices is different to number of connections");
			return FALSE;
		}
	}

	psIdentityMatrix = mpcSceneConverter->GetWorld()->GetIdentityMatrix();
	ppsMatrix = mpcGraphicsObject->AddMatricies(mpcConnectionsAndIndices->NumElements());
	for (i = 0; i < mpcConnectionsAndIndices->NumElements(); i++)
	{
		ppsMatrix[i] = psIdentityMatrix;
	}

	if (mpcMesh->mcSkin.InUse())
	{
		if (maiMatrixIndices.NumElements() != mpcMesh->mcSkin.mcInverseSkinMatricies.NumElements())
		{
			gcLogger.Error("Number of matrix indices is different to number inverse skin matrices");
			return FALSE;
		}

		for (i = 0; i < mpcMesh->mcSkin.mcInverseSkinMatricies.NumElements(); i++)
		{
			psInverseSkinMatrix = mpcMesh->mcSkin.mcInverseSkinMatricies.Get(i);
			psAdjMatrix = mpcGraphicsObject->GetAdjMatricies()->Add();
			*psAdjMatrix = *psInverseSkinMatrix;
		}
	}
	return TRUE;
}


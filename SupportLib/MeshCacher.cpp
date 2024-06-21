/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "StandardLib/ChannelsAccessorCreator.h"
#include "MeshCacher.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshCacher::Init(CMesh* pcMesh)
{
	//This assumes the mesh has been touched.
	mpcMesh = pcMesh;
	mcTypeIndices.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshCacher::Kill(void)
{
	size				i;
	CMeshTypeIndices*	pcType;

	for (i = 0; i < mcTypeIndices.NumElements(); i++)
	{
		pcType = mcTypeIndices.Get(i);
		pcType->Kill();
	}
	mcTypeIndices.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMeshCacher::GetIndexSize(SMeshFaceType* psFaceType)
{
	size	iOffset;
	size	iUVLayers;
	size	iMatricies;

	iOffset = 0;
	iOffset += sizeof(int);  //Position Index;

	if (psFaceType->IsNormal())
	{
		iOffset += sizeof(int);
	}

	if (psFaceType->IsColour())
	{
		iOffset += sizeof(int);
	}

	iUVLayers = psFaceType->GetUVs();
	if (iUVLayers > 0)
	{
		iOffset += sizeof(int) * iUVLayers;
	}

	iMatricies = psFaceType->GetMatricies();
	if (iMatricies > 0)
	{
		//If there are any matrices then a single skin vert is indexed.  It contains the list of matricies.
		iOffset += sizeof(int);
	}

	return iOffset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshCacher::Index(void)
{
	int					iUniqueFaceTypes;
	int					i;
	SMeshFaceType*		psFaceType;
	CArrayInt			aiFaces;
	int					iStride;
	void*				pvIndices;
	int					iOffset;
	int					iUVLayers;
	CMeshTypeIndices*	pcMeshTypeIndices;
	int					iNormalOffset;
	int					iColourOffset;
	int					iUVOffset;
	int					iMatrixOffset;
	int					iMatricies;
	CMeshVertexArray*	pcVertexArray;

	Kill();
	Init(mpcMesh);

	iUniqueFaceTypes = mpcMesh->mcFaceTypes.mcUniqueTypes.NumElements();

	for (i = 0; i < iUniqueFaceTypes; i++)
	{
		aiFaces.Init();
		psFaceType = mpcMesh->mcFaceTypes.mcUniqueTypes.Get(i);
		mpcMesh->mcFaceTypes.GetFacesForType(psFaceType, &aiFaces);

		iStride = GetIndexSize(psFaceType);
		pvIndices = malloc(iStride * 3 * aiFaces.NumElements());
		if (!pvIndices)
		{
			aiFaces.Kill();
			return false;
		}

		iOffset = 0;

		mpcMesh->GetConnectivity()->IndexFaceCorners((int*)pvIndices, iStride, &aiFaces);
		iOffset += sizeof(int);

		iNormalOffset = -1;
		if (psFaceType->IsNormal())
		{
			iNormalOffset = iOffset;
			mpcMesh->mcNormals.IndexFaceCorners((int*)RemapSinglePointer(pvIndices, iOffset), iStride, &aiFaces);
			iOffset += sizeof(int);
		}

		iColourOffset = -1;
		if (psFaceType->IsColour())
		{
			iColourOffset = iOffset;
			mpcMesh->mcColours.IndexFaceCorners((int*)RemapSinglePointer(pvIndices, iOffset), iStride, &aiFaces);
			iOffset += sizeof(int);
		}

		iUVOffset = -1;
		iUVLayers = psFaceType->GetUVs();
		if (iUVLayers > 0)
		{
			iUVOffset = iOffset;
			mpcMesh->mcUVs.IndexFaceCorners((int*)RemapSinglePointer(pvIndices, iOffset), iStride, &aiFaces, iUVLayers);
			iOffset += sizeof(int) * iUVLayers;
		}

		iMatrixOffset = -1;
		iMatricies = psFaceType->GetMatricies();
		if (iMatricies > 0)
		{
			iMatrixOffset = iOffset;
			mpcMesh->mcSkin.IndexFaceCorners((int*)RemapSinglePointer(pvIndices, iOffset), iStride, &aiFaces, mpcMesh->GetConnectivity());
			iOffset += sizeof(int) * iMatricies;
		}

		pcMeshTypeIndices = mcTypeIndices.Add();
		pcVertexArray = mpcMesh->mcCache.AddVertexArray(*psFaceType);
		pcMeshTypeIndices->Init(pcVertexArray, iStride, iNormalOffset, iColourOffset, iUVOffset, iMatrixOffset);

		pcMeshTypeIndices->Generate(&aiFaces, pvIndices);

		free(pvIndices);
		aiFaces.Kill();
	}

	return true;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshCacher::Vertex(void)
{
	int						i;
	CMeshTypeIndices*		pcTypeIndices;
	int						iUniqueTypes;
	int						iNumFaces;
	int						iNumVerts;
	int						iStartIndex;
	int						iNumIndices;
	CMeshVertexArray*		pcVertexArray;

	iStartIndex = 0;
	iUniqueTypes = mcTypeIndices.NumElements();
	for (i = 0; i < iUniqueTypes; i++)
	{
		pcTypeIndices = mcTypeIndices.Get(i);
		iNumFaces = pcTypeIndices->mpcVertexArray->GetFaceIndicies()->NumElements();
		iNumVerts = pcTypeIndices->maVerts.NumElements();
		iNumIndices = iNumFaces * 3;

		pcVertexArray = mpcMesh->mcCache.GetVertexArray(i);
		pcVertexArray->GetVertexArray()->BeginChange();
		pcVertexArray->GetVertexArray()->SetSize(pcTypeIndices->NumVerts());
		pcVertexArray->GetVertexArray()->EndChange();

		PopulatePositions(pcVertexArray, pcTypeIndices);
		PopulateNormals(pcVertexArray, pcTypeIndices);
		PopulateColours(pcVertexArray, pcTypeIndices);
		PopulateUVs(pcVertexArray, pcTypeIndices);
		PopulateSkin(pcVertexArray, pcTypeIndices);
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshCacher::PopulatePositions(CMeshVertexArray* pcVertexArray, CMeshTypeIndices* pcTypeIndices)
{
	int							j;
	int							iNumVerts;
	int							iIndex;
	SFloat3*					psPosition;
	CChannelsAccessorCreator	cCreator;
	CChannelsAccessor*			pcAccessor;

	cCreator.Init(pcVertexArray->GetVertexArray());
	cCreator.AddAccess(VERTEX_POSITION_X, VERTEX_POSITION_Y, VERTEX_POSITION_Z);
	pcAccessor = cCreator.CreateAndKill();

	iNumVerts = pcTypeIndices->NumVerts();
	for (j = 0; j < iNumVerts; j++)
	{
		iIndex = pcTypeIndices->GetPositionIndex(j);
		psPosition = mpcMesh->GetPosition(iIndex);
		pcAccessor->Set(j, psPosition);
	}

	pcAccessor->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshCacher::PopulateNormals(CMeshVertexArray* pcVertexArray, CMeshTypeIndices* pcTypeIndices)
{
	int							j;
	int							iNumVerts;
	int							iIndex;
	SFloat3*					psNormal;
	CChannelsAccessorCreator	cCreator;
	CChannelsAccessor*			pcAccessor;

	cCreator.Init(pcVertexArray->GetVertexArray());
	cCreator.AddAccess(VERTEX_NORMAL_X, VERTEX_NORMAL_Y, VERTEX_NORMAL_Z);
	pcAccessor = cCreator.CreateAndKill();
	if (pcAccessor)
	{
		iNumVerts = pcTypeIndices->NumVerts();
		for (j = 0; j < iNumVerts; j++)
		{
			iIndex = pcTypeIndices->GetNormalIndex(j);
			psNormal = mpcMesh->mcNormals.mcNormals.Get(iIndex);
			pcAccessor->Set(j, psNormal);
		}

		pcAccessor->Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshCacher::PopulateColours(CMeshVertexArray* pcVertexArray, CMeshTypeIndices* pcTypeIndices)
{
	size						j;
	size						iNumVerts;
	size						iIndex;
	uint32*						piColour;
	CChannelsAccessorCreator	cCreator;
	CChannelsAccessor*			pcAccessor;

	cCreator.Init(pcVertexArray->GetVertexArray());
	cCreator.AddAccess(VERTEX_DIFFUSE_RED, VERTEX_DIFFUSE_GREEN, VERTEX_DIFFUSE_BLUE, VERTEX_OPACITY);
	pcAccessor = cCreator.CreateAndKill();
	if (pcAccessor)
	{
		iNumVerts = pcTypeIndices->NumVerts();
		for (j = 0; j < iNumVerts; j++)
		{
			iIndex = pcTypeIndices->GetColourIndex(j);
			piColour = mpcMesh->mcColours.mcColours.Get(iIndex);
			pcAccessor->Set(j, piColour);
		}

		pcAccessor->Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshCacher::PopulateUVs(CMeshVertexArray* pcVertexArray, CMeshTypeIndices* pcTypeIndices)
{
	int							j;
	int							iNumVerts;
	int							iIndex;
	CChannelsAccessorCreator	cCreator;
	CChannelsAccessor*			pcAccessor;
	int							iNumUVs;
	int							k;
	CMeshUVLayer*				pcUVLayer;
	SFloat2*					psUV;

	iNumUVs = pcVertexArray->GetFaceType()->GetUVs();
	iNumVerts = pcTypeIndices->NumVerts();

	for (k = 0; k < iNumUVs; k++)
	{
		cCreator.Init(pcVertexArray->GetVertexArray());
		cCreator.AddAccess(CHANNEL(VP_UVCoord_(k), CT_U), CHANNEL(VP_UVCoord_(k), CT_V));
		pcAccessor = cCreator.CreateAndKill();

		pcUVLayer = mpcMesh->mcUVs.mcLayers.Get(k);

		for (j = 0; j < iNumVerts; j++)
		{
			iIndex = pcTypeIndices->GetUVIndex(j, k);

			psUV = pcUVLayer->mcUVs.Get(iIndex);
			pcAccessor->Set(j, psUV);
		}

		pcAccessor->Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshCacher::PopulateSkin(CMeshVertexArray* pcVertexArray, CMeshTypeIndices* pcTypeIndices)
{
	int							j;
	int							iNumVerts;
	int							iIndex;
	CChannelsAccessorCreator	cCreator;
	CChannelsAccessor*			pcMatrixAccessor;
	CChannelsAccessor*			pcWeightAccessor;
	int							iNumMatricies;
	int							k;
	CMeshSkinVert*				pcSkinVert;
	SSkinWeight*				psSkinWeight;

	iNumMatricies = pcVertexArray->GetFaceType()->GetMatricies();
	iNumVerts = pcTypeIndices->NumVerts();

	for (k = 0; k < iNumMatricies; k++)
	{
		cCreator.Init(pcVertexArray->GetVertexArray());
		cCreator.AddAccess(CHANNEL(VP_Matrix_(k), CT_Index));
		pcMatrixAccessor = cCreator.CreateAndKill();

		cCreator.Init(pcVertexArray->GetVertexArray());
		cCreator.AddAccess(CHANNEL(VP_Weight_(k), CT_Intensity));
		pcWeightAccessor = cCreator.CreateAndKill();

		for (j = 0; j < iNumVerts; j++)
		{
			iIndex = pcTypeIndices->GetSkinIndex(j);
			pcSkinVert = mpcMesh->mcSkin.mcSkinVerts.Get(iIndex);
			psSkinWeight = pcSkinVert->mcWeights.Get(k);
			pcMatrixAccessor->Set(j, &psSkinWeight->iMatrix);
			pcWeightAccessor->Set(j, &psSkinWeight->fWeight);
		}

		pcWeightAccessor->Kill();
		pcMatrixAccessor->Kill();
	}
}


/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela OldDragonLib

Codaphela OldDragonLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela OldDragonLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela OldDragonLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "BaseLib/Logger.h"
#include "OldDragonCursor.h"
#include "OldDragonImageReader.h"
#include "OldDragonReader.h"


//No source for these files survives, only the Viewer builds that read them, so all of this comes from the bytes.  Chunk
//names are numbers.  What each is was worked out from its contents.
#define CHUNK_2009_MESH				1
#define CHUNK_2009_MESH_FACES		5
#define CHUNK_2009_MESH_POSITIONS	6
#define CHUNK_2009_MATERIAL			8
#define CHUNK_2009_IMAGE			11
#define CHUNK_2009_IMAGE_INFO		12
#define CHUNK_2009_IMAGE_PIXELS		14
#define CHUNK_2009_CAMERA			17
#define CHUNK_2009_CONNECTION		26
#define CHUNK_2009_SEQUENCE			28
#define CHUNK_2009_INSTANCE			29
#define CHUNK_2009_MESH_VERTICES	2

#define MESH_2009_ARRAY_HEADER		24		//Element size, number, chunk size, allocated, a stale pointer, its type below.
#define MESH_2009_INFO_SIZE			20		//The block before the arrays in a mesh's chunk 6.
#define MESH_2009_POSITION_SIZE		12
#define MESH_2009_UV_SIZE			8
#define MESH_2009_WEIGHT_SIZE		8		//Weight, matrix.
#define MESH_2009_VERTEX_SIZE		28
#define MESH_2009_FACE_SIZE			84		//Starts with three {stale pointer, 2, position index}.
#define MESH_2009_FACE_SAVED_SIZE	188		//What a face takes in a September file.
#define MESH_2009_FACE_INTS			47
#define MATRIX_2009_SIZE			64
#define KEY_FRAME_2009_SIZE			72
#define MATERIAL_2009_PROPERTIES	80
#define MATERIAL_2009_IMAGE			28		//Image, then how it's addressed and filtered.

//An object could point to things in other arrays with a {kind, index, extra}.  Kind says what's being pointed at.
#define REFERENCE_2009_SIZE			12
#define REFERENCE_2009_POSITION		0x5d
#define REFERENCE_2009_UV			0x60
#define REFERENCE_2009_WEIGHT		0x65
#define REFERENCE_2009_MATERIAL		0x66
#define REFERENCE_2009_SMOOTHING	0x71
#define REFERENCE_2009_CONNECTION	0x6b


struct S2009Reference
{
	int32	iKind;
	int32	iIndex;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static const uint8* Read2009Array(COldDragonFile* pcFile, COldDragonCursor* pcCursor, size* puiElementSize, size* puiNum)
{
	if (pcFile->GetVersion() == ODV_August2009)
	{
		return pcCursor->ReadTaggedArrayAugust(puiElementSize, puiNum);
	}
	return pcCursor->ReadTaggedArray(puiElementSize, puiNum);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void Read2009Matrix(COldDragonCursor* pcCursor, SOldDragonMatrix* psMatrix)
{
	const uint8*	puiData;
	size			uiSize;

	puiData = pcCursor->ReadTaggedData(&uiSize);
	if ((puiData == NULL) || (uiSize != MATRIX_2009_SIZE))
	{
		pcCursor->Fail();
		return;
	}
	memcpy(psMatrix, puiData, MATRIX_2009_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
// Positions are each a block of tagged data, after a block describing
// the array they're in.
//
//////////////////////////////////////////////////////////////////////////
static bool Read2009MeshPositions(COldDragonCursor* pcCursor, COldDragonMesh* pcMesh)
{
	const uint8*	puiData;
	size			uiSize;
	uint32			auiHeader[6];
	uint32			i;
	SOldDragonVec3	sPosition;

	while (pcCursor->PeekTag() == OLD_DRAGON_TAG_DATA)
	{
		puiData = pcCursor->ReadTaggedData(&uiSize);
		if ((puiData != NULL) && (uiSize == MESH_2009_ARRAY_HEADER))
		{
			memcpy(auiHeader, puiData, MESH_2009_ARRAY_HEADER);
			if (auiHeader[0] == MESH_2009_POSITION_SIZE)
			{
				for (i = 0; i < auiHeader[1]; i++)
				{
					puiData = pcCursor->ReadTaggedData(&uiSize);
					if ((puiData == NULL) || (uiSize != MESH_2009_POSITION_SIZE))
					{
						return false;
					}
					memcpy(&sPosition, puiData, MESH_2009_POSITION_SIZE);
					pcMesh->macPositions.Add(&sPosition);
				}
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void InitOldDragonFace(SOldDragonFace* psFace)
{
	int32	j;

	for (j = 0; j < 3; j++)
	{
		psFace->aiPositions[j] = OLD_DRAGON_NO_INDEX;
		psFace->aiNormals[j] = OLD_DRAGON_NO_INDEX;
		psFace->aiUVs[j] = OLD_DRAGON_NO_INDEX;
	}
	psFace->iMaterial = OLD_DRAGON_NO_INDEX;
	psFace->uiSmoothing = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The faces were written whole, so they're the size the array says.
//
//////////////////////////////////////////////////////////////////////////
static bool Read2009MeshFacesAugust(COldDragonCursor* pcCursor, COldDragonMesh* pcMesh)
{
	const uint8*	puiData;
	size			uiElementSize;
	size			uiNum;
	size			i;
	int32			j;
	int32			aiFace[9];
	SOldDragonFace	sFace;

	puiData = pcCursor->ReadTaggedArrayAugust(&uiElementSize, &uiNum);
	if (pcCursor->Failed() || (uiElementSize != MESH_2009_FACE_SIZE))
	{
		return false;
	}

	for (i = 0; i < uiNum; i++)
	{
		memcpy(aiFace, &puiData[i * MESH_2009_FACE_SIZE], sizeof(aiFace));
		InitOldDragonFace(&sFace);
		for (j = 0; j < 3; j++)
		{
			sFace.aiPositions[j] = aiFace[j * 3 + 2];
		}
		pcMesh->macFaces.Add(&sFace);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns how many elements follow the tagged block describing an array.
//
//////////////////////////////////////////////////////////////////////////
static size Read2009ArrayDescription(COldDragonCursor* pcCursor, uint32 uiElementSize)
{
	const uint8*	puiData;
	size			uiSize;
	uint32			auiHeader[6];

	puiData = pcCursor->ReadTaggedData(&uiSize);
	if ((puiData == NULL) || (uiSize != MESH_2009_ARRAY_HEADER))
	{
		pcCursor->Fail();
		return 0;
	}
	memcpy(auiHeader, puiData, MESH_2009_ARRAY_HEADER);
	if (auiHeader[0] != uiElementSize)
	{
		pcCursor->Fail();
		return 0;
	}
	return auiHeader[1];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static const uint8* Read2009Element(COldDragonCursor* pcCursor, size uiElementSize)
{
	const uint8*	puiData;
	size			uiSize;

	puiData = pcCursor->ReadTaggedData(&uiSize);
	if ((puiData == NULL) || (uiSize != uiElementSize))
	{
		pcCursor->Fail();
		return NULL;
	}
	return puiData;
}


//////////////////////////////////////////////////////////////////////////
//
// The arrays in a September mesh's chunk 6.  Positions, UVs, weights and
// matrices are each individually tagged.  Smoothing and edge visibility
// aren't.  The chunk has to end exactly where the last array does.
//
//////////////////////////////////////////////////////////////////////////
static bool Read2009MeshArrays(COldDragonCursor* pcCursor, COldDragonMesh* pcMesh, CArrayTemplate<int32>* paiSmoothing, CArrayTemplate<SOldDragonSkinWeight>* pasWeights)
{
	size					uiNum;
	size					i;
	size					uiSize;
	const uint8*			puiData;
	SOldDragonVec3			sPosition;
	SOldDragonVec2			sUV;
	SOldDragonSkinWeight	sWeight;
	SOldDragonMatrix		sMatrix;
	int32					iSmoothing;

	puiData = pcCursor->ReadTaggedData(&uiSize);
	if ((puiData == NULL) || (uiSize != MESH_2009_INFO_SIZE))
	{
		return false;
	}

	uiNum = Read2009ArrayDescription(pcCursor, MESH_2009_POSITION_SIZE);
	for (i = 0; (i < uiNum) && !pcCursor->Failed(); i++)
	{
		puiData = Read2009Element(pcCursor, MESH_2009_POSITION_SIZE);
		if (puiData != NULL)
		{
			memcpy(&sPosition, puiData, MESH_2009_POSITION_SIZE);
			pcMesh->macPositions.Add(&sPosition);
		}
	}

	uiNum = Read2009ArrayDescription(pcCursor, sizeof(int32));
	for (i = 0; (i < uiNum) && !pcCursor->Failed(); i++)
	{
		iSmoothing = pcCursor->ReadInt32();
		paiSmoothing->Add(&iSmoothing);
	}

	uiNum = Read2009ArrayDescription(pcCursor, MESH_2009_UV_SIZE);
	for (i = 0; (i < uiNum) && !pcCursor->Failed(); i++)
	{
		puiData = Read2009Element(pcCursor, MESH_2009_UV_SIZE);
		if (puiData != NULL)
		{
			memcpy(&sUV, puiData, MESH_2009_UV_SIZE);
			pcMesh->macUVs.Add(&sUV);
		}
	}

	uiNum = Read2009ArrayDescription(pcCursor, MESH_2009_WEIGHT_SIZE);
	for (i = 0; (i < uiNum) && !pcCursor->Failed(); i++)
	{
		puiData = Read2009Element(pcCursor, MESH_2009_WEIGHT_SIZE);
		if (puiData != NULL)
		{
			memcpy(&sWeight.fWeight, puiData, sizeof(float32));
			memcpy(&sWeight.iMatrix, puiData + sizeof(float32), sizeof(int32));
			pasWeights->Add(&sWeight);
		}
	}

	uiNum = Read2009ArrayDescription(pcCursor, MATRIX_2009_SIZE);
	for (i = 0; (i < uiNum) && !pcCursor->Failed(); i++)
	{
		puiData = Read2009Element(pcCursor, MATRIX_2009_SIZE);
		if (puiData != NULL)
		{
			memcpy(&sMatrix, puiData, MATRIX_2009_SIZE);
			pcMesh->macInverseSkinMatrices.Add(&sMatrix);
		}
	}

	//Which edges are visible: three for each face.
	uiNum = Read2009ArrayDescription(pcCursor, sizeof(int32));
	if (uiNum > pcCursor->Remaining() / sizeof(int32))
	{
		return false;
	}
	pcCursor->ReadBytes(uiNum * sizeof(int32));

	return !pcCursor->Failed() && (pcCursor->Remaining() == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Chunk 2 has a list of references for each position: the position
// itself, its UVs and its weights.  Weights are copied here in position
// order so a position's are next to each other.  References are kept, as
// that's how faces get to UVs.
//
//////////////////////////////////////////////////////////////////////////
static bool Read2009MeshVertices(COldDragonCursor* pcCursor, COldDragonMesh* pcMesh, CArrayTemplate<SOldDragonSkinWeight>* pasWeights, CArrayTemplate<S2009Reference>* pasReferences, CArrayTemplate<int32>* paiFirstReference)
{
	size					uiElementSize;
	uint32					uiNum;
	uint32					i;
	size					j;
	size					uiNumReferences;
	size					uiEmpty;
	const uint8*			puiReferences;
	int32					aiReference[3];
	S2009Reference			sReference;
	SOldDragonSkinVert		sSkinVert;
	int32					iFirst;

	if (pcCursor->ReadUint32() != OLD_DRAGON_TAG_ARRAY)
	{
		return false;
	}
	uiElementSize = pcCursor->ReadUint32();
	uiNum = pcCursor->ReadUint32();
	pcCursor->ReadUint32();
	if (pcCursor->Failed() || (uiElementSize != MESH_2009_VERTEX_SIZE) || (uiNum != pcMesh->macPositions.NumElements()))
	{
		return false;
	}

	for (i = 0; i < uiNum; i++)
	{
		puiReferences = pcCursor->ReadTaggedSimple2009(REFERENCE_2009_SIZE, &uiNumReferences);
		if (pcCursor->Failed())
		{
			return false;
		}

		iFirst = (int32)pasReferences->NumElements();
		paiFirstReference->Add(&iFirst);
		sSkinVert.iFirstWeight = (int32)pcMesh->macSkinWeights.NumElements();
		sSkinVert.iNumWeights = 0;

		for (j = 0; j < uiNumReferences; j++)
		{
			memcpy(aiReference, &puiReferences[j * REFERENCE_2009_SIZE], REFERENCE_2009_SIZE);
			sReference.iKind = aiReference[0];
			sReference.iIndex = aiReference[1];
			pasReferences->Add(&sReference);

			if (sReference.iKind == REFERENCE_2009_POSITION)
			{
				if (sReference.iIndex != (int32)i)
				{
					return false;
				}
			}
			else if (sReference.iKind == REFERENCE_2009_WEIGHT)
			{
				if ((sReference.iIndex < 0) || ((size)sReference.iIndex >= pasWeights->NumElements()))
				{
					return false;
				}
				pcMesh->macSkinWeights.Add(pasWeights->Get((size)sReference.iIndex));
				sSkinVert.iNumWeights++;
			}
		}
		pcMesh->macSkinVerts.Add(&sSkinVert);

		//Two more lists, empty in every file there is, and a number that's always been 1.
		pcCursor->ReadTaggedSimple2009(0, &uiEmpty);
		if (uiEmpty != 0)
		{
			return false;
		}
		pcCursor->ReadTaggedSimple2009(0, &uiEmpty);
		if (uiEmpty != 0)
		{
			return false;
		}
		pcCursor->ReadTaggedInt();
	}
	iFirst = (int32)pasReferences->NumElements();
	paiFirstReference->Add(&iFirst);

	return !pcCursor->Failed() && (pcCursor->Remaining() == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The array's header still says a face is 84 bytes, but by September they
// were saved a field at a time and take 188:
//	The two references {smoothing, face} and {material, index}.
//	Three corners, each a tagged position and a list of two: which of that
//		position's references are the position and the UV.
//	Three edges and a tagged number that aren't read.
//
//////////////////////////////////////////////////////////////////////////
static bool Read2009MeshFacesSeptember(COldDragonCursor* pcCursor, COldDragonMesh* pcMesh, CArrayTemplate<int32>* paiSmoothing, CArrayTemplate<S2009Reference>* pasReferences, CArrayTemplate<int32>* paiFirstReference)
{
	size			uiElementSize;
	uint32			uiNum;
	uint32			i;
	int32			j;
	const uint8*	puiFace;
	int32			aiFace[MESH_2009_FACE_INTS];
	SOldDragonFace	sFace;
	int32			iPosition;
	int32			iFirst;
	int32			iNumReferences;
	int32			iPositionReference;
	int32			iUVReference;
	S2009Reference*	psReference;

	if (pcCursor->ReadUint32() != OLD_DRAGON_TAG_ARRAY)
	{
		return false;
	}
	uiElementSize = pcCursor->ReadUint32();
	uiNum = pcCursor->ReadUint32();
	pcCursor->ReadUint32();
	if (pcCursor->Failed() || (uiElementSize != MESH_2009_FACE_SIZE) || ((uint64)uiNum * MESH_2009_FACE_SAVED_SIZE != (uint64)pcCursor->Remaining()))
	{
		return false;
	}

	for (i = 0; i < uiNum; i++)
	{
		puiFace = pcCursor->ReadBytes(MESH_2009_FACE_SAVED_SIZE);
		memcpy(aiFace, puiFace, sizeof(aiFace));
		InitOldDragonFace(&sFace);

		if ((aiFace[0] != OLD_DRAGON_TAG_SIMPLE) || (aiFace[2] != 2) || (aiFace[3] != REFERENCE_2009_SMOOTHING) || (aiFace[6] != REFERENCE_2009_MATERIAL))
		{
			return false;
		}
		if ((aiFace[4] < 0) || ((size)aiFace[4] >= paiSmoothing->NumElements()))
		{
			return false;
		}
		sFace.uiSmoothing = (uint32)*paiSmoothing->Get((size)aiFace[4]);
		sFace.iMaterial = aiFace[7];

		for (j = 0; j < 3; j++)
		{
			if ((aiFace[j * 7 + 9] != OLD_DRAGON_TAG_INT) || (aiFace[j * 7 + 11] != OLD_DRAGON_TAG_SIMPLE) || (aiFace[j * 7 + 13] != 2))
			{
				return false;
			}
			iPosition = aiFace[j * 7 + 10];
			if ((iPosition < 0) || ((size)iPosition >= pcMesh->macPositions.NumElements()))
			{
				return false;
			}
			sFace.aiPositions[j] = iPosition;

			iFirst = *paiFirstReference->Get((size)iPosition);
			iNumReferences = *paiFirstReference->Get((size)iPosition + 1) - iFirst;
			iPositionReference = aiFace[j * 7 + 14];
			iUVReference = aiFace[j * 7 + 15];
			if ((iPositionReference < 0) || (iPositionReference >= iNumReferences) || (iUVReference < 0) || (iUVReference >= iNumReferences))
			{
				return false;
			}

			psReference = pasReferences->Get((size)(iFirst + iPositionReference));
			if ((psReference->iKind != REFERENCE_2009_POSITION) || (psReference->iIndex != iPosition))
			{
				return false;
			}
			psReference = pasReferences->Get((size)(iFirst + iUVReference));
			if (psReference->iKind != REFERENCE_2009_UV)
			{
				return false;
			}
			sFace.aiUVs[j] = psReference->iIndex;
		}
		pcMesh->macFaces.Add(&sFace);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A September mesh, which is all of it: positions, faces, UVs and skin.
//
//////////////////////////////////////////////////////////////////////////
static bool Read2009MeshSeptember(COldDragonFile* pcFile, size uiChunk, COldDragonMesh* pcMesh)
{
	CArrayTemplate<int32>					aiSmoothing;
	CArrayTemplate<SOldDragonSkinWeight>	asWeights;
	CArrayTemplate<S2009Reference>			asReferences;
	CArrayTemplate<int32>					aiFirstReference;
	COldDragonCursor						cCursor;
	size									uiArrays;
	size									uiVertices;
	size									uiFaces;
	bool									bResult;

	uiArrays = pcFile->FindChild(uiChunk, CHUNK_2009_MESH_POSITIONS);
	uiVertices = pcFile->FindChild(uiChunk, CHUNK_2009_MESH_VERTICES);
	uiFaces = pcFile->FindChild(uiChunk, CHUNK_2009_MESH_FACES);
	if ((uiArrays == pcFile->NumChunks()) || (uiVertices == pcFile->NumChunks()) || (uiFaces == pcFile->NumChunks()))
	{
		return gcLogger.Error2(__METHOD__, " Mesh [", pcMesh->mszName, "] is missing a chunk.", NULL);
	}

	aiSmoothing.Init();
	asWeights.Init();
	asReferences.Init();
	aiFirstReference.Init();

	cCursor.Init(pcFile->GetChunkData(pcFile->GetChunk(uiArrays)), pcFile->GetChunk(uiArrays)->uiDataSize);
	bResult = Read2009MeshArrays(&cCursor, pcMesh, &aiSmoothing, &asWeights);
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Mesh [", pcMesh->mszName, "]'s arrays couldn't be read.", NULL);
	}

	if (bResult)
	{
		cCursor.Init(pcFile->GetChunkData(pcFile->GetChunk(uiVertices)), pcFile->GetChunk(uiVertices)->uiDataSize);
		bResult = Read2009MeshVertices(&cCursor, pcMesh, &asWeights, &asReferences, &aiFirstReference);
		if (!bResult)
		{
			gcLogger.Error2(__METHOD__, " Mesh [", pcMesh->mszName, "]'s vertices couldn't be read.", NULL);
		}
	}

	if (bResult)
	{
		cCursor.Init(pcFile->GetChunkData(pcFile->GetChunk(uiFaces)), pcFile->GetChunk(uiFaces)->uiDataSize);
		bResult = Read2009MeshFacesSeptember(&cCursor, pcMesh, &aiSmoothing, &asReferences, &aiFirstReference);
		if (!bResult)
		{
			gcLogger.Error2(__METHOD__, " Mesh [", pcMesh->mszName, "]'s faces couldn't be read.", NULL);
		}
	}

	aiFirstReference.Kill();
	asReferences.Kill();
	asWeights.Kill();
	aiSmoothing.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// An August mesh's positions and faces.  Its UVs and skin aren't read:
// no August file has been checked against them.
//
//////////////////////////////////////////////////////////////////////////
static bool Read2009Mesh(COldDragonFile* pcFile, size uiChunk, COldDragonScene* pcScene)
{
	SOldDragonChunk*	psChunk;
	SOldDragonChunk*	psChild;
	COldDragonCursor	cCursor;
	COldDragonMesh*		pcMesh;
	size				uiChild;

	psChunk = pcFile->GetChunk(uiChunk);
	cCursor.Init(pcFile->GetChunkData(psChunk), psChunk->uiDataSize);
	pcMesh = pcScene->AddMesh();
	cCursor.ReadTaggedName(pcMesh->mszName, sizeof(pcMesh->mszName));
	if (cCursor.Failed())
	{
		return false;
	}

	if (pcFile->GetVersion() == ODV_September2009)
	{
		if (!Read2009MeshSeptember(pcFile, uiChunk, pcMesh))
		{
			return false;
		}
		if (!ValidateOldDragonMesh(pcMesh))
		{
			return gcLogger.Error2(__METHOD__, " Mesh [", pcMesh->mszName, "] has an index out of range.", NULL);
		}
		return true;
	}

	uiChild = pcFile->FindChild(uiChunk, CHUNK_2009_MESH_POSITIONS);
	if (uiChild == pcFile->NumChunks())
	{
		return gcLogger.Error2(__METHOD__, " Mesh [", pcMesh->mszName, "] has no positions.", NULL);
	}
	psChild = pcFile->GetChunk(uiChild);
	cCursor.Init(pcFile->GetChunkData(psChild), psChild->uiDataSize);
	if (!Read2009MeshPositions(&cCursor, pcMesh))
	{
		return gcLogger.Error2(__METHOD__, " Mesh [", pcMesh->mszName, "]'s positions couldn't be read.", NULL);
	}

	uiChild = pcFile->FindChild(uiChunk, CHUNK_2009_MESH_FACES);
	if (uiChild == pcFile->NumChunks())
	{
		return gcLogger.Error2(__METHOD__, " Mesh [", pcMesh->mszName, "] has no faces.", NULL);
	}
	psChild = pcFile->GetChunk(uiChild);
	cCursor.Init(pcFile->GetChunkData(psChild), psChild->uiDataSize);
	if (!Read2009MeshFacesAugust(&cCursor, pcMesh))
	{
		return gcLogger.Error2(__METHOD__, " Mesh [", pcMesh->mszName, "]'s faces couldn't be read.", NULL);
	}

	if (!ValidateOldDragonMesh(pcMesh))
	{
		return gcLogger.Error2(__METHOD__, " Mesh [", pcMesh->mszName, "] has a face indexing past its positions.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Name, an empty simple array, the two matrices and three flags, and a
// child chunk that's left alone.
//
//////////////////////////////////////////////////////////////////////////
static bool Read2009Connection(COldDragonCursor* pcCursor, COldDragonScene* pcScene)
{
	SOldDragonConnection*	psConnection;
	size					uiNum;

	psConnection = pcScene->AddConnection();
	pcCursor->ReadTaggedName(psConnection->szName, sizeof(psConnection->szName));
	pcCursor->ReadTaggedSimple(0, &uiNum);
	Read2009Matrix(pcCursor, &psConnection->sWorld);
	Read2009Matrix(pcCursor, &psConnection->sLocal);
	psConnection->bSelected = pcCursor->ReadTaggedInt() != 0;
	psConnection->bTopLevel = pcCursor->ReadTaggedInt() != 0;
	psConnection->bParity = pcCursor->ReadTaggedInt() != 0;
	return !pcCursor->Failed();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool Read2009Camera(COldDragonCursor* pcCursor, COldDragonScene* pcScene)
{
	SOldDragonCamera*	psCamera;

	psCamera = pcScene->AddCamera();
	pcCursor->ReadTaggedName(psCamera->szName, sizeof(psCamera->szName));
	psCamera->iType = pcCursor->ReadTaggedInt();
	psCamera->fFOV = pcCursor->ReadTaggedFloat();
	return !pcCursor->Failed() && (pcCursor->Remaining() == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool Read2009Sequence(COldDragonFile* pcFile, COldDragonCursor* pcCursor, COldDragonScene* pcScene)
{
	COldDragonSequence*	pcSequence;
	const uint8*		puiData;
	size				uiSize;
	size				uiNum;
	size				i;
	SOldDragonKeyFrame*	psKeyFrame;

	pcSequence = pcScene->AddSequence();
	pcCursor->ReadTaggedName(pcSequence->mszName, sizeof(pcSequence->mszName));

	puiData = pcCursor->ReadTaggedData(&uiSize);
	if ((puiData == NULL) || (uiSize != sizeof(int32)))
	{
		return false;
	}
	memcpy(&pcSequence->miConnection, puiData, sizeof(int32));

	puiData = Read2009Array(pcFile, pcCursor, &uiSize, &uiNum);
	if (pcCursor->Failed() || (uiSize != KEY_FRAME_2009_SIZE))
	{
		return false;
	}
	for (i = 0; i < uiNum; i++)
	{
		psKeyFrame = pcSequence->macKeyFrames.Add();
		memcpy(&psKeyFrame->sMatrix, &puiData[i * KEY_FRAME_2009_SIZE + sizeof(int32)], MATRIX_2009_SIZE);
		memcpy(&psKeyFrame->fTime, &puiData[i * KEY_FRAME_2009_SIZE + sizeof(int32) + MATRIX_2009_SIZE], sizeof(float32));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool Read2009Image(COldDragonFile* pcFile, size uiChunk, COldDragonScene* pcScene)
{
	SOldDragonChunk*	psChild;
	COldDragonCursor	cCursor;
	COldDragonImage*	pcImage;
	size				uiChild;

	uiChild = pcFile->FindChild(uiChunk, CHUNK_2009_IMAGE_INFO);
	if (uiChild == pcFile->NumChunks())
	{
		return gcLogger.Error2(__METHOD__, " Image has no size.", NULL);
	}
	psChild = pcFile->GetChunk(uiChild);
	cCursor.Init(pcFile->GetChunkData(psChild), psChild->uiDataSize);
	pcImage = pcScene->AddImage();
	cCursor.ReadTaggedName(pcImage->mszName, sizeof(pcImage->mszName));
	pcImage->miWidth = cCursor.ReadTaggedInt();
	pcImage->miHeight = cCursor.ReadTaggedInt();
	if (cCursor.Failed())
	{
		return gcLogger.Error2(__METHOD__, " Image's size couldn't be read.", NULL);
	}

	uiChild = pcFile->FindChild(uiChunk, CHUNK_2009_IMAGE_PIXELS);
	if (uiChild == pcFile->NumChunks())
	{
		return true;
	}
	psChild = pcFile->GetChunk(uiChild);
	cCursor.Init(pcFile->GetChunkData(psChild), psChild->uiDataSize);
	return ReadOldDragonImagePixels(&cCursor, pcImage);
}


//////////////////////////////////////////////////////////////////////////
//
// The colours in a material's properties aren't understood, so they
// aren't kept.  Its images are the first int of each slot.
//
//////////////////////////////////////////////////////////////////////////
static bool Read2009Material(COldDragonCursor* pcCursor, COldDragonScene* pcScene)
{
	SOldDragonMaterial*	psMaterial;
	const uint8*		puiData;

	psMaterial = pcScene->AddMaterial();
	pcCursor->ReadTaggedName(psMaterial->szName, sizeof(psMaterial->szName));
	if (Read2009Element(pcCursor, MATERIAL_2009_PROPERTIES) == NULL)
	{
		return false;
	}

	while (pcCursor->Remaining() > 0)
	{
		puiData = Read2009Element(pcCursor, MATERIAL_2009_IMAGE);
		if ((puiData == NULL) || (psMaterial->iNumTextures >= OLD_DRAGON_MAX_TEXTURES))
		{
			return false;
		}
		memcpy(&psMaterial->aiTextures[psMaterial->iNumTextures], puiData, sizeof(int32));
		psMaterial->iNumTextures++;
	}
	return !pcCursor->Failed();
}


//////////////////////////////////////////////////////////////////////////
//
// A list of references: the object, and the connections placing it.  A
// child chunk follows that's left alone.
//
//////////////////////////////////////////////////////////////////////////
static bool Read2009Instance(COldDragonCursor* pcCursor, COldDragonScene* pcScene)
{
	COldDragonInstance*	pcInstance;
	const uint8*		puiReferences;
	size				uiNum;
	size				i;
	int32				aiReference[3];

	pcInstance = pcScene->AddInstance();
	pcCursor->ReadTaggedName(pcInstance->mszName, sizeof(pcInstance->mszName));
	puiReferences = pcCursor->ReadTaggedSimple2009(REFERENCE_2009_SIZE, &uiNum);
	if (pcCursor->Failed())
	{
		return false;
	}

	for (i = 0; i < uiNum; i++)
	{
		memcpy(aiReference, &puiReferences[i * REFERENCE_2009_SIZE], REFERENCE_2009_SIZE);
		if (aiReference[0] == REFERENCE_2009_CONNECTION)
		{
			pcInstance->maiConnections.Add(&aiReference[1]);
		}
		else
		{
			pcInstance->miType = aiReference[0];
			pcInstance->miObject = aiReference[1];
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ReadOldDragon2009(COldDragonFile* pcFile, COldDragonScene* pcScene)
{
	size				i;
	SOldDragonChunk*	psChunk;
	COldDragonCursor	cCursor;
	bool				bResult;

	bResult = true;
	for (i = 1; (i < pcFile->NumChunks()) && bResult; i++)
	{
		psChunk = pcFile->GetChunk(i);
		if (psChunk->iParent != 0)
		{
			continue;
		}

		cCursor.Init(pcFile->GetChunkData(psChunk), psChunk->uiDataSize);
		switch (psChunk->uiName)
		{
		case CHUNK_2009_MESH:
			bResult = Read2009Mesh(pcFile, i, pcScene);
			break;
		case CHUNK_2009_MATERIAL:
			bResult = Read2009Material(&cCursor, pcScene);
			break;
		case CHUNK_2009_IMAGE:
			bResult = Read2009Image(pcFile, i, pcScene);
			break;
		case CHUNK_2009_CAMERA:
			bResult = Read2009Camera(&cCursor, pcScene);
			break;
		case CHUNK_2009_CONNECTION:
			bResult = Read2009Connection(&cCursor, pcScene);
			break;
		case CHUNK_2009_SEQUENCE:
			bResult = Read2009Sequence(pcFile, &cCursor, pcScene);
			break;
		case CHUNK_2009_INSTANCE:
			bResult = Read2009Instance(&cCursor, pcScene);
			break;
		default:
			pcScene->muiSkippedChunks++;
			break;
		}
	}
	return bResult;
}


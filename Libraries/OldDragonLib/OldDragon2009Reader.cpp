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

#define MESH_2009_ARRAY_HEADER		24		//Element size, number, allocated, used, a stale pointer, a type.
#define MESH_2009_POSITION_SIZE		12
#define MESH_2009_FACE_SIZE			84		//Starts with three {stale pointer, 2, position index}.
#define MESH_2009_FACE_SAVED_SIZE	188		//What a face takes in a September file.
#define MATRIX_2009_SIZE			64
#define KEY_FRAME_2009_SIZE			72


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
				for (i = 0; i < auiHeader[3]; i++)
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
// The array's header still says a face is 84 bytes, but by September they
// were saved a field at a time and take 188: nine ints, then three
// {tagged position, simple array of two ints} and more that isn't known.
// Each position's tag is checked, as nothing else marks where they are.
//
//////////////////////////////////////////////////////////////////////////
static bool Read2009MeshFacesSeptember(COldDragonCursor* pcCursor, COldDragonMesh* pcMesh)
{
	size			uiElementSize;
	uint32			uiNum;
	uint32			i;
	int32			j;
	const uint8*	puiFace;
	int32			aiFace[25];
	SOldDragonFace	sFace;

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
		for (j = 0; j < 3; j++)
		{
			if (aiFace[j * 7 + 9] != OLD_DRAGON_TAG_INT)
			{
				return false;
			}
			sFace.aiPositions[j] = aiFace[j * 7 + 10];
		}
		pcMesh->macFaces.Add(&sFace);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool Read2009MeshFaces(COldDragonFile* pcFile, COldDragonCursor* pcCursor, COldDragonMesh* pcMesh)
{
	if (pcFile->GetVersion() == ODV_August2009)
	{
		return Read2009MeshFacesAugust(pcCursor, pcMesh);
	}
	return Read2009MeshFacesSeptember(pcCursor, pcMesh);
}


//////////////////////////////////////////////////////////////////////////
//
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
	if (!Read2009MeshFaces(pcFile, &cCursor, pcMesh))
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
// A material's and an instance's other fields aren't understood yet, so
// they're only named.
//
//////////////////////////////////////////////////////////////////////////
static bool Read2009Material(COldDragonCursor* pcCursor, COldDragonScene* pcScene)
{
	SOldDragonMaterial*	psMaterial;

	psMaterial = pcScene->AddMaterial();
	pcCursor->ReadTaggedName(psMaterial->szName, sizeof(psMaterial->szName));
	return !pcCursor->Failed();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool Read2009Instance(COldDragonCursor* pcCursor, COldDragonScene* pcScene)
{
	COldDragonInstance*	pcInstance;

	pcInstance = pcScene->AddInstance();
	pcCursor->ReadTaggedName(pcInstance->mszName, sizeof(pcInstance->mszName));
	return !pcCursor->Failed();
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


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
#include "BaseLib/Float4x4.h"
#include "BaseLib/Logger.h"
#include "OldDragonCursor.h"
#include "OldDragonReader.h"


//From IOX2021\code\CommonLibraries: ChunkFileNames.h, GraphicsHeaders.h, MeshStructures.h and each chunk's helper.  These
//were written straight from 32 bit structures, so what they hold is spelled out here rather than taken from a sizeof.
#define IOX_CHUNK_MESH			0x01
#define IOX_CHUNK_IMAGE			0x02
#define IOX_CHUNK_CAMERA		0x03
#define IOX_CHUNK_LIGHT			0x04
#define IOX_CHUNK_LINKS			0x05
#define IOX_CHUNK_MATERIAL		0x06

#define IOX_MAX_NAME			256
#define IOX_MAX_TEX				8
#define IOX_MAX_SAMPLE			16

#define IOX_VERT_SIZE			32		//Point3, then a CDynamicArray of the faces using it.
#define IOX_FACE_SIZE			112
#define IOX_COLOUR_SIZE			8
#define IOX_EDGE_SIZE			20
#define IOX_NORMAL_SIZE			12
#define IOX_UV_SIZE				104
#define IOX_CORNER_SIZE			24
#define IOX_INT_SIZE			4
#define IOX_STAGE_STATE_SIZE	24
#define IOX_SAMPLER_STATE_SIZE	24
#define IOX_LINK_NODE_SIZE		60		//CAxis, unique ID, type, the Max node it came from.
#define IOX_MAX_PATH			32


//////////////////////////////////////////////////////////////////////////
//
// A CDynamicArray written whole: number of elements, used elements,
// allocate size, stale pointer, element size.  Then the used elements.
//
//////////////////////////////////////////////////////////////////////////
static const uint8* ReadIOXDynamicArray(COldDragonCursor* pcCursor, size uiElementSize, size* puiUsed)
{
	uint32	uiUsed;
	uint32	uiElementSizeInFile;
	uint64	ulliByteSize;

	*puiUsed = 0;
	pcCursor->ReadUint32();
	uiUsed = pcCursor->ReadUint32();
	pcCursor->ReadUint32();
	pcCursor->ReadUint32();
	uiElementSizeInFile = pcCursor->ReadUint32();
	if (uiUsed == 0)
	{
		return NULL;
	}

	if (uiElementSizeInFile != uiElementSize)
	{
		pcCursor->Fail();
		return NULL;
	}
	ulliByteSize = (uint64)uiUsed * (uint64)uiElementSize;
	if (ulliByteSize > (uint64)pcCursor->Remaining())
	{
		pcCursor->Fail();
		return NULL;
	}
	*puiUsed = uiUsed;
	return pcCursor->ReadBytes((size)ulliByteSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void ReadIOXFace(const uint8* puiFace, SOldDragonFace* psFace)
{
	int32	aiFace[28];
	int32	i;

	memcpy(aiFace, puiFace, sizeof(aiFace));
	for (i = 0; i < 3; i++)
	{
		psFace->aiPositions[i] = aiFace[i * 6 + 0];
		psFace->aiUVs[i] = aiFace[i * 6 + 1];
		psFace->aiNormals[i] = aiFace[i * 6 + 2];
	}

	//Corners (18 ints), edges (3), normal (3), smoothing groups, number of textures, material, data.
	psFace->uiSmoothing = (uint32)aiFace[24];
	psFace->iMaterial = aiFace[26];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadIOXMesh(COldDragonFile* pcFile, SOldDragonChunk* psChunk, COldDragonScene* pcScene)
{
	COldDragonCursor	cCursor;
	COldDragonMesh*		pcMesh;
	const uint8*		puiArray;
	size				uiNum;
	size				uiNumVerts;
	size				i;
	size				uiIgnored;
	SOldDragonFace		sFace;
	SOldDragonVec3		sVec3;
	SOldDragonVec2		sVec2;
	float32				afVec3[3];

	cCursor.Init(pcFile->GetChunkData(psChunk), psChunk->uiDataSize);
	pcMesh = pcScene->AddMesh();

	cCursor.ReadName(pcMesh->mszName, sizeof(pcMesh->mszName), IOX_MAX_NAME);
	cCursor.ReadBytes(sizeof(int32) * 8);	//The counts, which are the arrays' used elements again.
	pcMesh->miID = cCursor.ReadInt32();
	cCursor.ReadBytes(sizeof(int32) * 5);	//SMeshMisc.

	puiArray = ReadIOXDynamicArray(&cCursor, IOX_VERT_SIZE, &uiNumVerts);
	for (i = 0; i < uiNumVerts; i++)
	{
		memcpy(&sVec3, &puiArray[i * IOX_VERT_SIZE], sizeof(SOldDragonVec3));
		pcMesh->macPositions.Add(&sVec3);
	}

	puiArray = ReadIOXDynamicArray(&cCursor, IOX_FACE_SIZE, &uiNum);
	for (i = 0; i < uiNum; i++)
	{
		ReadIOXFace(&puiArray[i * IOX_FACE_SIZE], &sFace);
		pcMesh->macFaces.Add(&sFace);
	}

	ReadIOXDynamicArray(&cCursor, IOX_COLOUR_SIZE, &uiIgnored);
	ReadIOXDynamicArray(&cCursor, IOX_EDGE_SIZE, &uiIgnored);

	puiArray = ReadIOXDynamicArray(&cCursor, IOX_NORMAL_SIZE, &uiNum);
	for (i = 0; i < uiNum; i++)
	{
		memcpy(&sVec3, &puiArray[i * IOX_NORMAL_SIZE], sizeof(SOldDragonVec3));
		pcMesh->macNormals.Add(&sVec3);
	}

	puiArray = ReadIOXDynamicArray(&cCursor, IOX_UV_SIZE, &uiNum);
	for (i = 0; i < uiNum; i++)
	{
		memcpy(afVec3, &puiArray[i * IOX_UV_SIZE], sizeof(afVec3));
		sVec2.x = afVec3[0];
		sVec2.y = afVec3[1];
		pcMesh->macUVs.Add(&sVec2);
	}

	ReadIOXDynamicArray(&cCursor, IOX_CORNER_SIZE, &uiIgnored);

	//Each vertex's faces.
	for (i = 0; i < uiNumVerts; i++)
	{
		ReadIOXDynamicArray(&cCursor, IOX_INT_SIZE, &uiIgnored);
	}

	if (cCursor.Failed() || (cCursor.Remaining() != 0))
	{
		return gcLogger.Error2(__METHOD__, " Mesh [", pcMesh->mszName, "] doesn't fill its chunk.", NULL);
	}
	return ValidateOldDragonMesh(pcMesh);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadIOXImage(COldDragonFile* pcFile, SOldDragonChunk* psChunk, COldDragonScene* pcScene)
{
	COldDragonCursor	cCursor;
	COldDragonImage*	pcImage;
	uint64				ulliByteSize;

	cCursor.Init(pcFile->GetChunkData(psChunk), psChunk->uiDataSize);
	pcImage = pcScene->AddImage();
	cCursor.ReadName(pcImage->mszName, sizeof(pcImage->mszName), IOX_MAX_NAME);
	pcImage->miWidth = cCursor.ReadInt32();
	pcImage->miHeight = cCursor.ReadInt32();
	pcImage->miID = cCursor.ReadInt32();
	pcImage->miChannels = 4;

	ulliByteSize = (uint64)(uint32)pcImage->miWidth * (uint64)(uint32)pcImage->miHeight * 4;
	if (cCursor.Failed() || (ulliByteSize != (uint64)cCursor.Remaining()))
	{
		return gcLogger.Error2(__METHOD__, " Image [", pcImage->mszName, "] doesn't fill its chunk.", NULL);
	}
	return pcImage->SetPixels(cCursor.ReadBytes((size)ulliByteSize), (size)ulliByteSize);
}


//////////////////////////////////////////////////////////////////////////
//
// Into Oblivion's colours are Direct3D's: 0xAARRGGBB.
//
//////////////////////////////////////////////////////////////////////////
static void ConvertIOXColour(uint32 uiColour, SOldDragonColour* psColour)
{
	psColour->a = (float32)((uiColour >> 24) & 0xff) / 255.0f;
	psColour->r = (float32)((uiColour >> 16) & 0xff) / 255.0f;
	psColour->g = (float32)((uiColour >> 8) & 0xff) / 255.0f;
	psColour->b = (float32)(uiColour & 0xff) / 255.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadIOXMaterial(COldDragonFile* pcFile, SOldDragonChunk* psChunk, COldDragonScene* pcScene)
{
	COldDragonCursor	cCursor;
	SOldDragonMaterial*	psMaterial;
	int32				i;

	cCursor.Init(pcFile->GetChunkData(psChunk), psChunk->uiDataSize);
	psMaterial = pcScene->AddMaterial();
	cCursor.ReadName(psMaterial->szName, sizeof(psMaterial->szName), IOX_MAX_NAME);
	psMaterial->fSpecularStrength = cCursor.ReadFloat();
	psMaterial->uiDiffuseColour = cCursor.ReadUint32();
	psMaterial->uiSpecularColour = cCursor.ReadUint32();
	ConvertIOXColour(psMaterial->uiDiffuseColour, &psMaterial->sDiffuse);
	ConvertIOXColour(psMaterial->uiDiffuseColour, &psMaterial->sAmbient);
	ConvertIOXColour(psMaterial->uiSpecularColour, &psMaterial->sSpecular);
	psMaterial->sEmissive.a = 1.0f;
	psMaterial->fShininessPower = psMaterial->fSpecularStrength;
	for (i = 0; i < IOX_MAX_SAMPLE; i++)
	{
		psMaterial->aiTextures[i] = cCursor.ReadInt32();
	}
	cCursor.ReadBytes(IOX_STAGE_STATE_SIZE * IOX_MAX_TEX);
	psMaterial->iNumTextures = cCursor.ReadInt32();
	cCursor.ReadBytes(IOX_SAMPLER_STATE_SIZE * IOX_MAX_SAMPLE);
	cCursor.ReadInt32();	//Number of samplers.
	cCursor.ReadInt32();	//Alpha.

	if (cCursor.Failed() || (cCursor.Remaining() != 0))
	{
		return gcLogger.Error2(__METHOD__, " Material [", psMaterial->szName, "] doesn't fill its chunk.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadIOXLight(COldDragonFile* pcFile, SOldDragonChunk* psChunk, COldDragonScene* pcScene)
{
	COldDragonCursor	cCursor;
	SOldDragonLight*	psLight;

	cCursor.Init(pcFile->GetChunkData(psChunk), psChunk->uiDataSize);
	psLight = pcScene->AddLight();
	cCursor.ReadName(psLight->szName, sizeof(psLight->szName), IOX_MAX_NAME);
	psLight->uiColour = cCursor.ReadUint32();
	ConvertIOXColour(psLight->uiColour, &psLight->sColour);
	psLight->fDecayStart = cCursor.ReadFloat();	//Atten0.
	cCursor.ReadFloat();						//Atten1.
	cCursor.ReadFloat();						//Atten2.
	psLight->iType = cCursor.ReadInt32();
	psLight->fRange = cCursor.ReadFloat();
	psLight->iID = cCursor.ReadInt32();

	if (cCursor.Failed() || (cCursor.Remaining() != 0))
	{
		return gcLogger.Error2(__METHOD__, " Light [", psLight->szName, "] doesn't fill its chunk.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadIOXCamera(COldDragonFile* pcFile, SOldDragonChunk* psChunk, COldDragonScene* pcScene)
{
	COldDragonCursor	cCursor;
	SOldDragonCamera*	psCamera;

	cCursor.Init(pcFile->GetChunkData(psChunk), psChunk->uiDataSize);
	psCamera = pcScene->AddCamera();
	cCursor.ReadName(psCamera->szName, sizeof(psCamera->szName), IOX_MAX_NAME);
	psCamera->fFOV = cCursor.ReadFloat();
	psCamera->fNear = cCursor.ReadFloat();
	psCamera->fFar = cCursor.ReadFloat();
	psCamera->iID = cCursor.ReadInt32();

	if (cCursor.Failed() || (cCursor.Remaining() != 0))
	{
		return gcLogger.Error2(__METHOD__, " Camera [", psCamera->szName, "] doesn't fill its chunk.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A node's path is which child it is at each level down from the root.
// The nodes come parent first, so a node's parent is the last one with
// the path less its last step, and the paths before it say where that is.
// paiSteps holds the last step of the last node at each level, and is
// updated for this node.  iPreviousLevel is the level of the node before,
// -1 for none.  False if this node's parent isn't there.
//
//////////////////////////////////////////////////////////////////////////
static bool ReadIOXPath(COldDragonCursor* pcCursor, uint32 uiPathSize, int32 iPreviousLevel, int32* paiSteps)
{
	uint32	k;
	int32	iStep;
	bool	bParentThere;

	bParentThere = (int32)uiPathSize <= iPreviousLevel + 1;
	for (k = 0; k < uiPathSize; k++)
	{
		iStep = pcCursor->ReadInt32();
		if (k + 1 < uiPathSize)
		{
			bParentThere = bParentThere && (iStep == paiSteps[k + 1]);
		}
		else
		{
			paiSteps[uiPathSize] = iStep;
		}
	}
	return bParentThere;
}


//////////////////////////////////////////////////////////////////////////
//
// A tree of nodes, each with the path to it from the root.  An axis has
// no w column in the file, so the matrix gets one.  It's the node's local
// matrix, and the game made a node's world matrix its parent's world matrix
// times its local one.
//
//////////////////////////////////////////////////////////////////////////
static bool ReadIOXLinks(COldDragonFile* pcFile, SOldDragonChunk* psChunk, COldDragonScene* pcScene)
{
	COldDragonCursor		cCursor;
	COldDragonInstance*		pcInstance;
	SOldDragonConnection*	psConnection;
	int32					iNumNodes;
	uint32					uiElementSize;
	int32					i;
	int32					j;
	int32					iRow;
	uint32					uiPathSize;
	int32					iPreviousLevel;
	int32					aiSteps[IOX_MAX_PATH + 1];
	int32					aiAtLevel[IOX_MAX_PATH + 1];
	int32					iConnection;
	char					szName[OLD_DRAGON_NAME_SIZE];

	cCursor.Init(pcFile->GetChunkData(psChunk), psChunk->uiDataSize);
	cCursor.ReadName(szName, sizeof(szName), IOX_MAX_NAME);

	pcInstance = pcScene->AddInstance();
	strcpy(pcInstance->mszName, szName);
	pcInstance->miID = cCursor.ReadInt32();
	cCursor.ReadInt32();	//Data.

	//The CTree: stale root pointer, level, number of nodes, node size.
	cCursor.ReadUint32();
	cCursor.ReadInt32();
	iNumNodes = cCursor.ReadInt32();
	uiElementSize = cCursor.ReadUint32();
	if ((iNumNodes > 0) && (uiElementSize != IOX_LINK_NODE_SIZE))
	{
		cCursor.Fail();
	}

	iPreviousLevel = -1;
	for (i = 0; (i < iNumNodes) && !cCursor.Failed(); i++)
	{
		uiPathSize = cCursor.ReadUint32();
		if (uiPathSize > IOX_MAX_PATH)
		{
			cCursor.Fail();
			break;
		}
		if (!ReadIOXPath(&cCursor, uiPathSize, iPreviousLevel, aiSteps))
		{
			return gcLogger.Error2(__METHOD__, " Links [", szName, "] have a node whose parent isn't before it.", NULL);
		}

		iConnection = (int32)pcScene->macConnections.NumElements();
		psConnection = pcScene->AddConnection();
		strcpy(psConnection->szName, szName);
		for (iRow = 0; iRow < 4; iRow++)
		{
			for (j = 0; j < 3; j++)
			{
				psConnection->sLocal.af[iRow * 4 + j] = cCursor.ReadFloat();
			}
			psConnection->sLocal.af[iRow * 4 + 3] = (iRow == 3) ? 1.0f : 0.0f;
		}
		if (uiPathSize == 0)
		{
			psConnection->sWorld = psConnection->sLocal;
		}
		else
		{
			psConnection->iParent = aiAtLevel[uiPathSize - 1];
			Float4x4Multiply((SFloat4x4*)psConnection->sWorld.af, (SFloat4x4*)pcScene->macConnections.Get((size)psConnection->iParent)->sWorld.af, (SFloat4x4*)psConnection->sLocal.af);
		}
		aiAtLevel[uiPathSize] = iConnection;
		iPreviousLevel = (int32)uiPathSize;
		psConnection->iID = cCursor.ReadInt32();
		psConnection->iObjectType = cCursor.ReadInt32();
		cCursor.ReadUint32();	//The stale pointer to the Max node.
		psConnection->bTopLevel = (uiPathSize == 0);
		pcInstance->maiConnections.Add(&iConnection);
	}

	if (cCursor.Failed() || (cCursor.Remaining() != 0))
	{
		return gcLogger.Error2(__METHOD__, " Links [", szName, "] don't fill their chunk.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ReadOldDragonIOX(COldDragonFile* pcFile, COldDragonScene* pcScene)
{
	size				i;
	SOldDragonChunk*	psChunk;
	bool				bResult;

	bResult = true;
	for (i = 1; (i < pcFile->NumChunks()) && bResult; i++)
	{
		psChunk = pcFile->GetChunk(i);
		switch (psChunk->uiName)
		{
		case IOX_CHUNK_MESH:
			bResult = ReadIOXMesh(pcFile, psChunk, pcScene);
			break;
		case IOX_CHUNK_IMAGE:
			bResult = ReadIOXImage(pcFile, psChunk, pcScene);
			break;
		case IOX_CHUNK_CAMERA:
			bResult = ReadIOXCamera(pcFile, psChunk, pcScene);
			break;
		case IOX_CHUNK_LIGHT:
			bResult = ReadIOXLight(pcFile, psChunk, pcScene);
			break;
		case IOX_CHUNK_LINKS:
			bResult = ReadIOXLinks(pcFile, psChunk, pcScene);
			break;
		case IOX_CHUNK_MATERIAL:
			bResult = ReadIOXMaterial(pcFile, psChunk, pcScene);
			break;
		default:
			pcScene->muiSkippedChunks++;
			break;
		}
	}
	return bResult;
}


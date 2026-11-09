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


#define MAX_MATRIX_SIZE				64
#define MAX_KEY_FRAME_SIZE			72		//An int nobody has identified, the matrix, the time.
#define MAX_POSITION_SIZE			12
#define MAX_FACE_SIZE				24		//Three corners, three edges.
#define MAX_MATERIAL_PROPERTIES		128
#define MAX_MATERIAL_IMAGE			28		//Image ID, then how it's addressed and filtered.
#define MAX_COLOUR_OBJECT_SIZE		28
#define MAX_COLOUR_OBJECT_HEADER	12


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void ReadMaxMatrix(COldDragonCursor* pcCursor, SOldDragonMatrix* psMatrix)
{
	const uint8*	puiData;
	size			uiSize;

	puiData = pcCursor->ReadTaggedData(&uiSize);
	if ((puiData == NULL) || (uiSize != MAX_MATRIX_SIZE))
	{
		pcCursor->Fail();
		return;
	}
	memcpy(psMatrix, puiData, MAX_MATRIX_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadMaxConnection(COldDragonCursor* pcCursor, COldDragonScene* pcScene)
{
	SOldDragonConnection*	psConnection;

	psConnection = pcScene->AddConnection();
	psConnection->iID = pcCursor->ReadTaggedInt();
	pcCursor->ReadTaggedChars(psConnection->szName, sizeof(psConnection->szName));
	ReadMaxMatrix(pcCursor, &psConnection->sWorld);
	ReadMaxMatrix(pcCursor, &psConnection->sLocal);
	psConnection->bSelected = pcCursor->ReadTaggedInt() != 0;
	psConnection->bTopLevel = pcCursor->ReadTaggedInt() != 0;
	psConnection->bParity = pcCursor->ReadTaggedInt() != 0;
	return !pcCursor->Failed() && (pcCursor->Remaining() == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadMaxSequence(COldDragonCursor* pcCursor, COldDragonScene* pcScene)
{
	COldDragonSequence*	pcSequence;
	const uint8*		puiData;
	size				uiSize;
	size				uiNum;
	size				i;
	SOldDragonKeyFrame*	psKeyFrame;

	pcSequence = pcScene->AddSequence();
	pcSequence->miID = pcCursor->ReadTaggedInt();
	pcCursor->ReadTaggedChars(pcSequence->mszName, sizeof(pcSequence->mszName));

	puiData = pcCursor->ReadTaggedData(&uiSize);
	if ((puiData == NULL) || (uiSize != sizeof(int32)))
	{
		return false;
	}
	memcpy(&pcSequence->miConnection, puiData, sizeof(int32));

	puiData = pcCursor->ReadTaggedArray(&uiSize, &uiNum);
	if (pcCursor->Failed() || (uiSize != MAX_KEY_FRAME_SIZE))
	{
		return false;
	}
	for (i = 0; i < uiNum; i++)
	{
		psKeyFrame = pcSequence->macKeyFrames.Add();
		memcpy(&psKeyFrame->sMatrix, &puiData[i * MAX_KEY_FRAME_SIZE + sizeof(int32)], MAX_MATRIX_SIZE);
		memcpy(&psKeyFrame->fTime, &puiData[i * MAX_KEY_FRAME_SIZE + sizeof(int32) + MAX_MATRIX_SIZE], sizeof(float32));
	}
	return pcCursor->Remaining() == 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A colour object written whole: three words of object, then r, g, b, a.
//
//////////////////////////////////////////////////////////////////////////
static void ReadMaxColour(const uint8* puiColour, SOldDragonColour* psColour)
{
	memcpy(psColour, &puiColour[MAX_COLOUR_OBJECT_HEADER], sizeof(SOldDragonColour));
}


//////////////////////////////////////////////////////////////////////////
//
// Four colours, two shininesses, then lit and opacity type.  The exporter
// set the opacity after the colour with sDiffuseColour.a, but a is in the
// same place r gets written, so the diffuse colour's red is really the
// opacity.  Max's own ambient and diffuse reds are the same in every
// material in Melee.max, so the red is taken back from the ambient colour.
//
//////////////////////////////////////////////////////////////////////////
static void ReadMaxMaterialProperties(const uint8* puiProperties, SOldDragonMaterial* psMaterial)
{
	float32	fOpacity;

	ReadMaxColour(&puiProperties[MAX_COLOUR_OBJECT_SIZE * 0], &psMaterial->sDiffuse);
	ReadMaxColour(&puiProperties[MAX_COLOUR_OBJECT_SIZE * 1], &psMaterial->sAmbient);
	ReadMaxColour(&puiProperties[MAX_COLOUR_OBJECT_SIZE * 2], &psMaterial->sEmissive);
	ReadMaxColour(&puiProperties[MAX_COLOUR_OBJECT_SIZE * 3], &psMaterial->sSpecular);

	fOpacity = psMaterial->sDiffuse.r;
	psMaterial->sDiffuse.r = psMaterial->sAmbient.r;
	psMaterial->sDiffuse.a = fOpacity;

	memcpy(&psMaterial->fShininessPower, &puiProperties[MAX_COLOUR_OBJECT_SIZE * 4], sizeof(float32));
	memcpy(&psMaterial->fShininessBase, &puiProperties[MAX_COLOUR_OBJECT_SIZE * 4 + sizeof(float32)], sizeof(float32));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadMaxMaterial(COldDragonCursor* pcCursor, COldDragonScene* pcScene)
{
	SOldDragonMaterial*	psMaterial;
	const uint8*		puiData;
	size				uiSize;
	int32				iImage;

	psMaterial = pcScene->AddMaterial();
	psMaterial->iID = pcCursor->ReadTaggedInt();
	pcCursor->ReadTaggedChars(psMaterial->szName, sizeof(psMaterial->szName));

	puiData = pcCursor->ReadTaggedData(&uiSize);
	if ((puiData == NULL) || (uiSize != MAX_MATERIAL_PROPERTIES))
	{
		return false;
	}
	ReadMaxMaterialProperties(puiData, psMaterial);

	//Diffuse, diffuse level, specular, illumination, opacity, bump, detail, decal and reflection.
	while (pcCursor->Remaining() > 0)
	{
		puiData = pcCursor->ReadTaggedData(&uiSize);
		if ((puiData == NULL) || (uiSize != MAX_MATERIAL_IMAGE) || (psMaterial->iNumTextures >= OLD_DRAGON_MAX_TEXTURES))
		{
			return false;
		}
		memcpy(&iImage, puiData, sizeof(int32));
		psMaterial->aiTextures[psMaterial->iNumTextures] = iImage;
		psMaterial->iNumTextures++;
	}
	return !pcCursor->Failed();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadMaxLight(COldDragonCursor* pcCursor, COldDragonScene* pcScene)
{
	SOldDragonLight*	psLight;
	const uint8*		puiColour;
	size				uiSize;

	psLight = pcScene->AddLight();
	psLight->iID = pcCursor->ReadTaggedInt();
	pcCursor->ReadTaggedChars(psLight->szName, sizeof(psLight->szName));
	psLight->iType = pcCursor->ReadTaggedInt();
	puiColour = pcCursor->ReadTaggedData(&uiSize);
	if ((puiColour == NULL) || (uiSize != MAX_COLOUR_OBJECT_SIZE))
	{
		return false;
	}
	ReadMaxColour(puiColour, &psLight->sColour);
	pcCursor->ReadTaggedInt();				//Decay type.
	psLight->fDecayStart = pcCursor->ReadTaggedFloat();
	psLight->bShadows = pcCursor->ReadTaggedBool();
	psLight->fIntensity = pcCursor->ReadTaggedFloat();
	return !pcCursor->Failed() && (pcCursor->Remaining() == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadMaxCamera(COldDragonCursor* pcCursor, COldDragonScene* pcScene)
{
	SOldDragonCamera*	psCamera;

	psCamera = pcScene->AddCamera();
	psCamera->iID = pcCursor->ReadTaggedInt();
	pcCursor->ReadTaggedChars(psCamera->szName, sizeof(psCamera->szName));
	psCamera->iType = pcCursor->ReadTaggedInt();
	psCamera->fFOV = pcCursor->ReadTaggedFloat();
	return !pcCursor->Failed() && (pcCursor->Remaining() == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadMaxInstance(COldDragonCursor* pcCursor, COldDragonScene* pcScene)
{
	COldDragonInstance*	pcInstance;
	const uint8*		puiData;
	size				uiNum;
	size				i;
	int32				iConnection;

	pcInstance = pcScene->AddInstance();
	pcInstance->miID = pcCursor->ReadTaggedInt();
	pcCursor->ReadTaggedChars(pcInstance->mszName, sizeof(pcInstance->mszName));
	pcInstance->miType = pcCursor->ReadTaggedInt();
	pcInstance->miObject = pcCursor->ReadTaggedInt();
	puiData = pcCursor->ReadTaggedSimple(sizeof(int32), &uiNum);
	if (pcCursor->Failed())
	{
		return false;
	}
	for (i = 0; i < uiNum; i++)
	{
		memcpy(&iConnection, &puiData[i * sizeof(int32)], sizeof(int32));
		pcInstance->maiConnections.Add(&iConnection);
	}
	return pcCursor->Remaining() == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadMaxImage(COldDragonCursor* pcCursor, COldDragonScene* pcScene)
{
	COldDragonImage*	pcImage;

	pcImage = pcScene->AddImage();
	pcImage->miID = pcCursor->ReadTaggedInt();
	pcCursor->ReadTaggedChars(pcImage->mszName, sizeof(pcImage->mszName));
	pcImage->miWidth = pcCursor->ReadTaggedInt();
	pcImage->miHeight = pcCursor->ReadTaggedInt();
	if (pcCursor->Failed())
	{
		return false;
	}
	return ReadOldDragonImagePixels(pcCursor, pcImage);
}


//////////////////////////////////////////////////////////////////////////
//
// Only the geometry at the start of a mesh is read.  What comes after
// the faces hasn't been worked out, so the chunk isn't read to its end.
//
// In Melee.DRG every mesh stops four bytes after the header of its corner
// array, with the next mesh starting right there.  It looks like saving
// the first corner failed and the exporter, which ignores what Save
// returns, carried on.  Either way an unclosed mesh has positions and no
// faces, because that's all that was ever written.
//
//////////////////////////////////////////////////////////////////////////
static bool ReadMaxMesh(COldDragonCursor* pcCursor, COldDragonScene* pcScene, bool bUnclosed)
{
	COldDragonMesh*	pcMesh;
	const uint8*	puiData;
	size			uiElementSize;
	size			uiNum;
	size			i;
	SOldDragonVec3	sPosition;
	SOldDragonFace	sFace;
	int32			aiFace[6];
	int32			j;

	pcMesh = pcScene->AddMesh();
	pcMesh->miID = pcCursor->ReadTaggedInt();
	pcCursor->ReadTaggedChars(pcMesh->mszName, sizeof(pcMesh->mszName));

	puiData = pcCursor->ReadTaggedArray(&uiElementSize, &uiNum);
	if (pcCursor->Failed() || (uiElementSize != MAX_POSITION_SIZE))
	{
		return false;
	}
	for (i = 0; i < uiNum; i++)
	{
		memcpy(&sPosition, &puiData[i * MAX_POSITION_SIZE], MAX_POSITION_SIZE);
		pcMesh->macPositions.Add(&sPosition);
	}

	if (bUnclosed)
	{
		return true;
	}

	//The corners, and the array each corner keeps of its edges and faces.
	pcCursor->ReadTaggedArray(&uiElementSize, &uiNum);
	while (pcCursor->PeekTag() == OLD_DRAGON_TAG_SIMPLE)
	{
		pcCursor->ReadTaggedSimple(sizeof(int32), &uiNum);
	}

	//The edges.
	pcCursor->ReadTaggedArray(&uiElementSize, &uiNum);

	puiData = pcCursor->ReadTaggedArray(&uiElementSize, &uiNum);
	if (pcCursor->Failed() || (uiElementSize != MAX_FACE_SIZE))
	{
		return false;
	}
	for (i = 0; i < uiNum; i++)
	{
		memcpy(aiFace, &puiData[i * MAX_FACE_SIZE], MAX_FACE_SIZE);
		for (j = 0; j < 3; j++)
		{
			sFace.aiPositions[j] = aiFace[j];
			sFace.aiNormals[j] = OLD_DRAGON_NO_INDEX;
			sFace.aiUVs[j] = OLD_DRAGON_NO_INDEX;
		}
		sFace.iMaterial = OLD_DRAGON_NO_INDEX;
		sFace.uiSmoothing = 0;
		pcMesh->macFaces.Add(&sFace);
	}
	return ValidateOldDragonMesh(pcMesh);
}


//////////////////////////////////////////////////////////////////////////
//
// A chunk left unclosed has no name, and only meshes were ever left open
// in the files that exist.  It's still only a mesh if it reads as one.
//
//////////////////////////////////////////////////////////////////////////
static bool ReadMaxChunk(COldDragonFile* pcFile, SOldDragonChunk* psChunk, COldDragonScene* pcScene)
{
	COldDragonCursor	cCursor;
	const char*			szName;

	cCursor.Init(pcFile->GetChunkData(psChunk), psChunk->uiDataSize);
	szName = pcFile->GetChunkName(psChunk);
	if ((szName == NULL) && psChunk->bUnclosed)
	{
		return ReadMaxMesh(&cCursor, pcScene, true);
	}
	if (szName == NULL)
	{
		return false;
	}

	if (strcmp(szName, "CConnection") == 0)
	{
		return ReadMaxConnection(&cCursor, pcScene);
	}
	if (strcmp(szName, "CSequence") == 0)
	{
		return ReadMaxSequence(&cCursor, pcScene);
	}
	if (strcmp(szName, "CMaterial") == 0)
	{
		return ReadMaxMaterial(&cCursor, pcScene);
	}
	if (strcmp(szName, "CImage") == 0)
	{
		return ReadMaxImage(&cCursor, pcScene);
	}
	if (strcmp(szName, "CMesh") == 0)
	{
		return ReadMaxMesh(&cCursor, pcScene, false);
	}
	if (strcmp(szName, "CLight") == 0)
	{
		return ReadMaxLight(&cCursor, pcScene);
	}
	if (strcmp(szName, "CCamera") == 0)
	{
		return ReadMaxCamera(&cCursor, pcScene);
	}
	if (strcmp(szName, "CInstance") == 0)
	{
		return ReadMaxInstance(&cCursor, pcScene);
	}
	pcScene->muiSkippedChunks++;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ReadOldDragonMax(COldDragonFile* pcFile, COldDragonScene* pcScene)
{
	size				i;
	SOldDragonChunk*	psChunk;
	const char*			szName;

	for (i = 1; i < pcFile->NumChunks(); i++)
	{
		psChunk = pcFile->GetChunk(i);
		if (!ReadMaxChunk(pcFile, psChunk, pcScene))
		{
			szName = pcFile->GetChunkName(psChunk);
			return gcLogger.Error2(__METHOD__, " Could not read chunk [", (szName != NULL) ? szName : "unnamed", "].", NULL);
		}
	}
	return true;
}


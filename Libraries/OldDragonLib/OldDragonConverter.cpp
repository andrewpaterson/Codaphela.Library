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
#include <math.h>
#include <string.h>
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayTemplatePrimitive.h"
#include "BaseLib/Float4x4.h"
#include "BaseLib/Logger.h"
#include "SupportLib/SceneFileWriter.h"
#include "OldDragonFile.h"
#include "OldDragonConverter.h"


#define OLD_DRAGON_MAX_INSTANCE_MESH	1
#define OLD_DRAGON_MAX_INSTANCE_LIGHT	2
#define OLD_DRAGON_MAX_INSTANCE_CAMERA	3

#define OLD_DRAGON_DEGREES_TO_RADIANS	(3.14159265358979 / 180.0)
#define OLD_DRAGON_FLOATS_IN_MATRIX		16
#define OLD_DRAGON_PARENT_TOLERANCE		0.001f


typedef CArrayTemplatePrimitive<float32>	CArrayConvertFloats;
typedef CArrayTemplatePrimitive<uint32>	CArrayConvertUints;
typedef CArrayTemplatePrimitive<uint8>	CArrayConvertBytes;


//How a connection's matrices are turned into what a scene file means by them.
enum EOldDragonAim
{
	OLDA_AsItIs,
	OLDA_Exporter,		//A camera looks along its -y with -z up, and a light shines along its -y.
	OLDA_IOXCamera,		//Its x is forward, y is up and z is left, which mirrors it.  Its game put a view of its own in it when it ran.
};


enum EOldDragonPlacedKind
{
	OLDPK_Mesh,
	OLDPK_Light,
	OLDPK_Camera,
};


//Something placed in a scene, however the version it came from said so.
struct SOldDragonPlaced
{
	EOldDragonPlacedKind	eKind;
	int32					iObject;
	size					uiInstance;		//In macInstances, with iConnection being SCENE_FILE_NONE, or
	int32					iConnection;	//the connection that places it in Into Oblivion, and uiInstance isn't used.
};


typedef CArrayTemplate<SOldDragonPlaced>	CArrayOldDragonPlaced;


//A mesh as it is in a scene file.
class COldDragonFlatMesh
{
public:
	CArrayInt			maiPolygonSizes;
	CArrayInt			maiCornerPositions;
	CArrayInt			maiCornerNormals;
	CArrayInt			maiCornerUVs;
	CArrayInt			maiMaterialSlots;
	CArrayInt			maiPolygonSlots;
	CArrayConvertUints	mauiSmoothGroups;
	CArrayInt			maiSkinCounts;
	CArrayInt			maiSkinMatrices;
	CArrayConvertFloats	mafSkinWeights;
	bool				mbHasNormals;
	bool				mbHasUVs;
	bool				mbHasSlots;
	bool				mbHasSmoothGroups;

	void	Init(COldDragonMesh* pcMesh);
	void	Kill(void);

protected:
	int32	FindSlot(int32 iMaterial);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 COldDragonFlatMesh::FindSlot(int32 iMaterial)
{
	size	i;
	size	uiNumSlots;

	if (iMaterial == OLD_DRAGON_NO_INDEX)
	{
		return SCENE_FILE_NONE;
	}

	uiNumSlots = maiMaterialSlots.NumElements();
	for (i = 0; i < uiNumSlots; i++)
	{
		if (maiMaterialSlots.GetValue(i) == iMaterial)
		{
			return (int32)i;
		}
	}
	maiMaterialSlots.Add(iMaterial);
	return (int32)uiNumSlots;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COldDragonFlatMesh::Init(COldDragonMesh* pcMesh)
{
	SOldDragonFace*			psFace;
	SOldDragonSkinVert*		psSkinVert;
	SOldDragonSkinWeight*	psWeight;
	size					i;
	int32					j;

	maiPolygonSizes.Init();
	maiCornerPositions.Init();
	maiCornerNormals.Init();
	maiCornerUVs.Init();
	maiMaterialSlots.Init();
	maiPolygonSlots.Init();
	mauiSmoothGroups.Init();
	maiSkinCounts.Init();
	maiSkinMatrices.Init();
	mafSkinWeights.Init();
	mbHasNormals = false;
	mbHasUVs = false;
	mbHasSlots = false;
	mbHasSmoothGroups = false;

	for (i = 0; i < pcMesh->macFaces.NumElements(); i++)
	{
		psFace = pcMesh->macFaces.Get(i);
		maiPolygonSizes.Add(3);
		for (j = 0; j < 3; j++)
		{
			maiCornerPositions.Add(psFace->aiPositions[j]);
			maiCornerNormals.Add(psFace->aiNormals[j]);
			maiCornerUVs.Add(psFace->aiUVs[j]);
			mbHasNormals |= psFace->aiNormals[j] != OLD_DRAGON_NO_INDEX;
			mbHasUVs |= psFace->aiUVs[j] != OLD_DRAGON_NO_INDEX;
		}
		maiPolygonSlots.Add(FindSlot(psFace->iMaterial));
		mbHasSlots |= psFace->iMaterial != OLD_DRAGON_NO_INDEX;
		mauiSmoothGroups.Add(psFace->uiSmoothing);
		mbHasSmoothGroups |= psFace->uiSmoothing != 0;
	}

	for (i = 0; i < pcMesh->macSkinVerts.NumElements(); i++)
	{
		psSkinVert = pcMesh->macSkinVerts.Get(i);
		maiSkinCounts.Add(psSkinVert->iNumWeights);
		for (j = 0; j < psSkinVert->iNumWeights; j++)
		{
			psWeight = pcMesh->macSkinWeights.Get((size)(psSkinVert->iFirstWeight + j));
			maiSkinMatrices.Add(psWeight->iMatrix);
			mafSkinWeights.Add(psWeight->fWeight);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void GetInverseMatrices(COldDragonMesh* pcMesh, CArrayConvertFloats* paf4x4s)
{
	size	i;

	for (i = 0; i < pcMesh->macInverseSkinMatrices.NumElements(); i++)
	{
		paf4x4s->Add(pcMesh->macInverseSkinMatrices.Get(i)->af, OLD_DRAGON_FLOATS_IN_MATRIX);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COldDragonFlatMesh::Kill(void)
{
	mafSkinWeights.Kill();
	maiSkinMatrices.Kill();
	maiSkinCounts.Kill();
	mauiSmoothGroups.Kill();
	maiPolygonSlots.Kill();
	maiMaterialSlots.Kill();
	maiCornerUVs.Kill();
	maiCornerNormals.Kill();
	maiCornerPositions.Kill();
	maiPolygonSizes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Into Oblivion's link nodes say what they place.  In other versions
// instances do, with numbers that differ between them.
//
//////////////////////////////////////////////////////////////////////////
static bool FindOldDragonPlaced(COldDragonScene* pcScene, CArrayOldDragonPlaced* pasPlaced)
{
	SOldDragonPlaced		sPlaced;
	SOldDragonConnection*	psConnection;
	COldDragonInstance*		pcInstance;
	size					i;

	if (pcScene->meVersion == ODV_IOX)
	{
		for (i = 0; i < pcScene->macConnections.NumElements(); i++)
		{
			psConnection = pcScene->macConnections.Get(i);
			if ((psConnection->iObjectType != OIOX_Mesh) && (psConnection->iObjectType != OIOX_Light) && (psConnection->iObjectType != OIOX_Camera))
			{
				continue;
			}
			sPlaced.eKind = (psConnection->iObjectType == OIOX_Mesh) ? OLDPK_Mesh : ((psConnection->iObjectType == OIOX_Light) ? OLDPK_Light : OLDPK_Camera);
			sPlaced.iObject = psConnection->iID;
			sPlaced.uiInstance = 0;
			sPlaced.iConnection = (int32)i;
			pasPlaced->Add(&sPlaced);
		}
		return true;
	}

	for (i = 0; i < pcScene->macInstances.NumElements(); i++)
	{
		pcInstance = pcScene->macInstances.Get(i);
		if ((pcInstance->miType == OLD_DRAGON_2009_INSTANCE_MESH) || (pcInstance->miType == OLD_DRAGON_MAX_INSTANCE_MESH))
		{
			sPlaced.eKind = OLDPK_Mesh;
		}
		else if (pcInstance->miType == OLD_DRAGON_MAX_INSTANCE_LIGHT)
		{
			sPlaced.eKind = OLDPK_Light;
		}
		else if ((pcInstance->miType == OLD_DRAGON_2009_INSTANCE_CAMERA) || (pcInstance->miType == OLD_DRAGON_MAX_INSTANCE_CAMERA))
		{
			sPlaced.eKind = OLDPK_Camera;
		}
		else
		{
			return gcLogger.Error2(__METHOD__, " An instance is of a type that isn't known [", IntToString(pcInstance->miType), "].", NULL);
		}
		sPlaced.iObject = pcInstance->miObject;
		sPlaced.uiInstance = i;
		sPlaced.iConnection = SCENE_FILE_NONE;
		pasPlaced->Add(&sPlaced);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// SCENE_FILE_NONE if there's no connection with this name, or more than one.
//
//////////////////////////////////////////////////////////////////////////
static int32 FindConnectionNamed(COldDragonScene* pcScene, const char* szName)
{
	size	i;
	int32	iFound;

	iFound = SCENE_FILE_NONE;
	for (i = 0; i < pcScene->macConnections.NumElements(); i++)
	{
		if (strcmp(pcScene->macConnections.Get(i)->szName, szName) == 0)
		{
			if (iFound != SCENE_FILE_NONE)
			{
				return SCENE_FILE_NONE;
			}
			iFound = (int32)i;
		}
	}
	return iFound;
}


//////////////////////////////////////////////////////////////////////////
//
// A sequence's name says which connection it moves.  Its number doesn't: see
// GetNumberedConnection.
//
//////////////////////////////////////////////////////////////////////////
static int32 GetSequenceConnection(COldDragonScene* pcScene, COldDragonSequence* pcSequence)
{
	return FindConnectionNamed(pcScene, pcSequence->mszName);
}


//////////////////////////////////////////////////////////////////////////
//
// Instances and sequences number connections the same way, and in Goober
// that isn't their place in the scene's list: its IK chains and finger
// helpers are in the list but not counted.  So a number is the connection of
// the sequence with that number.  If there isn't one nothing moves it, as
// with Melee's cameras, and all such in the files converted are numbered
// by their place.
//
//////////////////////////////////////////////////////////////////////////
static int32 GetNumberedConnection(COldDragonScene* pcScene, int32 iNumber)
{
	COldDragonSequence*	pcSequence;
	size				i;

	for (i = 0; i < pcScene->macSequences.NumElements(); i++)
	{
		pcSequence = pcScene->macSequences.Get(i);
		if (pcSequence->miConnection == iNumber)
		{
			return GetSequenceConnection(pcScene, pcSequence);
		}
	}
	return iNumber;
}


//////////////////////////////////////////////////////////////////////////
//
// The connection a light or camera is attached to, and the first of an
// instance's if it has more.
//
//////////////////////////////////////////////////////////////////////////
static int32 GetPlacedConnection(COldDragonScene* pcScene, SOldDragonPlaced* psPlaced)
{
	COldDragonInstance*	pcInstance;

	if (psPlaced->iConnection != SCENE_FILE_NONE)
	{
		return psPlaced->iConnection;
	}
	pcInstance = pcScene->macInstances.Get(psPlaced->uiInstance);
	if (pcInstance->maiConnections.NumElements() == 0)
	{
		return SCENE_FILE_NONE;
	}
	return GetNumberedConnection(pcScene, *pcInstance->maiConnections.Get(0));
}


//////////////////////////////////////////////////////////////////////////
//
// A placed thing's name: the old instances have none.  Its connection has
// the name of what's there.
//
//////////////////////////////////////////////////////////////////////////
static const char* GetPlacedName(COldDragonScene* pcScene, SOldDragonPlaced* psPlaced)
{
	int32	iConnection;

	iConnection = GetPlacedConnection(pcScene, psPlaced);
	if ((iConnection >= 0) && ((size)iConnection < pcScene->macConnections.NumElements()))
	{
		return pcScene->macConnections.Get((size)iConnection)->szName;
	}
	return "";
}


//////////////////////////////////////////////////////////////////////////
//
// The connections and slots an instance has, for one mesh.  In Into Oblivion
// that's the one connection, and in others the instance's, whose order is
// what a skinned mesh's weights index.
//
//////////////////////////////////////////////////////////////////////////
static void GetInstanceConnections(COldDragonScene* pcScene, SOldDragonPlaced* psPlaced, CArrayInt* paiConnections, CArrayInt* paiSlots)
{
	COldDragonInstance*	pcInstance;
	size				i;

	if (psPlaced->iConnection != SCENE_FILE_NONE)
	{
		paiConnections->Add(psPlaced->iConnection);
	}
	else
	{
		pcInstance = pcScene->macInstances.Get(psPlaced->uiInstance);
		for (i = 0; i < pcInstance->maiConnections.NumElements(); i++)
		{
			paiConnections->Add(GetNumberedConnection(pcScene, *pcInstance->maiConnections.Get(i)));
		}
	}
	for (i = 0; i < paiConnections->NumElements(); i++)
	{
		paiSlots->Add((int32)i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How to turn the matrices of this connection, by what's attached to it.
//
//////////////////////////////////////////////////////////////////////////
static EOldDragonAim GetConnectionAim(COldDragonScene* pcScene, CArrayOldDragonPlaced* pasPlaced, int32 iConnection)
{
	SOldDragonPlaced*	psPlaced;
	size				i;

	for (i = 0; i < pasPlaced->NumElements(); i++)
	{
		psPlaced = pasPlaced->Get(i);
		if ((psPlaced->eKind != OLDPK_Mesh) && (GetPlacedConnection(pcScene, psPlaced) == iConnection))
		{
			return ((pcScene->meVersion == ODV_IOX) && (psPlaced->eKind == OLDPK_Camera)) ? OLDA_IOXCamera : OLDA_Exporter;
		}
	}
	return OLDA_AsItIs;
}


//////////////////////////////////////////////////////////////////////////
//
// A camera looks along its -z with y up, and a light shines along its -z.
// Subtracting from 0 doesn't turn a 0 into a -0.
//
//////////////////////////////////////////////////////////////////////////
static void AimMatrix(float32* pafDest, const float32* pafSource, EOldDragonAim eAim)
{
	int32	j;

	memcpy(pafDest, pafSource, sizeof(float32) * OLD_DRAGON_FLOATS_IN_MATRIX);
	for (j = 0; j < 3; j++)
	{
		if (eAim == OLDA_Exporter)
		{
			pafDest[4 + j] = 0.0f - pafSource[8 + j];
			pafDest[8 + j] = pafSource[4 + j];
		}
		else if (eAim == OLDA_IOXCamera)
		{
			pafDest[0 + j] = 0.0f - pafSource[8 + j];
			pafDest[8 + j] = 0.0f - pafSource[0 + j];
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// False if the channels aren't known.
//
//////////////////////////////////////////////////////////////////////////
static bool GetImagePixels(COldDragonScene* pcScene, COldDragonImage* pcImage, CArrayConvertBytes* pauiPixels, const char** pszChannels)
{
	size	uiNumPixels;
	size	uiNumBytes;
	size	i;
	uint8*	puiPixels;
	uint8	uiBlue;

	switch (pcImage->miChannels)
	{
	case 1:
		*pszChannels = "A";
		break;
	case 3:
		*pszChannels = "RGB";
		break;
	case 4:
		*pszChannels = "RGBA";
		break;
	default:
		return gcLogger.Error2(__METHOD__, " Image [", pcImage->mszName, "] has a number of channels that isn't known.", NULL);
	}
	if (pcImage->mpuiPixels == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Image [", pcImage->mszName, "] has no pixels.", NULL);
	}

	uiNumPixels = (size)pcImage->miWidth * (size)pcImage->miHeight;
	uiNumBytes = uiNumPixels * (size)pcImage->miChannels;
	pauiPixels->Resize(uiNumBytes);
	puiPixels = pauiPixels->GetData();
	memcpy(puiPixels, pcImage->mpuiPixels, uiNumBytes);

	if ((pcImage->miChannels >= 3) && (pcScene->meVersion != ODV_Max))
	{
		for (i = 0; i < uiNumPixels; i++)
		{
			uiBlue = puiPixels[i * (size)pcImage->miChannels];
			puiPixels[i * (size)pcImage->miChannels] = puiPixels[i * (size)pcImage->miChannels + 2];
			puiPixels[i * (size)pcImage->miChannels + 2] = uiBlue;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Times from the start of the animation, and each key's matrix.  The old
// files repeat the last key with no time after it.  Any other key with no
// time after it isn't understood, and fails.
//
//////////////////////////////////////////////////////////////////////////
static bool GetTrackKeys(COldDragonSequence* pcSequence, EOldDragonAim eAim, CArrayConvertFloats* pafTimes, CArrayConvertFloats* paf4x4s)
{
	SOldDragonKeyFrame*	psKey;
	SOldDragonKeyFrame*	psPrevious;
	float32				af4x4[OLD_DRAGON_FLOATS_IN_MATRIX];
	float64				fTime;
	float32				fKeyTime;
	size				i;
	size				uiNumKeys;

	fTime = 0.0;
	uiNumKeys = pcSequence->macKeyFrames.NumElements();
	for (i = 0; i < uiNumKeys; i++)
	{
		psKey = pcSequence->macKeyFrames.Get(i);
		if ((i > 0) && (psKey->fTime <= 0.0f))
		{
			psPrevious = pcSequence->macKeyFrames.Get(i - 1);
			if ((i == uiNumKeys - 1) && (memcmp(&psPrevious->sMatrix, &psKey->sMatrix, sizeof(SOldDragonMatrix)) == 0))
			{
				break;
			}
			return gcLogger.Error2(__METHOD__, " Sequence [", pcSequence->mszName, "] has a key with no time before it.", NULL);
		}

		fTime += (float64)psKey->fTime;
		fKeyTime = (float32)fTime;
		pafTimes->Add(fKeyTime);
		AimMatrix(af4x4, psKey->sMatrix.af, eAim);
		paf4x4s->Add(af4x4, OLD_DRAGON_FLOATS_IN_MATRIX);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static ESceneFileLightType GetLightType(SOldDragonLight* psLight)
{
	if (strstr(psLight->szName, "Direct") != NULL)
	{
		return SFLT_Directional;
	}
	return (psLight->iType == 0) ? SFLT_Omni : SFLT_Spot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static float32 GetLightIntensity(COldDragonScene* pcScene, SOldDragonLight* psLight)
{
	return (pcScene->meVersion == ODV_IOX) ? 1.0f : psLight->fIntensity;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static float32 GetCameraFieldOfView(COldDragonScene* pcScene, SOldDragonCamera* psCamera)
{
	return (pcScene->meVersion == ODV_IOX) ? (psCamera->fFOV * 4.0f / 3.0f) : (float32)((float64)psCamera->fFOV * OLD_DRAGON_DEGREES_TO_RADIANS);
}


//////////////////////////////////////////////////////////////////////////
//
// A material's images are in the order of ESceneFileTexture.  Into Oblivion's
// are only a list, but none has more than one, which is its diffuse.
//
//////////////////////////////////////////////////////////////////////////
static bool CheckMaterialTextures(COldDragonScene* pcScene, SOldDragonMaterial* psMaterial)
{
	if ((pcScene->meVersion == ODV_IOX) && (psMaterial->iNumTextures > 1))
	{
		return gcLogger.Error2(__METHOD__, " Material [", psMaterial->szName, "] has more than one image, and which is which isn't known.", NULL);
	}
	if (psMaterial->iNumTextures > (int32)SFTX_NumTextures)
	{
		return gcLogger.Error2(__METHOD__, " Material [", psMaterial->szName, "] has too many images.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void WriteMesh(CSceneFileWriter* pcWriter, COldDragonMesh* pcMesh)
{
	COldDragonFlatMesh	cFlat;
	CArrayConvertFloats	af4x4s;

	cFlat.Init(pcMesh);

	pcWriter->BeginMesh(pcMesh->mszName);
	pcWriter->WritePositions((float32*)pcMesh->macPositions.GetData(), pcMesh->macPositions.NumElements());
	if (pcMesh->macNormals.NumElements() > 0)
	{
		pcWriter->WriteNormals((float32*)pcMesh->macNormals.GetData(), pcMesh->macNormals.NumElements());
	}
	pcWriter->WritePolygons(cFlat.maiPolygonSizes.GetData(), cFlat.maiPolygonSizes.NumElements(), cFlat.maiCornerPositions.GetData(), cFlat.maiCornerPositions.NumElements());
	if (cFlat.mbHasNormals)
	{
		pcWriter->WriteCornerNormals(cFlat.maiCornerNormals.GetData(), cFlat.maiCornerNormals.NumElements());
	}
	if (cFlat.mbHasUVs)
	{
		pcWriter->WriteUVLayer((float32*)pcMesh->macUVs.GetData(), pcMesh->macUVs.NumElements(), cFlat.maiCornerUVs.GetData(), cFlat.maiCornerUVs.NumElements());
	}
	if (cFlat.mbHasSlots)
	{
		pcWriter->WriteMaterialSlots(cFlat.maiMaterialSlots.GetData(), cFlat.maiMaterialSlots.NumElements());
		pcWriter->WritePolygonSlots(cFlat.maiPolygonSlots.GetData(), cFlat.maiPolygonSlots.NumElements());
	}
	if (cFlat.mbHasSmoothGroups)
	{
		pcWriter->WriteSmoothGroups(cFlat.mauiSmoothGroups.GetData(), cFlat.mauiSmoothGroups.NumElements());
	}
	if (pcMesh->macSkinVerts.NumElements() > 0)
	{
		pcWriter->WriteSkin(cFlat.maiSkinCounts.GetData(), cFlat.maiSkinCounts.NumElements(), cFlat.maiSkinMatrices.GetData(), cFlat.mafSkinWeights.GetData(), cFlat.mafSkinWeights.NumElements());
	}
	if (pcMesh->macInverseSkinMatrices.NumElements() > 0)
	{
		af4x4s.Init();
		GetInverseMatrices(pcMesh, &af4x4s);
		pcWriter->WriteInverseMatrices(af4x4s.GetData(), pcMesh->macInverseSkinMatrices.NumElements());
		af4x4s.Kill();
	}
	pcWriter->EndMesh();

	cFlat.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool WriteImage(CSceneFileWriter* pcWriter, COldDragonScene* pcScene, COldDragonImage* pcImage)
{
	CArrayConvertBytes	auiPixels;
	const char*			szChannels;
	bool				bResult;

	auiPixels.Init();
	bResult = GetImagePixels(pcScene, pcImage, &auiPixels, &szChannels);
	if (bResult)
	{
		pcWriter->BeginImage(pcImage->mszName);
		pcWriter->WritePixels(pcImage->miWidth, pcImage->miHeight, szChannels, auiPixels.GetData());
		pcWriter->EndImage();
	}
	auiPixels.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool WriteMaterial(CSceneFileWriter* pcWriter, COldDragonScene* pcScene, SOldDragonMaterial* psMaterial)
{
	int32	i;

	ReturnOnFalse(CheckMaterialTextures(pcScene, psMaterial));

	pcWriter->BeginMaterial(psMaterial->szName);
	pcWriter->WriteMaterialColour(SFMC_Diffuse, &psMaterial->sDiffuse.r);
	pcWriter->WriteMaterialColour(SFMC_Ambient, &psMaterial->sAmbient.r);
	pcWriter->WriteMaterialColour(SFMC_Emissive, &psMaterial->sEmissive.r);
	pcWriter->WriteMaterialColour(SFMC_Specular, &psMaterial->sSpecular.r);
	pcWriter->WriteShininess(psMaterial->fShininessPower, psMaterial->fShininessBase);
	for (i = 0; i < psMaterial->iNumTextures; i++)
	{
		if (psMaterial->aiTextures[i] != OLD_DRAGON_NO_INDEX)
		{
			pcWriter->WriteMaterialTexture((ESceneFileTexture)i, psMaterial->aiTextures[i]);
		}
	}
	pcWriter->EndMaterial();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The largest difference between two matrices.
//
//////////////////////////////////////////////////////////////////////////
static float32 MatrixDifference(const float32* pafA, const float32* pafB)
{
	float32	fDifference;
	float32	fLargest;
	int32	i;

	fLargest = 0.0f;
	for (i = 0; i < OLD_DRAGON_FLOATS_IN_MATRIX; i++)
	{
		fDifference = fabsf(pafA[i] - pafB[i]);
		if (fDifference > fLargest)
		{
			fLargest = fDifference;
		}
	}
	return fLargest;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a connection's world matrix is its local one and psParent's
// world one.  Goober multiplies them the way Into Oblivion's game did,
// the parent's first, and Test the other way round, so either fits.  How
// close it must be grows with how far from the origin the connection is.
//
//////////////////////////////////////////////////////////////////////////
static bool IsLostParent(SOldDragonConnection* psConnection, SOldDragonConnection* psParent)
{
	SFloat4x4	sWorld;
	float32		fTolerance;
	float32		fFurthest;
	int32		i;

	fFurthest = 0.0f;
	for (i = 12; i < 15; i++)
	{
		if (fabsf(psConnection->sWorld.af[i]) > fFurthest)
		{
			fFurthest = fabsf(psConnection->sWorld.af[i]);
		}
	}
	fTolerance = OLD_DRAGON_PARENT_TOLERANCE * (fFurthest + 1.0f);

	Float4x4Multiply(&sWorld, (SFloat4x4*)psParent->sWorld.af, (SFloat4x4*)psConnection->sLocal.af);
	if (MatrixDifference((float32*)&sWorld, psConnection->sWorld.af) < fTolerance)
	{
		return true;
	}
	Float4x4Multiply(&sWorld, (SFloat4x4*)psConnection->sLocal.af, (SFloat4x4*)psParent->sWorld.af);
	return MatrixDifference((float32*)&sWorld, psConnection->sWorld.af) < fTolerance;
}


//////////////////////////////////////////////////////////////////////////
//
// A connection's parent.  Into Oblivion's files say what it is.  The others
// lost it, but a connection's local matrix is still relative to it, and in
// them a parent comes before its children.  So it's the first connection
// before this one that makes this one's world matrix from its local one.
// One whose local matrix is its world one has no parent.  In Goober a bone
// and a finger helper can be in the same place, and as either gives the
// same world matrix the first is taken.
//
//////////////////////////////////////////////////////////////////////////
static int32 GetConnectionParent(COldDragonScene* pcScene, int32 iConnection)
{
	SOldDragonConnection*	psConnection;
	int32					i;

	psConnection = pcScene->macConnections.Get((size)iConnection);
	if (pcScene->meVersion == ODV_IOX)
	{
		return (psConnection->iParent == OLD_DRAGON_NO_INDEX) ? SCENE_FILE_NONE : psConnection->iParent;
	}
	if (MatrixDifference(psConnection->sWorld.af, psConnection->sLocal.af) < OLD_DRAGON_PARENT_TOLERANCE)
	{
		return SCENE_FILE_NONE;
	}
	for (i = 0; i < iConnection; i++)
	{
		if (IsLostParent(psConnection, pcScene->macConnections.Get((size)i)))
		{
			return i;
		}
	}
	gcLogger.Warning2(__METHOD__, " Connection [", psConnection->szName, "] has a local matrix that doesn't put it where it is under any connection before it, so it has no parent.", NULL);
	return SCENE_FILE_NONE;
}


//////////////////////////////////////////////////////////////////////////
//
// The world matrix is kept, and the local matrix is worked out from it and
// the parent's, as not all the old files multiplied them the way a scene
// file does.
//
//////////////////////////////////////////////////////////////////////////
static void WriteConnection(CSceneFileWriter* pcWriter, COldDragonScene* pcScene, CArrayOldDragonPlaced* pasPlaced, int32 iConnection)
{
	SOldDragonConnection*	psConnection;
	SFloat4x4				sWorld;
	SFloat4x4				sParentWorld;
	SFloat4x4				sInverse;
	SFloat4x4				sLocal;
	int32					iParent;

	psConnection = pcScene->macConnections.Get((size)iConnection);
	AimMatrix((float32*)&sWorld, psConnection->sWorld.af, GetConnectionAim(pcScene, pasPlaced, iConnection));
	iParent = GetConnectionParent(pcScene, iConnection);

	pcWriter->BeginConnection(psConnection->szName);
	pcWriter->WriteWorldMatrix((float32*)&sWorld);
	if (iParent == SCENE_FILE_NONE)
	{
		pcWriter->WriteLocalMatrix((float32*)&sWorld);
	}
	else
	{
		AimMatrix((float32*)&sParentWorld, pcScene->macConnections.Get((size)iParent)->sWorld.af, GetConnectionAim(pcScene, pasPlaced, iParent));
		Float4x4Inverse(&sInverse, NULL, &sParentWorld);
		Float4x4Multiply(&sLocal, &sWorld, &sInverse);
		pcWriter->WriteParent(iParent);
		pcWriter->WriteLocalMatrix((float32*)&sLocal);
	}
	pcWriter->WriteParity(psConnection->bParity);
	pcWriter->EndConnection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void WriteLight(CSceneFileWriter* pcWriter, COldDragonScene* pcScene, SOldDragonLight* psLight, int32 iConnection)
{
	pcWriter->BeginLight(psLight->szName);
	pcWriter->WriteLightType(GetLightType(psLight));
	pcWriter->WriteLightColour(&psLight->sColour.r);
	pcWriter->WriteLightValues(psLight->fDecayStart, psLight->fRange, GetLightIntensity(pcScene, psLight));
	pcWriter->WriteShadows(psLight->bShadows);
	if (iConnection != SCENE_FILE_NONE)
	{
		pcWriter->WriteAttached(iConnection);
	}
	pcWriter->EndLight();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void WriteCamera(CSceneFileWriter* pcWriter, COldDragonScene* pcScene, SOldDragonCamera* psCamera, int32 iConnection)
{
	pcWriter->BeginCamera(psCamera->szName);
	pcWriter->WriteCameraType(SFCT_Perspective);
	pcWriter->WriteLens(GetCameraFieldOfView(pcScene, psCamera), psCamera->fNear, psCamera->fFar);
	if (iConnection != SCENE_FILE_NONE)
	{
		pcWriter->WriteAttached(iConnection);
	}
	pcWriter->EndCamera();
}


//////////////////////////////////////////////////////////////////////////
//
// Lights and cameras that are placed are written for each place, and those
// that aren't are written once, attached to nothing.
//
//////////////////////////////////////////////////////////////////////////
static bool IsObjectPlaced(CArrayOldDragonPlaced* pasPlaced, EOldDragonPlacedKind eKind, size uiObject)
{
	SOldDragonPlaced*	psPlaced;
	size				i;

	for (i = 0; i < pasPlaced->NumElements(); i++)
	{
		psPlaced = pasPlaced->Get(i);
		if ((psPlaced->eKind == eKind) && (psPlaced->iObject == (int32)uiObject))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The ID of something placed, whose index must be in range.  It's
// OLD_DRAGON_NO_INDEX if the version has no IDs.
//
//////////////////////////////////////////////////////////////////////////
static int32 GetPlacedID(COldDragonScene* pcScene, SOldDragonPlaced* psPlaced)
{
	if (psPlaced->eKind == OLDPK_Mesh)
	{
		return pcScene->macMeshes.Get((size)psPlaced->iObject)->miID;
	}
	if (psPlaced->eKind == OLDPK_Light)
	{
		return pcScene->macLights.Get((size)psPlaced->iObject)->iID;
	}
	return pcScene->macCameras.Get((size)psPlaced->iObject)->iID;
}


//////////////////////////////////////////////////////////////////////////
//
// What's placed may be named by its ID, but is written by its index.  Those
// are the same in every old file, and one where they aren't isn't understood.
//
//////////////////////////////////////////////////////////////////////////
static bool CheckPlaced(COldDragonScene* pcScene, CArrayOldDragonPlaced* pasPlaced)
{
	SOldDragonPlaced*	psPlaced;
	size				uiNum;
	size				i;
	int32				iID;

	for (i = 0; i < pasPlaced->NumElements(); i++)
	{
		psPlaced = pasPlaced->Get(i);
		uiNum = (psPlaced->eKind == OLDPK_Mesh) ? pcScene->macMeshes.NumElements() : ((psPlaced->eKind == OLDPK_Light) ? pcScene->macLights.NumElements() : pcScene->macCameras.NumElements());
		if ((psPlaced->iObject < 0) || ((size)psPlaced->iObject >= uiNum))
		{
			return gcLogger.Error2(__METHOD__, " Something placed in the scene isn't there [", IntToString(psPlaced->iObject), "].", NULL);
		}
		iID = GetPlacedID(pcScene, psPlaced);
		if ((iID != OLD_DRAGON_NO_INDEX) && (iID != psPlaced->iObject))
		{
			return gcLogger.Error2(__METHOD__, " Something placed in the scene has an ID that isn't its index [", IntToString(iID), "].", NULL);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void WriteLightsAndCameras(CSceneFileWriter* pcWriter, COldDragonScene* pcScene, CArrayOldDragonPlaced* pasPlaced)
{
	SOldDragonPlaced*	psPlaced;
	size				i;

	for (i = 0; i < pasPlaced->NumElements(); i++)
	{
		psPlaced = pasPlaced->Get(i);
		if (psPlaced->eKind == OLDPK_Light)
		{
			WriteLight(pcWriter, pcScene, pcScene->macLights.Get((size)psPlaced->iObject), GetPlacedConnection(pcScene, psPlaced));
		}
		else if (psPlaced->eKind == OLDPK_Camera)
		{
			WriteCamera(pcWriter, pcScene, pcScene->macCameras.Get((size)psPlaced->iObject), GetPlacedConnection(pcScene, psPlaced));
		}
	}
	for (i = 0; i < pcScene->macLights.NumElements(); i++)
	{
		if (!IsObjectPlaced(pasPlaced, OLDPK_Light, i))
		{
			WriteLight(pcWriter, pcScene, pcScene->macLights.Get(i), SCENE_FILE_NONE);
		}
	}
	for (i = 0; i < pcScene->macCameras.NumElements(); i++)
	{
		if (!IsObjectPlaced(pasPlaced, OLDPK_Camera, i))
		{
			WriteCamera(pcWriter, pcScene, pcScene->macCameras.Get(i), SCENE_FILE_NONE);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// One animation holds all of a scene's sequences, as they play together.
//
//////////////////////////////////////////////////////////////////////////
static bool WriteAnimation(CSceneFileWriter* pcWriter, COldDragonScene* pcScene, CArrayOldDragonPlaced* pasPlaced)
{
	COldDragonSequence*	pcSequence;
	CArrayConvertFloats	afTimes;
	CArrayConvertFloats	af4x4s;
	size				i;
	int32				iConnection;
	bool				bResult;

	if (pcScene->macSequences.NumElements() == 0)
	{
		return true;
	}

	bResult = true;
	pcWriter->BeginAnimation("Animation");
	for (i = 0; (i < pcScene->macSequences.NumElements()) && bResult; i++)
	{
		pcSequence = pcScene->macSequences.Get(i);
		iConnection = GetSequenceConnection(pcScene, pcSequence);
		if (iConnection == SCENE_FILE_NONE)
		{
			gcLogger.Error2(__METHOD__, " Sequence [", pcSequence->mszName, "] doesn't have one connection with its name.", NULL);
			bResult = false;
			break;
		}
		afTimes.Init();
		af4x4s.Init();
		bResult = GetTrackKeys(pcSequence, GetConnectionAim(pcScene, pasPlaced, iConnection), &afTimes, &af4x4s);
		if (bResult)
		{
			pcWriter->WriteTrack(iConnection, afTimes.GetData(), af4x4s.GetData(), afTimes.NumElements());
		}
		af4x4s.Kill();
		afTimes.Kill();
	}
	pcWriter->EndAnimation();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void WriteMeshInstance(CSceneFileWriter* pcWriter, COldDragonScene* pcScene, SOldDragonPlaced* psPlaced)
{
	CArrayInt	aiConnections;
	CArrayInt	aiSlots;

	aiConnections.Init();
	aiSlots.Init();
	GetInstanceConnections(pcScene, psPlaced, &aiConnections, &aiSlots);

	pcWriter->BeginInstance(GetPlacedName(pcScene, psPlaced));
	pcWriter->WriteConnections(aiConnections.GetData(), aiConnections.NumElements());
	pcWriter->WriteInstanceMesh(psPlaced->iObject, aiSlots.GetData(), aiSlots.NumElements(), NULL, 0);
	pcWriter->EndInstance();

	aiSlots.Kill();
	aiConnections.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void WriteParticles(CSceneFileWriter* pcWriter, COldDragonScene* pcScene)
{
	SOldDragonParticle*	psParticle;
	CArrayConvertFloats	afPositions;
	CArrayConvertFloats	afColours;
	CArrayConvertFloats	afRadii;
	CArrayConvertFloats	afIntensities;
	size				i;

	if (pcScene->macParticles.NumElements() == 0)
	{
		return;
	}

	afPositions.Init();
	afColours.Init();
	afRadii.Init();
	afIntensities.Init();
	for (i = 0; i < pcScene->macParticles.NumElements(); i++)
	{
		psParticle = pcScene->macParticles.Get(i);
		afPositions.Add(&psParticle->sPosition.x, 3);
		afColours.Add(&psParticle->fRed, 3);
		afRadii.Add(psParticle->fRadius);
		afIntensities.Add(psParticle->fIntensity);
	}
	pcWriter->WriteParticles("Particles", afPositions.GetData(), afColours.GetData(), afRadii.GetData(), afIntensities.GetData(), pcScene->macParticles.NumElements());
	afIntensities.Kill();
	afRadii.Kill();
	afColours.Kill();
	afPositions.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool WriteOldDragonSceneFile(COldDragonScene* pcScene, const char* szFilename)
{
	CSceneFileWriter		cWriter;
	CArrayOldDragonPlaced	asPlaced;
	size					i;
	bool					bResult;

	asPlaced.Init();
	bResult = FindOldDragonPlaced(pcScene, &asPlaced) && CheckPlaced(pcScene, &asPlaced);

	cWriter.Init(szFilename);
	bResult = bResult && cWriter.Open();

	if (bResult)
	{
		for (i = 0; i < pcScene->macMeshes.NumElements(); i++)
		{
			WriteMesh(&cWriter, pcScene->macMeshes.Get(i));
		}
		for (i = 0; (i < pcScene->macImages.NumElements()) && bResult; i++)
		{
			bResult = WriteImage(&cWriter, pcScene, pcScene->macImages.Get(i));
		}
		for (i = 0; (i < pcScene->macMaterials.NumElements()) && bResult; i++)
		{
			bResult = WriteMaterial(&cWriter, pcScene, pcScene->macMaterials.Get(i));
		}
		for (i = 0; i < pcScene->macConnections.NumElements(); i++)
		{
			WriteConnection(&cWriter, pcScene, &asPlaced, (int32)i);
		}
		WriteLightsAndCameras(&cWriter, pcScene, &asPlaced);
		bResult = bResult && WriteAnimation(&cWriter, pcScene, &asPlaced);
		for (i = 0; i < asPlaced.NumElements(); i++)
		{
			if (asPlaced.Get(i)->eKind == OLDPK_Mesh)
			{
				WriteMeshInstance(&cWriter, pcScene, asPlaced.Get(i));
			}
		}
		WriteParticles(&cWriter, pcScene);

		if (bResult)
		{
			bResult = cWriter.Close();
		}
	}
	cWriter.Kill();
	asPlaced.Kill();
	return bResult;
}


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
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayTemplatePrimitive.h"
#include "BaseLib/Chars.h"
#include "BaseLib/Logger.h"
#include "SupportLib/SceneFileReader.h"
#include "SupportLib/SceneFileWriter.h"
#include "OldDragonFile.h"
#include "OldDragonConverter.h"


#define OLD_DRAGON_MAX_INSTANCE_MESH	1
#define OLD_DRAGON_MAX_INSTANCE_LIGHT	2
#define OLD_DRAGON_MAX_INSTANCE_CAMERA	3

#define OLD_DRAGON_DEGREES_TO_RADIANS	(3.14159265358979 / 180.0)
#define OLD_DRAGON_FLOATS_IN_MATRIX		16


typedef CArrayTemplatePrimitive<float32>	CArrayConvertFloats;
typedef CArrayTemplatePrimitive<uint32>	CArrayConvertUints;
typedef CArrayTemplatePrimitive<uint8>	CArrayConvertBytes;


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


//A scene's particles as they are in a scene file.
class COldDragonFlatParticles
{
public:
	CArrayConvertFloats	mafPositions;
	CArrayConvertFloats	mafColours;
	CArrayConvertFloats	mafRadii;
	CArrayConvertFloats	mafIntensities;

	void	Init(COldDragonScene* pcScene);
	void	Kill(void);
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
void COldDragonFlatParticles::Init(COldDragonScene* pcScene)
{
	SOldDragonParticle*	psParticle;
	size				i;

	mafPositions.Init();
	mafColours.Init();
	mafRadii.Init();
	mafIntensities.Init();
	for (i = 0; i < pcScene->macParticles.NumElements(); i++)
	{
		psParticle = pcScene->macParticles.Get(i);
		mafPositions.Add(&psParticle->sPosition.x, 3);
		mafColours.Add(&psParticle->fRed, 3);
		mafRadii.Add(psParticle->fRadius);
		mafIntensities.Add(psParticle->fIntensity);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COldDragonFlatParticles::Kill(void)
{
	mafIntensities.Kill();
	mafRadii.Kill();
	mafColours.Kill();
	mafPositions.Kill();
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
static bool GetTrackKeys(COldDragonSequence* pcSequence, CArrayConvertFloats* pafTimes, CArrayConvertFloats* paf4x4s)
{
	SOldDragonKeyFrame*	psKey;
	SOldDragonKeyFrame*	psPrevious;
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
		paf4x4s->Add(psKey->sMatrix.af, OLD_DRAGON_FLOATS_IN_MATRIX);
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
	return (pcScene->meVersion == ODV_IOX) ? psCamera->fFOV : (float32)((float64)psCamera->fFOV * OLD_DRAGON_DEGREES_TO_RADIANS);
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
//
//////////////////////////////////////////////////////////////////////////
static void WriteConnection(CSceneFileWriter* pcWriter, SOldDragonConnection* psConnection)
{
	pcWriter->BeginConnection(psConnection->szName);
	pcWriter->WriteWorldMatrix(psConnection->sWorld.af);
	pcWriter->WriteLocalMatrix(psConnection->sLocal.af);
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
static bool WriteAnimation(CSceneFileWriter* pcWriter, COldDragonScene* pcScene)
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
		bResult = GetTrackKeys(pcSequence, &afTimes, &af4x4s);
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
	COldDragonFlatParticles	cFlat;

	if (pcScene->macParticles.NumElements() == 0)
	{
		return;
	}

	cFlat.Init(pcScene);
	pcWriter->WriteParticles("Particles", cFlat.mafPositions.GetData(), cFlat.mafColours.GetData(), cFlat.mafRadii.GetData(), cFlat.mafIntensities.GetData(), pcScene->macParticles.NumElements());
	cFlat.Kill();
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
			WriteConnection(&cWriter, pcScene->macConnections.Get(i));
		}
		WriteLightsAndCameras(&cWriter, pcScene, &asPlaced);
		bResult = bResult && WriteAnimation(&cWriter, pcScene);
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool VerifyFail(const char* szWhat, const char* szName)
{
	return gcLogger.Error2(__METHOD__, " [", szName, "] ", szWhat, " isn't what it should be.", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool VerifyName(CSceneFileReader* pcReader, const char* szExpected)
{
	CChars	sz;
	bool	bResult;

	sz.Init();
	bResult = pcReader->ReadName(&sz) && (strcmp(sz.Text(), szExpected) == 0);
	sz.Kill();
	if (!bResult)
	{
		return VerifyFail("Name", szExpected);
	}
	return true;
}


typedef bool (CSceneFileReader::*ReadInts)(int32*);
typedef bool (CSceneFileReader::*ReadFloats)(float32*);


//////////////////////////////////////////////////////////////////////////
//
// uiNum is how many pcReader says there are.  They're only read if that's
// how many there should be, so they fit.
//
//////////////////////////////////////////////////////////////////////////
static bool VerifyInts(CSceneFileReader* pcReader, ReadInts fRead, size uiNum, const int32* paiExpected, size uiExpected, const char* szWhat, const char* szName)
{
	CArrayInt	aiRead;
	bool		bResult;

	if (uiNum != uiExpected)
	{
		return VerifyFail(szWhat, szName);
	}
	if (uiNum == 0)
	{
		return true;
	}
	aiRead.Init();
	aiRead.Resize(uiNum);
	bResult = (pcReader->*fRead)(aiRead.GetData()) && (memcmp(aiRead.GetData(), paiExpected, uiNum * sizeof(int32)) == 0);
	aiRead.Kill();
	if (!bResult)
	{
		return VerifyFail(szWhat, szName);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool VerifyFloats(CSceneFileReader* pcReader, ReadFloats fRead, size uiNum, const float32* pafExpected, size uiExpected, const char* szWhat, const char* szName)
{
	CArrayConvertFloats	afRead;
	bool				bResult;

	if (uiNum != uiExpected)
	{
		return VerifyFail(szWhat, szName);
	}
	if (uiNum == 0)
	{
		return true;
	}
	afRead.Init();
	afRead.Resize(uiNum);
	bResult = (pcReader->*fRead)(afRead.GetData()) && (memcmp(afRead.GetData(), pafExpected, uiNum * sizeof(float32)) == 0);
	afRead.Kill();
	if (!bResult)
	{
		return VerifyFail(szWhat, szName);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool VerifyMeshSkin(CSceneFileReader* pcReader, COldDragonMesh* pcMesh, COldDragonFlatMesh* pcFlat)
{
	CArrayConvertFloats	af4x4s;
	bool				bResult;

	ReturnOnFalse(VerifyInts(pcReader, &CSceneFileReader::ReadSkinCounts, pcReader->NumSkinCounts(), pcFlat->maiSkinCounts.GetData(), pcFlat->maiSkinCounts.NumElements(), "SkinCounts", pcMesh->mszName));
	ReturnOnFalse(VerifyInts(pcReader, &CSceneFileReader::ReadSkinMatrices, pcReader->NumSkinWeights(), pcFlat->maiSkinMatrices.GetData(), pcFlat->maiSkinMatrices.NumElements(), "SkinMatrices", pcMesh->mszName));
	ReturnOnFalse(VerifyFloats(pcReader, &CSceneFileReader::ReadSkinWeights, pcReader->NumSkinWeights(), pcFlat->mafSkinWeights.GetData(), pcFlat->mafSkinWeights.NumElements(), "SkinWeights", pcMesh->mszName));

	af4x4s.Init();
	GetInverseMatrices(pcMesh, &af4x4s);
	bResult = VerifyFloats(pcReader, &CSceneFileReader::ReadInverseMatrices, pcReader->NumInverseMatrices() * OLD_DRAGON_FLOATS_IN_MATRIX, af4x4s.GetData(), af4x4s.NumElements(), "InverseMatrices", pcMesh->mszName);
	af4x4s.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// What's left out of a mesh is -1s or 0s when written, so it's compared
// only if it was written, and must be missing if it wasn't.
//
//////////////////////////////////////////////////////////////////////////
static bool VerifyMesh(CSceneFileReader* pcReader, COldDragonMesh* pcMesh)
{
	COldDragonFlatMesh	cFlat;
	CArrayConvertUints	auiGroups;
	bool				bResult;
	const char*			szName;
	size				uiNumCorners;

	cFlat.Init(pcMesh);
	szName = pcMesh->mszName;
	uiNumCorners = cFlat.maiCornerPositions.NumElements();

	bResult = VerifyName(pcReader, szName);
	bResult = bResult && VerifyFloats(pcReader, &CSceneFileReader::ReadPositions, pcReader->NumPositions() * 3, (float32*)pcMesh->macPositions.GetData(), pcMesh->macPositions.NumElements() * 3, "Positions", szName);
	bResult = bResult && VerifyFloats(pcReader, &CSceneFileReader::ReadNormals, pcReader->NumNormals() * 3, (float32*)pcMesh->macNormals.GetData(), pcMesh->macNormals.NumElements() * 3, "Normals", szName);
	bResult = bResult && VerifyInts(pcReader, &CSceneFileReader::ReadPolygonSizes, pcReader->NumPolygons(), cFlat.maiPolygonSizes.GetData(), cFlat.maiPolygonSizes.NumElements(), "PolygonSizes", szName);
	bResult = bResult && VerifyInts(pcReader, &CSceneFileReader::ReadCornerPositions, pcReader->NumCorners(), cFlat.maiCornerPositions.GetData(), uiNumCorners, "CornerPositions", szName);
	bResult = bResult && VerifyInts(pcReader, &CSceneFileReader::ReadCornerNormals, pcReader->NumCornerNormals(), cFlat.maiCornerNormals.GetData(), cFlat.mbHasNormals ? uiNumCorners : 0, "CornerNormals", szName);
	bResult = bResult && VerifyInts(pcReader, &CSceneFileReader::ReadMaterialSlots, pcReader->NumMaterialSlots(), cFlat.maiMaterialSlots.GetData(), cFlat.mbHasSlots ? cFlat.maiMaterialSlots.NumElements() : 0, "MaterialSlots", szName);
	bResult = bResult && VerifyInts(pcReader, &CSceneFileReader::ReadPolygonSlots, pcReader->NumPolygonSlots(), cFlat.maiPolygonSlots.GetData(), cFlat.mbHasSlots ? cFlat.maiPolygonSlots.NumElements() : 0, "PolygonSlots", szName);
	bResult = bResult && VerifyMeshSkin(pcReader, pcMesh, &cFlat);

	if (bResult && (pcReader->NumUVLayers() != (cFlat.mbHasUVs ? 1u : 0u)))
	{
		bResult = VerifyFail("UVLayers", szName);
	}
	if (bResult && cFlat.mbHasUVs)
	{
		bResult = pcReader->BeginUVLayer(0);
		bResult = bResult && VerifyFloats(pcReader, &CSceneFileReader::ReadUVs, pcReader->NumUVs() * 2, (float32*)pcMesh->macUVs.GetData(), pcMesh->macUVs.NumElements() * 2, "UVs", szName);
		bResult = bResult && VerifyInts(pcReader, &CSceneFileReader::ReadCornerUVs, pcReader->NumCornerUVs(), cFlat.maiCornerUVs.GetData(), uiNumCorners, "CornerUVs", szName);
		bResult = pcReader->End() && bResult;
	}

	if (bResult && (pcReader->NumSmoothGroups() != (cFlat.mbHasSmoothGroups ? cFlat.mauiSmoothGroups.NumElements() : 0)))
	{
		bResult = VerifyFail("SmoothGroups", szName);
	}
	if (bResult && cFlat.mbHasSmoothGroups)
	{
		auiGroups.Init();
		auiGroups.Resize(cFlat.mauiSmoothGroups.NumElements());
		bResult = pcReader->ReadSmoothGroups(auiGroups.GetData()) && (memcmp(auiGroups.GetData(), cFlat.mauiSmoothGroups.GetData(), auiGroups.ByteSize()) == 0);
		auiGroups.Kill();
		if (!bResult)
		{
			VerifyFail("SmoothGroups", szName);
		}
	}

	cFlat.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool VerifyImage(CSceneFileReader* pcReader, COldDragonScene* pcScene, COldDragonImage* pcImage)
{
	CArrayConvertBytes	auiPixels;
	CArrayConvertBytes	auiRead;
	CChars				szChannels;
	const char*			szExpected;
	int32				iWidth;
	int32				iHeight;
	bool				bResult;

	ReturnOnFalse(VerifyName(pcReader, pcImage->mszName));

	auiPixels.Init();
	auiRead.Init();
	szChannels.Init();
	bResult = GetImagePixels(pcScene, pcImage, &auiPixels, &szExpected);
	bResult = bResult && pcReader->ReadImageSize(&iWidth, &iHeight) && (iWidth == pcImage->miWidth) && (iHeight == pcImage->miHeight);
	bResult = bResult && pcReader->ReadChannels(&szChannels) && (strcmp(szChannels.Text(), szExpected) == 0);
	bResult = bResult && (pcReader->NumPixelBytes() == auiPixels.NumElements());
	if (bResult)
	{
		auiRead.Resize(auiPixels.NumElements());
		bResult = pcReader->ReadPixels(auiRead.GetData()) && (memcmp(auiRead.GetData(), auiPixels.GetData(), auiPixels.NumElements()) == 0);
	}
	szChannels.Kill();
	auiRead.Kill();
	auiPixels.Kill();
	if (!bResult)
	{
		return VerifyFail("Image", pcImage->mszName);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool SameFloats(const float32* pafA, const float32* pafB, size uiNum)
{
	return memcmp(pafA, pafB, uiNum * sizeof(float32)) == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool VerifyMaterial(CSceneFileReader* pcReader, COldDragonScene* pcScene, SOldDragonMaterial* psMaterial)
{
	float32	afColour[4];
	float32	fPower;
	float32	fBase;
	int32	i;
	int32	iExpected;

	ReturnOnFalse(VerifyName(pcReader, psMaterial->szName));
	ReturnOnFalse(CheckMaterialTextures(pcScene, psMaterial));

	if (!pcReader->ReadMaterialColour(SFMC_Diffuse, afColour) || !SameFloats(afColour, &psMaterial->sDiffuse.r, 4) ||
		!pcReader->ReadMaterialColour(SFMC_Ambient, afColour) || !SameFloats(afColour, &psMaterial->sAmbient.r, 4) ||
		!pcReader->ReadMaterialColour(SFMC_Emissive, afColour) || !SameFloats(afColour, &psMaterial->sEmissive.r, 4) ||
		!pcReader->ReadMaterialColour(SFMC_Specular, afColour) || !SameFloats(afColour, &psMaterial->sSpecular.r, 4) ||
		!pcReader->ReadShininess(&fPower, &fBase) || (fPower != psMaterial->fShininessPower) || (fBase != psMaterial->fShininessBase))
	{
		return VerifyFail("Colours", psMaterial->szName);
	}
	for (i = 0; i < (int32)SFTX_NumTextures; i++)
	{
		iExpected = (i < psMaterial->iNumTextures) ? psMaterial->aiTextures[i] : SCENE_FILE_NONE;
		if (pcReader->GetMaterialTexture((ESceneFileTexture)i) != iExpected)
		{
			return VerifyFail("Textures", psMaterial->szName);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool VerifyConnection(CSceneFileReader* pcReader, SOldDragonConnection* psConnection)
{
	float32	af4x4[OLD_DRAGON_FLOATS_IN_MATRIX];

	ReturnOnFalse(VerifyName(pcReader, psConnection->szName));
	if (!pcReader->ReadWorldMatrix(af4x4) || !SameFloats(af4x4, psConnection->sWorld.af, OLD_DRAGON_FLOATS_IN_MATRIX) ||
		!pcReader->ReadLocalMatrix(af4x4) || !SameFloats(af4x4, psConnection->sLocal.af, OLD_DRAGON_FLOATS_IN_MATRIX) ||
		(pcReader->GetParity() != psConnection->bParity) || (pcReader->GetParent() != SCENE_FILE_NONE))
	{
		return VerifyFail("Connection", psConnection->szName);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool VerifyLight(CSceneFileReader* pcReader, COldDragonScene* pcScene, SOldDragonLight* psLight, int32 iConnection)
{
	float32	afColour[4];
	float32	fDecayStart;
	float32	fRange;
	float32	fIntensity;

	ReturnOnFalse(VerifyName(pcReader, psLight->szName));
	if ((pcReader->GetLightType() != GetLightType(psLight)) || !pcReader->ReadLightColour(afColour) || !SameFloats(afColour, &psLight->sColour.r, 4) ||
		!pcReader->ReadLightValues(&fDecayStart, &fRange, &fIntensity) || (fDecayStart != psLight->fDecayStart) || (fRange != psLight->fRange) || (fIntensity != GetLightIntensity(pcScene, psLight)) ||
		(pcReader->GetShadows() != psLight->bShadows) || (pcReader->GetAttached() != iConnection))
	{
		return VerifyFail("Light", psLight->szName);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool VerifyCamera(CSceneFileReader* pcReader, COldDragonScene* pcScene, SOldDragonCamera* psCamera, int32 iConnection)
{
	float32	fFieldOfView;
	float32	fNear;
	float32	fFar;

	ReturnOnFalse(VerifyName(pcReader, psCamera->szName));
	if (!pcReader->ReadLens(&fFieldOfView, &fNear, &fFar) || (fFieldOfView != GetCameraFieldOfView(pcScene, psCamera)) || (fNear != psCamera->fNear) || (fFar != psCamera->fFar) ||
		(pcReader->GetCameraType() != SFCT_Perspective) || (pcReader->GetAttached() != iConnection))
	{
		return VerifyFail("Camera", psCamera->szName);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// In the order they were written.
//
//////////////////////////////////////////////////////////////////////////
static bool VerifyLightsAndCameras(CSceneFileReader* pcReader, COldDragonScene* pcScene, CArrayOldDragonPlaced* pasPlaced)
{
	SOldDragonPlaced*	psPlaced;
	size				uiLight;
	size				uiCamera;
	size				i;
	bool				bResult;

	uiLight = 0;
	uiCamera = 0;
	bResult = true;
	for (i = 0; (i < pasPlaced->NumElements()) && bResult; i++)
	{
		psPlaced = pasPlaced->Get(i);
		if (psPlaced->eKind == OLDPK_Light)
		{
			bResult = pcReader->BeginLight(uiLight) && VerifyLight(pcReader, pcScene, pcScene->macLights.Get((size)psPlaced->iObject), GetPlacedConnection(pcScene, psPlaced)) && pcReader->End();
			uiLight++;
		}
		else if (psPlaced->eKind == OLDPK_Camera)
		{
			bResult = pcReader->BeginCamera(uiCamera) && VerifyCamera(pcReader, pcScene, pcScene->macCameras.Get((size)psPlaced->iObject), GetPlacedConnection(pcScene, psPlaced)) && pcReader->End();
			uiCamera++;
		}
	}
	for (i = 0; (i < pcScene->macLights.NumElements()) && bResult; i++)
	{
		if (!IsObjectPlaced(pasPlaced, OLDPK_Light, i))
		{
			bResult = pcReader->BeginLight(uiLight) && VerifyLight(pcReader, pcScene, pcScene->macLights.Get(i), SCENE_FILE_NONE) && pcReader->End();
			uiLight++;
		}
	}
	for (i = 0; (i < pcScene->macCameras.NumElements()) && bResult; i++)
	{
		if (!IsObjectPlaced(pasPlaced, OLDPK_Camera, i))
		{
			bResult = pcReader->BeginCamera(uiCamera) && VerifyCamera(pcReader, pcScene, pcScene->macCameras.Get(i), SCENE_FILE_NONE) && pcReader->End();
			uiCamera++;
		}
	}
	return bResult && (pcReader->NumLights() == uiLight) && (pcReader->NumCameras() == uiCamera);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool VerifyAnimation(CSceneFileReader* pcReader, COldDragonScene* pcScene)
{
	COldDragonSequence*	pcSequence;
	CArrayConvertFloats	afTimes;
	CArrayConvertFloats	af4x4s;
	size				i;
	bool				bResult;

	if (pcScene->macSequences.NumElements() == 0)
	{
		return pcReader->NumAnimations() == 0;
	}

	bResult = (pcReader->NumAnimations() == 1) && pcReader->BeginAnimation(0) && (pcReader->NumTracks() == pcScene->macSequences.NumElements());
	for (i = 0; (i < pcScene->macSequences.NumElements()) && bResult; i++)
	{
		pcSequence = pcScene->macSequences.Get(i);
		afTimes.Init();
		af4x4s.Init();
		bResult = GetTrackKeys(pcSequence, &afTimes, &af4x4s) && pcReader->BeginTrack(i) && (pcReader->GetAttached() == GetSequenceConnection(pcScene, pcSequence));
		bResult = bResult && VerifyFloats(pcReader, &CSceneFileReader::ReadKeyTimes, pcReader->NumKeys(), afTimes.GetData(), afTimes.NumElements(), "KeyTimes", pcSequence->mszName);
		bResult = bResult && VerifyFloats(pcReader, &CSceneFileReader::ReadKeyMatrices, pcReader->NumKeys() * OLD_DRAGON_FLOATS_IN_MATRIX, af4x4s.GetData(), af4x4s.NumElements(), "KeyMatrices", pcSequence->mszName);
		bResult = bResult && pcReader->End();
		af4x4s.Kill();
		afTimes.Kill();
	}
	return bResult && pcReader->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool VerifyMeshInstance(CSceneFileReader* pcReader, COldDragonScene* pcScene, SOldDragonPlaced* psPlaced)
{
	CArrayInt	aiConnections;
	CArrayInt	aiSlots;
	bool		bResult;
	const char*	szName;

	aiConnections.Init();
	aiSlots.Init();
	GetInstanceConnections(pcScene, psPlaced, &aiConnections, &aiSlots);
	szName = GetPlacedName(pcScene, psPlaced);

	bResult = VerifyName(pcReader, szName);
	bResult = bResult && VerifyInts(pcReader, &CSceneFileReader::ReadInstanceConnections, pcReader->NumInstanceConnections(), aiConnections.GetData(), aiConnections.NumElements(), "Connections", szName);
	bResult = bResult && (pcReader->NumInstanceMeshes() == 1) && pcReader->BeginInstanceMesh(0) && (pcReader->GetInstanceMesh() == psPlaced->iObject);
	bResult = bResult && VerifyInts(pcReader, &CSceneFileReader::ReadConnectionSlots, pcReader->NumConnectionSlots(), aiSlots.GetData(), aiSlots.NumElements(), "ConnectionSlots", szName);
	bResult = bResult && (pcReader->NumInstanceMaterials() == 0) && pcReader->End();

	aiSlots.Kill();
	aiConnections.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool VerifyParticles(CSceneFileReader* pcReader, COldDragonScene* pcScene)
{
	COldDragonFlatParticles	cFlat;
	size					uiNum;
	bool					bResult;

	uiNum = pcScene->macParticles.NumElements();
	if (uiNum == 0)
	{
		return pcReader->NumParticleSets() == 0;
	}

	cFlat.Init(pcScene);
	bResult = (pcReader->NumParticleSets() == 1) && pcReader->BeginParticles(0) && (pcReader->NumParticles() == uiNum);
	bResult = bResult && VerifyFloats(pcReader, &CSceneFileReader::ReadParticlePositions, uiNum * 3, cFlat.mafPositions.GetData(), uiNum * 3, "ParticlePositions", "Particles");
	bResult = bResult && VerifyFloats(pcReader, &CSceneFileReader::ReadParticleColours, uiNum * 3, cFlat.mafColours.GetData(), uiNum * 3, "ParticleColours", "Particles");
	bResult = bResult && VerifyFloats(pcReader, &CSceneFileReader::ReadParticleRadii, uiNum, cFlat.mafRadii.GetData(), uiNum, "ParticleRadii", "Particles");
	bResult = bResult && VerifyFloats(pcReader, &CSceneFileReader::ReadParticleIntensities, uiNum, cFlat.mafIntensities.GetData(), uiNum, "ParticleIntensities", "Particles");
	bResult = bResult && pcReader->End();
	cFlat.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool VerifyScene(CSceneFileReader* pcReader, COldDragonScene* pcScene, CArrayOldDragonPlaced* pasPlaced)
{
	size	i;
	size	uiInstance;
	bool	bResult;

	bResult = (pcReader->NumMeshes() == pcScene->macMeshes.NumElements()) && (pcReader->NumImages() == pcScene->macImages.NumElements()) &&
			  (pcReader->NumMaterials() == pcScene->macMaterials.NumElements()) && (pcReader->NumConnections() == pcScene->macConnections.NumElements());
	if (!bResult)
	{
		return VerifyFail("Number of things", "Scene");
	}

	for (i = 0; (i < pcScene->macMeshes.NumElements()) && bResult; i++)
	{
		bResult = pcReader->BeginMesh(i) && VerifyMesh(pcReader, pcScene->macMeshes.Get(i)) && pcReader->End();
	}
	for (i = 0; (i < pcScene->macImages.NumElements()) && bResult; i++)
	{
		bResult = pcReader->BeginImage(i) && VerifyImage(pcReader, pcScene, pcScene->macImages.Get(i)) && pcReader->End();
	}
	for (i = 0; (i < pcScene->macMaterials.NumElements()) && bResult; i++)
	{
		bResult = pcReader->BeginMaterial(i) && VerifyMaterial(pcReader, pcScene, pcScene->macMaterials.Get(i)) && pcReader->End();
	}
	for (i = 0; (i < pcScene->macConnections.NumElements()) && bResult; i++)
	{
		bResult = pcReader->BeginConnection(i) && VerifyConnection(pcReader, pcScene->macConnections.Get(i)) && pcReader->End();
	}
	bResult = bResult && VerifyLightsAndCameras(pcReader, pcScene, pasPlaced);
	bResult = bResult && VerifyAnimation(pcReader, pcScene);

	uiInstance = 0;
	for (i = 0; (i < pasPlaced->NumElements()) && bResult; i++)
	{
		if (pasPlaced->Get(i)->eKind == OLDPK_Mesh)
		{
			bResult = pcReader->BeginInstance(uiInstance) && VerifyMeshInstance(pcReader, pcScene, pasPlaced->Get(i)) && pcReader->End();
			uiInstance++;
		}
	}
	bResult = bResult && (pcReader->NumInstances() == uiInstance);
	return bResult && VerifyParticles(pcReader, pcScene);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool VerifyOldDragonSceneFile(COldDragonScene* pcScene, const char* szFilename)
{
	CSceneFileReader		cReader;
	CArrayOldDragonPlaced	asPlaced;
	bool					bResult;

	asPlaced.Init();
	bResult = FindOldDragonPlaced(pcScene, &asPlaced);

	cReader.Init(szFilename);
	bResult = bResult && cReader.Open();
	bResult = bResult && VerifyScene(&cReader, pcScene, &asPlaced);
	bResult = bResult && !cReader.Failed();
	cReader.Kill();
	asPlaced.Kill();

	if (!bResult)
	{
		return gcLogger.Error2(__METHOD__, " Scene file [", szFilename, "] doesn't hold what it should.", NULL);
	}
	return true;
}


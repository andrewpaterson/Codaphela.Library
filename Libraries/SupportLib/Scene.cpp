/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SceneLib

Codaphela SceneLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SceneLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SceneLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "BaseLib/ArrayChars.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "StandardLib/Objects.h"
#include "ImageChannel.h"
#include "SceneFileMesh.h"
#include "Scene.h"


#define SCENE_IMAGE_RGBA	"RGBA"
#define SCENE_IMAGE_RGB		"RGB"
#define SCENE_IMAGE_ALPHA	"A"


//////////////////////////////////////////////////////////////////////////
//
// The channels of an image a scene file can hold as it is, or NULL: it has
// to be bytes in that order.
//
//////////////////////////////////////////////////////////////////////////
static bool IsSceneFileImageChannel(CImage* pcImage, size uiIndex, size uiChannel)
{
	return pcImage->GetChannelAtIndex(uiIndex)->iChannel == uiChannel;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static const char* GetSceneFileImageChannels(CImage* pcImage)
{
	if (pcImage->GetPrimitiveType() != PT_uint8)
	{
		return NULL;
	}
	if ((pcImage->NumChannels() >= 3) && IsSceneFileImageChannel(pcImage, 0, IMAGE_DIFFUSE_RED) && IsSceneFileImageChannel(pcImage, 1, IMAGE_DIFFUSE_GREEN) && IsSceneFileImageChannel(pcImage, 2, IMAGE_DIFFUSE_BLUE))
	{
		if (pcImage->NumChannels() == 3)
		{
			return SCENE_IMAGE_RGB;
		}
		if ((pcImage->NumChannels() == 4) && IsSceneFileImageChannel(pcImage, 3, IMAGE_OPACITY))
		{
			return SCENE_IMAGE_RGBA;
		}
		return NULL;
	}
	if ((pcImage->NumChannels() == 1) && IsSceneFileImageChannel(pcImage, 0, IMAGE_OPACITY))
	{
		return SCENE_IMAGE_ALPHA;
	}
	return NULL;
}


//A file's lists, as a scene's objects hold them.  This holds Ptrs, so it's an object.
class CSceneFileLoad : public CObject
{
CONSTRUCTABLE(CSceneFileLoad);
DESTRUCTABLE(CSceneFileLoad);
public:
	CSceneFileReader	mcReader;
	CArray<CConnection>	mapConnections;
	CArray<CImage>		mapImages;
	CArrayChars			maszImageNames;
	CArrayMaterial		mapMaterials;
	CArray<CMeshEditor>	mapEditors;

	void	Init(const char* szFilename);
	void	Free(void) override;
	void	Class(void) override;

	bool	Read(CScene* pcScene);

protected:
	bool	ReadConnections(void);
	bool	ReadImages(void);
	bool	ReadMaterials(void);
	bool	ReadMeshes(void);
	bool	ReadLights(CScene* pcScene);
	bool	ReadCameras(CScene* pcScene);
	bool	ReadAnimations(CScene* pcScene);
	bool	ReadInstances(CScene* pcScene);
	bool	ReadInstanceMesh(CInstance* pcInstance);
	bool	ReadParticles(CScene* pcScene);
	Ptr<CConnection>	GetAttached(void);
};


//What's written, in the order it's found in the scene: a file refers to each by its place in these.
class CSceneFileSave : public CObject
{
CONSTRUCTABLE(CSceneFileSave);
DESTRUCTABLE(CSceneFileSave);
public:
	CSceneFileWriter	mcWriter;
	CArray<CConnection>	mapConnections;
	CArray<CImage>		mapImages;
	CArrayChars			maszImageNames;
	CArrayMaterial		mapMaterials;
	CArray<CMeshEditor>	mapEditors;

	void	Init(const char* szFilename);
	void	Free(void) override;
	void	Class(void) override;

	bool	Write(CScene* pcScene);

protected:
	void	GatherMaterial(Ptr<CMaterial> pMaterial);
	void	GatherEditor(Ptr<CMeshEditor> pEditor);
	void	GatherConnection(Ptr<CConnection> pConnection);
	void	Gather(CScene* pcScene);
	bool	WriteMeshes(void);
	bool	WriteImages(void);
	void	WriteMaterials(void);
	void	WriteConnections(void);
	void	WriteLights(CScene* pcScene);
	void	WriteCameras(CScene* pcScene);
	void	WriteAnimations(CScene* pcScene);
	void	WriteInstances(CScene* pcScene);
	void	WriteParticles(CScene* pcScene);
};


//////////////////////////////////////////////////////////////////////////
//
// The place of an object in a list, adding it if it isn't there.  What's
// in a file is few enough that looking through them is fine.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
static int32 SceneFileIndex(CArray<M>* papList, Ptr<M> pObject, bool bAdd)
{
	size	i;
	size	uiNum;

	if (pObject.IsNull())
	{
		return SCENE_FILE_NONE;
	}

	uiNum = papList->NumElements();
	for (i = 0; i < uiNum; i++)
	{
		if (&papList->Get(i) == &pObject)
		{
			return (int32)i;
		}
	}
	if (!bAdd)
	{
		return SCENE_FILE_NONE;
	}
	papList->Add(pObject);
	return (int32)uiNum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void SceneFileColourToARGB(CImageColourARGB* pcDest, float32* pafRGBA)
{
	pcDest->Init(pafRGBA[3], pafRGBA[0], pafRGBA[1], pafRGBA[2]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void SceneFileColourFromARGB(float32* pafRGBA, CImageColourARGB* pcSource)
{
	pafRGBA[0] = pcSource->mfRed;
	pafRGBA[1] = pcSource->mfGreen;
	pafRGBA[2] = pcSource->mfBlue;
	pafRGBA[3] = pcSource->mfAlpha;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneFileLoad::Init(const char* szFilename)
{
	PreInit();

	mcReader.Init(szFilename);
	mapConnections.Init();
	mapImages.Init();
	maszImageNames.Init();
	mapMaterials.Init();
	mapEditors.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneFileLoad::Free(void)
{
	maszImageNames.Kill();
	mcReader.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneFileLoad::Class(void)
{
	M_Embedded(mapConnections);
	M_Embedded(mapImages);
	M_Embedded(mapMaterials);
	M_Embedded(mapEditors);
}


//////////////////////////////////////////////////////////////////////////
//
// A connection's parent can come after it, so they're all made first.
// What's missing from a file keeps its default, as it does below.
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileLoad::ReadConnections(void)
{
	Ptr<CConnection>	pConnection;
	size				uiNum;
	size				i;
	int32				iParent;

	uiNum = mcReader.NumConnections();
	for (i = 0; i < uiNum; i++)
	{
		mapConnections.Add(OMalloc<CConnection>());
	}
	for (i = 0; i < uiNum; i++)
	{
		pConnection = mapConnections.Get(i);
		ReturnOnFalse(mcReader.BeginConnection(i));
		ReturnOnFalse(mcReader.ReadName(&pConnection->mszName));
		mcReader.ReadWorldMatrix((float32*)&pConnection->msWorldMatrix);
		mcReader.ReadLocalMatrix((float32*)&pConnection->msLocalMatrix);
		pConnection->mbParity = mcReader.GetParity();
		iParent = mcReader.GetParent();
		if (iParent != SCENE_FILE_NONE)
		{
			pConnection->mpParent = mapConnections.Get((size)iParent);
		}
		ReturnOnFalse(mcReader.End());
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileLoad::ReadImages(void)
{
	Ptr<CImage>	pImage;
	CChars		szName;
	CChars		szChannels;
	int32		iWidth;
	int32		iHeight;
	size		i;

	for (i = 0; i < mcReader.NumImages(); i++)
	{
		ReturnOnFalse(mcReader.BeginImage(i));
		szName.Init();
		szChannels.Init();
		ReturnOnFalse(mcReader.ReadName(&szName));
		ReturnOnFalse(mcReader.ReadImageSize(&iWidth, &iHeight));
		ReturnOnFalse(mcReader.ReadChannels(&szChannels));

		if (szChannels.Equals(SCENE_IMAGE_RGBA))
		{
			pImage = OMalloc<CImage>((uint32)iWidth, (uint32)iHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
		}
		else if (szChannels.Equals(SCENE_IMAGE_RGB))
		{
			pImage = OMalloc<CImage>((uint32)iWidth, (uint32)iHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		}
		else if (szChannels.Equals(SCENE_IMAGE_ALPHA))
		{
			pImage = OMalloc<CImage>((uint32)iWidth, (uint32)iHeight, PT_uint8, IMAGE_OPACITY, CHANNEL_STOP);
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Image [", szName.Text(), "] has channels [", szChannels.Text(), "] that aren't known.", NULL);
			szChannels.Kill();
			szName.Kill();
			return false;
		}
		ReturnOnFalse(mcReader.ReadPixels((uint8*)pImage->GetData()));
		ReturnOnFalse(mcReader.End());

		mapImages.Add(pImage);
		maszImageNames.Add(szName.Text());
		szChannels.Kill();
		szName.Kill();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileLoad::ReadMaterials(void)
{
	Ptr<CMaterial>		pMaterial;
	CMaterialImage*		pcImage;
	CMaterialProperties*	psProperties;
	float32				afColour[4];
	size				i;
	int32				j;
	int32				iImage;

	for (i = 0; i < mcReader.NumMaterials(); i++)
	{
		pMaterial = OMalloc<CMaterial>();
		psProperties = &pMaterial->msProperties;
		ReturnOnFalse(mcReader.BeginMaterial(i));
		ReturnOnFalse(mcReader.ReadName(&pMaterial->mszName));
		if (mcReader.ReadMaterialColour(SFMC_Diffuse, afColour))
		{
			SceneFileColourToARGB(&psProperties->mcDiffuseColour, afColour);
		}
		if (mcReader.ReadMaterialColour(SFMC_Ambient, afColour))
		{
			SceneFileColourToARGB(&psProperties->mcAmbientColour, afColour);
		}
		if (mcReader.ReadMaterialColour(SFMC_Emissive, afColour))
		{
			SceneFileColourToARGB(&psProperties->mcEmissiveColour, afColour);
		}
		if (mcReader.ReadMaterialColour(SFMC_Specular, afColour))
		{
			SceneFileColourToARGB(&psProperties->mcSpecularColour, afColour);
		}
		mcReader.ReadShininess(&psProperties->mfShininessPower, &psProperties->mfShininessBase);
		for (j = 0; j < MI_NumImages; j++)
		{
			iImage = mcReader.GetMaterialTexture((ESceneFileTexture)j);
			if (iImage != SCENE_FILE_NONE)
			{
				pcImage = pMaterial->GetImage((EMaterialImage)j);
				pcImage->mpImage = mapImages.Get((size)iImage);
				pcImage->mszImageName.Append(maszImageNames.GetText((size)iImage));
			}
		}
		ReturnOnFalse(mcReader.End());
		mapMaterials.Add(pMaterial);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileLoad::ReadMeshes(void)
{
	Ptr<CMeshEditor>	pEditor;
	size				i;

	for (i = 0; i < mcReader.NumMeshes(); i++)
	{
		pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
		ReturnOnFalse(ReadSceneFileMesh(&mcReader, i, &pEditor, &mapMaterials));
		mapEditors.Add(pEditor);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// What a light, camera or track is attached to, NULL if it isn't.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CConnection> CSceneFileLoad::GetAttached(void)
{
	int32	iConnection;

	iConnection = mcReader.GetAttached();
	if (iConnection == SCENE_FILE_NONE)
	{
		return NULL;
	}
	return mapConnections.Get((size)iConnection);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileLoad::ReadLights(CScene* pcScene)
{
	Ptr<CLight>	pLight;
	float32		afColour[4];
	size		i;

	for (i = 0; i < mcReader.NumLights(); i++)
	{
		pLight = OMalloc<CLight>();
		ReturnOnFalse(mcReader.BeginLight(i));
		ReturnOnFalse(mcReader.ReadName(&pLight->mszName));
		pLight->meType = (ELightType)mcReader.GetLightType();
		pLight->meDecay = (ELightDecayType)mcReader.GetLightDecay();
		pLight->mbCastShadows = mcReader.GetShadows();
		if (mcReader.ReadLightColour(afColour))
		{
			SceneFileColourToARGB(&pLight->msColour, afColour);
		}
		mcReader.ReadLightValues(&pLight->mfDecayStart, &pLight->mfRange, &pLight->mfIntensity);
		pLight->mpConnection = GetAttached();
		ReturnOnFalse(mcReader.End());
		pcScene->mapLights.Add(pLight);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileLoad::ReadCameras(CScene* pcScene)
{
	Ptr<CCamera>	pCamera;
	size			i;

	for (i = 0; i < mcReader.NumCameras(); i++)
	{
		pCamera = OMalloc<CCamera>();
		ReturnOnFalse(mcReader.BeginCamera(i));
		ReturnOnFalse(mcReader.ReadName(&pCamera->mszName));
		pCamera->meCameraType = (mcReader.GetCameraType() == SFCT_Orthographic) ? CAT_Orthonormal : CAT_Perspective;
		mcReader.ReadLens(&pCamera->mfFOV, &pCamera->mfNear, &pCamera->mfFar);
		pCamera->mpConnection = GetAttached();
		ReturnOnFalse(mcReader.End());
		pcScene->mapCameras.Add(pCamera);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileLoad::ReadAnimations(CScene* pcScene)
{
	Ptr<CSceneAnimation>		pAnimation;
	Ptr<CConnectionAnimation>	pTrack;
	CArrayLargeTemplatePrimitive<float32>	afTimes;
	CArrayLargeTemplatePrimitive<float32>	af4x4s;
	size						i;
	size						j;
	size						k;

	for (i = 0; i < mcReader.NumAnimations(); i++)
	{
		pAnimation = OMalloc<CSceneAnimation>();
		ReturnOnFalse(mcReader.BeginAnimation(i));
		ReturnOnFalse(mcReader.ReadName(&pAnimation->mszName));
		for (j = 0; j < mcReader.NumTracks(); j++)
		{
			pTrack = OMalloc<CConnectionAnimation>();
			ReturnOnFalse(mcReader.BeginTrack(j));
			pTrack->mpConnection = GetAttached();

			afTimes.Init();
			af4x4s.Init();
			afTimes.Resize(mcReader.NumKeys());
			af4x4s.Resize(mcReader.NumKeys() * SCENE_FILE_FLOATS_IN_MATRIX);
			if (afTimes.NumElements() > 0)
			{
				ReturnOnFalse(mcReader.ReadKeyTimes(afTimes.GetData()));
				ReturnOnFalse(mcReader.ReadKeyMatrices(af4x4s.GetData()));
			}
			for (k = 0; k < afTimes.NumElements(); k++)
			{
				pTrack->AddKey((SFloat4x4*)(af4x4s.GetData() + k * SCENE_FILE_FLOATS_IN_MATRIX), afTimes.GetValue(k));
			}
			af4x4s.Kill();
			afTimes.Kill();

			ReturnOnFalse(mcReader.End());
			pAnimation->mapTracks.Add(pTrack);
		}
		ReturnOnFalse(mcReader.End());
		pcScene->mapAnimations.Add(pAnimation);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileLoad::ReadInstanceMesh(CInstance* pcInstance)
{
	Ptr<CInstanceMesh>		pInstanceMesh;
	CArraySceneFileIndex	aiSlots;
	CArraySceneFileIndex	aiMaterials;
	size					i;
	int32					iSlot;
	bool					bResult;

	pInstanceMesh = OMalloc<CInstanceMesh>(mapEditors.Get((size)mcReader.GetInstanceMesh()));

	aiSlots.Init();
	aiMaterials.Init();
	aiSlots.Resize(mcReader.NumConnectionSlots());
	aiMaterials.Resize(mcReader.NumInstanceMaterials());
	if (aiSlots.NumElements() > 0)
	{
		ReturnOnFalse(mcReader.ReadConnectionSlots(aiSlots.GetData()));
	}
	if (aiMaterials.NumElements() > 0)
	{
		ReturnOnFalse(mcReader.ReadInstanceMaterials(aiMaterials.GetData()));
	}
	bResult = true;
	for (i = 0; bResult && (i < aiSlots.NumElements()); i++)
	{
		iSlot = aiSlots.GetValue(i);
		if ((iSlot < 0) || ((size)iSlot >= pcInstance->mapConnections.NumElements()))
		{
			bResult = gcLogger.Error2(__METHOD__, " Instance [", pcInstance->mszName.Text(), "] has a mesh using its connection ", IntToString(iSlot), ", but it has ", SizeToString(pcInstance->mapConnections.NumElements()), ".", NULL);
		}
		else
		{
			pInstanceMesh->mapConnections.Add(pcInstance->mapConnections.Get((size)iSlot));
		}
	}
	for (i = 0; bResult && (i < aiMaterials.NumElements()); i++)
	{
		pInstanceMesh->mapMaterials.Add((aiMaterials.GetValue(i) == SCENE_FILE_NONE) ? Ptr<CMaterial>() : mapMaterials.Get((size)aiMaterials.GetValue(i)));
	}
	aiMaterials.Kill();
	aiSlots.Kill();

	if (bResult)
	{
		pcInstance->mapMeshes.Add(pInstanceMesh);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileLoad::ReadInstances(CScene* pcScene)
{
	Ptr<CInstance>			pInstance;
	CArraySceneFileIndex	aiConnections;
	size					i;
	size					j;

	for (i = 0; i < mcReader.NumInstances(); i++)
	{
		pInstance = OMalloc<CInstance>();
		ReturnOnFalse(mcReader.BeginInstance(i));
		ReturnOnFalse(mcReader.ReadName(&pInstance->mszName));

		aiConnections.Init();
		aiConnections.Resize(mcReader.NumInstanceConnections());
		if (aiConnections.NumElements() > 0)
		{
			ReturnOnFalse(mcReader.ReadInstanceConnections(aiConnections.GetData()));
		}
		for (j = 0; j < aiConnections.NumElements(); j++)
		{
			pInstance->mapConnections.Add(mapConnections.Get((size)aiConnections.GetValue(j)));
		}
		aiConnections.Kill();

		for (j = 0; j < mcReader.NumInstanceMeshes(); j++)
		{
			ReturnOnFalse(mcReader.BeginInstanceMesh(j));
			ReturnOnFalse(ReadInstanceMesh(&pInstance));
			ReturnOnFalse(mcReader.End());
		}
		ReturnOnFalse(mcReader.End());
		pcScene->mapInstances.Add(pInstance);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Only a particle's position is needed.
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileLoad::ReadParticles(CScene* pcScene)
{
	Ptr<CSceneParticles>	pParticles;
	lsize					uiNum;
	size					i;

	for (i = 0; i < mcReader.NumParticleSets(); i++)
	{
		pParticles = OMalloc<CSceneParticles>();
		ReturnOnFalse(mcReader.BeginParticles(i));
		ReturnOnFalse(mcReader.ReadName(&pParticles->mszName));
		uiNum = mcReader.NumParticles();
		pParticles->mafPositions.Resize(uiNum * 3);
		ReturnOnFalse(mcReader.ReadParticlePositions(pParticles->mafPositions.GetData()));
		if (mcReader.NumParticleColours() > 0)
		{
			pParticles->mafColours.Resize(uiNum * 3);
			ReturnOnFalse(mcReader.ReadParticleColours(pParticles->mafColours.GetData()));
		}
		if (mcReader.NumParticleRadii() > 0)
		{
			pParticles->mafRadii.Resize(uiNum);
			ReturnOnFalse(mcReader.ReadParticleRadii(pParticles->mafRadii.GetData()));
		}
		if (mcReader.NumParticleIntensities() > 0)
		{
			pParticles->mafIntensities.Resize(uiNum);
			ReturnOnFalse(mcReader.ReadParticleIntensities(pParticles->mafIntensities.GetData()));
		}
		ReturnOnFalse(mcReader.End());
		pcScene->mapParticles.Add(pParticles);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileLoad::Read(CScene* pcScene)
{
	ReturnOnFalse(mcReader.Open());
	ReturnOnFalse(ReadConnections());
	ReturnOnFalse(ReadImages());
	ReturnOnFalse(ReadMaterials());
	ReturnOnFalse(ReadMeshes());
	ReturnOnFalse(ReadLights(pcScene));
	ReturnOnFalse(ReadCameras(pcScene));
	ReturnOnFalse(ReadAnimations(pcScene));
	ReturnOnFalse(ReadInstances(pcScene));
	ReturnOnFalse(ReadParticles(pcScene));
	return !mcReader.Failed();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneFileSave::Init(const char* szFilename)
{
	PreInit();

	mcWriter.Init(szFilename);
	mapConnections.Init();
	mapImages.Init();
	maszImageNames.Init();
	mapMaterials.Init();
	mapEditors.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneFileSave::Free(void)
{
	maszImageNames.Kill();
	mcWriter.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneFileSave::Class(void)
{
	M_Embedded(mapConnections);
	M_Embedded(mapImages);
	M_Embedded(mapMaterials);
	M_Embedded(mapEditors);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneFileSave::GatherMaterial(Ptr<CMaterial> pMaterial)
{
	CMaterialImage*	pcImage;
	size			uiImages;
	int32			j;

	if (pMaterial.IsNull())
	{
		return;
	}

	SceneFileIndex<CMaterial>(&mapMaterials, pMaterial, true);
	for (j = 0; j < MI_NumImages; j++)
	{
		pcImage = pMaterial->GetImage((EMaterialImage)j);
		uiImages = mapImages.NumElements();
		SceneFileIndex<CImage>(&mapImages, pcImage->mpImage, true);
		if (mapImages.NumElements() != uiImages)
		{
			maszImageNames.Add(pcImage->mszImageName.Text());
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneFileSave::GatherEditor(Ptr<CMeshEditor> pEditor)
{
	CMeshMaterials*	pcMaterials;
	size			i;

	SceneFileIndex<CMeshEditor>(&mapEditors, pEditor, true);
	pcMaterials = &pEditor->mpMesh->mcMaterials;
	for (i = 0; i < pcMaterials->mapMaterials.NumElements(); i++)
	{
		GatherMaterial(pcMaterials->mapMaterials.Get(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Its parents come too, as a parent may have no one else that uses it.
//
//////////////////////////////////////////////////////////////////////////
void CSceneFileSave::GatherConnection(Ptr<CConnection> pConnection)
{
	Ptr<CConnection>	pParent;

	pParent = pConnection;
	while (pParent.IsNotNull())
	{
		SceneFileIndex<CConnection>(&mapConnections, pParent, true);
		pParent = pParent->mpParent;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneFileSave::Gather(CScene* pcScene)
{
	Ptr<CInstance>			pInstance;
	Ptr<CInstanceMesh>		pInstanceMesh;
	Ptr<CSceneAnimation>	pAnimation;
	size					i;
	size					j;
	size					k;

	for (i = 0; i < pcScene->mapInstances.NumElements(); i++)
	{
		pInstance = pcScene->mapInstances.Get(i);
		for (j = 0; j < pInstance->mapConnections.NumElements(); j++)
		{
			GatherConnection(pInstance->mapConnections.Get(j));
		}
		for (j = 0; j < pInstance->mapMeshes.NumElements(); j++)
		{
			pInstanceMesh = pInstance->mapMeshes.Get(j);
			GatherEditor(pInstanceMesh->mpEditor);
			for (k = 0; k < pInstanceMesh->mapMaterials.NumElements(); k++)
			{
				GatherMaterial(pInstanceMesh->mapMaterials.Get(k));
			}
		}
	}
	for (i = 0; i < pcScene->mapLights.NumElements(); i++)
	{
		GatherConnection(pcScene->mapLights.Get(i)->mpConnection);
	}
	for (i = 0; i < pcScene->mapCameras.NumElements(); i++)
	{
		GatherConnection(pcScene->mapCameras.Get(i)->mpConnection);
	}
	for (i = 0; i < pcScene->mapAnimations.NumElements(); i++)
	{
		pAnimation = pcScene->mapAnimations.Get(i);
		for (j = 0; j < pAnimation->mapTracks.NumElements(); j++)
		{
			GatherConnection(pAnimation->mapTracks.Get(j)->mpConnection);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileSave::WriteMeshes(void)
{
	CArraySceneFileIndex	aiMaterials;
	Ptr<CMeshEditor>		pEditor;
	CMeshMaterials*			pcMaterials;
	size					i;
	size					j;
	bool					bResult;

	bResult = true;
	for (i = 0; (i < mapEditors.NumElements()) && bResult; i++)
	{
		pEditor = mapEditors.Get(i);
		pcMaterials = &pEditor->mpMesh->mcMaterials;
		aiMaterials.Init();
		for (j = 0; j < pcMaterials->mapMaterials.NumElements(); j++)
		{
			aiMaterials.Add(SceneFileIndex<CMaterial>(&mapMaterials, pcMaterials->mapMaterials.Get(j), false));
		}
		bResult = WriteSceneFileMesh(&mcWriter, &pEditor, &aiMaterials);
		aiMaterials.Kill();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// False for an image a file can't hold as it is.
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileSave::WriteImages(void)
{
	Ptr<CImage>	pImage;
	const char*	szChannels;
	size		i;

	for (i = 0; i < mapImages.NumElements(); i++)
	{
		pImage = mapImages.Get(i);
		szChannels = GetSceneFileImageChannels(&pImage);
		if (szChannels == NULL)
		{
			return gcLogger.Error2(__METHOD__, " Image [", maszImageNames.GetText(i), "] isn't bytes of RGBA, RGB or A.", NULL);
		}
		mcWriter.BeginImage(maszImageNames.GetText(i));
		mcWriter.WritePixels((int32)pImage->GetWidth(), (int32)pImage->GetHeight(), szChannels, (uint8*)pImage->GetData());
		mcWriter.EndImage();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneFileSave::WriteMaterials(void)
{
	Ptr<CMaterial>			pMaterial;
	CMaterialImage*			pcImage;
	CMaterialProperties*	psProperties;
	float32					afColour[4];
	size					i;
	int32					j;

	for (i = 0; i < mapMaterials.NumElements(); i++)
	{
		pMaterial = mapMaterials.Get(i);
		psProperties = &pMaterial->msProperties;
		mcWriter.BeginMaterial(pMaterial->mszName.Text());
		SceneFileColourFromARGB(afColour, &psProperties->mcDiffuseColour);
		mcWriter.WriteMaterialColour(SFMC_Diffuse, afColour);
		SceneFileColourFromARGB(afColour, &psProperties->mcAmbientColour);
		mcWriter.WriteMaterialColour(SFMC_Ambient, afColour);
		SceneFileColourFromARGB(afColour, &psProperties->mcEmissiveColour);
		mcWriter.WriteMaterialColour(SFMC_Emissive, afColour);
		SceneFileColourFromARGB(afColour, &psProperties->mcSpecularColour);
		mcWriter.WriteMaterialColour(SFMC_Specular, afColour);
		mcWriter.WriteShininess(psProperties->mfShininessPower, psProperties->mfShininessBase);
		for (j = 0; j < MI_NumImages; j++)
		{
			pcImage = pMaterial->GetImage((EMaterialImage)j);
			if (pcImage->mpImage.IsNotNull())
			{
				mcWriter.WriteMaterialTexture((ESceneFileTexture)j, SceneFileIndex<CImage>(&mapImages, pcImage->mpImage, false));
			}
		}
		mcWriter.EndMaterial();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneFileSave::WriteConnections(void)
{
	Ptr<CConnection>	pConnection;
	size				i;

	for (i = 0; i < mapConnections.NumElements(); i++)
	{
		pConnection = mapConnections.Get(i);
		mcWriter.BeginConnection(pConnection->mszName.Text());
		if (pConnection->mpParent.IsNotNull())
		{
			mcWriter.WriteParent(SceneFileIndex<CConnection>(&mapConnections, pConnection->mpParent, false));
		}
		mcWriter.WriteWorldMatrix((float32*)&pConnection->msWorldMatrix);
		mcWriter.WriteLocalMatrix((float32*)&pConnection->msLocalMatrix);
		mcWriter.WriteParity(pConnection->mbParity);
		mcWriter.EndConnection();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneFileSave::WriteLights(CScene* pcScene)
{
	Ptr<CLight>	pLight;
	float32		afColour[4];
	size		i;

	for (i = 0; i < pcScene->mapLights.NumElements(); i++)
	{
		pLight = pcScene->mapLights.Get(i);
		mcWriter.BeginLight(pLight->mszName.Text());
		mcWriter.WriteLightType((ESceneFileLightType)pLight->meType);
		SceneFileColourFromARGB(afColour, &pLight->msColour);
		mcWriter.WriteLightColour(afColour);
		mcWriter.WriteLightValues(pLight->mfDecayStart, pLight->mfRange, pLight->mfIntensity);
		mcWriter.WriteLightDecay((ESceneFileLightDecay)pLight->meDecay);
		mcWriter.WriteShadows(pLight->mbCastShadows);
		if (pLight->mpConnection.IsNotNull())
		{
			mcWriter.WriteAttached(SceneFileIndex<CConnection>(&mapConnections, pLight->mpConnection, false));
		}
		mcWriter.EndLight();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneFileSave::WriteCameras(CScene* pcScene)
{
	Ptr<CCamera>	pCamera;
	size			i;

	for (i = 0; i < pcScene->mapCameras.NumElements(); i++)
	{
		pCamera = pcScene->mapCameras.Get(i);
		mcWriter.BeginCamera(pCamera->mszName.Text());
		mcWriter.WriteCameraType((pCamera->meCameraType == CAT_Orthonormal) ? SFCT_Orthographic : SFCT_Perspective);
		mcWriter.WriteLens(pCamera->mfFOV, pCamera->mfNear, pCamera->mfFar);
		if (pCamera->mpConnection.IsNotNull())
		{
			mcWriter.WriteAttached(SceneFileIndex<CConnection>(&mapConnections, pCamera->mpConnection, false));
		}
		mcWriter.EndCamera();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A scene file's keys are matrices.  A key of another kind is only made by
// code nothing calls yet, and its data isn't one.
//
//////////////////////////////////////////////////////////////////////////
void CSceneFileSave::WriteAnimations(CScene* pcScene)
{
	Ptr<CSceneAnimation>				pAnimation;
	Ptr<CConnectionAnimation>			pTrack;
	SConnectionKey*						psKey;
	CArrayLargeTemplatePrimitive<float32>	afTimes;
	CArrayLargeTemplatePrimitive<float32>	af4x4s;
	size								i;
	size								j;
	size								k;

	for (i = 0; i < pcScene->mapAnimations.NumElements(); i++)
	{
		pAnimation = pcScene->mapAnimations.Get(i);
		mcWriter.BeginAnimation(pAnimation->mszName.Text());
		for (j = 0; j < pAnimation->mapTracks.NumElements(); j++)
		{
			pTrack = pAnimation->mapTracks.Get(j);
			afTimes.Init();
			af4x4s.Init();
			for (k = 0; k < pTrack->NumKeys(); k++)
			{
				psKey = pTrack->GetKey(k);
				afTimes.Add(psKey->fTime);
				af4x4s.Add((float32*)&psKey->sMatrix, SCENE_FILE_FLOATS_IN_MATRIX);
			}
			mcWriter.WriteTrack(SceneFileIndex<CConnection>(&mapConnections, pTrack->mpConnection, false), afTimes.GetData(), af4x4s.GetData(), afTimes.NumElements());
			af4x4s.Kill();
			afTimes.Kill();
		}
		mcWriter.EndAnimation();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneFileSave::WriteInstances(CScene* pcScene)
{
	Ptr<CInstance>			pInstance;
	Ptr<CInstanceMesh>		pInstanceMesh;
	CArraySceneFileIndex	aiConnections;
	CArraySceneFileIndex	aiSlots;
	CArraySceneFileIndex	aiMaterials;
	size					i;
	size					j;
	size					k;

	for (i = 0; i < pcScene->mapInstances.NumElements(); i++)
	{
		pInstance = pcScene->mapInstances.Get(i);
		mcWriter.BeginInstance(pInstance->mszName.Text());

		aiConnections.Init();
		for (j = 0; j < pInstance->mapConnections.NumElements(); j++)
		{
			aiConnections.Add(SceneFileIndex<CConnection>(&mapConnections, pInstance->mapConnections.Get(j), false));
		}
		mcWriter.WriteConnections(aiConnections.GetData(), aiConnections.NumElements());
		aiConnections.Kill();

		for (j = 0; j < pInstance->mapMeshes.NumElements(); j++)
		{
			pInstanceMesh = pInstance->mapMeshes.Get(j);
			aiSlots.Init();
			aiMaterials.Init();
			for (k = 0; k < pInstanceMesh->mapConnections.NumElements(); k++)
			{
				aiSlots.Add(SceneFileIndex<CConnection>(&pInstance->mapConnections, pInstanceMesh->mapConnections.Get(k), false));
			}
			for (k = 0; k < pInstanceMesh->mapMaterials.NumElements(); k++)
			{
				aiMaterials.Add(SceneFileIndex<CMaterial>(&mapMaterials, pInstanceMesh->mapMaterials.Get(k), false));
			}
			mcWriter.WriteInstanceMesh(SceneFileIndex<CMeshEditor>(&mapEditors, pInstanceMesh->mpEditor, false), aiSlots.GetData(), aiSlots.NumElements(), aiMaterials.GetData(), aiMaterials.NumElements());
			aiMaterials.Kill();
			aiSlots.Kill();
		}
		mcWriter.EndInstance();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneFileSave::WriteParticles(CScene* pcScene)
{
	Ptr<CSceneParticles>	pParticles;
	size					i;

	for (i = 0; i < pcScene->mapParticles.NumElements(); i++)
	{
		pParticles = pcScene->mapParticles.Get(i);
		mcWriter.WriteParticles(pParticles->mszName.Text(), pParticles->mafPositions.GetData(),
								(pParticles->mafColours.NumElements() > 0) ? pParticles->mafColours.GetData() : NULL,
								(pParticles->mafRadii.NumElements() > 0) ? pParticles->mafRadii.GetData() : NULL,
								(pParticles->mafIntensities.NumElements() > 0) ? pParticles->mafIntensities.GetData() : NULL,
								pParticles->NumParticles());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileSave::Write(CScene* pcScene)
{
	Gather(pcScene);

	ReturnOnFalse(mcWriter.Open());
	ReturnOnFalse(WriteMeshes());
	ReturnOnFalse(WriteImages());
	WriteMaterials();
	WriteConnections();
	WriteLights(pcScene);
	WriteCameras(pcScene);
	WriteAnimations(pcScene);
	WriteInstances(pcScene);
	WriteParticles(pcScene);
	return mcWriter.Close();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScene::Init(void)
{
	PreInit();

	mapInstances.Init();
	mapLights.Init();
	mapCameras.Init();
	mapAnimations.Init();
	mapParticles.Init();
	muiUpdates = 0;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScene::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScene::Class(void)
{
	M_Embedded(mapInstances);
	M_Embedded(mapLights);
	M_Embedded(mapCameras);
	M_Embedded(mapAnimations);
	M_Embedded(mapParticles);
	U_Size(muiUpdates);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CScene::ReadFile(const char* szFilename)
{
	Ptr<CSceneFileLoad>	pLoad;

	pLoad = OMalloc<CSceneFileLoad>(szFilename);
	return pLoad->Read(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CScene::WriteFile(const char* szFilename)
{
	Ptr<CSceneFileSave>	pSave;

	pSave = OMalloc<CSceneFileSave>(szFilename);
	return pSave->Write(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScene::UpdateConnections(void)
{
	Ptr<CInstance>			pInstance;
	Ptr<CSceneAnimation>	pAnimation;
	Ptr<CConnection>		pConnection;
	size					i;
	size					j;

	muiUpdates++;
	for (i = 0; i < mapInstances.NumElements(); i++)
	{
		pInstance = mapInstances.Get(i);
		for (j = 0; j < pInstance->mapConnections.NumElements(); j++)
		{
			pInstance->mapConnections.Get(j)->UpdateWorldMatrix(muiUpdates);
		}
	}
	for (i = 0; i < mapLights.NumElements(); i++)
	{
		pConnection = mapLights.Get(i)->mpConnection;
		if (pConnection.IsNotNull())
		{
			pConnection->UpdateWorldMatrix(muiUpdates);
		}
	}
	for (i = 0; i < mapCameras.NumElements(); i++)
	{
		pConnection = mapCameras.Get(i)->mpConnection;
		if (pConnection.IsNotNull())
		{
			pConnection->UpdateWorldMatrix(muiUpdates);
		}
	}
	for (i = 0; i < mapAnimations.NumElements(); i++)
	{
		pAnimation = mapAnimations.Get(i);
		for (j = 0; j < pAnimation->mapTracks.NumElements(); j++)
		{
			pConnection = pAnimation->mapTracks.Get(j)->mpConnection;
			if (pConnection.IsNotNull())
			{
				pConnection->UpdateWorldMatrix(muiUpdates);
			}
		}
	}
}


/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RenderSceneFile

Codaphela RenderSceneFile is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RenderSceneFile is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RenderSceneFile.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BaseLib/ArrayTemplatePrimitive.h"
#include "BaseLib/Float32Vec3.h"
#include "BaseLib/Float4x4.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/LeakDetector.h"
#include "BaseLib/Logger.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/TypeNames.h"
#include "RasterLib/Rasteriser.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Image.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/SceneFileReader.h"


//RenderSceneFile <scene .DRG> <out .png> [--key <n>] [--yaw <degrees>] [--pitch <degrees>] [--camera <n>] [--eye <x,y,z>] [--instance <n>]
//                [--size <pixels>] [--particle-scale <n>]
//
//Draws a scene file with RasterLib, to look at what's in it.  Everything is posed in floats here and given to RasterLib as a
//CRasterMesh already where it is on the screen, as RasterLib has no skinning, connections or textures yet.  A texture is shown by
//cutting up each triangle and giving the pieces the colour of the texture beneath them.
//
//--key poses it at that key of each track, a key being its connection's local matrix, and draws the connections' world matrices if it isn't
//given.  The view turns around the middle of what's
//drawn, unless --camera looks through a camera in the file or --eye looks along -z from a place in the world.  --instance only draws that
//instance.  Particles are drawn where they are unless --particle-scale spreads them around the origin: Goober's are all in a box of
//size 1, and whatever made them bigger in the Viewer isn't in its file.


#define RENDER_DEFAULT_SIZE		800
#define RENDER_MAX_CUTS			16
#define RENDER_TEXELS_PER_CUT	4.0f
#define RENDER_ORBIT_EYE		4
#define RENDER_CAMERA_UNITS		(1 << 24)
#define RENDER_BACKGROUND		0xFF101018
#define RENDER_NO_MATERIAL		0xFFB0B0B0
#define RENDER_DEGREES			(3.14159265f / 180.0f)
#define RENDER_FLOATS_IN_MATRIX	SCENE_FILE_FLOATS_IN_MATRIX
#define RENDER_EYE_FIELD_OF_VIEW	0.785f


typedef CArrayTemplatePrimitive<float32>	CArrayRenderFloats;
typedef CArrayTemplatePrimitive<int32>	CArrayRenderInts;
typedef CArrayTemplatePrimitive<int32>		CArrayRenderInts;
typedef CArrayTemplatePrimitive<uint32>		CArrayRenderUints;
typedef CArrayTemplatePrimitive<uint8>		CArrayRenderBytes;


struct SRenderOptions
{
	const char*	szScene;
	const char*	szOutput;
	int32		iKey;
	int32		iCamera;
	int32		iInstance;
	int32		iSize;
	float32		fYaw;
	float32		fPitch;
	float32		fParticleScale;
	bool		bEye;
	float32		afEye[3];
};


struct SRenderMaterial
{
	float32	afDiffuse[4];
	int32	iTexture;
};


struct SRenderCorner
{
	SFloat32Vec3	sPosition;
	SFloat32Vec3	sNormal;
	float32			u;
	float32			v;
};


struct SRenderTriangle
{
	SRenderCorner	asCorners[3];
	int32			iMaterial;
};


struct SRenderView
{
	SFloat4x4	sToView;
	float32		fScale;
	int32		iEyeDistance;
	int32		iZOffset;
	int32		iFocalLength;
};


class CRenderImage
{
public:
	int32				miWidth;
	int32				miHeight;
	int32				miChannels;
	CArrayRenderBytes	mauiPixels;

	void	Init(void);
	void	Kill(void);
	uint32	Sample(float32 u, float32 v);
};


typedef CArrayTemplate<SRenderMaterial>	CArrayRenderMaterial;
typedef CArrayTemplate<CRenderImage>	CArrayRenderImage;
typedef CArrayTemplate<SRenderTriangle>	CArrayRenderTriangle;


//A scene file's triangles where they are in the world, and what's needed to colour them.
class CRenderScene
{
public:
	CSceneFileReader		mcReader;
	CArrayRenderFloats		mafWorlds;
	CArrayRenderInts		maiSkeletons;	//The skeleton each connection plays a bone of, SCENE_FILE_NONE if none.
	CArrayRenderInts		maiBones;		//And that bone.
	CArrayRenderMaterial	masMaterials;
	CArrayRenderImage		macImages;
	CArrayRenderTriangle	masTriangles;
	SFloat32Vec3			msMin;
	SFloat32Vec3			msMax;

	bool	Init(const char* szFilename);
	void	Kill(void);

	bool	Load(SRenderOptions* psOptions);
	bool	GetView(SRenderOptions* psOptions, SRenderView* psView);
	void	Build(SRenderOptions* psOptions, SRenderView* psView, CRasterMesh* pcMesh);

protected:
	bool	LoadWorlds(int32 iKey);
	bool	LoadImages(void);
	bool	LoadMaterials(void);
	bool	AddInstance(size uiInstance);
	bool	AddMesh(int32 iMesh, CArrayRenderInts* paiConnections, CArrayRenderInts* paiSlots);
	bool	PosePositions(CArrayRenderFloats* pafPositions, CArrayRenderInts* paiConnections, CArrayRenderInts* paiSlots);
	bool	GetSkinSlots(CArrayRenderInts* paiConnections, CArrayRenderInts* paiSlots);
	void	SetBoneLocals(int32 iSkeleton, int32 iBone, float32* pafKey, CArrayRenderFloats* pafLocals);
	void	AddBuiltTriangle(SRenderTriangle* psTriangle, SRenderView* psView, CRasterMesh* pcMesh);
	bool	AddParticles(SRenderView* psView, CRasterMesh* pcMesh, float32 fScale);
	void	SetLookThrough(SRenderOptions* psOptions, SRenderView* psView, float32 fRadius, float32 fFieldOfView);
	int32	GetCuts(SRenderTriangle* psTriangle);
	uint32	GetColour(int32 iMaterial, float32 u, float32 v);
	void	GetWorld(SFloat4x4* psDest, int32 iConnection);
	void	Bound(SFloat32Vec3* psPosition);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRenderImage::Init(void)
{
	miWidth = 0;
	miHeight = 0;
	miChannels = 0;
	mauiPixels.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRenderImage::Kill(void)
{
	mauiPixels.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// 0xAARRGGBB.  UVs wrap.
//
//////////////////////////////////////////////////////////////////////////
uint32 CRenderImage::Sample(float32 u, float32 v)
{
	int32	x;
	int32	y;
	uint8*	puiPixel;

	x = (int32)floorf((u - floorf(u)) * (float32)miWidth);
	y = (int32)floorf((v - floorf(v)) * (float32)miHeight);
	x = (x >= miWidth) ? miWidth - 1 : x;
	y = (y >= miHeight) ? miHeight - 1 : y;
	puiPixel = mauiPixels.GetData() + ((size)y * (size)miWidth + (size)x) * (size)miChannels;

	if (miChannels == 1)
	{
		return 0xFF000000 | ((uint32)puiPixel[0] << 16) | ((uint32)puiPixel[0] << 8) | puiPixel[0];
	}
	return ((miChannels == 4) ? ((uint32)puiPixel[3] << 24) : 0xFF000000) | ((uint32)puiPixel[0] << 16) | ((uint32)puiPixel[1] << 8) | puiPixel[2];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRenderScene::Init(const char* szFilename)
{
	mafWorlds.Init();
	maiSkeletons.Init();
	maiBones.Init();
	masMaterials.Init();
	macImages.Init();
	masTriangles.Init();
	msMin.Init(1e30f, 1e30f, 1e30f);
	msMax.Init(-1e30f, -1e30f, -1e30f);
	mcReader.Init(szFilename);
	return mcReader.Open();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRenderScene::Kill(void)
{
	size	i;

	mcReader.Kill();
	masTriangles.Kill();
	for (i = 0; i < macImages.NumElements(); i++)
	{
		macImages.Get(i)->Kill();
	}
	macImages.Kill();
	masMaterials.Kill();
	maiBones.Kill();
	maiSkeletons.Kill();
	mafWorlds.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRenderScene::Load(SRenderOptions* psOptions)
{
	size	i;

	ReturnOnFalse(LoadWorlds(psOptions->iKey));
	ReturnOnFalse(LoadImages());
	ReturnOnFalse(LoadMaterials());

	for (i = 0; i < mcReader.NumInstances(); i++)
	{
		if ((psOptions->iInstance == SCENE_FILE_NONE) || (psOptions->iInstance == (int32)i))
		{
			ReturnOnFalse(AddInstance(i));
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Connection uiConnection's world matrix: its local one times its parent's
// world one, which is made first, or its local one with no parent.
// pabMade stops a loop of parents going round for ever.
//
//////////////////////////////////////////////////////////////////////////
static void MakeRenderWorld(size uiConnection, CArrayRenderFloats* pafLocals, CArrayRenderInts* paiParents, bool* pabMade, CArrayRenderFloats* pafWorlds)
{
	float32*	pfWorld;
	float32*	pfLocal;
	int32		iParent;

	if (pabMade[uiConnection])
	{
		return;
	}
	pabMade[uiConnection] = true;

	pfWorld = pafWorlds->GetData() + uiConnection * RENDER_FLOATS_IN_MATRIX;
	pfLocal = pafLocals->GetData() + uiConnection * RENDER_FLOATS_IN_MATRIX;
	iParent = paiParents->GetValue(uiConnection);
	if ((iParent < 0) || ((size)iParent >= paiParents->NumElements()))
	{
		memcpy(pfWorld, pfLocal, sizeof(float32) * RENDER_FLOATS_IN_MATRIX);
		return;
	}
	MakeRenderWorld((size)iParent, pafLocals, paiParents, pabMade, pafWorlds);
	Float4x4Multiply((SFloat4x4*)pfWorld, (SFloat4x4*)pfLocal, (SFloat4x4*)(pafWorlds->GetData() + (size)iParent * RENDER_FLOATS_IN_MATRIX));
}


//////////////////////////////////////////////////////////////////////////
//
// Each connection's world matrix, or at a key the one that key makes: a
// track's key is its connection's local matrix there, and a connection's
// world matrix is its local one times its parent's.  A track with fewer
// keys holds its last.
//
//////////////////////////////////////////////////////////////////////////
bool CRenderScene::LoadWorlds(int32 iKey)
{
	CArrayRenderFloats	afKeys;
	CArrayRenderFloats	afLocals;
	CArrayRenderInts	aiParents;
	bool*				abMade;
	size				uiNumConnections;
	size				uiKey;
	size				i;
	size				j;
	int32				iSkeleton;
	int32				iBone;

	uiNumConnections = mcReader.NumConnections();
	mafWorlds.Resize(uiNumConnections * RENDER_FLOATS_IN_MATRIX);
	afLocals.Init();
	afLocals.Resize(uiNumConnections * RENDER_FLOATS_IN_MATRIX);
	aiParents.Init();
	for (i = 0; i < uiNumConnections; i++)
	{
		ReturnOnFalse(mcReader.BeginConnection(i));
		ReturnOnFalse(mcReader.ReadWorldMatrix(mafWorlds.GetData() + i * RENDER_FLOATS_IN_MATRIX));
		ReturnOnFalse(mcReader.ReadLocalMatrix(afLocals.GetData() + i * RENDER_FLOATS_IN_MATRIX));
		aiParents.Add(mcReader.GetParent());
		mcReader.GetSkeletonBone(&iSkeleton, &iBone);
		maiSkeletons.Add(iSkeleton);
		maiBones.Add(iBone);
		ReturnOnFalse(mcReader.End());
	}

	if ((iKey == SCENE_FILE_NONE) || (mcReader.NumAnimations() == 0))
	{
		aiParents.Kill();
		afLocals.Kill();
		return true;
	}

	ReturnOnFalse(mcReader.BeginAnimation(0));
	iSkeleton = mcReader.GetAnimationSkeleton();
	for (j = 0; j < mcReader.NumTracks(); j++)
	{
		ReturnOnFalse(mcReader.BeginTrack(j));
		afKeys.Init();
		afKeys.Resize(mcReader.NumKeys() * RENDER_FLOATS_IN_MATRIX);
		ReturnOnFalse(mcReader.ReadKeyMatrices(afKeys.GetData()));
		uiKey = ((size)iKey >= mcReader.NumKeys()) ? mcReader.NumKeys() - 1 : (size)iKey;
		if (iSkeleton != SCENE_FILE_NONE)
		{
			SetBoneLocals(iSkeleton, mcReader.GetBone(), afKeys.GetData() + uiKey * RENDER_FLOATS_IN_MATRIX, &afLocals);
		}
		else
		{
			memcpy(afLocals.GetData() + (size)mcReader.GetAttached() * RENDER_FLOATS_IN_MATRIX, afKeys.GetData() + uiKey * RENDER_FLOATS_IN_MATRIX, sizeof(float32) * RENDER_FLOATS_IN_MATRIX);
		}
		afKeys.Kill();
		ReturnOnFalse(mcReader.End());
	}

	abMade = (bool*)malloc(uiNumConnections * sizeof(bool));
	memset(abMade, 0, uiNumConnections * sizeof(bool));
	for (i = 0; i < uiNumConnections; i++)
	{
		MakeRenderWorld(i, &afLocals, &aiParents, abMade, &mafWorlds);
	}
	free(abMade);
	aiParents.Kill();
	afLocals.Kill();
	return mcReader.End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRenderScene::LoadImages(void)
{
	CRenderImage*	pcImage;
	CChars			szChannels;
	size			i;

	for (i = 0; i < mcReader.NumImages(); i++)
	{
		pcImage = macImages.Add();
		pcImage->Init();
		ReturnOnFalse(mcReader.BeginImage(i));
		ReturnOnFalse(mcReader.ReadImageSize(&pcImage->miWidth, &pcImage->miHeight));
		szChannels.Init();
		ReturnOnFalse(mcReader.ReadChannels(&szChannels));
		pcImage->miChannels = (int32)szChannels.Length();
		szChannels.Kill();
		pcImage->mauiPixels.Resize(mcReader.NumPixelBytes());
		ReturnOnFalse(mcReader.ReadPixels(pcImage->mauiPixels.GetData()));
		ReturnOnFalse(mcReader.End());
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRenderScene::LoadMaterials(void)
{
	SRenderMaterial*	psMaterial;
	size				i;

	for (i = 0; i < mcReader.NumMaterials(); i++)
	{
		psMaterial = masMaterials.Add();
		ReturnOnFalse(mcReader.BeginMaterial(i));
		ReturnOnFalse(mcReader.ReadMaterialColour(SFMC_Diffuse, psMaterial->afDiffuse));
		psMaterial->iTexture = mcReader.GetMaterialTexture(SFTX_Diffuse);
		ReturnOnFalse(mcReader.End());
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRenderScene::AddInstance(size uiInstance)
{
	CArrayRenderInts	aiConnections;
	CArrayRenderInts	aiSlots;
	size				i;
	int32				iMesh;
	bool				bResult;

	aiConnections.Init();
	ReturnOnFalse(mcReader.BeginInstance(uiInstance));
	aiConnections.Resize(mcReader.NumInstanceConnections());
	ReturnOnFalse(mcReader.ReadInstanceConnections(aiConnections.GetData()));
	bResult = true;
	for (i = 0; (i < mcReader.NumInstanceMeshes()) && bResult; i++)
	{
		ReturnOnFalse(mcReader.BeginInstanceMesh(i));
		iMesh = mcReader.GetInstanceMesh();
		aiSlots.Init();
		aiSlots.Resize(mcReader.NumConnectionSlots());
		if (aiSlots.NumElements() > 0)
		{
			ReturnOnFalse(mcReader.ReadConnectionSlots(aiSlots.GetData()));
		}
		ReturnOnFalse(mcReader.End());

		//The mesh is read outside the instance, as blocks don't nest, and then back to where it was.
		ReturnOnFalse(mcReader.End());
		bResult = AddMesh(iMesh, &aiConnections, &aiSlots);
		aiSlots.Kill();
		ReturnOnFalse(mcReader.BeginInstance(uiInstance));
	}
	aiConnections.Kill();
	ReturnOnFalse(mcReader.End());
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRenderScene::GetWorld(SFloat4x4* psDest, int32 iConnection)
{
	memcpy(psDest, mafWorlds.GetData() + (size)iConnection * RENDER_FLOATS_IN_MATRIX, sizeof(SFloat4x4));
}


//////////////////////////////////////////////////////////////////////////
//
// A skinned mesh, with vectors on the left of row-major matrices, adds up
// for each of its weights:
//   fWeight * (position * inverse[iMatrix] * world of connection[slot[iMatrix]])
// A rigid one is at its first connection.
//
//////////////////////////////////////////////////////////////////////////
bool CRenderScene::PosePositions(CArrayRenderFloats* pafPositions, CArrayRenderInts* paiConnections, CArrayRenderInts* paiSlots)
{
	CArrayRenderInts	aiCounts;
	CArrayRenderInts	aiMatrices;
	CArrayRenderFloats	afWeights;
	CArrayRenderFloats	afInverses;
	CArrayRenderInts	aiSkinSlots;
	CArrayRenderFloats	afPosed;
	CArrayTemplate<SFloat4x4>	asPalette;
	SFloat4x4			sWorld;
	SFloat4x4			sInverse;
	SFloat32Vec3		sPosition;
	SFloat32Vec3		sWeighted;
	SFloat32Vec3		sSum;
	size				uiNumPositions;
	size				uiWeight;
	size				i;
	int32				j;
	bool				bSkinned;

	uiNumPositions = pafPositions->NumElements() / 3;
	bSkinned = mcReader.NumSkinCounts() > 0;
	if (!bSkinned)
	{
		GetWorld(&sWorld, paiConnections->GetValue((size)paiSlots->GetValue(0)));
		for (i = 0; i < uiNumPositions; i++)
		{
			sPosition.Init(pafPositions->GetValue(i * 3), pafPositions->GetValue(i * 3 + 1), pafPositions->GetValue(i * 3 + 2));
			Float32Vec3TransformCoord(&sWeighted, &sPosition, &sWorld);
			memcpy(pafPositions->GetData() + i * 3, &sWeighted, sizeof(float32) * 3);
		}
		return true;
	}

	aiCounts.Init();
	aiMatrices.Init();
	afWeights.Init();
	afInverses.Init();
	aiSkinSlots.Init();
	aiCounts.Resize(mcReader.NumSkinCounts());
	aiMatrices.Resize(mcReader.NumSkinWeights());
	afWeights.Resize(mcReader.NumSkinWeights());
	afInverses.Resize(mcReader.NumInverseMatrices() * RENDER_FLOATS_IN_MATRIX);
	ReturnOnFalse(mcReader.ReadSkinCounts(aiCounts.GetData()));
	ReturnOnFalse(mcReader.ReadSkinMatrices(aiMatrices.GetData()));
	ReturnOnFalse(mcReader.ReadSkinWeights(afWeights.GetData()));
	ReturnOnFalse(mcReader.ReadInverseMatrices(afInverses.GetData()));
	if (mcReader.GetSkinSkeleton() != SCENE_FILE_NONE)
	{
		ReturnOnFalse(GetSkinSlots(paiConnections, &aiSkinSlots));
		paiSlots = &aiSkinSlots;
	}

	asPalette.Init();
	for (i = 0; i < mcReader.NumInverseMatrices(); i++)
	{
		memcpy(&sInverse, afInverses.GetData() + i * RENDER_FLOATS_IN_MATRIX, sizeof(SFloat4x4));
		GetWorld(&sWorld, paiConnections->GetValue((size)paiSlots->GetValue(i)));
		Float4x4Multiply(asPalette.Add(), &sInverse, &sWorld);
	}

	afPosed.Init();
	afPosed.Resize(uiNumPositions * 3);
	uiWeight = 0;
	for (i = 0; i < uiNumPositions; i++)
	{
		sPosition.Init(pafPositions->GetValue(i * 3), pafPositions->GetValue(i * 3 + 1), pafPositions->GetValue(i * 3 + 2));
		sSum.Init(0.0f, 0.0f, 0.0f);
		for (j = 0; j < aiCounts.GetValue(i); j++)
		{
			Float32Vec3TransformCoord(&sWeighted, &sPosition, asPalette.Get((size)aiMatrices.GetValue(uiWeight)));
			sSum.x += sWeighted.x * afWeights.GetValue(uiWeight);
			sSum.y += sWeighted.y * afWeights.GetValue(uiWeight);
			sSum.z += sWeighted.z * afWeights.GetValue(uiWeight);
			uiWeight++;
		}
		memcpy(afPosed.GetData() + i * 3, &sSum, sizeof(float32) * 3);
	}
	memcpy(pafPositions->GetData(), afPosed.GetData(), uiNumPositions * 3 * sizeof(float32));

	afPosed.Kill();
	asPalette.Kill();
	aiSkinSlots.Kill();
	afInverses.Kill();
	afWeights.Kill();
	aiMatrices.Kill();
	aiCounts.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A skin posed by a skeleton's bones uses the instance's connections for
// them: the slot of each of its matrices is the place in the instance's
// connections of the one playing its bone.
//
//////////////////////////////////////////////////////////////////////////
bool CRenderScene::GetSkinSlots(CArrayRenderInts* paiConnections, CArrayRenderInts* paiSlots)
{
	CArrayRenderInts	aiBones;
	size				i;
	size				j;
	int32				iSkeleton;
	int32				iConnection;

	iSkeleton = mcReader.GetSkinSkeleton();
	aiBones.Init();
	aiBones.Resize(mcReader.NumSkinBones());
	ReturnOnFalse(mcReader.ReadSkinBones(aiBones.GetData()));
	for (i = 0; i < aiBones.NumElements(); i++)
	{
		for (j = 0; j < paiConnections->NumElements(); j++)
		{
			iConnection = paiConnections->GetValue(j);
			if ((maiSkeletons.GetValue((size)iConnection) == iSkeleton) && (maiBones.GetValue((size)iConnection) == aiBones.GetValue(i)))
			{
				break;
			}
		}
		if (j == paiConnections->NumElements())
		{
			aiBones.Kill();
			printf("An instance hasn't a connection for one of its skin's bones.\n");
			return false;
		}
		paiSlots->Add((int32)j);
	}
	aiBones.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A skeleton's animation poses every connection playing the bone.
//
//////////////////////////////////////////////////////////////////////////
void CRenderScene::SetBoneLocals(int32 iSkeleton, int32 iBone, float32* pafKey, CArrayRenderFloats* pafLocals)
{
	size	i;

	for (i = 0; i < maiSkeletons.NumElements(); i++)
	{
		if ((maiSkeletons.GetValue(i) == iSkeleton) && (maiBones.GetValue(i) == iBone))
		{
			memcpy(pafLocals->GetData() + i * RENDER_FLOATS_IN_MATRIX, pafKey, sizeof(float32) * RENDER_FLOATS_IN_MATRIX);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRenderScene::Bound(SFloat32Vec3* psPosition)
{
	msMin.x = (psPosition->x < msMin.x) ? psPosition->x : msMin.x;
	msMin.y = (psPosition->y < msMin.y) ? psPosition->y : msMin.y;
	msMin.z = (psPosition->z < msMin.z) ? psPosition->z : msMin.z;
	msMax.x = (psPosition->x > msMax.x) ? psPosition->x : msMax.x;
	msMax.y = (psPosition->y > msMax.y) ? psPosition->y : msMax.y;
	msMax.z = (psPosition->z > msMax.z) ? psPosition->z : msMax.z;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void RenderNormalise(SFloat32Vec3* psVec)
{
	float32	fLength;

	fLength = sqrtf(psVec->x * psVec->x + psVec->y * psVec->y + psVec->z * psVec->z);
	if (fLength > 0.0f)
	{
		psVec->x /= fLength;
		psVec->y /= fLength;
		psVec->z /= fLength;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the triangles of a mesh, posed.  Normals come from its smoothing
// groups: a corner's is the sum of the faces around its position that share
// a group with its face.  A face in no group is flat.
//
//////////////////////////////////////////////////////////////////////////
bool CRenderScene::AddMesh(int32 iMesh, CArrayRenderInts* paiConnections, CArrayRenderInts* paiSlots)
{
	CArrayRenderFloats				afPositions;
	CArrayRenderFloats				afUVs;
	CArrayRenderInts				aiCorners;
	CArrayRenderInts				aiCornerUVs;
	CArrayRenderInts				aiSizes;
	CArrayRenderInts				aiMaterialSlots;
	CArrayRenderInts				aiPolygonSlots;
	CArrayRenderUints				auiGroups;
	CArrayRenderInts				aiFaceHeads;
	CArrayRenderInts				aiFaceNexts;
	CArrayTemplate<SFloat32Vec3>	asFaceNormals;
	SFloat32Vec3					sA;
	SFloat32Vec3					sB;
	SFloat32Vec3*					psFaceNormal;
	SFloat32Vec3*					psCornerPosition[3];
	SRenderTriangle*				psTriangle;
	SRenderCorner*					psCorner;
	size							uiNumFaces;
	size							uiNumPositions;
	size							i;
	int32							j;
	int32							iFace;
	int32							iPosition;
	int32							iUV;
	int32							iSlot;
	uint32							uiGroup;
	bool							bHasUVs;

	ReturnOnFalse(mcReader.BeginMesh((size)iMesh));

	afPositions.Init();
	afPositions.Resize(mcReader.NumPositions() * 3);
	ReturnOnFalse(mcReader.ReadPositions(afPositions.GetData()));
	ReturnOnFalse(PosePositions(&afPositions, paiConnections, paiSlots));
	uiNumPositions = afPositions.NumElements() / 3;

	uiNumFaces = mcReader.NumPolygons();
	aiSizes.Init();
	aiCorners.Init();
	aiCornerUVs.Init();
	afUVs.Init();
	aiMaterialSlots.Init();
	aiPolygonSlots.Init();
	auiGroups.Init();
	aiSizes.Resize(uiNumFaces);
	aiCorners.Resize(mcReader.NumCorners());
	ReturnOnFalse(mcReader.ReadPolygonSizes(aiSizes.GetData()));
	ReturnOnFalse(mcReader.ReadCornerPositions(aiCorners.GetData()));
	for (i = 0; i < uiNumFaces; i++)
	{
		if (aiSizes.GetValue(i) != 3)
		{
			printf("Only triangles are drawn here.\n");
			return false;
		}
	}

	bHasUVs = mcReader.NumUVLayers() > 0;
	if (bHasUVs)
	{
		ReturnOnFalse(mcReader.BeginUVLayer(0));
		afUVs.Resize(mcReader.NumUVs() * 2);
		aiCornerUVs.Resize(mcReader.NumCornerUVs());
		ReturnOnFalse(mcReader.ReadUVs(afUVs.GetData()));
		ReturnOnFalse(mcReader.ReadCornerUVs(aiCornerUVs.GetData()));
		ReturnOnFalse(mcReader.End());
	}
	if (mcReader.NumPolygonSlots() > 0)
	{
		aiMaterialSlots.Resize(mcReader.NumMaterialSlots());
		aiPolygonSlots.Resize(mcReader.NumPolygonSlots());
		ReturnOnFalse(mcReader.ReadMaterialSlots(aiMaterialSlots.GetData()));
		ReturnOnFalse(mcReader.ReadPolygonSlots(aiPolygonSlots.GetData()));
	}
	if (mcReader.NumSmoothGroups() > 0)
	{
		auiGroups.Resize(mcReader.NumSmoothGroups());
		ReturnOnFalse(mcReader.ReadSmoothGroups(auiGroups.GetData()));
	}
	ReturnOnFalse(mcReader.End());

	//Which faces touch each position, as lists through aiFaceNexts, one entry for each corner.
	aiFaceHeads.Init();
	aiFaceNexts.Init();
	asFaceNormals.Init();
	aiFaceHeads.Resize(uiNumPositions);
	aiFaceNexts.Resize(uiNumFaces * 3);
	for (i = 0; i < uiNumPositions; i++)
	{
		aiFaceHeads.SetValue(i, -1);
	}
	for (i = 0; i < uiNumFaces; i++)
	{
		for (j = 0; j < 3; j++)
		{
			iPosition = aiCorners.GetValue(i * 3 + (size)j);
			psCornerPosition[j] = (SFloat32Vec3*)(afPositions.GetData() + (size)iPosition * 3);
			aiFaceNexts.SetValue(i * 3 + (size)j, aiFaceHeads.GetValue((size)iPosition));
			aiFaceHeads.SetValue((size)iPosition, (int32)(i * 3 + (size)j));
		}
		sA.Init(psCornerPosition[1]->x - psCornerPosition[0]->x, psCornerPosition[1]->y - psCornerPosition[0]->y, psCornerPosition[1]->z - psCornerPosition[0]->z);
		sB.Init(psCornerPosition[2]->x - psCornerPosition[0]->x, psCornerPosition[2]->y - psCornerPosition[0]->y, psCornerPosition[2]->z - psCornerPosition[0]->z);
		Float32Vec3Cross(asFaceNormals.Add(), &sA, &sB);
	}

	for (i = 0; i < uiNumFaces; i++)
	{
		psTriangle = masTriangles.Add();
		iSlot = (aiPolygonSlots.NumElements() > 0) ? aiPolygonSlots.GetValue(i) : SCENE_FILE_NONE;
		psTriangle->iMaterial = (iSlot != SCENE_FILE_NONE) ? aiMaterialSlots.GetValue((size)iSlot) : SCENE_FILE_NONE;
		uiGroup = (auiGroups.NumElements() > 0) ? auiGroups.GetValue(i) : 0;
		for (j = 0; j < 3; j++)
		{
			psCorner = &psTriangle->asCorners[j];
			iPosition = aiCorners.GetValue(i * 3 + (size)j);
			memcpy(&psCorner->sPosition, afPositions.GetData() + (size)iPosition * 3, sizeof(float32) * 3);
			Bound(&psCorner->sPosition);

			iUV = bHasUVs ? aiCornerUVs.GetValue(i * 3 + (size)j) : SCENE_FILE_NONE;
			psCorner->u = (iUV != SCENE_FILE_NONE) ? afUVs.GetValue((size)iUV * 2) : 0.0f;
			psCorner->v = (iUV != SCENE_FILE_NONE) ? afUVs.GetValue((size)iUV * 2 + 1) : 0.0f;

			psCorner->sNormal = *asFaceNormals.Get(i);
			if (uiGroup != 0)
			{
				psCorner->sNormal.Init(0.0f, 0.0f, 0.0f);
				for (iFace = aiFaceHeads.GetValue((size)iPosition); iFace != -1; iFace = aiFaceNexts.GetValue((size)iFace))
				{
					if ((auiGroups.GetValue((size)iFace / 3) & uiGroup) != 0)
					{
						psFaceNormal = asFaceNormals.Get((size)iFace / 3);
						psCorner->sNormal.x += psFaceNormal->x;
						psCorner->sNormal.y += psFaceNormal->y;
						psCorner->sNormal.z += psFaceNormal->z;
					}
				}
			}
			RenderNormalise(&psCorner->sNormal);
		}
	}

	asFaceNormals.Kill();
	aiFaceNexts.Kill();
	aiFaceHeads.Kill();
	auiGroups.Kill();
	aiPolygonSlots.Kill();
	aiMaterialSlots.Kill();
	afUVs.Kill();
	aiCornerUVs.Kill();
	aiCorners.Kill();
	aiSizes.Kill();
	afPositions.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Through a camera in the file, or turning around the middle of what's
// there.  A view has x right, y up and z towards the eye.
//
//////////////////////////////////////////////////////////////////////////
bool CRenderScene::GetView(SRenderOptions* psOptions, SRenderView* psView)
{
	SFloat4x4	sWorld;
	SFloat4x4	sYaw;
	SFloat4x4	sPitch;
	SFloat4x4	sTurn;
	SFloat4x4	sMove;
	float32		fRadius;
	float32		fX;
	float32		fY;
	float32		fZ;
	float32		fFieldOfView;
	float32		fNear;
	float32		fFar;
	float		fDeterminant;
	int32		iConnection;

	fX = msMax.x - msMin.x;
	fY = msMax.y - msMin.y;
	fZ = msMax.z - msMin.z;
	fRadius = 0.5f * sqrtf(fX * fX + fY * fY + fZ * fZ);
	if (fRadius <= 0.0f)
	{
		fRadius = 1.0f;
	}

	if (psOptions->iCamera != SCENE_FILE_NONE)
	{
		ReturnOnFalse(mcReader.BeginCamera((size)psOptions->iCamera));
		ReturnOnFalse(mcReader.ReadLens(&fFieldOfView, &fNear, &fFar));
		iConnection = mcReader.GetAttached();
		ReturnOnFalse(mcReader.End());
		if (iConnection == SCENE_FILE_NONE)
		{
			printf("That camera isn't attached to anything.\n");
			return false;
		}
		GetWorld(&sWorld, iConnection);
		ReturnOnFalse(Float4x4Inverse(&psView->sToView, &fDeterminant, &sWorld));
		SetLookThrough(psOptions, psView, fRadius, fFieldOfView);
		return true;
	}

	if (psOptions->bEye)
	{
		psView->sToView.Identity();
		psView->sToView.pos.x = -psOptions->afEye[0];
		psView->sToView.pos.y = -psOptions->afEye[1];
		psView->sToView.pos.z = -psOptions->afEye[2];
		SetLookThrough(psOptions, psView, fRadius, RENDER_EYE_FIELD_OF_VIEW);
		return true;
	}

	sMove.Identity();
	sMove.pos.x = -0.5f * (msMin.x + msMax.x);
	sMove.pos.y = -0.5f * (msMin.y + msMax.y);
	sMove.pos.z = -0.5f * (msMin.z + msMax.z);
	Float4x4RotationY(&sYaw, psOptions->fYaw * RENDER_DEGREES);
	Float4x4RotationX(&sPitch, psOptions->fPitch * RENDER_DEGREES);
	Float4x4Multiply(&sTurn, &sYaw, &sPitch);
	Float4x4Multiply(&psView->sToView, &sMove, &sTurn);
	psView->fScale = (float32)RASTER_MESH_HALF_EXTENT / fRadius;
	psView->iEyeDistance = RASTER_MESH_HALF_EXTENT * RENDER_ORBIT_EYE;
	psView->iZOffset = 0;
	psView->iFocalLength = psOptions->iSize * (RENDER_ORBIT_EYE - 1) / 2;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// RasterLib's eye is on its z axis, looking along -z, so everything moves
// that far.  fFieldOfView is horizontal.
//
//////////////////////////////////////////////////////////////////////////
void CRenderScene::SetLookThrough(SRenderOptions* psOptions, SRenderView* psView, float32 fRadius, float32 fFieldOfView)
{
	psView->fScale = (float32)RENDER_CAMERA_UNITS / (4.0f * fRadius);
	psView->iEyeDistance = RENDER_CAMERA_UNITS;
	psView->iZOffset = RENDER_CAMERA_UNITS;
	psView->iFocalLength = (int32)(0.5f * (float32)psOptions->iSize / tanf(0.5f * fFieldOfView));
}


//////////////////////////////////////////////////////////////////////////
//
// 0xAARRGGBB.
//
//////////////////////////////////////////////////////////////////////////
uint32 CRenderScene::GetColour(int32 iMaterial, float32 u, float32 v)
{
	SRenderMaterial*	psMaterial;
	uint32				uiTexture;
	uint32				uiAlpha;

	if (iMaterial == SCENE_FILE_NONE)
	{
		return RENDER_NO_MATERIAL;
	}

	psMaterial = masMaterials.Get((size)iMaterial);
	uiAlpha = (uint32)(psMaterial->afDiffuse[3] * 255.0f);
	if (psMaterial->iTexture == SCENE_FILE_NONE)
	{
		return (uiAlpha << 24) | ((uint32)(psMaterial->afDiffuse[0] * 255.0f) << 16) | ((uint32)(psMaterial->afDiffuse[1] * 255.0f) << 8) | (uint32)(psMaterial->afDiffuse[2] * 255.0f);
	}

	uiTexture = macImages.Get((size)psMaterial->iTexture)->Sample(u, v);
	uiAlpha = (uiAlpha * (uiTexture >> 24)) / 255;
	return (uiAlpha << 24) | (uiTexture & 0xFFFFFF);
}


//////////////////////////////////////////////////////////////////////////
//
// How many times to cut a triangle's sides, so that its pieces show the
// texture beneath them.
//
//////////////////////////////////////////////////////////////////////////
int32 CRenderScene::GetCuts(SRenderTriangle* psTriangle)
{
	CRenderImage*	pcImage;
	float32			fLongest;
	float32			fU;
	float32			fV;
	float32			fLength;
	int32			iTexture;
	int32			iCuts;
	int32			j;

	if (psTriangle->iMaterial == SCENE_FILE_NONE)
	{
		return 1;
	}
	iTexture = masMaterials.Get((size)psTriangle->iMaterial)->iTexture;
	if (iTexture == SCENE_FILE_NONE)
	{
		return 1;
	}

	pcImage = macImages.Get((size)iTexture);
	fLongest = 0.0f;
	for (j = 0; j < 3; j++)
	{
		fU = (psTriangle->asCorners[(j + 1) % 3].u - psTriangle->asCorners[j].u) * (float32)pcImage->miWidth;
		fV = (psTriangle->asCorners[(j + 1) % 3].v - psTriangle->asCorners[j].v) * (float32)pcImage->miHeight;
		fLength = sqrtf(fU * fU + fV * fV);
		fLongest = (fLength > fLongest) ? fLength : fLongest;
	}
	iCuts = (int32)(fLongest / RENDER_TEXELS_PER_CUT) + 1;
	return (iCuts > RENDER_MAX_CUTS) ? RENDER_MAX_CUTS : iCuts;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void RenderMix(SRenderCorner* psDest, SRenderTriangle* psTriangle, float32 fA, float32 fB, float32 fC)
{
	SRenderCorner*	psCorners;

	psCorners = psTriangle->asCorners;
	psDest->sPosition.Init(psCorners[0].sPosition.x * fA + psCorners[1].sPosition.x * fB + psCorners[2].sPosition.x * fC,
						   psCorners[0].sPosition.y * fA + psCorners[1].sPosition.y * fB + psCorners[2].sPosition.y * fC,
						   psCorners[0].sPosition.z * fA + psCorners[1].sPosition.z * fB + psCorners[2].sPosition.z * fC);
	psDest->sNormal.Init(psCorners[0].sNormal.x * fA + psCorners[1].sNormal.x * fB + psCorners[2].sNormal.x * fC,
						 psCorners[0].sNormal.y * fA + psCorners[1].sNormal.y * fB + psCorners[2].sNormal.y * fC,
						 psCorners[0].sNormal.z * fA + psCorners[1].sNormal.z * fB + psCorners[2].sNormal.z * fC);
	psDest->u = psCorners[0].u * fA + psCorners[1].u * fB + psCorners[2].u * fC;
	psDest->v = psCorners[0].v * fA + psCorners[1].v * fB + psCorners[2].v * fC;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int32 RenderAddVertex(CRasterMesh* pcMesh, SRenderCorner* psCorner, SRenderView* psView)
{
	SRasterVertex*	psVertex;
	SFloat32Vec3	sPosition;
	SFloat32Vec3	sNormal;
	int32			iVertex;

	Float32Vec3TransformCoord(&sPosition, &psCorner->sPosition, &psView->sToView);
	Float32Vec3TransformNormal(&sNormal, &psCorner->sNormal, &psView->sToView);
	RenderNormalise(&sNormal);

	iVertex = (int32)pcMesh->masVertices.NumElements();
	psVertex = pcMesh->masVertices.Add();
	psVertex->sPosition.Init((int32)(sPosition.x * psView->fScale), (int32)(sPosition.y * psView->fScale), (int32)(sPosition.z * psView->fScale) + psView->iZOffset);
	psVertex->sNormal.Init(SRasterUnit::FromFloat(sNormal.x), SRasterUnit::FromFloat(sNormal.y), SRasterUnit::FromFloat(sNormal.z));
	return iVertex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void RenderAddPiece(CRasterMesh* pcMesh, int32 iA, int32 iB, int32 iC, uint32 uiColour)
{
	SRasterTriangle*	psTriangle;

	psTriangle = ((uiColour >> 24) == 0xFF) ? pcMesh->masOpaque.Add() : pcMesh->masTransparent.Add();
	psTriangle->aiVertex[0] = iA;
	psTriangle->aiVertex[1] = iB;
	psTriangle->aiVertex[2] = iC;
	psTriangle->uiColour = uiColour;
}


//////////////////////////////////////////////////////////////////////////
//
// A triangle cut into iCuts * iCuts pieces, each a colour of its own.
//
//////////////////////////////////////////////////////////////////////////
void CRenderScene::AddBuiltTriangle(SRenderTriangle* psTriangle, SRenderView* psView, CRasterMesh* pcMesh)
{
	SRenderCorner	sCorner;
	CArrayRenderInts	aiVertices;
	float32			fCuts;
	float32			fThird;
	int32			iCuts;
	int32			iRow;
	int32			iColumn;
	int32			iRowStart;
	int32			iNextStart;
	int32			iB;
	int32			iC;

	iCuts = GetCuts(psTriangle);
	fCuts = (float32)iCuts;
	fThird = 1.0f / 3.0f;

	aiVertices.Init();
	for (iRow = 0; iRow <= iCuts; iRow++)
	{
		for (iColumn = 0; iColumn <= iCuts - iRow; iColumn++)
		{
			RenderMix(&sCorner, psTriangle, (float32)(iCuts - iRow - iColumn) / fCuts, (float32)iColumn / fCuts, (float32)iRow / fCuts);
			aiVertices.Add(RenderAddVertex(pcMesh, &sCorner, psView));
		}
	}

	iRowStart = 0;
	for (iRow = 0; iRow < iCuts; iRow++)
	{
		iNextStart = iRowStart + iCuts - iRow + 1;
		for (iColumn = 0; iColumn < iCuts - iRow; iColumn++)
		{
			RenderMix(&sCorner, psTriangle, ((float32)(iCuts - iRow - iColumn) - 2.0f * fThird) / fCuts, ((float32)iColumn + fThird) / fCuts, ((float32)iRow + fThird) / fCuts);
			RenderAddPiece(pcMesh, aiVertices.GetValue((size)(iRowStart + iColumn)), aiVertices.GetValue((size)(iRowStart + iColumn + 1)), aiVertices.GetValue((size)(iNextStart + iColumn)), GetColour(psTriangle->iMaterial, sCorner.u, sCorner.v));
			if (iColumn < iCuts - iRow - 1)
			{
				iB = iRowStart + iColumn + 1;
				iC = iNextStart + iColumn;
				RenderMix(&sCorner, psTriangle, ((float32)(iCuts - iRow - iColumn) - 1.0f - fThird) / fCuts, ((float32)iColumn + 2.0f * fThird) / fCuts, ((float32)iRow + 2.0f * fThird) / fCuts);
				RenderAddPiece(pcMesh, aiVertices.GetValue((size)iB), aiVertices.GetValue((size)(iC + 1)), aiVertices.GetValue((size)iC), GetColour(psTriangle->iMaterial, sCorner.u, sCorner.v));
			}
		}
		iRowStart = iNextStart;
	}
	aiVertices.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each particle is a square facing the eye.
//
//////////////////////////////////////////////////////////////////////////
bool CRenderScene::AddParticles(SRenderView* psView, CRasterMesh* pcMesh, float32 fScale)
{
	CArrayRenderFloats	afPositions;
	CArrayRenderFloats	afColours;
	CArrayRenderFloats	afRadii;
	CArrayRenderFloats	afIntensities;
	SRasterVertex*		psVertex;
	SFloat32Vec3		sPosition;
	SFloat32Vec3		sView;
	float32*			pfColour;
	float32				fOffset;
	float32				fRadius;
	float32				fIntensity;
	size				uiNum;
	size				uiSet;
	size				i;
	int32				j;
	int32				iFirst;
	int32				iRed;
	int32				iGreen;
	int32				iBlue;
	uint32				uiColour;

	for (uiSet = 0; uiSet < mcReader.NumParticleSets(); uiSet++)
	{
		ReturnOnFalse(mcReader.BeginParticles(uiSet));
		uiNum = mcReader.NumParticles();
		afPositions.Init();
		afColours.Init();
		afRadii.Init();
		afIntensities.Init();
		afPositions.Resize(uiNum * 3);
		afColours.Resize(uiNum * 3);
		afRadii.Resize(uiNum);
		afIntensities.Resize(uiNum);
		ReturnOnFalse(mcReader.ReadParticlePositions(afPositions.GetData()));
		ReturnOnFalse(mcReader.ReadParticleColours(afColours.GetData()));
		ReturnOnFalse(mcReader.ReadParticleRadii(afRadii.GetData()));
		ReturnOnFalse(mcReader.ReadParticleIntensities(afIntensities.GetData()));
		ReturnOnFalse(mcReader.End());

		fOffset = (fScale != 1.0f) ? 0.5f : 0.0f;
		for (i = 0; i < uiNum; i++)
		{
			sPosition.Init((afPositions.GetValue(i * 3) - fOffset) * fScale, (afPositions.GetValue(i * 3 + 1) - fOffset) * fScale, (afPositions.GetValue(i * 3 + 2) - fOffset) * fScale);
			Float32Vec3TransformCoord(&sView, &sPosition, &psView->sToView);
			fRadius = afRadii.GetValue(i) * fScale;
			fIntensity = afIntensities.GetValue(i);
			pfColour = afColours.GetData() + i * 3;
			iRed = (int32)(pfColour[0] * fIntensity * 255.0f);
			iGreen = (int32)(pfColour[1] * fIntensity * 255.0f);
			iBlue = (int32)(pfColour[2] * fIntensity * 255.0f);
			uiColour = 0xFF000000 | ((uint32)((iRed > 255) ? 255 : iRed) << 16) | ((uint32)((iGreen > 255) ? 255 : iGreen) << 8) | (uint32)((iBlue > 255) ? 255 : iBlue);

			iFirst = (int32)pcMesh->masVertices.NumElements();
			for (j = 0; j < 4; j++)
			{
				psVertex = pcMesh->masVertices.Add();
				psVertex->sPosition.Init((int32)((sView.x + ((j == 1) || (j == 2) ? fRadius : -fRadius)) * psView->fScale), (int32)((sView.y + ((j >= 2) ? fRadius : -fRadius)) * psView->fScale), (int32)(sView.z * psView->fScale) + psView->iZOffset);
				psVertex->sNormal.Init(SRasterUnit::FromFloat(0.0f), SRasterUnit::FromFloat(0.0f), SRasterUnit::FromFloat(1.0f));
			}
			RenderAddPiece(pcMesh, iFirst, iFirst + 1, iFirst + 2, uiColour);
			RenderAddPiece(pcMesh, iFirst, iFirst + 2, iFirst + 3, uiColour);
		}

		afIntensities.Kill();
		afRadii.Kill();
		afColours.Kill();
		afPositions.Kill();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRenderScene::Build(SRenderOptions* psOptions, SRenderView* psView, CRasterMesh* pcMesh)
{
	size	i;

	for (i = 0; i < masTriangles.NumElements(); i++)
	{
		AddBuiltTriangle(masTriangles.Get(i), psView, pcMesh);
	}
	AddParticles(psView, pcMesh, psOptions->fParticleScale);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool RenderScene(SRenderOptions* psOptions)
{
	CRenderScene	cScene;
	SRenderView		sView;
	CRasterMesh		cMesh;
	CRasterCamera	cCamera;
	CRasteriser		cRasteriser;
	CRasterTarget	cTarget;
	bool			bResult;

	bResult = cScene.Init(psOptions->szScene);
	bResult = bResult && cScene.Load(psOptions);
	bResult = bResult && cScene.GetView(psOptions, &sView);
	if (bResult)
	{
		cMesh.Init();
		cScene.Build(psOptions, &sView, &cMesh);
		printf("%d triangles to draw, %d pieces.\n", (int)cScene.masTriangles.NumElements(), (int)cMesh.NumTriangles());

		Ptr<CImage> pImage = OMalloc<CImage>(psOptions->iSize, psOptions->iSize, PT_uint8, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, IMAGE_IGNORED, CHANNEL_STOP);
		bResult = cTarget.Init((uint32*)pImage->GetData(), psOptions->iSize, psOptions->iSize, psOptions->iSize);
		if (bResult)
		{
			cTarget.Clear(RENDER_BACKGROUND);
			cCamera.Init();
			cCamera.SetPerspective(sView.iEyeDistance, sView.iFocalLength);
			cRasteriser.Init();
			cRasteriser.SetCullBackFaces(true);
			cRasteriser.Draw(&cMesh, &cCamera, &cTarget);
			cRasteriser.Kill();
			cTarget.Kill();
			bResult = WriteImage(pImage, (char*)psOptions->szOutput, IT_PNG);
		}
		cMesh.Kill();
	}
	cScene.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int __cdecl main(int argc, char* argv[])
{
	SRenderOptions	sOptions;
	int				i;
	bool			bResult;

	if (argc < 3)
	{
		printf("RenderSceneFile <scene .DRG> <out .png> [--key <n>] [--yaw <degrees>] [--pitch <degrees>] [--camera <n>] [--eye <x,y,z>] [--instance <n>] [--size <pixels>] [--particle-scale <n>]\n");
		return 1;
	}

	sOptions.szScene = argv[1];
	sOptions.szOutput = argv[2];
	sOptions.iKey = SCENE_FILE_NONE;
	sOptions.iCamera = SCENE_FILE_NONE;
	sOptions.iInstance = SCENE_FILE_NONE;
	sOptions.iSize = RENDER_DEFAULT_SIZE;
	sOptions.fYaw = 0.0f;
	sOptions.fPitch = 0.0f;
	sOptions.fParticleScale = 1.0f;
	sOptions.bEye = false;
	for (i = 3; i + 1 < argc; i += 2)
	{
		if (strcmp(argv[i], "--key") == 0)
		{
			sOptions.iKey = atoi(argv[i + 1]);
		}
		else if (strcmp(argv[i], "--yaw") == 0)
		{
			sOptions.fYaw = (float32)atof(argv[i + 1]);
		}
		else if (strcmp(argv[i], "--pitch") == 0)
		{
			sOptions.fPitch = (float32)atof(argv[i + 1]);
		}
		else if (strcmp(argv[i], "--camera") == 0)
		{
			sOptions.iCamera = atoi(argv[i + 1]);
		}
		else if (strcmp(argv[i], "--instance") == 0)
		{
			sOptions.iInstance = atoi(argv[i + 1]);
		}
		else if (strcmp(argv[i], "--eye") == 0)
		{
			sOptions.bEye = sscanf(argv[i + 1], "%f,%f,%f", &sOptions.afEye[0], &sOptions.afEye[1], &sOptions.afEye[2]) == 3;
		}
		else if (strcmp(argv[i], "--particle-scale") == 0)
		{
			sOptions.fParticleScale = (float32)atof(argv[i + 1]);
		}
		else if (strcmp(argv[i], "--size") == 0)
		{
			sOptions.iSize = atoi(argv[i + 1]);
		}
		else
		{
			printf("Don't know [%s].\n", argv[i]);
			return 1;
		}
	}

	gcLogger.Init();
	TypesInit();
	TypeConverterInit();
	MemoryInit();
	DataIOInit();
	ObjectsInit();

	bResult = RenderScene(&sOptions);
	if (!bResult)
	{
		printf("Could not render [%s].\n", sOptions.szScene);
	}

	ObjectsKill(false);
	DataIOKill();
	MemoryKill();
	TypeConverterKill();
	TypesKill();
	LeakDetection();
	gcLogger.Kill();
	return bResult ? 0 : 1;
}


/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SceneFileLib

Codaphela SceneFileLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SceneFileLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SceneFileLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __SCENE_FILE_WRITER_H__
#define __SCENE_FILE_WRITER_H__
#include "BaseLib/ChunkFile.h"
#include "BaseLib/Chars.h"
#include "SceneFileGeneral.h"


enum ESceneFileBlock
{
	SFB_None,
	SFB_Mesh,
	SFB_Image,
	SFB_Material,
	SFB_Connection,
	SFB_Light,
	SFB_Camera,
	SFB_Animation,
	SFB_Instance,
	SFB_Particles,
};


//What has been written of the mesh being written, to check at EndMesh that it agrees with itself.
struct SSceneFileMeshCounts
{
	size	uiPositions;
	size	uiNormals;
	size	uiColours;
	size	uiPolygons;
	size	uiCorners;
	size	uiCornerNormals;
	size	uiCornerColours;
	size	uiMaterialSlots;
	size	uiPolygonSlots;
	size	uiSmoothGroups;
	size	uiSkinCounts;
	size	uiInverseMatrices;
	int32	iMaxPosition;
	int32	iMaxNormal;
	int32	iMaxColour;
	int32	iMaxSlot;
	int32	iMaxSkinMatrix;
};


//Writes a scene file, with nothing but BaseLib.  What's written isn't kept: it's only counted, to check that what it refers to
//is there.  Things in a file refer to each other by their index among those of their kind, in the order they were begun.
//
//Everything in a Begin and End goes in any order, once, and may be left out.  If anything fails or is wrong, everything after
//it fails, and Close fails and leaves no file, so an exporter that only looks at Close's result still knows.
class CSceneFileWriter
{
protected:
	CChunkFile				mcFile;
	CChars					mszFilename;
	bool					mbOpen;
	bool					mbFailed;
	ESceneFileBlock			meBlock;
	SSceneFileMeshCounts	msMesh;
	size					muiNumMeshes;
	size					muiNumImages;
	size					muiNumMaterials;
	size					muiNumConnections;
	size					muiNumInstanceConnections;
	int32					miMaxMesh;
	int32					miMaxImage;
	int32					miMaxMaterial;
	int32					miMaxConnection;

public:
	void	Init(const char* szFilename);
	void	Kill(void);

	bool	Open(void);
	bool	Close(void);

	bool	BeginMesh(const char* szName);
	bool	WritePositions(const float32* pafXYZ, size uiNumPositions);
	bool	WriteNormals(const float32* pafXYZ, size uiNumNormals);
	bool	WriteColours(const uint32* pauiColours, size uiNumColours);
	bool	WritePolygons(const int32* paiSizes, size uiNumPolygons, const int32* paiCornerPositions, size uiNumCorners);
	bool	WriteCornerNormals(const int32* paiNormals, size uiNumCorners);
	bool	WriteCornerColours(const int32* paiColours, size uiNumCorners);
	bool	WriteUVLayer(const float32* pafUVs, size uiNumUVs, const int32* paiCornerUVs, size uiNumCorners);
	bool	WriteMaterialSlots(const int32* paiMaterials, size uiNumSlots);
	bool	WritePolygonSlots(const int32* paiSlots, size uiNumPolygons);
	bool	WriteSmoothGroups(const uint32* pauiGroups, size uiNumPolygons);
	bool	WriteSkin(const int32* paiCounts, size uiNumPositions, const int32* paiMatrices, const float32* pafWeights, size uiNumWeights);
	bool	WriteInverseMatrices(const float32* paf4x4s, size uiNumMatrices);
	bool	EndMesh(void);

	bool	BeginImage(const char* szName);
	bool	WritePixels(int32 iWidth, int32 iHeight, const char* szChannels, const uint8* puiPixels);
	bool	EndImage(void);

	bool	BeginMaterial(const char* szName);
	bool	WriteMaterialColour(ESceneFileMaterialColour eColour, const float32* pafRGBA);
	bool	WriteShininess(float32 fPower, float32 fBase);
	bool	WriteMaterialTexture(ESceneFileTexture eTexture, int32 iImage);
	bool	EndMaterial(void);

	bool	BeginConnection(const char* szName);
	bool	WriteParent(int32 iConnection);
	bool	WriteWorldMatrix(const float32* paf4x4);
	bool	WriteLocalMatrix(const float32* paf4x4);
	bool	WriteParity(bool bParity);
	bool	EndConnection(void);

	bool	BeginLight(const char* szName);
	bool	WriteLightType(ESceneFileLightType eType);
	bool	WriteLightColour(const float32* pafRGBA);
	bool	WriteLightValues(float32 fDecayStart, float32 fRange, float32 fIntensity);
	bool	WriteLightDecay(ESceneFileLightDecay eDecay);
	bool	WriteShadows(bool bShadows);
	bool	EndLight(void);

	bool	BeginCamera(const char* szName);
	bool	WriteCameraType(ESceneFileCameraType eType);
	bool	WriteLens(float32 fFieldOfView, float32 fNear, float32 fFar);
	bool	EndCamera(void);

	bool	WriteAttached(int32 iConnection);  //For a light or a camera.

	bool	BeginAnimation(const char* szName);
	bool	WriteTrack(int32 iConnection, const float32* pafTimes, const float32* paf4x4s, size uiNumKeys);  //Times must go up.
	bool	EndAnimation(void);

	bool	BeginInstance(const char* szName);
	bool	WriteConnections(const int32* paiConnections, size uiNumConnections);  //Before the meshes, whose slots index them.
	bool	WriteInstanceMesh(int32 iMesh, const int32* paiConnectionSlots, size uiNumSlots, const int32* paiMaterials, size uiNumMaterials);  //paiMaterials can be NULL.
	bool	EndInstance(void);

	bool	WriteParticles(const char* szName, const float32* pafXYZ, const float32* pafRGB, const float32* pafRadii, const float32* pafIntensities, size uiNumParticles);  //All but pafXYZ can be NULL.

protected:
	bool	Fail(const char* szMessage);
	bool	InBlock(ESceneFileBlock eBlock);
	bool	Begin(ESceneFileBlock eBlock, const char* szChunk, const char* szName);
	bool	End(ESceneFileBlock eBlock, const char* szChunk);
	bool	WriteChunk(const char* szChunk, const void* pvData, size uiElementSize, size uiNumElements);
	bool	WriteInt32Chunk(const char* szChunk, int32 iValue);
	bool	CheckIndices(const int32* paiIndices, size uiNumIndices, int32* piMax);
	bool	CheckRequiredIndices(const int32* paiIndices, size uiNumIndices, int32* piMax);
	bool	CheckMesh(void);
};


#endif // __SCENE_FILE_WRITER_H__


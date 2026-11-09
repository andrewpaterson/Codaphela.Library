/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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

** ------------------------------------------------------------------------ **/
#ifndef __SCENE_FILE_READER_H__
#define __SCENE_FILE_READER_H__
#include "BaseLib/ChunkFile.h"
#include "BaseLib/Chars.h"
#include "SceneFileGeneral.h"


//Reads a scene file, with nothing but BaseLib.  It keeps nothing: the caller asks how many of something there are, makes room,
//and has them read into it.  Anything left out of a file is simply none: Num gives 0, a Read of a value it hasn't got returns
//false, and an int32 value that's an index gives SCENE_FILE_NONE.
//
//Begin goes into the thing at that index among those of its kind.  End leaves it.  Only what's in the thing you're in can be
//read, and the Reads that belong to a mesh's UV layer, an animation's track or an instance's mesh need a Begin for those too.
//
//If a chunk's size doesn't fit what it should hold the file is damaged: the Num gives 0, the Read fails, and Failed is true.
class CSceneFileReader
{
protected:
	CChunkFile	mcFile;
	CChars		mszFilename;
	bool		mbOpen;
	bool		mbFailed;

public:
	void	Init(const char* szFilename);
	void	Kill(void);

	bool	Open(void);
	bool	Close(void);
	bool	Failed(void);

	bool	End(void);
	bool	ReadName(CChars* pszName);

	size	NumMeshes(void);
	bool	BeginMesh(size uiMesh);
	size	NumPositions(void);
	bool	ReadPositions(float32* pafXYZ);
	size	NumNormals(void);
	bool	ReadNormals(float32* pafXYZ);
	size	NumColours(void);
	bool	ReadColours(uint32* pauiColours);
	size	NumPolygons(void);
	bool	ReadPolygonSizes(int32* paiSizes);
	size	NumCorners(void);
	bool	ReadCornerPositions(int32* paiPositions);
	size	NumCornerNormals(void);
	bool	ReadCornerNormals(int32* paiNormals);
	size	NumCornerColours(void);
	bool	ReadCornerColours(int32* paiColours);
	size	NumMaterialSlots(void);
	bool	ReadMaterialSlots(int32* paiMaterials);
	size	NumPolygonSlots(void);
	bool	ReadPolygonSlots(int32* paiSlots);
	size	NumSmoothGroups(void);
	bool	ReadSmoothGroups(uint32* pauiGroups);
	size	NumSkinCounts(void);
	bool	ReadSkinCounts(int32* paiCounts);
	size	NumSkinWeights(void);
	bool	ReadSkinMatrices(int32* paiMatrices);
	bool	ReadSkinWeights(float32* pafWeights);
	size	NumInverseMatrices(void);
	bool	ReadInverseMatrices(float32* paf4x4s);

	size	NumUVLayers(void);
	bool	BeginUVLayer(size uiLayer);
	size	NumUVs(void);
	bool	ReadUVs(float32* pafUVs);
	size	NumCornerUVs(void);
	bool	ReadCornerUVs(int32* paiUVs);

	size	NumImages(void);
	bool	BeginImage(size uiImage);
	bool	ReadImageSize(int32* piWidth, int32* piHeight);
	bool	ReadChannels(CChars* pszChannels);
	size	NumPixelBytes(void);
	bool	ReadPixels(uint8* puiPixels);

	size	NumMaterials(void);
	bool	BeginMaterial(size uiMaterial);
	bool	ReadMaterialColour(ESceneFileMaterialColour eColour, float32* pafRGBA);
	bool	ReadShininess(float32* pfPower, float32* pfBase);
	int32	GetMaterialTexture(ESceneFileTexture eTexture);

	size	NumConnections(void);
	bool	BeginConnection(size uiConnection);
	int32	GetParent(void);
	bool	ReadWorldMatrix(float32* paf4x4);
	bool	ReadLocalMatrix(float32* paf4x4);
	bool	GetParity(void);

	size	NumLights(void);
	bool	BeginLight(size uiLight);
	ESceneFileLightType		GetLightType(void);
	bool	ReadLightColour(float32* pafRGBA);
	bool	ReadLightValues(float32* pfDecayStart, float32* pfRange, float32* pfIntensity);
	ESceneFileLightDecay	GetLightDecay(void);
	bool	GetShadows(void);

	size	NumCameras(void);
	bool	BeginCamera(size uiCamera);
	ESceneFileCameraType	GetCameraType(void);
	bool	ReadLens(float32* pfFieldOfView, float32* pfNear, float32* pfFar);

	int32	GetAttached(void);  //For a light, a camera or a track.

	size	NumAnimations(void);
	bool	BeginAnimation(size uiAnimation);
	size	NumTracks(void);
	bool	BeginTrack(size uiTrack);
	size	NumKeys(void);
	bool	ReadKeyTimes(float32* pafTimes);
	bool	ReadKeyMatrices(float32* paf4x4s);

	size	NumInstances(void);
	bool	BeginInstance(size uiInstance);
	size	NumInstanceConnections(void);
	bool	ReadInstanceConnections(int32* paiConnections);
	size	NumInstanceMeshes(void);
	bool	BeginInstanceMesh(size uiMesh);
	int32	GetInstanceMesh(void);
	size	NumConnectionSlots(void);
	bool	ReadConnectionSlots(int32* paiSlots);
	size	NumInstanceMaterials(void);
	bool	ReadInstanceMaterials(int32* paiMaterials);

	size	NumParticleSets(void);
	bool	BeginParticles(size uiSet);
	size	NumParticles(void);
	bool	ReadParticlePositions(float32* pafXYZ);
	bool	ReadParticleColours(float32* pafRGB);
	bool	ReadParticleRadii(float32* pafRadii);
	bool	ReadParticleIntensities(float32* pafIntensities);

protected:
	bool	Fail(const char* szMessage);
	size	NumChunks(const char* szChunk);
	bool	Begin(const char* szChunk, size uiIndex);
	size	NumElements(const char* szChunk, size uiElementSize);
	bool	ReadChunk(const char* szChunk, void* pvData, size uiElementSize);
	bool	ReadFixed(const char* szChunk, void* pvData, size uiSize);
	int32	GetInt32(const char* szChunk, int32 iDefault);
	bool	ReadString(const char* szChunk, CChars* psz);
};


#endif // __SCENE_FILE_READER_H__


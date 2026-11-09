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
#include "BaseLib/DiskFile.h"
#include "BaseLib/Logger.h"
#include "SceneFileReader.h"


#define SCENE_FILE_MATRIX_SIZE	(sizeof(float32) * SCENE_FILE_FLOATS_IN_MATRIX)


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneFileReader::Init(const char* szFilename)
{
	mszFilename.Init(szFilename);
	mbOpen = false;
	mbFailed = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneFileReader::Kill(void)
{
	if (mbOpen)
	{
		Close();
	}
	mszFilename.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A file from a later version of the format is refused, as only then does
// the version change: what it holds can't be read as this version would.
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileReader::Open(void)
{
	int32	iVersion;

	mcFile.Init(DiskFile(mszFilename.Text()));
	if (!mcFile.ReadOpen())
	{
		mcFile.ReadClose();
		mcFile.Kill();
		return Fail("Could not read it as a chunk file");
	}
	mbOpen = true;

	if (mcFile.GetUserID() != SCENE_FILE_ID)
	{
		Close();
		return Fail("It's a chunk file but not a scene file");
	}

	iVersion = GetInt32(SCENE_CHUNK_VERSION, 0);
	if ((iVersion < 1) || (iVersion > SCENE_FILE_VERSION))
	{
		Close();
		return Fail("It's a version of the format this doesn't know");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileReader::Close(void)
{
	bool	bResult;

	if (!mbOpen)
	{
		return Fail("It isn't open");
	}
	bResult = mcFile.ReadClose();
	mcFile.Kill();
	mbOpen = false;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileReader::Failed(void)
{
	return mbFailed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileReader::Fail(const char* szMessage)
{
	mbFailed = true;
	return gcLogger.Error2(__METHOD__, " ", szMessage, " in [", mszFilename.Text(), "].", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CSceneFileReader::NumChunks(const char* szChunk)
{
	lsize	iChunk;
	size	uiCount;

	uiCount = 0;
	iChunk = mcFile.FindFirstChunkWithName(szChunk);
	while (iChunk != CHUNK_FILE_NOT_FOUND)
	{
		uiCount++;
		iChunk = mcFile.FindNextChunkWithName();
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileReader::Begin(const char* szChunk, size uiIndex)
{
	lsize	iChunk;
	size	i;

	iChunk = mcFile.FindFirstChunkWithName(szChunk);
	for (i = 0; (i < uiIndex) && (iChunk != CHUNK_FILE_NOT_FOUND); i++)
	{
		iChunk = mcFile.FindNextChunkWithName();
	}
	if (iChunk == CHUNK_FILE_NOT_FOUND)
	{
		return false;
	}
	return mcFile.ReadChunkBegin(iChunk);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileReader::End(void)
{
	return mcFile.ReadChunkEnd();
}


//////////////////////////////////////////////////////////////////////////
//
// How many values a chunk holds comes from its size.  It's damaged if that
// isn't a whole number of them.
//
//////////////////////////////////////////////////////////////////////////
size CSceneFileReader::NumElements(const char* szChunk, size uiElementSize)
{
	filePos	iSize;

	if (!Begin(szChunk, 0))
	{
		return 0;
	}
	iSize = mcFile.ChunkSize();
	mcFile.ReadChunkEnd();

	if ((iSize % (filePos)uiElementSize) != 0)
	{
		Fail("A chunk isn't a whole number of what it holds");
		return 0;
	}
	return (size)(iSize / (filePos)uiElementSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileReader::ReadChunk(const char* szChunk, void* pvData, size uiElementSize)
{
	filePos	iSize;
	bool	bResult;

	ReturnOnFalse(Begin(szChunk, 0));
	iSize = mcFile.ChunkSize();
	if ((iSize % (filePos)uiElementSize) != 0)
	{
		mcFile.ReadChunkEnd();
		return Fail("A chunk isn't a whole number of what it holds");
	}

	bResult = (iSize == 0) || mcFile.ReadData(pvData, (lsize)iSize);
	mcFile.ReadChunkEnd();
	if (!bResult)
	{
		return Fail("Could not read a chunk");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A chunk that must be exactly uiSize.  False if it's not there.
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileReader::ReadFixed(const char* szChunk, void* pvData, size uiSize)
{
	bool	bResult;

	ReturnOnFalse(Begin(szChunk, 0));
	if (mcFile.ChunkSize() != (filePos)uiSize)
	{
		mcFile.ReadChunkEnd();
		return Fail("A chunk isn't the size of what it holds");
	}

	bResult = mcFile.ReadData(pvData, (lsize)uiSize);
	mcFile.ReadChunkEnd();
	if (!bResult)
	{
		return Fail("Could not read a chunk");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CSceneFileReader::GetInt32(const char* szChunk, int32 iDefault)
{
	int32	iValue;

	if (ReadFixed(szChunk, &iValue, sizeof(int32)))
	{
		return iValue;
	}
	return iDefault;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileReader::ReadString(const char* szChunk, CChars* psz)
{
	size	uiLength;
	char*	szData;
	bool	bResult;

	uiLength = NumElements(szChunk, sizeof(char));
	szData = (char*)malloc(uiLength + 1);
	if (szData == NULL)
	{
		return Fail("Could not allocate a string");
	}

	bResult = (uiLength == 0) || ReadChunk(szChunk, szData, sizeof(char));
	szData[bResult ? uiLength : 0] = '\0';
	psz->Append(szData);
	free(szData);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileReader::ReadName(CChars* pszName)
{
	return ReadString(SCENE_CHUNK_NAME, pszName);
}


size	CSceneFileReader::NumMeshes(void) { return NumChunks(SCENE_CHUNK_MESH); }
bool	CSceneFileReader::BeginMesh(size uiMesh) { return Begin(SCENE_CHUNK_MESH, uiMesh); }
size	CSceneFileReader::NumPositions(void) { return NumElements(SCENE_CHUNK_POSITIONS, sizeof(float32) * 3); }
bool	CSceneFileReader::ReadPositions(float32* pafXYZ) { return ReadChunk(SCENE_CHUNK_POSITIONS, pafXYZ, sizeof(float32) * 3); }
size	CSceneFileReader::NumNormals(void) { return NumElements(SCENE_CHUNK_NORMALS, sizeof(float32) * 3); }
bool	CSceneFileReader::ReadNormals(float32* pafXYZ) { return ReadChunk(SCENE_CHUNK_NORMALS, pafXYZ, sizeof(float32) * 3); }
size	CSceneFileReader::NumColours(void) { return NumElements(SCENE_CHUNK_COLOURS, sizeof(uint32)); }
bool	CSceneFileReader::ReadColours(uint32* pauiColours) { return ReadChunk(SCENE_CHUNK_COLOURS, pauiColours, sizeof(uint32)); }
size	CSceneFileReader::NumPolygons(void) { return NumElements(SCENE_CHUNK_POLYGON_SIZES, sizeof(int32)); }
bool	CSceneFileReader::ReadPolygonSizes(int32* paiSizes) { return ReadChunk(SCENE_CHUNK_POLYGON_SIZES, paiSizes, sizeof(int32)); }
size	CSceneFileReader::NumCorners(void) { return NumElements(SCENE_CHUNK_CORNER_POSITIONS, sizeof(int32)); }
bool	CSceneFileReader::ReadCornerPositions(int32* paiPositions) { return ReadChunk(SCENE_CHUNK_CORNER_POSITIONS, paiPositions, sizeof(int32)); }
size	CSceneFileReader::NumCornerNormals(void) { return NumElements(SCENE_CHUNK_CORNER_NORMALS, sizeof(int32)); }
bool	CSceneFileReader::ReadCornerNormals(int32* paiNormals) { return ReadChunk(SCENE_CHUNK_CORNER_NORMALS, paiNormals, sizeof(int32)); }
size	CSceneFileReader::NumCornerColours(void) { return NumElements(SCENE_CHUNK_CORNER_COLOURS, sizeof(int32)); }
bool	CSceneFileReader::ReadCornerColours(int32* paiColours) { return ReadChunk(SCENE_CHUNK_CORNER_COLOURS, paiColours, sizeof(int32)); }
size	CSceneFileReader::NumMaterialSlots(void) { return NumElements(SCENE_CHUNK_MATERIAL_SLOTS, sizeof(int32)); }
bool	CSceneFileReader::ReadMaterialSlots(int32* paiMaterials) { return ReadChunk(SCENE_CHUNK_MATERIAL_SLOTS, paiMaterials, sizeof(int32)); }
size	CSceneFileReader::NumPolygonSlots(void) { return NumElements(SCENE_CHUNK_POLYGON_SLOTS, sizeof(int32)); }
bool	CSceneFileReader::ReadPolygonSlots(int32* paiSlots) { return ReadChunk(SCENE_CHUNK_POLYGON_SLOTS, paiSlots, sizeof(int32)); }
size	CSceneFileReader::NumSmoothGroups(void) { return NumElements(SCENE_CHUNK_SMOOTH_GROUPS, sizeof(uint32)); }
bool	CSceneFileReader::ReadSmoothGroups(uint32* pauiGroups) { return ReadChunk(SCENE_CHUNK_SMOOTH_GROUPS, pauiGroups, sizeof(uint32)); }
size	CSceneFileReader::NumSkinCounts(void) { return NumElements(SCENE_CHUNK_SKIN_COUNTS, sizeof(int32)); }
bool	CSceneFileReader::ReadSkinCounts(int32* paiCounts) { return ReadChunk(SCENE_CHUNK_SKIN_COUNTS, paiCounts, sizeof(int32)); }
size	CSceneFileReader::NumSkinWeights(void) { return NumElements(SCENE_CHUNK_SKIN_WEIGHTS, sizeof(float32)); }
bool	CSceneFileReader::ReadSkinMatrices(int32* paiMatrices) { return ReadChunk(SCENE_CHUNK_SKIN_MATRICES, paiMatrices, sizeof(int32)); }
bool	CSceneFileReader::ReadSkinWeights(float32* pafWeights) { return ReadChunk(SCENE_CHUNK_SKIN_WEIGHTS, pafWeights, sizeof(float32)); }
size	CSceneFileReader::NumInverseMatrices(void) { return NumElements(SCENE_CHUNK_INVERSE_MATRICES, SCENE_FILE_MATRIX_SIZE); }
bool	CSceneFileReader::ReadInverseMatrices(float32* paf4x4s) { return ReadChunk(SCENE_CHUNK_INVERSE_MATRICES, paf4x4s, SCENE_FILE_MATRIX_SIZE); }

size	CSceneFileReader::NumUVLayers(void) { return NumChunks(SCENE_CHUNK_UV_LAYER); }
bool	CSceneFileReader::BeginUVLayer(size uiLayer) { return Begin(SCENE_CHUNK_UV_LAYER, uiLayer); }
size	CSceneFileReader::NumUVs(void) { return NumElements(SCENE_CHUNK_UVS, sizeof(float32) * 2); }
bool	CSceneFileReader::ReadUVs(float32* pafUVs) { return ReadChunk(SCENE_CHUNK_UVS, pafUVs, sizeof(float32) * 2); }
size	CSceneFileReader::NumCornerUVs(void) { return NumElements(SCENE_CHUNK_CORNER_UVS, sizeof(int32)); }
bool	CSceneFileReader::ReadCornerUVs(int32* paiUVs) { return ReadChunk(SCENE_CHUNK_CORNER_UVS, paiUVs, sizeof(int32)); }

size	CSceneFileReader::NumImages(void) { return NumChunks(SCENE_CHUNK_IMAGE); }
bool	CSceneFileReader::BeginImage(size uiImage) { return Begin(SCENE_CHUNK_IMAGE, uiImage); }
bool	CSceneFileReader::ReadChannels(CChars* pszChannels) { return ReadString(SCENE_CHUNK_CHANNELS, pszChannels); }
size	CSceneFileReader::NumPixelBytes(void) { return NumElements(SCENE_CHUNK_PIXELS, sizeof(uint8)); }
bool	CSceneFileReader::ReadPixels(uint8* puiPixels) { return ReadChunk(SCENE_CHUNK_PIXELS, puiPixels, sizeof(uint8)); }

size	CSceneFileReader::NumMaterials(void) { return NumChunks(SCENE_CHUNK_MATERIAL); }
bool	CSceneFileReader::BeginMaterial(size uiMaterial) { return Begin(SCENE_CHUNK_MATERIAL, uiMaterial); }
bool	CSceneFileReader::ReadMaterialColour(ESceneFileMaterialColour eColour, float32* pafRGBA) { return ReadFixed(gaszSceneFileMaterialColours[eColour], pafRGBA, sizeof(float32) * 4); }
int32	CSceneFileReader::GetMaterialTexture(ESceneFileTexture eTexture) { return GetInt32(gaszSceneFileTextures[eTexture], SCENE_FILE_NONE); }

size	CSceneFileReader::NumConnections(void) { return NumChunks(SCENE_CHUNK_CONNECTION); }
bool	CSceneFileReader::BeginConnection(size uiConnection) { return Begin(SCENE_CHUNK_CONNECTION, uiConnection); }
int32	CSceneFileReader::GetParent(void) { return GetInt32(SCENE_CHUNK_PARENT, SCENE_FILE_NONE); }
bool	CSceneFileReader::ReadWorldMatrix(float32* paf4x4) { return ReadFixed(SCENE_CHUNK_WORLD, paf4x4, SCENE_FILE_MATRIX_SIZE); }
bool	CSceneFileReader::ReadLocalMatrix(float32* paf4x4) { return ReadFixed(SCENE_CHUNK_LOCAL, paf4x4, SCENE_FILE_MATRIX_SIZE); }
bool	CSceneFileReader::GetParity(void) { return GetInt32(SCENE_CHUNK_PARITY, 0) != 0; }

size	CSceneFileReader::NumLights(void) { return NumChunks(SCENE_CHUNK_LIGHT); }
bool	CSceneFileReader::BeginLight(size uiLight) { return Begin(SCENE_CHUNK_LIGHT, uiLight); }
ESceneFileLightType		CSceneFileReader::GetLightType(void) { return (ESceneFileLightType)GetInt32(SCENE_CHUNK_LIGHT_TYPE, SFLT_Omni); }
bool	CSceneFileReader::ReadLightColour(float32* pafRGBA) { return ReadFixed(SCENE_CHUNK_COLOUR, pafRGBA, sizeof(float32) * 4); }
ESceneFileLightDecay	CSceneFileReader::GetLightDecay(void) { return (ESceneFileLightDecay)GetInt32(SCENE_CHUNK_LIGHT_DECAY, SFLD_None); }
bool	CSceneFileReader::GetShadows(void) { return GetInt32(SCENE_CHUNK_SHADOWS, 0) != 0; }

size	CSceneFileReader::NumCameras(void) { return NumChunks(SCENE_CHUNK_CAMERA); }
bool	CSceneFileReader::BeginCamera(size uiCamera) { return Begin(SCENE_CHUNK_CAMERA, uiCamera); }
ESceneFileCameraType	CSceneFileReader::GetCameraType(void) { return (ESceneFileCameraType)GetInt32(SCENE_CHUNK_CAMERA_TYPE, SFCT_Perspective); }

int32	CSceneFileReader::GetAttached(void) { return GetInt32(SCENE_CHUNK_ATTACHED, SCENE_FILE_NONE); }

size	CSceneFileReader::NumAnimations(void) { return NumChunks(SCENE_CHUNK_ANIMATION); }
bool	CSceneFileReader::BeginAnimation(size uiAnimation) { return Begin(SCENE_CHUNK_ANIMATION, uiAnimation); }
size	CSceneFileReader::NumTracks(void) { return NumChunks(SCENE_CHUNK_TRACK); }
bool	CSceneFileReader::BeginTrack(size uiTrack) { return Begin(SCENE_CHUNK_TRACK, uiTrack); }
size	CSceneFileReader::NumKeys(void) { return NumElements(SCENE_CHUNK_KEY_TIMES, sizeof(float32)); }
bool	CSceneFileReader::ReadKeyTimes(float32* pafTimes) { return ReadChunk(SCENE_CHUNK_KEY_TIMES, pafTimes, sizeof(float32)); }
bool	CSceneFileReader::ReadKeyMatrices(float32* paf4x4s) { return ReadChunk(SCENE_CHUNK_KEY_MATRICES, paf4x4s, SCENE_FILE_MATRIX_SIZE); }

size	CSceneFileReader::NumInstances(void) { return NumChunks(SCENE_CHUNK_INSTANCE); }
bool	CSceneFileReader::BeginInstance(size uiInstance) { return Begin(SCENE_CHUNK_INSTANCE, uiInstance); }
size	CSceneFileReader::NumInstanceConnections(void) { return NumElements(SCENE_CHUNK_CONNECTIONS, sizeof(int32)); }
bool	CSceneFileReader::ReadInstanceConnections(int32* paiConnections) { return ReadChunk(SCENE_CHUNK_CONNECTIONS, paiConnections, sizeof(int32)); }
size	CSceneFileReader::NumInstanceMeshes(void) { return NumChunks(SCENE_CHUNK_INSTANCE_MESH); }
bool	CSceneFileReader::BeginInstanceMesh(size uiMesh) { return Begin(SCENE_CHUNK_INSTANCE_MESH, uiMesh); }
int32	CSceneFileReader::GetInstanceMesh(void) { return GetInt32(SCENE_CHUNK_MESH_INDEX, SCENE_FILE_NONE); }
size	CSceneFileReader::NumConnectionSlots(void) { return NumElements(SCENE_CHUNK_CONNECTION_SLOTS, sizeof(int32)); }
bool	CSceneFileReader::ReadConnectionSlots(int32* paiSlots) { return ReadChunk(SCENE_CHUNK_CONNECTION_SLOTS, paiSlots, sizeof(int32)); }
size	CSceneFileReader::NumInstanceMaterials(void) { return NumElements(SCENE_CHUNK_MATERIALS, sizeof(int32)); }
bool	CSceneFileReader::ReadInstanceMaterials(int32* paiMaterials) { return ReadChunk(SCENE_CHUNK_MATERIALS, paiMaterials, sizeof(int32)); }

size	CSceneFileReader::NumParticleSets(void) { return NumChunks(SCENE_CHUNK_PARTICLES); }
bool	CSceneFileReader::BeginParticles(size uiSet) { return Begin(SCENE_CHUNK_PARTICLES, uiSet); }
size	CSceneFileReader::NumParticles(void) { return NumElements(SCENE_CHUNK_POSITIONS, sizeof(float32) * 3); }
size	CSceneFileReader::NumParticleColours(void) { return NumElements(SCENE_CHUNK_COLOURS, sizeof(float32) * 3); }
size	CSceneFileReader::NumParticleRadii(void) { return NumElements(SCENE_CHUNK_RADII, sizeof(float32)); }
size	CSceneFileReader::NumParticleIntensities(void) { return NumElements(SCENE_CHUNK_INTENSITIES, sizeof(float32)); }
bool	CSceneFileReader::ReadParticlePositions(float32* pafXYZ) { return ReadChunk(SCENE_CHUNK_POSITIONS, pafXYZ, sizeof(float32) * 3); }
bool	CSceneFileReader::ReadParticleColours(float32* pafRGB) { return ReadChunk(SCENE_CHUNK_COLOURS, pafRGB, sizeof(float32) * 3); }
bool	CSceneFileReader::ReadParticleRadii(float32* pafRadii) { return ReadChunk(SCENE_CHUNK_RADII, pafRadii, sizeof(float32)); }
bool	CSceneFileReader::ReadParticleIntensities(float32* pafIntensities) { return ReadChunk(SCENE_CHUNK_INTENSITIES, pafIntensities, sizeof(float32)); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileReader::ReadImageSize(int32* piWidth, int32* piHeight)
{
	int32	aiSize[2];

	ReturnOnFalse(ReadFixed(SCENE_CHUNK_SIZE, aiSize, sizeof(aiSize)));
	*piWidth = aiSize[0];
	*piHeight = aiSize[1];
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileReader::ReadShininess(float32* pfPower, float32* pfBase)
{
	float32	afShininess[2];

	ReturnOnFalse(ReadFixed(SCENE_CHUNK_SHININESS, afShininess, sizeof(afShininess)));
	*pfPower = afShininess[0];
	*pfBase = afShininess[1];
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileReader::ReadLightValues(float32* pfDecayStart, float32* pfRange, float32* pfIntensity)
{
	float32	afValues[3];

	ReturnOnFalse(ReadFixed(SCENE_CHUNK_LIGHT_VALUES, afValues, sizeof(afValues)));
	*pfDecayStart = afValues[0];
	*pfRange = afValues[1];
	*pfIntensity = afValues[2];
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileReader::ReadLens(float32* pfFieldOfView, float32* pfNear, float32* pfFar)
{
	float32	afLens[3];

	ReturnOnFalse(ReadFixed(SCENE_CHUNK_LENS, afLens, sizeof(afLens)));
	*pfFieldOfView = afLens[0];
	*pfNear = afLens[1];
	*pfFar = afLens[2];
	return true;
}


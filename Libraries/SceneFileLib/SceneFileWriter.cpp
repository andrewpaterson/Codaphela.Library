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
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/Logger.h"
#include "SceneFileWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneFileWriter::Init(const char* szFilename)
{
	mszFilename.Init(szFilename);
	mbOpen = false;
	mbFailed = false;
	meBlock = SFB_None;
	memset(&msMesh, 0, sizeof(SSceneFileMeshCounts));
	muiNumMeshes = 0;
	muiNumImages = 0;
	muiNumMaterials = 0;
	muiNumConnections = 0;
	muiNumInstanceConnections = 0;
	miMaxMesh = SCENE_FILE_NONE;
	miMaxImage = SCENE_FILE_NONE;
	miMaxMaterial = SCENE_FILE_NONE;
	miMaxConnection = SCENE_FILE_NONE;
}


//////////////////////////////////////////////////////////////////////////
//
// A file that's still open was never finished, so it's as if it failed.
//
//////////////////////////////////////////////////////////////////////////
void CSceneFileWriter::Kill(void)
{
	if (mbOpen)
	{
		Fail("Was not closed");
		Close();
	}
	mszFilename.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::Open(void)
{
	if (mbOpen)
	{
		return Fail("Is already open");
	}

	mcFile.Init(DiskFile(mszFilename.Text()));
	if (!mcFile.WriteOpen(SCENE_FILE_ID))
	{
		mcFile.Kill();
		return Fail("Could not create the file");
	}
	mbOpen = true;
	return WriteInt32Chunk(SCENE_CHUNK_VERSION, SCENE_FILE_VERSION);
}


//////////////////////////////////////////////////////////////////////////
//
// Checks everything refers to something that's there.  Things can be
// written in any order, so this can only be done at the end.
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::Close(void)
{
	CFileUtil	cFileUtil;
	bool		bClosed;

	if (!mbOpen)
	{
		return Fail("Is not open");
	}

	if (!mbFailed)
	{
		if (meBlock != SFB_None)
		{
			Fail("Something was begun and not ended");
		}
		else if ((miMaxMesh >= (int32)muiNumMeshes) || (miMaxImage >= (int32)muiNumImages) || (miMaxMaterial >= (int32)muiNumMaterials) || (miMaxConnection >= (int32)muiNumConnections))
		{
			Fail("Something refers to a mesh, image, material or connection that isn't there");
		}
	}

	bClosed = mcFile.WriteClose();
	mcFile.Kill();
	mbOpen = false;

	if (!bClosed)
	{
		Fail("Could not finish the file");
	}
	if (mbFailed)
	{
		cFileUtil.Delete(mszFilename.Text());
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::Fail(const char* szMessage)
{
	if (!mbFailed)
	{
		mbFailed = true;
		gcLogger.Error2(__METHOD__, " ", szMessage, " in [", mszFilename.Text(), "].", NULL);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::InBlock(ESceneFileBlock eBlock)
{
	if (mbFailed)
	{
		return false;
	}
	if (meBlock != eBlock)
	{
		return Fail("Written outside the Begin and End it belongs in");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::Begin(ESceneFileBlock eBlock, const char* szChunk, const char* szName)
{
	if (!mbOpen)
	{
		return Fail("Is not open");
	}
	if (!InBlock(SFB_None))
	{
		return false;
	}
	if (!mcFile.WriteChunkBegin())
	{
		return Fail("Could not write a chunk");
	}
	meBlock = eBlock;
	return WriteChunk(SCENE_CHUNK_NAME, szName, sizeof(char), strlen(szName));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::End(ESceneFileBlock eBlock, const char* szChunk)
{
	if (!InBlock(eBlock))
	{
		return false;
	}
	meBlock = SFB_None;
	if (!mcFile.WriteChunkEnd(szChunk))
	{
		return Fail("Could not write a chunk");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteChunk(const char* szChunk, const void* pvData, size uiElementSize, size uiNumElements)
{
	if (mbFailed)
	{
		return false;
	}
	if (!mbOpen)
	{
		return Fail("Is not open");
	}
	if ((pvData == NULL) && (uiNumElements > 0))
	{
		return Fail("No data was given");
	}

	if (!mcFile.WriteChunkBegin() ||
		((uiNumElements > 0) && !mcFile.WriteData(pvData, (lsize)uiElementSize * uiNumElements)) ||
		!mcFile.WriteChunkEnd(szChunk))
	{
		return Fail("Could not write a chunk");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteInt32Chunk(const char* szChunk, int32 iValue)
{
	return WriteChunk(szChunk, &iValue, sizeof(int32), 1);
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps the largest index.  SCENE_FILE_NONE is allowed, and anything below it
// is wrong.
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::CheckIndices(const int32* paiIndices, size uiNumIndices, int32* piMax)
{
	size	i;

	if ((paiIndices == NULL) && (uiNumIndices > 0))
	{
		return Fail("No indices were given");
	}
	for (i = 0; i < uiNumIndices; i++)
	{
		if (paiIndices[i] < SCENE_FILE_NONE)
		{
			return Fail("An index is below -1");
		}
		if (paiIndices[i] > *piMax)
		{
			*piMax = paiIndices[i];
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// For indices where being none means nothing.  A polygon's corner can go
// without a normal but not without a position, and something that's not
// attached leaves out being attached.
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::CheckRequiredIndices(const int32* paiIndices, size uiNumIndices, int32* piMax)
{
	size	i;

	ReturnOnFalse(CheckIndices(paiIndices, uiNumIndices, piMax));
	for (i = 0; i < uiNumIndices; i++)
	{
		if (paiIndices[i] == SCENE_FILE_NONE)
		{
			return Fail("An index that has to be there is -1");
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::BeginMesh(const char* szName)
{
	ReturnOnFalse(Begin(SFB_Mesh, SCENE_CHUNK_MESH, szName));

	memset(&msMesh, 0, sizeof(SSceneFileMeshCounts));
	msMesh.iMaxPosition = SCENE_FILE_NONE;
	msMesh.iMaxNormal = SCENE_FILE_NONE;
	msMesh.iMaxColour = SCENE_FILE_NONE;
	msMesh.iMaxSlot = SCENE_FILE_NONE;
	msMesh.iMaxSkinMatrix = SCENE_FILE_NONE;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WritePositions(const float32* pafXYZ, size uiNumPositions)
{
	ReturnOnFalse(InBlock(SFB_Mesh));
	msMesh.uiPositions = uiNumPositions;
	return WriteChunk(SCENE_CHUNK_POSITIONS, pafXYZ, sizeof(float32) * 3, uiNumPositions);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteNormals(const float32* pafXYZ, size uiNumNormals)
{
	ReturnOnFalse(InBlock(SFB_Mesh));
	msMesh.uiNormals = uiNumNormals;
	return WriteChunk(SCENE_CHUNK_NORMALS, pafXYZ, sizeof(float32) * 3, uiNumNormals);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteColours(const uint32* pauiColours, size uiNumColours)
{
	ReturnOnFalse(InBlock(SFB_Mesh));
	msMesh.uiColours = uiNumColours;
	return WriteChunk(SCENE_CHUNK_COLOURS, pauiColours, sizeof(uint32), uiNumColours);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WritePolygons(const int32* paiSizes, size uiNumPolygons, const int32* paiCornerPositions, size uiNumCorners)
{
	size	i;
	size	uiTotal;

	ReturnOnFalse(InBlock(SFB_Mesh));

	uiTotal = 0;
	for (i = 0; i < uiNumPolygons; i++)
	{
		if (paiSizes[i] < 3)
		{
			return Fail("A polygon has fewer than three corners");
		}
		uiTotal += (size)paiSizes[i];
	}
	if (uiTotal != uiNumCorners)
	{
		return Fail("The polygons' sizes don't add up to their corners");
	}
	ReturnOnFalse(CheckRequiredIndices(paiCornerPositions, uiNumCorners, &msMesh.iMaxPosition));

	msMesh.uiPolygons = uiNumPolygons;
	msMesh.uiCorners = uiNumCorners;
	ReturnOnFalse(WriteChunk(SCENE_CHUNK_POLYGON_SIZES, paiSizes, sizeof(int32), uiNumPolygons));
	return WriteChunk(SCENE_CHUNK_CORNER_POSITIONS, paiCornerPositions, sizeof(int32), uiNumCorners);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteCornerNormals(const int32* paiNormals, size uiNumCorners)
{
	ReturnOnFalse(InBlock(SFB_Mesh));
	ReturnOnFalse(CheckIndices(paiNormals, uiNumCorners, &msMesh.iMaxNormal));
	msMesh.uiCornerNormals = uiNumCorners;
	return WriteChunk(SCENE_CHUNK_CORNER_NORMALS, paiNormals, sizeof(int32), uiNumCorners);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteCornerColours(const int32* paiColours, size uiNumCorners)
{
	ReturnOnFalse(InBlock(SFB_Mesh));
	ReturnOnFalse(CheckIndices(paiColours, uiNumCorners, &msMesh.iMaxColour));
	msMesh.uiCornerColours = uiNumCorners;
	return WriteChunk(SCENE_CHUNK_CORNER_COLOURS, paiColours, sizeof(int32), uiNumCorners);
}


//////////////////////////////////////////////////////////////////////////
//
// A layer's own indices can be checked here as it comes all at once.  Its
// corners are checked with the others when the mesh ends.  Write the polygons
// first.
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteUVLayer(const float32* pafUVs, size uiNumUVs, const int32* paiCornerUVs, size uiNumCorners)
{
	int32	iMaxUV;

	ReturnOnFalse(InBlock(SFB_Mesh));

	iMaxUV = SCENE_FILE_NONE;
	ReturnOnFalse(CheckIndices(paiCornerUVs, uiNumCorners, &iMaxUV));
	if (iMaxUV >= (int32)uiNumUVs)
	{
		return Fail("A corner refers to a UV that isn't there");
	}
	if (uiNumCorners != msMesh.uiCorners)
	{
		return Fail("A UV layer doesn't have one index for each corner");
	}

	if (!mcFile.WriteChunkBegin())
	{
		return Fail("Could not write a chunk");
	}
	ReturnOnFalse(WriteChunk(SCENE_CHUNK_UVS, pafUVs, sizeof(float32) * 2, uiNumUVs));
	ReturnOnFalse(WriteChunk(SCENE_CHUNK_CORNER_UVS, paiCornerUVs, sizeof(int32), uiNumCorners));
	if (!mcFile.WriteChunkEnd(SCENE_CHUNK_UV_LAYER))
	{
		return Fail("Could not write a chunk");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteMaterialSlots(const int32* paiMaterials, size uiNumSlots)
{
	ReturnOnFalse(InBlock(SFB_Mesh));
	ReturnOnFalse(CheckRequiredIndices(paiMaterials, uiNumSlots, &miMaxMaterial));
	msMesh.uiMaterialSlots = uiNumSlots;
	return WriteChunk(SCENE_CHUNK_MATERIAL_SLOTS, paiMaterials, sizeof(int32), uiNumSlots);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WritePolygonSlots(const int32* paiSlots, size uiNumPolygons)
{
	ReturnOnFalse(InBlock(SFB_Mesh));
	ReturnOnFalse(CheckIndices(paiSlots, uiNumPolygons, &msMesh.iMaxSlot));
	msMesh.uiPolygonSlots = uiNumPolygons;
	return WriteChunk(SCENE_CHUNK_POLYGON_SLOTS, paiSlots, sizeof(int32), uiNumPolygons);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteSmoothGroups(const uint32* pauiGroups, size uiNumPolygons)
{
	ReturnOnFalse(InBlock(SFB_Mesh));
	msMesh.uiSmoothGroups = uiNumPolygons;
	return WriteChunk(SCENE_CHUNK_SMOOTH_GROUPS, pauiGroups, sizeof(uint32), uiNumPolygons);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteSkin(const int32* paiCounts, size uiNumPositions, const int32* paiMatrices, const float32* pafWeights, size uiNumWeights)
{
	size	i;
	size	uiTotal;

	ReturnOnFalse(InBlock(SFB_Mesh));

	uiTotal = 0;
	for (i = 0; i < uiNumPositions; i++)
	{
		if (paiCounts[i] < 0)
		{
			return Fail("A position has fewer than no weights");
		}
		uiTotal += (size)paiCounts[i];
	}
	if (uiTotal != uiNumWeights)
	{
		return Fail("The positions' counts don't add up to the weights");
	}
	ReturnOnFalse(CheckRequiredIndices(paiMatrices, uiNumWeights, &msMesh.iMaxSkinMatrix));

	msMesh.uiSkinCounts = uiNumPositions;
	ReturnOnFalse(WriteChunk(SCENE_CHUNK_SKIN_COUNTS, paiCounts, sizeof(int32), uiNumPositions));
	ReturnOnFalse(WriteChunk(SCENE_CHUNK_SKIN_MATRICES, paiMatrices, sizeof(int32), uiNumWeights));
	return WriteChunk(SCENE_CHUNK_SKIN_WEIGHTS, pafWeights, sizeof(float32), uiNumWeights);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteInverseMatrices(const float32* paf4x4s, size uiNumMatrices)
{
	ReturnOnFalse(InBlock(SFB_Mesh));
	msMesh.uiInverseMatrices = uiNumMatrices;
	return WriteChunk(SCENE_CHUNK_INVERSE_MATRICES, paf4x4s, sizeof(float32) * SCENE_FILE_FLOATS_IN_MATRIX, uiNumMatrices);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::CheckMesh(void)
{
	if (msMesh.iMaxPosition >= (int32)msMesh.uiPositions)
	{
		return Fail("A corner refers to a position that isn't there");
	}
	if (msMesh.iMaxNormal >= (int32)msMesh.uiNormals)
	{
		return Fail("A corner refers to a normal that isn't there");
	}
	if (msMesh.iMaxColour >= (int32)msMesh.uiColours)
	{
		return Fail("A corner refers to a colour that isn't there");
	}
	if ((msMesh.uiCornerNormals != 0) && (msMesh.uiCornerNormals != msMesh.uiCorners))
	{
		return Fail("There isn't one normal index for each corner");
	}
	if ((msMesh.uiCornerColours != 0) && (msMesh.uiCornerColours != msMesh.uiCorners))
	{
		return Fail("There isn't one colour index for each corner");
	}
	if ((msMesh.uiPolygonSlots != 0) && (msMesh.uiPolygonSlots != msMesh.uiPolygons))
	{
		return Fail("There isn't one material slot for each polygon");
	}
	if (msMesh.iMaxSlot >= (int32)msMesh.uiMaterialSlots)
	{
		return Fail("A polygon refers to a material slot that isn't there");
	}
	if ((msMesh.uiSmoothGroups != 0) && (msMesh.uiSmoothGroups != msMesh.uiPolygons))
	{
		return Fail("There isn't one smoothing group for each polygon");
	}
	if ((msMesh.uiSkinCounts != 0) && (msMesh.uiSkinCounts != msMesh.uiPositions))
	{
		return Fail("There isn't one skin count for each position");
	}
	if (msMesh.iMaxSkinMatrix >= (int32)msMesh.uiInverseMatrices)
	{
		return Fail("A weight refers to a matrix that isn't there");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::EndMesh(void)
{
	ReturnOnFalse(InBlock(SFB_Mesh));
	ReturnOnFalse(CheckMesh());
	muiNumMeshes++;
	return End(SFB_Mesh, SCENE_CHUNK_MESH);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::BeginImage(const char* szName)
{
	return Begin(SFB_Image, SCENE_CHUNK_IMAGE, szName);
}


//////////////////////////////////////////////////////////////////////////
//
// szChannels has a char for each channel, as "RGBA".  Each is a byte.
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WritePixels(int32 iWidth, int32 iHeight, const char* szChannels, const uint8* puiPixels)
{
	int32	aiSize[2];
	size	uiNumChannels;

	ReturnOnFalse(InBlock(SFB_Image));

	if ((iWidth < 0) || (iHeight < 0))
	{
		return Fail("An image's size is below zero");
	}
	aiSize[0] = iWidth;
	aiSize[1] = iHeight;
	uiNumChannels = strlen(szChannels);

	ReturnOnFalse(WriteChunk(SCENE_CHUNK_SIZE, aiSize, sizeof(int32), 2));
	ReturnOnFalse(WriteChunk(SCENE_CHUNK_CHANNELS, szChannels, sizeof(char), uiNumChannels));
	return WriteChunk(SCENE_CHUNK_PIXELS, puiPixels, uiNumChannels, (size)iWidth * (size)iHeight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::EndImage(void)
{
	ReturnOnFalse(InBlock(SFB_Image));
	muiNumImages++;
	return End(SFB_Image, SCENE_CHUNK_IMAGE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::BeginMaterial(const char* szName)
{
	return Begin(SFB_Material, SCENE_CHUNK_MATERIAL, szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteMaterialColour(ESceneFileMaterialColour eColour, const float32* pafRGBA)
{
	ReturnOnFalse(InBlock(SFB_Material));
	return WriteChunk(gaszSceneFileMaterialColours[eColour], pafRGBA, sizeof(float32), 4);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteShininess(float32 fPower, float32 fBase)
{
	float32	afShininess[2];

	ReturnOnFalse(InBlock(SFB_Material));
	afShininess[0] = fPower;
	afShininess[1] = fBase;
	return WriteChunk(SCENE_CHUNK_SHININESS, afShininess, sizeof(float32), 2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteMaterialTexture(ESceneFileTexture eTexture, int32 iImage)
{
	ReturnOnFalse(InBlock(SFB_Material));
	ReturnOnFalse(CheckRequiredIndices(&iImage, 1, &miMaxImage));
	return WriteInt32Chunk(gaszSceneFileTextures[eTexture], iImage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::EndMaterial(void)
{
	ReturnOnFalse(InBlock(SFB_Material));
	muiNumMaterials++;
	return End(SFB_Material, SCENE_CHUNK_MATERIAL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::BeginConnection(const char* szName)
{
	return Begin(SFB_Connection, SCENE_CHUNK_CONNECTION, szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteParent(int32 iConnection)
{
	ReturnOnFalse(InBlock(SFB_Connection));
	ReturnOnFalse(CheckRequiredIndices(&iConnection, 1, &miMaxConnection));
	return WriteInt32Chunk(SCENE_CHUNK_PARENT, iConnection);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteWorldMatrix(const float32* paf4x4)
{
	ReturnOnFalse(InBlock(SFB_Connection));
	return WriteChunk(SCENE_CHUNK_WORLD, paf4x4, sizeof(float32), SCENE_FILE_FLOATS_IN_MATRIX);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteLocalMatrix(const float32* paf4x4)
{
	ReturnOnFalse(InBlock(SFB_Connection));
	return WriteChunk(SCENE_CHUNK_LOCAL, paf4x4, sizeof(float32), SCENE_FILE_FLOATS_IN_MATRIX);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteParity(bool bParity)
{
	ReturnOnFalse(InBlock(SFB_Connection));
	return WriteInt32Chunk(SCENE_CHUNK_PARITY, bParity ? 1 : 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::EndConnection(void)
{
	ReturnOnFalse(InBlock(SFB_Connection));
	muiNumConnections++;
	return End(SFB_Connection, SCENE_CHUNK_CONNECTION);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::BeginLight(const char* szName)
{
	return Begin(SFB_Light, SCENE_CHUNK_LIGHT, szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteLightType(ESceneFileLightType eType)
{
	ReturnOnFalse(InBlock(SFB_Light));
	return WriteInt32Chunk(SCENE_CHUNK_LIGHT_TYPE, eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteLightColour(const float32* pafRGBA)
{
	ReturnOnFalse(InBlock(SFB_Light));
	return WriteChunk(SCENE_CHUNK_COLOUR, pafRGBA, sizeof(float32), 4);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteLightValues(float32 fDecayStart, float32 fRange, float32 fIntensity)
{
	float32	afValues[3];

	ReturnOnFalse(InBlock(SFB_Light));
	afValues[0] = fDecayStart;
	afValues[1] = fRange;
	afValues[2] = fIntensity;
	return WriteChunk(SCENE_CHUNK_LIGHT_VALUES, afValues, sizeof(float32), 3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteLightDecay(ESceneFileLightDecay eDecay)
{
	ReturnOnFalse(InBlock(SFB_Light));
	return WriteInt32Chunk(SCENE_CHUNK_LIGHT_DECAY, eDecay);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteShadows(bool bShadows)
{
	ReturnOnFalse(InBlock(SFB_Light));
	return WriteInt32Chunk(SCENE_CHUNK_SHADOWS, bShadows ? 1 : 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::EndLight(void)
{
	return End(SFB_Light, SCENE_CHUNK_LIGHT);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::BeginCamera(const char* szName)
{
	return Begin(SFB_Camera, SCENE_CHUNK_CAMERA, szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteCameraType(ESceneFileCameraType eType)
{
	ReturnOnFalse(InBlock(SFB_Camera));
	return WriteInt32Chunk(SCENE_CHUNK_CAMERA_TYPE, eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteLens(float32 fFieldOfView, float32 fNear, float32 fFar)
{
	float32	afLens[3];

	ReturnOnFalse(InBlock(SFB_Camera));
	afLens[0] = fFieldOfView;
	afLens[1] = fNear;
	afLens[2] = fFar;
	return WriteChunk(SCENE_CHUNK_LENS, afLens, sizeof(float32), 3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::EndCamera(void)
{
	return End(SFB_Camera, SCENE_CHUNK_CAMERA);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteAttached(int32 iConnection)
{
	if (mbFailed)
	{
		return false;
	}
	if ((meBlock != SFB_Light) && (meBlock != SFB_Camera))
	{
		return Fail("Only a light or a camera is attached");
	}
	ReturnOnFalse(CheckRequiredIndices(&iConnection, 1, &miMaxConnection));
	return WriteInt32Chunk(SCENE_CHUNK_ATTACHED, iConnection);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::BeginAnimation(const char* szName)
{
	return Begin(SFB_Animation, SCENE_CHUNK_ANIMATION, szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteTrack(int32 iConnection, const float32* pafTimes, const float32* paf4x4s, size uiNumKeys)
{
	size	i;

	ReturnOnFalse(InBlock(SFB_Animation));
	ReturnOnFalse(CheckRequiredIndices(&iConnection, 1, &miMaxConnection));

	for (i = 1; i < uiNumKeys; i++)
	{
		if (pafTimes[i] <= pafTimes[i - 1])
		{
			return Fail("A track's times don't go up");
		}
	}

	if (!mcFile.WriteChunkBegin())
	{
		return Fail("Could not write a chunk");
	}
	ReturnOnFalse(WriteInt32Chunk(SCENE_CHUNK_ATTACHED, iConnection));
	ReturnOnFalse(WriteChunk(SCENE_CHUNK_KEY_TIMES, pafTimes, sizeof(float32), uiNumKeys));
	ReturnOnFalse(WriteChunk(SCENE_CHUNK_KEY_MATRICES, paf4x4s, sizeof(float32) * SCENE_FILE_FLOATS_IN_MATRIX, uiNumKeys));
	if (!mcFile.WriteChunkEnd(SCENE_CHUNK_TRACK))
	{
		return Fail("Could not write a chunk");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::EndAnimation(void)
{
	return End(SFB_Animation, SCENE_CHUNK_ANIMATION);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::BeginInstance(const char* szName)
{
	ReturnOnFalse(Begin(SFB_Instance, SCENE_CHUNK_INSTANCE, szName));
	muiNumInstanceConnections = 0;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteConnections(const int32* paiConnections, size uiNumConnections)
{
	ReturnOnFalse(InBlock(SFB_Instance));
	ReturnOnFalse(CheckRequiredIndices(paiConnections, uiNumConnections, &miMaxConnection));
	muiNumInstanceConnections = uiNumConnections;
	return WriteChunk(SCENE_CHUNK_CONNECTIONS, paiConnections, sizeof(int32), uiNumConnections);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteInstanceMesh(int32 iMesh, const int32* paiConnectionSlots, size uiNumSlots, const int32* paiMaterials, size uiNumMaterials)
{
	int32	iMaxSlot;

	ReturnOnFalse(InBlock(SFB_Instance));
	ReturnOnFalse(CheckRequiredIndices(&iMesh, 1, &miMaxMesh));
	ReturnOnFalse(CheckIndices(paiMaterials, uiNumMaterials, &miMaxMaterial));

	iMaxSlot = SCENE_FILE_NONE;
	ReturnOnFalse(CheckIndices(paiConnectionSlots, uiNumSlots, &iMaxSlot));
	if (iMaxSlot >= (int32)muiNumInstanceConnections)
	{
		return Fail("A mesh refers to a slot that isn't in its instance's connections");
	}

	if (!mcFile.WriteChunkBegin())
	{
		return Fail("Could not write a chunk");
	}
	ReturnOnFalse(WriteInt32Chunk(SCENE_CHUNK_MESH_INDEX, iMesh));
	ReturnOnFalse(WriteChunk(SCENE_CHUNK_CONNECTION_SLOTS, paiConnectionSlots, sizeof(int32), uiNumSlots));
	if (uiNumMaterials > 0)
	{
		ReturnOnFalse(WriteChunk(SCENE_CHUNK_MATERIALS, paiMaterials, sizeof(int32), uiNumMaterials));
	}
	if (!mcFile.WriteChunkEnd(SCENE_CHUNK_INSTANCE_MESH))
	{
		return Fail("Could not write a chunk");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::EndInstance(void)
{
	return End(SFB_Instance, SCENE_CHUNK_INSTANCE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneFileWriter::WriteParticles(const char* szName, const float32* pafXYZ, const float32* pafRGB, const float32* pafRadii, const float32* pafIntensities, size uiNumParticles)
{
	ReturnOnFalse(Begin(SFB_Particles, SCENE_CHUNK_PARTICLES, szName));
	ReturnOnFalse(WriteChunk(SCENE_CHUNK_POSITIONS, pafXYZ, sizeof(float32) * 3, uiNumParticles));
	if (pafRGB)
	{
		ReturnOnFalse(WriteChunk(SCENE_CHUNK_COLOURS, pafRGB, sizeof(float32) * 3, uiNumParticles));
	}
	if (pafRadii)
	{
		ReturnOnFalse(WriteChunk(SCENE_CHUNK_RADII, pafRadii, sizeof(float32), uiNumParticles));
	}
	if (pafIntensities)
	{
		ReturnOnFalse(WriteChunk(SCENE_CHUNK_INTENSITIES, pafIntensities, sizeof(float32), uiNumParticles));
	}
	return End(SFB_Particles, SCENE_CHUNK_PARTICLES);
}


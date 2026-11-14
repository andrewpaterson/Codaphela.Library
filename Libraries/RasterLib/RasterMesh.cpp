/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RasterLib

Codaphela RasterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RasterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RasterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "BaseLib/DiskFile.h"
#include "StandardLib/ClassDefines.h"
#include "RasterMesh.h"


#define RASTER_MESH_FILE_MAGIC		0x48534D52  //RMSH.
#define RASTER_MESH_FILE_VERSION	3  //Version 2 added skins, 3 UVs and texture runs.


struct SRasterMeshFileHeader
{
	uint32		uiMagic;
	uint32		uiVersion;
	uint64		uiSource;
	uint32		uiNumVertices;
	uint32		uiNumOpaque;
	uint32		uiNumTransparent;
	uint32		uiNumSkinVertices;
	uint32		uiNumSkinWeights;
	uint32		uiNumMatrices;
	SInt32Vec3	sMin;
	SInt32Vec3	sMax;
	uint32		uiNumUVs;
	uint32		uiNumOpaqueRuns;
	uint32		uiNumTransparentRuns;
	uint32		uiPad;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterMesh::Init(void)
{
	PreInit();

	masVertices.Init();
	masOpaque.Init();
	masTransparent.Init();
	masSkinVertices.Init();
	masSkinWeights.Init();
	muiNumMatrices = 0;
	msMin.Init(0, 0, 0);
	msMax.Init(0, 0, 0);
	masUVs.Init();
	masOpaqueRuns.Init();
	masTransparentRuns.Init();
	mapTextures.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterMesh::Free(void)
{
	masTransparentRuns.Kill();
	masOpaqueRuns.Kill();
	masUVs.Kill();
	masSkinWeights.Kill();
	masSkinVertices.Kill();
	masTransparent.Kill();
	masOpaque.Kill();
	masVertices.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Its triangles are saved by WriteFile, not with the object.
//
//////////////////////////////////////////////////////////////////////////
void CRasterMesh::Class(void)
{
	U_Unknown(CArrayRasterVertex, masVertices);
	U_Unknown(CArrayRasterTriangle, masOpaque);
	U_Unknown(CArrayRasterTriangle, masTransparent);
	U_Unknown(CArrayRasterSkinVertex, masSkinVertices);
	U_Unknown(CArrayRasterSkinWeight, masSkinWeights);
	U_UInt32(muiNumMatrices);
	U_3Int32(msMin);
	U_3Int32(msMax);
	U_Unknown(CArrayInt32Vec2, masUVs);
	U_Unknown(CArrayRasterTextureRun, masOpaqueRuns);
	U_Unknown(CArrayRasterTextureRun, masTransparentRuns);
	M_Embedded(mapTextures);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterMesh::Clear(void)
{
	masVertices.ReInit();
	masOpaque.ReInit();
	masTransparent.ReInit();
	masSkinVertices.ReInit();
	masSkinWeights.ReInit();
	muiNumMatrices = 0;
	msMin.Init(0, 0, 0);
	msMax.Init(0, 0, 0);
	masUVs.ReInit();
	masOpaqueRuns.ReInit();
	masTransparentRuns.ReInit();
	mapTextures.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterMesh::UpdateBounds(void)
{
	size	i;

	msMin.Init(0, 0, 0);
	msMax.Init(0, 0, 0);
	if (masVertices.NumElements() == 0)
	{
		return;
	}
	msMin = masVertices.Get(0)->sPosition;
	msMax = msMin;
	for (i = 1; i < masVertices.NumElements(); i++)
	{
		Int32Vec3Minimize(&msMin, &msMin, &masVertices.Get(i)->sPosition);
		Int32Vec3Maximize(&msMax, &msMax, &masVertices.Get(i)->sPosition);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CRasterMesh::NumTriangles(void)
{
	return masOpaque.NumElements() + masTransparent.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRasterMesh::IsSkinned(void)
{
	return masSkinVertices.NumElements() > 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRasterMesh::IsTextured(void)
{
	return (masOpaqueRuns.NumElements() > 0) || (masTransparentRuns.NumElements() > 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Its index, which is the one it already has if it was added before.
//
//////////////////////////////////////////////////////////////////////////
int32 CRasterMesh::AddTexture(Ptr<CRasterTexture> pTexture)
{
	size	i;

	for (i = 0; i < mapTextures.NumElements(); i++)
	{
		if (&mapTextures.Get(i) == &pTexture)
		{
			return (int32)i;
		}
	}
	mapTextures.Add(pTexture);
	return (int32)(mapTextures.NumElements() - 1);
}


//////////////////////////////////////////////////////////////////////////
//
// Triangle uiTriangle of a list, the one after the last of its runs, is
// textured by iTexture.  It's in the last run if that's the same texture.
//
//////////////////////////////////////////////////////////////////////////
void CRasterMesh::AddRun(CArrayRasterTextureRun* pasRuns, uint32 uiTriangle, int32 iTexture)
{
	SRasterTextureRun*	psRun;

	psRun = pasRuns->Tail();
	if ((psRun != NULL) && (psRun->iTexture == iTexture) && (psRun->uiFirst + psRun->uiNumTriangles == uiTriangle))
	{
		psRun->uiNumTriangles++;
		return;
	}
	psRun = pasRuns->Add();
	psRun->uiFirst = uiTriangle;
	psRun->uiNumTriangles = 1;
	psRun->iTexture = iTexture;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL for none, and for one it hasn't got.
//
//////////////////////////////////////////////////////////////////////////
CRasterTexture* CRasterMesh::GetTexture(int32 iTexture)
{
	if ((iTexture < 0) || ((size)iTexture >= mapTextures.NumElements()))
	{
		return NULL;
	}
	return &mapTextures.Get((size)iTexture);
}


//////////////////////////////////////////////////////////////////////////
//
// The triangle's normal, not made a unit, by the right hand rule.
//
//////////////////////////////////////////////////////////////////////////
static void RasterTriangleNormal(CRasterMesh* pcMesh, SRasterTriangle* psTriangle, int64* piX, int64* piY, int64* piZ)
{
	SInt32Vec3*	psA;
	SInt32Vec3*	psB;
	SInt32Vec3*	psC;
	int64		iBX;
	int64		iBY;
	int64		iBZ;
	int64		iCX;
	int64		iCY;
	int64		iCZ;

	psA = &pcMesh->masVertices.Get(psTriangle->aiVertex[0])->sPosition;
	psB = &pcMesh->masVertices.Get(psTriangle->aiVertex[1])->sPosition;
	psC = &pcMesh->masVertices.Get(psTriangle->aiVertex[2])->sPosition;
	iBX = (int64)psB->x - psA->x;
	iBY = (int64)psB->y - psA->y;
	iBZ = (int64)psB->z - psA->z;
	iCX = (int64)psC->x - psA->x;
	iCY = (int64)psC->y - psA->y;
	iCZ = (int64)psC->z - psA->z;
	*piX = iBY * iCZ - iBZ * iCY;
	*piY = iBZ * iCX - iBX * iCZ;
	*piZ = iBX * iCY - iBY * iCX;
}


//////////////////////////////////////////////////////////////////////////
//
// Moves those from uiFirst to uiEnd with some area down to *puiKept.
//
//////////////////////////////////////////////////////////////////////////
static void RasterKeepTriangles(CRasterMesh* pcMesh, SRasterTriangle* psTriangle, size uiFirst, size uiEnd, size* puiKept)
{
	size	i;
	int64	iX;
	int64	iY;
	int64	iZ;

	for (i = uiFirst; i < uiEnd; i++)
	{
		RasterTriangleNormal(pcMesh, &psTriangle[i], &iX, &iY, &iZ);
		if ((iX != 0) || (iY != 0) || (iZ != 0))
		{
			psTriangle[*puiKept] = psTriangle[i];
			(*puiKept)++;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps the triangles with some area, and the runs with any of them, which
// still cover the list.  Returns how many triangles went.
//
//////////////////////////////////////////////////////////////////////////
static size RasterRemoveDegenerate(CRasterMesh* pcMesh, CArrayRasterTriangle* pasTriangles, CArrayRasterTextureRun* pasRuns)
{
	SRasterTriangle*	psTriangle;
	SRasterTextureRun*	psRun;
	SRasterTextureRun*	psKeptRun;
	size				uiNumTriangles;
	size				uiNumRuns;
	size				uiKept;
	size				uiRunsKept;
	size				uiFirstKept;
	size				i;

	uiNumTriangles = pasTriangles->NumElements();
	psTriangle = pasTriangles->GetData();
	uiNumRuns = pasRuns->NumElements();
	uiKept = 0;
	uiRunsKept = 0;
	if (uiNumRuns == 0)
	{
		RasterKeepTriangles(pcMesh, psTriangle, 0, uiNumTriangles, &uiKept);
	}
	for (i = 0; i < uiNumRuns; i++)
	{
		psRun = pasRuns->Get(i);
		uiFirstKept = uiKept;
		RasterKeepTriangles(pcMesh, psTriangle, psRun->uiFirst, psRun->uiFirst + psRun->uiNumTriangles, &uiKept);
		if (uiKept > uiFirstKept)
		{
			psKeptRun = pasRuns->Get(uiRunsKept);
			psKeptRun->iTexture = psRun->iTexture;
			psKeptRun->uiFirst = (uint32)uiFirstKept;
			psKeptRun->uiNumTriangles = (uint32)(uiKept - uiFirstKept);
			uiRunsKept++;
		}
	}
	if (uiKept < uiNumTriangles)
	{
		pasTriangles->RemoveRange(uiKept, uiNumTriangles);
	}
	if (uiRunsKept < uiNumRuns)
	{
		pasRuns->RemoveRange(uiRunsKept, uiNumRuns);
	}
	return uiNumTriangles - uiKept;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns how many triangles went.
//
//////////////////////////////////////////////////////////////////////////
size CRasterMesh::RemoveDegenerate(void)
{
	size	uiRemoved;

	uiRemoved = RasterRemoveDegenerate(this, &masOpaque, &masOpaqueRuns);
	uiRemoved += RasterRemoveDegenerate(this, &masTransparent, &masTransparentRuns);
	return uiRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRasterMesh::WriteFile(const char* szFilename, uint64 uiSource)
{
	CDiskFile				cFile;
	SRasterMeshFileHeader	sHeader;
	bool					bResult;

	memset(&sHeader, 0, sizeof(sHeader));
	sHeader.uiMagic = RASTER_MESH_FILE_MAGIC;
	sHeader.uiVersion = RASTER_MESH_FILE_VERSION;
	sHeader.uiSource = uiSource;
	sHeader.uiNumVertices = (uint32)masVertices.NumElements();
	sHeader.uiNumOpaque = (uint32)masOpaque.NumElements();
	sHeader.uiNumTransparent = (uint32)masTransparent.NumElements();
	sHeader.uiNumSkinVertices = (uint32)masSkinVertices.NumElements();
	sHeader.uiNumSkinWeights = (uint32)masSkinWeights.NumElements();
	sHeader.uiNumMatrices = muiNumMatrices;
	sHeader.sMin = msMin;
	sHeader.sMax = msMax;
	sHeader.uiNumUVs = (uint32)masUVs.NumElements();
	sHeader.uiNumOpaqueRuns = (uint32)masOpaqueRuns.NumElements();
	sHeader.uiNumTransparentRuns = (uint32)masTransparentRuns.NumElements();

	cFile.Init(szFilename);
	if (!cFile.Open(EFM_Write_Replace))
	{
		cFile.Kill();
		return false;
	}
	bResult = cFile.Write(&sHeader, sizeof(sHeader), 1) == 1;
	bResult = bResult && ((sHeader.uiNumVertices == 0) || (cFile.Write(masVertices.GetData(), sizeof(SRasterVertex), sHeader.uiNumVertices) == sHeader.uiNumVertices));
	bResult = bResult && ((sHeader.uiNumOpaque == 0) || (cFile.Write(masOpaque.GetData(), sizeof(SRasterTriangle), sHeader.uiNumOpaque) == sHeader.uiNumOpaque));
	bResult = bResult && ((sHeader.uiNumTransparent == 0) || (cFile.Write(masTransparent.GetData(), sizeof(SRasterTriangle), sHeader.uiNumTransparent) == sHeader.uiNumTransparent));
	bResult = bResult && ((sHeader.uiNumSkinVertices == 0) || (cFile.Write(masSkinVertices.GetData(), sizeof(SRasterSkinVertex), sHeader.uiNumSkinVertices) == sHeader.uiNumSkinVertices));
	bResult = bResult && ((sHeader.uiNumSkinWeights == 0) || (cFile.Write(masSkinWeights.GetData(), sizeof(SRasterSkinWeight), sHeader.uiNumSkinWeights) == sHeader.uiNumSkinWeights));
	bResult = bResult && ((sHeader.uiNumUVs == 0) || (cFile.Write(masUVs.GetData(), sizeof(SInt32Vec2), sHeader.uiNumUVs) == sHeader.uiNumUVs));
	bResult = bResult && ((sHeader.uiNumOpaqueRuns == 0) || (cFile.Write(masOpaqueRuns.GetData(), sizeof(SRasterTextureRun), sHeader.uiNumOpaqueRuns) == sHeader.uiNumOpaqueRuns));
	bResult = bResult && ((sHeader.uiNumTransparentRuns == 0) || (cFile.Write(masTransparentRuns.GetData(), sizeof(SRasterTextureRun), sHeader.uiNumTransparentRuns) == sHeader.uiNumTransparentRuns));
	cFile.Close();
	cFile.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// False, leaving the mesh as it was, if the file isn't there, isn't this
// version or wasn't built from uiSource.
//
//////////////////////////////////////////////////////////////////////////
bool CRasterMesh::ReadFile(const char* szFilename, uint64 uiSource)
{
	CDiskFile				cFile;
	SRasterMeshFileHeader	sHeader;
	bool					bResult;

	cFile.Init(szFilename);
	if (!cFile.Open(EFM_Read))
	{
		cFile.Kill();
		return false;
	}
	bResult = cFile.Read(&sHeader, sizeof(sHeader), 1) == 1;
	bResult = bResult && (sHeader.uiMagic == RASTER_MESH_FILE_MAGIC) && (sHeader.uiVersion == RASTER_MESH_FILE_VERSION) && (sHeader.uiSource == uiSource);
	if (bResult)
	{
		Clear();
		masVertices.Resize(sHeader.uiNumVertices);
		masOpaque.Resize(sHeader.uiNumOpaque);
		masTransparent.Resize(sHeader.uiNumTransparent);
		masSkinVertices.Resize(sHeader.uiNumSkinVertices);
		masSkinWeights.Resize(sHeader.uiNumSkinWeights);
		masUVs.Resize(sHeader.uiNumUVs);
		masOpaqueRuns.Resize(sHeader.uiNumOpaqueRuns);
		masTransparentRuns.Resize(sHeader.uiNumTransparentRuns);
		muiNumMatrices = sHeader.uiNumMatrices;
		bResult = (sHeader.uiNumVertices == 0) || (cFile.Read(masVertices.GetData(), sizeof(SRasterVertex), sHeader.uiNumVertices) == sHeader.uiNumVertices);
		bResult = bResult && ((sHeader.uiNumOpaque == 0) || (cFile.Read(masOpaque.GetData(), sizeof(SRasterTriangle), sHeader.uiNumOpaque) == sHeader.uiNumOpaque));
		bResult = bResult && ((sHeader.uiNumTransparent == 0) || (cFile.Read(masTransparent.GetData(), sizeof(SRasterTriangle), sHeader.uiNumTransparent) == sHeader.uiNumTransparent));
		bResult = bResult && ((sHeader.uiNumSkinVertices == 0) || (cFile.Read(masSkinVertices.GetData(), sizeof(SRasterSkinVertex), sHeader.uiNumSkinVertices) == sHeader.uiNumSkinVertices));
		bResult = bResult && ((sHeader.uiNumSkinWeights == 0) || (cFile.Read(masSkinWeights.GetData(), sizeof(SRasterSkinWeight), sHeader.uiNumSkinWeights) == sHeader.uiNumSkinWeights));
		bResult = bResult && ((sHeader.uiNumUVs == 0) || (cFile.Read(masUVs.GetData(), sizeof(SInt32Vec2), sHeader.uiNumUVs) == sHeader.uiNumUVs));
		bResult = bResult && ((sHeader.uiNumOpaqueRuns == 0) || (cFile.Read(masOpaqueRuns.GetData(), sizeof(SRasterTextureRun), sHeader.uiNumOpaqueRuns) == sHeader.uiNumOpaqueRuns));
		bResult = bResult && ((sHeader.uiNumTransparentRuns == 0) || (cFile.Read(masTransparentRuns.GetData(), sizeof(SRasterTextureRun), sHeader.uiNumTransparentRuns) == sHeader.uiNumTransparentRuns));
		msMin = sHeader.sMin;
		msMax = sHeader.sMax;
	}
	cFile.Close();
	cFile.Kill();
	return bResult;
}

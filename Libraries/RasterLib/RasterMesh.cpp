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
#include "RasterMesh.h"


#define RASTER_MESH_FILE_MAGIC		0x48534D52  //RMSH.
#define RASTER_MESH_FILE_VERSION	1


struct SRasterMeshFileHeader
{
	uint32		uiMagic;
	uint32		uiVersion;
	uint64		uiSource;
	uint32		uiNumVertices;
	uint32		uiNumOpaque;
	uint32		uiNumTransparent;
	uint32		uiPad;
	SInt32Vec3	sMin;
	SInt32Vec3	sMax;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterMesh::Init(void)
{
	masVertices.Init();
	masOpaque.Init();
	masTransparent.Init();
	msMin.Init(0, 0, 0);
	msMax.Init(0, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterMesh::Kill(void)
{
	masTransparent.Kill();
	masOpaque.Kill();
	masVertices.Kill();
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
	msMin.Init(0, 0, 0);
	msMax.Init(0, 0, 0);
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
// Keeps the triangles with some area.  Returns how many went.
//
//////////////////////////////////////////////////////////////////////////
static size RasterRemoveDegenerate(CRasterMesh* pcMesh, CArrayRasterTriangle* pasTriangles)
{
	SRasterTriangle*	psTriangle;
	size				uiNumTriangles;
	size				uiKept;
	size				i;
	int64				iX;
	int64				iY;
	int64				iZ;

	uiNumTriangles = pasTriangles->NumElements();
	psTriangle = pasTriangles->GetData();
	uiKept = 0;
	for (i = 0; i < uiNumTriangles; i++)
	{
		RasterTriangleNormal(pcMesh, &psTriangle[i], &iX, &iY, &iZ);
		if ((iX != 0) || (iY != 0) || (iZ != 0))
		{
			psTriangle[uiKept] = psTriangle[i];
			uiKept++;
		}
	}
	if (uiKept < uiNumTriangles)
	{
		pasTriangles->RemoveRange(uiKept, uiNumTriangles);
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

	uiRemoved = RasterRemoveDegenerate(this, &masOpaque);
	uiRemoved += RasterRemoveDegenerate(this, &masTransparent);
	return uiRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRasterMesh::Save(const char* szFilename, uint64 uiSource)
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
	sHeader.sMin = msMin;
	sHeader.sMax = msMax;

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
bool CRasterMesh::Load(const char* szFilename, uint64 uiSource)
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
		bResult = (sHeader.uiNumVertices == 0) || (cFile.Read(masVertices.GetData(), sizeof(SRasterVertex), sHeader.uiNumVertices) == sHeader.uiNumVertices);
		bResult = bResult && ((sHeader.uiNumOpaque == 0) || (cFile.Read(masOpaque.GetData(), sizeof(SRasterTriangle), sHeader.uiNumOpaque) == sHeader.uiNumOpaque));
		bResult = bResult && ((sHeader.uiNumTransparent == 0) || (cFile.Read(masTransparent.GetData(), sizeof(SRasterTriangle), sHeader.uiNumTransparent) == sHeader.uiNumTransparent));
		msMin = sHeader.sMin;
		msMax = sHeader.sMax;
	}
	cFile.Close();
	cFile.Kill();
	return bResult;
}

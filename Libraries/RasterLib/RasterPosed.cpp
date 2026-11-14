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
#include "BaseLib/Int64Vec3.h"
#include "RasterCamera.h"
#include "RasterPosed.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterPosed::Init(void)
{
	masVertices.Init();
	msMin.Init(0, 0, 0);
	msMax.Init(0, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterPosed::Kill(void)
{
	masVertices.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The largest whole number whose square isn't more than uiValue.
//
//////////////////////////////////////////////////////////////////////////
static uint64 RasterSquareRoot(uint64 uiValue)
{
	uint64	uiRoot;
	uint64	uiBit;

	uiRoot = 0;
	uiBit = (uint64)1 << 62;
	while (uiBit > uiValue)
	{
		uiBit >>= 2;
	}
	while (uiBit != 0)
	{
		if (uiValue >= uiRoot + uiBit)
		{
			uiValue -= uiRoot + uiBit;
			uiRoot = (uiRoot >> 1) + uiBit;
		}
		else
		{
			uiRoot >>= 1;
		}
		uiBit >>= 2;
	}
	return uiRoot;
}


//////////////////////////////////////////////////////////////////////////
//
// A blended normal made a unit again.  One that came to nothing stays
// nothing.
//
//////////////////////////////////////////////////////////////////////////
static void RasterUnitNormal(SRasterUnitVec3* psDest, SInt64Vec3* psNormal)
{
	int64	iLength;

	iLength = (int64)RasterSquareRoot((uint64)(psNormal->x * psNormal->x + psNormal->y * psNormal->y + psNormal->z * psNormal->z));
	if (iLength == 0)
	{
		psDest->Init(SRasterUnit::FromRaw(0), SRasterUnit::FromRaw(0), SRasterUnit::FromRaw(0));
		return;
	}
	psDest->Init(SRasterUnit::FromRaw((int16)((psNormal->x << SRasterUnit::Bits) / iLength)),
				 SRasterUnit::FromRaw((int16)((psNormal->y << SRasterUnit::Bits) / iLength)),
				 SRasterUnit::FromRaw((int16)((psNormal->z << SRasterUnit::Bits) / iLength)));
}


//////////////////////////////////////////////////////////////////////////
//
// False, leaving what it had, if the mesh isn't skinned or has a matrix
// asBones hasn't.  A weight's share of a position is rounded once, when
// they're added up, so a vertex all of one matrix is where that matrix
// puts it, as an instance would be.
//
//////////////////////////////////////////////////////////////////////////
bool CRasterPosed::Pose(CRasterMesh* pcMesh, SRasterBone* asBones, size uiNumBones)
{
	SRasterVertex*		psFrom;
	SRasterVertex*		psTo;
	SRasterSkinVertex*	psSkin;
	SRasterSkinWeight*	psWeight;
	SRasterBone*		psBone;
	SInt64Vec3			sPosition;
	SInt64Vec3			sNormal;
	SInt64Vec3			sSum;
	SInt64Vec3			sNormalSum;
	SInt64Vec3			sAxes[3];
	SInt64Vec3			sLightAxes[3];
	int64				iWeight;
	size				uiNumVertices;
	size				i;
	uint32				j;
	int					k;

	if (!pcMesh->IsSkinned() || (uiNumBones < pcMesh->muiNumMatrices))
	{
		return false;
	}
	for (i = 0; i < pcMesh->masSkinWeights.NumElements(); i++)
	{
		if (pcMesh->masSkinWeights.Get(i)->uiMatrix >= uiNumBones)
		{
			return false;
		}
	}

	uiNumVertices = pcMesh->masVertices.NumElements();
	masVertices.Resize(uiNumVertices);
	psFrom = pcMesh->masVertices.GetData();
	psTo = masVertices.GetData();
	for (i = 0; i < uiNumVertices; i++)
	{
		psSkin = pcMesh->masSkinVertices.Get(i);
		if (psSkin->uiCount == 0)
		{
			psTo[i] = psFrom[i];
			continue;
		}

		sPosition.Init(psFrom[i].sPosition.x, psFrom[i].sPosition.y, psFrom[i].sPosition.z);
		sNormal.Init(psFrom[i].sNormal.x.miRaw, psFrom[i].sNormal.y.miRaw, psFrom[i].sNormal.z.miRaw);
		sSum.Init(0, 0, 0);
		sNormalSum.Init(0, 0, 0);
		for (j = 0; j < psSkin->uiCount; j++)
		{
			psWeight = pcMesh->masSkinWeights.Get(psSkin->uiFirst + j);
			psBone = &asBones[psWeight->uiMatrix];
			iWeight = psWeight->uiWeight;
			for (k = 0; k < 3; k++)
			{
				sAxes[k].Init(psBone->asAxes[k].x, psBone->asAxes[k].y, psBone->asAxes[k].z);
				sLightAxes[k].Init(psBone->asLightAxes[k].x, psBone->asLightAxes[k].y, psBone->asLightAxes[k].z);
			}
			sSum.x += ((Int64Vec3Dot(&sPosition, &sAxes[0]) >> RASTER_AXIS_BITS) + psBone->sOffset.x) * iWeight;
			sSum.y += ((Int64Vec3Dot(&sPosition, &sAxes[1]) >> RASTER_AXIS_BITS) + psBone->sOffset.y) * iWeight;
			sSum.z += ((Int64Vec3Dot(&sPosition, &sAxes[2]) >> RASTER_AXIS_BITS) + psBone->sOffset.z) * iWeight;
			sNormalSum.x += (Int64Vec3Dot(&sNormal, &sLightAxes[0]) >> RASTER_AXIS_BITS) * iWeight;
			sNormalSum.y += (Int64Vec3Dot(&sNormal, &sLightAxes[1]) >> RASTER_AXIS_BITS) * iWeight;
			sNormalSum.z += (Int64Vec3Dot(&sNormal, &sLightAxes[2]) >> RASTER_AXIS_BITS) * iWeight;
		}
		psTo[i].sPosition.Init((int32)((sSum.x + (RASTER_SKIN_ONE >> 1)) >> RASTER_SKIN_WEIGHT_BITS),
							   (int32)((sSum.y + (RASTER_SKIN_ONE >> 1)) >> RASTER_SKIN_WEIGHT_BITS),
							   (int32)((sSum.z + (RASTER_SKIN_ONE >> 1)) >> RASTER_SKIN_WEIGHT_BITS));
		sNormalSum.Init(sNormalSum.x >> RASTER_SKIN_WEIGHT_BITS, sNormalSum.y >> RASTER_SKIN_WEIGHT_BITS, sNormalSum.z >> RASTER_SKIN_WEIGHT_BITS);
		RasterUnitNormal(&psTo[i].sNormal, &sNormalSum);
	}

	msMin.Init(0, 0, 0);
	msMax.Init(0, 0, 0);
	if (uiNumVertices > 0)
	{
		msMin = psTo[0].sPosition;
		msMax = msMin;
		for (i = 1; i < uiNumVertices; i++)
		{
			Int32Vec3Minimize(&msMin, &msMin, &psTo[i].sPosition);
			Int32Vec3Maximize(&msMax, &msMax, &psTo[i].sPosition);
		}
	}
	return true;
}


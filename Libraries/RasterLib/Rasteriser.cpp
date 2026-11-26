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
#ifdef __AVX2__
#include <immintrin.h>
#endif
#include "BaseLib/FixedPoint.h"
#include "BaseLib/Int64Vec3.h"
#include "Rasteriser.h"
#include "RasterDivide.h"


//Twice a triangle's area in subpixels squared, 16 pixels.  Below it each covered pixel is worked out exactly, and there
//is little to do because most triangles here cover a few pixels at most.  From it, gradients step across the triangle.
#define RASTER_SMALL_AREA		16

//From this many pixels across, it's worth working out where each row's covered pixels are rather than looking for them.
//A long thin triangle at an angle, as a board's outline makes, covers next to nothing of a very big box.  4 was the
//fastest when measured on a board: at 2 the working out costs more than it saves.
#define RASTER_WIDE_PIXELS		4

//How many pixels of a row a small triangle's box can have for AVX2 to test them at once: two vectors of eight.
#define RASTER_LANES			16

//How much work is one job when drawing with jobs.  Enough jobs for threads to share out what's slow to do, not so many that
//starting them costs anything.
#define RASTER_JOB_VERTICES		32768
#define RASTER_JOB_TRIANGLES	32768
#define RASTER_JOB_ROWS			16

//Twice a triangle's area, or how far a pixel is inside one of its edges: subpixel times subpixel.
typedef SFixed<int64, int64, 8>		SRasterEdge;

//A depth or shade.
typedef SFixed<int64, int64, 0>		SRasterWideWhole;

//How much a depth or shade changes across a pixel.
typedef SFixed<int64, int64, 8>		SRasterGradient;

//A depth or shade part of the way across a triangle: a gradient times a distance in subpixels.
typedef SFixed<int64, int64, 12>	SRasterInterpolated;

//A distance across a triangle.
typedef SFixed<int64, int64, 4>		SRasterWideSubpixel;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SRasterInstance::Init(CRasterMesh* pcRasterMesh)
{
	int32	iOne;

	iOne = 1 << RASTER_AXIS_BITS;
	pcMesh = pcRasterMesh;
	pcPosed = NULL;
	asAxes[0].Init(iOne, 0, 0);
	asAxes[1].Init(0, iOne, 0);
	asAxes[2].Init(0, 0, iOne);
	sOffset.Init(0, 0, 0);
	OnlyTurns();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SRasterInstance::OnlyTurns(void)
{
	asLightAxes[0] = asAxes[0];
	asLightAxes[1] = asAxes[1];
	asLightAxes[2] = asAxes[2];
	bMirrored = false;
}


//////////////////////////////////////////////////////////////////////////
//
// The vertices drawn, and their bounds before they're turned and moved.
//
//////////////////////////////////////////////////////////////////////////
SRasterVertex* SRasterInstance::GetVertices(void) const { return pcPosed ? pcPosed->masVertices.GetData() : pcMesh->masVertices.GetData(); }
SInt32Vec3* SRasterInstance::GetMin(void) const { return pcPosed ? &pcPosed->msMin : &pcMesh->msMin; }
SInt32Vec3* SRasterInstance::GetMax(void) const { return pcPosed ? &pcPosed->msMax : &pcMesh->msMax; }


//////////////////////////////////////////////////////////////////////////
//
// A shade, no brighter than the most.
//
//////////////////////////////////////////////////////////////////////////
static FORCE_INLINE int32 RasterClampShade(int32 iShade)
{
	return (iShade > RASTER_MAX_SHADE) ? RASTER_MAX_SHADE : iShade;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::Init(void)
{
	masScreen.Init();
	mbCullBackFaces = true;
	mbPerspective = true;
	meFilter = RF_Nearest;
	mpcJobs = NULL;
	masProjectJobs.Init();
	masListJobs.Init();
	masBandJobs.Init();
	masDrawables.Init();
	mcLights.Init();
	muiNumTowards = 0;
	mauiPointLights.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::Kill(void)
{
	mauiPointLights.Kill();
	mcLights.Kill();
	masDrawables.Kill();
	masBandJobs.Kill();
	masListJobs.Kill();
	masProjectJobs.Kill();
	masScreen.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::SetCullBackFaces(bool bCullBackFaces)
{
	mbCullBackFaces = bCullBackFaces;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::SetFilter(ERasterFilter eFilter)
{
	meFilter = eFilter;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::SetJobs(CJobs* pcJobs)
{
	mpcJobs = pcJobs;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CRasterLights* CRasteriser::GetLights(void)
{
	return &mcLights;
}


//////////////////////////////////////////////////////////////////////////
//
// Each light far away towards it in the space the instances are put in, for
// this frame.
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::PrepareLights(CRasterCamera* pcCamera)
{
	SRasterDirectionalLight*	psLight;
	size						i;

	muiNumTowards = mcLights.masDirectional.NumElements();
	for (i = 0; i < muiNumTowards; i++)
	{
		psLight = mcLights.masDirectional.Get(i);
		if (psLight->bOnScreen)
		{
			pcCamera->ViewToMesh(&masTowards[i], &psLight->sTowards);
		}
		else
		{
			masTowards[i] = psLight->sTowards;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The point lights that may reach an instance: those whose reach gets to the
// box its mesh's bounds are in once turned and moved.
//
//////////////////////////////////////////////////////////////////////////
uint64 CRasteriser::FindPointLights(SRasterInstance* psInstance)
{
	SRasterPointLight*	psLight;
	SInt64Vec3			sCorner;
	SInt64Vec3			sMin;
	SInt64Vec3			sMax;
	int64				aiMoved[3];
	int64				aiClosest[3];
	int64				aiLight[3];
	uint64				uiPointLights;
	uint64				uiSquared;
	SInt32Vec3*			psMeshMin;
	SInt32Vec3*			psMeshMax;
	size				uiNumLights;
	size				i;
	int					iCorner;
	int					iAxis;

	uiNumLights = mcLights.masPoint.NumElements();
	if (uiNumLights == 0)
	{
		return 0;
	}

	psMeshMin = psInstance->GetMin();
	psMeshMax = psInstance->GetMax();
	sMin.Init(0x7FFFFFFFFFFFFFFFLL, 0x7FFFFFFFFFFFFFFFLL, 0x7FFFFFFFFFFFFFFFLL);
	sMax.Init(-0x7FFFFFFFFFFFFFFFLL, -0x7FFFFFFFFFFFFFFFLL, -0x7FFFFFFFFFFFFFFFLL);
	for (iCorner = 0; iCorner < 8; iCorner++)
	{
		sCorner.Init((iCorner & 1) ? psMeshMax->x : psMeshMin->x, (iCorner & 2) ? psMeshMax->y : psMeshMin->y, (iCorner & 4) ? psMeshMax->z : psMeshMin->z);
		for (iAxis = 0; iAxis < 3; iAxis++)
		{
			aiMoved[iAxis] = ((sCorner.x * psInstance->asAxes[iAxis].x + sCorner.y * psInstance->asAxes[iAxis].y + sCorner.z * psInstance->asAxes[iAxis].z) >> RASTER_AXIS_BITS) + (&psInstance->sOffset.x)[iAxis];
		}
		sMin.Init((aiMoved[0] < sMin.x) ? aiMoved[0] : sMin.x, (aiMoved[1] < sMin.y) ? aiMoved[1] : sMin.y, (aiMoved[2] < sMin.z) ? aiMoved[2] : sMin.z);
		sMax.Init((aiMoved[0] > sMax.x) ? aiMoved[0] : sMax.x, (aiMoved[1] > sMax.y) ? aiMoved[1] : sMax.y, (aiMoved[2] > sMax.z) ? aiMoved[2] : sMax.z);
	}

	uiPointLights = 0;
	for (i = 0; i < uiNumLights; i++)
	{
		psLight = mcLights.masPoint.Get(i);
		aiLight[0] = psLight->sPosition.x;
		aiLight[1] = psLight->sPosition.y;
		aiLight[2] = psLight->sPosition.z;
		uiSquared = 0;
		for (iAxis = 0; iAxis < 3; iAxis++)
		{
			aiClosest[iAxis] = aiLight[iAxis];
			if (aiClosest[iAxis] < (&sMin.x)[iAxis])
			{
				aiClosest[iAxis] = (&sMin.x)[iAxis];
			}
			else if (aiClosest[iAxis] > (&sMax.x)[iAxis])
			{
				aiClosest[iAxis] = (&sMax.x)[iAxis];
			}
			aiClosest[iAxis] -= aiLight[iAxis];
			if ((aiClosest[iAxis] >= psLight->iReach) || (aiClosest[iAxis] <= -psLight->iReach))
			{
				uiSquared = psLight->uiReachSquared;
				break;
			}
			uiSquared += (uint64)(aiClosest[iAxis] * aiClosest[iAxis]);
		}
		if (uiSquared < psLight->uiReachSquared)
		{
			uiPointLights |= (uint64)1 << i;
		}
	}
	return uiPointLights;
}


//////////////////////////////////////////////////////////////////////////
//
// What the point lights in uiPointLights add at a vertex.  psNormal is its
// normal in the space the instances are put in, as psPosition is.
//
//////////////////////////////////////////////////////////////////////////
int32 CRasteriser::ShadePointLights(uint64 uiPointLights, SInt64Vec3* psNormal, SInt32Vec3* psPosition)
{
	int32	iShade;
	size	i;

	iShade = 0;
	for (i = 0; uiPointLights != 0; i++, uiPointLights >>= 1)
	{
		if (uiPointLights & 1)
		{
			iShade += RasterPointShade(mcLights.masPoint.Get(i), psNormal, psPosition);
		}
	}
	return iShade;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::Draw(CRasterMesh* pcMesh, CRasterCamera* pcCamera, CRasterTarget* pcTarget)
{
	size						i;
	size						j;
	size						uiNumVertices;
	SRasterVertex*				psVertex;
	SRasterScreenVertex*		psScreen;
	SFixed<int32, int32, 28>	sDot;
	SInt64Vec3					sNormal;
	uint64						uiPointLights;
	int32						iShade;

	pcCamera->Prepare(pcTarget->miWidth, pcTarget->miHeight);
	PrepareLights(pcCamera);
	mbPerspective = pcCamera->GetProjection() == RP_Perspective;
	//All the point lights, as the mesh is where it is.
	uiPointLights = (mcLights.masPoint.NumElements() == 0) ? 0 : ((((uint64)1 << (mcLights.masPoint.NumElements() - 1)) << 1) - 1);

	uiNumVertices = pcMesh->masVertices.NumElements();
	GrowScreen(uiNumVertices);

	psVertex = pcMesh->masVertices.GetData();
	psScreen = masScreen.GetData();
	for (i = 0; i < uiNumVertices; i++)
	{
		pcCamera->Project(&psScreen[i], &psVertex[i].sPosition);

		iShade = mcLights.miAmbient;
		for (j = 0; j < muiNumTowards; j++)
		{
			sDot = psVertex[i].sNormal.x * masTowards[j].x + psVertex[i].sNormal.y * masTowards[j].y + psVertex[i].sNormal.z * masTowards[j].z;
			if (sDot.miRaw < 0)
			{
				sDot.miRaw = 0;
			}
			iShade += (int32)(SFixed<int64, int64, 28>::FromRaw(sDot.miRaw) * SRasterWideWhole::FromRaw(mcLights.masDirectional.Get(j)->iAmount)).Floor();
		}
		if (uiPointLights != 0)
		{
			sNormal.Init(psVertex[i].sNormal.x.miRaw, psVertex[i].sNormal.y.miRaw, psVertex[i].sNormal.z.miRaw);
			iShade += ShadePointLights(uiPointLights, &sNormal, &psVertex[i].sPosition);
		}
		psScreen[i].iShade = RasterClampShade(iShade);
	}

	DrawTriangles(pcMesh, psScreen, pcTarget, false, false);
	DrawTriangles(pcMesh, psScreen, pcTarget, true, false);
}


//////////////////////////////////////////////////////////////////////////
//
// The screen holds every instance's vertices, one after another, so that the
// transparent triangles drawn after all the opaque ones use them as they are.
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::Draw(SRasterInstance* asInstances, size uiNumInstances, CRasterCamera* pcCamera, CRasterTarget* pcTarget)
{
	SRasterScreenVertex*	psScreen;
	size					uiNumVertices;
	size					i;

	pcCamera->Prepare(pcTarget->miWidth, pcTarget->miHeight);
	PrepareLights(pcCamera);
	mbPerspective = pcCamera->GetProjection() == RP_Perspective;

	uiNumVertices = 0;
	mauiPointLights.Resize(uiNumInstances);
	for (i = 0; i < uiNumInstances; i++)
	{
		uiNumVertices += asInstances[i].pcMesh->masVertices.NumElements();
		mauiPointLights.SetValue(i, FindPointLights(&asInstances[i]));
	}
	GrowScreen(uiNumVertices);

	if ((mpcJobs != NULL) && (mpcJobs->NumThreads() > 1))
	{
		DrawWithJobs(asInstances, uiNumInstances, pcCamera, pcTarget);
		return;
	}

	psScreen = masScreen.GetData();
	for (i = 0; i < uiNumInstances; i++)
	{
		ProjectInstance(&asInstances[i], pcCamera, mauiPointLights.GetValue(i), psScreen, 0, asInstances[i].pcMesh->masVertices.NumElements());
		DrawTriangles(asInstances[i].pcMesh, psScreen, pcTarget, false, asInstances[i].bMirrored);
		psScreen += asInstances[i].pcMesh->masVertices.NumElements();
	}

	psScreen = masScreen.GetData();
	for (i = 0; i < uiNumInstances; i++)
	{
		DrawTriangles(asInstances[i].pcMesh, psScreen, pcTarget, true, asInstances[i].bMirrored);
		psScreen += asInstances[i].pcMesh->masVertices.NumElements();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A mesh's UVs, or NULL if it hasn't one for each vertex.
//
//////////////////////////////////////////////////////////////////////////
static SInt32Vec2* RasterMeshUVs(CRasterMesh* pcMesh)
{
	if (pcMesh->masUVs.NumElements() != pcMesh->masVertices.NumElements())
	{
		return NULL;
	}
	return pcMesh->masUVs.GetData();
}


//////////////////////////////////////////////////////////////////////////
//
// What a run is textured by: NULL for none, for a texture the mesh hasn't
// got, and for a mesh with no UVs.
//
//////////////////////////////////////////////////////////////////////////
static CRasterTexture* RasterRunTexture(CRasterMesh* pcMesh, SRasterTextureRun* psRun, SInt32Vec2* psUVs)
{
	if (psUVs == NULL)
	{
		return NULL;
	}
	return pcMesh->GetTexture(psRun->iTexture);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::DrawWithJobs(SRasterInstance* asInstances, size uiNumInstances, CRasterCamera* pcCamera, CRasterTarget* pcTarget)
{
	SRasterProjectJob*		psProjectJob;
	SRasterBandJob*			psBandJob;
	SRasterScreenVertex*	psScreen;
	SRasterDrawable*		psDrawables;
	size					uiNumTriangles;
	size					uiNumVertices;
	size					uiFirst;
	size					i;
	int32					iTop;

	//Every array is grown here, as allocating isn't for the jobs to do.
	uiNumTriangles = 0;
	for (i = 0; i < uiNumInstances; i++)
	{
		uiNumTriangles += asInstances[i].pcMesh->NumTriangles();
	}
	if (masDrawables.NumElements() < uiNumTriangles)
	{
		masDrawables.Resize(uiNumTriangles);
	}

	masProjectJobs.Resize(0);
	psScreen = masScreen.GetData();
	for (i = 0; i < uiNumInstances; i++)
	{
		uiNumVertices = asInstances[i].pcMesh->masVertices.NumElements();
		for (uiFirst = 0; uiFirst < uiNumVertices; uiFirst += RASTER_JOB_VERTICES)
		{
			psProjectJob = masProjectJobs.Add();
			psProjectJob->pcRasteriser = this;
			psProjectJob->psInstance = &asInstances[i];
			psProjectJob->pcCamera = pcCamera;
			psProjectJob->uiPointLights = mauiPointLights.GetValue(i);
			psProjectJob->psScreen = psScreen;
			psProjectJob->uiFirst = uiFirst;
			psProjectJob->uiNumVertices = ((uiNumVertices - uiFirst) < RASTER_JOB_VERTICES) ? (uiNumVertices - uiFirst) : RASTER_JOB_VERTICES;
		}
		psScreen += uiNumVertices;
	}
	for (i = 0; i < masProjectJobs.NumElements(); i++)
	{
		mpcJobs->Run(ProjectJob, masProjectJobs.Get(i));
	}
	mpcJobs->Wait();

	//All the opaque triangles and then all the transparent ones, as without jobs.
	masListJobs.Resize(0);
	psDrawables = masDrawables.GetData();
	psScreen = masScreen.GetData();
	for (i = 0; i < uiNumInstances; i++)
	{
		AddListJobs(asInstances[i].pcMesh, psScreen, pcTarget, false, asInstances[i].bMirrored, &psDrawables);
		psScreen += asInstances[i].pcMesh->masVertices.NumElements();
	}
	psScreen = masScreen.GetData();
	for (i = 0; i < uiNumInstances; i++)
	{
		AddListJobs(asInstances[i].pcMesh, psScreen, pcTarget, true, asInstances[i].bMirrored, &psDrawables);
		psScreen += asInstances[i].pcMesh->masVertices.NumElements();
	}
	for (i = 0; i < masListJobs.NumElements(); i++)
	{
		mpcJobs->Run(ListJob, masListJobs.Get(i));
	}
	mpcJobs->Wait();

	masBandJobs.Resize(0);
	for (iTop = 0; iTop < pcTarget->miHeight; iTop += RASTER_JOB_ROWS)
	{
		psBandJob = masBandJobs.Add();
		psBandJob->pcRasteriser = this;
		psBandJob->pcTarget = pcTarget;
		psBandJob->iTop = iTop;
		psBandJob->iBottom = (iTop + RASTER_JOB_ROWS - 1 < pcTarget->miHeight - 1) ? iTop + RASTER_JOB_ROWS - 1 : pcTarget->miHeight - 1;
	}
	for (i = 0; i < masBandJobs.NumElements(); i++)
	{
		mpcJobs->Run(BandJob, masBandJobs.Get(i));
	}
	mpcJobs->Wait();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::AddListJobs(CRasterMesh* pcMesh, SRasterScreenVertex* psScreen, CRasterTarget* pcTarget, bool bTransparent, bool bMirrored, SRasterDrawable** ppsDrawables)
{
	CArrayRasterTriangle*	pasTriangles;
	CArrayRasterTextureRun*	pasRuns;
	SRasterTextureRun*		psRun;
	SInt32Vec2*				psUVs;
	size					i;

	pasTriangles = bTransparent ? &pcMesh->masTransparent : &pcMesh->masOpaque;
	pasRuns = bTransparent ? &pcMesh->masTransparentRuns : &pcMesh->masOpaqueRuns;
	if (pasRuns->NumElements() == 0)
	{
		AddListJobs(pasTriangles->GetData(), pasTriangles->NumElements(), NULL, NULL, psScreen, pcTarget, bTransparent, bMirrored, ppsDrawables);
		return;
	}
	psUVs = RasterMeshUVs(pcMesh);
	for (i = 0; i < pasRuns->NumElements(); i++)
	{
		psRun = pasRuns->Get(i);
		AddListJobs(pasTriangles->GetData() + psRun->uiFirst, psRun->uiNumTriangles, RasterRunTexture(pcMesh, psRun, psUVs), psUVs, psScreen, pcTarget, bTransparent, bMirrored, ppsDrawables);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Triangles that are all textured by pcTexture, or by none.
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::AddListJobs(SRasterTriangle* psTriangles, size uiNumTriangles, CRasterTexture* pcTexture, SInt32Vec2* psUVs, SRasterScreenVertex* psScreen, CRasterTarget* pcTarget, bool bTransparent, bool bMirrored, SRasterDrawable** ppsDrawables)
{
	SRasterListJob*	psJob;
	size			uiFirst;

	for (uiFirst = 0; uiFirst < uiNumTriangles; uiFirst += RASTER_JOB_TRIANGLES)
	{
		psJob = masListJobs.Add();
		psJob->pcRasteriser = this;
		psJob->pcTarget = pcTarget;
		psJob->psTriangles = psTriangles + uiFirst;
		psJob->uiNumTriangles = ((uiNumTriangles - uiFirst) < RASTER_JOB_TRIANGLES) ? (uiNumTriangles - uiFirst) : RASTER_JOB_TRIANGLES;
		psJob->pcTexture = pcTexture;
		psJob->psUVs = psUVs;
		psJob->psScreen = psScreen;
		psJob->psDrawables = *ppsDrawables;
		psJob->uiNumDrawables = 0;
		psJob->iTop = 0x7FFFFFFF;
		psJob->iBottom = -0x7FFFFFFF;
		psJob->bTransparent = bTransparent;
		psJob->bMirrored = bMirrored;
		*ppsDrawables += psJob->uiNumTriangles;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::GrowScreen(size uiNumVertices)
{
	if (masScreen.NumElements() < uiNumVertices)
	{
		masScreen.Resize(uiNumVertices);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an instance's axis is exactly one of the mesh's or its opposite,
// and so nothing to multiply or round, and which.
//
//////////////////////////////////////////////////////////////////////////
static bool RasterQuarterTurn(SInt32Vec3* psAxis, int32* piFrom, int32* piSign)
{
	int32	iOne;

	iOne = 1 << RASTER_AXIS_BITS;
	if (((psAxis->x == iOne) || (psAxis->x == -iOne)) && (psAxis->y == 0) && (psAxis->z == 0))
	{
		*piFrom = 0;
		*piSign = (psAxis->x > 0) ? 1 : -1;
		return true;
	}
	if ((psAxis->x == 0) && ((psAxis->y == iOne) || (psAxis->y == -iOne)) && (psAxis->z == 0))
	{
		*piFrom = 1;
		*piSign = (psAxis->y > 0) ? 1 : -1;
		return true;
	}
	if ((psAxis->x == 0) && (psAxis->y == 0) && ((psAxis->z == iOne) || (psAxis->z == -iOne)))
	{
		*piFrom = 2;
		*piSign = (psAxis->z > 0) ? 1 : -1;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Some of an instance's vertices on the screen, from psScreen, which is where
// its first vertex goes.  Turning the lights far away back into the mesh's
// space by the light axes, once, is the same as turning every normal out of
// it.  A board's vertices often have the normal of the one before, and so its
// shade from them.  A point light is worked out at each vertex, from the
// normal turned out by the light axes.
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::ProjectInstance(SRasterInstance* psInstance, CRasterCamera* pcCamera, uint64 uiPointLights, SRasterScreenVertex* psScreen, size uiFirst, size uiNumVertices)
{
	size					i;
	size					j;
	SRasterVertex*			psVertex;
	SInt64Vec3				sAxes[3];
	SInt64Vec3				sLightAxes[3];
	SInt64Vec3				sPosition;
	SInt64Vec3				sNormal;
	SInt64Vec3				sTurnedNormal;
	SInt64Vec3				asTowards[RASTER_MAX_DIRECTIONAL_LIGHTS];
	SInt32Vec3				sMoved;
	int64					iDot;
	int32					aiFrom[3];
	int32					aiSign[3];
	int32*					piPosition;
	int32					iShade;
	int32					iFarShade;
	int32					iLastX;
	int32					iLastY;
	int32					iLastZ;
	bool					bQuarterTurns;

	bQuarterTurns = RasterQuarterTurn(&psInstance->asAxes[0], &aiFrom[0], &aiSign[0]) &&
					RasterQuarterTurn(&psInstance->asAxes[1], &aiFrom[1], &aiSign[1]) &&
					RasterQuarterTurn(&psInstance->asAxes[2], &aiFrom[2], &aiSign[2]);

	//No normal's parts are this.
	iLastX = 0x7FFFFFFF;
	iLastY = 0;
	iLastZ = 0;
	iShade = 0;
	iFarShade = 0;

	for (i = 0; i < 3; i++)
	{
		sAxes[i].Init(psInstance->asAxes[i].x, psInstance->asAxes[i].y, psInstance->asAxes[i].z);
		sLightAxes[i].Init(psInstance->asLightAxes[i].x, psInstance->asLightAxes[i].y, psInstance->asLightAxes[i].z);
	}
	for (j = 0; j < muiNumTowards; j++)
	{
		asTowards[j].Init((sLightAxes[0].x * masTowards[j].x.miRaw + sLightAxes[1].x * masTowards[j].y.miRaw + sLightAxes[2].x * masTowards[j].z.miRaw) >> RASTER_AXIS_BITS,
						  (sLightAxes[0].y * masTowards[j].x.miRaw + sLightAxes[1].y * masTowards[j].y.miRaw + sLightAxes[2].y * masTowards[j].z.miRaw) >> RASTER_AXIS_BITS,
						  (sLightAxes[0].z * masTowards[j].x.miRaw + sLightAxes[1].z * masTowards[j].y.miRaw + sLightAxes[2].z * masTowards[j].z.miRaw) >> RASTER_AXIS_BITS);
	}

	psVertex = psInstance->GetVertices();
	for (i = uiFirst; i < uiFirst + uiNumVertices; i++)
	{
		if (bQuarterTurns)
		{
			piPosition = &psVertex[i].sPosition.x;
			sMoved.Init(aiSign[0] * piPosition[aiFrom[0]] + psInstance->sOffset.x,
						aiSign[1] * piPosition[aiFrom[1]] + psInstance->sOffset.y,
						aiSign[2] * piPosition[aiFrom[2]] + psInstance->sOffset.z);
		}
		else
		{
			sPosition.Init(psVertex[i].sPosition.x, psVertex[i].sPosition.y, psVertex[i].sPosition.z);
			sMoved.Init((int32)(Int64Vec3Dot(&sPosition, &sAxes[0]) >> RASTER_AXIS_BITS) + psInstance->sOffset.x,
						(int32)(Int64Vec3Dot(&sPosition, &sAxes[1]) >> RASTER_AXIS_BITS) + psInstance->sOffset.y,
						(int32)(Int64Vec3Dot(&sPosition, &sAxes[2]) >> RASTER_AXIS_BITS) + psInstance->sOffset.z);
		}
		pcCamera->Project(&psScreen[i], &sMoved);

		if ((psVertex[i].sNormal.x.miRaw != iLastX) || (psVertex[i].sNormal.y.miRaw != iLastY) || (psVertex[i].sNormal.z.miRaw != iLastZ))
		{
			iLastX = psVertex[i].sNormal.x.miRaw;
			iLastY = psVertex[i].sNormal.y.miRaw;
			iLastZ = psVertex[i].sNormal.z.miRaw;
			sNormal.Init(iLastX, iLastY, iLastZ);
			iFarShade = mcLights.miAmbient;
			for (j = 0; j < muiNumTowards; j++)
			{
				iDot = Int64Vec3Dot(&sNormal, &asTowards[j]);
				if (iDot < 0)
				{
					iDot = 0;
				}
				iFarShade += (int32)((iDot * mcLights.masDirectional.Get(j)->iAmount) >> (SRasterUnit::Bits * 2));
			}
			iShade = RasterClampShade(iFarShade);
		}
		if (uiPointLights != 0)
		{
			sTurnedNormal.Init(Int64Vec3Dot(&sNormal, &sLightAxes[0]) >> RASTER_AXIS_BITS, Int64Vec3Dot(&sNormal, &sLightAxes[1]) >> RASTER_AXIS_BITS, Int64Vec3Dot(&sNormal, &sLightAxes[2]) >> RASTER_AXIS_BITS);
			psScreen[i].iShade = RasterClampShade(iFarShade + ShadePointLights(uiPointLights, &sTurnedNormal, &sMoved));
		}
		else
		{
			psScreen[i].iShade = iShade;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static SRasterSubpixel Min3(SRasterSubpixel a, SRasterSubpixel b, SRasterSubpixel c)
{
	SRasterSubpixel	s;

	s = (a < b) ? a : b;
	return (s < c) ? s : c;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static SRasterSubpixel Max3(SRasterSubpixel a, SRasterSubpixel b, SRasterSubpixel c)
{
	SRasterSubpixel	s;

	s = (a > b) ? a : b;
	return (s > c) ? s : c;
}


//////////////////////////////////////////////////////////////////////////
//
// Positive inside a triangle with a positive area.  Edges that are a top or a left edge own the pixels exactly on them;
// the rest are moved inwards by the smallest amount.
//
//////////////////////////////////////////////////////////////////////////
static SRasterEdge RasterEdgeBias(SRasterSubpixelVec2* psFrom, SRasterSubpixelVec2* psTo)
{
	SRasterSubpixel	sDeltaX;
	SRasterSubpixel	sDeltaY;

	sDeltaX = psTo->x - psFrom->x;
	sDeltaY = psTo->y - psFrom->y;
	return SRasterEdge::FromRaw(((sDeltaY.miRaw < 0) || ((sDeltaY.miRaw == 0) && (sDeltaX.miRaw > 0))) ? 0 : -1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static SRasterEdge RasterEdge(SRasterSubpixelVec2* psFrom, SRasterSubpixelVec2* psTo, SRasterSubpixel sX, SRasterSubpixel sY)
{
	return (psTo->x - psFrom->x) * (sY - psFrom->y) - (psTo->y - psFrom->y) * (sX - psFrom->x);
}


//////////////////////////////////////////////////////////////////////////
//
// The weights are how far a pixel is inside each edge, so a value is the weighted sum over the area.  Depths and shades
// on one triangle share one format.
//
//////////////////////////////////////////////////////////////////////////
static FORCE_INLINE int32 RasterWeigh(SRasterEdge s0, SRasterEdge s1, SRasterEdge s2, SRasterWideWhole sA, SRasterWideWhole sB, SRasterWideWhole sC, SRasterEdge sArea, uint64 uiInverse)
{
	return (int32)RasterDivideFloor((s0 * sA + s1 * sB + s2 * sC).miRaw, (uint64)sArea.miRaw, uiInverse);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static SRasterGradient RasterGradient(SRasterWideWhole sDeltaB, SRasterWideWhole sDeltaC, SRasterSubpixel sDistanceB, SRasterSubpixel sDistanceC, SRasterEdge sArea)
{
	return (sDeltaB * sDistanceC.To<SRasterWideSubpixel>() - sDeltaC * sDistanceB.To<SRasterWideSubpixel>()).To<SFixed<int64, int64, 16> >().DivideFloor(sArea);
}


//////////////////////////////////////////////////////////////////////////
//
// A pixel of colour uiColour times iShade.  An opaque one writes its depth.
//
//////////////////////////////////////////////////////////////////////////
static FORCE_INLINE void RasterWritePixel(uint32* puiPixel, int32* piDepth, int32 iDepth, int32 iShade, uint32 uiColour, bool bTransparent)
{
	uint32	uiRB;
	uint32	uiG;
	uint32	uiAlpha;
	uint32	uiPixel;

	uiRB = (((uiColour & 0x00FF00FF) * (uint32)iShade) >> 8) & 0x00FF00FF;
	uiG = (((uiColour & 0x0000FF00) * (uint32)iShade) >> 8) & 0x0000FF00;
	if (bTransparent)
	{
		uiAlpha = uiColour >> 24;
		uiAlpha += uiAlpha >> 7;
		uiPixel = *puiPixel;
		uiRB = ((uiRB * uiAlpha + (uiPixel & 0x00FF00FF) * (256 - uiAlpha)) >> 8) & 0x00FF00FF;
		uiG = ((uiG * uiAlpha + (uiPixel & 0x0000FF00) * (256 - uiAlpha)) >> 8) & 0x0000FF00;
	}
	else
	{
		*piDepth = iDepth;
	}
	*puiPixel = 0xFF000000 | uiRB | uiG;
}


#ifdef __AVX2__
//How far along a row each pixel's edge values are, for each edge, and which pixels are in the box.  Most of a
//board's triangles cover a few pixels at most of a small box, so nearly every test of a pixel on its own is a branch that
//can't be predicted.  Testing a row at once and then going to the hits has none.
struct SRasterLanes
{
	__m256i	asAlong[3];
	__m256i	asAlongNext[3];  //For the second eight pixels.
	bool	bNext;
	uint32	uiInBox;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void RasterLanesInit(SRasterLanes* psLanes, SRasterEdge sStepX0, SRasterEdge sStepX1, SRasterEdge sStepX2, int32 iPixels)
{
	__m256i	sIndex;

	sIndex = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
	psLanes->asAlong[0] = _mm256_mullo_epi32(sIndex, _mm256_set1_epi32((int32)sStepX0.miRaw));
	psLanes->asAlong[1] = _mm256_mullo_epi32(sIndex, _mm256_set1_epi32((int32)sStepX1.miRaw));
	psLanes->asAlong[2] = _mm256_mullo_epi32(sIndex, _mm256_set1_epi32((int32)sStepX2.miRaw));
	psLanes->asAlongNext[0] = _mm256_add_epi32(psLanes->asAlong[0], _mm256_set1_epi32((int32)sStepX0.miRaw * 8));
	psLanes->asAlongNext[1] = _mm256_add_epi32(psLanes->asAlong[1], _mm256_set1_epi32((int32)sStepX1.miRaw * 8));
	psLanes->asAlongNext[2] = _mm256_add_epi32(psLanes->asAlong[2], _mm256_set1_epi32((int32)sStepX2.miRaw * 8));
	psLanes->bNext = iPixels > 8;
	psLanes->uiInBox = (1u << iPixels) - 1u;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an edge's value stays in the 32 bits a lane has, everywhere in a
// box of iRows rows and RASTER_LANES pixels.
//
//////////////////////////////////////////////////////////////////////////
static bool RasterLanesFit(SRasterEdge sRow, SRasterEdge sStepX, SRasterEdge sStepY, int32 iRows)
{
	int64	iBound;

	iBound = (sRow.miRaw < 0) ? -sRow.miRaw : sRow.miRaw;
	iBound += ((sStepX.miRaw < 0) ? -sStepX.miRaw : sStepX.miRaw) * RASTER_LANES;
	iBound += ((sStepY.miRaw < 0) ? -sStepY.miRaw : sStepY.miRaw) * iRows;
	return iBound <= 0x7FFFFFFF;
}


//////////////////////////////////////////////////////////////////////////
//
// A bit for each pixel of the row in the box that's inside all three edges.
//
//////////////////////////////////////////////////////////////////////////
static FORCE_INLINE uint32 RasterLaneHits(SRasterLanes* psLanes, SRasterEdge s0, SRasterEdge s1, SRasterEdge s2)
{
	__m256i	sRow0;
	__m256i	sRow1;
	__m256i	sRow2;
	__m256i	sAny;
	uint32	uiOutside;

	sRow0 = _mm256_set1_epi32((int32)s0.miRaw);
	sRow1 = _mm256_set1_epi32((int32)s1.miRaw);
	sRow2 = _mm256_set1_epi32((int32)s2.miRaw);
	sAny = _mm256_or_si256(_mm256_add_epi32(sRow0, psLanes->asAlong[0]), _mm256_add_epi32(sRow1, psLanes->asAlong[1]));
	sAny = _mm256_or_si256(sAny, _mm256_add_epi32(sRow2, psLanes->asAlong[2]));
	uiOutside = (uint32)_mm256_movemask_ps(_mm256_castsi256_ps(sAny));
	if (psLanes->bNext)
	{
		sAny = _mm256_or_si256(_mm256_add_epi32(sRow0, psLanes->asAlongNext[0]), _mm256_add_epi32(sRow1, psLanes->asAlongNext[1]));
		sAny = _mm256_or_si256(sAny, _mm256_add_epi32(sRow2, psLanes->asAlongNext[2]));
		uiOutside |= (uint32)_mm256_movemask_ps(_mm256_castsi256_ps(sAny)) << 8;
	}
	return ~uiOutside & psLanes->uiInBox;
}
#endif


//////////////////////////////////////////////////////////////////////////
//
// Narrows a row's pixels to those an edge lets in: where its value, sValue
// at pixel iX and changing by sStep each pixel, isn't negative.
//
//////////////////////////////////////////////////////////////////////////
static FORCE_INLINE void RasterEdgeSpan(SRasterEdge sValue, SRasterEdge sStep, uint64 uiInverse, int32 iX, int32* piFirstX, int32* piLastX)
{
	int64	iValue;
	int64	iStep;
	int64	iPixels;

	iValue = sValue.miRaw;
	iStep = sStep.miRaw;
	if (iStep > 0)
	{
		if (iValue < 0)
		{
			iPixels = RasterDivideFloor(-iValue + iStep - 1, (uint64)iStep, uiInverse);
			if (iX + iPixels > *piFirstX)
			{
				*piFirstX = (iX + iPixels > *piLastX) ? *piLastX + 1 : (int32)(iX + iPixels);
			}
		}
	}
	else if (iValue < 0)
	{
		*piLastX = iX - 1;  //It only falls from here, or stays.
	}
	else if (iStep < 0)
	{
		iPixels = RasterDivideFloor(iValue, (uint64)-iStep, uiInverse);
		if (iX + iPixels < *piLastX)
		{
			*piLastX = (int32)(iX + iPixels);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// False for a triangle DrawTriangle would return from without looking at a
// pixel, which is most of them, for less than it costs to call DrawTriangle.
//
//////////////////////////////////////////////////////////////////////////
static FORCE_INLINE bool RasterMayDraw(CRasterTarget* pcTarget, SRasterScreenVertex* psA, SRasterScreenVertex* psB, SRasterScreenVertex* psC, bool bCullBackFaces, int32* piTop, int32* piBottom)
{
	SRasterEdge	sArea;
	int32		iMinX;
	int32		iMaxX;
	int32		iMinY;
	int32		iMaxY;

	if ((psA->iDepth == 0) || (psB->iDepth == 0) || (psC->iDepth == 0))
	{
		return false;
	}
	sArea = RasterEdge(&psA->sPosition, &psB->sPosition, psC->sPosition.x, psC->sPosition.y);
	if ((sArea.miRaw == 0) || ((sArea.miRaw > 0) && bCullBackFaces))
	{
		return false;
	}
	iMinX = (Min3(psA->sPosition.x, psB->sPosition.x, psC->sPosition.x) + SRasterSubpixel::FromRaw(7)).Floor();
	iMinY = (Min3(psA->sPosition.y, psB->sPosition.y, psC->sPosition.y) + SRasterSubpixel::FromRaw(7)).Floor();
	iMaxX = (Max3(psA->sPosition.x, psB->sPosition.x, psC->sPosition.x) - SRasterSubpixel::FromRaw(8)).Floor();
	iMaxY = (Max3(psA->sPosition.y, psB->sPosition.y, psC->sPosition.y) - SRasterSubpixel::FromRaw(8)).Floor();
	*piTop = iMinY;
	*piBottom = iMaxY;
	return (iMinX <= iMaxX) && (iMinY <= iMaxY) && (iMinX < pcTarget->miWidth) && (iMinY < pcTarget->miHeight) && (iMaxX >= 0) && (iMaxY >= 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The pixels whose samples may be in a triangle, on the target and on rows
// iTop to iBottom.  False if there are none.
//
//////////////////////////////////////////////////////////////////////////
static FORCE_INLINE bool RasterTriangleBox(CRasterTarget* pcTarget, SRasterScreenVertex* psA, SRasterScreenVertex* psB, SRasterScreenVertex* psC, int32 iTop, int32 iBottom, int32* piMinX, int32* piMinY, int32* piMaxX, int32* piMaxY)
{
	int32	iMinX;
	int32	iMinY;
	int32	iMaxX;
	int32	iMaxY;

	iMinX = (Min3(psA->sPosition.x, psB->sPosition.x, psC->sPosition.x) + SRasterSubpixel::FromRaw(7)).Floor();
	iMinY = (Min3(psA->sPosition.y, psB->sPosition.y, psC->sPosition.y) + SRasterSubpixel::FromRaw(7)).Floor();
	iMaxX = (Max3(psA->sPosition.x, psB->sPosition.x, psC->sPosition.x) - SRasterSubpixel::FromRaw(8)).Floor();
	iMaxY = (Max3(psA->sPosition.y, psB->sPosition.y, psC->sPosition.y) - SRasterSubpixel::FromRaw(8)).Floor();
	if (iMinX < 0)
	{
		iMinX = 0;
	}
	if (iMinY < 0)
	{
		iMinY = 0;
	}
	if (iMaxX >= pcTarget->miWidth)
	{
		iMaxX = pcTarget->miWidth - 1;
	}
	if (iMaxY >= pcTarget->miHeight)
	{
		iMaxY = pcTarget->miHeight - 1;
	}
	if (iMinY < iTop)
	{
		iMinY = iTop;
	}
	if (iMaxY > iBottom)
	{
		iMaxY = iBottom;
	}
	*piMinX = iMinX;
	*piMinY = iMinY;
	*piMaxX = iMaxX;
	*piMaxY = iMaxY;
	return (iMinX <= iMaxX) && (iMinY <= iMaxY);
}


//////////////////////////////////////////////////////////////////////////
//
// How many bits a number needs: 0 for 0.
//
//////////////////////////////////////////////////////////////////////////
static int32 RasterBits(uint64 uiValue)
{
	int32	iBits;

	iBits = 0;
	while (uiValue != 0)
	{
		iBits++;
		uiValue >>= 1;
	}
	return iBits;
}


//////////////////////////////////////////////////////////////////////////
//
// Each of a colour's channels times a texel's, rounded, as the Vulkan
// renderer multiplies them.
//
//////////////////////////////////////////////////////////////////////////
static FORCE_INLINE uint32 RasterModulate(uint32 uiColour, uint32 uiTexel)
{
	uint32	uiResult;
	uint32	uiProduct;
	int		iShift;

	if (uiColour == 0xFFFFFFFF)
	{
		return uiTexel;
	}
	uiResult = 0;
	for (iShift = 0; iShift < 32; iShift += 8)
	{
		uiProduct = ((uiColour >> iShift) & 0xFF) * ((uiTexel >> iShift) & 0xFF) + 128;
		uiResult |= ((uiProduct + (uiProduct >> 8)) >> 8) << iShift;
	}
	return uiResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A mesh's opaque or transparent triangles, a texture's run at a time.
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::DrawTriangles(CRasterMesh* pcMesh, SRasterScreenVertex* psScreen, CRasterTarget* pcTarget, bool bTransparent, bool bMirrored)
{
	CArrayRasterTriangle*	pasTriangles;
	CArrayRasterTextureRun*	pasRuns;
	SRasterTextureRun*		psRun;
	SInt32Vec2*				psUVs;
	size					i;

	pasTriangles = bTransparent ? &pcMesh->masTransparent : &pcMesh->masOpaque;
	pasRuns = bTransparent ? &pcMesh->masTransparentRuns : &pcMesh->masOpaqueRuns;
	if (pasRuns->NumElements() == 0)
	{
		DrawTriangles(pasTriangles->GetData(), pasTriangles->NumElements(), NULL, NULL, psScreen, pcTarget, bTransparent, bMirrored);
		return;
	}
	psUVs = RasterMeshUVs(pcMesh);
	for (i = 0; i < pasRuns->NumElements(); i++)
	{
		psRun = pasRuns->Get(i);
		DrawTriangles(pasTriangles->GetData() + psRun->uiFirst, psRun->uiNumTriangles, RasterRunTexture(pcMesh, psRun, psUVs), psUVs, psScreen, pcTarget, bTransparent, bMirrored);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Triangles that are all textured by pcTexture, or by none.
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::DrawTriangles(SRasterTriangle* psTriangles, size uiNumTriangles, CRasterTexture* pcTexture, SInt32Vec2* psUVs, SRasterScreenVertex* psScreen, CRasterTarget* pcTarget, bool bTransparent, bool bMirrored)
{
	size				i;
	SRasterTriangle*	psTriangle;
	int32				iTop;
	int32				iBottom;
	int32				iSecond;
	int32				iThird;

	iSecond = bMirrored ? 2 : 1;
	iThird = bMirrored ? 1 : 2;
	psTriangle = psTriangles;
	if (pcTexture == NULL)
	{
		for (i = 0; i < uiNumTriangles; i++)
		{
			if (RasterMayDraw(pcTarget, &psScreen[psTriangle[i].aiVertex[0]], &psScreen[psTriangle[i].aiVertex[iSecond]], &psScreen[psTriangle[i].aiVertex[iThird]], mbCullBackFaces, &iTop, &iBottom))
			{
				DrawTriangleRows(pcTarget, &psScreen[psTriangle[i].aiVertex[0]], &psScreen[psTriangle[i].aiVertex[iSecond]], &psScreen[psTriangle[i].aiVertex[iThird]], psTriangle[i].uiColour, bTransparent, 0, pcTarget->miHeight - 1);
			}
		}
		return;
	}

	for (i = 0; i < uiNumTriangles; i++)
	{
		if (RasterMayDraw(pcTarget, &psScreen[psTriangle[i].aiVertex[0]], &psScreen[psTriangle[i].aiVertex[iSecond]], &psScreen[psTriangle[i].aiVertex[iThird]], mbCullBackFaces, &iTop, &iBottom))
		{
			DrawTexturedTriangleRows(pcTarget, &psScreen[psTriangle[i].aiVertex[0]], &psScreen[psTriangle[i].aiVertex[iSecond]], &psScreen[psTriangle[i].aiVertex[iThird]],
									 &psUVs[psTriangle[i].aiVertex[0]], &psUVs[psTriangle[i].aiVertex[iSecond]], &psUVs[psTriangle[i].aiVertex[iThird]],
									 psTriangle[i].uiColour, pcTexture, bTransparent, 0, pcTarget->miHeight - 1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::ProjectJob(void* pvJob)
{
	SRasterProjectJob*	psJob;

	psJob = (SRasterProjectJob*)pvJob;
	psJob->pcRasteriser->ProjectInstance(psJob->psInstance, psJob->pcCamera, psJob->uiPointLights, psJob->psScreen, psJob->uiFirst, psJob->uiNumVertices);
}


//////////////////////////////////////////////////////////////////////////
//
// Writes only to its own drawables.
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::ListJob(void* pvJob)
{
	SRasterListJob*			psJob;
	SRasterTriangle*		psTriangle;
	SRasterScreenVertex*	psScreen;
	SRasterDrawable*		psDrawable;
	size					i;
	int32					iTop;
	int32					iBottom;
	int32					iSecond;
	int32					iThird;
	bool					bCullBackFaces;

	psJob = (SRasterListJob*)pvJob;
	psScreen = psJob->psScreen;
	bCullBackFaces = psJob->pcRasteriser->mbCullBackFaces;
	iSecond = psJob->bMirrored ? 2 : 1;
	iThird = psJob->bMirrored ? 1 : 2;
	for (i = 0; i < psJob->uiNumTriangles; i++)
	{
		psTriangle = &psJob->psTriangles[i];
		if (RasterMayDraw(psJob->pcTarget, &psScreen[psTriangle->aiVertex[0]], &psScreen[psTriangle->aiVertex[iSecond]], &psScreen[psTriangle->aiVertex[iThird]], bCullBackFaces, &iTop, &iBottom))
		{
			psDrawable = &psJob->psDrawables[psJob->uiNumDrawables];
			psJob->uiNumDrawables++;
			psDrawable->psTriangle = psTriangle;
			psDrawable->iTop = iTop;
			psDrawable->iBottom = iBottom;
			psJob->iTop = (iTop < psJob->iTop) ? iTop : psJob->iTop;
			psJob->iBottom = (iBottom > psJob->iBottom) ? iBottom : psJob->iBottom;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Writes only to its own rows of the target.
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::BandJob(void* pvJob)
{
	SRasterBandJob*			psJob;
	CRasteriser*			pcRasteriser;
	SRasterListJob*			psList;
	SRasterDrawable*		psDrawable;
	SRasterTriangle*		psTriangle;
	SRasterScreenVertex*	psScreen;
	size					uiNumLists;
	size					uiList;
	size					i;
	int32					iSecond;
	int32					iThird;

	psJob = (SRasterBandJob*)pvJob;
	pcRasteriser = psJob->pcRasteriser;
	uiNumLists = pcRasteriser->masListJobs.NumElements();
	for (uiList = 0; uiList < uiNumLists; uiList++)
	{
		psList = pcRasteriser->masListJobs.Get(uiList);
		if ((psList->iBottom >= psJob->iTop) && (psList->iTop <= psJob->iBottom))
		{
			psScreen = psList->psScreen;
			iSecond = psList->bMirrored ? 2 : 1;
			iThird = psList->bMirrored ? 1 : 2;
			for (i = 0; i < psList->uiNumDrawables; i++)
			{
				psDrawable = &psList->psDrawables[i];
				if ((psDrawable->iBottom >= psJob->iTop) && (psDrawable->iTop <= psJob->iBottom))
				{
					psTriangle = psDrawable->psTriangle;
					if (psList->pcTexture == NULL)
					{
						pcRasteriser->DrawTriangleRows(psJob->pcTarget, &psScreen[psTriangle->aiVertex[0]], &psScreen[psTriangle->aiVertex[iSecond]], &psScreen[psTriangle->aiVertex[iThird]], psTriangle->uiColour, psList->bTransparent, psJob->iTop, psJob->iBottom);
					}
					else
					{
						pcRasteriser->DrawTexturedTriangleRows(psJob->pcTarget, &psScreen[psTriangle->aiVertex[0]], &psScreen[psTriangle->aiVertex[iSecond]], &psScreen[psTriangle->aiVertex[iThird]], &psList->psUVs[psTriangle->aiVertex[0]], &psList->psUVs[psTriangle->aiVertex[iSecond]], &psList->psUVs[psTriangle->aiVertex[iThird]], psTriangle->uiColour, psList->pcTexture, psList->bTransparent, psJob->iTop, psJob->iBottom);
					}
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::DrawTriangle(CRasterTarget* pcTarget, SRasterScreenVertex* psA, SRasterScreenVertex* psB, SRasterScreenVertex* psC, uint32 uiColour, bool bTransparent)
{
	DrawTriangleRows(pcTarget, psA, psB, psC, uiColour, bTransparent, 0, pcTarget->miHeight - 1);
}


//////////////////////////////////////////////////////////////////////////
//
// As much of it as is on rows iTop to iBottom.  What's drawn on a row is the
// same whichever others are.
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::DrawTriangleRows(CRasterTarget* pcTarget, SRasterScreenVertex* psA, SRasterScreenVertex* psB, SRasterScreenVertex* psC, uint32 uiColour, bool bTransparent, int32 iTop, int32 iBottom)
{
	SRasterScreenVertex*	psSwap;
	SRasterEdge				sArea;
	int32					iMinX;
	int32					iMaxX;
	int32					iMinY;
	int32					iMaxY;
	int32					x;
	int32					y;
	SRasterSubpixel			sSampleX;
	SRasterSubpixel			sSampleY;
	SRasterSubpixel			sOnePixel;
	SRasterEdge				sBias0;
	SRasterEdge				sBias1;
	SRasterEdge				sBias2;
	SRasterEdge				sRow0;
	SRasterEdge				sRow1;
	SRasterEdge				sRow2;
	SRasterEdge				s0;
	SRasterEdge				s1;
	SRasterEdge				s2;
	SRasterEdge				sStepX0;
	SRasterEdge				sStepX1;
	SRasterEdge				sStepX2;
	SRasterEdge				sStepY0;
	SRasterEdge				sStepY1;
	SRasterEdge				sStepY2;
	SRasterWideWhole		sDepthA;
	SRasterWideWhole		sDepthB;
	SRasterWideWhole		sDepthC;
	SRasterWideWhole		sShadeA;
	SRasterWideWhole		sShadeB;
	SRasterWideWhole		sShadeC;
	int32					iMinDepth;
	int32					iMaxDepth;
	SRasterGradient			sDepthX;
	SRasterGradient			sDepthY;
	SRasterGradient			sShadeX;
	SRasterGradient			sShadeY;
	SRasterInterpolated		sDepth;
	SRasterInterpolated		sShade;
	SRasterInterpolated		sStepDepth;
	SRasterInterpolated		sStepShade;
	SRasterWideSubpixel		sOffsetX;
	SRasterWideSubpixel		sOffsetY;
	SRasterSubpixel			sDeltaBX;
	SRasterSubpixel			sDeltaBY;
	SRasterSubpixel			sDeltaCX;
	SRasterSubpixel			sDeltaCY;
	int32					iNextX;
	int32					iFirstX;
	int32					iLastX;
	bool					bWide;
	bool					bSmall;
	int32					iDepth;
	int32					iShade;
	uint32*					puiPixelRow;
	int32*					piDepthRow;
	uint64					uiAreaInverse;  //Made when a triangle first covers a pixel, and most don't cover any.
	uint64					auiStepInverse[3];
#ifdef __AVX2__
	SRasterLanes			sLanes;
	bool					bLanes;
	uint32					uiHits;
	int32					iHit;
	SRasterEdge				sHit0;
	SRasterEdge				sHit1;
	SRasterEdge				sHit2;
#endif

	if ((psA->iDepth == 0) || (psB->iDepth == 0) || (psC->iDepth == 0))
	{
		return;
	}

	sArea = RasterEdge(&psA->sPosition, &psB->sPosition, psC->sPosition.x, psC->sPosition.y);
	if (sArea.miRaw == 0)
	{
		return;
	}

	//The mesh's front faces wind anticlockwise looking at them with y up.  With y down their area is negative.
	if (sArea.miRaw < 0)
	{
		psSwap = psB;
		psB = psC;
		psC = psSwap;
		sArea = -sArea;
	}
	else if (mbCullBackFaces)
	{
		return;
	}

	//A pixel's sample is at its centre.
	sOnePixel = SRasterSubpixel::FromInt(1);
	if (!RasterTriangleBox(pcTarget, psA, psB, psC, iTop, iBottom, &iMinX, &iMinY, &iMaxX, &iMaxY))
	{
		return;
	}

	sSampleX = SRasterSubpixel::FromInt(iMinX) + SRasterSubpixel::FromRaw(8);
	sSampleY = SRasterSubpixel::FromInt(iMinY) + SRasterSubpixel::FromRaw(8);

	sBias0 = RasterEdgeBias(&psB->sPosition, &psC->sPosition);
	sBias1 = RasterEdgeBias(&psC->sPosition, &psA->sPosition);
	sBias2 = RasterEdgeBias(&psA->sPosition, &psB->sPosition);

	sRow0 = RasterEdge(&psB->sPosition, &psC->sPosition, sSampleX, sSampleY) + sBias0;
	sRow1 = RasterEdge(&psC->sPosition, &psA->sPosition, sSampleX, sSampleY) + sBias1;
	sRow2 = RasterEdge(&psA->sPosition, &psB->sPosition, sSampleX, sSampleY) + sBias2;

	sStepX0 = (psB->sPosition.y - psC->sPosition.y) * sOnePixel;
	sStepX1 = (psC->sPosition.y - psA->sPosition.y) * sOnePixel;
	sStepX2 = (psA->sPosition.y - psB->sPosition.y) * sOnePixel;
	sStepY0 = (psC->sPosition.x - psB->sPosition.x) * sOnePixel;
	sStepY1 = (psA->sPosition.x - psC->sPosition.x) * sOnePixel;
	sStepY2 = (psB->sPosition.x - psA->sPosition.x) * sOnePixel;

	sDepthA = SRasterWideWhole::FromRaw(psA->iDepth);
	sDepthB = SRasterWideWhole::FromRaw(psB->iDepth);
	sDepthC = SRasterWideWhole::FromRaw(psC->iDepth);
	sShadeA = SRasterWideWhole::FromRaw(psA->iShade);
	sShadeB = SRasterWideWhole::FromRaw(psB->iShade);
	sShadeC = SRasterWideWhole::FromRaw(psC->iShade);

	bSmall = sArea < SRasterEdge::FromInt(RASTER_SMALL_AREA);
	sDepthX = SRasterGradient::FromRaw(0);
	sDepthY = SRasterGradient::FromRaw(0);
	sShadeX = SRasterGradient::FromRaw(0);
	sShadeY = SRasterGradient::FromRaw(0);
	iMinDepth = 0;
	iMaxDepth = 0;
	if (!bSmall)
	{
		sDeltaBX = psB->sPosition.x - psA->sPosition.x;
		sDeltaBY = psB->sPosition.y - psA->sPosition.y;
		sDeltaCX = psC->sPosition.x - psA->sPosition.x;
		sDeltaCY = psC->sPosition.y - psA->sPosition.y;
		sDepthX = RasterGradient(sDepthB - sDepthA, sDepthC - sDepthA, sDeltaBY, sDeltaCY, sArea);
		sDepthY = RasterGradient(sDepthC - sDepthA, sDepthB - sDepthA, sDeltaCX, sDeltaBX, sArea);
		sShadeX = RasterGradient(sShadeB - sShadeA, sShadeC - sShadeA, sDeltaBY, sDeltaCY, sArea);
		sShadeY = RasterGradient(sShadeC - sShadeA, sShadeB - sShadeA, sDeltaCX, sDeltaBX, sArea);
		iMinDepth = psA->iDepth;
		iMinDepth = (psB->iDepth < iMinDepth) ? psB->iDepth : iMinDepth;
		iMinDepth = (psC->iDepth < iMinDepth) ? psC->iDepth : iMinDepth;
		iMaxDepth = psA->iDepth;
		iMaxDepth = (psB->iDepth > iMaxDepth) ? psB->iDepth : iMaxDepth;
		iMaxDepth = (psC->iDepth > iMaxDepth) ? psC->iDepth : iMaxDepth;
	}
	sStepDepth = sDepthX.To<SRasterInterpolated>();
	sStepShade = sShadeX.To<SRasterInterpolated>();

	puiPixelRow = &pcTarget->mpuiPixels[(lsize)iMinY * (lsize)pcTarget->miStride];
	piDepthRow = &pcTarget->mpiDepths[(lsize)iMinY * (lsize)pcTarget->miWidth];
	sDepth = SRasterInterpolated::FromRaw(0);
	sShade = SRasterInterpolated::FromRaw(0);
	bWide = (iMaxX - iMinX) >= RASTER_WIDE_PIXELS;
	uiAreaInverse = 0;
#ifdef __AVX2__
	bLanes = bSmall && ((iMaxX - iMinX) < RASTER_LANES) &&
			 RasterLanesFit(sRow0, sStepX0, sStepY0, iMaxY - iMinY + 1) &&
			 RasterLanesFit(sRow1, sStepX1, sStepY1, iMaxY - iMinY + 1) &&
			 RasterLanesFit(sRow2, sStepX2, sStepY2, iMaxY - iMinY + 1);
	if (bLanes)
	{
		RasterLanesInit(&sLanes, sStepX0, sStepX1, sStepX2, iMaxX - iMinX + 1);
		for (y = iMinY; y <= iMaxY; y++)
		{
			uiHits = RasterLaneHits(&sLanes, sRow0, sRow1, sRow2);
			while (uiHits != 0)
			{
				iHit = (int32)_tzcnt_u32(uiHits);
				uiHits &= uiHits - 1;
				x = iMinX + iHit;
				if (uiAreaInverse == 0)
				{
					uiAreaInverse = RasterDivideInverse((uint64)sArea.miRaw);
				}
				sHit0 = SRasterEdge::FromRaw(sRow0.miRaw + sStepX0.miRaw * iHit);
				sHit1 = SRasterEdge::FromRaw(sRow1.miRaw + sStepX1.miRaw * iHit);
				sHit2 = SRasterEdge::FromRaw(sRow2.miRaw + sStepX2.miRaw * iHit);
				iDepth = RasterWeigh(sHit0 - sBias0, sHit1 - sBias1, sHit2 - sBias2, sDepthA, sDepthB, sDepthC, sArea, uiAreaInverse);
				if (iDepth > piDepthRow[x])
				{
					iShade = RasterWeigh(sHit0 - sBias0, sHit1 - sBias1, sHit2 - sBias2, sShadeA, sShadeB, sShadeC, sArea, uiAreaInverse);
					RasterWritePixel(&puiPixelRow[x], &piDepthRow[x], iDepth, iShade, uiColour, bTransparent);
				}
			}
			sRow0 += sStepY0;
			sRow1 += sStepY1;
			sRow2 += sStepY2;
			puiPixelRow += pcTarget->miStride;
			piDepthRow += pcTarget->miWidth;
		}
		return;
	}
#endif
	if (bWide)
	{
		//A step of 0 never divides.
		auiStepInverse[0] = (sStepX0.miRaw != 0) ? RasterDivideInverse((uint64)((sStepX0.miRaw < 0) ? -sStepX0.miRaw : sStepX0.miRaw)) : 0;
		auiStepInverse[1] = (sStepX1.miRaw != 0) ? RasterDivideInverse((uint64)((sStepX1.miRaw < 0) ? -sStepX1.miRaw : sStepX1.miRaw)) : 0;
		auiStepInverse[2] = (sStepX2.miRaw != 0) ? RasterDivideInverse((uint64)((sStepX2.miRaw < 0) ? -sStepX2.miRaw : sStepX2.miRaw)) : 0;
	}
	for (y = iMinY; y <= iMaxY; y++)
	{
		s0 = sRow0;
		s1 = sRow1;
		s2 = sRow2;
		iNextX = -1;
		iFirstX = iMinX;
		iLastX = iMaxX;
		if (bWide)
		{
			//A triangle is convex, so a row covers one run of pixels and each edge says where it can start or stop.
			RasterEdgeSpan(s0, sStepX0, auiStepInverse[0], iMinX, &iFirstX, &iLastX);
			RasterEdgeSpan(s1, sStepX1, auiStepInverse[1], iMinX, &iFirstX, &iLastX);
			RasterEdgeSpan(s2, sStepX2, auiStepInverse[2], iMinX, &iFirstX, &iLastX);
			if (iFirstX > iMinX)
			{
				s0 += SRasterEdge::FromRaw(sStepX0.miRaw * (iFirstX - iMinX));
				s1 += SRasterEdge::FromRaw(sStepX1.miRaw * (iFirstX - iMinX));
				s2 += SRasterEdge::FromRaw(sStepX2.miRaw * (iFirstX - iMinX));
			}
		}
		for (x = iFirstX; x <= iLastX; x++)
		{
			if ((s0.miRaw | s1.miRaw | s2.miRaw) >= 0)
			{
				if (bSmall)
				{
					if (uiAreaInverse == 0)
					{
						uiAreaInverse = RasterDivideInverse((uint64)sArea.miRaw);
					}
					iDepth = RasterWeigh(s0 - sBias0, s1 - sBias1, s2 - sBias2, sDepthA, sDepthB, sDepthC, sArea, uiAreaInverse);
					iShade = 0;  //Worked out below, if the pixel is to be drawn: most of a mesh's pixels are behind another.
				}
				else
				{
					//Each row's run of covered pixels only needs its start working out.
					if (iNextX != x)
					{
						sOffsetX = (SRasterSubpixel::FromInt(x) + SRasterSubpixel::FromRaw(8) - psA->sPosition.x).To<SRasterWideSubpixel>();
						sOffsetY = (SRasterSubpixel::FromInt(y) + SRasterSubpixel::FromRaw(8) - psA->sPosition.y).To<SRasterWideSubpixel>();
						sDepth = sDepthA.To<SRasterInterpolated>() + sDepthX * sOffsetX + sDepthY * sOffsetY;
						sShade = sShadeA.To<SRasterInterpolated>() + sShadeX * sOffsetX + sShadeY * sOffsetY;
					}
					iDepth = sDepth.Floor();
					iShade = sShade.Floor();
					sDepth += sStepDepth;
					sShade += sStepShade;
					iNextX = x + 1;

					//Rounding must not move a pixel out of its triangle's depths, or 0 which is the empty depth.
					if (iDepth < iMinDepth)
					{
						iDepth = iMinDepth;
					}
					else if (iDepth > iMaxDepth)
					{
						iDepth = iMaxDepth;
					}
					if (iShade < 0)
					{
						iShade = 0;
					}
					else if (iShade > 256)
					{
						iShade = 256;
					}
				}

				if (iDepth > piDepthRow[x])
				{
					if (bSmall)
					{
						iShade = RasterWeigh(s0 - sBias0, s1 - sBias1, s2 - sBias2, sShadeA, sShadeB, sShadeC, sArea, uiAreaInverse);
					}
					RasterWritePixel(&puiPixelRow[x], &piDepthRow[x], iDepth, iShade, uiColour, bTransparent);
				}
			}
			s0 += sStepX0;
			s1 += sStepX1;
			s2 += sStepX2;
		}
		sRow0 += sStepY0;
		sRow1 += sStepY1;
		sRow2 += sStepY2;
		puiPixelRow += pcTarget->miStride;
		piDepthRow += pcTarget->miWidth;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What a textured triangle needs at each pixel, made once for it.  A pixel's
// weights are how far it is inside each edge, shifted right by iWeightShift
// for a triangle so big that they'd overflow, which they add up to.  Its
// depth and shade are its corners' by its weights.
//
// Its UV is its first corner's and a fraction of the way to the others', the
// fraction being by its weights times its corners' depths, which are the
// distance from the eye's inverse and so straight across the screen.  The
// depths are aiUVDepth, shifted right so that nothing can overflow.  With an
// orthographic camera they're all one, and the UV is straight across the
// screen too.  aiDeltaU and aiDeltaV are the second and third corners'
// differences from the first's times their aiUVDepth.
//
// Its mip level of detail (iLod, RASTER_LOD_BITS of fraction) is half
// the log of its texels at level 0 over its pixels (each squared to the
// other, as areas): a level coarser for each doubling of texels to a
// pixel across.  With perspective it's for its corners' mean depth, and
// each pixel's is moved a level for each doubling of its distance from
// that (bLodByDepth): far pixels have more texels each.
//
//////////////////////////////////////////////////////////////////////////
struct SRasterTextured
{
	int32	iWeightShift;
	uint64	uiArea;
	uint64	uiAreaInverse;  //Made when a pixel is first drawn, and most triangles draw none.
	int64	aiDepth[3];
	int64	aiShade[3];
	int64	aiUVDepth[3];
	int64	aiDeltaU[2];
	int64	aiDeltaV[2];
	int64	iU;
	int64	iV;
	bool	bConstantUV;
	int32	iLod;
	bool	bLodByDepth;
};


//////////////////////////////////////////////////////////////////////////
//
// The log base 2 of a number, RASTER_LOD_BITS of fraction (0 for 0): its
// highest bit and, as the fraction, the bits after it, which is near
// enough the log between powers of two for picking a mip level.
//
//////////////////////////////////////////////////////////////////////////
static FORCE_INLINE int32 RasterLog2(uint64 uiValue)
{
	uint64	uiHigh;
	int32	iBit;
	int32	iStep;

	if (uiValue == 0)
	{
		return 0;
	}
	iBit = 0;
	uiHigh = uiValue;
	for (iStep = 32; iStep > 0; iStep >>= 1)
	{
		if ((uiHigh >> iStep) != 0)
		{
			uiHigh >>= iStep;
			iBit += iStep;
		}
	}
	return (iBit << RASTER_LOD_BITS) + (int32)(((uiValue << (63 - iBit)) >> (63 - RASTER_LOD_BITS)) & (RASTER_LOD_ONE - 1));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void RasterTexturedInit(SRasterTextured* psTextured, SRasterScreenVertex* psA, SRasterScreenVertex* psB, SRasterScreenVertex* psC, SInt32Vec2* psUVA, SInt32Vec2* psUVB, SInt32Vec2* psUVC, SRasterEdge sArea, bool bPerspective, CRasterTexture* pcTexture)
{
	int64	aiDU[2];
	int64	aiDV[2];
	int64	iCross;
	uint64	uiMaxDelta;
	uint64	uiMaxDepth;
	int32	iWeightBits;
	int32	iDepthShift;
	int		i;

	psTextured->uiArea = (uint64)sArea.miRaw;
	psTextured->uiAreaInverse = 0;
	psTextured->iWeightShift = RasterBits(psTextured->uiArea) - 30;
	if (psTextured->iWeightShift < 0)
	{
		psTextured->iWeightShift = 0;
	}
	iWeightBits = RasterBits(psTextured->uiArea) - psTextured->iWeightShift;

	psTextured->aiDepth[0] = psA->iDepth;
	psTextured->aiDepth[1] = psB->iDepth;
	psTextured->aiDepth[2] = psC->iDepth;
	psTextured->aiShade[0] = psA->iShade;
	psTextured->aiShade[1] = psB->iShade;
	psTextured->aiShade[2] = psC->iShade;

	psTextured->iU = psUVA->x;
	psTextured->iV = psUVA->y;
	aiDU[0] = (int64)psUVB->x - psUVA->x;
	aiDU[1] = (int64)psUVC->x - psUVA->x;
	aiDV[0] = (int64)psUVB->y - psUVA->y;
	aiDV[1] = (int64)psUVC->y - psUVA->y;
	uiMaxDelta = 0;
	for (i = 0; i < 2; i++)
	{
		uiMaxDelta |= (uint64)((aiDU[i] < 0) ? -aiDU[i] : aiDU[i]);
		uiMaxDelta |= (uint64)((aiDV[i] < 0) ? -aiDV[i] : aiDV[i]);
	}
	psTextured->bConstantUV = uiMaxDelta == 0;

	if (bPerspective)
	{
		psTextured->aiUVDepth[0] = psA->iDepth;
		psTextured->aiUVDepth[1] = psB->iDepth;
		psTextured->aiUVDepth[2] = psC->iDepth;
	}
	else
	{
		psTextured->aiUVDepth[0] = 1;
		psTextured->aiUVDepth[1] = 1;
		psTextured->aiUVDepth[2] = 1;
	}
	uiMaxDepth = (uint64)(psTextured->aiUVDepth[0] | psTextured->aiUVDepth[1] | psTextured->aiUVDepth[2]);

	//The weights add up to under 2^iWeightBits, so a weighted sum of depth times delta is under 2^62.
	iDepthShift = iWeightBits + RasterBits(uiMaxDepth) + RasterBits(uiMaxDelta) - 62;
	if (iDepthShift > 0)
	{
		for (i = 0; i < 3; i++)
		{
			psTextured->aiUVDepth[i] >>= iDepthShift;
		}
	}
	for (i = 0; i < 2; i++)
	{
		psTextured->aiDeltaU[i] = aiDU[i] * psTextured->aiUVDepth[i + 1];
		psTextured->aiDeltaV[i] = aiDV[i] * psTextured->aiUVDepth[i + 1];
	}

	//Twice its area in UV has RASTER_UV_BITS of fraction each way, on screen 4 (a subpixel's) each way.
	iCross = (aiDU[0] * aiDV[1]) - (aiDU[1] * aiDV[0]);
	psTextured->iLod = (RasterLog2((uint64)((iCross < 0) ? -iCross : iCross)) + RasterLog2((uint64)pcTexture->GetWidth()) + RasterLog2((uint64)pcTexture->GetHeight()) - ((2 * RASTER_UV_BITS) << RASTER_LOD_BITS)
						- (RasterLog2(psTextured->uiArea) - (8 << RASTER_LOD_BITS))) / 2;
	psTextured->bLodByDepth = bPerspective && (pcTexture->NumLevels() > 1);
	if (psTextured->bLodByDepth)
	{
		psTextured->iLod += RasterLog2((uint64)(((int64)psA->iDepth + psB->iDepth + psC->iDepth) / 3));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The texel for UV (iU, iV) at a pixel iDepth deep, by eFilter.
//
//////////////////////////////////////////////////////////////////////////
static FORCE_INLINE uint32 RasterTexturedSample(SRasterTextured* psTextured, CRasterTexture* pcTexture, ERasterFilter eFilter, int64 iU, int64 iV, int32 iDepth)
{
	int32	iLod;
	int32	iLevel;

	iLod = psTextured->iLod;
	if (psTextured->bLodByDepth)
	{
		iLod -= RasterLog2((uint64)((iDepth > 0) ? iDepth : 1));
	}
	if (eFilter == RF_Trilinear)
	{
		return pcTexture->SampleTrilinear(iU, iV, iLod);
	}
	iLevel = (iLod + (RASTER_LOD_ONE / 2)) >> RASTER_LOD_BITS;
	if (eFilter == RF_Bilinear)
	{
		return pcTexture->SampleBilinear(iU, iV, iLevel);
	}
	return pcTexture->SampleNearest(iU, iV, iLevel);
}


//////////////////////////////////////////////////////////////////////////
//
// The pixel with edge values s0, s1 and s2, its biases taken off, if it's in
// front of what's at *piDepth.
//
//////////////////////////////////////////////////////////////////////////
static FORCE_INLINE void RasterTexturedPixel(SRasterTextured* psTextured, CRasterTexture* pcTexture, ERasterFilter eFilter, uint32* puiPixel, int32* piDepth, SRasterEdge s0, SRasterEdge s1, SRasterEdge s2, uint32 uiColour, bool bTransparent)
{
	int64	iW0;
	int64	iW1;
	int64	iW2;
	uint64	uiSum;
	uint64	uiInverse;
	int64	iWeighted;
	int32	iDepth;
	int32	iShade;
	int64	iU;
	int64	iV;

	iW0 = s0.miRaw >> psTextured->iWeightShift;
	iW1 = s1.miRaw >> psTextured->iWeightShift;
	iW2 = s2.miRaw >> psTextured->iWeightShift;
	if (psTextured->iWeightShift == 0)
	{
		//Unshifted, they add up to the area exactly.
		if (psTextured->uiAreaInverse == 0)
		{
			psTextured->uiAreaInverse = RasterDivideInverse(psTextured->uiArea);
		}
		uiSum = psTextured->uiArea;
		uiInverse = psTextured->uiAreaInverse;
	}
	else
	{
		uiSum = (uint64)(iW0 + iW1 + iW2);
		uiInverse = RasterDivideInverse(uiSum);
	}

	iDepth = (int32)RasterDivideFloor(iW0 * psTextured->aiDepth[0] + iW1 * psTextured->aiDepth[1] + iW2 * psTextured->aiDepth[2], uiSum, uiInverse);
	if (iDepth <= *piDepth)
	{
		return;
	}
	iShade = (int32)RasterDivideFloor(iW0 * psTextured->aiShade[0] + iW1 * psTextured->aiShade[1] + iW2 * psTextured->aiShade[2], uiSum, uiInverse);

	iU = psTextured->iU;
	iV = psTextured->iV;
	if (!psTextured->bConstantUV)
	{
		iWeighted = iW0 * psTextured->aiUVDepth[0] + iW1 * psTextured->aiUVDepth[1] + iW2 * psTextured->aiUVDepth[2];
		if (iWeighted > 0)
		{
			uiInverse = RasterDivideInverse((uint64)iWeighted);
			iU += RasterDivideFloor(iW1 * psTextured->aiDeltaU[0] + iW2 * psTextured->aiDeltaU[1], (uint64)iWeighted, uiInverse);
			iV += RasterDivideFloor(iW1 * psTextured->aiDeltaV[0] + iW2 * psTextured->aiDeltaV[1], (uint64)iWeighted, uiInverse);
		}
	}
	RasterWritePixel(puiPixel, piDepth, iDepth, iShade, RasterModulate(uiColour, RasterTexturedSample(psTextured, pcTexture, eFilter, iU, iV, iDepth)), bTransparent);
}


//////////////////////////////////////////////////////////////////////////
//
// As DrawTriangleRows, with uiColour times pcTexture's texel at each pixel
// (by meFilter, at the pixel's mip level).
// Each pixel is worked out from its weights: a textured pixel costs a divide
// for its UV anyway.  psUVA, psUVB and psUVC are its corners' UVs.
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::DrawTexturedTriangleRows(CRasterTarget* pcTarget, SRasterScreenVertex* psA, SRasterScreenVertex* psB, SRasterScreenVertex* psC, SInt32Vec2* psUVA, SInt32Vec2* psUVB, SInt32Vec2* psUVC, uint32 uiColour, CRasterTexture* pcTexture, bool bTransparent, int32 iTop, int32 iBottom)
{
	SRasterScreenVertex*	psSwap;
	SInt32Vec2*				psSwapUV;
	SRasterEdge				sArea;
	SRasterTextured			sTextured;
	int32					iMinX;
	int32					iMaxX;
	int32					iMinY;
	int32					iMaxY;
	int32					iFirstX;
	int32					iLastX;
	int32					x;
	int32					y;
	SRasterSubpixel			sSampleX;
	SRasterSubpixel			sSampleY;
	SRasterSubpixel			sOnePixel;
	SRasterEdge				sBias0;
	SRasterEdge				sBias1;
	SRasterEdge				sBias2;
	SRasterEdge				sRow0;
	SRasterEdge				sRow1;
	SRasterEdge				sRow2;
	SRasterEdge				s0;
	SRasterEdge				s1;
	SRasterEdge				s2;
	SRasterEdge				sStepX0;
	SRasterEdge				sStepX1;
	SRasterEdge				sStepX2;
	SRasterEdge				sStepY0;
	SRasterEdge				sStepY1;
	SRasterEdge				sStepY2;
	uint32*					puiPixelRow;
	int32*					piDepthRow;
	uint64					auiStepInverse[3];
	bool					bWide;

	if ((psA->iDepth == 0) || (psB->iDepth == 0) || (psC->iDepth == 0))
	{
		return;
	}

	sArea = RasterEdge(&psA->sPosition, &psB->sPosition, psC->sPosition.x, psC->sPosition.y);
	if (sArea.miRaw == 0)
	{
		return;
	}
	if (sArea.miRaw < 0)
	{
		psSwap = psB;
		psB = psC;
		psC = psSwap;
		psSwapUV = psUVB;
		psUVB = psUVC;
		psUVC = psSwapUV;
		sArea = -sArea;
	}
	else if (mbCullBackFaces)
	{
		return;
	}

	sOnePixel = SRasterSubpixel::FromInt(1);
	if (!RasterTriangleBox(pcTarget, psA, psB, psC, iTop, iBottom, &iMinX, &iMinY, &iMaxX, &iMaxY))
	{
		return;
	}

	sSampleX = SRasterSubpixel::FromInt(iMinX) + SRasterSubpixel::FromRaw(8);
	sSampleY = SRasterSubpixel::FromInt(iMinY) + SRasterSubpixel::FromRaw(8);

	sBias0 = RasterEdgeBias(&psB->sPosition, &psC->sPosition);
	sBias1 = RasterEdgeBias(&psC->sPosition, &psA->sPosition);
	sBias2 = RasterEdgeBias(&psA->sPosition, &psB->sPosition);

	sRow0 = RasterEdge(&psB->sPosition, &psC->sPosition, sSampleX, sSampleY) + sBias0;
	sRow1 = RasterEdge(&psC->sPosition, &psA->sPosition, sSampleX, sSampleY) + sBias1;
	sRow2 = RasterEdge(&psA->sPosition, &psB->sPosition, sSampleX, sSampleY) + sBias2;

	sStepX0 = (psB->sPosition.y - psC->sPosition.y) * sOnePixel;
	sStepX1 = (psC->sPosition.y - psA->sPosition.y) * sOnePixel;
	sStepX2 = (psA->sPosition.y - psB->sPosition.y) * sOnePixel;
	sStepY0 = (psC->sPosition.x - psB->sPosition.x) * sOnePixel;
	sStepY1 = (psA->sPosition.x - psC->sPosition.x) * sOnePixel;
	sStepY2 = (psB->sPosition.x - psA->sPosition.x) * sOnePixel;

	RasterTexturedInit(&sTextured, psA, psB, psC, psUVA, psUVB, psUVC, sArea, mbPerspective, pcTexture);

	bWide = (iMaxX - iMinX) >= RASTER_WIDE_PIXELS;
	if (bWide)
	{
		auiStepInverse[0] = (sStepX0.miRaw != 0) ? RasterDivideInverse((uint64)((sStepX0.miRaw < 0) ? -sStepX0.miRaw : sStepX0.miRaw)) : 0;
		auiStepInverse[1] = (sStepX1.miRaw != 0) ? RasterDivideInverse((uint64)((sStepX1.miRaw < 0) ? -sStepX1.miRaw : sStepX1.miRaw)) : 0;
		auiStepInverse[2] = (sStepX2.miRaw != 0) ? RasterDivideInverse((uint64)((sStepX2.miRaw < 0) ? -sStepX2.miRaw : sStepX2.miRaw)) : 0;
	}

	puiPixelRow = &pcTarget->mpuiPixels[(lsize)iMinY * (lsize)pcTarget->miStride];
	piDepthRow = &pcTarget->mpiDepths[(lsize)iMinY * (lsize)pcTarget->miWidth];
	for (y = iMinY; y <= iMaxY; y++)
	{
		s0 = sRow0;
		s1 = sRow1;
		s2 = sRow2;
		iFirstX = iMinX;
		iLastX = iMaxX;
		if (bWide)
		{
			RasterEdgeSpan(s0, sStepX0, auiStepInverse[0], iMinX, &iFirstX, &iLastX);
			RasterEdgeSpan(s1, sStepX1, auiStepInverse[1], iMinX, &iFirstX, &iLastX);
			RasterEdgeSpan(s2, sStepX2, auiStepInverse[2], iMinX, &iFirstX, &iLastX);
			if (iFirstX > iMinX)
			{
				s0 += SRasterEdge::FromRaw(sStepX0.miRaw * (iFirstX - iMinX));
				s1 += SRasterEdge::FromRaw(sStepX1.miRaw * (iFirstX - iMinX));
				s2 += SRasterEdge::FromRaw(sStepX2.miRaw * (iFirstX - iMinX));
			}
		}
		for (x = iFirstX; x <= iLastX; x++)
		{
			if ((s0.miRaw | s1.miRaw | s2.miRaw) >= 0)
			{
				RasterTexturedPixel(&sTextured, pcTexture, meFilter, &puiPixelRow[x], &piDepthRow[x], s0 - sBias0, s1 - sBias1, s2 - sBias2, uiColour, bTransparent);
			}
			s0 += sStepX0;
			s1 += sStepX1;
			s2 += sStepX2;
		}
		sRow0 += sStepY0;
		sRow1 += sStepY1;
		sRow2 += sStepY2;
		puiPixelRow += pcTarget->miStride;
		piDepthRow += pcTarget->miWidth;
	}
}


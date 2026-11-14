/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RasterRenderLib

Codaphela RasterRenderLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RasterRenderLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RasterRenderLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <math.h>
#include <stdlib.h>
#include "BaseLib/Logger.h"
#include "StandardLib/ClassDefines.h"
#include "SupportLib/Instance.h"
#include "SupportLib/Light.h"
#include "RasterRenderer.h"
#include "RasterRenderMesh.h"
#include "RasterFrame.h"


//How far the point the camera turns around is in front of it when there's nothing to put it in the middle of, and the nearest
//it's put, as it can't be where the eye is.
#define RASTER_FRAME_DISTANCE		(1 << 22)
#define RASTER_FRAME_MIN_DISTANCE	1


//////////////////////////////////////////////////////////////////////////
//
// A part of a matrix's row: x, y, z or the position.
//
//////////////////////////////////////////////////////////////////////////
static float32* RasterRow(SFloat4x4* psMatrix, int iRow)
{
	return &(&psMatrix->x)[iRow].x;
}


//////////////////////////////////////////////////////////////////////////
//
// The rotation of a matrix's x, y and z rows: each a unit, at right angles
// to those before it, and pointing as it did, so that a mirror stays one.
//
//////////////////////////////////////////////////////////////////////////
static void RasterRotation(SFloat4x4* psDest, SFloat4x4* psSource)
{
	float64		afRows[3][3];
	float64		fDot;
	float64		fLength;
	float32*	pfRow;
	int			iRow;
	int			iBefore;
	int			k;

	psDest->Identity();
	for (iRow = 0; iRow < 3; iRow++)
	{
		pfRow = RasterRow(psSource, iRow);
		for (k = 0; k < 3; k++)
		{
			afRows[iRow][k] = pfRow[k];
		}
		for (iBefore = 0; iBefore < iRow; iBefore++)
		{
			fDot = afRows[iRow][0] * afRows[iBefore][0] + afRows[iRow][1] * afRows[iBefore][1] + afRows[iRow][2] * afRows[iBefore][2];
			for (k = 0; k < 3; k++)
			{
				afRows[iRow][k] -= fDot * afRows[iBefore][k];
			}
		}
		fLength = sqrt(afRows[iRow][0] * afRows[iRow][0] + afRows[iRow][1] * afRows[iRow][1] + afRows[iRow][2] * afRows[iRow][2]);
		if (fLength <= 0.0)
		{
			for (k = 0; k < 3; k++)
			{
				afRows[iRow][k] = (k == iRow) ? 1.0 : 0.0;
			}
			fLength = 1.0;
		}
		pfRow = RasterRow(psDest, iRow);
		for (k = 0; k < 3; k++)
		{
			afRows[iRow][k] /= fLength;
			pfRow[k] = (float32)afRows[iRow][k];
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A scene distance in the renderer's units, kept in an int32.
//
//////////////////////////////////////////////////////////////////////////
static int32 RasterUnits(float64 fValue, int32 iScale)
{
	fValue = floor(fValue * iScale + 0.5);
	if (fValue > 2147483647.0)
	{
		return 2147483647;
	}
	if (fValue < -2147483647.0)
	{
		return -2147483647;
	}
	return (int32)fValue;
}


//////////////////////////////////////////////////////////////////////////
//
// The axes, light axes and offset of where a world matrix puts a mesh.
// What a position is dotted with for a new part is that part's column.
//
//////////////////////////////////////////////////////////////////////////
static void RasterAxes(SInt32Vec3* asAxes, SInt32Vec3* asLightAxes, SInt32Vec3* psOffset, SFloat4x4* psWorld, int32 iScale)
{
	SFloat4x4	sRotation;
	int			k;

	RasterRotation(&sRotation, psWorld);
	for (k = 0; k < 3; k++)
	{
		asAxes[k].Init(SRasterAxis::FromFloat(RasterRow(psWorld, 0)[k]).miRaw, SRasterAxis::FromFloat(RasterRow(psWorld, 1)[k]).miRaw, SRasterAxis::FromFloat(RasterRow(psWorld, 2)[k]).miRaw);
		asLightAxes[k].Init(SRasterAxis::FromFloat(RasterRow(&sRotation, 0)[k]).miRaw, SRasterAxis::FromFloat(RasterRow(&sRotation, 1)[k]).miRaw, SRasterAxis::FromFloat(RasterRow(&sRotation, 2)[k]).miRaw);
	}
	psOffset->Init(RasterUnits(psWorld->pos.x, iScale), RasterUnits(psWorld->pos.y, iScale), RasterUnits(psWorld->pos.z, iScale));
}


//////////////////////////////////////////////////////////////////////////
//
// A mesh where a world matrix puts it.
//
//////////////////////////////////////////////////////////////////////////
static void RasterPlace(SRasterInstance* psInstance, CRasterMesh* pcMesh, SFloat4x4* psWorld, int32 iScale)
{
	psInstance->Init(pcMesh);
	RasterAxes(psInstance->asAxes, psInstance->asLightAxes, &psInstance->sOffset, psWorld, iScale);
	psInstance->bMirrored = psWorld->Determinant() < 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterFrame::Init(Ptr<CRenderScene> pRenderScene, Ptr<CCamera> pCamera)
{
	PreInit();

	mpRenderScene = pRenderScene;
	mpCamera = pCamera;
	masInstances.Init();
	mapMeshes.Init();
	masPosed.Init();
	masPalette.Init();
	masBones.Init();
	mcCamera.Init();
	msCameraWorld.Identity();
	muiCameraChanges = 0;
	mbCameraSet = false;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterFrame::Class(void)
{
	M_Pointer(mpRenderScene);
	M_Pointer(mpCamera);
	U_Unknown(CArrayRasterInstance, masInstances);
	M_Embedded(mapMeshes);
	U_Unknown(CArrayRasterFramePosed, masPosed);
	U_Unknown(CArrayFloat4x4, masPalette);
	U_Unknown(CArrayRasterBone, masBones);
	U_Data(CRasterCamera, mcCamera);
	U_4x4Float32(msCameraWorld);
	U_Size(muiCameraChanges);
	U_Bool(mbCameraSet);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterFrame::Free(void)
{
	size	i;

	for (i = 0; i < masPosed.NumElements(); i++)
	{
		ReleasePosed(masPosed.Get(i));
	}
	masPosed.Kill();
	masBones.Kill();
	masPalette.Kill();
	masInstances.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The posed vertices of the instance mesh with that object index, made the
// first time.  Each is made once, so an instance's pointer to it stays good
// however many more are made.
//
//////////////////////////////////////////////////////////////////////////
SRasterFramePosed* CRasterFrame::GetPosed(OIndex oiInstanceMesh)
{
	SRasterFramePosed*	psPosed;
	size				i;

	for (i = 0; i < masPosed.NumElements(); i++)
	{
		psPosed = masPosed.Get(i);
		if (psPosed->oiInstanceMesh == oiInstanceMesh)
		{
			psPosed->bUsed = true;
			return psPosed;
		}
	}
	psPosed = masPosed.Add();
	psPosed->oiInstanceMesh = oiInstanceMesh;
	psPosed->pcPosed = (CRasterPosed*)malloc(sizeof(CRasterPosed));
	psPosed->pcPosed->Init();
	psPosed->bMirrorReported = false;
	psPosed->bUsed = true;
	return psPosed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterFrame::ReleasePosed(SRasterFramePosed* psPosed)
{
	psPosed->pcPosed->Kill();
	free(psPosed->pcPosed);
	psPosed->pcPosed = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// What this frame didn't draw or try is let go, so that a mesh that comes
// back starts again, and can report again.
//
//////////////////////////////////////////////////////////////////////////
void CRasterFrame::ReleaseUnusedPosed(void)
{
	size	i;

	for (i = masPosed.NumElements(); i > 0; i--)
	{
		if (!masPosed.Get(i - 1)->bUsed)
		{
			ReleasePosed(masPosed.Get(i - 1));
			masPosed.RemoveAt(i - 1);
		}
	}
	for (i = 0; i < masPosed.NumElements(); i++)
	{
		masPosed.Get(i)->bUsed = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Poses a skinned mesh, and adds its instance, or doesn't if it can't be:
// a missing connection was logged when it was bound, and a mirror is
// logged here the first time.  Its matrices are where the connections put
// it from where it was bound, each placed as a rigid mesh would be.
//
//////////////////////////////////////////////////////////////////////////
bool CRasterFrame::ExtractPosed(CInstance* pcInstance, CInstanceMesh* pcInstanceMesh, CRasterMesh* pcRasterMesh, int32 iScale)
{
	SRasterFramePosed*	psPosed;
	SRasterInstance*	psInstance;
	SRasterBone*		psBone;
	SFloat4x4*			psMatrix;
	size				uiNumMatrices;
	size				i;

	uiNumMatrices = pcRasterMesh->muiNumMatrices;
	if (!pcInstanceMesh->GetPalette(&masPalette) || (masPalette.NumElements() < uiNumMatrices))
	{
		return false;
	}

	psPosed = GetPosed(pcInstanceMesh->GetIndex());
	masBones.ReInit();
	for (i = 0; i < uiNumMatrices; i++)
	{
		psMatrix = masPalette.Get(i);
		if (psMatrix->Determinant() < 0.0f)
		{
			if (!psPosed->bMirrorReported)
			{
				pcInstanceMesh->LogMirrored(pcInstance, i);
				psPosed->bMirrorReported = true;
			}
			return false;
		}
		psBone = masBones.Add();
		RasterAxes(psBone->asAxes, psBone->asLightAxes, &psBone->sOffset, psMatrix, iScale);
	}

	if (!psPosed->pcPosed->Pose(pcRasterMesh, masBones.GetData(), masBones.NumElements()))
	{
		return false;
	}
	psInstance = masInstances.Add();
	psInstance->Init(pcRasterMesh);
	psInstance->pcPosed = psPosed->pcPosed;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterFrame::Extract(CRasteriser* pcRasteriser, int32 iWidth)
{
	Ptr<CRasterRenderer>	pRenderer;
	int32					iScale;

	mpRenderScene->Update();
	pRenderer = mpRenderScene->GetRenderer();
	iScale = pRenderer->GetScale();

	ExtractInstances(iScale);
	ExtractCamera(iWidth, iScale);
	ExtractLights(pcRasteriser->GetLights(), iScale);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterFrame::Draw(CRasteriser* pcRasteriser, CRasterTarget* pcTarget)
{
	Extract(pcRasteriser, pcTarget->miWidth);
	pcRasteriser->Draw(masInstances.GetData(), masInstances.NumElements(), &mcCamera, pcTarget);
}


//////////////////////////////////////////////////////////////////////////
//
// A rigid mesh is placed by its one connection, and a skinned one posed by
// its connections.
//
//////////////////////////////////////////////////////////////////////////
void CRasterFrame::ExtractInstances(int32 iScale)
{
	Ptr<CScene>				pScene;
	Ptr<CInstance>			pInstance;
	Ptr<CInstanceMesh>		pInstanceMesh;
	Ptr<CConnection>		pConnection;
	Ptr<CRasterRenderMesh>	pRenderMesh;
	Ptr<CRasterMesh>		pRasterMesh;
	size					i;
	size					j;

	masInstances.ReInit();
	mapMeshes.RemoveAll();
	pScene = mpRenderScene->GetScene();
	for (i = 0; i < pScene->mapInstances.NumElements(); i++)
	{
		pInstance = pScene->mapInstances.Get(i);
		for (j = 0; j < pInstance->mapMeshes.NumElements(); j++)
		{
			pInstanceMesh = pInstance->mapMeshes.Get(j);
			if (pInstanceMesh->mpEditor.IsNull() || (pInstanceMesh->mapConnections.NumElements() == 0))
			{
				continue;
			}
			pRenderMesh = mpRenderScene->GetMesh(pInstanceMesh->mpEditor);
			if (pRenderMesh.IsNull())
			{
				continue;
			}
			pRasterMesh = pRenderMesh->GetRasterMesh();
			if (pRasterMesh->NumTriangles() == 0)
			{
				continue;
			}
			if (pRasterMesh->IsSkinned())
			{
				if (ExtractPosed(&pInstance, &pInstanceMesh, &pRasterMesh, iScale))
				{
					mapMeshes.Add(pRasterMesh);
				}
			}
			else if (!pInstanceMesh->mpEditor->mpMesh->mcSkin.InUse())
			{
				pConnection = pInstanceMesh->mapConnections.Get(0);
				mapMeshes.Add(pRasterMesh);
				RasterPlace(masInstances.Add(), &pRasterMesh, &pConnection->msWorldMatrix, iScale);
			}
		}
	}
	ReleaseUnusedPosed();
}


//////////////////////////////////////////////////////////////////////////
//
// The controls moved the camera since it last matched the scene camera, so
// that's written back, or the scene camera moved, so the camera follows it.
//
//////////////////////////////////////////////////////////////////////////
void CRasterFrame::ExtractCamera(int32 iWidth, int32 iScale)
{
	Ptr<CConnection>	pConnection;
	float64				fFocalLength;

	if (mpCamera.IsNull() || mpCamera->mpConnection.IsNull())
	{
		return;
	}
	pConnection = mpCamera->mpConnection;

	if (mbCameraSet && (mcCamera.NumChanges() != muiCameraChanges))
	{
		WriteCamera(iScale);
	}
	else if (!mbCameraSet || !Float4x4Equals(&pConnection->msWorldMatrix, &msCameraWorld))
	{
		SetCameraFromScene(&pConnection->msWorldMatrix, iScale);
	}

	mcCamera.SetProjection((mpCamera->meCameraType == CAT_Orthonormal) ? RP_Orthographic : RP_Perspective);
	if ((mpCamera->mfFOV > 0.0f) && (mpCamera->mfFOV < 3.14159f))
	{
		fFocalLength = (iWidth * 0.5) / tan(mpCamera->mfFOV * 0.5);
		mcCamera.SetFocalLength((int32)floor(fFocalLength + 0.5));
	}
	muiCameraChanges = mcCamera.NumChanges();
}


//////////////////////////////////////////////////////////////////////////
//
// The camera looks along its -z with y up, as the screen does, so its axes
// are what turns the scene onto the screen.  The eye is where it is.
//
//////////////////////////////////////////////////////////////////////////
void CRasterFrame::SetCameraFromScene(SFloat4x4* psWorld, int32 iScale)
{
	SFloat4x4		sAxes;
	SFloat4x4		sRotation;
	SFloat32Vec3	sEye;
	SInt32Vec3		sPivot;
	int32			iDistance;
	int				k;

	RasterRotation(&sAxes, psWorld);
	sRotation.Identity();
	for (k = 0; k < 3; k++)
	{
		RasterRow(&sRotation, k)[0] = RasterRow(&sAxes, 0)[k];
		RasterRow(&sRotation, k)[1] = RasterRow(&sAxes, 1)[k];
		RasterRow(&sRotation, k)[2] = RasterRow(&sAxes, 2)[k];
	}

	sEye.Init(psWorld->pos.x * iScale, psWorld->pos.y * iScale, psWorld->pos.z * iScale);
	iDistance = mbCameraSet ? mcCamera.GetDistance() : FindPivotDistance(&sEye);
	sPivot.Init(RasterUnits(psWorld->pos.x - sAxes.z.x * iDistance / iScale, iScale),
				RasterUnits(psWorld->pos.y - sAxes.z.y * iDistance / iScale, iScale),
				RasterUnits(psWorld->pos.z - sAxes.z.z * iDistance / iScale, iScale));
	mcCamera.SetView(&sRotation, &sPivot, iDistance);

	msCameraWorld = *psWorld;
	mbCameraSet = true;
}


//////////////////////////////////////////////////////////////////////////
//
// The eye is the pivot less the pan, and as far as the camera's distance
// back along the screen's z.
//
//////////////////////////////////////////////////////////////////////////
void CRasterFrame::WriteCamera(int32 iScale)
{
	Ptr<CConnection>	pConnection;
	SFloat4x4*			psRotation;
	SFloat4x4			sWorld;
	SFloat4x4			sInverse;
	SInt32Vec3*			psPivot;
	SInt32Vec2*			psPan;
	float64				afEye[3];
	int32				iDistance;
	int					k;

	psRotation = mcCamera.GetRotation();
	psPivot = mcCamera.GetPivot();
	psPan = mcCamera.GetPan();
	iDistance = mcCamera.GetDistance();

	sWorld.Identity();
	for (k = 0; k < 3; k++)
	{
		RasterRow(&sWorld, 0)[k] = RasterRow(psRotation, k)[0];
		RasterRow(&sWorld, 1)[k] = RasterRow(psRotation, k)[1];
		RasterRow(&sWorld, 2)[k] = RasterRow(psRotation, k)[2];
		afEye[k] = (&psPivot->x)[k] - (float64)psPan->x * RasterRow(&sWorld, 0)[k] - (float64)psPan->y * RasterRow(&sWorld, 1)[k] + (float64)iDistance * RasterRow(&sWorld, 2)[k];
	}
	sWorld.pos.Init((float32)(afEye[0] / iScale), (float32)(afEye[1] / iScale), (float32)(afEye[2] / iScale), 1.0f);

	pConnection = mpCamera->mpConnection;
	pConnection->msWorldMatrix = sWorld;
	if (pConnection->mpParent.IsNotNull() && Float4x4Inverse(&sInverse, NULL, &pConnection->mpParent->msWorldMatrix))
	{
		Float4x4Multiply(&pConnection->msLocalMatrix, &sWorld, &sInverse);
	}
	else
	{
		pConnection->msLocalMatrix = sWorld;
	}
	pConnection->mbParity = false;
	msCameraWorld = sWorld;
}


//////////////////////////////////////////////////////////////////////////
//
// From the eye to the middle of the box everything drawn is in.
//
//////////////////////////////////////////////////////////////////////////
int32 CRasterFrame::FindPivotDistance(SFloat32Vec3* psEye)
{
	SRasterInstance*	psInstance;
	float64				afMin[3];
	float64				afMax[3];
	float64				afCorner[3];
	float64				fMoved;
	float64				fDistance;
	SInt32Vec3*			psMin;
	SInt32Vec3*			psMax;
	size				i;
	int					iCorner;
	int					k;

	if (masInstances.NumElements() == 0)
	{
		return RASTER_FRAME_DISTANCE;
	}

	for (k = 0; k < 3; k++)
	{
		afMin[k] = 1e300;
		afMax[k] = -1e300;
	}
	for (i = 0; i < masInstances.NumElements(); i++)
	{
		psInstance = masInstances.Get(i);
		psMin = &psInstance->pcMesh->msMin;
		psMax = &psInstance->pcMesh->msMax;
		for (iCorner = 0; iCorner < 8; iCorner++)
		{
			afCorner[0] = (iCorner & 1) ? psMax->x : psMin->x;
			afCorner[1] = (iCorner & 2) ? psMax->y : psMin->y;
			afCorner[2] = (iCorner & 4) ? psMax->z : psMin->z;
			for (k = 0; k < 3; k++)
			{
				fMoved = (afCorner[0] * psInstance->asAxes[k].x + afCorner[1] * psInstance->asAxes[k].y + afCorner[2] * psInstance->asAxes[k].z) / (1 << RASTER_AXIS_BITS) + (&psInstance->sOffset.x)[k];
				afMin[k] = (fMoved < afMin[k]) ? fMoved : afMin[k];
				afMax[k] = (fMoved > afMax[k]) ? fMoved : afMax[k];
			}
		}
	}

	fDistance = 0.0;
	for (k = 0; k < 3; k++)
	{
		fMoved = (afMin[k] + afMax[k]) * 0.5 - (&psEye->x)[k];
		fDistance += fMoved * fMoved;
	}
	fDistance = sqrt(fDistance);
	if (fDistance < RASTER_FRAME_MIN_DISTANCE)
	{
		return RASTER_FRAME_MIN_DISTANCE;
	}
	return (fDistance < 2147483647.0) ? (int32)fDistance : 2147483647;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterFrame::ExtractLights(CRasterLights* pcLights, int32 iScale)
{
	Ptr<CScene>		pScene;
	Ptr<CLight>		pLight;
	SFloat4x4*		psWorld;
	SRasterUnitVec3	sTowards;
	SInt32Vec3		sPosition;
	float64			fLength;
	int32			iAmount;
	int32			iAmbient;
	size			i;

	pScene = mpRenderScene->GetScene();
	if (pScene->mapLights.NumElements() == 0)
	{
		pcLights->Default();
		return;
	}

	pcLights->Clear();
	iAmbient = 0;
	for (i = 0; i < pScene->mapLights.NumElements(); i++)
	{
		pLight = pScene->mapLights.Get(i);
		iAmount = (int32)floor(RASTER_MAX_SHADE * pLight->GetStrength() + 0.5);
		if (iAmount <= 0)
		{
			continue;
		}
		if (pLight->meType == LT_Ambient)
		{
			iAmbient += iAmount;
			continue;
		}
		if (pLight->mpConnection.IsNull())
		{
			continue;
		}

		psWorld = &pLight->mpConnection->msWorldMatrix;
		if (pLight->meType == LT_Directional)
		{
			fLength = sqrt((float64)psWorld->z.x * psWorld->z.x + (float64)psWorld->z.y * psWorld->z.y + (float64)psWorld->z.z * psWorld->z.z);
			if (fLength > 0.0)
			{
				sTowards.Init(SRasterUnit::FromFloat((float32)(psWorld->z.x / fLength)), SRasterUnit::FromFloat((float32)(psWorld->z.y / fLength)), SRasterUnit::FromFloat((float32)(psWorld->z.z / fLength)));
				pcLights->AddDirectional(&sTowards, iAmount, false);
			}
		}
		else
		{
			sPosition.Init(RasterUnits(psWorld->pos.x, iScale), RasterUnits(psWorld->pos.y, iScale), RasterUnits(psWorld->pos.z, iScale));
			pcLights->AddPoint(&sPosition, iAmount, (pLight->meDecay == LDT_Linear) ? RLD_Linear : ((pLight->meDecay == LDT_Square) ? RLD_Square : RLD_None),
							   RasterUnits(pLight->mfDecayStart, iScale), RasterUnits(pLight->mfRange, iScale));
		}
	}
	pcLights->SetAmbient(iAmbient);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CRasterCamera* CRasterFrame::GetCamera(void)
{
	return &mcCamera;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SRasterInstance* CRasterFrame::GetInstances(void)
{
	return masInstances.GetData();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CRasterFrame::NumInstances(void)
{
	return masInstances.NumElements();
}


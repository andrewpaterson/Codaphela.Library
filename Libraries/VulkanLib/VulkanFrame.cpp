/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela VulkanLib

Codaphela VulkanLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela VulkanLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela VulkanLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <math.h>
#include <string.h>
#include "BaseLib/Float4x4.h"
#include "StandardLib/ClassDefines.h"
#include "SupportLib/Instance.h"
#include "SupportLib/Light.h"
#include "VulkanFrame.h"


//As CRasteriser shades: the most a shade can be, and its light when a scene has none, 64 ambient and 192 from over the viewer's
//shoulder, which is towards x, y and z on the screen in 16384ths (see CRasterLights::Default).  That's a little short of a unit,
//and as the rasteriser doesn't make it one neither is it here.
#define VULKAN_MAX_SHADE		256.0f
#define VULKAN_DEFAULT_AMBIENT	64.0f
#define VULKAN_DEFAULT_DIFFUSE	192.0f
#define VULKAN_DEFAULT_TOWARDS	4096.0f, 6144.0f, 14336.0f
#define VULKAN_DEFAULT_UNIT		16384.0f

//A point light that never falls off reaches this far.
#define VULKAN_FOREVER			1.0e30f

//The ways a point light falls off, as the shader has them.
#define VULKAN_DECAY_NONE		0.0f
#define VULKAN_DECAY_LINEAR		1.0f
#define VULKAN_DECAY_SQUARE		2.0f


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVulkanFrame::Init(Ptr<CRenderScene> pRenderScene, Ptr<CCamera> pCamera)
{
	PreInit();

	mpRenderScene = pRenderScene;
	mpCamera = pCamera;
	masInstances.Init();
	mapMeshes.Init();
	memset(&msData, 0, sizeof(msData));

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVulkanFrame::Class(void)
{
	M_Pointer(mpRenderScene);
	M_Pointer(mpCamera);
	U_Unknown(CArrayVulkanFrameInstance, masInstances);
	M_Embedded(mapMeshes);
	U_Data(SVulkanFrameData, msData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVulkanFrame::Free(void)
{
	masInstances.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The rows of a matrix's inverse transpose, which turn a normal as the
// matrix turns what it's on.
//
//////////////////////////////////////////////////////////////////////////
static void VulkanNormalRows(float32 aafRows[3][4], SFloat4x4* psWorld)
{
	SFloat4x4	sInverse;
	float32*	pfInverse;
	int			i;
	int			k;

	Float4x4Inverse(&sInverse, NULL, psWorld);
	pfInverse = (float32*)&sInverse;
	for (i = 0; i < 3; i++)
	{
		for (k = 0; k < 3; k++)
		{
			aafRows[i][k] = pfInverse[k * 4 + i];
		}
		aafRows[i][3] = 0.0f;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A rigid mesh is placed by its one connection.
//
//////////////////////////////////////////////////////////////////////////
void CVulkanFrame::ExtractInstances(void)
{
	Ptr<CScene>				pScene;
	Ptr<CInstance>			pInstance;
	Ptr<CInstanceMesh>		pInstanceMesh;
	Ptr<CVulkanRenderMesh>	pRenderMesh;
	SVulkanFrameInstance*	psInstance;
	SFloat4x4*				psWorld;
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
			if (pInstanceMesh->mpEditor.IsNull() || pInstanceMesh->mpEditor->mpMesh->mcSkin.InUse() || (pInstanceMesh->mapConnections.NumElements() == 0))
			{
				continue;
			}
			pRenderMesh = mpRenderScene->GetMesh(pInstanceMesh->mpEditor);
			if (pRenderMesh.IsNull() || !pRenderMesh->IsDrawable())
			{
				continue;
			}

			psWorld = &pInstanceMesh->mapConnections.Get(0)->msWorldMatrix;
			mapMeshes.Add(pRenderMesh);
			psInstance = masInstances.Add();
			psInstance->pcMesh = &pRenderMesh;
			memcpy(psInstance->sDraw.afWorld, psWorld, sizeof(psInstance->sDraw.afWorld));
			VulkanNormalRows(psInstance->sDraw.aafNormal, psWorld);
			psInstance->bMirrored = psWorld->Determinant() < 0.0f;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The view is the camera's world matrix's inverse.  The projection's y is
// down the screen, and its depth 0 at the near plane and 1 at the far, as
// Vulkan has them.  Vectors are on the left.
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanFrame::ExtractCamera(int32 iWidth, int32 iHeight)
{
	Ptr<CConnection>	pConnection;
	SFloat4x4			sView;
	SFloat4x4			sProjection;
	SFloat4x4			sViewProjection;
	float32				fAcross;
	float32				fNear;
	float32				fFar;

	if (mpCamera.IsNull() || mpCamera->mpConnection.IsNull())
	{
		return false;
	}
	pConnection = mpCamera->mpConnection;
	Float4x4Inverse(&sView, NULL, &pConnection->msWorldMatrix);

	fAcross = 1.0f / tanf(mpCamera->mfFOV * 0.5f);
	fNear = mpCamera->mfNear;
	fFar = mpCamera->mfFar;
	memset(&sProjection, 0, sizeof(sProjection));
	sProjection.x.x = fAcross;
	sProjection.y.y = -fAcross * (float32)iWidth / (float32)iHeight;
	sProjection.z.z = fFar / (fNear - fFar);
	sProjection.z.w = -1.0f;
	sProjection.pos.z = fNear * fFar / (fNear - fFar);

	Float4x4Multiply(&sViewProjection, &sView, &sProjection);
	memcpy(msData.afViewProjection, &sViewProjection, sizeof(msData.afViewProjection));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// What a point light falls off by, and how far it reaches before what it
// adds is less than a shade, as CRasterLights works it out, or its range if
// that's nearer.
//
//////////////////////////////////////////////////////////////////////////
static void VulkanPointLight(float32* pafLight, CLight* pcLight, SFloat4x4* psWorld, float32 fAmount)
{
	float32	fStart;
	float32	fReach;
	float32	fDecay;

	fStart = (pcLight->mfDecayStart > 0.0f) ? pcLight->mfDecayStart : 1.0e-6f;
	if (pcLight->meDecay == LDT_Linear)
	{
		fDecay = VULKAN_DECAY_LINEAR;
		fReach = fAmount * fStart;
	}
	else if (pcLight->meDecay == LDT_Square)
	{
		fDecay = VULKAN_DECAY_SQUARE;
		fReach = fStart * (floorf(sqrtf(fAmount)) + 1.0f);
	}
	else
	{
		fDecay = VULKAN_DECAY_NONE;
		fReach = VULKAN_FOREVER;
	}
	if ((pcLight->mfRange > 0.0f) && (pcLight->mfRange < fReach))
	{
		fReach = pcLight->mfRange;
	}

	pafLight[0] = psWorld->pos.x;
	pafLight[1] = psWorld->pos.y;
	pafLight[2] = psWorld->pos.z;
	pafLight[3] = fAmount;
	pafLight[4] = fStart;
	pafLight[5] = fReach;
	pafLight[6] = fDecay;
	pafLight[7] = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
// A unit towards a light far away, and its amount.
//
//////////////////////////////////////////////////////////////////////////
static void VulkanDirectionalLight(float32* pafLight, float32 x, float32 y, float32 z, float32 fAmount)
{
	float32	fLength;

	fLength = sqrtf(x * x + y * y + z * z);
	pafLight[0] = (fLength > 0.0f) ? x / fLength : 0.0f;
	pafLight[1] = (fLength > 0.0f) ? y / fLength : 0.0f;
	pafLight[2] = (fLength > 0.0f) ? z / fLength : 0.0f;
	pafLight[3] = (fLength > 0.0f) ? fAmount : 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
// The default light is over the shoulder of the camera, so it's turned out
// of the camera's space.
//
//////////////////////////////////////////////////////////////////////////
void CVulkanFrame::ExtractLights(void)
{
	Ptr<CScene>		pScene;
	Ptr<CLight>		pLight;
	SFloat4x4*		psWorld;
	SFloat4x4*		psCamera;
	SFloat32Vec3	sOnScreen;
	float32			fAmount;
	float32			fAmbient;
	size			uiNumDirectional;
	size			uiNumPoint;
	size			i;

	pScene = mpRenderScene->GetScene();
	fAmbient = 0.0f;
	uiNumDirectional = 0;
	uiNumPoint = 0;
	for (i = 0; i < pScene->mapLights.NumElements(); i++)
	{
		pLight = pScene->mapLights.Get(i);
		fAmount = floorf((float32)(VULKAN_MAX_SHADE * pLight->GetStrength()) + 0.5f);
		if (fAmount <= 0.0f)
		{
			continue;
		}
		if (pLight->meType == LT_Ambient)
		{
			fAmbient += fAmount;
		}
		else if (pLight->mpConnection.IsNotNull())
		{
			psWorld = &pLight->mpConnection->msWorldMatrix;
			if ((pLight->meType == LT_Directional) && (uiNumDirectional < VULKAN_MAX_DIRECTIONAL_LIGHTS))
			{
				VulkanDirectionalLight(msData.aafDirectional[uiNumDirectional], psWorld->z.x, psWorld->z.y, psWorld->z.z, fAmount);
				uiNumDirectional++;
			}
			else if ((pLight->meType != LT_Directional) && (uiNumPoint < VULKAN_MAX_POINT_LIGHTS))
			{
				VulkanPointLight(msData.aafPoint[uiNumPoint], &pLight, psWorld, fAmount);
				uiNumPoint++;
			}
		}
	}

	if (pScene->mapLights.NumElements() == 0)
	{
		fAmbient = VULKAN_DEFAULT_AMBIENT;
		psCamera = &mpCamera->mpConnection->msWorldMatrix;
		sOnScreen.Init(VULKAN_DEFAULT_TOWARDS);
		msData.aafDirectional[0][0] = (sOnScreen.x * psCamera->x.x + sOnScreen.y * psCamera->y.x + sOnScreen.z * psCamera->z.x) / VULKAN_DEFAULT_UNIT;
		msData.aafDirectional[0][1] = (sOnScreen.x * psCamera->x.y + sOnScreen.y * psCamera->y.y + sOnScreen.z * psCamera->z.y) / VULKAN_DEFAULT_UNIT;
		msData.aafDirectional[0][2] = (sOnScreen.x * psCamera->x.z + sOnScreen.y * psCamera->y.z + sOnScreen.z * psCamera->z.z) / VULKAN_DEFAULT_UNIT;
		msData.aafDirectional[0][3] = VULKAN_DEFAULT_DIFFUSE;
		uiNumDirectional = 1;
	}

	msData.afLights[0] = fAmbient;
	msData.afLights[1] = (float32)uiNumDirectional;
	msData.afLights[2] = (float32)uiNumPoint;
	msData.afLights[3] = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
// False, drawing nothing, with no camera.
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanFrame::Extract(int32 iWidth, int32 iHeight)
{
	mpRenderScene->Update();
	ExtractInstances();
	if (!ExtractCamera(iWidth, iHeight))
	{
		return false;
	}
	ExtractLights();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVulkanFrame::DrawSubmeshes(CVulkanRenderer* pcRenderer, bool bTransparent)
{
	SVulkanFrameInstance*	psInstance;
	SVulkanSubmesh*			psSubmesh;
	size					i;
	size					j;
	bool					bBound;

	for (i = 0; i < masInstances.NumElements(); i++)
	{
		psInstance = masInstances.Get(i);
		bBound = false;
		for (j = 0; j < psInstance->pcMesh->NumSubmeshes(); j++)
		{
			psSubmesh = psInstance->pcMesh->GetSubmesh(j);
			if (psSubmesh->bTransparent != bTransparent)
			{
				continue;
			}
			if (!bBound)
			{
				psInstance->pcMesh->Bind();
				bBound = true;
			}
			psInstance->sDraw.afColour[0] = (float32)((psSubmesh->uiColour >> 16) & 0xFF) / 255.0f;
			psInstance->sDraw.afColour[1] = (float32)((psSubmesh->uiColour >> 8) & 0xFF) / 255.0f;
			psInstance->sDraw.afColour[2] = (float32)(psSubmesh->uiColour & 0xFF) / 255.0f;
			psInstance->sDraw.afColour[3] = (float32)(psSubmesh->uiColour >> 24) / 255.0f;
			pcRenderer->UseTexture(psSubmesh->hTexture);
			pcRenderer->Draw(&psInstance->sDraw, psInstance->bMirrored, psSubmesh->uiFirstIndex, psSubmesh->uiNumIndices);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Every opaque triangle is drawn before any transparent one, which aren't
// sorted, as CRasteriser draws them.  False if nothing could be drawn.
//
//////////////////////////////////////////////////////////////////////////
bool CVulkanFrame::Draw(uint32* puiPixels, int32 iWidth, int32 iHeight, uint32 uiBackground)
{
	Ptr<CVulkanRenderer>	pRenderer;

	pRenderer = mpRenderScene->GetRenderer();
	if (!pRenderer->IsReady() || !Extract(iWidth, iHeight) || !pRenderer->BeginFrame(iWidth, iHeight, uiBackground, &msData))
	{
		return false;
	}
	pRenderer->UseOpaque();
	DrawSubmeshes(&pRenderer, false);
	pRenderer->UseTransparent();
	DrawSubmeshes(&pRenderer, true);
	return pRenderer->EndFrame(puiPixels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CVulkanFrame::NumInstances(void)
{
	return masInstances.NumElements();
}

